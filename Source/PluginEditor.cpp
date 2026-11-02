#include "PluginEditor.h"
#include "UI/Theme.h"

using ferruxe::ParamIDs::Ceiling;
using ferruxe::ParamIDs::Drive;
using ferruxe::ParamIDs::Input;
using ferruxe::ParamIDs::Output;
using ferruxe::ParamIDs::SubLows;
using ferruxe::ParamIDs::Threshold;
using ferruxe::ui::KnobArcOrigin;
using namespace ferruxe::ui;

FerruxeBassAudioProcessorEditor::FerruxeBassAudioProcessorEditor (FerruxeBassAudioProcessor& ownerProcessor)
    : AudioProcessorEditor (&ownerProcessor),
      audioProcessor (ownerProcessor),
      inputKnob     (ownerProcessor.getValueTreeState(), Input,     "INPUT",    Theme::GainNeutralAccent, KnobArcOrigin::BipolarCenter),
      thresholdKnob (ownerProcessor.getValueTreeState(), Threshold, "THRESH",   Theme::ThresholdGold,     KnobArcOrigin::FromMaximum),
      subLowsKnob   (ownerProcessor.getValueTreeState(), SubLows,   "SUB-LOWS", Theme::SubLowsCopper,     KnobArcOrigin::FromMinimum),
      driveKnob     (ownerProcessor.getValueTreeState(), Drive,     "DRIVE",    Theme::DriveRust,         KnobArcOrigin::FromMinimum),
      outputKnob    (ownerProcessor.getValueTreeState(), Output,    "OUTPUT",   Theme::GainNeutralAccent, KnobArcOrigin::BipolarCenter),
      ceilingKnob   (ownerProcessor.getValueTreeState(), Ceiling,   "CEILING",  Theme::CeilingCrimson,    KnobArcOrigin::FromMaximum)
{
    setLookAndFeel (&lookAndFeel);

    addAndMakeVisible (meterStrip);
    for (auto* knob : { &inputKnob, &thresholdKnob, &subLowsKnob, &driveKnob, &outputKnob, &ceilingKnob })
        addAndMakeVisible (*knob);

    setSize (780, 265);
    startTimerHz (30);
}

FerruxeBassAudioProcessorEditor::~FerruxeBassAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void FerruxeBassAudioProcessorEditor::timerCallback()
{
    meterStrip.updateFromSnapshot (audioProcessor.consumeMeterSnapshot());
}

void FerruxeBassAudioProcessorEditor::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();

    // Chassis gradient background
    juce::ColourGradient chassisGrad (Theme::ChassisTop,    0.0f, 0.0f,
                                      Theme::ChassisBottom, 0.0f, bounds.getHeight(), false);
    g.setGradientFill (chassisGrad);
    g.fillAll();

    // Top header bar + rust divider stripe
    auto header = getLocalBounds().removeFromTop (52).toFloat();
    g.setColour (Theme::HeaderBackground);
    g.fillRect (header);

    g.setColour (Theme::HeaderRustLine);
    g.fillRect (0.0f, header.getBottom() - 2.0f, bounds.getWidth(), 2.0f);

    // Branding text
    g.setColour (Theme::BrandRust);
    g.setFont (juce::FontOptions (20.0f).withStyle ("Bold"));
    g.drawText ("FERRUXE", 20, 12, 110, 28, juce::Justification::centredLeft, false);

    g.setColour (Theme::BrandLight);
    g.setFont (juce::FontOptions (20.0f));
    g.drawText ("BASS", 118, 12, 80, 28, juce::Justification::centredLeft, false);

    // Recessed card grouping the 3 core tone processors (THRESH -> SUB-LOWS -> DRIVE)
    const float columnWidth = (bounds.getWidth() - 32.0f) / 6.0f;
    const juce::Rectangle<float> toneSectionCard (16.0f + columnWidth, 64.0f, columnWidth * 3.0f, bounds.getHeight() - 78.0f);

    g.setColour (Theme::ToneCardFill);
    g.fillRoundedRectangle (toneSectionCard, 8.0f);
    g.setColour (Theme::ToneCardBorder);
    g.drawRoundedRectangle (toneSectionCard, 8.0f, 1.0f);

    // Outer border
    g.setColour (Theme::ChassisBorder);
    g.drawRect (getLocalBounds(), 1);
}

void FerruxeBassAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Header layout
    auto header = bounds.removeFromTop (52);
    meterStrip.setBounds (header.removeFromRight (394).reduced (0, 6));

    // 6-column horizontal FlexBox row for knobs
    auto knobArea = bounds.reduced (16, 10);
    knobArea.removeFromTop (6);

    juce::FlexBox row;
    row.flexDirection = juce::FlexBox::Direction::row;

    for (auto* knob : { &inputKnob, &thresholdKnob, &subLowsKnob, &driveKnob, &outputKnob, &ceilingKnob })
        row.items.add (juce::FlexItem (*knob).withFlex (1.0f));

    row.performLayout (knobArea);
}
