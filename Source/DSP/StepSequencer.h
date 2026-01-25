/*
  ==============================================================================

    StepSequencer.h
    16-step gate sequencer with DAW tempo sync

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class StepSequencer
{
public:
    static constexpr int NUM_STEPS = 16;

    enum class Rate
    {
        OneBar,      // 1/1
        HalfNote,    // 1/2
        QuarterNote, // 1/4
        EighthNote,  // 1/8
        SixteenthNote, // 1/16
        ThirtySecondNote // 1/32
    };

    StepSequencer();
    ~StepSequencer() = default;

    void prepare(double sampleRate);
    void reset();

    // Process the sequencer for the current block
    // Returns true if a step was triggered during this call
    // Must be called once per sample
    bool process(const juce::AudioPlayHead::PositionInfo& positionInfo);

    // Set/get step state
    void setStep(int stepIndex, bool active);
    bool getStep(int stepIndex) const;

    // Set/get rate
    void setRate(Rate newRate);
    Rate getRate() const { return rate; }

    // Get current step index (0-15)
    int getCurrentStep() const { return currentStep; }

    // Check if the current step is active
    bool isCurrentStepActive() const;

private:
    double sampleRate = 44100.0;
    
    // Step states
    bool steps[NUM_STEPS] = { false };

    // Current state
    int currentStep = 0;
    Rate rate = Rate::SixteenthNote;
    double lastPpqPosition = -1.0;

    // Convert rate enum to beats per step
    double getBeatsPerStep() const;
};
