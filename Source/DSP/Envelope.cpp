/*
  ==============================================================================

    Envelope.cpp
    AHD (Attack-Hold-Decay) Envelope Generator

  ==============================================================================
*/

#include "Envelope.h"

Envelope::Envelope()
{
    calculateCoefficients();
}

void Envelope::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    calculateCoefficients();
    reset();
}

void Envelope::reset()
{
    phase = Phase::Idle;
    currentValue = 0.0f;
    sampleCounter = 0;
}

void Envelope::trigger()
{
    phase = Phase::Attack;
    sampleCounter = 0;
    // Don't reset currentValue - allows retriggering from current position
}

float Envelope::process()
{
    switch (phase)
    {
        case Phase::Idle:
            currentValue = 0.0f;
            break;

        case Phase::Attack:
            currentValue += attackIncrement;
            sampleCounter++;
            
            if (currentValue >= 1.0f || sampleCounter >= attackSamples)
            {
                currentValue = 1.0f;
                phase = Phase::Hold;
                sampleCounter = 0;
            }
            break;

        case Phase::Hold:
            currentValue = 1.0f;
            sampleCounter++;
            
            if (sampleCounter >= holdSamples)
            {
                phase = Phase::Decay;
                sampleCounter = 0;
            }
            break;

        case Phase::Decay:
            currentValue -= decayDecrement;
            sampleCounter++;
            
            if (currentValue <= 0.0f || sampleCounter >= decaySamples)
            {
                currentValue = 0.0f;
                phase = Phase::Idle;
                sampleCounter = 0;
            }
            break;
    }

    return currentValue;
}

void Envelope::setAttack(float attackTimeSeconds)
{
    attackTime = juce::jmax(0.001f, attackTimeSeconds);
    calculateCoefficients();
}

void Envelope::setHold(float holdTimeSeconds)
{
    holdTime = juce::jmax(0.0f, holdTimeSeconds);
    calculateCoefficients();
}

void Envelope::setDecay(float decayTimeSeconds)
{
    decayTime = juce::jmax(0.001f, decayTimeSeconds);
    calculateCoefficients();
}

void Envelope::calculateCoefficients()
{
    // Calculate sample counts
    attackSamples = static_cast<int>(attackTime * sampleRate);
    holdSamples = static_cast<int>(holdTime * sampleRate);
    decaySamples = static_cast<int>(decayTime * sampleRate);

    // Ensure minimum of 1 sample for attack and decay
    attackSamples = juce::jmax(1, attackSamples);
    decaySamples = juce::jmax(1, decaySamples);

    // Calculate linear increments
    attackIncrement = 1.0f / static_cast<float>(attackSamples);
    decayDecrement = 1.0f / static_cast<float>(decaySamples);
}
