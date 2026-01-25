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
    
    // Filter controls
    juce::Label filterLabel;
    juce::Label filterModeLabel;
    juce::Label filterCutoffLabel;
    juce::Label filterResonanceLabel;
    
    juce::ComboBox filterModeCombo;
    juce::Slider filterCutoffSlider;
    juce::Slider filterResonanceSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> filterModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;

    // Envelope lanes
    std::unique_ptr<EnvelopeLane> lanes[EnvGenAudioProcessor::NUM_LANES];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvGenAudioProcessorEditor)
};
