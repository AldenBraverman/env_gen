/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


namespace ParameterID
{
    #define PARAMETER_ID(str) const juce::ParameterID str(#str, 1);

    // Lane One
    PARAMETER_ID(lane_one_0b);
    PARAMETER_ID(lane_one_1b);
    PARAMETER_ID(lane_one_2b);
    PARAMETER_ID(lane_one_3b);
    PARAMETER_ID(lane_one_4b);
    PARAMETER_ID(lane_one_5b);
    PARAMETER_ID(lane_one_6b);
    PARAMETER_ID(lane_one_7b);
    PARAMETER_ID(lane_one_8b);
    PARAMETER_ID(lane_one_9b);
    PARAMETER_ID(lane_one_10b);
    PARAMETER_ID(lane_one_11b);
    PARAMETER_ID(lane_one_12b);
    PARAMETER_ID(lane_one_13b);
    PARAMETER_ID(lane_one_14b);
    PARAMETER_ID(lane_one_15b);

    PARAMETER_ID(lane_one_attack);
    PARAMETER_ID(lane_one_hold);
    PARAMETER_ID(lane_one_decay);
    PARAMETER_ID(lane_one_rate);

    // Lane Two
    PARAMETER_ID(lane_two_0b);
    PARAMETER_ID(lane_two_1b);
    PARAMETER_ID(lane_two_2b);
    PARAMETER_ID(lane_two_3b);
    PARAMETER_ID(lane_two_4b);
    PARAMETER_ID(lane_two_5b);
    PARAMETER_ID(lane_two_6b);
    PARAMETER_ID(lane_two_7b);
    PARAMETER_ID(lane_two_8b);
    PARAMETER_ID(lane_two_9b);
    PARAMETER_ID(lane_two_10b);
    PARAMETER_ID(lane_two_11b);
    PARAMETER_ID(lane_two_12b);
    PARAMETER_ID(lane_two_13b);
    PARAMETER_ID(lane_two_14b);
    PARAMETER_ID(lane_two_15b);

    PARAMETER_ID(lane_two_attack);
    PARAMETER_ID(lane_two_hold);
    PARAMETER_ID(lane_two_decay);
    PARAMETER_ID(lane_two_rate);

    // Lane Three
    PARAMETER_ID(lane_three_0b);
    PARAMETER_ID(lane_three_1b);
    PARAMETER_ID(lane_three_2b);
    PARAMETER_ID(lane_three_3b);
    PARAMETER_ID(lane_three_4b);
    PARAMETER_ID(lane_three_5b);
    PARAMETER_ID(lane_three_6b);
    PARAMETER_ID(lane_three_7b);
    PARAMETER_ID(lane_three_8b);
    PARAMETER_ID(lane_three_9b);
    PARAMETER_ID(lane_three_10b);
    PARAMETER_ID(lane_three_11b);
    PARAMETER_ID(lane_three_12b);
    PARAMETER_ID(lane_three_13b);
    PARAMETER_ID(lane_three_14b);
    PARAMETER_ID(lane_three_15b);

    PARAMETER_ID(lane_three_attack);
    PARAMETER_ID(lane_three_hold);
    PARAMETER_ID(lane_three_decay);
    PARAMETER_ID(lane_three_rate);

    // Lane Four
    PARAMETER_ID(lane_four_0b);
    PARAMETER_ID(lane_four_1b);
    PARAMETER_ID(lane_four_2b);
    PARAMETER_ID(lane_four_3b);
    PARAMETER_ID(lane_four_4b);
    PARAMETER_ID(lane_four_5b);
    PARAMETER_ID(lane_four_6b);
    PARAMETER_ID(lane_four_7b);
    PARAMETER_ID(lane_four_8b);
    PARAMETER_ID(lane_four_9b);
    PARAMETER_ID(lane_four_10b);
    PARAMETER_ID(lane_four_11b);
    PARAMETER_ID(lane_four_12b);
    PARAMETER_ID(lane_four_13b);
    PARAMETER_ID(lane_four_14b);
    PARAMETER_ID(lane_four_15b);

    PARAMETER_ID(lane_four_attack);
    PARAMETER_ID(lane_four_hold);
    PARAMETER_ID(lane_four_decay);
    PARAMETER_ID(lane_four_rate);


}



//==============================================================================
/**
*/
class Env_genAudioProcessor  : public juce::AudioProcessor,
    private juce::ValueTree::Listener
{
public:
    //==============================================================================
    Env_genAudioProcessor();
    ~Env_genAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts(*this, nullptr, "Parameters", createParameterLayout() );

private:

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // Lane One
    juce::AudioParameterBool* lane_one_0b_param;
    juce::AudioParameterBool* lane_one_1b_param;
    juce::AudioParameterBool* lane_one_2b_param;
    juce::AudioParameterBool* lane_one_3b_param;
    juce::AudioParameterBool* lane_one_4b_param;
    juce::AudioParameterBool* lane_one_5b_param;
    juce::AudioParameterBool* lane_one_6b_param;
    juce::AudioParameterBool* lane_one_7b_param;
    juce::AudioParameterBool* lane_one_8b_param;
    juce::AudioParameterBool* lane_one_9b_param;
    juce::AudioParameterBool* lane_one_10b_param;
    juce::AudioParameterBool* lane_one_11b_param;
    juce::AudioParameterBool* lane_one_12b_param;
    juce::AudioParameterBool* lane_one_13b_param;
    juce::AudioParameterBool* lane_one_14b_param;
    juce::AudioParameterBool* lane_one_15b_param;

    // Lane Two
    juce::AudioParameterBool* lane_two_0b_param;
    juce::AudioParameterBool* lane_two_1b_param;
    juce::AudioParameterBool* lane_two_2b_param;
    juce::AudioParameterBool* lane_two_3b_param;
    juce::AudioParameterBool* lane_two_4b_param;
    juce::AudioParameterBool* lane_two_5b_param;
    juce::AudioParameterBool* lane_two_6b_param;
    juce::AudioParameterBool* lane_two_7b_param;
    juce::AudioParameterBool* lane_two_8b_param;
    juce::AudioParameterBool* lane_two_9b_param;
    juce::AudioParameterBool* lane_two_10b_param;
    juce::AudioParameterBool* lane_two_11b_param;
    juce::AudioParameterBool* lane_two_12b_param;
    juce::AudioParameterBool* lane_two_13b_param;
    juce::AudioParameterBool* lane_two_14b_param;
    juce::AudioParameterBool* lane_two_15b_param;

    // Lane Three
    juce::AudioParameterBool* lane_three_0b_param;
    juce::AudioParameterBool* lane_three_1b_param;
    juce::AudioParameterBool* lane_three_2b_param;
    juce::AudioParameterBool* lane_three_3b_param;
    juce::AudioParameterBool* lane_three_4b_param;
    juce::AudioParameterBool* lane_three_5b_param;
    juce::AudioParameterBool* lane_three_6b_param;
    juce::AudioParameterBool* lane_three_7b_param;
    juce::AudioParameterBool* lane_three_8b_param;
    juce::AudioParameterBool* lane_three_9b_param;
    juce::AudioParameterBool* lane_three_10b_param;
    juce::AudioParameterBool* lane_three_11b_param;
    juce::AudioParameterBool* lane_three_12b_param;
    juce::AudioParameterBool* lane_three_13b_param;
    juce::AudioParameterBool* lane_three_14b_param;
    juce::AudioParameterBool* lane_three_15b_param;

    // Lane Four
    juce::AudioParameterBool* lane_four_0b_param;
    juce::AudioParameterBool* lane_four_1b_param;
    juce::AudioParameterBool* lane_four_2b_param;
    juce::AudioParameterBool* lane_four_3b_param;
    juce::AudioParameterBool* lane_four_4b_param;
    juce::AudioParameterBool* lane_four_5b_param;
    juce::AudioParameterBool* lane_four_6b_param;
    juce::AudioParameterBool* lane_four_7b_param;
    juce::AudioParameterBool* lane_four_8b_param;
    juce::AudioParameterBool* lane_four_9b_param;
    juce::AudioParameterBool* lane_four_10b_param;
    juce::AudioParameterBool* lane_four_11b_param;
    juce::AudioParameterBool* lane_four_12b_param;
    juce::AudioParameterBool* lane_four_13b_param;
    juce::AudioParameterBool* lane_four_14b_param;
    juce::AudioParameterBool* lane_four_15b_param;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Env_genAudioProcessor)
};
