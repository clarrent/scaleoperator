#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "FMVoice.h"
#include "ScaleMapper.h"

class ScaleOperatorAudioProcessor : public juce::AudioProcessor
{
public:
    ScaleOperatorAudioProcessor();
    ~ScaleOperatorAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Scale Operator"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    static constexpr int kNumVoices = 16;

private:
    juce::Synthesiser synth;
    VoiceParams voiceParams;
    ScaleMapper scaleMapper;

    void updateVoiceParamsFromState();
    void updateScaleMapperFromState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScaleOperatorAudioProcessor)
};
