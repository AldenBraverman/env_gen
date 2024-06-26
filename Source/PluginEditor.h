/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/customLookAndFeel.h"

//==============================================================================
/**
*/
class Env_genAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Env_genAudioProcessorEditor (Env_genAudioProcessor&);
    ~Env_genAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Env_genAudioProcessor& audioProcessor;

    customLookAndFeel otherLookAndFeel;
    juce::ToggleButton lane_one_0b;
    juce::TextButton lane_one_1b;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Env_genAudioProcessorEditor)
};
