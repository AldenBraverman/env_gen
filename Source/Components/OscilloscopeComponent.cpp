#include "OscilloscopeComponent.h"

OsciloscopeComponent::OsciloscopeComponent()
    : measureBufferWritePosition(0)
    , measureBufferCapacity(192000) // ~4 seconds at 48kHz - enough for slow tempos
    , lastPPQPosition(-1.0)
    , ppqAtMeasureStart(0.0)
    , quarterNotesPerBar(4.0)
    , lastBPM(120.0)
    , lastSampleRate(44100.0)
    , samplesInCurrentMeasure(0)
    , hasValidPlayheadInfo(false)
    , isNewMeasure(false)
    , verticalZoom(1.0f)
    , refreshRateHz(30)
    , backgroundColour(juce::Colour(0xff1a1a1a))
    , waveformColour(juce::Colour(0xff00ff00))
    , gridColour(juce::Colour(0xff333333))
    , envelopeColour(juce::Colour(0xff00ffaa))  // Cyan-green for envelope
    , showGrid(true)
    , showEnvelope(true)
    , needsDisplayUpdate(false)
{
    // Initialize measure buffers
    measureBuffer.resize(measureBufferCapacity, 0.0f);
    envelopeMeasureBuffer.resize(measureBufferCapacity, 0.0f);
    
    // Initialize playhead info
    playheadInfo.resetToDefault();
    
    // Start display timer
    startTimerHz(refreshRateHz);
}

OsciloscopeComponent::~OsciloscopeComponent()
{
    stopTimer();
}

void OsciloscopeComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    
    // Fill background
    g.fillAll(backgroundColour);
    
    // Draw border
    g.setColour(gridColour.brighter(0.3f));
    g.drawRoundedRectangle(bounds.toFloat(), 2.0f, 1.0f);
    
    auto displayArea = bounds.reduced(2);
    
    // Draw grid if enabled
    if (showGrid)
        drawGrid(g, displayArea);
    
    // Draw center line
    g.setColour(gridColour.brighter(0.5f));
    int centerY = displayArea.getCentreY();
    g.drawHorizontalLine(centerY, static_cast<float>(displayArea.getX()), static_cast<float>(displayArea.getRight()));
    
    // Draw envelope first (behind waveform)
    if (showEnvelope)
        drawEnvelope(g, displayArea);
    
    // Draw waveform
    drawWaveform(g, displayArea);
}

void OsciloscopeComponent::resized()
{
    // Component resized - trigger display update
    needsDisplayUpdate = true;
}

void OsciloscopeComponent::timerCallback()
{
    if (needsDisplayUpdate)
    {
        updateDisplayBuffer();
        needsDisplayUpdate = false;
        repaint();
    }
}

void OsciloscopeComponent::pushBuffer(const float* samples, int numSamples)
{
    const juce::ScopedLock lock(bufferLock);
    
    // Add samples to the measure buffer
    for (int i = 0; i < numSamples; ++i)
    {
        if (measureBufferWritePosition < measureBufferCapacity)
        {
            measureBuffer[measureBufferWritePosition] = samples[i];
            measureBufferWritePosition++;
        }
        else
        {
            // Buffer full - stop writing
            break;
        }
    }
    
    // Track total samples in this measure
    samplesInCurrentMeasure += numSamples;
    
    needsDisplayUpdate = true;
}

void OsciloscopeComponent::pushEnvelopeBuffer(const float* samples, int numSamples)
{
    const juce::ScopedLock lock(bufferLock);
    
    // Add envelope samples - note: we use the same write position as audio buffer
    // so they stay in sync. The audio pushBuffer increments measureBufferWritePosition,
    // so we write at the position BEFORE the current write position.
    int startPos = measureBufferWritePosition - numSamples;
    if (startPos < 0) startPos = 0;
    
    for (int i = 0; i < numSamples; ++i)
    {
        int writePos = startPos + i;
        if (writePos >= 0 && writePos < measureBufferCapacity)
        {
            envelopeMeasureBuffer[writePos] = samples[i];
        }
    }
}

void OsciloscopeComponent::updatePlayheadInfo(const juce::AudioPlayHead::CurrentPositionInfo& info)
{
    const juce::ScopedLock lock(bufferLock);
    
    hasValidPlayheadInfo = true;
    
    // Calculate quarter notes per bar based on time signature
    if (info.timeSigNumerator > 0 && info.timeSigDenominator > 0)
    {
        quarterNotesPerBar = (4.0 * info.timeSigNumerator) / info.timeSigDenominator;
    }
    
    // Track BPM and sample rate changes
    if (info.bpm > 0.0)
        lastBPM = info.bpm;
    
    // Check for discontinuities (seeking/jumping)
    bool discontinuity = false;
    if (lastPPQPosition >= 0.0 && info.ppqPosition >= 0.0)
    {
        double ppqDiff = std::abs(info.ppqPosition - lastPPQPosition);
        // If PPQ jumped more than expected (more than a typical buffer would advance)
        if (ppqDiff > 0.5) // Arbitrary threshold - half a beat
        {
            discontinuity = true;
        }
    }
    
    // Check if we've crossed a measure boundary
    if (lastPPQPosition >= 0.0 && info.ppqPosition >= 0.0 && !discontinuity)
    {
        double lastPPQInMeasure = getPPQWithinMeasure(lastPPQPosition);
        double currentPPQInMeasure = getPPQWithinMeasure(info.ppqPosition);
        
        // Detect measure wrap (current position is less than previous position within measure)
        if (currentPPQInMeasure < lastPPQInMeasure)
        {
            // New measure started - reset the buffer
            resetMeasureBuffer();
            ppqAtMeasureStart = info.ppqPosition;
            isNewMeasure = true;
        }
    }
    else if (lastPPQPosition < 0.0 || discontinuity)
    {
        // First time or discontinuity - reset and re-initialize
        resetMeasureBuffer();
        ppqAtMeasureStart = info.ppqPosition;
    }
    
    playheadInfo = info;
    lastPPQPosition = info.ppqPosition;
}

void OsciloscopeComponent::setVerticalZoom(float zoom)
{
    verticalZoom = juce::jlimit(0.1f, 10.0f, zoom);
    repaint();
}

void OsciloscopeComponent::resetMeasureBuffer()
{
    // Clear the measure buffers
    std::fill(measureBuffer.begin(), measureBuffer.end(), 0.0f);
    std::fill(envelopeMeasureBuffer.begin(), envelopeMeasureBuffer.end(), 0.0f);
    measureBufferWritePosition = 0;
    samplesInCurrentMeasure = 0;
}

void OsciloscopeComponent::updateDisplayBuffer()
{
    const juce::ScopedLock lock(bufferLock);
    
    int width = getWidth();
    if (width <= 0)
        return;
    
    // Resize display buffers to match component width
    if (displayBuffer.size() != static_cast<size_t>(width))
        displayBuffer.resize(width, 0.0f);
    if (envelopeDisplayBuffer.size() != static_cast<size_t>(width))
        envelopeDisplayBuffer.resize(width, 0.0f);
    
    if (measureBufferWritePosition == 0)
    {
        // No samples yet - clear display
        std::fill(displayBuffer.begin(), displayBuffer.end(), 0.0f);
        std::fill(envelopeDisplayBuffer.begin(), envelopeDisplayBuffer.end(), 0.0f);
        return;
    }
    
    // Calculate expected total samples in a measure using BPM and sample rate
    int expectedTotalSamples = measureBufferCapacity;
    if (lastBPM > 0.0 && lastSampleRate > 0.0 && quarterNotesPerBar > 0.0)
    {
        // Calculate samples per quarter note
        double secondsPerQuarterNote = 60.0 / lastBPM;
        double samplesPerQuarterNote = secondsPerQuarterNote * lastSampleRate;
        
        // Total samples in a measure
        expectedTotalSamples = static_cast<int>(samplesPerQuarterNote * quarterNotesPerBar);
        expectedTotalSamples = juce::jlimit(1, measureBufferCapacity, expectedTotalSamples);
    }
    
    // Map samples to pixels: waveform = one sample per pixel; envelope = peak-hold per pixel column
    const float widthF = static_cast<float>(width);
    for (int x = 0; x < width; ++x)
    {
        // Waveform: single sample per pixel (unchanged)
        float sampleIndex = (x / widthF) * expectedTotalSamples;
        int index = static_cast<int>(sampleIndex);
        if (index < measureBufferWritePosition)
            displayBuffer[x] = measureBuffer[index];
        else
            displayBuffer[x] = 0.0f;

        // Envelope: max over the sample range that maps to this pixel column (peak-hold)
        int startIdx = static_cast<int>((x / widthF) * expectedTotalSamples);
        int endIdx = static_cast<int>(((x + 1) / widthF) * expectedTotalSamples);
        if (endIdx <= startIdx)
            endIdx = startIdx + 1;
        startIdx = juce::jlimit(0, measureBufferWritePosition, startIdx);
        endIdx = juce::jlimit(0, measureBufferWritePosition, endIdx);
        float peak = 0.0f;
        for (int k = startIdx; k < endIdx; ++k)
            peak = juce::jmax(peak, envelopeMeasureBuffer[static_cast<size_t>(k)]);
        envelopeDisplayBuffer[static_cast<size_t>(x)] = peak;
    }
}

void OsciloscopeComponent::drawGrid(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour(gridColour);
    
    // Calculate number of 8th notes in the measure
    int eighthNotesPerMeasure = 0;
    if (quarterNotesPerBar > 0.0)
    {
        eighthNotesPerMeasure = static_cast<int>(quarterNotesPerBar * 2); // 2 eighth notes per quarter note
    }
    else
    {
        eighthNotesPerMeasure = 8; // Default to 8 eighth notes (4/4 time)
    }
    
    // Draw vertical lines for each 8th note subdivision
    for (int i = 0; i <= eighthNotesPerMeasure; ++i)
    {
        float x = bounds.getX() + (bounds.getWidth() * i / static_cast<float>(eighthNotesPerMeasure));
        
        // Make quarter note lines brighter/thicker
        bool isQuarterNote = (i % 2 == 0);
        bool isMeasureStart = (i == 0);
        bool isMeasureEnd = (i == eighthNotesPerMeasure);
        
        if (isMeasureStart || isMeasureEnd)
        {
            // Measure boundaries - brightest and thickest
            g.setColour(gridColour.brighter(0.8f));
            g.drawLine(x, static_cast<float>(bounds.getY()),
                      x, static_cast<float>(bounds.getBottom()), 2.0f);
        }
        else if (isQuarterNote)
        {
            // Quarter note subdivisions - medium brightness
            g.setColour(gridColour.brighter(0.5f));
            g.drawLine(x, static_cast<float>(bounds.getY()),
                      x, static_cast<float>(bounds.getBottom()), 1.5f);
        }
        else
        {
            // 8th note subdivisions - dimmer
            g.setColour(gridColour.brighter(0.2f));
            g.drawLine(x, static_cast<float>(bounds.getY()),
                      x, static_cast<float>(bounds.getBottom()), 1.0f);
        }
    }
    
    // Horizontal grid lines - fewer lines for cleaner look
    g.setColour(gridColour);
    int numHorizontalLines = 4; // Top, middle-top, center, middle-bottom, bottom
    for (int i = 0; i <= numHorizontalLines; ++i)
    {
        float y = bounds.getY() + (bounds.getHeight() * i / static_cast<float>(numHorizontalLines));
        
        // Make center line slightly brighter
        if (i == numHorizontalLines / 2)
            g.setColour(gridColour.brighter(0.3f));
        else
            g.setColour(gridColour);
            
        g.drawHorizontalLine(static_cast<int>(y),
                           static_cast<float>(bounds.getX()),
                           static_cast<float>(bounds.getRight()));
    }
}

void OsciloscopeComponent::drawWaveform(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (displayBuffer.empty())
        return;
    
    g.setColour(waveformColour);
    
    juce::Path wavePath;
    bool pathStarted = false;
    
    int width = bounds.getWidth();
    int height = bounds.getHeight();
    int centerY = bounds.getCentreY();
    
    // Draw the waveform - one pixel per display buffer sample
    int samplesToDraw = juce::jmin(width, static_cast<int>(displayBuffer.size()));
    
    for (int x = 0; x < samplesToDraw; ++x)
    {
        float sample = displayBuffer[x];
        float y = getScaledSample(sample, height);
        float pixelY = centerY - y;
        
        // Clamp to bounds
        pixelY = juce::jlimit(static_cast<float>(bounds.getY()),
                            static_cast<float>(bounds.getBottom()),
                            pixelY);
        
        if (!pathStarted)
        {
            wavePath.startNewSubPath(static_cast<float>(bounds.getX() + x), pixelY);
            pathStarted = true;
        }
        else
        {
            wavePath.lineTo(static_cast<float>(bounds.getX() + x), pixelY);
        }
    }
    
    // Draw the path
    g.strokePath(wavePath, juce::PathStrokeType(1.5f));
}

void OsciloscopeComponent::drawEnvelope(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    if (envelopeDisplayBuffer.empty())
        return;
    
    int width = bounds.getWidth();
    int height = bounds.getHeight();
    
    // Find max envelope value for scaling
    float maxEnvValue = 0.0f;
    int samplesToDraw = juce::jmin(width, static_cast<int>(envelopeDisplayBuffer.size()));
    
    for (int x = 0; x < samplesToDraw; ++x)
    {
        maxEnvValue = juce::jmax(maxEnvValue, envelopeDisplayBuffer[x]);
    }
    
    // Scale envelope to fill height (if there's any envelope activity)
    if (maxEnvValue < 0.01f)
        return;  // No significant envelope activity
    
    float envelopeScale = (height * 0.9f) / maxEnvValue;
    
    g.setColour(envelopeColour.withAlpha(0.8f));
    
    juce::Path envPath;
    bool pathStarted = false;
    
    for (int x = 0; x < samplesToDraw; ++x)
    {
        float envValue = envelopeDisplayBuffer[x];
        float pixelY = bounds.getBottom() - (envValue * envelopeScale);
        
        // Clamp to bounds
        pixelY = juce::jlimit(static_cast<float>(bounds.getY()),
                            static_cast<float>(bounds.getBottom()),
                            pixelY);
        
        if (!pathStarted)
        {
            envPath.startNewSubPath(static_cast<float>(bounds.getX() + x), pixelY);
            pathStarted = true;
        }
        else
        {
            envPath.lineTo(static_cast<float>(bounds.getX() + x), pixelY);
        }
    }
    
    // Draw the envelope path
    g.strokePath(envPath, juce::PathStrokeType(2.0f));
}

float OsciloscopeComponent::getScaledSample(float sample, int height) const
{
    return sample * verticalZoom * height * 0.4f;
}

double OsciloscopeComponent::getPPQWithinMeasure(double ppq) const
{
    if (quarterNotesPerBar <= 0.0)
        return 0.0;
    
    return fmod(ppq, quarterNotesPerBar);
}
