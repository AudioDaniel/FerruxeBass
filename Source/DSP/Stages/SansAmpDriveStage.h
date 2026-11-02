#pragma once

#include "../BiquadFilter.h"
#include <cmath>

namespace ferruxe::dsp
{

/**
 * Stage 2b (High Band > 160 Hz): Tech 21 SansAmp Bass Driver DI-inspired saturation.
 *
 * Boosts upper-mid "Presence" clank into a two-stage asymmetric soft clipper, shapes
 * the result through an SVT + 8x10 cab contour, and blends in parallel with clean DI highs.
 */
class SansAmpDriveStage
{
public:
    static constexpr size_t MaxChannels        = 2;
    static constexpr float  PresenceFreqHz     = 2800.0f;
    static constexpr float  PresenceGainDb     = 8.0f;
    static constexpr float  InterStageHpfHz    = 85.0f;
    static constexpr float  MidScoopFreqHz     = 650.0f;
    static constexpr float  MidScoopGainDb     = -4.5f;
    static constexpr float  UpperMidBiteHz     = 2150.0f;
    static constexpr float  UpperMidBiteGainDb = 2.5f;
    static constexpr float  CabRollOffHz       = 5000.0f;
    static constexpr float  TubeBias           = 0.18f;

    /** Precomputed per-sample gain/mix coefficients shared across stereo channels. */
    struct FrameGains
    {
        float dryMix          = 1.0f;
        float wetMix          = 0.0f;
        float drivePreGain    = 1.2f;
        float autoMakeupScale = 1.0f;
    };

    void prepare (double oversampledRate) noexcept
    {
        prePresenceShelf.configureHighShelf (oversampledRate, PresenceFreqHz, 0.75f, PresenceGainDb);
        interStageHpf.configureHighPass (oversampledRate, InterStageHpfHz);

        postMidScoop.configurePeaking (oversampledRate, MidScoopFreqHz, 0.85f, MidScoopGainDb);
        postUpperMidBite.configurePeaking (oversampledRate, UpperMidBiteHz, 1.05f, UpperMidBiteGainDb);
        postCabRollOff.configureLowPass (oversampledRate, CabRollOffHz, 0.80f);

        biasDcOffset = std::tanh (TubeBias);
        reset();
    }

    void reset() noexcept
    {
        prePresenceShelf.reset();
        interStageHpf.reset();
        postMidScoop.reset();
        postUpperMidBite.reset();
        postCabRollOff.reset();
    }

    [[nodiscard]] static FrameGains computeFrameGains (float driveNormalized) noexcept
    {
        const float wetMix = std::sin (driveNormalized * juce::MathConstants<float>::halfPi) * 0.88f;
        const float dryMix = 1.0f - 0.72f * wetMix;
        const float preGain = 1.2f + 16.0f * (driveNormalized * driveNormalized);
        const float autoMakeup = 1.15f / std::pow (preGain, 0.38f);

        return FrameGains { dryMix, wetMix, preGain, autoMakeup };
    }

    [[nodiscard]] float processSample (size_t channel, float highBandSample, const FrameGains& gains) noexcept
    {
        // 1. Pre-emphasis ("Presence" clank boost feeding into saturation)
        const float preVoiced = prePresenceShelf.processSample (channel, highBandSample) * gains.drivePreGain;

        // 2. Two-stage asymmetric FET/op-amp soft clipping with inter-stage coupling HPF
        const float stage1 = std::tanh (preVoiced + TubeBias) - biasDcOffset;
        const float coupled = interStageHpf.processSample (channel, stage1);
        const float stage2 = std::tanh (1.35f * coupled) * gains.autoMakeupScale;

        // 3. SVT + 8x10 speaker contour (mid scoop + bite + 5 kHz fizz roll-off)
        const float cabVoiced = postCabRollOff.processSample (
            channel, postUpperMidBite.processSample (
                channel, postMidScoop.processSample (channel, stage2)));

        // 4. Parallel blend preserving clean DI transient punch
        return gains.dryMix * highBandSample + gains.wetMix * cabVoiced;
    }

private:
    StereoBiquadFilter<MaxChannels> prePresenceShelf;
    StereoBiquadFilter<MaxChannels> interStageHpf;
    StereoBiquadFilter<MaxChannels> postMidScoop;
    StereoBiquadFilter<MaxChannels> postUpperMidBite;
    StereoBiquadFilter<MaxChannels> postCabRollOff;

    float biasDcOffset = 0.0f;
};

} // namespace ferruxe::dsp
