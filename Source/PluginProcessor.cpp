#include "PluginProcessor.h"
#include "PluginEditor.h"

FerruxeBassAudioProcessor::FerruxeBassAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", ferruxe::ParameterReferences::createLayout()),
      parameters (apvts)
#else
    : apvts (*this, nullptr, "Parameters", ferruxe::ParameterReferences::createLayout()),
      parameters (apvts)
#endif
{
}

void FerruxeBassAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec {};
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (juce::jmax (1, samplesPerBlock));
    spec.numChannels      = static_cast<juce::uint32> (juce::jmax (1, getTotalNumInputChannels()));

    dspEngine.prepare (spec);
    dspEngine.updateParameters (parameters.readCurrentValues(), /*snapImmediately=*/true);

    setLatencySamples (dspEngine.getLatencySamples());
}

void FerruxeBassAudioProcessor::releaseResources()
{
    dspEngine.reset();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool FerruxeBassAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto outputSet = layouts.getMainOutputChannelSet();

    if (outputSet != juce::AudioChannelSet::mono() && outputSet != juce::AudioChannelSet::stereo())
        return false;

    return outputSet == layouts.getMainInputChannelSet();
}
#endif

void FerruxeBassAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    const int numInputChannels  = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples        = buffer.getNumSamples();

    for (int ch = numInputChannels; ch < numOutputChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    const float inputPeak = measureBufferPeak (buffer, numInputChannels);

    dspEngine.updateParameters (parameters.readCurrentValues());
    const auto processResult = dspEngine.process (buffer);

    const float outputPeak = measureBufferPeak (buffer, numInputChannels);
    meterBridge.publishFromAudioThread (inputPeak,
                                        outputPeak,
                                        processResult.peakGainReductionDb,
                                        processResult.ceilingClipped);
}

float FerruxeBassAudioProcessor::measureBufferPeak (const juce::AudioBuffer<float>& buffer, int numChannels) noexcept
{
    const int numSamples = buffer.getNumSamples();
    float peak = 0.0f;

    for (int ch = 0; ch < numChannels; ++ch)
        peak = juce::jmax (peak, buffer.getMagnitude (ch, 0, numSamples));

    return peak;
}

juce::AudioProcessorEditor* FerruxeBassAudioProcessor::createEditor()
{
    return new FerruxeBassAudioProcessorEditor (*this);
}

void FerruxeBassAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void FerruxeBassAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
    {
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FerruxeBassAudioProcessor();
}
