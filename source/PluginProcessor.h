#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <RTNeural/RTNeural.h>
#include "Utils/AAParamUtils.h"

class AmpPusherProcessor : public juce::AudioProcessor
{
public:
    AmpPusherProcessor();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override { return new juce::GenericAudioProcessorEditor (*this); }
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "AmpPusher"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

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
    using PedalModel = RTNeural::ModelT<float, 3, 1,
                                        RTNeural::GRULayerT<float, 3, 32>,
                                        RTNeural::DenseT<float, 32, 1>>;

    std::array<PedalModel, 2> pedalModels {};
    juce::SmoothedValue<float> smoothedDrive {0.0f};
    juce::SmoothedValue<float> smoothedLevelGain {1.0f};
    juce::SmoothedValue<float> smoothedBright {0.0f};

    juce::AudioParameterFloat* driveParam {nullptr};
    juce::AudioParameterBool* brightParam {nullptr};
    juce::AudioParameterFloat* levelParam {nullptr};
    juce::AudioParameterChoice* odModeParam {nullptr};

    static float levelToGain (float level);

    template <typename ParamType>
    ParamType* getParam (const juce::String& paramID)
    {
        auto* param = dynamic_cast<ParamType*> (parameters.getParameter (paramID));
        jassert (param != nullptr);
        return param;
    }

    void linkParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AmpPusherProcessor)
};
