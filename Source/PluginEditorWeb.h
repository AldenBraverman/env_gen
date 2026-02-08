#pragma once

#if ENVGEN_USE_WEB_GUI && JUCE_WEB_BROWSER

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"
#include "Components/OscilloscopeComponent.h"

//==============================================================================
// Web-based plugin editor: native oscilloscope + WebBrowserComponent (setParameter, getState).
// Used when ENVGEN_USE_WEB_GUI is ON; otherwise PluginEditor (native) is used.
class EnvGenEditorWeb : public juce::AudioProcessorEditor,
                        public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit EnvGenEditorWeb(EnvGenAudioProcessor&);
    ~EnvGenEditorWeb() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void parameterChanged(const juce::String& parameterID, float newValue) override;

private:
    EnvGenAudioProcessor& processorRef;
    std::unique_ptr<OsciloscopeComponent> oscilloscope;
    std::unique_ptr<juce::WebBrowserComponent> webBrowser;
    juce::File guiRootDir;

    std::unique_ptr<juce::Component> envelopeOverlayHolder;
    juce::WebBrowserComponent* envelopeOverlayBrowser = nullptr;
    bool overlayTransparencyApplied = false;

    void pushParameterToWeb(const juce::String& id, float value);
    void tryApplyOverlayTransparency();
    void pushEnvelopeToOverlay(const float* data, int size);
    static juce::String escapeJsString(const juce::String& s);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvGenEditorWeb)
};

#endif
