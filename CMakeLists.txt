cmake_minimum_required(VERSION 3.22)

project(EnvGen VERSION 1.0.0)

option(ENVGEN_USE_WEB_GUI "Use React web UI in plugin editor" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

set(ENVGEN_PLUGIN_WEB_OPTS "")
if(ENVGEN_USE_WEB_GUI)
    list(APPEND ENVGEN_PLUGIN_WEB_OPTS NEEDS_WEB_BROWSER TRUE NEEDS_WEBVIEW2 TRUE)
endif()

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
    ${ENVGEN_PLUGIN_WEB_OPTS}
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
if(ENVGEN_USE_WEB_GUI)
    target_sources(EnvGen PRIVATE
        Source/PluginEditorWeb.cpp
        Source/PluginEditorWeb.h
    )
    set(GUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/gui")
    set(ENVGEN_GUI_OUT "${CMAKE_CURRENT_BINARY_DIR}/EnvGenGui")
    add_custom_target(EnvGenGui
        COMMAND ${CMAKE_COMMAND} -E echo "Building web GUI..."
        COMMAND npm ci
        COMMAND npm run build
        WORKING_DIRECTORY ${GUI_DIR}
        COMMENT "Build React GUI in gui/"
    )
    add_custom_command(TARGET EnvGenGui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory "${ENVGEN_GUI_OUT}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${GUI_DIR}/dist" "${ENVGEN_GUI_OUT}"
        COMMENT "Copy gui/dist to EnvGenGui for plugin"
    )
endif()

# Set include directories
target_include_directories(EnvGen
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# JUCE modules and web GUI
target_compile_definitions(EnvGen
    PUBLIC
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
if(ENVGEN_USE_WEB_GUI)
    target_compile_definitions(EnvGen PRIVATE ENVGEN_USE_WEB_GUI=1 JUCE_WEB_BROWSER=1)
else()
    target_compile_definitions(EnvGen PUBLIC JUCE_WEB_BROWSER=0)
endif()

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
