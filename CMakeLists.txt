cmake_minimum_required(VERSION 3.22)

project(ScaleOperator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch JUCE automatically so you don't have to install it by hand ---
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(ScaleOperator
    COMPANY_NAME "HomeBrew"
    PLUGIN_MANUFACTURER_CODE Hbrw
    PLUGIN_CODE Scop
    FORMATS VST3 Standalone
    PRODUCT_NAME "Scale Operator"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    VST3_CATEGORIES "Instrument" "Synth")

target_sources(ScaleOperator PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/FMVoice.cpp
    Source/ScalePage.cpp
    Source/OperatorPage.cpp)

target_compile_definitions(ScaleOperator PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

target_link_libraries(ScaleOperator PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_audio_processors
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
