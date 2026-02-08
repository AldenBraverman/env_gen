#pragma once

#include <JuceHeader.h>
#include "../ScopeDataSink.h"
#include <functional>

class OsciloscopeComponent : public juce::Component,
                              public juce::Timer,
                              public ScopeDataSink
{
public:
    OsciloscopeComponent();
    ~OsciloscopeComponent() override;

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Timer callback for display updates
    void timerCallback() override;
    
    // ScopeDataSink interface
    void pushBuffer(const float* samples, int numSamples) override;
    void pushEnvelopeBuffer(const float* samples, int numSamples) override;
    void updatePlayheadInfo(const juce::AudioPlayHead::CurrentPositionInfo& info) override;
    
    // Configuration
    void setRefreshRate(int hz) { startTimerHz(hz); }
    void setVerticalZoom(float zoom);
    
    // Visual settings
    void setBackgroundColour(juce::Colour colour) { backgroundColour = colour; }
    void setWaveformColour(juce::Colour colour) { waveformColour = colour; }
    void setGridColour(juce::Colour colour) { gridColour = colour; }
    void setEnvelopeColour(juce::Colour colour) { envelopeColour = colour; }
    void setShowGrid(bool show) { showGrid = show; }
    void setShowEnvelope(bool show) { showEnvelope = show; }

    /** When set, the scope does not draw the envelope in paint(); instead it calls this
        after each display update with the current envelope display buffer (0..1 per pixel column).
        Used for a WebView overlay that draws the envelope with smooth rendering. */
    using EnvelopeOverlayCallback = std::function<void(const float* data, int size)>;
    void setEnvelopeOverlayCallback(EnvelopeOverlayCallback cb) { envelopeOverlayCallback = std::move(cb); }

private:
    // Audio data storage - stores samples for one full measure
    std::vector<float> measureBuffer;
    std::vector<float> envelopeMeasureBuffer;  // Envelope values for one measure
    int measureBufferWritePosition;
    int measureBufferCapacity;
    
    // Playhead tracking
    juce::AudioPlayHead::CurrentPositionInfo playheadInfo;
    double lastPPQPosition;
    double ppqAtMeasureStart;
    double quarterNotesPerBar;
    double lastBPM;
    double lastSampleRate;
    int samplesInCurrentMeasure;
    bool hasValidPlayheadInfo;
    bool isNewMeasure;
    
    // Display settings
    float verticalZoom;
    int refreshRateHz;
    
    // Visual settings
    juce::Colour backgroundColour;
    juce::Colour waveformColour;
    juce::Colour gridColour;
    juce::Colour envelopeColour;
    bool showGrid;
    bool showEnvelope;
    
    // Thread safety
    juce::CriticalSection bufferLock;
    
    // Display buffer for rendering
    std::vector<float> displayBuffer;
    std::vector<float> envelopeDisplayBuffer;
    bool needsDisplayUpdate;

    EnvelopeOverlayCallback envelopeOverlayCallback;
    
    // Helper methods
    void updateDisplayBuffer();
    void drawGrid(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds);
    void drawEnvelope(juce::Graphics& g, juce::Rectangle<int> bounds);
    float getScaledSample(float sample, int height) const;
    double getPPQWithinMeasure(double ppq) const;
    void resetMeasureBuffer();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OsciloscopeComponent)
};
