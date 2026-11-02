#pragma once

#include <juce_graphics/juce_graphics.h>

namespace ferruxe::ui
{

/** Controls where the active colored value arc begins on a rotary knob. */
enum class KnobArcOrigin : int
{
    FromMinimum   = 0, // 0% -> 100% (Sub-Lows, Drive)
    BipolarCenter = 1, // -24 dB <- 0 dB -> +24 dB (Input, Output)
    FromMaximum   = 2  // 0 dB -> -40/-24 dB pull-down (Threshold, Ceiling)
};

/** Centralized color tokens (eliminates magic hex codes across UI components). */
namespace Theme
{
    // Chassis & panels
    inline const juce::Colour ChassisTop        { 0xFF23252A };
    inline const juce::Colour ChassisBottom     { 0xFF17181C };
    inline const juce::Colour ChassisBorder     { 0xFF34373F };
    inline const juce::Colour HeaderBackground  { 0xFF131417 };
    inline const juce::Colour HeaderRustLine    { 0xFF8C3B1E };
    inline const juce::Colour ToneCardFill      { 0x26000000 };
    inline const juce::Colour ToneCardBorder    { 0xFF332925 };

    // Typography & branding
    inline const juce::Colour BrandRust         { 0xFFD95A2B };
    inline const juce::Colour BrandLight        { 0xFFD8D2C9 };
    inline const juce::Colour LabelText         { 0xFFB8B0A4 };
    inline const juce::Colour MutedLabel        { 0xFF8E887E };
    inline const juce::Colour ReadoutText       { 0xFFE6DFD5 };
    inline const juce::Colour ReadoutBackground { 0xFF141518 };

    // Rotary dial hardware
    inline const juce::Colour ArcTrackBg        { 0xFF0E0F12 };
    inline const juce::Colour DialBodyTop       { 0xFF32353B };
    inline const juce::Colour DialBodyBottom    { 0xFF1A1C20 };
    inline const juce::Colour DialRim           { 0xFF434750 };
    inline const juce::Colour DialPointer       { 0xFFF2ECE4 };

    // Parameter & meter accents
    inline const juce::Colour GainNeutralAccent { 0xFFA3A9B5 };
    inline const juce::Colour ThresholdGold     { 0xFFD9A441 };
    inline const juce::Colour SubLowsCopper     { 0xFFC86D2D };
    inline const juce::Colour DriveRust         { 0xFFD94A27 };
    inline const juce::Colour CeilingCrimson    { 0xFFE0483E };

    inline const juce::Colour MeterTrackBg      { 0xFF101114 };
    inline const juce::Colour MeterNormalFill   { 0xFFD4622A };
    inline const juce::Colour MeterClipFill     { 0xFFE53935 };
    inline const juce::Colour ClipBadgeIdleBg   { 0xFF1E2025 };
    inline const juce::Colour ClipBadgeIdleText { 0xFF565962 };
    inline const juce::Colour ClipBadgeHotBg    { 0xFFD32F2F };
    inline const juce::Colour ClipBadgeHotText  { 0xFFFFCDD2 };
}

} // namespace ferruxe::ui
