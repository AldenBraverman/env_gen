/*
  ==============================================================================

    PluginEditor.h
    Envelope Generator Plugin Editor

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Components/CustomLookAndFeel.h"
#include "Components/EnvelopeLane.h"
#include "Components/OscilloscopeComponent.h"

//==============================================================================
class EnvGenAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
public:
    EnvGenAudioProcessorEditor(EnvGenAudioProcessor&);
    ~EnvGenAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    EnvGenAudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;

    // Header section
    juce::Label titleLabel;
    
    // Gain controls
    juce::Label inputGainLabel;
    juce::Label outputGainLabel;
    juce::Slider inputGainSlider;
    juce::Slider outputGainSlider;
    juce::ToggleButton dryPassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> dryPassAttachment;

    // Oscilloscope display
    std::unique_ptr<OsciloscopeComponent> oscilloscope;

    // Single envelope lane
    std::unique_ptr<EnvelopeLane> envelopeLane;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvGenAudioProcessorEditor)
};
