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
    for (auto& buf : envelopeMeasureBuffers)
        buf.resize(static_cast<size_t>(measureBufferCapacity), 0.0f);
    
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
    
    // Draw envelope(s) first (behind waveform) unless an overlay callback is set (overlay draws it)
    if (showEnvelope && !envelopeOverlayCallback)
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
        if (envelopeOverlayCallback)
        {
            std::array<std::vector<float>, kMaxEnvelopeLanes> copies;
            std::array<const float*, kMaxEnvelopeLanes> ptrs;
            std::array<int, kMaxEnvelopeLanes> sizes;
            juce::Colour colours[kMaxEnvelopeLanes];
            int numLanes = 0;
            {
                const juce::ScopedLock lock(bufferLock);
                for (int i = 0; i < kMaxEnvelopeLanes; ++i)
                {
                    if (!envelopeDisplayBuffers[static_cast<size_t>(i)].empty())
                    {
                        copies[static_cast<size_t>(i)] = envelopeDisplayBuffers[static_cast<size_t>(i)];
                        ptrs[static_cast<size_t>(i)] = copies[static_cast<size_t>(i)].data();
                        sizes[static_cast<size_t>(i)] = static_cast<int>(copies[static_cast<size_t>(i)].size());
                        colours[i] = getLaneColour(i);
                        numLanes = i + 1;
                    }
                    else
                    {
                        ptrs[static_cast<size_t>(i)] = nullptr;
                        sizes[static_cast<size_t>(i)] = 0;
                        colours[i] = getLaneColour(i);
                    }
                }
            }
            if (numLanes > 0)
                envelopeOverlayCallback(ptrs.data(), sizes.data(), numLanes, colours);
        }
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
    pushEnvelopeBuffer(samples, numSamples, 0);
}

void OsciloscopeComponent::pushEnvelopeBuffer(const float* samples, int numSamples, int laneIndex)
{
    if (laneIndex < 0 || laneIndex >= kMaxEnvelopeLanes)
        return;
    const juce::ScopedLock lock(bufferLock);
    auto& buf = envelopeMeasureBuffers[static_cast<size_t>(laneIndex)];
    if (buf.size() != static_cast<size_t>(measureBufferCapacity))
        buf.resize(static_cast<size_t>(measureBufferCapacity), 0.0f);
    int startPos = measureBufferWritePosition - numSamples;
    if (startPos < 0) startPos = 0;
    for (int i = 0; i < numSamples; ++i)
    {
        int writePos = startPos + i;
        if (writePos >= 0 && writePos < measureBufferCapacity)
            buf[static_cast<size_t>(writePos)] = samples[i];
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
    std::fill(measureBuffer.begin(), measureBuffer.end(), 0.0f);
    for (auto& buf : envelopeMeasureBuffers)
        std::fill(buf.begin(), buf.end(), 0.0f);
    measureBufferWritePosition = 0;
    samplesInCurrentMeasure = 0;
}

void OsciloscopeComponent::updateDisplayBuffer()
{
    const juce::ScopedLock lock(bufferLock);
    
    int width = getWidth();
    if (width <= 0)
        return;
    
    if (displayBuffer.size() != static_cast<size_t>(width))
        displayBuffer.resize(static_cast<size_t>(width), 0.0f);
    for (auto& buf : envelopeDisplayBuffers)
    {
        if (buf.size() != static_cast<size_t>(width))
            buf.resize(static_cast<size_t>(width), 0.0f);
    }
    
    if (measureBufferWritePosition == 0)
    {
        std::fill(displayBuffer.begin(), displayBuffer.end(), 0.0f);
        for (auto& buf : envelopeDisplayBuffers)
            std::fill(buf.begin(), buf.end(), 0.0f);
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

        // Envelope: per-lane peak-hold
        int startIdx = static_cast<int>((x / widthF) * expectedTotalSamples);
        int endIdx = static_cast<int>(((x + 1) / widthF) * expectedTotalSamples);
        if (endIdx <= startIdx)
            endIdx = startIdx + 1;
        startIdx = juce::jlimit(0, measureBufferWritePosition, startIdx);
        endIdx = juce::jlimit(0, measureBufferWritePosition, endIdx);
        for (int lane = 0; lane < kMaxEnvelopeLanes; ++lane)
        {
            const auto& measBuf = envelopeMeasureBuffers[static_cast<size_t>(lane)];
            if (measBuf.empty())
                continue;
            float peak = 0.0f;
            for (int k = startIdx; k < endIdx; ++k)
                peak = juce::jmax(peak, measBuf[static_cast<size_t>(k)]);
            envelopeDisplayBuffers[static_cast<size_t>(lane)][static_cast<size_t>(x)] = peak;
        }
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

juce::Colour OsciloscopeComponent::getLaneColour(int laneIndex)
{
    static const juce::Colour palette[] = {
        juce::Colour(0xff00ffaa),  // 0: cyan-green
        juce::Colour(0xffff8c00),  // 1: orange
        juce::Colour(0xff4da6ff),  // 2: blue
        juce::Colour(0xffe040fb),  // 3: magenta
        juce::Colour(0xffffeb3b),  // 4: yellow
        juce::Colour(0xff26a69a),  // 5: teal
        juce::Colour(0xffff7043),  // 6: coral
        juce::Colour(0xffb39ddb),  // 7: lavender
    };
    int i = juce::jlimit(0, kMaxEnvelopeLanes - 1, laneIndex);
    return palette[i];
}

void OsciloscopeComponent::drawEnvelope(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    int width = bounds.getWidth();
    int height = bounds.getHeight();
    if (width <= 0 || height <= 0)
        return;

    float maxEnvValue = 0.0f;
    const float one16 = 1.0f / 16.0f;

    for (int lane = 0; lane < kMaxEnvelopeLanes; ++lane)
    {
        const auto& dispBuf = envelopeDisplayBuffers[static_cast<size_t>(lane)];
        int samplesToDraw = juce::jmin(width, static_cast<int>(dispBuf.size()));
        if (samplesToDraw <= 0)
            continue;
        for (int x = 0; x < samplesToDraw; ++x)
        {
            float v = dispBuf[static_cast<size_t>(x)];
            if (x >= 2 && x < samplesToDraw - 2)
                v = (dispBuf[static_cast<size_t>(x - 2)] + 4.0f * dispBuf[static_cast<size_t>(x - 1)]
                     + 6.0f * v + 4.0f * dispBuf[static_cast<size_t>(x + 1)] + dispBuf[static_cast<size_t>(x + 2)]) * one16;
            else if (x > 0 && x < samplesToDraw - 1)
                v = (dispBuf[static_cast<size_t>(x - 1)] + 2.0f * v + dispBuf[static_cast<size_t>(x + 1)]) * 0.25f;
            maxEnvValue = juce::jmax(maxEnvValue, v);
        }
    }
    if (maxEnvValue < 0.01f)
        return;
    float envelopeScale = (height * 0.9f) / maxEnvValue;

    for (int lane = 0; lane < kMaxEnvelopeLanes; ++lane)
    {
        const auto& dispBuf = envelopeDisplayBuffers[static_cast<size_t>(lane)];
        int samplesToDraw = juce::jmin(width, static_cast<int>(dispBuf.size()));
        if (samplesToDraw <= 0)
            continue;
        std::vector<float> smoothed(static_cast<size_t>(samplesToDraw));
        for (int x = 0; x < samplesToDraw; ++x)
        {
            float v = dispBuf[static_cast<size_t>(x)];
            if (x >= 2 && x < samplesToDraw - 2)
                v = (dispBuf[static_cast<size_t>(x - 2)] + 4.0f * dispBuf[static_cast<size_t>(x - 1)]
                     + 6.0f * v + 4.0f * dispBuf[static_cast<size_t>(x + 1)] + dispBuf[static_cast<size_t>(x + 2)]) * one16;
            else if (x > 0 && x < samplesToDraw - 1)
                v = (dispBuf[static_cast<size_t>(x - 1)] + 2.0f * v + dispBuf[static_cast<size_t>(x + 1)]) * 0.25f;
            smoothed[static_cast<size_t>(x)] = v;
        }
        g.setColour(getLaneColour(lane).withAlpha(0.8f));
        juce::Path envPath;
        bool pathStarted = false;
        for (int x = 0; x < samplesToDraw; ++x)
        {
            float pixelY = bounds.getBottom() - (smoothed[static_cast<size_t>(x)] * envelopeScale);
            pixelY = juce::jlimit(static_cast<float>(bounds.getY()), static_cast<float>(bounds.getBottom()), pixelY);
            if (!pathStarted)
            {
                envPath.startNewSubPath(static_cast<float>(bounds.getX() + x), pixelY);
                pathStarted = true;
            }
            else
                envPath.lineTo(static_cast<float>(bounds.getX() + x), pixelY);
        }
        g.strokePath(envPath, juce::PathStrokeType(2.0f));
    }
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
