#pragma once

#include "PluginProcessor.h"
#include "APLook.h"
#include "UI/Components/RotaryImageKnob.h"
#include "UI/Components/BypassLED.h"
#include "Utils/AAUITools.h"

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

    juce::Image pedalBoxImage {juce::ImageCache::getFromMemory(BinaryData::pedalBoxWithTape2_png, BinaryData::pedalBoxWithTape2_pngSize)};
    juce::Image bigKnobBaseImage {juce::ImageCache::getFromMemory(BinaryData::bigKnobBase2_png, BinaryData::bigKnobBase2_pngSize)};
    juce::Image bigKnobRotorImage {juce::ImageCache::getFromMemory(BinaryData::bigKnobRotator2_png, BinaryData::bigKnobRotator2_pngSize)};
    juce::Image smallKnobBaseImage {juce::ImageCache::getFromMemory(BinaryData::smallKnobBase_png, BinaryData::smallKnobBase_pngSize)};
    juce::Image smallKnobRotorImage {juce::ImageCache::getFromMemory(BinaryData::smallKnobRotor2_png, BinaryData::smallKnobRotor2_pngSize)};
    juce::Image switchUpImage {juce::ImageCache::getFromMemory(BinaryData::switchUP_png, BinaryData::switchUP_pngSize)};
    juce::Image switchDownImage {juce::ImageCache::getFromMemory(BinaryData::switchDOWN_png, BinaryData::switchDOWN_pngSize)};
    juce::Image footswitchDownImage {juce::ImageCache::getFromMemory(BinaryData::sw4_down_png, BinaryData::sw4_down_pngSize)};
    juce::Image footswitchUpImage {juce::ImageCache::getFromMemory(BinaryData::sw4_up_png, BinaryData::sw4_up_pngSize)};
    RotaryImageKnob driveKnob {*audioProcessor.parameters.getParameter(ParamNames::getDriveNames().id), bigKnobBaseImage, bigKnobRotorImage};
    RotaryImageKnob levelKnob {*audioProcessor.parameters.getParameter(ParamNames::getLevelNames().id), bigKnobBaseImage, bigKnobRotorImage};
    RotaryImageKnob toneKnob {*audioProcessor.parameters.getParameter(ParamNames::getToneNames().id), smallKnobBaseImage, smallKnobRotorImage};

    AAUITools::PrintedText driveLabel {"DRIVE", APColors::labelInk};
    AAUITools::PrintedText levelLabel {"LEVEL", APColors::labelInk};
    AAUITools::PrintedText toneLabel {"TONE", APColors::labelInk};

    AAUITools::PrintedText odModeLabel {"808", APColors::markerInk};
    juce::ParameterAttachment odModeLabelAttachment {*audioProcessor.parameters.getParameter(ParamNames::getODModeNames().id),
                                                     [this](float modeIndex) { odModeLabel.setText(modeIndex >= 0.5f ? "S1D" : "808"); }};

    juce::ImageButton odModeSwitch;
    juce::AudioProcessorValueTreeState::ButtonAttachment odModeSwitchAttachment {audioProcessor.parameters, ParamNames::getODModeNames().id, odModeSwitch};

    BypassLED bypassLED {*audioProcessor.parameters.getParameter(ParamNames::getBypassNames().id), *audioProcessor.parameters.getParameter(ParamNames::getODModeNames().id)};

    juce::DrawableButton bypassFootswitch {"bypassButton", juce::DrawableButton::ImageFitted};
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassFootswitchAttachment {audioProcessor.parameters, ParamNames::getBypassNames().id, bypassFootswitch};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherEditor)
};
