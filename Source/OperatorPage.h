#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// One operator's worth of controls (wave, coarse, fine, level, fixed, ADSR, feedback)
class OperatorStrip : public juce::Component
{
public:
    OperatorStrip (ScaleOperatorAudioProcessor& p, int opIndex);
    void resized() override;

private:
    juce::Label title;
    juce::ComboBox waveBox, coarseBox;
    juce::Slider fineSlider, levelSlider, fixedHzSlider;
    juce::ToggleButton fixedButton { "Fixed" };
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider, feedbackSlider;
    juce::Label fineLabel, levelLabel, fixedHzLabel, aLabel, dLabel, sLabel, rLabel, fbLabel;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fixedAttachment;
};

class OperatorPage : public juce::Component
{
public:
    explicit OperatorPage (ScaleOperatorAudioProcessor& p);
    void resized() override;

private:
    ScaleOperatorAudioProcessor& proc;
    juce::Label titleLabel;

    std::array<std::unique_ptr<OperatorStrip>, 4> operatorStrips;

    juce::ComboBox algoBox;
    juce::Label algoLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algoAttachment;

    juce::ComboBox filterTypeBox, filterSlopeBox;
    juce::Slider filterFreqSlider, filterResSlider, filterEnvSlider,
                 filterASlider, filterDSlider, filterSSlider, filterRSlider;
    juce::Label filterLabel, freqLabel, resLabel, envLabel, faLabel, fdLabel, fsLabel, frLabel;

    juce::ComboBox lfoShapeBox;
    juce::Slider lfoRateSlider, lfoPitchSlider, lfoFilterSlider, lfoAmpSlider;
    juce::Label lfoLabel, rateLabel, lfoPitchLabel, lfoFilterLabel, lfoAmpLabel;

    juce::Slider pitchEnvAmtSlider, pitchEnvTimeSlider;
    juce::Label pitchEnvLabel, peAmtLabel, peTimeLabel;

    juce::Slider volumeSlider, voicesSlider, glideSlider;
    juce::Label globalLabel, volLabel, voicesLabel, glideLabel;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
};
