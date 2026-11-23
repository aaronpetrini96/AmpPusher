#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <RTNeural/RTNeural.h>
#include "Utils/AAParamUtils.h"
#include "DSP/FixedRateResampler.h"
#include "Utils/AASmoothedBypassed.h"

class AmpPusherProcessor : public juce::AudioProcessor
{
public:
    AmpPusherProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AmpPusher"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    juce::AudioProcessorParameter* getBypassParameter() const override { return bypassParam; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState parameters {*this, nullptr, "Parameters", createParameterLayout()};

private:
    using PedalModel = RTNeural::ModelT<float, 3, 1, RTNeural::GRULayerT<float, 3, 32>, RTNeural::DenseT<float, 32, 1>>;



    std::array<PedalModel, 2> models808;
    std::array<PedalModel, 2> modelsS1D;
    int activeODMode {0};
    juce::SmoothedValue<float> switchCrossfade {1.0f};
    static constexpr double modelSampleRate {48000.0};
    FixedRateResampler resampler;
    AASmoothedBypassed smoothedBypass;

    juce::SmoothedValue<float> smoothedDrive {0.0f};
    juce::SmoothedValue<float> smoothedGain808 {1.0f};
    juce::SmoothedValue<float> smoothedGainS1D {1.0f};
    juce::SmoothedValue<float> smoothedTone {0.5f};

    juce::AudioParameterFloat* driveParam {nullptr};
    juce::AudioParameterFloat* toneParam {nullptr};
    juce::AudioParameterFloat* levelParam {nullptr};
    juce::AudioParameterChoice* odModeParam {nullptr};
    juce::AudioParameterBool* bypassParam {nullptr};

    static float levelToGain (float level, int odMode);
    static void loadModels (std::array<PedalModel, 2>& models, const char* jsonData, int jsonSize);

    template <typename ParamType>
    ParamType* getParam (const juce::String& paramID)
    {
        auto* param = dynamic_cast<ParamType*> (parameters.getParameter (paramID));
        jassert (param != nullptr);
        return param;
    }

    void linkParameters();
    void resetModels();
    void processModels (juce::AudioBuffer<float>& buffer, int numSamples);

    enum class SmootherUpdateMode
    {
        initialize,
        liveInRealTime
    };

    void updateSmoothersFromParams (SmootherUpdateMode mode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherProcessor)
};
