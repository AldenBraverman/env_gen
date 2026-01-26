/*
  ==============================================================================

    PluginProcessor.cpp
    Envelope Generator Audio Plugin

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Components/OscilloscopeComponent.h"

//==============================================================================
EnvGenAudioProcessor::EnvGenAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Get global parameter pointers
    inputGainParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("inputGain"));
    outputGainParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("outputGain"));
    filterModeParam = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter("filterMode"));
    filterCutoffParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterCutoff"));
    filterResonanceParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("filterResonance"));

    // Get per-lane parameter pointers
    const char* laneNames[] = { "lane1", "lane2", "lane3", "lane4" };
    
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        juce::String prefix(laneNames[lane]);
        
        for (int step = 0; step < NUM_STEPS; ++step)
        {
            juce::String paramName = prefix + "_step" + juce::String(step);
            laneParams[lane].steps[step] = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(paramName));
        }
        
        laneParams[lane].attack = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_attack"));
        laneParams[lane].hold = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_hold"));
        laneParams[lane].decay = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_decay"));
        laneParams[lane].amount = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_amount"));
        laneParams[lane].rate = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(prefix + "_rate"));
        laneParams[lane].destination = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(prefix + "_destination"));
    }
}

EnvGenAudioProcessor::~EnvGenAudioProcessor()
{
}

//==============================================================================
const juce::String EnvGenAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EnvGenAudioProcessor::acceptsMidi() const
{
    return false;
}

bool EnvGenAudioProcessor::producesMidi() const
{
    return false;
}

bool EnvGenAudioProcessor::isMidiEffect() const
{
    return false;
}

double EnvGenAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EnvGenAudioProcessor::getNumPrograms()
{
    return 1;
}

int EnvGenAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EnvGenAudioProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String EnvGenAudioProcessor::getProgramName(int /*index*/)
{
    return {};
}

void EnvGenAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

//==============================================================================
void EnvGenAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Prepare filter
    filter.prepare(sampleRate, samplesPerBlock, 2);

    // Prepare envelopes and sequencers
    for (int i = 0; i < NUM_LANES; ++i)
    {
        envelopes[i].prepare(sampleRate);
        sequencers[i].prepare(sampleRate);
    }

    // Initialize from parameters
    updateFilterFromParams();
    for (int i = 0; i < NUM_LANES; ++i)
    {
        updateLaneFromParams(i);
    }

    // Allocate temporary buffers for oscilloscope data
    monoBuffer.resize(static_cast<size_t>(samplesPerBlock));
    envelopeBuffer.resize(static_cast<size_t>(samplesPerBlock));
}

void EnvGenAudioProcessor::releaseResources()
{
}

bool EnvGenAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void EnvGenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Apply input gain
    float inputGainLinear = juce::Decibels::decibelsToGain(inputGainParam->get());
    buffer.applyGain(inputGainLinear);

    // Get playhead info
    juce::AudioPlayHead::PositionInfo positionInfo;
    if (auto* playHead = getPlayHead())
    {
        auto pos = playHead->getPosition();
        if (pos.hasValue())
            positionInfo = *pos;
    }

    // Update parameters
    updateFilterFromParams();
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        updateLaneFromParams(lane);
    }

    // Get base filter cutoff
    float baseCutoff = filterCutoffParam->get();
    float baseResonance = filterResonanceParam->get();
    int filterModeIndex = filterModeParam->getIndex();
    
    filter.setMode(static_cast<Filter::Mode>(filterModeIndex));
    filter.setResonance(baseResonance);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Process sample by sample for accurate envelope/sequencer timing
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Process each lane's sequencer and trigger envelopes
        for (int lane = 0; lane < NUM_LANES; ++lane)
        {
            if (sequencers[lane].process(positionInfo))
            {
                envelopes[lane].trigger();
            }
        }

        // Calculate modulation amounts
        float cutoffModulation = 0.0f;
        float volumeModulation = 0.0f;

        for (int lane = 0; lane < NUM_LANES; ++lane)
        {
            float envValue = envelopes[lane].process();
            float amount = laneParams[lane].amount->get();  // -1.0 to 1.0
            float modulatedEnvValue = envValue * amount;
            
            int destIndex = laneParams[lane].destination->getIndex();
            
            if (destIndex == 0) // Filter Cutoff
            {
                cutoffModulation += modulatedEnvValue;
            }
            else // Volume
            {
                volumeModulation += modulatedEnvValue;
            }
        }

        // Apply cutoff modulation (exponential scaling for musical response)
        // Modulation adds up to 4 octaves to the base cutoff
        float modulatedCutoff = baseCutoff * std::pow(2.0f, cutoffModulation * 4.0f);
        modulatedCutoff = juce::jlimit(20.0f, 20000.0f, modulatedCutoff);
        filter.setCutoff(modulatedCutoff);

        // Calculate volume gain (envelope adds to base gain of 1.0)
        // When no envelope is active targeting volume, gain = 1.0
        // Envelope can boost volume by up to 4x (12dB)
        float volumeGain = 1.0f;
        if (volumeModulation > 0.0f)
        {
            volumeGain = 1.0f + volumeModulation * 3.0f; // 1.0 to 4.0
        }
        else if (volumeModulation < 0.0f)
        {
            // Negative amount reduces volume (inverted envelope)
            volumeGain = 1.0f + volumeModulation; // Can go down to 0.0
            volumeGain = juce::jmax(0.0f, volumeGain);
        }

        // Process each channel
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            
            // Apply filter
            float filtered = filter.processSample(channelData[sample], channel);
            
            // Apply volume modulation
            channelData[sample] = filtered * volumeGain;
        }
    }

    // Apply output gain
    float outputGainLinear = juce::Decibels::decibelsToGain(outputGainParam->get());
    buffer.applyGain(outputGainLinear);

    // Push data to oscilloscope if connected
    if (oscilloscope != nullptr)
    {
        // Convert PositionInfo to CurrentPositionInfo for oscilloscope
        juce::AudioPlayHead::CurrentPositionInfo currentPosInfo;
        currentPosInfo.resetToDefault();
        
        auto bpmOpt = positionInfo.getBpm();
        auto ppqOpt = positionInfo.getPpqPosition();
        auto timeSigOpt = positionInfo.getTimeSignature();
        
        currentPosInfo.bpm = bpmOpt.hasValue() ? *bpmOpt : 120.0;
        currentPosInfo.ppqPosition = ppqOpt.hasValue() ? *ppqOpt : 0.0;
        currentPosInfo.isPlaying = positionInfo.getIsPlaying();
        
        if (timeSigOpt.hasValue())
        {
            currentPosInfo.timeSigNumerator = timeSigOpt->numerator;
            currentPosInfo.timeSigDenominator = timeSigOpt->denominator;
        }
        
        // Update playhead info first (handles measure boundary detection)
        oscilloscope->updatePlayheadInfo(currentPosInfo);
        
        // Prepare mono mix buffer
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float monoSample = 0.0f;
            for (int channel = 0; channel < numChannels; ++channel)
            {
                monoSample += buffer.getSample(channel, sample);
            }
            monoSample /= static_cast<float>(numChannels);
            monoBuffer[static_cast<size_t>(sample)] = juce::jlimit(-1.0f, 1.0f, monoSample);
            
            // Sum of all envelope values
            float envelopeSum = 0.0f;
            for (int lane = 0; lane < NUM_LANES; ++lane)
            {
                envelopeSum += envelopes[lane].getCurrentValue();
            }
            envelopeBuffer[static_cast<size_t>(sample)] = juce::jlimit(0.0f, 4.0f, envelopeSum);
        }
        
        // Push to oscilloscope
        oscilloscope->pushBuffer(monoBuffer.data(), numSamples);
        oscilloscope->pushEnvelopeBuffer(envelopeBuffer.data(), numSamples);
    }
}

//==============================================================================
bool EnvGenAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* EnvGenAudioProcessor::createEditor()
{
    return new EnvGenAudioProcessorEditor(*this);
}

//==============================================================================
void EnvGenAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void EnvGenAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
int EnvGenAudioProcessor::getCurrentStep(int laneIndex) const
{
    if (laneIndex >= 0 && laneIndex < NUM_LANES)
        return sequencers[laneIndex].getCurrentStep();
    return 0;
}

//==============================================================================
void EnvGenAudioProcessor::updateFilterFromParams()
{
    filter.setCutoff(filterCutoffParam->get());
    filter.setResonance(filterResonanceParam->get());
    filter.setMode(static_cast<Filter::Mode>(filterModeParam->getIndex()));
}

void EnvGenAudioProcessor::updateLaneFromParams(int laneIndex)
{
    if (laneIndex < 0 || laneIndex >= NUM_LANES)
        return;

    auto& params = laneParams[laneIndex];
    auto& envelope = envelopes[laneIndex];
    auto& sequencer = sequencers[laneIndex];

    // Update envelope parameters
    envelope.setAttack(params.attack->get());
    envelope.setHold(params.hold->get());
    envelope.setDecay(params.decay->get());

    // Update sequencer steps
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        sequencer.setStep(step, params.steps[step]->get());
    }

    // Update sequencer rate
    sequencer.setRate(static_cast<StepSequencer::Rate>(params.rate->getIndex()));
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout EnvGenAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    // Global gain parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ::ParameterID::inputGain, "Input Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f, "dB"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ::ParameterID::outputGain, "Output Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f),
        0.0f, "dB"));

    // Global filter parameters
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        ::ParameterID::filterMode, "Filter Mode",
        juce::StringArray{ "Lowpass", "Highpass", "Bandpass" }, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ::ParameterID::filterCutoff, "Filter Cutoff",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 0.1f, 0.3f), // Skewed for better control
        1000.0f, "Hz"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        ::ParameterID::filterResonance, "Filter Resonance",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.0f));

    // Rate choices
    juce::StringArray rateChoices{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" };
    juce::StringArray destChoices{ "Filter Cutoff", "Volume" };

    // Lane parameters
    auto addLaneParams = [&](const juce::String& prefix, int laneNum)
    {
        // Step buttons
        for (int step = 0; step < NUM_STEPS; ++step)
        {
            juce::String stepId = prefix + "_step" + juce::String(step);
            juce::String stepName = "Lane " + juce::String(laneNum) + " Step " + juce::String(step + 1);
            layout.add(std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID(stepId, 1), stepName, false));
        }

        // Envelope parameters
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "_attack", 1), "Lane " + juce::String(laneNum) + " Attack",
            juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f),
            0.01f, "s"));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "_hold", 1), "Lane " + juce::String(laneNum) + " Hold",
            juce::NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f),
            0.1f, "s"));

        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "_decay", 1), "Lane " + juce::String(laneNum) + " Decay",
            juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f),
            0.5f, "s"));

        // Rate
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(prefix + "_rate", 1), "Lane " + juce::String(laneNum) + " Rate",
            rateChoices, 4)); // Default to 1/16

        // Destination
        layout.add(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID(prefix + "_destination", 1), "Lane " + juce::String(laneNum) + " Destination",
            destChoices, 0)); // Default to Filter Cutoff

        // Amount (bipolar: -100% to +100%)
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(prefix + "_amount", 1), "Lane " + juce::String(laneNum) + " Amount",
            juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
            1.0f)); // Default to 100%
    };

    addLaneParams("lane1", 1);
    addLaneParams("lane2", 2);
    addLaneParams("lane3", 3);
    addLaneParams("lane4", 4);

    return layout;
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnvGenAudioProcessor();
}
