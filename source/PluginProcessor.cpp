#include "PluginProcessor.h"
#include <BinaryData.h>

AmpPusherProcessor::AmpPusherProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    linkParameters();
    loadModels (models808, BinaryData::ts808_json, BinaryData::ts808_jsonSize);
    loadModels (modelsS1D, BinaryData::sd1_json, BinaryData::sd1_jsonSize);
}

void AmpPusherProcessor::prepareToPlay (double sampleRate, int)
{
    for (auto* models : {&models808, &modelsS1D})
        for (auto& model : *models)
            model.reset();
    activeODMode = odModeParam->getIndex();
    constexpr double smoothingTime = 0.05;
    smoothedDrive.reset (sampleRate, smoothingTime);
    smoothedDrive.setCurrentAndTargetValue (driveParam->get());
    smoothedBright.reset(sampleRate, smoothingTime);
    smoothedBright.setCurrentAndTargetValue (brightParam->get() ? 1.0f : 0.0f);
    smoothedLevelGain.reset (sampleRate, smoothingTime);
    smoothedLevelGain.setCurrentAndTargetValue (levelToGain (levelParam->get(), activeODMode));
}

void AmpPusherProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    smoothedDrive.setTargetValue(driveParam->get());
    smoothedBright.setTargetValue(brightParam->get() ? 1.0f : 0.0f);
    
    const auto odModeIndex = odModeParam->getIndex();
    auto& activeModels = odModeIndex == 0 ? models808 : modelsS1D;
    
    if (activeODMode != odModeIndex) 
    {
        for (auto& model : activeModels)
            model.reset();
        activeODMode = odModeIndex;
    }
    smoothedLevelGain.setTargetValue(levelToGain(levelParam->get(), odModeIndex));

    const int numChannels = std::min(buffer.getNumChannels(), static_cast<int>(activeModels.size()));

    for (int n = 0; n < buffer.getNumSamples(); ++n) {
        const float drive = smoothedDrive.getNextValue();
        const float bright = smoothedBright.getNextValue();
        const float level = smoothedLevelGain.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch) {
            auto* channelData = buffer.getWritePointer(ch);
            const float modelInput[3] {channelData[n], drive, bright};
            channelData[n] = activeModels[static_cast<size_t>(ch)].forward (modelInput) * level;
        }
    }
}

void AmpPusherProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AmpPusherProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

float AmpPusherProcessor::levelToGain (float level, int odMode)
{
    constexpr std::array<float, 5> decibelsAtKnob808 {-100.f, -32.f, 0.f, 7.5f, 12.f};
    constexpr std::array<float, 5> decibelsAtKnobS1D {-100.f, 6.6f, 15.f, 19.2f, 21.f};
    const auto& decibelsAtKnob = odMode == 0 ? decibelsAtKnob808 : decibelsAtKnobS1D;
    const auto position = juce::jlimit(0.f, 4.f, level / 2.5f);
    const auto lowerIndex = static_cast<size_t>(std::min(position, 3.f));
    const auto fraction = position - static_cast<float>(lowerIndex);
    const auto decibels = juce::jmap(fraction, decibelsAtKnob[lowerIndex], decibelsAtKnob[lowerIndex + 1]);
    return juce::Decibels::decibelsToGain(decibels);
}

juce::AudioProcessorValueTreeState::ParameterLayout AmpPusherProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    constexpr auto versionHint {1};

    auto names = ParamNames::getDriveNames();
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (names.id, versionHint), names.name, 0.0f, 1.0f, 0.5f));
    names = ParamNames::getBrightNames();
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID (names.id, versionHint), names.name, false));
    names = ParamNames::getLevelNames();
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (names.id, versionHint), names.name, 0.0f, 10.0f, 5.0f));
    names = ParamNames::getODModeNames();
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID (names.id, versionHint), names.name, juce::StringArray { "Green 808", "Yellow S1D" }, 0));
    return layout;
}

void AmpPusherProcessor::linkParameters()
{
    driveParam = getParam<juce::AudioParameterFloat> (ParamNames::getDriveNames().id);
    brightParam = getParam<juce::AudioParameterBool> (ParamNames::getBrightNames().id);
    levelParam = getParam<juce::AudioParameterFloat> (ParamNames::getLevelNames().id);
    odModeParam = getParam<juce::AudioParameterChoice> (ParamNames::getODModeNames().id);
}

void AmpPusherProcessor::loadModels (std::array<PedalModel, 2>& models, const char* jsonData, int jsonSize)
{
    const auto weights = nlohmann::json::parse (jsonData, jsonData + jsonSize);

    for (auto& pedalModel : models)
    {
        RTNeural::torch_helpers::loadGRU<float> (weights, "gru.", pedalModel.get<0>());
        RTNeural::torch_helpers::loadDense<float> (weights, "output.", pedalModel.get<1>());
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpPusherProcessor();
}