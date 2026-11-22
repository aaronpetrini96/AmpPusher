#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../../APLook.h"

class TapeButtonLAF : public juce::LookAndFeel_V4
{
public:
    TapeButtonLAF() = default;

    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override
    {
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool, bool) override
    {
        const auto boundsF = button.getLocalBounds().toFloat();
        g.setColour(APColors::markerInk);
        g.setFont(APFonts::permanentMarker(boundsF.getHeight() * 0.7f));
        g.drawText(button.getToggleState() ? "S1D" : "808", boundsF, juce::Justification::centred, true);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TapeButtonLAF)
};
