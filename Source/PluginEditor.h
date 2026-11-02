#pragma once

#include "PluginProcessor.h"
#include "UI/FerruxeLookAndFeel.h"
#include "UI/Components/RotaryKnob.h"
#include "UI/Components/HeaderMeterStrip.h"

/**
 * Top-level plugin window (analogous to a WPF Window / MAUI ContentPage).
 * Composes reusable RotaryKnob and HeaderMeterStrip child components.
 */
class FerruxeBassAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::Timer
{
public:
    explicit FerruxeBassAudioProcessorEditor (FerruxeBassAudioProcessor& ownerProcessor);
    ~FerruxeBassAudioProcessorEditor() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    void timerCallback() override;

    FerruxeBassAudioProcessor&      audioProcessor;
    ferruxe::ui::FerruxeLookAndFeel lookAndFeel;
    ferruxe::ui::HeaderMeterStrip   meterStrip;

    ferruxe::ui::RotaryKnob inputKnob;
    ferruxe::ui::RotaryKnob thresholdKnob;
    ferruxe::ui::RotaryKnob subLowsKnob;
    ferruxe::ui::RotaryKnob driveKnob;
    ferruxe::ui::RotaryKnob outputKnob;
    ferruxe::ui::RotaryKnob ceilingKnob;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FerruxeBassAudioProcessorEditor)
};
