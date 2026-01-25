/*
  ==============================================================================

    CustomLookAndFeel.h
    Custom UI styling for the Envelope Generator plugin

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    // Custom toggle button for step sequencer
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    // Custom rotary slider
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    // Custom combo box
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    // Colors
    static const juce::Colour backgroundColour;
    static const juce::Colour panelColour;
    static const juce::Colour accentColour;
    static const juce::Colour stepActiveColour;
    static const juce::Colour stepInactiveColour;
    static const juce::Colour stepPlayingColour;
    static const juce::Colour textColour;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};
