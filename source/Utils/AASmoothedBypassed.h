//
// Created by Aaron Petrini on 23/11/2026.
//

#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <vector>

class AASmoothedBypassed
{
public:
    AASmoothedBypassed() = default;

    void prepare (double newSampleRate, int maxBlockSize, int numChannels, bool startBypassed, int latencySamples)
    {
        assert (newSampleRate > 0.0 && maxBlockSize > 0 && numChannels > 0 && latencySamples >= 0);

        maxChunkSize = std::max(1, maxBlockSize);
        const auto channelCount = static_cast<std::size_t>(numChannels);
        const auto delayLength = static_cast<std::size_t>(std::max(0, latencySamples)) + 1;

        dryChannels.assign(channelCount, std::vector<float>(static_cast<std::size_t>(maxChunkSize), 0.0f));
        delayChannels.assign(channelCount, std::vector<float>(delayLength, 0.0f));
        chunkChannels.assign(channelCount, nullptr);
        writePosition = 0;

        sampleRate = newSampleRate;
        updateStepPerSample();

        targetWet = startBypassed ? 0.f : 1.f;
        currentWet = targetWet;
    }

    void setRampSeconds (double newRampSeconds)
    {
        assert(newRampSeconds >= 0.0);
        rampSeconds = newRampSeconds;
        updateStepPerSample();
    }

    void setBypassed (bool shouldBeBypassed)
    {
        targetWet = shouldBeBypassed ? 0.f : 1.f;
    }

    bool isFullyBypassed() const
    {
        return currentWet == 0.f && targetWet == 0.f;
    }

    template <typename ProcessFunction>
    void process (float* const* channelsData, int numChannels, int numSamples, ProcessFunction&& processUnit)
    {
        assert (numChannels > 0 && numChannels <= static_cast<int>(chunkChannels.size()) && numSamples >= 0);

        const auto channelsToUse = std::min(numChannels, static_cast<int>(chunkChannels.size()));
        const auto delayLength = delayChannels.front().size();

        for (int chunkStart {0}; chunkStart < numSamples;)
        {
            const auto chunkSize = std::min(maxChunkSize, numSamples - chunkStart);

            for (int ch {0}; ch < channelsToUse; ++ch)
                chunkChannels[static_cast<std::size_t>(ch)] = channelsData[ch] + chunkStart;

            for (int n {0}; n < chunkSize; ++n)
            {
                auto readPosition = writePosition + 1;
                if (readPosition == delayLength)
                    readPosition = 0;

                for (int ch {0}; ch < channelsToUse; ++ch)
                {
                    const auto channelIndex = static_cast<std::size_t>(ch);
                    auto& delay = delayChannels[channelIndex];

                    delay[writePosition] = chunkChannels[channelIndex][n];
                    dryChannels[channelIndex][static_cast<std::size_t>(n)] = delay[readPosition];
                }

                writePosition = readPosition;
            }

            processUnit(chunkChannels.data(), channelsToUse, chunkSize);
            chunkStart += chunkSize;

            if (currentWet == 1.f && targetWet == 1.f)
                continue;

            for (int n {0}; n < chunkSize; ++n)
            {
                currentWet = currentWet < targetWet ? std::min(currentWet + stepPerSample, targetWet)
                                                    : std::max(currentWet - stepPerSample, targetWet);

                for (int ch {0}; ch < channelsToUse; ++ch)
                {
                    const auto channelIndex = static_cast<std::size_t>(ch);
                    const auto drySample = dryChannels[channelIndex][static_cast<std::size_t>(n)];
                    auto& sample = chunkChannels[channelIndex][n];

                    if (currentWet == 0.f)
                        sample = drySample;
                    else if (currentWet < 1.f)
                        sample = drySample + (sample - drySample) * currentWet;
                }
            }
        }
    }

private:
    void updateStepPerSample()
    {
        const auto rampSamples = std::max (1, static_cast<int> (sampleRate * rampSeconds));
        stepPerSample = 1.f / static_cast<float> (rampSamples);
    }

    std::vector<std::vector<float>> dryChannels;
    std::vector<std::vector<float>> delayChannels;
    std::vector<float*> chunkChannels;

    std::size_t writePosition {0};
    int maxChunkSize {1};
    float currentWet {1.f};
    float targetWet {1.f};
    float stepPerSample {0.f};
    double sampleRate {44100.0};
    double rampSeconds {0.015};
};
