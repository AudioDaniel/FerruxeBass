#include "FerruxeEngine.h"
#include <array>

namespace ferruxe::dsp
{

void FerruxeEngine::prepare (const juce::dsp::ProcessSpec& spec)
{
    const size_t numChannels = juce::jmin (static_cast<size_t> (spec.numChannels), MaxChannels);

    if (oversampler == nullptr || preparedChannels != numChannels)
    {
        oversampler = std::make_unique<juce::dsp::Oversampling<float>> (
            numChannels,
            OversamplingOrder,
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true);
        preparedChannels = numChannels;
    }

    oversampler->initProcessing (spec.maximumBlockSize);
    latencySamples = juce::roundToInt (oversampler->getLatencyInSamples());

    const double baseRate        = spec.sampleRate;
    const double oversampledRate = baseRate * static_cast<double> (oversampler->getOversamplingFactor());

    juce::dsp::ProcessSpec oversampledSpec {};
    oversampledSpec.sampleRate       = oversampledRate;
    oversampledSpec.maximumBlockSize = spec.maximumBlockSize * static_cast<juce::uint32> (oversampler->getOversamplingFactor());
    oversampledSpec.numChannels      = static_cast<juce::uint32> (numChannels);

    crossover.prepare (oversampledSpec);
    crossover.setType (juce::dsp::LinkwitzRileyFilterType::lowpass);
    crossover.setCutoffFrequency (CrossoverFrequencyHz);

    compressor.prepare (baseRate);
    subExciter.prepare (oversampledRate);
    driveStage.prepare (oversampledRate);

    inputGainSmoothed.reset   (baseRate,        ParameterSmoothingSecs);
    thresholdDbSmoothed.reset (baseRate,        ParameterSmoothingSecs);
    subLowsSmoothed.reset     (oversampledRate, ParameterSmoothingSecs);
    driveSmoothed.reset       (oversampledRate, ParameterSmoothingSecs);
    outputGainSmoothed.reset  (oversampledRate, ParameterSmoothingSecs);
    ceilingGainSmoothed.reset (oversampledRate, ParameterSmoothingSecs);

    reset();
}

void FerruxeEngine::reset()
{
    if (oversampler != nullptr)
        oversampler->reset();

    crossover.reset();
    compressor.reset();
    subExciter.reset();
    driveStage.reset();
}

void FerruxeEngine::updateParameters (const ParameterValues& values, bool snapImmediately)
{
    const float inputLinear   = juce::Decibels::decibelsToGain (values.inputDb);
    const float thresholdDb   = juce::jlimit (-40.0f, 0.0f, values.thresholdDb);
    const float subNormalized = juce::jlimit (0.0f, 1.0f, values.subLowsPercent * 0.01f);
    const float drvNormalized = juce::jlimit (0.0f, 1.0f, values.drivePercent * 0.01f);
    const float outputLinear  = juce::Decibels::decibelsToGain (values.outputDb);
    const float ceilingLinear = juce::Decibels::decibelsToGain (juce::jlimit (-24.0f, 0.0f, values.ceilingDb));

    if (snapImmediately)
    {
        inputGainSmoothed.setCurrentAndTargetValue   (inputLinear);
        thresholdDbSmoothed.setCurrentAndTargetValue (thresholdDb);
        subLowsSmoothed.setCurrentAndTargetValue     (subNormalized);
        driveSmoothed.setCurrentAndTargetValue       (drvNormalized);
        outputGainSmoothed.setCurrentAndTargetValue  (outputLinear);
        ceilingGainSmoothed.setCurrentAndTargetValue (ceilingLinear);
    }
    else
    {
        inputGainSmoothed.setTargetValue   (inputLinear);
        thresholdDbSmoothed.setTargetValue (thresholdDb);
        subLowsSmoothed.setTargetValue     (subNormalized);
        driveSmoothed.setTargetValue       (drvNormalized);
        outputGainSmoothed.setTargetValue  (outputLinear);
        ceilingGainSmoothed.setTargetValue (ceilingLinear);
    }
}

FerruxeEngine::ProcessResult FerruxeEngine::process (juce::AudioBuffer<float>& buffer) noexcept
{
    ProcessResult result {};

    const int numSamples  = buffer.getNumSamples();
    const size_t numCh    = juce::jmin (static_cast<size_t> (buffer.getNumChannels()), preparedChannels);

    if (numSamples <= 0 || numCh == 0 || oversampler == nullptr)
        return result;

    // Stage 1: Input trim + subsonic rumble filter + 4:1 bass compressor (base sample rate)
    result.peakGainReductionDb = compressor.processBlock (buffer, numCh, inputGainSmoothed, thresholdDbSmoothed);

    // Stage 2: Upsample 4x -> 160 Hz Split-Band [SubHarmonicExciter | SansAmpDriveStage] -> Hard Clip
    juce::dsp::AudioBlock<float> baseBlock (buffer);
    auto activeBaseBlock = baseBlock.getSubsetChannelBlock (0, numCh);
    auto oversampledBlock = oversampler->processSamplesUp (activeBaseBlock);

    std::array<float*, MaxChannels> channelPtrs { nullptr, nullptr };
    for (size_t ch = 0; ch < numCh; ++ch)
        channelPtrs[ch] = oversampledBlock.getChannelPointer (ch);

    const float ceilingAtBlockStart = ceilingGainSmoothed.getCurrentValue();
    const size_t oversampledCount   = oversampledBlock.getNumSamples();

    for (size_t i = 0; i < oversampledCount; ++i)
    {
        const float subAmount    = subLowsSmoothed.getNextValue();
        const float driveAmount  = driveSmoothed.getNextValue();
        const float outputGain   = outputGainSmoothed.getNextValue();
        const float ceilingLimit = ceilingGainSmoothed.getNextValue();

        const auto driveGains = SansAmpDriveStage::computeFrameGains (driveAmount);

        for (size_t ch = 0; ch < numCh; ++ch)
        {
            float lowBand  = 0.0f;
            float highBand = 0.0f;
            crossover.processSample (static_cast<int> (ch), channelPtrs[ch][i], lowBand, highBand);

            const float processedLow  = subExciter.processSample (ch, lowBand, subAmount);
            const float processedHigh = driveStage.processSample (ch, highBand, driveGains);
            const float summedOutput  = (processedLow + processedHigh) * outputGain;

            channelPtrs[ch][i] = BrickwallClipper::clipSample (summedOutput, ceilingLimit, result.ceilingClipped);
        }
    }

    // Stage 3: Downsample back to base rate + final brickwall clamp to eliminate filter overshoot
    oversampler->processSamplesDown (activeBaseBlock);

    const float brickwallBound = juce::jmax (ceilingAtBlockStart, ceilingGainSmoothed.getCurrentValue());
    BrickwallClipper::clampBufferBrickwall (buffer, numCh, brickwallBound, result.ceilingClipped);

    return result;
}

} // namespace ferruxe::dsp
