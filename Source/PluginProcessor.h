#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Parameters/PluginParameters.h"
#include "Parameters/AudioMeterBridge.h"
#include "DSP/FerruxeEngine.h"

/**
 * Host-facing AudioProcessor adapter (analogous to a Controller / Host Service in .NET).
 * Bridges DAW audio callbacks, parameter state (APVTS), DSP execution, and UI metering.
 */
class FerruxeBassAudioProcessor : public juce::AudioProcessor
{
public:
    FerruxeBassAudioProcessor();
    ~FerruxeBassAudioProcessor() override = default;

    // Lifecycle & real-time audio processing
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    using AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // Editor & preset state serialization
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Plugin metadata
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override           { return false; }
    bool producesMidi() const override          { return false; }
    bool isMidiEffect() const override          { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override                                              { return 1; }
    int getCurrentProgram() override                                           { return 0; }
    void setCurrentProgram (int index) override                                { juce::ignoreUnused (index); }
    const juce::String getProgramName (int index) override                     { juce::ignoreUnused (index); return {}; }
    void changeProgramName (int index, const juce::String& newName) override   { juce::ignoreUnused (index, newName); }

    // Accessors for UI layer
    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept { return apvts; }
    ferruxe::MeterSnapshot consumeMeterSnapshot() noexcept           { return meterBridge.consumeForUiThread(); }

private:
    static float measureBufferPeak (const juce::AudioBuffer<float>& buffer, int numChannels) noexcept;

    juce::AudioProcessorValueTreeState apvts;
    ferruxe::ParameterReferences       parameters;
    ferruxe::dsp::FerruxeEngine        dspEngine;
    ferruxe::AudioMeterBridge          meterBridge;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FerruxeBassAudioProcessor)
};
