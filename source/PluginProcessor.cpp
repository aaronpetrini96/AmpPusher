#include "PluginProcessor.h"
#include "PluginEditor.h"
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

void AmpPusherProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    resampler.prepare (sampleRate, modelSampleRate, samplesPerBlock, getTotalNumOutputChannels());
    setLatencySamples (resampler.getLatencySamples());
    const auto processingRate = resampler.isNeeded() ? modelSampleRate : sampleRate;

    switchCrossfade.reset (processingRate, 0.02);
    resetModels();
    smoothedEffectMix.reset (processingRate, 0.02);

    constexpr double smoothingTime = 0.05;
    for (auto* smoother : {&smoothedDrive, &smoothedTone, &smoothedGain808, &smoothedGainS1D})
        smoother->reset (processingRate, smoothingTime);

    updateSmoothersFromParams (SmootherUpdateMode::initialize);
}

void AmpPusherProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateSmoothersFromParams (SmootherUpdateMode::liveInRealTime);

    if (smoothedEffectMix.getCurrentValue() == 0.0f)
    {
        if (smoothedEffectMix.getTargetValue() == 0.0f)
        {
            if (resampler.isNeeded())
                resampler.process (buffer, [] (juce::AudioBuffer<float>&, int) {});
            return;
        }
        resetModels();
    }

    if (resampler.isNeeded())
        resampler.process (buffer, [this] (juce::AudioBuffer<float>& modelRateBuffer, int numSamples) { processModels (modelRateBuffer, numSamples); });
    else
        processModels (buffer, buffer.getNumSamples());
}

bool AmpPusherProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainOutput = layouts.getMainOutputChannelSet();
    if (mainOutput != juce::AudioChannelSet::mono() && mainOutput != juce::AudioChannelSet::stereo())
        return false;

    return layouts.getMainInputChannelSet() == mainOutput;
}

juce::AudioProcessorEditor* AmpPusherProcessor::createEditor()
{
    return new AmpPusherEditor (*this);
}

void AmpPusherProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = parameters.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void AmpPusherProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (parameters.state.getType()))
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
    names = ParamNames::getToneNames();
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (names.id, versionHint), names.name, 0.0f, 1.0f, 0.5f));
    names = ParamNames::getLevelNames();
    layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID (names.id, versionHint), names.name, 0.0f, 10.0f, 5.0f));
    names = ParamNames::getODModeNames();
    layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID (names.id, versionHint), names.name, juce::StringArray { "Green 808", "Yellow S1D" }, 0));
    names = ParamNames::getBypassNames();
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID (names.id, versionHint), names.name, false));
    return layout;
}

void AmpPusherProcessor::linkParameters()
{
    driveParam = getParam<juce::AudioParameterFloat> (ParamNames::getDriveNames().id);
    toneParam = getParam<juce::AudioParameterFloat> (ParamNames::getToneNames().id);
    levelParam = getParam<juce::AudioParameterFloat> (ParamNames::getLevelNames().id);
    odModeParam = getParam<juce::AudioParameterChoice> (ParamNames::getODModeNames().id);
    bypassParam = getParam<juce::AudioParameterBool> (ParamNames::getBypassNames().id);
}

void AmpPusherProcessor::resetModels()
{
    for (auto* models : {&models808, &modelsS1D})
        for (auto& model : *models)
            model.reset();

    activeODMode = odModeParam->getIndex();
    switchCrossfade.setCurrentAndTargetValue (1.0f);
}

void AmpPusherProcessor::processModels (juce::AudioBuffer<float>& buffer, int numSamples)
{
    const auto odModeIndex = odModeParam->getIndex();
    auto& activeModels = odModeIndex == 0 ? models808 : modelsS1D;
    auto& previousModels = odModeIndex == 0 ? modelsS1D : models808;

    if (activeODMode != odModeIndex)
    {
        for (auto& model : activeModels)
            model.reset();
        activeODMode = odModeIndex;

        switchCrossfade.setCurrentAndTargetValue(0.0f);
        switchCrossfade.setTargetValue(1.0f);
    }

    const int numChannels = std::min(buffer.getNumChannels(), static_cast<int>(activeModels.size()));

    for (int n = 0; n < numSamples; ++n)
    {
        const float drive = smoothedDrive.getNextValue();
        const float tone = smoothedTone.getNextValue();
        const float gain808 = smoothedGain808.getNextValue();
        const float gainS1D = smoothedGainS1D.getNextValue();
        const float activeGain = odModeIndex == 0 ? gain808 : gainS1D;
        const float previousGain = odModeIndex == 0 ? gainS1D : gain808;
        const float switchCrossfadeValue = switchCrossfade.getNextValue();
        const float effectMix = smoothedEffectMix.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* channelData = buffer.getWritePointer(ch);
            const float drySample = channelData[n];
            const float modelInput[3] {drySample, drive, tone};
            float pedalOutput = activeModels[static_cast<size_t>(ch)].forward(modelInput) * activeGain;
            if (switchCrossfadeValue < 1.0f)
                pedalOutput = pedalOutput * switchCrossfadeValue + previousModels[static_cast<size_t>(ch)].forward(modelInput) * previousGain * (1.0f - switchCrossfadeValue);
            channelData[n] = pedalOutput * effectMix + drySample * (1.0f - effectMix);
        }
    }
}

void AmpPusherProcessor::updateSmoothersFromParams (SmootherUpdateMode mode)
{
    const auto update = [mode] (juce::SmoothedValue<float>& smoother, float value)
    {
        if (mode == SmootherUpdateMode::initialize)
            smoother.setCurrentAndTargetValue (value);
        else
            smoother.setTargetValue (value);
    };

    const float level = levelParam->get();
    update (smoothedDrive, driveParam->get());
    update (smoothedTone, toneParam->get());
    update (smoothedGain808, levelToGain (level, 0));
    update (smoothedGainS1D, levelToGain (level, 1));
    update (smoothedEffectMix, bypassParam->get() ? 0.0f : 1.0f);
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