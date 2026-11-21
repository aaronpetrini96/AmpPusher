#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>

namespace APColors
{
    inline const juce::Colour background {0, 0, 0};
    inline const juce::Colour markerInk {22, 22, 26};
    inline const juce::Colour ledOn {220, 25, 20};
    inline const juce::Colour ledOff {70, 20, 20};
}

namespace APFonts
{
    inline juce::Font permanentMarker (const float fontHeight)
    {
        static const juce::Typeface::Ptr typeface = juce::Typeface::createSystemTypefaceFor (
            BinaryData::PermanentMarkerRegular_ttf, BinaryData::PermanentMarkerRegular_ttfSize);
        return juce::Font {juce::FontOptions (typeface).withHeight (juce::jmax (1.0f, fontHeight))};
    }
}

