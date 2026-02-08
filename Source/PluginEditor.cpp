/*
  ==============================================================================

    PluginEditor.cpp
    Envelope Generator Plugin Editor

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
EnvGenAudioProcessorEditor::EnvGenAudioProcessorEditor(EnvGenAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);

    // Title
    titleLabel.setText("Envelope Generator", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold)));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(titleLabel);

    // Input gain slider
    inputGainLabel.setText("Input", juce::dontSendNotification);
    inputGainLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    inputGainLabel.setJustificationType(juce::Justification::centred);
    inputGainLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(inputGainLabel);

    inputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    inputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(inputGainSlider);
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "inputGain", inputGainSlider);

    // Output gain slider
    outputGainLabel.setText("Output", juce::dontSendNotification);
    outputGainLabel.setFont(juce::Font(juce::FontOptions(11.0f)));
    outputGainLabel.setJustificationType(juce::Justification::centred);
    outputGainLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(outputGainLabel);

    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
    outputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(outputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "outputGain", outputGainSlider);

    // Dry pass toggle (ON = dry at unity, envelope adds; OFF = silence until envelope)
    dryPassButton.setButtonText("Dry");
    dryPassButton.setColour(juce::ToggleButton::tickColourId, CustomLookAndFeel::accentColour);
    dryPassButton.setColour(juce::ToggleButton::tickDisabledColourId, CustomLookAndFeel::textColour.withAlpha(0.5f));
    addAndMakeVisible(dryPassButton);
    dryPassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "dryPass", dryPassButton);

    // Reset All button
    resetAllButton.setButtonText("Reset All");
    resetAllButton.onClick = [this]() { audioProcessor.resetAllParametersToDefault(); };
    addAndMakeVisible(resetAllButton);

    // Create oscilloscope display
    oscilloscope = std::make_unique<OsciloscopeComponent>();
    
    // Configure oscilloscope colors to match CustomLookAndFeel
    oscilloscope->setBackgroundColour(CustomLookAndFeel::panelColour);
    oscilloscope->setWaveformColour(CustomLookAndFeel::accentColour);
    oscilloscope->setGridColour(CustomLookAndFeel::textColour.withAlpha(0.3f));
    oscilloscope->setEnvelopeColour(CustomLookAndFeel::stepActiveColour);
    oscilloscope->setShowGrid(true);
    oscilloscope->setShowEnvelope(true);
    
    addAndMakeVisible(*oscilloscope);
    
    // Register oscilloscope with processor for audio data
    audioProcessor.setScopeSink(oscilloscope.get());

    // Create single envelope lane
    envelopeLane = std::make_unique<EnvelopeLane>(audioProcessor.apvts, 1);
    addAndMakeVisible(*envelopeLane);

    // Start timer for step visualization
    startTimerHz(30);

    setSize(900, 520);
}

EnvGenAudioProcessorEditor::~EnvGenAudioProcessorEditor()
{
    // Unregister oscilloscope from processor before destroying
    audioProcessor.setScopeSink(nullptr);
    
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void EnvGenAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(CustomLookAndFeel::backgroundColour);

    // Header/gain section background
    auto headerBounds = juce::Rectangle<int>(10, 45, getWidth() - 20, 55);
    g.setColour(CustomLookAndFeel::panelColour);
    g.fillRoundedRectangle(headerBounds.toFloat(), 6.0f);
    g.setColour(CustomLookAndFeel::accentColour.withAlpha(0.3f));
    g.drawRoundedRectangle(headerBounds.toFloat().reduced(0.5f), 6.0f, 1.0f);
}

void EnvGenAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 10;
    const int laneHeight = 70;
    const int headerHeight = 55;
    const int waveformHeight = 200;
    const int titleHeight = 35;

    // Title
    titleLabel.setBounds(bounds.removeFromTop(titleHeight));

    // Header section (gain + dry)
    auto headerSection = bounds.removeFromTop(headerHeight + margin).reduced(margin, 0);
    headerSection.removeFromTop(5);

    // Input gain
    auto inputGainArea = headerSection.removeFromLeft(70);
    inputGainLabel.setBounds(inputGainArea.removeFromTop(16));
    inputGainSlider.setBounds(inputGainArea);
    headerSection.removeFromLeft(margin);

    // Output gain
    auto outputGainArea = headerSection.removeFromLeft(70);
    outputGainLabel.setBounds(outputGainArea.removeFromTop(16));
    outputGainSlider.setBounds(outputGainArea);
    headerSection.removeFromLeft(margin);

    // Dry pass button
    auto dryPassArea = headerSection.removeFromLeft(50);
    dryPassButton.setBounds(dryPassArea.reduced(0, 18));
    headerSection.removeFromLeft(margin);

    // Reset All button (right side of header)
    auto resetArea = headerSection.removeFromRight(80);
    resetAllButton.setBounds(resetArea.reduced(0, 18));

    bounds.removeFromTop(margin);

    // Oscilloscope display
    auto waveformArea = bounds.removeFromTop(waveformHeight).reduced(margin, 0);
    oscilloscope->setBounds(waveformArea);
    bounds.removeFromTop(margin);

    // Single envelope lane
    auto laneArea = bounds.removeFromTop(laneHeight).reduced(margin, 0);
    envelopeLane->setBounds(laneArea);
}

void EnvGenAudioProcessorEditor::timerCallback()
{
    int currentStep = audioProcessor.getCurrentStep(0);
    envelopeLane->setCurrentStep(currentStep);
}
