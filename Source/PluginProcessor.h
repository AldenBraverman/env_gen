/*
  ==============================================================================

    PluginProcessor.h
    Envelope Generator Audio Plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DSP/Envelope.h"
#include "DSP/Filter.h"
#include "DSP/StepSequencer.h"

// Forward declaration
class OsciloscopeComponent;

//==============================================================================
// Parameter IDs
namespace ParameterID
{
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);

    // Global parameters
    PARAMETER_ID(inputGain)
    PARAMETER_ID(outputGain)
    PARAMETER_ID(dryPass)
    PARAMETER_ID(filterMode)
    PARAMETER_ID(filterCutoff)
    PARAMETER_ID(filterResonance)

    // Lane parameters - using arrays would be cleaner but macros make it simpler
    // Lane 1
    PARAMETER_ID(lane1_step0)  PARAMETER_ID(lane1_step1)  PARAMETER_ID(lane1_step2)  PARAMETER_ID(lane1_step3)
    PARAMETER_ID(lane1_step4)  PARAMETER_ID(lane1_step5)  PARAMETER_ID(lane1_step6)  PARAMETER_ID(lane1_step7)
    PARAMETER_ID(lane1_step8)  PARAMETER_ID(lane1_step9)  PARAMETER_ID(lane1_step10) PARAMETER_ID(lane1_step11)
    PARAMETER_ID(lane1_step12) PARAMETER_ID(lane1_step13) PARAMETER_ID(lane1_step14) PARAMETER_ID(lane1_step15)
    PARAMETER_ID(lane1_attack) PARAMETER_ID(lane1_hold)   PARAMETER_ID(lane1_decay)  PARAMETER_ID(lane1_rate)
    PARAMETER_ID(lane1_destination) PARAMETER_ID(lane1_amount)

    // Lane 2
    PARAMETER_ID(lane2_step0)  PARAMETER_ID(lane2_step1)  PARAMETER_ID(lane2_step2)  PARAMETER_ID(lane2_step3)
    PARAMETER_ID(lane2_step4)  PARAMETER_ID(lane2_step5)  PARAMETER_ID(lane2_step6)  PARAMETER_ID(lane2_step7)
    PARAMETER_ID(lane2_step8)  PARAMETER_ID(lane2_step9)  PARAMETER_ID(lane2_step10) PARAMETER_ID(lane2_step11)
    PARAMETER_ID(lane2_step12) PARAMETER_ID(lane2_step13) PARAMETER_ID(lane2_step14) PARAMETER_ID(lane2_step15)
    PARAMETER_ID(lane2_attack) PARAMETER_ID(lane2_hold)   PARAMETER_ID(lane2_decay)  PARAMETER_ID(lane2_rate)
    PARAMETER_ID(lane2_destination) PARAMETER_ID(lane2_amount)

    // Lane 3
    PARAMETER_ID(lane3_step0)  PARAMETER_ID(lane3_step1)  PARAMETER_ID(lane3_step2)  PARAMETER_ID(lane3_step3)
    PARAMETER_ID(lane3_step4)  PARAMETER_ID(lane3_step5)  PARAMETER_ID(lane3_step6)  PARAMETER_ID(lane3_step7)
    PARAMETER_ID(lane3_step8)  PARAMETER_ID(lane3_step9)  PARAMETER_ID(lane3_step10) PARAMETER_ID(lane3_step11)
    PARAMETER_ID(lane3_step12) PARAMETER_ID(lane3_step13) PARAMETER_ID(lane3_step14) PARAMETER_ID(lane3_step15)
    PARAMETER_ID(lane3_attack) PARAMETER_ID(lane3_hold)   PARAMETER_ID(lane3_decay)  PARAMETER_ID(lane3_rate)
    PARAMETER_ID(lane3_destination) PARAMETER_ID(lane3_amount)

    // Lane 4
    PARAMETER_ID(lane4_step0)  PARAMETER_ID(lane4_step1)  PARAMETER_ID(lane4_step2)  PARAMETER_ID(lane4_step3)
    PARAMETER_ID(lane4_step4)  PARAMETER_ID(lane4_step5)  PARAMETER_ID(lane4_step6)  PARAMETER_ID(lane4_step7)
    PARAMETER_ID(lane4_step8)  PARAMETER_ID(lane4_step9)  PARAMETER_ID(lane4_step10) PARAMETER_ID(lane4_step11)
    PARAMETER_ID(lane4_step12) PARAMETER_ID(lane4_step13) PARAMETER_ID(lane4_step14) PARAMETER_ID(lane4_step15)
    PARAMETER_ID(lane4_attack) PARAMETER_ID(lane4_hold)   PARAMETER_ID(lane4_decay)  PARAMETER_ID(lane4_rate)
    PARAMETER_ID(lane4_destination) PARAMETER_ID(lane4_amount)

    #undef PARAMETER_ID
}

//==============================================================================
class EnvGenAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    static constexpr int NUM_LANES = 4;
    static constexpr int NUM_STEPS = 16;

    enum class Destination
    {
        FilterCutoff,
        Volume
    };

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

    // Set oscilloscope component for waveform display
    void setOscilloscope(OsciloscopeComponent* osc) { oscilloscope = osc; }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // DSP components
    Filter filter;
    Envelope envelopes[NUM_LANES];
    StepSequencer sequencers[NUM_LANES];

    // Parameter pointers for fast access
    // Global
    juce::AudioParameterFloat* inputGainParam = nullptr;
    juce::AudioParameterFloat* outputGainParam = nullptr;
    juce::AudioParameterBool* dryPassParam = nullptr;
    juce::AudioParameterChoice* filterModeParam = nullptr;
    juce::AudioParameterFloat* filterCutoffParam = nullptr;
    juce::AudioParameterFloat* filterResonanceParam = nullptr;

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
    void updateFilterFromParams();
    void updateLaneFromParams(int laneIndex);

    // Oscilloscope component for waveform display (owned by editor)
    OsciloscopeComponent* oscilloscope = nullptr;
    
    // Temporary buffers for oscilloscope data (allocated once in prepareToPlay)
    std::vector<float> monoBuffer;
    std::vector<float> envelopeBuffer;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvGenAudioProcessor)
};
