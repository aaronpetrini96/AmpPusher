#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include "PluginProcessor.h"

class AmpPusherEditor : public juce::AudioProcessorEditor
{
public:
    AmpPusherEditor (AmpPusherProcessor&);
    ~AmpPusherEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AmpPusherProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherEditor)
};
