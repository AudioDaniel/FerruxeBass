#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../../Parameters/AudioMeterBridge.h"
#include "../Theme.h"

namespace ferruxe::ui
{

/**
 * Header metering widget displaying compressor Gain Reduction (GR),
 * Input/Output peak bars, and the Ceiling brickwall CLIP badge.
 */
class HeaderMeterStrip : public juce::Component
{
public:
    HeaderMeterStrip() = default;

    void updateFromSnapshot (const MeterSnapshot& snapshot)
    {
        constexpr float ballisticDecay = 0.82f;

        displayInputPeak  = (snapshot.inputPeakLinear  > displayInputPeak)  ? snapshot.inputPeakLinear  : displayInputPeak  * ballisticDecay;
        displayOutputPeak = (snapshot.outputPeakLinear > displayOutputPeak) ? snapshot.outputPeakLinear : displayOutputPeak * ballisticDecay;
        displayGrDb       = (snapshot.gainReductionDb  > displayGrDb)       ? snapshot.gainReductionDb  : displayGrDb       * ballisticDecay;

        if (snapshot.ceilingClipped)
            clipHoldFrames = 8; // Hold indicator for ~260 ms at 30 Hz refresh
        else if (clipHoldFrames > 0)
            --clipHoldFrames;

        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const bool isClipping = (clipHoldFrames > 0);

        drawGainReductionBar (g, { 0.0f, 14.0f, 152.0f, 11.0f }, displayGrDb);
        drawPeakLevelBar     (g, { 168.0f, 7.0f,  160.0f, 10.0f }, "IN",  displayInputPeak,  false);
        drawPeakLevelBar     (g, { 168.0f, 22.0f, 160.0f, 10.0f }, "OUT", displayOutputPeak, isClipping);
        drawClipBadge        (g, { bounds.getWidth() - 40.0f, 10.0f, 38.0f, 18.0f }, isClipping);
    }

private:
    static void drawPeakLevelBar (juce::Graphics& g,
                                  juce::Rectangle<float> area,
                                  const juce::String& label,
                                  float linearLevel,
                                  bool highlightClip)
    {
        auto labelArea = area.removeFromLeft (28.0f);
        g.setColour (Theme::MutedLabel);
        g.setFont (juce::FontOptions (10.5f).withStyle ("Bold"));
        g.drawText (label, labelArea, juce::Justification::centredLeft, false);

        g.setColour (Theme::MeterTrackBg);
        g.fillRoundedRectangle (area, 3.0f);

        const float levelDb = juce::Decibels::gainToDecibels (linearLevel, -60.0f);
        const float normalized = juce::jlimit (0.0f, 1.0f, (levelDb + 60.0f) / 66.0f); // -60 dB .. +6 dB

        if (normalized > 0.005f)
        {
            auto fill = area.reduced (1.5f);
            fill.setWidth (fill.getWidth() * normalized);

            const bool showRed = highlightClip || (linearLevel >= 1.0f);
            g.setColour (showRed ? Theme::MeterClipFill : Theme::MeterNormalFill);
            g.fillRoundedRectangle (fill, 2.0f);
        }
    }

    static void drawGainReductionBar (juce::Graphics& g, juce::Rectangle<float> area, float grDb)
    {
        auto labelArea = area.removeFromLeft (26.0f);
        g.setColour (Theme::ThresholdGold);
        g.setFont (juce::FontOptions (10.5f).withStyle ("Bold"));
        g.drawText ("GR", labelArea, juce::Justification::centredLeft, false);

        auto readoutArea = area.removeFromRight (46.0f);
        g.setColour (Theme::LabelText);
        g.setFont (juce::FontOptions (10.5f));
        const juce::String text = (grDb >= 0.1f) ? ("-" + juce::String (grDb, 1)) : "0.0";
        g.drawText (text + " dB", readoutArea, juce::Justification::centredRight, false);

        area.removeFromRight (4.0f);
        g.setColour (Theme::MeterTrackBg);
        g.fillRoundedRectangle (area, 3.0f);

        const float normalized = juce::jlimit (0.0f, 1.0f, grDb / 18.0f);
        if (normalized > 0.005f)
        {
            auto fill = area.reduced (1.5f);
            fill = fill.removeFromRight (fill.getWidth() * normalized);
            g.setColour (Theme::ThresholdGold);
            g.fillRoundedRectangle (fill, 2.0f);
        }
    }

    static void drawClipBadge (juce::Graphics& g, juce::Rectangle<float> area, bool isClipping)
    {
        g.setColour (isClipping ? Theme::ClipBadgeHotBg : Theme::ClipBadgeIdleBg);
        g.fillRoundedRectangle (area, 4.0f);

        g.setColour (isClipping ? Theme::ClipBadgeHotText : Theme::ClipBadgeIdleText);
        g.setFont (juce::FontOptions (9.5f).withStyle ("Bold"));
        g.drawText ("CLIP", area, juce::Justification::centred, false);
    }

    float displayInputPeak  = 0.0f;
    float displayOutputPeak = 0.0f;
    float displayGrDb       = 0.0f;
    int   clipHoldFrames    = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderMeterStrip)
};

} // namespace ferruxe::ui
