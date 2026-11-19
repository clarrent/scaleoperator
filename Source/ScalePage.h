#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// A 12x12 clickable grid mimicking Ableton Scale's X-Y map: one square
// per input pitch-class column, placed on the row of its mapped output
// pitch-class. Click a cell to remap that column; click the lit cell again
// to delete (mute) that input note.
class ScaleGrid : public juce::Component
{
public:
    explicit ScaleGrid (ScaleOperatorAudioProcessor& p);
    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void refresh();

private:
    ScaleOperatorAudioProcessor& proc;
    static constexpr int kSize = 12;
};

class ScalePage : public juce::Component, private juce::Timer
{
public:
    explicit ScalePage (ScaleOperatorAudioProcessor& p);

private:
    ScaleOperatorAudioProcessor& proc;
    ScaleGrid grid;

    juce::ComboBox baseBox;
    juce::Slider transposeSlider, rangeSlider, lowestSlider;
    juce::ToggleButton foldButton { "Fold" };
    juce::Label baseLabel, transposeLabel, rangeLabel, lowestLabel, titleLabel;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> foldAttachment;

    void resized() override;
    void timerCallback() override { grid.refresh(); }
};
