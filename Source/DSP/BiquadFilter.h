#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>

namespace ferruxe::dsp
{

/**
 * Zero-allocation 2nd-order IIR EQ filter (Audio EQ Cookbook Direct-Form II Transposed).
 * Safe to run on the real-time audio thread without GC/heap pressure.
 */
class BiquadFilter
{
public:
    static constexpr float ButterworthQ = 0.70710678f;

    void reset() noexcept
    {
        z1 = 0.0f;
        z2 = 0.0f;
    }

    [[nodiscard]] float processSample (float input) noexcept
    {
        const float output = b0 * input + z1;
        z1 = b1 * input - a1 * output + z2;
        z2 = b2 * input - a2 * output;
        return output;
    }

    void configureHighPass (double sampleRate, float freqHz, float q = ButterworthQ) noexcept
    {
        const double w0 = juce::MathConstants<double>::twoPi * static_cast<double> (freqHz) / sampleRate;
        const double cosW0 = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * static_cast<double> (q));
        const double a0Inv = 1.0 / (1.0 + alpha);
        const double halfOnePlusCos = 0.5 * (1.0 + cosW0);

        setNormalizedCoeffs (halfOnePlusCos * a0Inv,
                             -(1.0 + cosW0) * a0Inv,
                             halfOnePlusCos * a0Inv,
                             -2.0 * cosW0 * a0Inv,
                             (1.0 - alpha) * a0Inv);
    }

    void configureLowPass (double sampleRate, float freqHz, float q = ButterworthQ) noexcept
    {
        const double w0 = juce::MathConstants<double>::twoPi * static_cast<double> (freqHz) / sampleRate;
        const double cosW0 = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * static_cast<double> (q));
        const double a0Inv = 1.0 / (1.0 + alpha);
        const double halfOneMinusCos = 0.5 * (1.0 - cosW0);

        setNormalizedCoeffs (halfOneMinusCos * a0Inv,
                             (1.0 - cosW0) * a0Inv,
                             halfOneMinusCos * a0Inv,
                             -2.0 * cosW0 * a0Inv,
                             (1.0 - alpha) * a0Inv);
    }

    void configurePeaking (double sampleRate, float freqHz, float q, float gainDb) noexcept
    {
        const double amp = std::pow (10.0, static_cast<double> (gainDb) / 40.0);
        const double w0 = juce::MathConstants<double>::twoPi * static_cast<double> (freqHz) / sampleRate;
        const double cosW0 = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * static_cast<double> (q));
        const double a0Inv = 1.0 / (1.0 + alpha / amp);

        setNormalizedCoeffs ((1.0 + alpha * amp) * a0Inv,
                             -2.0 * cosW0 * a0Inv,
                             (1.0 - alpha * amp) * a0Inv,
                             -2.0 * cosW0 * a0Inv,
                             (1.0 - alpha / amp) * a0Inv);
    }

    void configureLowShelf (double sampleRate, float freqHz, float q, float gainDb) noexcept
    {
        const double amp = std::pow (10.0, static_cast<double> (gainDb) / 40.0);
        const double w0 = juce::MathConstants<double>::twoPi * static_cast<double> (freqHz) / sampleRate;
        const double cosW0 = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * static_cast<double> (q));
        const double twoSqrtAmpAlpha = 2.0 * std::sqrt (amp) * alpha;
        const double a0Inv = 1.0 / ((amp + 1.0) + (amp - 1.0) * cosW0 + twoSqrtAmpAlpha);

        setNormalizedCoeffs (amp * ((amp + 1.0) - (amp - 1.0) * cosW0 + twoSqrtAmpAlpha) * a0Inv,
                             2.0 * amp * ((amp - 1.0) - (amp + 1.0) * cosW0) * a0Inv,
                             amp * ((amp + 1.0) - (amp - 1.0) * cosW0 - twoSqrtAmpAlpha) * a0Inv,
                             -2.0 * ((amp - 1.0) + (amp + 1.0) * cosW0) * a0Inv,
                             ((amp + 1.0) + (amp - 1.0) * cosW0 - twoSqrtAmpAlpha) * a0Inv);
    }

    void configureHighShelf (double sampleRate, float freqHz, float q, float gainDb) noexcept
    {
        const double amp = std::pow (10.0, static_cast<double> (gainDb) / 40.0);
        const double w0 = juce::MathConstants<double>::twoPi * static_cast<double> (freqHz) / sampleRate;
        const double cosW0 = std::cos (w0);
        const double alpha = std::sin (w0) / (2.0 * static_cast<double> (q));
        const double twoSqrtAmpAlpha = 2.0 * std::sqrt (amp) * alpha;
        const double a0Inv = 1.0 / ((amp + 1.0) - (amp - 1.0) * cosW0 + twoSqrtAmpAlpha);

        setNormalizedCoeffs (amp * ((amp + 1.0) + (amp - 1.0) * cosW0 + twoSqrtAmpAlpha) * a0Inv,
                             -2.0 * amp * ((amp - 1.0) + (amp + 1.0) * cosW0) * a0Inv,
                             amp * ((amp + 1.0) + (amp - 1.0) * cosW0 - twoSqrtAmpAlpha) * a0Inv,
                             2.0 * ((amp - 1.0) - (amp + 1.0) * cosW0) * a0Inv,
                             ((amp + 1.0) - (amp - 1.0) * cosW0 - twoSqrtAmpAlpha) * a0Inv);
    }

private:
    void setNormalizedCoeffs (double newB0, double newB1, double newB2, double newA1, double newA2) noexcept
    {
        b0 = static_cast<float> (newB0);
        b1 = static_cast<float> (newB1);
        b2 = static_cast<float> (newB2);
        a1 = static_cast<float> (newA1);
        a2 = static_cast<float> (newA2);
    }

    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float z1 = 0.0f, z2 = 0.0f;
};

/**
 * Multi-channel wrapper around BiquadFilter so left/right channels keep independent
 * delay state while sharing identical filter configuration.
 */
template <size_t MaxChannels = 2>
class StereoBiquadFilter
{
public:
    void reset() noexcept
    {
        for (auto& filter : channelFilters)
            filter.reset();
    }

    [[nodiscard]] float processSample (size_t channel, float input) noexcept
    {
        return channelFilters[channel].processSample (input);
    }

    void configureHighPass (double sampleRate, float freqHz, float q = BiquadFilter::ButterworthQ) noexcept
    {
        channelFilters[0].configureHighPass (sampleRate, freqHz, q);
        syncOtherChannels();
    }

    void configureLowPass (double sampleRate, float freqHz, float q = BiquadFilter::ButterworthQ) noexcept
    {
        channelFilters[0].configureLowPass (sampleRate, freqHz, q);
        syncOtherChannels();
    }

    void configurePeaking (double sampleRate, float freqHz, float q, float gainDb) noexcept
    {
        channelFilters[0].configurePeaking (sampleRate, freqHz, q, gainDb);
        syncOtherChannels();
    }

    void configureLowShelf (double sampleRate, float freqHz, float q, float gainDb) noexcept
    {
        channelFilters[0].configureLowShelf (sampleRate, freqHz, q, gainDb);
        syncOtherChannels();
    }

    void configureHighShelf (double sampleRate, float freqHz, float q, float gainDb) noexcept
    {
        channelFilters[0].configureHighShelf (sampleRate, freqHz, q, gainDb);
        syncOtherChannels();
    }

private:
    void syncOtherChannels() noexcept
    {
        for (size_t ch = 1; ch < MaxChannels; ++ch)
        {
            channelFilters[ch] = channelFilters[0];
            channelFilters[ch].reset();
        }
    }

    std::array<BiquadFilter, MaxChannels> channelFilters {};
};

} // namespace ferruxe::dsp
