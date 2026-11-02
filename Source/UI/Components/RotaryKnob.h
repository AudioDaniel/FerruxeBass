#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../FerruxeLookAndFeel.h"

namespace ferruxe::ui
{

/**
 * Self-contained UI UserControl combining a title label, rotary slider,
 * and automatic two-way data binding (SliderAttachment) to an APVTS parameter.
 */
class RotaryKnob : public juce::Component
{
public:
    RotaryKnob (juce::AudioProcessorValueTreeState& apvts,
                const juce::String& parameterId,
                const juce::String& titleText,
                juce::Colour accentColour,
                KnobArcOrigin arcOrigin)
    {
        titleLabel.setText (titleText, juce::dontSendNotification);
        titleLabel.setJustificationType (juce::Justification::centred);
        titleLabel.setFont (juce::FontOptions (13.0f).withStyle ("Bold"));
        addAndMakeVisible (titleLabel);

        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 78, 22);
        slider.setColour (juce::Slider::rotarySliderFillColourId, accentColour);
        slider.getProperties().set (FerruxeLookAndFeel::ArcOriginProperty, static_cast<int> (arcOrigin));
        addAndMakeVisible (slider);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, parameterId, slider);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        titleLabel.setBounds (bounds.removeFromTop (24));
        slider.setBounds (bounds.reduced (6, 2));
    }

private:
    juce::Label  titleLabel;
    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RotaryKnob)
};

} // namespace ferruxe::ui
