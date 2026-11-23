#include "PluginProcessor.h"
#include "PluginEditor.h"

AmpPusherEditor::AmpPusherEditor (AmpPusherProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    for (auto* label : {&driveLabel, &levelLabel, &toneLabel})
        addAndMakeVisible(label);
    for (auto* knob : {&driveKnob, &levelKnob, &toneKnob})
    {
        knob->setMouseDragSensitivity(350);
        addAndMakeVisible(knob);
    }
    toneKnob.setMouseDragSensitivity(250);
    odModeLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(odModeLabel);

    AAUITools::setToggleImageButton(odModeSwitch, switchUpDrawable, switchDownDrawable, switchDownDrawable, switchDownDrawable);
    addAndMakeVisible(odModeSwitch);

    odModeSwitch.onClick = [this]
    {
        updateOdModeLabel();
        updateModeColours();
    };
    updateOdModeLabel();

    addAndMakeVisible(bypassLED);
    AAUITools::setToggleImageButton(bypassFootswitch, footswitchUpDrawable, footswitchUpDrawable, footswitchDownDrawable, footswitchDownDrawable);
    addAndMakeVisible(bypassFootswitch);

    addAndMakeVisible (resizeButton);
    updateModeColours();
    resizeButton.setToggleable (true);
    resizeButton.setClickingTogglesState (false);
    resizeButton.onClick = [this] { showUISizeOptions(); };

    addChildComponent (resizeOptionsPopup);
    resizeOptionsPopup.addComponentListener (this);
    resizeOptionsPopup.onSizeChanged = [this]
    {
        updateEditorSize();
        uiResize.makeCurrentSizeDefault();
    };

    addMouseListener (this, true);
    setResizable (false, false);
    updateEditorSize();
}

AmpPusherEditor::~AmpPusherEditor()
{
}

void AmpPusherEditor::paint (juce::Graphics& g)
{
    const auto boundsF = getLocalBounds().toFloat();
    g.fillAll (juce::Colours::black);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImage (pedalBoxImage, boundsF, juce::RectanglePlacement::fillDestination);
}

void AmpPusherEditor::resized()
{
    const auto boundsF = getLocalBounds().toFloat();
    const auto boundsFWidth = boundsF.getWidth();
    const auto boundsFHeight = boundsF.getHeight();

    const auto bigKnobSize = boundsFWidth * 0.33f;
    const auto bigKnobBounds = boundsF.withSize(bigKnobSize, bigKnobSize).withCentre({boundsF.getCentreX(), boundsFHeight * 0.125f});
    driveKnob.setBounds(bigKnobBounds.withX(boundsFWidth * 0.25f - bigKnobSize * 0.5f).toNearestInt());
    levelKnob.setBounds(bigKnobBounds.withX(boundsFWidth * 0.75f - bigKnobSize  * 0.5f).toNearestInt());

    const auto smallKnobSize = boundsFWidth * 0.22f;
    const auto smallKnobBounds = bigKnobBounds.withSizeKeepingCentre(smallKnobSize, smallKnobSize).translated(0.f, boundsFHeight * 0.15f);
    toneKnob.setBounds(smallKnobBounds.toNearestInt());

    const auto labelHeight = boundsFHeight * 0.05f;
    const auto labelFont = APFonts::arimoBold(labelHeight * 0.65f);
    const auto placeBelow = [&](AAUITools::PrintedText& label, const juce::Component& knob)
    {
        const auto knobBounds = knob.getBounds().toFloat();
        label.setFont(labelFont);
        label.setBounds(knobBounds.withY(knobBounds.getBottom()).withHeight(labelHeight).toNearestInt());
    };

    placeBelow(driveLabel, driveKnob);
    placeBelow(levelLabel, levelKnob);
    placeBelow(toneLabel, toneKnob);

    const juce::Point<float> modeLabelCentre {boundsFWidth * 0.5f, boundsFHeight * 0.873f};
    const auto modeLabelWidth = boundsFWidth * 0.91f;
    const auto modeLabelHeight = modeLabelWidth * static_cast<float>(modeLabel808Image.getHeight()) / static_cast<float>(modeLabel808Image.getWidth());
    odModeLabel.setBounds(boundsF.withSize(modeLabelWidth, modeLabelHeight).withCentre(modeLabelCentre).toNearestInt());

    const auto ledSize = boundsFWidth * 0.15f;
    const auto bypassLedBounds = boundsF.withSizeKeepingCentre(ledSize, ledSize);
    bypassLED.setBounds(bypassLedBounds.toNearestInt());
    const auto footswitchSize = boundsFWidth * 0.27f;
    bypassFootswitch.setBounds(bypassLedBounds.withSizeKeepingCentre(footswitchSize, footswitchSize).withY(bypassLedBounds.getBottom() + ledSize * 0.1f).toNearestInt());

    const auto switchSize = boundsFWidth * 0.225f;
    odModeSwitch.setBounds(boundsF.withSize(switchSize, switchSize).withCentre({boundsFWidth * 0.1f, boundsFHeight * 0.775f}).toNearestInt());

    const auto resizeButtonSize = boundsFHeight * 0.0375f;
    const auto resizeButtonGap = resizeButtonSize * 0.5f;
    const auto resizeButtonBounds = boundsF.withSize (resizeButtonSize, resizeButtonSize).withCentre ({boundsF.getRight() - resizeButtonGap, boundsF.getBottom() - resizeButtonGap});
    resizeButton.setBounds (resizeButtonBounds.toNearestInt());

    const auto sizeOptionsWidth = boundsFWidth * 0.45f;
    const auto sizeOptionsHeight = boundsFHeight * 0.04f * resizeOptionsPopup.getNumRows();
    resizeOptionsPopup.setBounds (juce::Rectangle<float> (sizeOptionsWidth, sizeOptionsHeight).withRightX (resizeButtonBounds.getRight()).withBottomY (resizeButtonBounds.getY()).toNearestInt());
}

void AmpPusherEditor::mouseUp (const juce::MouseEvent& event)
{
    const auto isClickInside = [&event] (const juce::Component& component)
    {
        return event.eventComponent == &component || component.isParentOf (event.eventComponent);
    };

    if (resizeOptionsPopup.isVisible() && ! isClickInside (resizeOptionsPopup) && ! isClickInside (resizeButton))
        resizeOptionsPopup.setVisible (false);
}

void AmpPusherEditor::componentVisibilityChanged (juce::Component& component)
{
    if (&component == &resizeOptionsPopup)
        resizeButton.setToggleState (resizeOptionsPopup.isVisible(), juce::dontSendNotification);
}

void AmpPusherEditor::updateOdModeLabel()
{
    odModeLabel.setImage(odModeSwitch.getToggleState() ? modeLabelS1DImage : modeLabel808Image);
}

void AmpPusherEditor::updateModeColours()
{
    const auto modeColour = odModeSwitch.getToggleState() ? APColors::ledYellow : APColors::ledGreen;
    AAUITools::setIconButtonImages (resizeButton, BinaryData::resizeLines_svg, BinaryData::resizeLines_svgSize, APColors::iconSVGColor,
                                    APColors::iconNormalColor.withAlpha (0.9f), APColors::iconSVGColor, APColors::iconOverColor, modeColour);
    resizeOptionsPopup.setSelectedTextColour (modeColour);
}

void AmpPusherEditor::updateEditorSize()
{
    const auto editorSize = uiResize.getEditorSize (getScreenBounds(), 0.0f);
    setSize (editorSize.getWidth(), editorSize.getHeight());
}

void AmpPusherEditor::showUISizeOptions()
{
    if (resizeOptionsPopup.isVisible())
    {
        resizeOptionsPopup.setVisible (false);
        return;
    }

    resizeOptionsPopup.updateItems();
    resized();
    resizeOptionsPopup.toFront (false);
    resizeOptionsPopup.setVisible (true);
}
