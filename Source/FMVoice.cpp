#include "FMVoice.h"

void FMVoice::setCurrentPlaybackSampleRate (double newRate)
{
    juce::SynthesiserVoice::setCurrentPlaybackSampleRate (newRate);
    if (newRate > 0)
    {
        juce::dsp::ProcessSpec spec { newRate, 512, 1 };
        filterStage1.prepare (spec);
        filterStage2.prepare (spec);
        for (auto& e : opEnv)
            e.setSampleRate (newRate);
        filterEnv.setSampleRate (newRate);
    }
}

float FMVoice::renderOsc (OpWave wave, double phase)
{
    const double p = phase - std::floor (phase); // wrap to 0..1
    switch (wave)
    {
        case OpWave::Sine:     return (float) std::sin (juce::MathConstants<double>::twoPi * p);
        case OpWave::Triangle: return (float) (4.0 * std::abs (p - 0.5) - 1.0);
        case OpWave::Saw:      return (float) (2.0 * p - 1.0);
        case OpWave::Square:   return p < 0.5 ? 1.0f : -1.0f;
    }
    return 0.0f;
}

float FMVoice::computeLfo (float rateHz)
{
    lfoPhase += rateHz / getSampleRate();
    if (lfoPhase >= 1.0) lfoPhase -= std::floor (lfoPhase);

    switch (params.lfoShape)
    {
        case 0: return (float) std::sin (juce::MathConstants<double>::twoPi * lfoPhase);
        case 1: return (float) (4.0 * std::abs (lfoPhase - 0.5) - 1.0);
        case 2: return lfoPhase < 0.5f ? 1.0f : -1.0f;
        case 3: return (float) (2.0 * lfoPhase - 1.0);
        case 4:
            if (lfoPhase < (rateHz / (float) getSampleRate()))
                lastRandomValue = random.nextFloat() * 2.0f - 1.0f;
            return lastRandomValue;
        default: return 0.0f;
    }
}

void FMVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    targetFrequencyHz = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    if (! noteOn)
        currentFrequencyHz = targetFrequencyHz; // no glide from silence
    noteOn = true;
    velocityLevel = velocity;
    pitchEnvTime = 0.0;

    for (int i = 0; i < 4; ++i)
    {
        const auto& op = params.ops[i];
        opEnv[i].setParameters ({ juce::jmax (0.0001f, op.attackMs / 1000.0f),
                                   juce::jmax (0.0001f, op.decayMs / 1000.0f),
                                   op.sustain,
                                   juce::jmax (0.0001f, op.releaseMs / 1000.0f) });
        opEnv[i].noteOn();
        opPhase[i] = 0.0;
        opLastOutput[i] = 0.0f;
    }

    filterEnv.setParameters ({ juce::jmax (0.0001f, params.filterAttackMs / 1000.0f),
                                juce::jmax (0.0001f, params.filterDecayMs / 1000.0f),
                                params.filterSustain,
                                juce::jmax (0.0001f, params.filterReleaseMs / 1000.0f) });
    filterEnv.noteOn();
}

void FMVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        for (auto& e : opEnv) e.noteOff();
        filterEnv.noteOff();
        noteOn = false;
    }
    else
    {
        clearCurrentNote();
        for (auto& e : opEnv) e.reset();
        filterEnv.reset();
        noteOn = false;
    }
}

void FMVoice::updateFilterCoefficients (float cutoffHz, float resonance)
{
    cutoffHz = juce::jlimit (20.0f, 20000.0f, cutoffHz);
    const float q = juce::jmap (resonance, 0.0f, 1.0f, 0.707f, 8.0f);

    auto type = juce::dsp::StateVariableTPTFilterType::lowpass;
    if (params.filterType == 1) type = juce::dsp::StateVariableTPTFilterType::highpass;
    else if (params.filterType == 2) type = juce::dsp::StateVariableTPTFilterType::bandpass;

    filterStage1.setType (type);
    filterStage1.setCutoffFrequency (cutoffHz);
    filterStage1.setResonance (q);

    filterStage2.setType (type);
    filterStage2.setCutoffFrequency (cutoffHz);
    filterStage2.setResonance (q);
}

void FMVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    if (! isVoiceActive() && ! noteOn)
        return;

    const auto& algo = kAlgorithms[(size_t) juce::jlimit (0, 7, params.algorithm)];
    const double sr = getSampleRate();
    const double glideSamplesInv = params.glideTimeMs > 0.0f ? 1.0 / (params.glideTimeMs / 1000.0 * sr) : 1.0;

    for (int s = 0; s < numSamples; ++s)
    {
        // Glide towards target pitch
        if (currentFrequencyHz != targetFrequencyHz)
        {
            const double diff = targetFrequencyHz - currentFrequencyHz;
            const double step = diff * juce::jmin (1.0, glideSamplesInv);
            currentFrequencyHz += step;
            if (std::abs (targetFrequencyHz - currentFrequencyHz) < 0.01)
                currentFrequencyHz = targetFrequencyHz;
        }

        // Pitch envelope: decays from pitchEnvSemitones down to 0 over pitchEnvTimeMs
        pitchEnvTime += 1000.0 / sr;
        const double envT = params.pitchEnvTimeMs > 0.0f ? juce::jlimit (0.0, 1.0, pitchEnvTime / params.pitchEnvTimeMs) : 1.0;
        const float pitchEnvNow = params.pitchEnvSemitones * (float) (1.0 - envT);

        const float lfoValue = computeLfo (params.lfoRateHz);
        const float lfoPitch = lfoValue * params.lfoPitchSemitones;

        const double noteFreq = currentFrequencyHz * std::pow (2.0, (pitchEnvNow + lfoPitch) / 12.0);

        float outputs[4] { 0, 0, 0, 0 };

        for (int idx = 3; idx >= 0; --idx)
        {
            const auto& op = params.ops[idx];

            double opFreq = op.fixed ? (double) op.fixedHz
                                      : noteFreq * (double) op.coarseRatio * std::pow (2.0, op.fineSemitones / 12.0);

            opPhase[idx] += opFreq / sr;
            if (opPhase[idx] >= 1.0) opPhase[idx] -= std::floor (opPhase[idx]);

            float modSum = 0.0f;
            const int sources = algo.modSources[idx];
            for (int src = 0; src < 4; ++src)
                if ((sources & (1 << src)) != 0)
                    modSum += outputs[src];

            const float feedbackTerm = op.feedback > 0.0f ? opLastOutput[idx] * op.feedback : 0.0f;
            const double modulatedPhase = opPhase[idx] + (double) modSum * 2.0 + (double) feedbackTerm;

            const float raw = renderOsc (op.wave, modulatedPhase);
            const float envVal = opEnv[idx].getNextSample();
            const float levelLin = juce::Decibels::decibelsToGain (op.levelDb, -60.0f);

            outputs[idx] = raw * envVal * levelLin;
            opLastOutput[idx] = outputs[idx];
        }

        float voiceOut = 0.0f;
        for (int i = 0; i < 4; ++i)
            if ((algo.carrierMask & (1 << i)) != 0)
                voiceOut += outputs[i] * 0.25f; // headroom for up to 4 summed carriers

        // Filter, with envelope + LFO modulating cutoff
        const float filterEnvVal = filterEnv.getNextSample();
        const float cutoffMod = params.filterEnvAmount * filterEnvVal * 12.0f /*octaves->semitone-ish scaling*/
                                 + params.lfoFilterAmount * lfoValue * 24.0f;
        const float modCutoff = params.filterCutoffHz * std::pow (2.0f, cutoffMod / 12.0f);
        updateFilterCoefficients (modCutoff, params.filterResonance);

        voiceOut = filterStage1.processSample (0, voiceOut);
        if (params.filterSlope == 1)
            voiceOut = filterStage2.processSample (0, voiceOut);

        // Amp LFO (tremolo)
        const float ampLfo = 1.0f - params.lfoAmpAmount * 0.5f * (1.0f - lfoValue);
        voiceOut *= ampLfo;

        voiceOut *= velocityLevel;
        voiceOut *= juce::Decibels::decibelsToGain (params.masterVolumeDb, -60.0f);

        for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
            outputBuffer.addSample (ch, startSample + s, voiceOut);

        if (! noteOn && ! opEnv[0].isActive())
        {
            clearCurrentNote();
            break;
        }
    }
}
