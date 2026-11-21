#include "PluginProcessor.h"
#include "PluginEditor.h"

AmpPusherEditor::AmpPusherEditor (AmpPusherProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    for (auto* knob : {&driveKnob, &levelKnob})
        addAndMakeVisible(knob);

    setSize (340, 600);
}

AmpPusherEditor::~AmpPusherEditor()
{
}

void AmpPusherEditor::paint (juce::Graphics& g)
{
    const auto boundsF = getLocalBounds().toFloat();
    const auto boundsFWidth = boundsF.getWidth();
    const auto boundsFHeight = boundsF.getHeight();
    g.fillAll (juce::Colours::black);
    g.drawImage (pedalBoxImage, boundsF, juce::RectanglePlacement::fillDestination);
}

void AmpPusherEditor::resized()
{
    const auto boundsF = getLocalBounds().toFloat();
    const auto boundsFWidth = boundsF.getWidth();
    const auto boundsFHeight = boundsF.getHeight();

    const auto bigKnobSize = boundsFWidth * 0.375f;
    const auto bigKnobBounds = boundsF.withSize(bigKnobSize, bigKnobSize).withCentre({boundsF.getCentreX(), boundsFHeight * 0.175f});
    const auto knobXGap = bigKnobSize * 0.6f;
    driveKnob.setBounds(bigKnobBounds.translated(-knobXGap, 0.f).toNearestInt());
    levelKnob.setBounds(bigKnobBounds.translated(knobXGap, 0.f).toNearestInt());
}
