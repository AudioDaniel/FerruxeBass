#pragma once

#include <juce_dsp/juce_dsp.h>
#include "../Parameters/PluginParameters.h"
#include "Stages/BassCompressor.h"
#include "Stages/SubHarmonicExciter.h"
#include "Stages/SansAmpDriveStage.h"
#include "Stages/BrickwallClipper.h"
#include <memory>

namespace ferruxe::dsp
{

/**
 * Orchestrates the complete FerruxeBass audio pipeline:
 *   Input + BassCompressor -> 4x Oversampled [160 Hz Crossover -> SubHarmonicExciter | SansAmpDriveStage] -> BrickwallClipper
 */
class FerruxeEngine
{
public:
    struct ProcessResult
    {
        float peakGainReductionDb = 0.0f;
        bool  ceilingClipped      = false;
    };

    static constexpr size_t MaxChannels            = 2;
    static constexpr size_t OversamplingOrder      = 2; // 2^2 = 4x oversampling
    static constexpr float  CrossoverFrequencyHz   = 160.0f;
    static constexpr double ParameterSmoothingSecs = 0.020;

    FerruxeEngine() = default;

    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();

    void updateParameters (const ParameterValues& values, bool snapImmediately = false);
    ProcessResult process (juce::AudioBuffer<float>& buffer) noexcept;

    [[nodiscard]] int getLatencySamples() const noexcept { return latencySamples; }

private:
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;
    juce::dsp::LinkwitzRileyFilter<float> crossover;

    BassCompressor     compressor;
    SubHarmonicExciter subExciter;
    SansAmpDriveStage  driveStage;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> inputGainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> thresholdDbSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> subLowsSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> driveSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> outputGainSmoothed;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> ceilingGainSmoothed;

    size_t preparedChannels = 0;
    int    latencySamples   = 0;
};

} // namespace ferruxe::dsp
