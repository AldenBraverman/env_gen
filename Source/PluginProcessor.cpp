/*
  ==============================================================================

    PluginProcessor.cpp
    Envelope Generator Audio Plugin

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "Components/OscilloscopeComponent.h"
#if ENVGEN_USE_WEB_GUI
#include "PluginEditorWeb.h"
#else
#include "PluginEditor.h"
#endif

//==============================================================================
EnvGenAudioProcessor::EnvGenAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Get global parameter pointers
    inputGainParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("inputGain"));
    outputGainParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("outputGain"));
    dryPassParam = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("dryPass"));

    // Get per-lane parameter pointers (single lane)
    const juce::String prefix("lane1");
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        juce::String paramName = prefix + "_step" + juce::String(step);
        laneParams[0].steps[step] = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(paramName));
    }
    laneParams[0].attack = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_attack"));
    laneParams[0].hold = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_hold"));
    laneParams[0].decay = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_decay"));
    laneParams[0].amount = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(prefix + "_amount"));
    laneParams[0].rate = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(prefix + "_rate"));
    laneParams[0].destination = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(prefix + "_destination"));
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
    // Prepare envelopes and sequencers
    for (int i = 0; i < NUM_LANES; ++i)
    {
        envelopes[i].prepare(sampleRate);
        sequencers[i].prepare(sampleRate);
    }

    // Initialize from parameters
    updateLaneFromParams(0);

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
    updateLaneFromParams(0);

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Process sample by sample for accurate envelope/sequencer timing
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Process sequencer and trigger envelope
        if (sequencers[0].process(positionInfo))
        {
            envelopes[0].trigger();
        }

        // Single lane: apply to amplitude only when destination is Amplitude
        float envValue = envelopes[0].process();
        float volumeModulation = 0.0f;
        if (laneParams[0].destination != nullptr && laneParams[0].destination->getIndex() == 1) // Amplitude
        {
            float amount = laneParams[0].amount->get();  // -1.0 to 1.0
            volumeModulation = envValue * amount;
        }

        // Volume gain: Dry OFF = silence until envelope; Dry ON = dry at unity, envelope adds on top
        bool dryPass = dryPassParam->get();
        float baseGain = dryPass ? 1.0f : 0.0f;
        float volumeGain;
        if (volumeModulation > 0.0f)
            volumeGain = baseGain + volumeModulation * 3.0f;  // envelope adds on top
        else if (volumeModulation < 0.0f)
            volumeGain = baseGain + volumeModulation;          // can pull down from base
        else
            volumeGain = baseGain;                             // no envelope: 0 or 1
        volumeGain = juce::jmax(0.0f, volumeGain);

        // Apply volume modulation to each channel
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            channelData[sample] *= volumeGain;
        }
    }

    // Apply output gain
    float outputGainLinear = juce::Decibels::decibelsToGain(outputGainParam->get());
    buffer.applyGain(outputGainLinear);

    // Push data to scope sink if connected
    if (scopeSink != nullptr)
    {
        // Convert PositionInfo to CurrentPositionInfo for scope
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
        
        scopeSink->updatePlayheadInfo(currentPosInfo);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            float monoSample = 0.0f;
            for (int channel = 0; channel < numChannels; ++channel)
            {
                monoSample += buffer.getSample(channel, sample);
            }
            monoSample /= static_cast<float>(numChannels);
            monoBuffer[static_cast<size_t>(sample)] = juce::jlimit(-1.0f, 1.0f, monoSample);
            float envelopeSum = envelopes[0].getCurrentValue();
            envelopeBuffer[static_cast<size_t>(sample)] = juce::jlimit(0.0f, 1.0f, envelopeSum);
        }
        
        scopeSink->pushBuffer(monoBuffer.data(), numSamples);
        scopeSink->pushEnvelopeBuffer(envelopeBuffer.data(), numSamples);
    }
}

//==============================================================================
bool EnvGenAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* EnvGenAudioProcessor::createEditor()
{
#if ENVGEN_USE_WEB_GUI
    return new EnvGenEditorWeb(*this);
#else
    return new EnvGenAudioProcessorEditor(*this);
#endif
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
void EnvGenAudioProcessor::resetAllParametersToDefault()
{
    for (auto* param : getParameters())
    {
        if (param != nullptr)
            param->setValueNotifyingHost(param->getDefaultValue());
    }
}

//==============================================================================
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

    layout.add(std::make_unique<juce::AudioParameterBool>(
        ::ParameterID::dryPass, "Dry", false));  // OFF = silence until envelope; ON = dry passes, envelope adds

    // Single lane parameters (steps, attack, hold, decay, rate, amount)
    juce::StringArray rateChoices{ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" };
    const juce::String prefix("lane1");

    // Step buttons
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        juce::String stepId = prefix + "_step" + juce::String(step);
        juce::String stepName = "Step " + juce::String(step + 1);
        layout.add(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID(stepId, 1), stepName, false));
    }

    // Envelope parameters
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(prefix + "_attack", 1), "Attack",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f),
        0.01f, "s"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(prefix + "_hold", 1), "Hold",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.001f, 0.3f),
        0.1f, "s"));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(prefix + "_decay", 1), "Decay",
        juce::NormalisableRange<float>(0.001f, 10.0f, 0.001f, 0.3f),
        0.5f, "s"));

    // Rate
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(prefix + "_rate", 1), "Rate",
        rateChoices, 4)); // Default to 1/16

    // Destination (None = no modulation, Amplitude = volume)
    juce::StringArray destChoices{ "None", "Amplitude" };
    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID(prefix + "_destination", 1), "Assign",
        destChoices, 0)); // Default to None

    // Amount (bipolar: -100% to +100%)
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID(prefix + "_amount", 1), "Amount",
        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
        1.0f)); // Default to 100%

    return layout;
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EnvGenAudioProcessor();
}
