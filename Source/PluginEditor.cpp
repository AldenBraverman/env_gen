/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Env_genAudioProcessorEditor::Env_genAudioProcessorEditor (Env_genAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    // lane_one_0b.setButtonText("test");
    // lane_one_0b.setLookAndFeel(&otherLookAndFeel);
    addAndMakeVisible(lane_one_0b);
    addAndMakeVisible(lane_one_1b);
    lane_one_1b.setColour(juce::TextButton::buttonColourId, juce::Colour::fromRGB(65, 61, 135));
    lane_one_1b.setColour(juce::TextButton::buttonOnColourId, juce::Colour::fromRGB(0, 61, 135));
    lane_one_1b.setClickingTogglesState(true);
    lane_one_1b.isToggleable();


    setSize (400, 300);
}

Env_genAudioProcessorEditor::~Env_genAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void Env_genAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void Env_genAudioProcessorEditor::resized()
{
    lane_one_0b.setBounds(10, 10, 50, 50);
    lane_one_1b.setBounds(70, 10, 50, 50);
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
