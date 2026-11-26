//
// Created by Aaron Petrini on 19/09/2026.
//

#pragma once
#include "../../APLook.h"

class ListOptionsPopupLAF : public juce::LookAndFeel_V4
{
public:
    ListOptionsPopupLAF () = default;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override {}

    void drawButtonText (juce::Graphics& g, juce::TextButton& button, const bool shouldDrawButtonAsHighlighted, bool) override
    {
        const auto boundsF = button.getLocalBounds().toFloat();
        const auto cornerSize = boundsF.getHeight() * 0.15f;
        const auto textColour = button.getToggleState() ? selectedTextColour : APPopupTextColor;

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour (APColors::presetBoxBackground);
            g.fillRoundedRectangle (boundsF.reduced (boundsF.getHeight() * APOutlineThicknessPOH, 0.f), cornerSize);
        }

        g.setColour (textColour);
        g.setFont (APFonts::montserratMedium (boundsF.getHeight() * APFontHeightPOH));
        g.drawText (button.getButtonText(), boundsF, juce::Justification::centred, true);
    }

    juce::Colour selectedTextColour {APPopupSelectedTextColor};
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListOptionsPopupLAF)
};
