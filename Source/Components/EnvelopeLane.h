/*
  ==============================================================================

    EnvelopeLane.h
    Lane UI with step sequencer and envelope controls (attack, hold, decay, rate, amount)

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "StepButton.h"
#include "CustomLookAndFeel.h"

class EnvelopeLane : public juce::Component,
                     public juce::Timer
{
public:
    static constexpr int NUM_STEPS = 16;

    EnvelopeLane(juce::AudioProcessorValueTreeState& apvts, int laneNumber);
    ~EnvelopeLane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

    void setCurrentStep(int step);

private:
    int currentPlayingStep = -1;

    // Labels
    juce::Label laneLabel;
    juce::Label attackLabel;
    juce::Label holdLabel;
    juce::Label decayLabel;
    juce::Label amountLabel;
    juce::Label rateLabel;

    // Step buttons
    StepButton stepButtons[NUM_STEPS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stepAttachments[NUM_STEPS];

    // Envelope knobs
    juce::Slider attackSlider;
    juce::Slider holdSlider;
    juce::Slider decaySlider;
    juce::Slider amountSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> holdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountAttachment;

    // Rate selector
    juce::ComboBox rateCombo;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAttachment;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText);
    void setupComboBox(juce::ComboBox& combo, juce::Label& label, const juce::String& labelText);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopeLane)
};
