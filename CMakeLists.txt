cmake_minimum_required(VERSION 3.22)

project(FerruxeBass VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(JUCE_DIR "$ENV{HOME}/opt/JUCE" CACHE PATH "Path to local JUCE installation")
option(FERRUXE_COPY_PLUGIN_AFTER_BUILD "Copy built plugin to user ~/.vst3 directory" OFF)

if(EXISTS "${JUCE_DIR}/CMakeLists.txt")
    message(STATUS "Found JUCE at: ${JUCE_DIR}")
    add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")
else()
    message(FATAL_ERROR
        "JUCE CMakeLists.txt not found at '${JUCE_DIR}'.\n"
        "Set the path with: cmake -B build -DJUCE_DIR=/path/to/JUCE")
endif()

juce_add_plugin(FerruxeBass
    COMPANY_NAME "FerruxeAudio"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${FERRUXE_COPY_PLUGIN_AFTER_BUILD}
    PLUGIN_MANUFACTURER_CODE Frxe
    PLUGIN_CODE FrxB
    FORMATS VST3 LV2 Standalone
    LV2URI "https://ferruxeaudio.com/plugins/ferruxebass"
    VST3_CATEGORIES Fx Distortion
    PRODUCT_NAME "FerruxeBass"
)

target_sources(FerruxeBass
    PRIVATE
        Source/Parameters/AudioMeterBridge.h
        Source/Parameters/PluginParameters.h
        Source/DSP/BiquadFilter.h
        Source/DSP/Stages/BassCompressor.h
        Source/DSP/Stages/BrickwallClipper.h
        Source/DSP/Stages/SansAmpDriveStage.h
        Source/DSP/Stages/SubHarmonicExciter.h
        Source/DSP/FerruxeEngine.cpp
        Source/DSP/FerruxeEngine.h
        Source/UI/Theme.h
        Source/UI/FerruxeLookAndFeel.h
        Source/UI/Components/HeaderMeterStrip.h
        Source/UI/Components/RotaryKnob.h
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_include_directories(FerruxeBass
    PRIVATE
        Source
)

target_compile_definitions(FerruxeBass
    PUBLIC
        JUCE_ALSA=1
        JUCE_JACK=1
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
)

target_link_libraries(FerruxeBass
    PRIVATE
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
