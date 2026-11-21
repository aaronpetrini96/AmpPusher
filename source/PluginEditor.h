#pragma once

#include "PluginProcessor.h"
#include "APLook.h"
#include "RotaryImageKnob.h"

class AmpPusherEditor : public juce::AudioProcessorEditor
{
public:
    AmpPusherEditor (AmpPusherProcessor&);
    ~AmpPusherEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AmpPusherProcessor& audioProcessor;

    juce::Image pedalBoxImage {juce::ImageCache::getFromMemory(BinaryData::pedalBoxWithTape_png, BinaryData::pedalBoxWithTape_pngSize)};
    juce::Image bigKnobBaseImage {juce::ImageCache::getFromMemory(BinaryData::bigKnobBase_png, BinaryData::bigKnobBase_pngSize)};
    juce::Image bigKnobRotorImage {juce::ImageCache::getFromMemory(BinaryData::bigKnobRotator_png, BinaryData::bigKnobRotator_pngSize)};

    RotaryImageKnob driveKnob {*audioProcessor.parameters.getParameter(ParamNames::getDriveNames().id), bigKnobBaseImage, bigKnobRotorImage};
    RotaryImageKnob levelKnob {*audioProcessor.parameters.getParameter(ParamNames::getLevelNames().id), bigKnobBaseImage, bigKnobRotorImage};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherEditor)
};
