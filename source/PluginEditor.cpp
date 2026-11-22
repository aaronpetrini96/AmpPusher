#include "PluginProcessor.h"
#include "PluginEditor.h"

AmpPusherEditor::AmpPusherEditor (AmpPusherProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    for (auto* label : {&driveLabel, &levelLabel, &toneLabel})
        addAndMakeVisible(label);
    for (auto* knob : {&driveKnob, &levelKnob, &toneKnob})
    {
        knob->setMouseDragSensitivity(250);
        addAndMakeVisible(knob);
    }
    // toneKnob.setMouseDragSensitivity(250);
    odModeLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(odModeLabel);

    AAUITools::setToggleImageButton(odModeSwitch, switchUpDrawable, switchUpDrawable, switchDownDrawable, switchDownDrawable);
    addAndMakeVisible(odModeSwitch);

    odModeSwitch.onClick = [this] { updateOdModeLabel(); };
    updateOdModeLabel();

    addAndMakeVisible(bypassLED);
    AAUITools::setToggleImageButton(bypassFootswitch, footswitchDownDrawable, footswitchUpDrawable, footswitchUpDrawable, footswitchDownDrawable);
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

    const auto bigKnobSize = boundsFWidth * 0.35f;
    const auto bigKnobBounds = boundsF.withSize(bigKnobSize, bigKnobSize).withCentre({boundsF.getCentreX(), boundsFHeight * 0.15f});
    driveKnob.setBounds(bigKnobBounds.withX(boundsFWidth * 0.25f - bigKnobSize * 0.5f).toNearestInt());
    levelKnob.setBounds(bigKnobBounds.withX(boundsFWidth * 0.75f - bigKnobSize  * 0.5f).toNearestInt());

    const auto smallKnobSize = boundsFWidth * 0.215f;
    const auto smallKnobBounds = boundsF.withSizeKeepingCentre(smallKnobSize, smallKnobSize).withY(bigKnobBounds.getCentreY() + bigKnobSize * 0.5f);
    toneKnob.setBounds(smallKnobBounds.toNearestInt());

    const auto labelHeight = boundsFHeight * 0.0537f;
    const auto labelFont = APFonts::arimoBold(labelHeight * 0.7f);
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

    const auto ledSize = boundsFWidth * 0.17f;
    bypassLED.setBounds(boundsF.withSizeKeepingCentre(ledSize, ledSize).toNearestInt());
    const auto footswitchSize = boundsFWidth * 0.27f;
    bypassFootswitch.setBounds(boundsF.withSizeKeepingCentre(footswitchSize, footswitchSize).translated(0, ledSize * 1.25f).toNearestInt());

    const auto switchSize = boundsFWidth * 0.25f;
    odModeSwitch.setBounds(boundsF.withSize(switchSize, switchSize).withCentre({boundsFWidth * 0.1f, boundsFHeight * 0.775f}).toNearestInt());
}

void AmpPusherEditor::updateOdModeLabel()
{
    odModeLabel.setImage(odModeSwitch.getToggleState() ? modeLabelS1DImage : modeLabel808Image);
}
