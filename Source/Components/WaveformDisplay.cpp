/*
  ==============================================================================

    WaveformDisplay.cpp
    Real-time waveform visualization component

  ==============================================================================
*/

#include "WaveformDisplay.h"
#include "../PluginProcessor.h"

WaveformDisplay::WaveformDisplay(EnvGenAudioProcessor& processor)
    : audioProcessor(processor)
{
    // Start timer for repaints at ~30fps
    startTimerHz(30);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background panel
    g.setColour(CustomLookAndFeel::panelColour);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Draw subtle border
    g.setColour(CustomLookAndFeel::accentColour.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
    
    // Get waveform data from processor (double buffered - reads display buffer)
    const auto& waveformBuffer = audioProcessor.getWaveformDisplayBuffer();
    const auto& envelopeBuffer = audioProcessor.getEnvelopeDisplayBuffer();
    int displaySize = audioProcessor.getDisplayBufferSize();
    int samplesPerBar = audioProcessor.getWaveformSamplesPerBar();
    
    if (displaySize <= 0 || samplesPerBar <= 0)
        return;
    
    // Calculate drawing area (with padding)
    auto drawBounds = bounds.reduced(8.0f, 4.0f);
    float centerY = drawBounds.getCentreY();
    float waveformAmplitude = drawBounds.getHeight() * 0.45f;  // Leave some margin
    
    // Calculate how many samples to draw
    int samplesToShow = juce::jmin(displaySize, samplesPerBar);
    
    if (samplesToShow > 0)
    {
        // Map samples to pixel positions
        float pixelsPerSample = drawBounds.getWidth() / static_cast<float>(samplesPerBar);
        
        // Skip samples if there are too many for pixel resolution
        int step = juce::jmax(1, samplesToShow / static_cast<int>(drawBounds.getWidth()));
        
        // Draw envelope overlay first (behind waveform)
        juce::Path envelopePath;
        
        // Find max envelope value for scaling
        float maxEnvValue = 0.0f;
        for (int i = 0; i < samplesToShow; i += step)
        {
            maxEnvValue = juce::jmax(maxEnvValue, envelopeBuffer[i]);
        }
        
        // Scale envelope to fill height (if there's any envelope activity)
        float envelopeScale = (maxEnvValue > 0.01f) ? (drawBounds.getHeight() * 0.9f) / maxEnvValue : 0.0f;
        
        if (envelopeScale > 0.0f)
        {
            // Start envelope path from bottom
            float envStartY = drawBounds.getBottom() - (envelopeBuffer[0] * envelopeScale);
            envelopePath.startNewSubPath(drawBounds.getX(), envStartY);
            
            for (int i = step; i < samplesToShow; i += step)
            {
                float x = drawBounds.getX() + (i * pixelsPerSample);
                float y = drawBounds.getBottom() - (envelopeBuffer[i] * envelopeScale);
                y = juce::jlimit(drawBounds.getY(), drawBounds.getBottom(), y);
                envelopePath.lineTo(x, y);
            }
            
            // Draw envelope with distinct color (stepActiveColour - typically green/bright)
            g.setColour(CustomLookAndFeel::stepActiveColour.withAlpha(0.8f));
            g.strokePath(envelopePath, juce::PathStrokeType(2.0f));
        }
        
        // Draw audio waveform
        juce::Path waveformPath;
        
        float startY = centerY - (waveformBuffer[0] * waveformAmplitude);
        waveformPath.startNewSubPath(drawBounds.getX(), startY);
        
        for (int i = step; i < samplesToShow; i += step)
        {
            float x = drawBounds.getX() + (i * pixelsPerSample);
            float y = centerY - (waveformBuffer[i] * waveformAmplitude);
            y = juce::jlimit(drawBounds.getY(), drawBounds.getBottom(), y);
            waveformPath.lineTo(x, y);
        }
        
        // Draw the waveform
        g.setColour(CustomLookAndFeel::accentColour);
        g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
    }
    
    // Draw center line
    g.setColour(CustomLookAndFeel::textColour.withAlpha(0.2f));
    g.drawHorizontalLine(static_cast<int>(centerY), drawBounds.getX(), drawBounds.getRight());
}

void WaveformDisplay::resized()
{
    // Nothing specific to do here
}

void WaveformDisplay::timerCallback()
{
    // Trigger repaint to update waveform display
    repaint();
}
