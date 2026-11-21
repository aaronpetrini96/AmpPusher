cmake_minimum_required(VERSION 3.22)
project(AmpPusher VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/JUCE)

set(RTNEURAL_STL    ON CACHE BOOL "" FORCE)
set(BUILD_TESTS     OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(BUILD_BENCH     OFF CACHE BOOL "" FORCE)

add_subdirectory(libs/RTNeural)

enable_testing()

add_executable(parity_test tests/parity_test.cpp)
target_include_directories(parity_test PRIVATE libs/RTNeural)
target_link_libraries(parity_test PRIVATE RTNeural)
target_compile_definitions(parity_test PRIVATE REPO_ROOT="${CMAKE_SOURCE_DIR}")
add_test(NAME ts808_parity COMMAND parity_test ts808)
add_test(NAME sd1_parity COMMAND parity_test sd1)

juce_add_plugin(AmpPusher
    COMPANY_NAME "Aaron Petrini"
    PLUGIN_MANUFACTURER_CODE Aapt
    PLUGIN_CODE Ampp
    FORMATS AU VST3 Standalone
    COPY_PLUGIN_AFTER_BUILD TRUE
    PRODUCT_NAME "AmpPusher")

juce_add_binary_data(AmpPusherData SOURCES models/ts808.json models/sd1.json)

target_sources(AmpPusher PRIVATE source/PluginProcessor.cpp source/PluginEditor.cpp)
target_include_directories(AmpPusher PRIVATE libs/RTNeural)
target_compile_definitions(AmpPusher PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(AmpPusher
    PRIVATE AmpPusherData RTNeural juce::juce_audio_utils
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_lto_flags juce::juce_recommended_warning_flags)