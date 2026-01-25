/*
  ==============================================================================

    StepSequencer.cpp
    16-step gate sequencer with DAW tempo sync

  ==============================================================================
*/

#include "StepSequencer.h"

StepSequencer::StepSequencer()
{
    // Initialize all steps to off
    for (int i = 0; i < NUM_STEPS; ++i)
        steps[i] = false;
}

void StepSequencer::prepare(double newSampleRate)
{
    sampleRate = newSampleRate;
    reset();
}

void StepSequencer::reset()
{
    currentStep = 0;
    lastPpqPosition = -1.0;
}

bool StepSequencer::process(const juce::AudioPlayHead::PositionInfo& positionInfo)
{
    // Check if we have valid position info and transport is playing
    if (!positionInfo.getIsPlaying())
    {
        lastPpqPosition = -1.0;
        return false;
    }

    auto ppqPositionOpt = positionInfo.getPpqPosition();
    if (!ppqPositionOpt.hasValue())
        return false;

    double ppqPosition = *ppqPositionOpt;
    double beatsPerStep = getBeatsPerStep();

    // Calculate which step we're on based on PPQ position
    // Wrap around for the 16 steps
    double totalStepPosition = ppqPosition / beatsPerStep;
    int newStep = static_cast<int>(std::fmod(totalStepPosition, static_cast<double>(NUM_STEPS)));
    
    // Handle negative PPQ (before song start)
    if (newStep < 0)
        newStep += NUM_STEPS;

    // Check if we've moved to a new step
    bool triggered = false;
    if (newStep != currentStep)
    {
        currentStep = newStep;
        
        // Trigger if the new step is active
        if (steps[currentStep])
        {
            triggered = true;
        }
    }
    else if (lastPpqPosition < 0.0)
    {
        // First call after reset/stop - check if current step should trigger
        if (steps[currentStep])
        {
            triggered = true;
        }
    }

    lastPpqPosition = ppqPosition;
    return triggered;
}

void StepSequencer::setStep(int stepIndex, bool active)
{
    if (stepIndex >= 0 && stepIndex < NUM_STEPS)
        steps[stepIndex] = active;
}

bool StepSequencer::getStep(int stepIndex) const
{
    if (stepIndex >= 0 && stepIndex < NUM_STEPS)
        return steps[stepIndex];
    return false;
}

void StepSequencer::setRate(Rate newRate)
{
    rate = newRate;
}

bool StepSequencer::isCurrentStepActive() const
{
    return steps[currentStep];
}

double StepSequencer::getBeatsPerStep() const
{
    switch (rate)
    {
        case Rate::OneBar:           return 4.0;    // 1 bar = 4 beats
        case Rate::HalfNote:         return 2.0;    // 1/2 note = 2 beats
        case Rate::QuarterNote:      return 1.0;    // 1/4 note = 1 beat
        case Rate::EighthNote:       return 0.5;    // 1/8 note = 0.5 beats
        case Rate::SixteenthNote:    return 0.25;   // 1/16 note = 0.25 beats
        case Rate::ThirtySecondNote: return 0.125;  // 1/32 note = 0.125 beats
        default:                     return 0.25;
    }
}
