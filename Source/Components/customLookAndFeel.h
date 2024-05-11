/*
  ==============================================================================

    customLookAndFeel.h
    Created: 9 May 2024 7:24:50pm
    Author:  Alden

    https://docs.juce.com/master/tutorial_look_and_feel_customisation.html
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class customLookAndFeel  : public juce::LookAndFeel_V4
{
public:
    customLookAndFeel();
    ~customLookAndFeel() override;

    // void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColor, bool isMouseOverButton, bool isButtonDown) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& toggleButton, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    void drawTickBox(juce::Graphics& g, juce::Component& toggleButton, float x, float y, float w, float h, bool ticked, bool isEnabled, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    // g = graphics conext
    // button = Button Object
    // backgroundColour = base background color that should be used
    // is MouseOverButton = where mouse pointer is within the bounds of the button
    // isButtonDown = Whether the mouse button is down

    //void paint (juce::Graphics&) override;
    //void resized() override;


private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (customLookAndFeel)
};
