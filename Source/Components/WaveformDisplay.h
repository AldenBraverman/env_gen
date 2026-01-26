/*
  ==============================================================================

    WaveformDisplay.h
    Real-time waveform visualization component

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "CustomLookAndFeel.h"

// Forward declaration
class EnvGenAudioProcessor;

class WaveformDisplay : public juce::Component,
                        public juce::Timer
{
public:
    WaveformDisplay(EnvGenAudioProcessor& processor);
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

private:
    EnvGenAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};
