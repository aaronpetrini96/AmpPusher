#pragma once

#include "PluginProcessor.h"
#include "APLook.h"
#include "RotaryImageKnob.h"
#include "BypassLED.h"
#include "Utils/AAUITools.h"
#include "TapeButtonLAF.h"

class AmpPusherEditor : public juce::AudioProcessorEditor
{
public:
    AmpPusherEditor (AmpPusherProcessor&);
    ~AmpPusherEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AmpPusherProcessor& audioProcessor;
    const int uiWidth {340};
    const int uiHeight {600};

    juce::Image pedalBoxImage {juce::ImageCache::getFromMemory(BinaryData::pedalBoxWithTape_png, BinaryData::pedalBoxWithTape_pngSize)};
    juce::Image bigKnobBaseImage {juce::ImageCache::getFromMemory(BinaryData::bigKnobBase_png, BinaryData::bigKnobBase_pngSize)};
    juce::Image bigKnobRotorImage {juce::ImageCache::getFromMemory(BinaryData::bigKnobRotator_png, BinaryData::bigKnobRotator_pngSize)};
    juce::Image smallKnobBaseImage {juce::ImageCache::getFromMemory(BinaryData::smallKnobBase_png, BinaryData::smallKnobBase_pngSize)};
    juce::Image smallKnobRotorImage {juce::ImageCache::getFromMemory(BinaryData::smallKnobRotor_png, BinaryData::smallKnobRotor_pngSize)};
    juce::Image switchUpImage {juce::ImageCache::getFromMemory(BinaryData::switchUP_png, BinaryData::switchUP_pngSize)};
    juce::Image switchDownImage {juce::ImageCache::getFromMemory(BinaryData::switchDOWN_png, BinaryData::switchDOWN_pngSize)};
    RotaryImageKnob driveKnob {*audioProcessor.parameters.getParameter(ParamNames::getDriveNames().id), bigKnobBaseImage, bigKnobRotorImage};
    RotaryImageKnob levelKnob {*audioProcessor.parameters.getParameter(ParamNames::getLevelNames().id), bigKnobBaseImage, bigKnobRotorImage};
    RotaryImageKnob toneKnob {*audioProcessor.parameters.getParameter(ParamNames::getToneNames().id), smallKnobBaseImage, smallKnobRotorImage};

    AAUITools::PrintedText driveLabel {"DRIVE", APColors::labelInk};
    AAUITools::PrintedText levelLabel {"LEVEL", APColors::labelInk};
    AAUITools::PrintedText toneLabel {"TONE", APColors::labelInk};

    TapeButtonLAF tapeButtonLAF;
    juce::TextButton odModeTapeButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment odModeTapeButtonAttachment {audioProcessor.parameters, ParamNames::getODModeNames().id, odModeTapeButton};

    juce::ImageButton odModeSwitch;
    juce::AudioProcessorValueTreeState::ButtonAttachment odModeSwitchAttachment {audioProcessor.parameters, ParamNames::getODModeNames().id, odModeSwitch};

    BypassLED bypassLED {*audioProcessor.parameters.getParameter(ParamNames::getBypassNames().id), *audioProcessor.parameters.getParameter(ParamNames::getODModeNames().id)};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherEditor)
};
