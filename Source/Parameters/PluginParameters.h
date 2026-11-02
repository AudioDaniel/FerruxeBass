#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <atomic>

namespace ferruxe
{

/** Unique parameter IDs exposed to the DAW host (VST3 / LV2 automation & presets). */
namespace ParamIDs
{
    inline constexpr const char* Input     = "input";
    inline constexpr const char* Threshold = "threshold";
    inline constexpr const char* SubLows   = "subLows";
    inline constexpr const char* Drive     = "drive";
    inline constexpr const char* Output    = "output";
    inline constexpr const char* Ceiling   = "ceiling";
}

/** Value-type snapshot (similar to a C# record struct) of all user knob settings. */
struct ParameterValues
{
    float inputDb         = 0.0f;
    float thresholdDb     = -18.0f;
    float subLowsPercent  = 30.0f;
    float drivePercent    = 30.0f;
    float outputDb        = 0.0f;
    float ceilingDb       = 0.0f;
};

/**
 * Defines the plugin's parameter schema and caches lock-free atomic pointers
 * so the real-time audio thread can read knob values without string lookups or locks.
 */
class ParameterReferences
{
public:
    explicit ParameterReferences (juce::AudioProcessorValueTreeState& apvts)
        : inputPtr     (apvts.getRawParameterValue (ParamIDs::Input)),
          thresholdPtr (apvts.getRawParameterValue (ParamIDs::Threshold)),
          subLowsPtr   (apvts.getRawParameterValue (ParamIDs::SubLows)),
          drivePtr     (apvts.getRawParameterValue (ParamIDs::Drive)),
          outputPtr    (apvts.getRawParameterValue (ParamIDs::Output)),
          ceilingPtr   (apvts.getRawParameterValue (ParamIDs::Ceiling))
    {
    }

    [[nodiscard]] ParameterValues readCurrentValues() const noexcept
    {
        return ParameterValues
        {
            inputPtr->load (std::memory_order_relaxed),
            thresholdPtr->load (std::memory_order_relaxed),
            subLowsPtr->load (std::memory_order_relaxed),
            drivePtr->load (std::memory_order_relaxed),
            outputPtr->load (std::memory_order_relaxed),
            ceilingPtr->load (std::memory_order_relaxed)
        };
    }

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        auto formatDb = [] (float value, int) { return juce::String (value, 1) + " dB"; };
        auto formatPercent = [] (float value, int) { return juce::String (juce::roundToInt (value)) + " %"; };

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::Input, 1 },
            "Input",
            juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (formatDb)));

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::Threshold, 1 },
            "Threshold",
            juce::NormalisableRange<float> (-40.0f, 0.0f, 0.1f),
            -18.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (formatDb)));

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::SubLows, 1 },
            "Sub-Lows",
            juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
            30.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (formatPercent)));

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::Drive, 1 },
            "Drive",
            juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f),
            30.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (formatPercent)));

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::Output, 1 },
            "Output",
            juce::NormalisableRange<float> (-24.0f, 24.0f, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (formatDb)));

        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { ParamIDs::Ceiling, 1 },
            "Ceiling",
            juce::NormalisableRange<float> (-24.0f, 0.0f, 0.1f),
            0.0f,
            juce::AudioParameterFloatAttributes().withStringFromValueFunction (formatDb)));

        return layout;
    }

private:
    std::atomic<float>* inputPtr     = nullptr;
    std::atomic<float>* thresholdPtr = nullptr;
    std::atomic<float>* subLowsPtr   = nullptr;
    std::atomic<float>* drivePtr     = nullptr;
    std::atomic<float>* outputPtr    = nullptr;
    std::atomic<float>* ceilingPtr   = nullptr;
};

} // namespace ferruxe
