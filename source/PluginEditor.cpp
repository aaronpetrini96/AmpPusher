#include "PluginProcessor.h"
#include "PluginEditor.h"

AmpPusherEditor::AmpPusherEditor (AmpPusherProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    for (auto* knob : {&driveKnob, &levelKnob, &toneKnob})
    {
        knob->setMouseDragSensitivity(300);
        addAndMakeVisible(knob);
    }

    toneKnob.setMouseDragSensitivity(250);

    for (auto* label : {&driveLabel, &levelLabel, &toneLabel})
        addAndMakeVisible(label);

    odModeLabelAttachment.sendInitialUpdate();
    addAndMakeVisible(odModeLabel);

    odModeSwitch.setImages(false, true, true,
                           switchDownImage, 1.f, juce::Colours::transparentBlack,
                           switchDownImage, 1.f, juce::Colours::transparentBlack,
                           switchUpImage, 1.f, juce::Colours::transparentBlack);
    odModeSwitch.setClickingTogglesState(true);
    addAndMakeVisible(odModeSwitch);
    addAndMakeVisible(bypassLED);

    juce::DrawableImage footswitchDownDrawable {footswitchDownImage};
    juce::DrawableImage footswitchUpDrawable {footswitchUpImage};
    bypassFootswitch.setImages(&footswitchDownDrawable, &footswitchDownDrawable, &footswitchUpDrawable, nullptr,
                               &footswitchUpDrawable, &footswitchUpDrawable, &footswitchDownDrawable);
    bypassFootswitch.setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentBlack);
    bypassFootswitch.setColour(juce::DrawableButton::backgroundOnColourId, juce::Colours::transparentBlack);
    bypassFootswitch.setClickingTogglesState(true);
    bypassFootswitch.setTriggeredOnMouseDown(true);
    addAndMakeVisible(bypassFootswitch);

    setResizable(true, true);
    setResizeLimits(uiWidth * 3/4, uiHeight * 3/4, uiWidth * 3/2, uiHeight * 3/2);
    getConstrainer()->setFixedAspectRatio(static_cast<double>(uiWidth)/ static_cast<double>(uiHeight));
    setSize (uiWidth, uiHeight);
}

AmpPusherEditor::~AmpPusherEditor()
{
}

void AmpPusherEditor::paint (juce::Graphics& g)
{
    const auto boundsF = getLocalBounds().toFloat();
    g.fillAll (juce::Colours::black);
    g.drawImage (pedalBoxImage, boundsF, juce::RectanglePlacement::fillDestination);
}

void AmpPusherEditor::resized()
{
    const auto boundsF = getLocalBounds().toFloat();
    const auto boundsFWidth = boundsF.getWidth();
    const auto boundsFHeight = boundsF.getHeight();

    const auto bigKnobSize = boundsFWidth * 0.4f;
    const auto bigKnobBounds = boundsF.withSize(bigKnobSize, bigKnobSize).withCentre({boundsF.getCentreX(), boundsFHeight * 0.2f});
    const auto knobXGap = boundsFWidth * 0.225f;
    driveKnob.setBounds(bigKnobBounds.translated(-knobXGap, 0.f).toNearestInt());
    levelKnob.setBounds(bigKnobBounds.translated(knobXGap, 0.f).toNearestInt());

    const auto smallKnobSize = boundsFWidth * 0.185f;
    const auto smallKnobBounds = boundsF.withSizeKeepingCentre(smallKnobSize, smallKnobSize).withY(bigKnobBounds.getCentreY() + bigKnobSize * 0.55f);
    toneKnob.setBounds(smallKnobBounds.toNearestInt());

    const auto labelHeight = boundsFHeight * 0.05f;
    const auto labelFont = APFonts::arimoBold(labelHeight * 0.725f);
    const auto placeBelow = [&](AAUITools::PrintedText& label, const juce::Component& knob)
    {
        const auto knobBounds = knob.getBounds().toFloat();
        label.setFont(labelFont);
        label.setBounds(knobBounds.withY(knobBounds.getBottom()).withHeight(labelHeight).toNearestInt());
    };

    placeBelow(driveLabel, driveKnob);
    placeBelow(levelLabel, levelKnob);
    placeBelow(toneLabel, toneKnob);

    const auto tapeTextHeight = boundsFHeight * 0.17f;
    const juce::Point<float> tapeCentre {boundsFWidth * 0.5f, boundsFHeight * 0.8475f};
    const auto tapeFont = APFonts::permanentMarker(tapeTextHeight);
    const auto tapeTextWidth = juce::jmax(juce::GlyphArrangement::getStringWidth(tapeFont, "808"), juce::GlyphArrangement::getStringWidth(tapeFont, "S1D"));
    odModeLabel.setFont(tapeFont);
    odModeLabel.setBounds(boundsF.withSize(tapeTextWidth, tapeTextHeight * 1.4f).withCentre(tapeCentre).toNearestInt());
    odModeLabel.setTransform(juce::AffineTransform::rotation(-0.174f, tapeCentre.x, tapeCentre.y));

    const auto switchSize = boundsFWidth * 0.225f;
    odModeSwitch.setBounds(boundsF.withSize(switchSize, switchSize).withCentre({boundsFWidth * 0.15f, boundsFHeight * 0.725f}).toNearestInt());

    const auto ledSize = boundsFWidth * 0.125f;
    bypassLED.setBounds(boundsF.withSize(ledSize, ledSize).withCentre({boundsF.getCentreX(), boundsFHeight * 0.085f}).toNearestInt());

    const auto footswitchSize = boundsFWidth * 0.25f;
    bypassFootswitch.setBounds(boundsF.withSize(footswitchSize, footswitchSize).withCentre({boundsF.getCentreX(), boundsFHeight * 0.625f}).toNearestInt());
}
