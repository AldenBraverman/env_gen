/*
  ==============================================================================

    Filter.h
    State Variable Filter with LP/HP/BP modes

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Filter
{
public:
    enum class Mode
    {
        Lowpass,
        Highpass,
        Bandpass
    };

    Filter();
    ~Filter() = default;

    void prepare(double sampleRate, int samplesPerBlock, int numChannels);
    void reset();

    // Process a single sample (mono)
    float processSample(float input, int channel);

    // Process a buffer
    void processBlock(juce::AudioBuffer<float>& buffer);

    // Set filter parameters
    void setCutoff(float frequencyHz);
    void setResonance(float resonance);  // 0.0 to 1.0
    void setMode(Mode newMode);

    // Get current parameters
    float getCutoff() const { return cutoffFrequency; }
    float getResonance() const { return resonanceAmount; }
    Mode getMode() const { return filterMode; }

private:
    double sampleRate = 44100.0;
    int numChannels = 2;

    // Filter parameters
    float cutoffFrequency = 1000.0f;
    float resonanceAmount = 0.0f;
    Mode filterMode = Mode::Lowpass;

    // State variables per channel (max 2 channels for stereo)
    static constexpr int maxChannels = 2;
    float z1[maxChannels] = { 0.0f };
    float z2[maxChannels] = { 0.0f };

    // Coefficients
    float g = 0.0f;   // frequency coefficient
    float k = 0.0f;   // resonance coefficient
    float a1 = 0.0f;
    float a2 = 0.0f;
    float a3 = 0.0f;

    void calculateCoefficients();
};
