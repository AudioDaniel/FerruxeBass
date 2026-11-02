#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace ferruxe::dsp
{

/**
 * Stage 3 (Output): Hard clipper / brickwall ceiling limiter.
 *
 * Clips first inside the 4x oversampled domain (removing harsh aliasing foldback)
 * and applies a final brickwall clamp after downsampling so zero filter ripple ever
 * exceeds the user's Ceiling dBFS setting.
 */
class BrickwallClipper
{
public:
    [[nodiscard]] static float clipSample (float sample, float ceilingLinear, bool& outDidClip) noexcept
    {
        if (std::abs (sample) > ceilingLinear)
            outDidClip = true;

        return juce::jlimit (-ceilingLinear, ceilingLinear, sample);
    }

    static void clampBufferBrickwall (juce::AudioBuffer<float>& buffer,
                                      size_t numChannels,
                                      float ceilingLinear,
                                      bool& outDidClip) noexcept
    {
        const int numSamples = buffer.getNumSamples();

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            float* samples = buffer.getWritePointer (static_cast<int> (ch));

            for (int i = 0; i < numSamples; ++i)
                samples[i] = clipSample (samples[i], ceilingLinear, outDidClip);
        }
    }
};

} // namespace ferruxe::dsp
