#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../../APLook.h"

class BypassLED : public juce::Component
{
public:
    BypassLED (juce::RangedAudioParameter& bypassParameter, juce::RangedAudioParameter& odModeParameter)
        : bypassAttachment (bypassParameter, [this] (float bypassValue) { isOn = bypassValue < 0.5f; repaint(); }),
          odModeAttachment (odModeParameter, [this] (float modeIndex) { isYellow = modeIndex >= 0.5f; repaint(); })
    {
        bypassAttachment.sendInitialUpdate();
        odModeAttachment.sendInitialUpdate();
    }

    void paint (juce::Graphics& g) override
    {
        const auto boundsF = getLocalBounds().toFloat();
        const auto lightDiameter = juce::jmin (boundsF.getWidth(), boundsF.getHeight()) * 0.5f;
        const auto ledDiameter = lightDiameter * 0.8f;
        const auto ledCentre = boundsF.getCentre();
        const auto ledBounds = juce::Rectangle<float> (ledDiameter, ledDiameter).withCentre (ledCentre);
        const auto ledColour = isYellow ? APColors::ledYellow : APColors::ledGreen;
        const auto litAmount = isOn ? 1.f : 0.f;

        if (isOn)
        {
            const auto glowRadius = lightDiameter;
            g.setGradientFill (juce::ColourGradient (ledColour.withAlpha (0.35f), ledCentre, ledColour.withAlpha (0.f), ledCentre.translated (glowRadius, 0.f), true));
            g.fillEllipse (ledBounds.withSizeKeepingCentre (glowRadius * 2.f, glowRadius * 2.f));
        }

        g.setColour (APColors::ledRing);
        g.fillEllipse (ledBounds.withSizeKeepingCentre (lightDiameter, lightDiameter));

        const auto centreColour = ledColour.withMultipliedBrightness (0.4f).interpolatedWith (ledColour.interpolatedWith (juce::Colours::white, 0.5f), litAmount);
        const auto edgeColour = ledColour.withMultipliedBrightness (0.15f).interpolatedWith (ledColour, litAmount);
        g.setGradientFill (juce::ColourGradient (centreColour, ledCentre.translated (0.f, ledDiameter * 0.3f), edgeColour, {ledCentre.x, ledBounds.getY()}, true));
        g.fillEllipse (ledBounds);

        g.setColour (juce::Colours::white.withAlpha (0.55f));
        g.fillEllipse (ledBounds.withSizeKeepingCentre (ledDiameter * 0.15f, ledDiameter * 0.12f).translated (0.f, -ledDiameter * 0.22f));
    }

private:
    juce::ParameterAttachment bypassAttachment;
    juce::ParameterAttachment odModeAttachment;
    bool isOn {true};
    bool isYellow {false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassLED)
};
