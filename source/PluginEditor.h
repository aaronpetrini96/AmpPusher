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
    void updateOdModeLabel();

    AmpPusherProcessor& audioProcessor;
    const int uiWidth {340};
    const int uiHeight {635};

    juce::Image pedalBoxImage {juce::ImageCache::getFromMemory(BinaryData::pedalBoxWithTapev1_png, BinaryData::pedalBoxWithTapev1_pngSize)};
    juce::Image metalKnobBaseImage {juce::ImageCache::getFromMemory(BinaryData::blackMetalKnobBase_png, BinaryData::blackMetalKnobBase_pngSize)};
    juce::Image metalKnobRotorImage {juce::ImageCache::getFromMemory(BinaryData::blackMetalKnobRotor_png, BinaryData::blackMetalKnobRotor_pngSize)};
    juce::DrawableImage switchUpDrawable {juce::ImageCache::getFromMemory(BinaryData::switchUP_png, BinaryData::switchUP_pngSize)};
    juce::DrawableImage switchDownDrawable {juce::ImageCache::getFromMemory(BinaryData::switchDOWN_png, BinaryData::switchDOWN_pngSize)};
    juce::DrawableImage footswitchDownDrawable {juce::ImageCache::getFromMemory(BinaryData::sw4_down_png, BinaryData::sw4_down_pngSize)};
    juce::DrawableImage footswitchUpDrawable {juce::ImageCache::getFromMemory(BinaryData::sw4_up_png, BinaryData::sw4_up_pngSize)};
    juce::Image modeLabel808Image {juce::ImageCache::getFromMemory(BinaryData::modeLabel808_png, BinaryData::modeLabel808_pngSize)};
    juce::Image modeLabelS1DImage {juce::ImageCache::getFromMemory(BinaryData::modeLabelS1D_png, BinaryData::modeLabelS1D_pngSize)};

    RotaryImageKnob driveKnob {*audioProcessor.parameters.getParameter(ParamNames::getDriveNames().id), metalKnobBaseImage, metalKnobRotorImage};
    RotaryImageKnob levelKnob {*audioProcessor.parameters.getParameter(ParamNames::getLevelNames().id), metalKnobBaseImage, metalKnobRotorImage};
    RotaryImageKnob toneKnob {*audioProcessor.parameters.getParameter(ParamNames::getToneNames().id), metalKnobBaseImage, metalKnobRotorImage};

    AAUITools::PrintedText driveLabel {"DRIVE", APColors::labelInk};
    AAUITools::PrintedText levelLabel {"LEVEL", APColors::labelInk};
    AAUITools::PrintedText toneLabel {"TONE", APColors::labelInk};

    juce::ImageComponent odModeLabel;

    juce::DrawableButton odModeSwitch {"odModeSwitch", juce::DrawableButton::ImageFitted};
    juce::AudioProcessorValueTreeState::ButtonAttachment odModeSwitchAttachment {audioProcessor.parameters, ParamNames::getODModeNames().id, odModeSwitch};

    BypassLED bypassLED {*audioProcessor.parameters.getParameter(ParamNames::getBypassNames().id), *audioProcessor.parameters.getParameter(ParamNames::getODModeNames().id)};
    juce::DrawableButton bypassFootswitch {"bypassButton", juce::DrawableButton::ImageFitted};
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassFootswitchAttachment {audioProcessor.parameters, ParamNames::getBypassNames().id, bypassFootswitch};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherEditor)
};
