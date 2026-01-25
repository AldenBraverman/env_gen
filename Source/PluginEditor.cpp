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
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(titleLabel);

    // Filter section label
    filterLabel.setText("FILTER", juce::dontSendNotification);
    filterLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    filterLabel.setJustificationType(juce::Justification::centred);
    filterLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(filterLabel);

    // Filter mode combo
    filterModeLabel.setText("Mode", juce::dontSendNotification);
    filterModeLabel.setFont(juce::Font(11.0f));
    filterModeLabel.setJustificationType(juce::Justification::centred);
    filterModeLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(filterModeLabel);

    filterModeCombo.addItemList({ "Lowpass", "Highpass", "Bandpass" }, 1);
    addAndMakeVisible(filterModeCombo);
    filterModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "filterMode", filterModeCombo);

    // Filter cutoff slider
    filterCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    filterCutoffLabel.setFont(juce::Font(11.0f));
    filterCutoffLabel.setJustificationType(juce::Justification::centred);
    filterCutoffLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(filterCutoffLabel);

    filterCutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filterCutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    filterCutoffSlider.setTextValueSuffix(" Hz");
    addAndMakeVisible(filterCutoffSlider);
    filterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filterCutoff", filterCutoffSlider);

    // Filter resonance slider
    filterResonanceLabel.setText("Resonance", juce::dontSendNotification);
    filterResonanceLabel.setFont(juce::Font(11.0f));
    filterResonanceLabel.setJustificationType(juce::Justification::centred);
    filterResonanceLabel.setColour(juce::Label::textColourId, CustomLookAndFeel::textColour);
    addAndMakeVisible(filterResonanceLabel);

    filterResonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    filterResonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    addAndMakeVisible(filterResonanceSlider);
    filterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "filterResonance", filterResonanceSlider);

    // Create envelope lanes
    for (int i = 0; i < EnvGenAudioProcessor::NUM_LANES; ++i)
    {
        lanes[i] = std::make_unique<EnvelopeLane>(audioProcessor.apvts, i + 1);
        addAndMakeVisible(*lanes[i]);
    }

    // Start timer for step visualization
    startTimerHz(30);

    // Set window size
    setSize(900, 450);
}

EnvGenAudioProcessorEditor::~EnvGenAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void EnvGenAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(CustomLookAndFeel::backgroundColour);

    // Filter section background
    auto filterBounds = juce::Rectangle<int>(10, 45, getWidth() - 20, 85);
    g.setColour(CustomLookAndFeel::panelColour);
    g.fillRoundedRectangle(filterBounds.toFloat(), 6.0f);
    g.setColour(CustomLookAndFeel::accentColour.withAlpha(0.3f));
    g.drawRoundedRectangle(filterBounds.toFloat().reduced(0.5f), 6.0f, 1.0f);
}

void EnvGenAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    const int margin = 10;
    const int laneHeight = 70;
    const int filterHeight = 85;
    const int titleHeight = 35;

    // Title
    titleLabel.setBounds(bounds.removeFromTop(titleHeight));

    // Filter section
    auto filterSection = bounds.removeFromTop(filterHeight + margin).reduced(margin, 0);
    filterSection.removeFromTop(5); // Padding

    // Filter label
    filterLabel.setBounds(filterSection.removeFromLeft(60).reduced(0, 20));
    filterSection.removeFromLeft(margin);

    // Filter mode
    auto modeArea = filterSection.removeFromLeft(90);
    filterModeLabel.setBounds(modeArea.removeFromTop(16));
    filterModeCombo.setBounds(modeArea.removeFromTop(24));
    filterSection.removeFromLeft(margin);

    // Filter cutoff
    auto cutoffArea = filterSection.removeFromLeft(80);
    filterCutoffLabel.setBounds(cutoffArea.removeFromTop(16));
    filterCutoffSlider.setBounds(cutoffArea);
    filterSection.removeFromLeft(margin);

    // Filter resonance
    auto resonanceArea = filterSection.removeFromLeft(80);
    filterResonanceLabel.setBounds(resonanceArea.removeFromTop(16));
    filterResonanceSlider.setBounds(resonanceArea);

    bounds.removeFromTop(margin);

    // Envelope lanes
    for (int i = 0; i < EnvGenAudioProcessor::NUM_LANES; ++i)
    {
        auto laneArea = bounds.removeFromTop(laneHeight).reduced(margin, 0);
        lanes[i]->setBounds(laneArea);
        bounds.removeFromTop(5); // Spacing between lanes
    }
}

void EnvGenAudioProcessorEditor::timerCallback()
{
    // Update step visualization for each lane
    for (int i = 0; i < EnvGenAudioProcessor::NUM_LANES; ++i)
    {
        int currentStep = audioProcessor.getCurrentStep(i);
        lanes[i]->setCurrentStep(currentStep);
    }
}
