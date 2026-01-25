/*
  ==============================================================================

    StepButton.cpp
    Custom step sequencer button component

  ==============================================================================
*/

#include "StepButton.h"

StepButton::StepButton()
{
    setClickingTogglesState(true);
}

void StepButton::setPlaying(bool isPlaying)
{
    if (playing != isPlaying)
    {
        playing = isPlaying;
        repaint();
    }
}

void StepButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    
    // Determine color based on state
    juce::Colour fillColour;
    
    if (playing)
    {
        // Currently playing step - brightest
        fillColour = CustomLookAndFeel::stepPlayingColour;
    }
    else if (getToggleState())
    {
        // Active but not playing
        fillColour = CustomLookAndFeel::stepActiveColour;
    }
    else
    {
        // Inactive
        fillColour = CustomLookAndFeel::stepInactiveColour;
    }
    
    // Highlight on hover
    if (isMouseOver())
    {
        fillColour = fillColour.brighter(0.15f);
    }
    
    // Draw rounded rectangle
    g.setColour(fillColour);
    g.fillRoundedRectangle(bounds, 3.0f);
    
    // Draw border
    g.setColour(CustomLookAndFeel::accentColour.withAlpha(0.5f));
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    
    // If playing, add glow effect
    if (playing)
    {
        g.setColour(CustomLookAndFeel::stepPlayingColour.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds.expanded(2.0f), 4.0f, 2.0f);
    }
}
