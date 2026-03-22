cmake_minimum_required(VERSION 3.22)

project(HeadturnerHat VERSION 0.1.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(HeadturnerHat
    COMPANY_NAME "Codex Audio Lab"
    BUNDLE_ID "com.codexaudiolab.headturnerhat"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE CxAL
    PLUGIN_CODE HtHt
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "Headturner Hat"
)

target_sources(HeadturnerHat
    PRIVATE
        src/PluginEditor.cpp
        src/PluginEditor.h
        src/PluginProcessor.cpp
        src/PluginProcessor.h
)

juce_generate_juce_header(HeadturnerHat)

target_compile_definitions(HeadturnerHat
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(HeadturnerHat
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(WIN32)
    target_link_libraries(HeadturnerHat PRIVATE ole32)

    if(TARGET HeadturnerHat_vst3_helper)
        target_link_libraries(HeadturnerHat_vst3_helper PRIVATE ole32)
    endif()
endif()
