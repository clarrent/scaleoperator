#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ScalePage.h"
#include "OperatorPage.h"

class ScaleOperatorAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ScaleOperatorAudioProcessorEditor (ScaleOperatorAudioProcessor&);

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ScaleOperatorAudioProcessor& processorRef;

    juce::TextButton scaleTab { "Page 1: Scale" };
    juce::TextButton operatorTab { "Page 2: Operator" };
    ScalePage scalePage;
    OperatorPage operatorPage;

    void showPage (int index);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScaleOperatorAudioProcessorEditor)
};
