/*
  ==============================================================================

    Filter.cpp
    State Variable Filter with LP/HP/BP modes

  ==============================================================================
*/

#include "Filter.h"

Filter::Filter()
{
    calculateCoefficients();
}

void Filter::prepare(double newSampleRate, int /*samplesPerBlock*/, int channels)
{
    sampleRate = newSampleRate;
    numChannels = juce::jmin(channels, maxChannels);
    calculateCoefficients();
    reset();
}

void Filter::reset()
{
    for (int i = 0; i < maxChannels; ++i)
    {
        z1[i] = 0.0f;
        z2[i] = 0.0f;
    }
}

float Filter::processSample(float input, int channel)
{
    if (channel >= maxChannels)
        return input;

    // State Variable Filter (SVF) topology
    // Based on the TPT (Topology Preserving Transform) structure
    
    float v3 = input - z2[channel];
    float v1 = a1 * z1[channel] + a2 * v3;
    float v2 = z2[channel] + a2 * z1[channel] + a3 * v3;

    z1[channel] = 2.0f * v1 - z1[channel];
    z2[channel] = 2.0f * v2 - z2[channel];

    float lowpass = v2;
    float bandpass = v1;
    float highpass = input - k * v1 - v2;

    switch (filterMode)
    {
        case Mode::Lowpass:
            return lowpass;
        case Mode::Highpass:
            return highpass;
        case Mode::Bandpass:
            return bandpass;
        default:
            return lowpass;
    }
}

void Filter::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = juce::jmin(buffer.getNumChannels(), numChannels);

    for (int channel = 0; channel < channels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
        {
            channelData[sample] = processSample(channelData[sample], channel);
        }
    }
}

void Filter::setCutoff(float frequencyHz)
{
    cutoffFrequency = juce::jlimit(20.0f, 20000.0f, frequencyHz);
    calculateCoefficients();
}

void Filter::setResonance(float resonance)
{
    resonanceAmount = juce::jlimit(0.0f, 1.0f, resonance);
    calculateCoefficients();
}

void Filter::setMode(Mode newMode)
{
    filterMode = newMode;
}

void Filter::calculateCoefficients()
{
    // Prewarp the cutoff frequency
    g = std::tan(juce::MathConstants<float>::pi * cutoffFrequency / static_cast<float>(sampleRate));
    
    // Convert resonance (0-1) to Q factor
    // Q ranges from 0.5 (no resonance) to ~20 (high resonance)
    float Q = 0.5f + resonanceAmount * 19.5f;
    k = 1.0f / Q;

    // Calculate coefficients for SVF
    a1 = 1.0f / (1.0f + g * (g + k));
    a2 = g * a1;
    a3 = g * a2;
}
