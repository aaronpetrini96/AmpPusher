#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

class FixedRateResampler
{
public:
    void prepare (double hostSampleRate, double modelSampleRate, int maxHostBlockSizeToUse, int numChannels)
    {
        jassert (numChannels <= maxChannels);
        hostToModelRatio = hostSampleRate / modelSampleRate;
        needed = ! juce::approximatelyEqual (hostSampleRate, modelSampleRate);
        hostIsFaster = hostSampleRate > modelSampleRate;
        maxHostBlockSize = maxHostBlockSizeToUse;

        maxModelBlockSize = static_cast<int> (std::ceil (maxHostBlockSize / hostToModelRatio)) + inputMargin;
        const auto fifoCapacity = maxHostBlockSize + maxModelBlockSize + outputHeadStart + 16;
        inputFifo.setSize (numChannels, fifoCapacity);
        outputFifo.setSize (numChannels, fifoCapacity);
        modelBuffer.setSize (numChannels, maxModelBlockSize);

        const auto fasterRate = juce::jmax (hostSampleRate, modelSampleRate);
        const auto cutoff = 0.45 * juce::jmin (hostSampleRate, modelSampleRate);
        for (auto& filters : antiAliasFilters)
        {
            filters[0].setCoefficients (juce::IIRCoefficients::makeLowPass (fasterRate, cutoff, 0.5412));
            filters[1].setCoefficients (juce::IIRCoefficients::makeLowPass (fasterRate, cutoff, 1.3066));
        }

        reset();
    }

    void reset()
    {
        for (auto& interpolator : toModelRate)
            interpolator.reset();
        for (auto& interpolator : toHostRate)
            interpolator.reset();
        for (auto& filters : antiAliasFilters)
            for (auto& filter : filters)
                filter.reset();

        inputFifo.clear();
        outputFifo.clear();
        inputCount = 0;
        outputCount = outputHeadStart;
    }

    bool isNeeded() const { return needed; }

    int getLatencySamples() const
    {
        if (! needed)
            return 0;

        const auto interpolatorLatency = juce::LagrangeInterpolator::getBaseLatency();
        const auto modelSideLatency = (static_cast<float> (outputHeadStart) + interpolatorLatency) * static_cast<float> (hostToModelRatio);
        return juce::roundToInt (static_cast<float> (inputMargin) + interpolatorLatency + modelSideLatency);
    }

    template <typename ModelCallback>
    void process (juce::AudioBuffer<float>& buffer, ModelCallback&& processAtModelRate)
    {
        for (int start = 0; start < buffer.getNumSamples(); start += maxHostBlockSize)
        {
            const auto numSamples = juce::jmin (maxHostBlockSize, buffer.getNumSamples() - start);
            juce::AudioBuffer<float> chunk (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), start, numSamples);
            processChunk (chunk, processAtModelRate);
        }
    }

private:
    template <typename ModelCallback>
    void processChunk (juce::AudioBuffer<float>& buffer, ModelCallback& processAtModelRate)
    {
        const auto numChannels = juce::jmin (buffer.getNumChannels(), modelBuffer.getNumChannels());
        const auto numHostSamples = buffer.getNumSamples();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            inputFifo.copyFrom (ch, inputCount, buffer, ch, 0, numHostSamples);
            if (hostIsFaster)
                antiAlias (ch, inputFifo.getWritePointer (ch, inputCount), numHostSamples);
        }
        inputCount += numHostSamples;

        const auto numModelSamples = juce::jlimit (0, maxModelBlockSize, static_cast<int> ((inputCount - inputMargin) / hostToModelRatio));
        int usedHostSamples = 0;
        for (int ch = 0; ch < numChannels; ++ch)
            usedHostSamples = toModelRate[static_cast<size_t> (ch)].process (hostToModelRatio, inputFifo.getReadPointer (ch), modelBuffer.getWritePointer (ch), numModelSamples);
        dropFront (inputFifo, inputCount, usedHostSamples, numChannels);

        processAtModelRate (modelBuffer, numModelSamples);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            outputFifo.copyFrom (ch, outputCount, modelBuffer, ch, 0, numModelSamples);
            if (! hostIsFaster)
                antiAlias (ch, outputFifo.getWritePointer (ch, outputCount), numModelSamples);
        }
        outputCount += numModelSamples;

        int usedModelSamples = 0;
        for (int ch = 0; ch < numChannels; ++ch)
            usedModelSamples = toHostRate[static_cast<size_t> (ch)].process (1.0 / hostToModelRatio, outputFifo.getReadPointer (ch), buffer.getWritePointer (ch), numHostSamples);
        jassert (usedModelSamples <= outputCount);
        dropFront (outputFifo, outputCount, usedModelSamples, numChannels);
    }

    void antiAlias (int ch, float* samples, int numSamples)
    {
        for (auto& filter : antiAliasFilters[static_cast<size_t> (ch)])
            filter.processSamples (samples, numSamples);
    }

    static void dropFront (juce::AudioBuffer<float>& fifo, int& count, int numToDrop, int numChannels)
    {
        count -= numToDrop;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* data = fifo.getWritePointer (ch);
            std::memmove (data, data + numToDrop, static_cast<size_t> (count) * sizeof (float));
        }
    }

    static constexpr int maxChannels {2};
    static constexpr int inputMargin {2};
    static constexpr int outputHeadStart {8};

    std::array<juce::LagrangeInterpolator, maxChannels> toModelRate;
    std::array<juce::LagrangeInterpolator, maxChannels> toHostRate;
    std::array<std::array<juce::IIRFilter, 2>, maxChannels> antiAliasFilters;
    juce::AudioBuffer<float> inputFifo, outputFifo, modelBuffer;
    double hostToModelRatio {1.0};
    int maxHostBlockSize {0};
    int maxModelBlockSize {0};
    int inputCount {0};
    int outputCount {0};
    bool needed {false};
    bool hostIsFaster {false};
};
