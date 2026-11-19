#include "OperatorPage.h"

static const char* opLetters[4] = { "A", "B", "C", "D" };

static void setupKnob (juce::Slider& s, juce::Label& l, juce::Component& parent, const juce::String& text)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 16);
    parent.addAndMakeVisible (s);
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::Font (11.0f));
    parent.addAndMakeVisible (l);
}

OperatorStrip::OperatorStrip (ScaleOperatorAudioProcessor& p, int opIndex)
{
    juce::String prefix = juce::String ("op") + opLetters[opIndex] + "_";

    title.setText (juce::String (opLetters[opIndex]), juce::dontSendNotification);
    title.setFont (juce::Font (18.0f, juce::Font::bold));
    title.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (title);

    addAndMakeVisible (waveBox);
    for (auto* n : { "Sine", "Triangle", "Saw", "Square" }) waveBox.addItem (n, waveBox.getNumItems() + 1);
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.apvts, prefix + "wave", waveBox));

    addAndMakeVisible (coarseBox);
    for (auto* n : { "0.5x","1x","2x","3x","4x","5x","6x","7x","8x","9x","10x","12x","16x" })
        coarseBox.addItem (n, coarseBox.getNumItems() + 1);
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (
        p.apvts, prefix + "coarse", coarseBox));

    setupKnob (fineSlider, fineLabel, *this, "Fine");
    setupKnob (levelSlider, levelLabel, *this, "Level");
    setupKnob (fixedHzSlider, fixedHzLabel, *this, "Fixed Hz");
    setupKnob (attackSlider, aLabel, *this, "Attack");
    setupKnob (decaySlider, dLabel, *this, "Decay");
    setupKnob (sustainSlider, sLabel, *this, "Sustain");
    setupKnob (releaseSlider, rLabel, *this, "Release");
    setupKnob (feedbackSlider, fbLabel, *this, "Feedback");

    addAndMakeVisible (fixedButton);
    fixedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        p.apvts, prefix + "fixed", fixedButton);

    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "fine", fineSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "level", levelSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "fixedHz", fixedHzSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "attack", attackSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "decay", decaySlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "sustain", sustainSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "release", releaseSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (p.apvts, prefix + "feedback", feedbackSlider));
}

void OperatorStrip::resized()
{
    auto area = getLocalBounds().reduced (4);
    title.setBounds (area.removeFromTop (22));
    waveBox.setBounds (area.removeFromTop (22));
    area.removeFromTop (4);
    coarseBox.setBounds (area.removeFromTop (22));
    fixedButton.setBounds (area.removeFromTop (22));

    auto knobRow = [&area] (int n)
    {
        return area.removeFromTop (70);
    };

    auto placeRow = [] (juce::Rectangle<int> row, std::initializer_list<std::pair<juce::Slider*, juce::Label*>> items)
    {
        const int w = row.getWidth() / (int) items.size();
        for (auto& it : items)
        {
            auto cell = row.removeFromLeft (w);
            it.second->setBounds (cell.removeFromTop (14));
            it.first->setBounds (cell);
        }
    };

    auto row1 = knobRow (3);
    placeRow (row1, { { &fineSlider, &fineLabel }, { &levelSlider, &levelLabel }, { &fixedHzSlider, &fixedHzLabel } });

    auto row2 = knobRow (3);
    placeRow (row2, { { &attackSlider, &aLabel }, { &decaySlider, &dLabel }, { &sustainSlider, &sLabel } });

    auto row3 = knobRow (2);
    placeRow (row3, { { &releaseSlider, &rLabel }, { &feedbackSlider, &fbLabel } });
}

OperatorPage::OperatorPage (ScaleOperatorAudioProcessor& p) : proc (p)
{
    titleLabel.setText ("Operator", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (20.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    for (int i = 0; i < 4; ++i)
    {
        operatorStrips[(size_t) i] = std::make_unique<OperatorStrip> (proc, i);
        addAndMakeVisible (*operatorStrips[(size_t) i]);
    }

    algoLabel.setText ("Algorithm", juce::dontSendNotification);
    addAndMakeVisible (algoLabel);
    addAndMakeVisible (algoBox);
    for (int i = 1; i <= 8; ++i) algoBox.addItem ("Algo " + juce::String (i), i);
    algoAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "algorithm", algoBox);

    filterLabel.setText ("Filter", juce::dontSendNotification);
    filterLabel.setFont (juce::Font (15.0f, juce::Font::bold));
    addAndMakeVisible (filterLabel);

    addAndMakeVisible (filterTypeBox);
    for (auto* n : { "LP", "HP", "BP" }) filterTypeBox.addItem (n, filterTypeBox.getNumItems() + 1);
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "filterType", filterTypeBox));

    addAndMakeVisible (filterSlopeBox);
    for (auto* n : { "12", "24" }) filterSlopeBox.addItem (n, filterSlopeBox.getNumItems() + 1);
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "filterSlope", filterSlopeBox));

    setupKnob (filterFreqSlider, freqLabel, *this, "Freq");
    setupKnob (filterResSlider, resLabel, *this, "Res");
    setupKnob (filterEnvSlider, envLabel, *this, "Env Amt");
    setupKnob (filterASlider, faLabel, *this, "Attack");
    setupKnob (filterDSlider, fdLabel, *this, "Decay");
    setupKnob (filterSSlider, fsLabel, *this, "Sustain");
    setupKnob (filterRSlider, frLabel, *this, "Release");

    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "filterCutoff", filterFreqSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "filterRes", filterResSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "filterEnvAmount", filterEnvSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "filterAttack", filterASlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "filterDecay", filterDSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "filterSustain", filterSSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "filterRelease", filterRSlider));

    lfoLabel.setText ("LFO", juce::dontSendNotification);
    lfoLabel.setFont (juce::Font (15.0f, juce::Font::bold));
    addAndMakeVisible (lfoLabel);
    addAndMakeVisible (lfoShapeBox);
    for (auto* n : { "Sine", "Triangle", "Square", "Saw", "Random" }) lfoShapeBox.addItem (n, lfoShapeBox.getNumItems() + 1);
    comboAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, "lfoShape", lfoShapeBox));

    setupKnob (lfoRateSlider, rateLabel, *this, "Rate");
    setupKnob (lfoPitchSlider, lfoPitchLabel, *this, "> Pitch");
    setupKnob (lfoFilterSlider, lfoFilterLabel, *this, "> Filter");
    setupKnob (lfoAmpSlider, lfoAmpLabel, *this, "> Amp");

    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "lfoRate", lfoRateSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "lfoPitch", lfoPitchSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "lfoFilter", lfoFilterSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "lfoAmp", lfoAmpSlider));

    pitchEnvLabel.setText ("Pitch Env", juce::dontSendNotification);
    pitchEnvLabel.setFont (juce::Font (15.0f, juce::Font::bold));
    addAndMakeVisible (pitchEnvLabel);
    setupKnob (pitchEnvAmtSlider, peAmtLabel, *this, "Amount");
    setupKnob (pitchEnvTimeSlider, peTimeLabel, *this, "Time");
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "pitchEnvAmount", pitchEnvAmtSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "pitchEnvTime", pitchEnvTimeSlider));

    globalLabel.setText ("Global", juce::dontSendNotification);
    globalLabel.setFont (juce::Font (15.0f, juce::Font::bold));
    addAndMakeVisible (globalLabel);
    setupKnob (volumeSlider, volLabel, *this, "Volume");
    setupKnob (voicesSlider, voicesLabel, *this, "Voices");
    setupKnob (glideSlider, glideLabel, *this, "Glide");
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "masterVolume", volumeSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "voices", voicesSlider));
    sliderAttachments.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, "glideTime", glideSlider));
}

void OperatorPage::resized()
{
    auto area = getLocalBounds().reduced (10);
    titleLabel.setBounds (area.removeFromTop (30));

    auto opsArea = area.removeFromTop (260);
    const int stripW = opsArea.getWidth() / 4;
    for (int i = 0; i < 4; ++i)
        operatorStrips[(size_t) i]->setBounds (opsArea.removeFromLeft (stripW).reduced (4));

    area.removeFromTop (8);
    auto algoRow = area.removeFromTop (26);
    algoLabel.setBounds (algoRow.removeFromLeft (80));
    algoBox.setBounds (algoRow.removeFromLeft (120));

    area.removeFromTop (8);
    auto bottom = area;
    const int colW = bottom.getWidth() / 4;

    auto filterCol = bottom.removeFromLeft (colW);
    filterLabel.setBounds (filterCol.removeFromTop (20));
    auto ftRow = filterCol.removeFromTop (22);
    filterTypeBox.setBounds (ftRow.removeFromLeft (colW / 2 - 4));
    filterSlopeBox.setBounds (ftRow);
    auto fKnobs1 = filterCol.removeFromTop (70);
    {
        int w = fKnobs1.getWidth() / 2;
        auto c1 = fKnobs1.removeFromLeft (w); freqLabel.setBounds (c1.removeFromTop (14)); filterFreqSlider.setBounds (c1);
        auto c2 = fKnobs1; resLabel.setBounds (c2.removeFromTop (14)); filterResSlider.setBounds (c2);
    }
    auto fKnobs2 = filterCol.removeFromTop (70);
    envLabel.setBounds (fKnobs2.removeFromTop (14)); filterEnvSlider.setBounds (fKnobs2);
    auto fKnobs3 = filterCol.removeFromTop (70);
    {
        int w = fKnobs3.getWidth() / 2;
        auto c1 = fKnobs3.removeFromLeft (w); faLabel.setBounds (c1.removeFromTop (14)); filterASlider.setBounds (c1);
        auto c2 = fKnobs3; fdLabel.setBounds (c2.removeFromTop (14)); filterDSlider.setBounds (c2);
    }
    auto fKnobs4 = filterCol.removeFromTop (70);
    {
        int w = fKnobs4.getWidth() / 2;
        auto c1 = fKnobs4.removeFromLeft (w); fsLabel.setBounds (c1.removeFromTop (14)); filterSSlider.setBounds (c1);
        auto c2 = fKnobs4; frLabel.setBounds (c2.removeFromTop (14)); filterRSlider.setBounds (c2);
    }

    auto lfoCol = bottom.removeFromLeft (colW);
    lfoLabel.setBounds (lfoCol.removeFromTop (20));
    lfoShapeBox.setBounds (lfoCol.removeFromTop (22));
    auto lKnobs1 = lfoCol.removeFromTop (70);
    rateLabel.setBounds (lKnobs1.removeFromTop (14)); lfoRateSlider.setBounds (lKnobs1);
    auto lKnobs2 = lfoCol.removeFromTop (70);
    lfoPitchLabel.setBounds (lKnobs2.removeFromTop (14)); lfoPitchSlider.setBounds (lKnobs2);
    auto lKnobs3 = lfoCol.removeFromTop (70);
    lfoFilterLabel.setBounds (lKnobs3.removeFromTop (14)); lfoFilterSlider.setBounds (lKnobs3);
    auto lKnobs4 = lfoCol.removeFromTop (70);
    lfoAmpLabel.setBounds (lKnobs4.removeFromTop (14)); lfoAmpSlider.setBounds (lKnobs4);

    auto peCol = bottom.removeFromLeft (colW);
    pitchEnvLabel.setBounds (peCol.removeFromTop (20));
    auto peKnobs = peCol.removeFromTop (70);
    peAmtLabel.setBounds (peKnobs.removeFromTop (14)); pitchEnvAmtSlider.setBounds (peKnobs);
    auto peKnobs2 = peCol.removeFromTop (70);
    peTimeLabel.setBounds (peKnobs2.removeFromTop (14)); pitchEnvTimeSlider.setBounds (peKnobs2);

    auto gCol = bottom;
    globalLabel.setBounds (gCol.removeFromTop (20));
    auto gKnobs = gCol.removeFromTop (70);
    volLabel.setBounds (gKnobs.removeFromTop (14)); volumeSlider.setBounds (gKnobs);
    auto gKnobs2 = gCol.removeFromTop (70);
    voicesLabel.setBounds (gKnobs2.removeFromTop (14)); voicesSlider.setBounds (gKnobs2);
    auto gKnobs3 = gCol.removeFromTop (70);
    glideLabel.setBounds (gKnobs3.removeFromTop (14)); glideSlider.setBounds (gKnobs3);
}
