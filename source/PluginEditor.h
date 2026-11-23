#pragma once

#include "PluginProcessor.h"
#include "APLook.h"
#include "UI/Components/RotaryImageKnob.h"
#include "UI/Components/BypassLED.h"
#include "Utils/AAUITools.h"
#include "Utils/AAResizeUI.h"
#include "UI/PopupPanels/UISizeOptionsPanel.h"

class AmpPusherEditor : public juce::AudioProcessorEditor,
                        private juce::ComponentListener
{
public:
    AmpPusherEditor (AmpPusherProcessor&);
    ~AmpPusherEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseUp (const juce::MouseEvent& event) override;
    void componentVisibilityChanged (juce::Component& component) override;

private:
    void updateOdModeLabel();
    void updateModeColours();
    void updateEditorSize();
    void showUISizeOptions();

    AmpPusherProcessor& audioProcessor;
    const int uiWidth {340};
    const int uiHeight {635};

    juce::Image pedalBoxImage {juce::ImageCache::getFromMemory(BinaryData::pedalBoxWithTapev1_png, BinaryData::pedalBoxWithTapev1_pngSize)};
    juce::Image metalKnobBaseImage {juce::ImageCache::getFromMemory(BinaryData::blackMetalKnobBase_png, BinaryData::blackMetalKnobBase_pngSize)};
    juce::Image metalKnobRotorImage {juce::ImageCache::getFromMemory(BinaryData::blackMetalKnobRotor_png, BinaryData::blackMetalKnobRotor_pngSize)};
    juce::DrawableImage switchUpDrawable {juce::ImageCache::getFromMemory(BinaryData::switchLeft_png, BinaryData::switchLeft_pngSize)};
    juce::DrawableImage switchDownDrawable {juce::ImageCache::getFromMemory(BinaryData::switchRight_png, BinaryData::switchRight_pngSize)};
    juce::DrawableImage footswitchDownDrawable {juce::ImageCache::getFromMemory(BinaryData::footswitchDown_png, BinaryData::footswitchDown_pngSize)};
    juce::DrawableImage footswitchUpDrawable {juce::ImageCache::getFromMemory(BinaryData::footswitchUp_png, BinaryData::footswitchUp_pngSize)};
    juce::Image modeLabel808Image {juce::ImageCache::getFromMemory(BinaryData::modeLabel808_png, BinaryData::modeLabel808_pngSize)};
    juce::Image modeLabelS1DImage {juce::ImageCache::getFromMemory(BinaryData::modeLabelS1D_png, BinaryData::modeLabelS1D_pngSize)};

    RotaryImageKnob driveKnob {*audioProcessor.parameters.getParameter(ParamNames::getDriveNames().id), metalKnobBaseImage, metalKnobRotorImage};
    RotaryImageKnob levelKnob {*audioProcessor.parameters.getParameter(ParamNames::getLevelNames().id), metalKnobBaseImage, metalKnobRotorImage};
    RotaryImageKnob toneKnob {*audioProcessor.parameters.getParameter(ParamNames::getToneNames().id), metalKnobBaseImage, metalKnobRotorImage};

    AAUITools::PrintedText driveLabel {"DRIVE", APColors::labelInk};
    AAUITools::PrintedText levelLabel {"LEVEL", APColors::labelInk};
    AAUITools::PrintedText toneLabel {"TONE", APColors::labelInk};
    AAUITools::PrintedText modeLabel {"MODE", APColors::labelInk};

    juce::ImageComponent odModeLabel;

    juce::DrawableButton odModeSwitch {"odModeSwitch", juce::DrawableButton::ImageFitted};
    juce::AudioProcessorValueTreeState::ButtonAttachment odModeSwitchAttachment {audioProcessor.parameters, ParamNames::getODModeNames().id, odModeSwitch};

    BypassLED bypassLED {*audioProcessor.parameters.getParameter(ParamNames::getBypassNames().id), *audioProcessor.parameters.getParameter(ParamNames::getODModeNames().id)};
    BypassLED secondBypassLED {*audioProcessor.parameters.getParameter(ParamNames::getBypassNames().id), *audioProcessor.parameters.getParameter(ParamNames::getODModeNames().id)};
    juce::DrawableButton bypassFootswitch {"bypassButton", juce::DrawableButton::ImageFitted};
    juce::AudioProcessorValueTreeState::ButtonAttachment bypassFootswitchAttachment {audioProcessor.parameters, ParamNames::getBypassNames().id, bypassFootswitch};

    float uiScale {0.0f};
    AAResizeUI uiResize {uiScale, uiWidth, uiHeight};
    UISizeOptionsPanel resizeOptionsPopup {uiResize};
    juce::DrawableButton resizeButton {"GUI Size", juce::DrawableButton::ImageFitted};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherEditor)
};
