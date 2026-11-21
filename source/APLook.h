#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>

namespace APColors
{
    inline const juce::Colour background {0, 0, 0};
    inline const juce::Colour markerInk {22, 22, 26};
    inline const juce::Colour labelInk {232, 230, 224};
    inline const juce::Colour ledGreen {60, 200, 80};
    inline const juce::Colour ledYellow {235, 200, 40};
    inline const juce::Colour ledRing {5, 5, 5};
}

namespace APFonts
{
    inline juce::Font permanentMarker (const float fontHeight)
    {
        static const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::PermanentMarkerRegular_ttf, BinaryData::PermanentMarkerRegular_ttfSize);
        return juce::Font {juce::FontOptions (typeface).withHeight (juce::jmax (1.0f, fontHeight))};
    }

    inline juce::Font arimoBold (const float fontHeight)
    {
        static const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::ArimoBold_ttf, BinaryData::ArimoBold_ttfSize);
        return juce::Font {juce::FontOptions (typeface).withHeight (juce::jmax (1.0f, fontHeight))};
    }
}

namespace APLook
{
}

