/*
  ==============================================================================

    Envelope.h
    AHD (Attack-Hold-Decay) Envelope Generator

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class Envelope
{
public:
    enum class Phase
    {
        Idle,
        Attack,
        Hold,
        Decay
    };

    Envelope();
    ~Envelope() = default;

    void prepare(double sampleRate);
    void reset();

    // Trigger the envelope from the beginning
    void trigger();

    // Process and return the current envelope value (0.0 to 1.0)
    float process();

    // Get current envelope value without advancing
    float getCurrentValue() const { return currentValue; }

    // Check if envelope is active
    bool isActive() const { return phase != Phase::Idle; }

    // Set envelope parameters (in seconds)
    void setAttack(float attackTimeSeconds);
    void setHold(float holdTimeSeconds);
    void setDecay(float decayTimeSeconds);

    // Get current phase
    Phase getPhase() const { return phase; }

private:
    double sampleRate = 44100.0;
    
    // Time parameters in seconds
    float attackTime = 0.01f;
    float holdTime = 0.1f;
    float decayTime = 0.5f;

    // Calculated sample counts
    int attackSamples = 0;
    int holdSamples = 0;
    int decaySamples = 0;

    // Current state
    Phase phase = Phase::Idle;
    float currentValue = 0.0f;
    int sampleCounter = 0;

    // Increment/decrement per sample
    float attackIncrement = 0.0f;
    float decayDecrement = 0.0f;

    void calculateCoefficients();
};
