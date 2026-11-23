#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>

namespace APColors
{
    inline const juce::Colour labelInk {232, 230, 224};
    inline const juce::Colour ledGreen {60, 200, 80};
    inline const juce::Colour ledYellow {235, 200, 40};
    inline const juce::Colour ledRing {15, 15, 15};
    inline const juce::Colour ledOff {45, 45, 48};

    inline const juce::Colour activeNeonBlue {64, 196, 255};
    inline const juce::Colour iconSVGColor {255, 255, 255};
    inline const juce::Colour iconOverColor {100, 100, 100};
    inline const juce::Colour presetBoxBackground {35, 42, 60};
}

constexpr float APOutlineThicknessPOH {0.06f};
constexpr float APCornerPOH {0.0f};
constexpr float APFontHeightPOH {0.625f};
constexpr float APPopupTitleRowRatio {1.1f};

inline const juce::Colour APPopupBackgroundColor {5, 7, 14};
inline const juce::Colour APPopupOutlineColor {40, 50, 70};
inline const juce::Colour APPopupTextColor {215, 215, 215};
inline const juce::Colour APPopupSelectedTextColor {APColors::activeNeonBlue};

namespace APFonts
{
    inline juce::Font montserratMedium (const float fontHeight)
    {
        static const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::MontserratMedium_ttf, BinaryData::MontserratMedium_ttfSize);
        return juce::Font {juce::FontOptions (typeface).withHeight (juce::jmax (1.0f, fontHeight))};
    }

    inline juce::Font montserratSemiBold (const float fontHeight)
    {
        static const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::MontserratSemiBold_ttf, BinaryData::MontserratSemiBold_ttfSize);
        return juce::Font {juce::FontOptions (typeface).withHeight (juce::jmax (1.0f, fontHeight))};
    }

    inline juce::Font arimoBold (const float fontHeight)
    {
        static const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::ArimoBold_ttf, BinaryData::ArimoBold_ttfSize);
        return juce::Font {juce::FontOptions (typeface).withHeight (juce::jmax (1.0f, fontHeight))};
    }
}

