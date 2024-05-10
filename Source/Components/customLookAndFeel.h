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

    //void paint (juce::Graphics&) override;
    //void resized() override;


private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (customLookAndFeel)
};
