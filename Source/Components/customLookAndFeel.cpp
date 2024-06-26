/*
  ==============================================================================

    customLookAndFeel.cpp
    Created: 9 May 2024 7:24:50pm
    Author:  Alden

  ==============================================================================
*/

#include <JuceHeader.h>
#include "customLookAndFeel.h"

//==============================================================================
customLookAndFeel::customLookAndFeel()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

customLookAndFeel::~customLookAndFeel()
{
    
}

//void customLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColor, bool, bool isButtonDown)
//{
//    auto buttonArea = button.getLocalBounds();
//    g.setColour(juce::Colours::cyan);
//    g.fillRect(buttonArea);
//}

void customLookAndFeel::drawTickBox(juce::Graphics& g, juce::Component& toggleButton, float x, float y, float w, float h, bool ticked, bool isEnabled, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto buttonArea = toggleButton.getLocalBounds();
    g.setColour(juce::Colours::cyan);
    g.fillRect(buttonArea);
}

void customLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& toggleButton, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    auto buttonArea = toggleButton.getLocalBounds();
    g.setColour(juce::Colours::red);
    g.fillRect(buttonArea);
}

//void customLookAndFeel::paint (juce::Graphics& g)
//{
//    /* This demo code just fills the component's background and
//       draws some placeholder text to get you started.
//
//       You should replace everything in this method with your own
//       drawing code..
//    */
//
//    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
//
//    g.setColour (juce::Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (juce::Colours::white);
//    g.setFont (14.0f);
//    g.drawText ("customLookAndFeel", getLocalBounds(),
//                juce::Justification::centred, true);   // draw some placeholder text
//}
//
//void customLookAndFeel::resized()
//{
//    // This method is where you should set the bounds of any child
//    // components that your component contains..
//
//}
