#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "BinaryData.h"

namespace AAUITools
{
    inline void setIconButtonImages (juce::DrawableButton& button, const void* iconData, const int iconDataSize,
                                     const juce::Colour iconColour, const juce::Colour overColour, const juce::Colour activeColour)
    {
        if (const auto iconNormal = juce::Drawable::createFromImageData (iconData,  static_cast<size_t>(iconDataSize)))
        {
            const auto iconOver = iconNormal->createCopy();
            const auto iconActive = iconNormal->createCopy();
            iconOver->replaceColour (iconColour, overColour);
            iconActive->replaceColour (iconColour, activeColour);
            button.setImages (iconNormal.get(), iconOver.get(), iconActive.get(), nullptr, iconActive.get());
            button.setEdgeIndent (0);
            button.setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
            button.setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
        }
    }

    inline void setToggleImageButton (juce::DrawableButton& button, const juce::Drawable& offImage, const juce::Drawable& onImage,
                                      const juce::Drawable& offPressedImage, const juce::Drawable& onPressedImage)
    {
        button.setImages (&offImage, &offImage, &offPressedImage, nullptr, &onImage, &onImage, &onPressedImage);
        button.setEdgeIndent (0);
        button.setColour (juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
        button.setColour (juce::DrawableButton::backgroundOnColourId, juce::Colours::transparentBlack);
        button.setClickingTogglesState (true);
        button.setTriggeredOnMouseDown (true);
    }

    class PrintedText : public juce::Component
    {
    public:
        PrintedText (const juce::String& textToUse, const juce::Colour inkColorToUse = juce::Colours::whitesmoke, const juce::Justification justificationToUse = juce::Justification::centredTop)
            : text (textToUse), inkColor (inkColorToUse), justification (justificationToUse)
        {
            jassert (grainTile.isValid());
            setInterceptsMouseClicks (false, false);
        }

        void paint (juce::Graphics& g) override
        {
            if (textPathIsStale)
                rebuildTextPath();

            g.setColour (inkColor);
            g.fillPath (textPath);

            const auto originInTopLevel = getTopLevelComponent()->getLocalPoint (this, juce::Point<float>());
            g.setFillType (juce::FillType (grainTile, juce::AffineTransform::translation (-originInTopLevel)));
            g.fillPath (textPath);
        }

        void resized() override
        {
            textPathIsStale = true;
        }

        void setText (const juce::String& newText)
        {
            if (text == newText)
                return;

            text = newText;
            textPathIsStale = true;
            repaint();
        }

        void setFont (const juce::Font& newFont)
        {
            if (font == newFont)
                return;

            font = newFont;
            textPathIsStale = true;
            repaint();
        }

    private:
        void rebuildTextPath()
        {
            juce::GlyphArrangement glyphs;
            glyphs.addFittedText (font, text, 0.f, 0.f, static_cast<float> (getWidth()), static_cast<float> (getHeight()), justification, 1);

            textPath.clear();
            glyphs.createPath (textPath);
            textPathIsStale = false;
        }

        juce::Image grainTile = juce::ImageCache::getFromMemory (BinaryData::inkGrainTile_png, BinaryData::inkGrainTile_pngSize);
        juce::String text;
        juce::Colour inkColor;
        juce::Justification justification;
        juce::Font font {juce::FontOptions {}};
        juce::Path textPath;
        bool textPathIsStale {true};

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PrintedText)
    };
}
