/*
  ==============================================================================

    PluginProcessor.h
    Envelope Generator Audio Plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/Envelope.h"
#include "DSP/StepSequencer.h"
#include "ScopeDataSink.h"

//==============================================================================
// Parameter IDs
namespace ParameterID
{
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);

    // Global parameters
    PARAMETER_ID(inputGain)
    PARAMETER_ID(outputGain)
    PARAMETER_ID(dryPass)

    // Lane 1 (single lane)
    PARAMETER_ID(lane1_step0)  PARAMETER_ID(lane1_step1)  PARAMETER_ID(lane1_step2)  PARAMETER_ID(lane1_step3)
    PARAMETER_ID(lane1_step4)  PARAMETER_ID(lane1_step5)  PARAMETER_ID(lane1_step6)  PARAMETER_ID(lane1_step7)
    PARAMETER_ID(lane1_step8)  PARAMETER_ID(lane1_step9)  PARAMETER_ID(lane1_step10) PARAMETER_ID(lane1_step11)
    PARAMETER_ID(lane1_step12) PARAMETER_ID(lane1_step13) PARAMETER_ID(lane1_step14) PARAMETER_ID(lane1_step15)
    PARAMETER_ID(lane1_attack) PARAMETER_ID(lane1_hold)   PARAMETER_ID(lane1_decay)  PARAMETER_ID(lane1_rate)
    PARAMETER_ID(lane1_destination) PARAMETER_ID(lane1_amount)

    #undef PARAMETER_ID
}

//==============================================================================
class EnvGenAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    static constexpr int NUM_LANES = 1;
    static constexpr int NUM_STEPS = 16;

    //==============================================================================
    EnvGenAudioProcessor();
    ~EnvGenAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState apvts{ *this, nullptr, "Parameters", createParameterLayout() };

    // Get current step for UI visualization
    int getCurrentStep(int laneIndex) const;

    // Set scope data sink for waveform display (native OsciloscopeComponent or web ScopeBuffer)
    void setScopeSink(ScopeDataSink* sink) { scopeSink = sink; }

    /** Set every parameter to its default value (from createParameterLayout). */
    void resetAllParametersToDefault();

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP components
    Envelope envelopes[NUM_LANES];
    StepSequencer sequencers[NUM_LANES];

    // Parameter pointers for fast access
    // Global
    juce::AudioParameterFloat* inputGainParam = nullptr;
    juce::AudioParameterFloat* outputGainParam = nullptr;
    juce::AudioParameterBool* dryPassParam = nullptr;

    // Per-lane parameters
    struct LaneParams
    {
        juce::AudioParameterBool* steps[NUM_STEPS] = { nullptr };
        juce::AudioParameterFloat* attack = nullptr;
        juce::AudioParameterFloat* hold = nullptr;
        juce::AudioParameterFloat* decay = nullptr;
        juce::AudioParameterFloat* amount = nullptr;
        juce::AudioParameterChoice* rate = nullptr;
        juce::AudioParameterChoice* destination = nullptr;
    };
    LaneParams laneParams[NUM_LANES];

    // Helper to get step parameter IDs
    static juce::ParameterID getStepParamID(int laneIndex, int stepIndex);

    // Update DSP from parameters
    void updateLaneFromParams(int laneIndex);

    // Scope data sink for waveform display (owned by editor: native or web)
    ScopeDataSink* scopeSink = nullptr;
    
    // Temporary buffers for oscilloscope data (allocated once in prepareToPlay)
    std::vector<float> monoBuffer;
    std::vector<float> envelopeBuffer;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvGenAudioProcessor)
};
