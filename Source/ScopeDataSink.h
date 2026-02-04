/*
  ==============================================================================

    ScopeDataSink.h
    Interface for oscilloscope data (waveform + envelope). Implemented by
    OsciloscopeComponent (native UI) and ScopeBuffer (web UI).

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/** Interface for receiving scope data from the audio processor.
    The processor pushes mono audio, envelope values, and playhead info each block.
*/
class ScopeDataSink
{
public:
    virtual ~ScopeDataSink() = default;

    /** Add mono audio samples for the current measure. */
    virtual void pushBuffer(const float* samples, int numSamples) = 0;

    /** Add envelope values (0..1) aligned with the audio buffer. */
    virtual void pushEnvelopeBuffer(const float* samples, int numSamples) = 0;

    /** Update playhead (BPM, PPQ, time sig) for measure-boundary detection. */
    virtual void updatePlayheadInfo(const juce::AudioPlayHead::CurrentPositionInfo& info) = 0;
};
