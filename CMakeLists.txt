cmake_minimum_required(VERSION 3.22)

project(EnvGen VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE from GitHub
include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(JUCE)

# Add the plugin target
juce_add_plugin(EnvGen
    COMPANY_NAME "EnvGen"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Envg
    PLUGIN_CODE Envg
    FORMATS VST3 Standalone
    PRODUCT_NAME "Envelope Generator"
)

# Generate JuceHeader.h for legacy include style
juce_generate_juce_header(EnvGen)

# Source files
target_sources(EnvGen
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
        Source/DSP/Envelope.cpp
        Source/DSP/Envelope.h
        Source/DSP/Filter.cpp
        Source/DSP/Filter.h
        Source/DSP/StepSequencer.cpp
        Source/DSP/StepSequencer.h
        Source/Components/CustomLookAndFeel.cpp
        Source/Components/CustomLookAndFeel.h
        Source/Components/StepButton.cpp
        Source/Components/StepButton.h
        Source/Components/EnvelopeLane.cpp
        Source/Components/EnvelopeLane.h
        Source/Components/OscilloscopeComponent.cpp
        Source/Components/OscilloscopeComponent.h
)

# Set include directories
target_include_directories(EnvGen
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# JUCE modules
target_compile_definitions(EnvGen
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(EnvGen
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
