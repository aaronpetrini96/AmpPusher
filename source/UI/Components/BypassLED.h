#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "../../APLook.h"

class BypassLED : public juce::Component
{
public:
    BypassLED (juce::RangedAudioParameter& bypassParameter, juce::RangedAudioParameter& odModeParameter)
        : bypassAttachment (bypassParameter, [this] (float bypassValue) { isLit = bypassValue < 0.5f; repaint(); }),
          odModeAttachment (odModeParameter, [this] (float modeIndex) { isS1DMode = modeIndex >= 0.5f; repaint(); })
    {
        bypassAttachment.sendInitialUpdate();
        odModeAttachment.sendInitialUpdate();
    }

    void paint (juce::Graphics& g) override
    {
        const auto boundsF = getLocalBounds().toFloat();
        const auto diameter = juce::jmin (boundsF.getWidth(), boundsF.getHeight());
        const auto ringDiameter = diameter * 0.75f;
        const auto ringBounds = boundsF.withSizeKeepingCentre (ringDiameter, ringDiameter);

        g.setColour (APColors::ledRing);
        g.fillEllipse (ringBounds);

        const auto lensBounds = ringBounds.reduced (ringDiameter * 0.11f);
        const auto lensColour = isLit ? (isS1DMode ? APColors::ledYellow : APColors::ledGreen) : APColors::ledOff;
        const auto lensColourBrighter = isLit ? lensColour.brighter (1.f) : lensColour;
        const auto lensGradient = juce::ColourGradient (lensColourBrighter, lensBounds.getCentre(),
                                                        lensColour, {lensBounds.getRight(), lensBounds.getCentreY()}, true);

        g.setGradientFill (lensGradient);
        g.fillEllipse (lensBounds);

        if (isLit)
        {
            const juce::ColourGradient glow (lensColourBrighter.withAlpha (0.4f), boundsF.getCentre(),
                                             lensColourBrighter.withAlpha (0.f), boundsF.getCentre().translated (diameter * 0.5f, 0.f), true);
            g.setGradientFill (glow);
            g.fillEllipse (boundsF.withSizeKeepingCentre (diameter, diameter));
        }
    }

private:
    juce::ParameterAttachment bypassAttachment;
    juce::ParameterAttachment odModeAttachment;
    bool isLit {true};
    bool isS1DMode {false};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BypassLED)
};
