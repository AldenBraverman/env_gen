/*
  ==============================================================================

    EnvelopeLane.cpp
    A complete lane UI with step sequencer, envelope controls, and destination

  ==============================================================================
*/

#include "EnvelopeLane.h"

EnvelopeLane::EnvelopeLane(juce::AudioProcessorValueTreeState& apvts, int laneNumber)
    : laneIndex(laneNumber - 1)
{
    juce::String prefix = "lane" + juce::String(laneNumber);

    // Setup lane label
    laneLabel.setText("Lane " + juce::String(laneNumber), juce::dontSendNotification);
    laneLabel.setJustificationType(juce::Justification::centred);
    laneLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(laneLabel);

    // Setup step buttons
    for (int i = 0; i < NUM_STEPS; ++i)
    {
        addAndMakeVisible(stepButtons[i]);
        juce::String paramId = prefix + "_step" + juce::String(i);
        stepAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            apvts, paramId, stepButtons[i]);
    }

    // Setup envelope sliders
    setupSlider(attackSlider, attackLabel, "A");
    setupSlider(holdSlider, holdLabel, "H");
    setupSlider(decaySlider, decayLabel, "D");

    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix + "_attack", attackSlider);
    holdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix + "_hold", holdSlider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, prefix + "_decay", decaySlider);

    // Setup rate combo
    setupComboBox(rateCombo, rateLabel, "Rate");
    rateCombo.addItemList({ "1/1", "1/2", "1/4", "1/8", "1/16", "1/32" }, 1);
    rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, prefix + "_rate", rateCombo);

    // Setup destination combo
    setupComboBox(destCombo, destLabel, "Dest");
    destCombo.addItemList({ "Cutoff", "Volume" }, 1);
    destAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        apvts, prefix + "_destination", destCombo);

    // Start timer for step visualization
    startTimerHz(30);
}

EnvelopeLane::~EnvelopeLane()
{
    stopTimer();
}

void EnvelopeLane::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background panel
    g.setColour(CustomLookAndFeel::panelColour);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Draw subtle border
    g.setColour(CustomLookAndFeel::accentColour.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void EnvelopeLane::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    
    const int labelWidth = 50;
    const int knobSize = 50;
    const int comboWidth = 65;
    const int comboHeight = 24;
    const int stepSize = 24;
    const int spacing = 4;
    
    // Lane label on the left
    laneLabel.setBounds(bounds.removeFromLeft(labelWidth));
    bounds.removeFromLeft(spacing);

    // Destination combo
    auto destArea = bounds.removeFromLeft(comboWidth);
    destLabel.setBounds(destArea.removeFromTop(16));
    destCombo.setBounds(destArea.removeFromTop(comboHeight));
    bounds.removeFromLeft(spacing);

    // Envelope knobs (A, H, D)
    auto knobArea = bounds.removeFromLeft(knobSize);
    attackLabel.setBounds(knobArea.removeFromTop(16));
    attackSlider.setBounds(knobArea.removeFromTop(knobSize - 16));
    bounds.removeFromLeft(spacing);

    knobArea = bounds.removeFromLeft(knobSize);
    holdLabel.setBounds(knobArea.removeFromTop(16));
    holdSlider.setBounds(knobArea.removeFromTop(knobSize - 16));
    bounds.removeFromLeft(spacing);

    knobArea = bounds.removeFromLeft(knobSize);
    decayLabel.setBounds(knobArea.removeFromTop(16));
    decaySlider.setBounds(knobArea.removeFromTop(knobSize - 16));
    bounds.removeFromLeft(spacing);

    // Rate combo
    auto rateArea = bounds.removeFromLeft(comboWidth);
    rateLabel.setBounds(rateArea.removeFromTop(16));
    rateCombo.setBounds(rateArea.removeFromTop(comboHeight));
    bounds.removeFromLeft(spacing * 2);

    // Step buttons - fill remaining space
    int totalStepWidth = bounds.getWidth();
    int stepWidth = (totalStepWidth - (NUM_STEPS - 1) * 2) / NUM_STEPS;
    stepWidth = juce::jmin(stepWidth, stepSize);
    
    int stepY = bounds.getCentreY() - stepWidth / 2;
    int stepX = bounds.getX();

    for (int i = 0; i < NUM_STEPS; ++i)
    {
        stepButtons[i].setBounds(stepX, stepY, stepWidth, stepWidth);
        stepX += stepWidth + 2;
    }
}

void EnvelopeLane::timerCallback()
{
    // Update step visualization from parent (would need processor reference)
    // For now, rely on setCurrentStep being called from editor
}

void EnvelopeLane::setCurrentStep(int step)
{
    if (step != currentPlayingStep)
    {
        // Clear previous
        if (currentPlayingStep >= 0 && currentPlayingStep < NUM_STEPS)
        {
            stepButtons[currentPlayingStep].setPlaying(false);
        }
        
        currentPlayingStep = step;
        
        // Set new
        if (currentPlayingStep >= 0 && currentPlayingStep < NUM_STEPS)
        {
            stepButtons[currentPlayingStep].setPlaying(true);
        }
    }
}

void EnvelopeLane::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& labelText)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(slider);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    label.setFont(juce::Font(11.0f));
    addAndMakeVisible(label);
}

void EnvelopeLane::setupComboBox(juce::ComboBox& combo, juce::Label& label, const juce::String& labelText)
{
    addAndMakeVisible(combo);

    label.setText(labelText, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    label.setFont(juce::Font(11.0f));
    addAndMakeVisible(label);
}
