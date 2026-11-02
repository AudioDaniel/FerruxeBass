#pragma once

#include "../BiquadFilter.h"
#include <array>
#include <cmath>

namespace ferruxe::dsp
{

/**
 * Stage 2a (Low Band < 160 Hz): Waves RBass-inspired psychoacoustic bass enhancer.
 *
 * Isolates fundamental bass notes (35 - 105 Hz), normalizes their envelope so quiet
 * and loud notes generate consistent warmth, and synthesizes 2nd/3rd/4th harmonics
 * sculpted into the 65 - 280 Hz chest-punch region.
 */
class SubHarmonicExciter
{
public:
    static constexpr size_t MaxChannels       = 2;
    static constexpr float  DetectorLowHz     = 35.0f;
    static constexpr float  DetectorHighHz    = 105.0f;
    static constexpr float  HarmonicLowCutHz  = 65.0f;
    static constexpr float  HarmonicHighCutHz = 280.0f;

    void prepare (double oversampledRate) noexcept
    {
        detectorHpf.configureHighPass (oversampledRate, DetectorLowHz);
        detectorLpf.configureLowPass (oversampledRate, DetectorHighHz);

        harmonicHpf.configureHighPass (oversampledRate, HarmonicLowCutHz);
        harmonicLpf.configureLowPass (oversampledRate, HarmonicHighCutHz, 0.80f);

        // 5 ms attack / 65 ms release envelope follower for fundamental tracking
        envAttackCoeff  = static_cast<float> (std::exp (-1.0 / (0.005 * oversampledRate)));
        envReleaseCoeff = static_cast<float> (std::exp (-1.0 / (0.065 * oversampledRate)));

        reset();
    }

    void reset() noexcept
    {
        detectorHpf.reset();
        detectorLpf.reset();
        harmonicHpf.reset();
        harmonicLpf.reset();
        fundamentalEnvelope.fill (0.0f);
    }

    [[nodiscard]] float processSample (size_t channel, float lowBandSample, float amountNormalized) noexcept
    {
        // 1. Isolate fundamental frequency band (35 - 105 Hz)
        const float fundamental = detectorLpf.processSample (
            channel, detectorHpf.processSample (channel, lowBandSample));

        // 2. Track amplitude envelope
        float& env = fundamentalEnvelope[channel];
        const float absFund = std::abs (fundamental);
        const float coeff = (absFund > env) ? envAttackCoeff : envReleaseCoeff;
        env = coeff * env + (1.0f - coeff) * absFund;

        // 3. Noise-floor gate (-54 dBFS knee) + envelope-normalized carrier
        const float envSquared = env * env;
        const float noiseGate = envSquared / (envSquared + 0.00025f);
        const float normalizedCarrier = fundamental / (env + 1.0e-4f);
        const float compressedEnv = std::pow (env + 1.0e-6f, 0.72f) * 0.65f * noiseGate;

        // 4. Synthesize even (2nd/4th via smooth half-wave rectifier) + odd (3rd) harmonics
        const float evenHarmonics = 0.5f * (normalizedCarrier + std::sqrt (normalizedCarrier * normalizedCarrier + 0.01f)) - 0.35f;
        const float oddHarmonics  = std::tanh (1.6f * normalizedCarrier) - 0.85f * normalizedCarrier;
        const float rawHarmonics  = compressedEnv * (1.35f * evenHarmonics + 0.65f * oddHarmonics);

        // 5. Band-limit harmonics to 65 - 280 Hz and blend with gently peak-glued low band
        const float sculptedHarmonics = harmonicLpf.processSample (
            channel, harmonicHpf.processSample (channel, rawHarmonics));

        const float gluedCleanLow = lowBandSample / std::sqrt (1.0f + (0.35f * amountNormalized) * lowBandSample * lowBandSample);
        return gluedCleanLow + (amountNormalized * 1.65f) * sculptedHarmonics;
    }

private:
    StereoBiquadFilter<MaxChannels> detectorHpf;
    StereoBiquadFilter<MaxChannels> detectorLpf;
    StereoBiquadFilter<MaxChannels> harmonicHpf;
    StereoBiquadFilter<MaxChannels> harmonicLpf;

    std::array<float, MaxChannels> fundamentalEnvelope { 0.0f, 0.0f };
    float envAttackCoeff  = 0.0f;
    float envReleaseCoeff = 0.0f;
};

} // namespace ferruxe::dsp
