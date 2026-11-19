#include "ScalePage.h"

ScaleGrid::ScaleGrid (ScaleOperatorAudioProcessor& p) : proc (p)
{
    setInterceptsMouseClicks (true, false);
}

void ScaleGrid::paint (juce::Graphics& g)
{
    const float cell = (float) juce::jmin (getWidth(), getHeight()) / (float) kSize;
    static const bool blackKey[12] = { false,true,false,true,false,false,true,false,true,false,true,false };

    for (int col = 0; col < kSize; ++col)
    {
        for (int row = 0; row < kSize; ++row)
        {
            juce::Rectangle<float> r (col * cell, (kSize - 1 - row) * cell, cell - 1.0f, cell - 1.0f);
            g.setColour (blackKey[row] ? juce::Colours::darkgrey.darker() : juce::Colours::darkgrey);
            g.fillRect (r);
        }
    }

    int choiceIndex = (int) proc.apvts.getRawParameterValue ("scaleMap0")->load();
    for (int col = 0; col < kSize; ++col)
    {
        choiceIndex = (int) proc.apvts.getRawParameterValue ("scaleMap" + juce::String (col))->load();
        if (choiceIndex == 0)
            continue; // deleted column, nothing lit
        const int row = choiceIndex - 1;
        juce::Rectangle<float> r (col * cell, (kSize - 1 - row) * cell, cell - 1.0f, cell - 1.0f);
        g.setColour (juce::Colours::lightblue);
        g.fillRect (r);
    }

    g.setColour (juce::Colours::white.withAlpha (0.3f));
    g.drawRect (getLocalBounds());
}

void ScaleGrid::mouseDown (const juce::MouseEvent& e)
{
    const float cell = (float) juce::jmin (getWidth(), getHeight()) / (float) kSize;
    const int col = juce::jlimit (0, kSize - 1, (int) (e.position.x / cell));
    const int row = juce::jlimit (0, kSize - 1, kSize - 1 - (int) (e.position.y / cell));

    auto* param = proc.apvts.getParameter ("scaleMap" + juce::String (col));
    if (param == nullptr) return;

    int current = (int) proc.apvts.getRawParameterValue ("scaleMap" + juce::String (col))->load();
    int newChoiceIndex = (current == row + 1) ? 0 : row + 1; // click lit cell again -> delete

    param->setValueNotifyingHost (param->convertTo0to1 ((float) newChoiceIndex));
    repaint();
}

void ScaleGrid::refresh() { repaint(); }

ScalePage::ScalePage (ScaleOperatorAudioProcessor& p) : proc (p), grid (p)
{
    addAndMakeVisible (grid);

    titleLabel.setText ("Scale", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (20.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    addAndMakeVisible (baseBox);
    for (auto* n : { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" })
        baseBox.addItem (n, baseBox.getNumItems() + 1);
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        proc.apvts, "scaleBase", baseBox));
    baseLabel.setText ("Base", juce::dontSendNotification);
    addAndMakeVisible (baseLabel);

    auto setupSlider = [this] (juce::Slider& s, juce::Label& l, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::LinearHorizontal);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
        addAndMakeVisible (s);
        l.setText (text, juce::dontSendNotification);
        addAndMakeVisible (l);
    };

    setupSlider (transposeSlider, transposeLabel, "Transpose");
    setupSlider (rangeSlider, rangeLabel, "Range");
    setupSlider (lowestSlider, lowestLabel, "Lowest");

    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, "scaleTranspose", transposeSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, "scaleRange", rangeSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, "scaleLowest", lowestSlider));

    addAndMakeVisible (foldButton);
    foldAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        proc.apvts, "scaleFold", foldButton);

    startTimerHz (10);
}

void ScalePage::resized()
{
    auto area = getLocalBounds().reduced (10);
    titleLabel.setBounds (area.removeFromTop (30));

    auto gridArea = area.removeFromLeft (juce::jmin (area.getWidth() - 220, area.getHeight()));
    grid.setBounds (gridArea.withSizeKeepingCentre (juce::jmin (gridArea.getWidth(), gridArea.getHeight()),
                                                     juce::jmin (gridArea.getWidth(), gridArea.getHeight())));

    area.removeFromLeft (10);
    auto controls = area;

    auto row1 = controls.removeFromTop (30);
    baseLabel.setBounds (row1.removeFromLeft (60));
    baseBox.setBounds (row1);

    controls.removeFromTop (10);
    auto row2 = controls.removeFromTop (24);
    transposeLabel.setBounds (row2.removeFromLeft (70));
    transposeSlider.setBounds (row2);

    controls.removeFromTop (6);
    auto row3 = controls.removeFromTop (24);
    rangeLabel.setBounds (row3.removeFromLeft (70));
    rangeSlider.setBounds (row3);

    controls.removeFromTop (6);
    auto row4 = controls.removeFromTop (24);
    lowestLabel.setBounds (row4.removeFromLeft (70));
    lowestSlider.setBounds (row4);

    controls.removeFromTop (10);
    foldButton.setBounds (controls.removeFromTop (24));
}
