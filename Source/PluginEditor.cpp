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

    // Input gain slider
    inputGainLabel.setText("Input", juce::dontSendNotification);
    inputGainLabel.setFont(juce::Font(11.0f));
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
    outputGainLabel.setFont(juce::Font(11.0f));
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
    audioProcessor.setOscilloscope(oscilloscope.get());

    // Create envelope lanes
    for (int i = 0; i < EnvGenAudioProcessor::NUM_LANES; ++i)
    {
        lanes[i] = std::make_unique<EnvelopeLane>(audioProcessor.apvts, i + 1);
        addAndMakeVisible(*lanes[i]);
    }

    // Start timer for step visualization
    startTimerHz(30);

    // Set window size (increased height to accommodate waveform display)
    setSize(900, 520);
}

EnvGenAudioProcessorEditor::~EnvGenAudioProcessorEditor()
{
    // Unregister oscilloscope from processor before destroying
    audioProcessor.setOscilloscope(nullptr);
    
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
    const int waveformHeight = 60;
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
    filterSection.removeFromLeft(margin * 2);

    // Input gain (positioned towards the right)
    auto inputGainArea = filterSection.removeFromLeft(70);
    inputGainLabel.setBounds(inputGainArea.removeFromTop(16));
    inputGainSlider.setBounds(inputGainArea);
    filterSection.removeFromLeft(margin);

    // Output gain
    auto outputGainArea = filterSection.removeFromLeft(70);
    outputGainLabel.setBounds(outputGainArea.removeFromTop(16));
    outputGainSlider.setBounds(outputGainArea);
    filterSection.removeFromLeft(margin);

    // Dry pass button
    auto dryPassArea = filterSection.removeFromLeft(50);
    dryPassButton.setBounds(dryPassArea.reduced(0, 18));

    bounds.removeFromTop(margin);

    // Oscilloscope display (below filter section, above lanes)
    auto waveformArea = bounds.removeFromTop(waveformHeight).reduced(margin, 0);
    oscilloscope->setBounds(waveformArea);
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
