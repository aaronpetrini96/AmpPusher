#include "PluginProcessor.h"
#include "PluginEditor.h"

AmpPusherEditor::AmpPusherEditor (AmpPusherProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (400, 300);
}

AmpPusherEditor::~AmpPusherEditor()
{
}

void AmpPusherEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AmpPusherEditor::resized()
{
}
