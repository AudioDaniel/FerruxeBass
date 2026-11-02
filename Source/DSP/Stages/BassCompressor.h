#pragma once

#include "../BiquadFilter.h"
#include <array>
#include <cmath>

namespace ferruxe::dsp
{

/**
 * Stage 1 (Pre-Distortion): Subsonic rumble filter + studio bass compressor.
 *
 * Evens out pick/finger dynamics before hitting saturation so loud open-E plucks
 * don't blow out the clipper while lighter higher-string notes stay punchy.
 */
class BassCompressor
{
public:
    static constexpr size_t MaxChannels       = 2;
    static constexpr float  SubsonicCutHz     = 28.0f;
    static constexpr float  Ratio             = 4.0f;
    static constexpr float  CompressionSlope  = 1.0f - (1.0f / Ratio); // 0.75 for 4:1
    static constexpr float  SoftKneeWidthDb   = 6.0f;
    static constexpr float  AttackSeconds     = 0.018f; // 18 ms: lets pick/finger attack pass through
    static constexpr float  ReleaseSeconds    = 0.110f; // 110 ms: recovers cleanly between 8th/16th notes
    static constexpr float  ReferenceDiDb     = -14.0f;
    static constexpr float  MaxAutoMakeupDb   = 8.5f;

    void prepare (double sampleRate) noexcept
    {
        subsonicHpf.configureHighPass (sampleRate, SubsonicCutHz);

        // Detector-only sidechain filters: prevent heavy low B/E fundamentals from
        // compressing 8 dB harder than A/D/G strings (does not alter audio frequency response).
        sidechainHpf.configureHighPass (sampleRate, 38.0f);
        sidechainLowTilt.configureLowShelf (sampleRate, 95.0f, BiquadFilter::ButterworthQ, -4.5f);

        attackCoeff  = static_cast<float> (std::exp (-1.0 / (static_cast<double> (AttackSeconds)  * sampleRate)));
        releaseCoeff = static_cast<float> (std::exp (-1.0 / (static_cast<double> (ReleaseSeconds) * sampleRate)));

        reset();
    }

    void reset() noexcept
    {
        subsonicHpf.reset();
        sidechainHpf.reset();
        sidechainLowTilt.reset();
        envelopeGainReductionDb = 0.0f;
    }

    /** Processes an entire audio block in-place and returns peak gain reduction in positive dB. */
    float processBlock (juce::AudioBuffer<float>& buffer,
                        size_t numChannels,
                        juce::SmoothedValue<float>& inputGainSmoothed,
                        juce::SmoothedValue<float>& thresholdDbSmoothed) noexcept
    {
        const int numSamples = buffer.getNumSamples();
        std::array<float*, MaxChannels> channelPtrs { nullptr, nullptr };

        for (size_t ch = 0; ch < numChannels; ++ch)
            channelPtrs[ch] = buffer.getWritePointer (static_cast<int> (ch));

        float peakGrDb = 0.0f;

        for (int sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
        {
            const float inputGain   = inputGainSmoothed.getNextValue();
            const float thresholdDb = thresholdDbSmoothed.getNextValue();

            float detectorPeak = 0.0f;
            std::array<float, MaxChannels> filteredInput { 0.0f, 0.0f };

            for (size_t ch = 0; ch < numChannels; ++ch)
            {
                const float cleanSample = subsonicHpf.processSample (ch, channelPtrs[ch][sampleIdx] * inputGain);
                filteredInput[ch] = cleanSample;

                const float detectorSample = sidechainLowTilt.processSample (
                    ch, sidechainHpf.processSample (ch, cleanSample));
                detectorPeak = juce::jmax (detectorPeak, std::abs (detectorSample));
            }

            const float detectorDb = juce::Decibels::gainToDecibels (detectorPeak, -96.0f);
            const float targetGrDb = computeSoftKneeGainReductionDb (detectorDb, thresholdDb);

            const float coeff = (targetGrDb > envelopeGainReductionDb) ? attackCoeff : releaseCoeff;
            envelopeGainReductionDb = coeff * envelopeGainReductionDb + (1.0f - coeff) * targetGrDb;
            peakGrDb = juce::jmax (peakGrDb, envelopeGainReductionDb);

            const float autoMakeupDb = juce::jmin (MaxAutoMakeupDb,
                                                   0.72f * computeSoftKneeGainReductionDb (ReferenceDiDb, thresholdDb));
            const float netLinearGain = juce::Decibels::decibelsToGain (autoMakeupDb - envelopeGainReductionDb);

            for (size_t ch = 0; ch < numChannels; ++ch)
                channelPtrs[ch][sampleIdx] = filteredInput[ch] * netLinearGain;
        }

        return peakGrDb;
    }

private:
    [[nodiscard]] static float computeSoftKneeGainReductionDb (float inputDb, float thresholdDb) noexcept
    {
        constexpr float halfKnee = SoftKneeWidthDb * 0.5f;
        const float overshootDb = inputDb - thresholdDb;

        if (overshootDb <= -halfKnee)
            return 0.0f;

        if (overshootDb < halfKnee)
        {
            const float kneePos = overshootDb + halfKnee;
            return CompressionSlope * (kneePos * kneePos) / (2.0f * SoftKneeWidthDb);
        }

        return CompressionSlope * overshootDb;
    }

    StereoBiquadFilter<MaxChannels> subsonicHpf;
    StereoBiquadFilter<MaxChannels> sidechainHpf;
    StereoBiquadFilter<MaxChannels> sidechainLowTilt;

    float attackCoeff             = 0.0f;
    float releaseCoeff            = 0.0f;
    float envelopeGainReductionDb = 0.0f;
};

} // namespace ferruxe::dsp
