#include "PluginProcessor.h"
#include "PluginEditor.h"

static const char* opLetters[4] = { "A", "B", "C", "D" };

juce::AudioProcessorValueTreeState::ParameterLayout ScaleOperatorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    using namespace juce;

    // ---- Scale page ----
    StringArray noteNames { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    params.push_back (std::make_unique<AudioParameterChoice> ("scaleBase", "Base", noteNames, 0));
    params.push_back (std::make_unique<AudioParameterFloat> ("scaleTranspose", "Transpose",
        NormalisableRange<float> (-36.0f, 36.0f, 1.0f), 0.0f));
    params.push_back (std::make_unique<AudioParameterBool> ("scaleFold", "Fold", false));
    params.push_back (std::make_unique<AudioParameterInt> ("scaleRange", "Range", 0, 127, 127));
    params.push_back (std::make_unique<AudioParameterInt> ("scaleLowest", "Lowest", 0, 127, 0));

    StringArray mapChoices;
    mapChoices.add ("Off");
    for (auto& n : noteNames) mapChoices.add (n);
    for (int i = 0; i < 12; ++i)
        params.push_back (std::make_unique<AudioParameterChoice> ("scaleMap" + String (i), "Map " + noteNames[i],
            mapChoices, i + 1));

    // ---- Operator page: 4 operators ----
    StringArray waveNames { "Sine", "Triangle", "Saw", "Square" };
    StringArray ratioNames { "0.5x","1x","2x","3x","4x","5x","6x","7x","8x","9x","10x","12x","16x" };

    for (int i = 0; i < 4; ++i)
    {
        String p = String ("op") + opLetters[i] + "_";
        params.push_back (std::make_unique<AudioParameterChoice> (p + "wave", String (opLetters[i]) + " Wave", waveNames, 0));
        params.push_back (std::make_unique<AudioParameterChoice> (p + "coarse", String (opLetters[i]) + " Coarse", ratioNames, 1));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "fine", String (opLetters[i]) + " Fine",
            NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "level", String (opLetters[i]) + " Level",
            NormalisableRange<float> (-60.0f, 0.0f, 0.01f), i == 0 ? 0.0f : -60.0f));
        params.push_back (std::make_unique<AudioParameterBool> (p + "fixed", String (opLetters[i]) + " Fixed", false));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "fixedHz", String (opLetters[i]) + " Fixed Hz",
            NormalisableRange<float> (0.1f, 20000.0f, 0.1f, 0.3f), 440.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "attack", String (opLetters[i]) + " Attack",
            NormalisableRange<float> (0.0f, 10000.0f, 1.0f, 0.3f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "decay", String (opLetters[i]) + " Decay",
            NormalisableRange<float> (0.0f, 10000.0f, 1.0f, 0.3f), 600.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "sustain", String (opLetters[i]) + " Sustain",
            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "release", String (opLetters[i]) + " Release",
            NormalisableRange<float> (0.0f, 10000.0f, 1.0f, 0.3f), 50.0f));
        params.push_back (std::make_unique<AudioParameterFloat> (p + "feedback", String (opLetters[i]) + " Feedback",
            NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    }

    StringArray algoNames;
    for (int i = 1; i <= 8; ++i) algoNames.add ("Algo " + String (i));
    params.push_back (std::make_unique<AudioParameterChoice> ("algorithm", "Algorithm", algoNames, 0));

    // ---- Filter ----
    StringArray filterTypes { "LP", "HP", "BP" };
    StringArray filterSlopes { "12", "24" };
    params.push_back (std::make_unique<AudioParameterChoice> ("filterType", "Filter Type", filterTypes, 0));
    params.push_back (std::make_unique<AudioParameterChoice> ("filterSlope", "Filter Slope", filterSlopes, 0));
    params.push_back (std::make_unique<AudioParameterFloat> ("filterCutoff", "Filter Freq",
        NormalisableRange<float> (20.0f, 20000.0f, 1.0f, 0.3f), 8000.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("filterRes", "Filter Res",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.3f));
    params.push_back (std::make_unique<AudioParameterFloat> ("filterEnvAmount", "Filter Env Amt",
        NormalisableRange<float> (-1.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("filterAttack", "Filter Attack",
        NormalisableRange<float> (0.0f, 10000.0f, 1.0f, 0.3f), 0.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("filterDecay", "Filter Decay",
        NormalisableRange<float> (0.0f, 10000.0f, 1.0f, 0.3f), 400.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("filterSustain", "Filter Sustain",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 1.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("filterRelease", "Filter Release",
        NormalisableRange<float> (0.0f, 10000.0f, 1.0f, 0.3f), 50.0f));

    // ---- LFO ----
    StringArray lfoShapes { "Sine", "Triangle", "Square", "Saw", "Random" };
    params.push_back (std::make_unique<AudioParameterChoice> ("lfoShape", "LFO Shape", lfoShapes, 0));
    params.push_back (std::make_unique<AudioParameterFloat> ("lfoRate", "LFO Rate",
        NormalisableRange<float> (0.01f, 20.0f, 0.01f, 0.3f), 4.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("lfoPitch", "LFO>Pitch",
        NormalisableRange<float> (0.0f, 24.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("lfoFilter", "LFO>Filter",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("lfoAmp", "LFO>Amp",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f));

    // ---- Pitch envelope ----
    params.push_back (std::make_unique<AudioParameterFloat> ("pitchEnvAmount", "Pitch Env Amt",
        NormalisableRange<float> (-48.0f, 48.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<AudioParameterFloat> ("pitchEnvTime", "Pitch Env Time",
        NormalisableRange<float> (1.0f, 5000.0f, 1.0f, 0.3f), 100.0f));

    // ---- Global ----
    params.push_back (std::make_unique<AudioParameterFloat> ("masterVolume", "Volume",
        NormalisableRange<float> (-60.0f, 6.0f, 0.01f), -6.0f));
    params.push_back (std::make_unique<AudioParameterInt> ("voices", "Voices", 1, 16, 8));
    params.push_back (std::make_unique<AudioParameterFloat> ("glideTime", "Glide",
        NormalisableRange<float> (0.0f, 2000.0f, 1.0f, 0.3f), 0.0f));

    return { params.begin(), params.end() };
}

ScaleOperatorAudioProcessor::ScaleOperatorAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    for (int i = 0; i < kNumVoices; ++i)
        synth.addVoice (new FMVoice (voiceParams));
    synth.addSound (new FMSound());
}

void ScaleOperatorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    juce::ignoreUnused (samplesPerBlock);
}

bool ScaleOperatorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        || layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono();
}

static float ratioFromChoiceIndex (int index)
{
    static const float ratios[13] = { 0.5f,1,2,3,4,5,6,7,8,9,10,12,16 };
    return ratios[juce::jlimit (0, 12, index)];
}

void ScaleOperatorAudioProcessor::updateScaleMapperFromState()
{
    scaleMapper.base = (int) apvts.getRawParameterValue ("scaleBase")->load();
    scaleMapper.transposeSemitones = (int) apvts.getRawParameterValue ("scaleTranspose")->load();
    scaleMapper.fold = apvts.getRawParameterValue ("scaleFold")->load() > 0.5f;
    scaleMapper.rangeSemitones = (int) apvts.getRawParameterValue ("scaleRange")->load();
    scaleMapper.lowestNote = (int) apvts.getRawParameterValue ("scaleLowest")->load();

    for (int i = 0; i < 12; ++i)
    {
        int choiceIndex = (int) apvts.getRawParameterValue ("scaleMap" + juce::String (i))->load();
        scaleMapper.mapping[(size_t) i] = choiceIndex == 0 ? -1 : choiceIndex - 1;
    }
}

void ScaleOperatorAudioProcessor::updateVoiceParamsFromState()
{
    for (int i = 0; i < 4; ++i)
    {
        juce::String p = juce::String ("op") + opLetters[i] + "_";
        auto& op = voiceParams.ops[i];
        op.wave = (OpWave) (int) apvts.getRawParameterValue (p + "wave")->load();
        op.coarseRatio = ratioFromChoiceIndex ((int) apvts.getRawParameterValue (p + "coarse")->load());
        op.fineSemitones = apvts.getRawParameterValue (p + "fine")->load();
        op.levelDb = apvts.getRawParameterValue (p + "level")->load();
        op.fixed = apvts.getRawParameterValue (p + "fixed")->load() > 0.5f;
        op.fixedHz = apvts.getRawParameterValue (p + "fixedHz")->load();
        op.attackMs = apvts.getRawParameterValue (p + "attack")->load();
        op.decayMs = apvts.getRawParameterValue (p + "decay")->load();
        op.sustain = apvts.getRawParameterValue (p + "sustain")->load();
        op.releaseMs = apvts.getRawParameterValue (p + "release")->load();
        op.feedback = apvts.getRawParameterValue (p + "feedback")->load();
    }

    voiceParams.algorithm = (int) apvts.getRawParameterValue ("algorithm")->load();

    voiceParams.filterType = (int) apvts.getRawParameterValue ("filterType")->load();
    voiceParams.filterSlope = (int) apvts.getRawParameterValue ("filterSlope")->load();
    voiceParams.filterCutoffHz = apvts.getRawParameterValue ("filterCutoff")->load();
    voiceParams.filterResonance = apvts.getRawParameterValue ("filterRes")->load();
    voiceParams.filterEnvAmount = apvts.getRawParameterValue ("filterEnvAmount")->load();
    voiceParams.filterAttackMs = apvts.getRawParameterValue ("filterAttack")->load();
    voiceParams.filterDecayMs = apvts.getRawParameterValue ("filterDecay")->load();
    voiceParams.filterSustain = apvts.getRawParameterValue ("filterSustain")->load();
    voiceParams.filterReleaseMs = apvts.getRawParameterValue ("filterRelease")->load();

    voiceParams.lfoShape = (int) apvts.getRawParameterValue ("lfoShape")->load();
    voiceParams.lfoRateHz = apvts.getRawParameterValue ("lfoRate")->load();
    voiceParams.lfoPitchSemitones = apvts.getRawParameterValue ("lfoPitch")->load();
    voiceParams.lfoFilterAmount = apvts.getRawParameterValue ("lfoFilter")->load();
    voiceParams.lfoAmpAmount = apvts.getRawParameterValue ("lfoAmp")->load();

    voiceParams.pitchEnvSemitones = apvts.getRawParameterValue ("pitchEnvAmount")->load();
    voiceParams.pitchEnvTimeMs = apvts.getRawParameterValue ("pitchEnvTime")->load();

    voiceParams.masterVolumeDb = apvts.getRawParameterValue ("masterVolume")->load();
    voiceParams.glideTimeMs = apvts.getRawParameterValue ("glideTime")->load();

    int wantedVoices = (int) apvts.getRawParameterValue ("voices")->load();
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<FMVoice*> (synth.getVoice (i)))
            juce::ignoreUnused (v); // voice count is advisory only (all voices stay allocated; see README)
    juce::ignoreUnused (wantedVoices);
}

void ScaleOperatorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    updateScaleMapperFromState();
    updateVoiceParamsFromState();

    juce::MidiBuffer remapped;
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        const int time = metadata.samplePosition;

        if (msg.isNoteOn() || msg.isNoteOff())
        {
            const int mapped = scaleMapper.mapNote (msg.getNoteNumber());
            if (mapped < 0)
                continue; // deleted by the scale map

            auto newMsg = msg.isNoteOn()
                ? juce::MidiMessage::noteOn (msg.getChannel(), mapped, msg.getVelocity())
                : juce::MidiMessage::noteOff (msg.getChannel(), mapped, msg.getVelocity());
            remapped.addEvent (newMsg, time);
        }
        else
        {
            remapped.addEvent (msg, time);
        }
    }

    synth.renderNextBlock (buffer, remapped, 0, buffer.getNumSamples());
}

void ScaleOperatorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); true)
    {
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
    }
}

void ScaleOperatorAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* ScaleOperatorAudioProcessor::createEditor()
{
    return new ScaleOperatorAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ScaleOperatorAudioProcessor();
}
