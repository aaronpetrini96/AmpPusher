//
// Created by Aaron Petrini on 17/09/2026.
//

#pragma once

#include <functional>
#include "../LAF/ListOptionsPopupLAF.h"

class AAListOptionsPopup : public juce::Component
{
public:
    struct Item
    {
        juce::String text;
        std::function<void()> onSelected;
        bool isSelected {false};
    };

    explicit AAListOptionsPopup (const juce::String& titleToUse) : title (titleToUse)
    {
        setAlwaysOnTop (true);
        setLookAndFeel (&listOptionsLAF);
    }

    ~AAListOptionsPopup() override
    {
        setLookAndFeel (nullptr);
    }

    void paint (juce::Graphics& g) override
    {
        const auto boundsF = getLocalBounds().toFloat();
        const auto boundsFHeight = boundsF.getHeight();
        const auto cornerSize = boundsFHeight * APCornerPOH;
        g.setColour (APPopupBackgroundColor);
        g.fillRoundedRectangle (boundsF, cornerSize);

        juce::Path titleShape;
        titleShape.addRoundedRectangle (titleBounds.getX(), titleBounds.getY(), titleBounds.getWidth(), titleBounds.getHeight(), cornerSize, cornerSize,
                                        true, true, false, false);
        g.setColour (APColors::presetBoxBackground);
        g.fillPath (titleShape);

        const auto titleDividerThickness = boundsFHeight / getNumRows() * APOutlineThicknessPOH * 0.5f;
        g.setColour (APPopupOutlineColor);
        g.fillRect (titleBounds.withTop (titleBounds.getBottom() - titleDividerThickness));

        g.setColour (APPopupTextColor.withBrightness (2.f));
        g.setFont (APFonts::montserratSemiBold (titleBounds.getHeight() * APFontHeightPOH));
        g.drawText (title, titleBounds, juce::Justification::centred, true);
    }

    void resized() override
    {
        const auto boundsF = getLocalBounds().toFloat();
        const auto outlineThickness = boundsF.getHeight() / getNumRows() * APOutlineThicknessPOH;
        const auto rowHeight = (boundsF.getHeight() - outlineThickness) / getNumRows();
        titleBounds = boundsF.withHeight (rowHeight * APPopupTitleRowRatio);

        for (int rowIndex = 0; rowIndex < rows.size(); ++rowIndex)
            rows[rowIndex]->setBounds (boundsF.withHeight (rowHeight).withY (titleBounds.getBottom() + rowHeight * (float) rowIndex).toNearestInt());
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        const auto boundsF = getLocalBounds().toFloat();
        const auto boundsFHeight = boundsF.getHeight();
        const auto outlineThickness = boundsFHeight / getNumRows() * APOutlineThicknessPOH;
        g.setColour (APPopupOutlineColor);
        g.drawRoundedRectangle (boundsF.reduced (outlineThickness * 0.5f), boundsFHeight * APCornerPOH, outlineThickness);
    }

    float getNumRows() const { return (float) rows.size() + APPopupTitleRowRatio; }

    void setSelectedTextColour (const juce::Colour newColour)
    {
        listOptionsLAF.selectedTextColour = newColour;
        repaint();
    }

protected:
    ListOptionsPopupLAF listOptionsLAF;

    void setItems (const juce::Array<Item>& newItems)
    {
        rows.clear();

        for (const auto& item : newItems)
        {
            auto* row = rows.add (new juce::TextButton (item.text));
            row->setToggleState (item.isSelected, juce::dontSendNotification);
            row->onClick = [this, onSelected = item.onSelected]
            {
                setVisible (false);

                if (onSelected != nullptr)
                    onSelected();
            };
            addAndMakeVisible (row);
        }

        resized();
    }

private:
    juce::String title;
    juce::Rectangle<float> titleBounds;
    juce::OwnedArray<juce::TextButton> rows;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AAListOptionsPopup)
};
