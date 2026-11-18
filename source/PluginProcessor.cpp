#include "PluginProcessor.h"
#include <BinaryData.h>

AmpPusherProcessor::AmpPusherProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    linkParameters();

    const auto* modelJsonBegin = BinaryData::ts808_json;
    const auto* modelJsonEnd = modelJsonBegin + BinaryData::ts808_jsonSize;
    const auto weights = nlohmann::json::parse (modelJsonBegin, modelJsonEnd);

    for (auto& pedalModel : pedalModels)
    {
        RTNeural::torch_helpers::loadGRU<float> (weights, "gru.", pedalModel.get<0>());
        RTNeural::torch_helpers::loadDense<float> (weights, "output.", pedalModel.get<1>());
    }
}

void AmpPusherProcessor::prepareToPlay (double sampleRate, int)
{
    for (auto& pedalModel : pedalModels)
        pedalModel.reset();
    constexpr double smoothingTime = 0.05;
    smoothedDrive.reset (sampleRate, smoothingTime);
    smoothedDrive.setCurrentAndTargetValue (driveParam->get());
    smoothedBright.reset(sampleRate, smoothingTime);
    smoothedBright.setCurrentAndTargetValue (brightParam->get() ? 1.0f : 0.0f);
    smoothedLevelGain.reset (sampleRate, smoothingTime);
    smoothedLevelGain.setCurrentAndTargetValue (levelToGain (levelParam->get()));
}

void AmpPusherProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    smoothedDrive.setTargetValue(driveParam->get());
    smoothedBright.setTargetValue(brightParam->get() ? 1.0f : 0.0f);
    smoothedLevelGain.setTargetValue(levelToGain(levelParam->get()));

    const int numChannels = std::min(buffer.getNumChannels(), static_cast<int>(pedalModels.size()));

    for (int n = 0; n < buffer.getNumSamples(); ++n) {
        const float drive = smoothedDrive.getNextValue();
        const float bright = smoothedBright.getNextValue();
        const float level = smoothedLevelGain.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch) {
            auto* channelData = buffer.getWritePointer(ch);
            const float modelInput[3] {channelData[n], drive, bright};
            channelData[n] = pedalModels[static_cast<size_t>(ch)].forward (modelInput) * level;
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

float AmpPusherProcessor::levelToGain (float level)
{
    constexpr  std::array<float, 5> decibelsAtKnob {-100.f, -32.f, 0.f, 7.5f, 12.f};
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

    const auto drive = ParamNames::getDriveNames();
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (drive.id, versionHint), drive.name, 0.0f, 1.0f, 0.5f));
    const auto bright = ParamNames::getBrightNames();
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID (bright.id, versionHint), bright.name, false));
    const auto level = ParamNames::getLevelNames();
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (level.id, versionHint), level.name, 0.0f, 10.0f, 5.0f));
    return layout;
}

void AmpPusherProcessor::linkParameters()
{
    driveParam = getParam<juce::AudioParameterFloat> (ParamNames::getDriveNames().id);
    brightParam = getParam<juce::AudioParameterBool> (ParamNames::getBrightNames().id);
    levelParam = getParam<juce::AudioParameterFloat> (ParamNames::getLevelNames().id);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AmpPusherProcessor();
}