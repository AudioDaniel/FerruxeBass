#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Theme.h"
#include <cmath>

namespace ferruxe::ui
{

/** Custom vector renderer for FerruxeBass metallic rotary dials and value arcs. */
class FerruxeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    static inline const juce::Identifier ArcOriginProperty { "arcOrigin" };

    FerruxeLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId,       Theme::ReadoutText);
        setColour (juce::Slider::textBoxOutlineColourId,    juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxBackgroundColourId, Theme::ReadoutBackground);
        setColour (juce::Slider::textBoxHighlightColourId,  Theme::HeaderRustLine);
        setColour (juce::Label::textColourId,               Theme::LabelText);
    }

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const auto bounds     = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (8.0f);
        const float radius    = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto centre     = bounds.getCentre();
        const float arcRadius = radius - 4.0f;
        const float toAngle   = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // 1. Background groove arc
        juce::Path trackArc;
        trackArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour (Theme::ArcTrackBg);
        g.strokePath (trackArc, juce::PathStrokeType (4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // 2. Active colored arc
        const float originAngle = resolveOriginAngle (slider, rotaryStartAngle, rotaryEndAngle);
        if (std::abs (toAngle - originAngle) > 0.005f)
        {
            juce::Path valueArc;
            valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, originAngle, toAngle, true);
            g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
            g.strokePath (valueArc, juce::PathStrokeType (4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // 3. Brushed metallic knob body
        const float dialRadius = arcRadius - 9.0f;
        juce::ColourGradient dialGradient (Theme::DialBodyTop,    centre.x, centre.y - dialRadius,
                                           Theme::DialBodyBottom, centre.x, centre.y + dialRadius, false);
        g.setGradientFill (dialGradient);
        g.fillEllipse (centre.x - dialRadius, centre.y - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f);

        g.setColour (Theme::DialRim);
        g.drawEllipse (centre.x - dialRadius, centre.y - dialRadius, dialRadius * 2.0f, dialRadius * 2.0f, 1.5f);

        // 4. Indicator notch
        juce::Path pointer;
        const float pointerLength = dialRadius * 0.68f;
        constexpr float pointerWidth = 3.0f;
        pointer.addRoundedRectangle (-pointerWidth * 0.5f, -dialRadius + 4.0f, pointerWidth, pointerLength, 1.5f);

        g.setColour (Theme::DialPointer);
        g.fillPath (pointer, juce::AffineTransform::rotation (toAngle).translated (centre.x, centre.y));
    }

private:
    [[nodiscard]] static float resolveOriginAngle (const juce::Slider& slider,
                                                   float startAngle,
                                                   float endAngle) noexcept
    {
        const int rawMode = slider.getProperties().getWithDefault (
            ArcOriginProperty, static_cast<int> (KnobArcOrigin::FromMinimum));

        switch (static_cast<KnobArcOrigin> (rawMode))
        {
            case KnobArcOrigin::BipolarCenter:
                return (startAngle + endAngle) * 0.5f;
            case KnobArcOrigin::FromMaximum:
                return endAngle;
            case KnobArcOrigin::FromMinimum:
            default:
                return startAngle;
        }
    }
};

} // namespace ferruxe::ui
