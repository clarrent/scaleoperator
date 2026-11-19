#include "PluginEditor.h"

ScaleOperatorAudioProcessorEditor::ScaleOperatorAudioProcessorEditor (ScaleOperatorAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), scalePage (p), operatorPage (p)
{
    setSize (1000, 620);
    setResizable (true, true);
    setResizeLimits (800, 500, 1600, 1000);

    addAndMakeVisible (scaleTab);
    addAndMakeVisible (operatorTab);
    addAndMakeVisible (scalePage);
    addAndMakeVisible (operatorPage);

    scaleTab.onClick = [this] { showPage (0); };
    operatorTab.onClick = [this] { showPage (1); };

    showPage (0);
}

void ScaleOperatorAudioProcessorEditor::showPage (int index)
{
    scalePage.setVisible (index == 0);
    operatorPage.setVisible (index == 1);
    scaleTab.setToggleState (index == 0, juce::dontSendNotification);
    operatorTab.setToggleState (index == 1, juce::dontSendNotification);
}

void ScaleOperatorAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff2b2b2b));
}

void ScaleOperatorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    auto tabs = area.removeFromTop (34);
    scaleTab.setBounds (tabs.removeFromLeft (getWidth() / 2));
    operatorTab.setBounds (tabs);

    scalePage.setBounds (area);
    operatorPage.setBounds (area);
}
