/*
  ==============================================================================

    StepButton.h
    Custom step sequencer button component

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

class StepButton : public juce::ToggleButton
{
public:
    StepButton();
    ~StepButton() override = default;

    void setPlaying(bool isPlaying);
    bool isPlaying() const { return playing; }

    void paint(juce::Graphics& g) override;

private:
    bool playing = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StepButton)
};
