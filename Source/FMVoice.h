#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>

// Trivial "always matches" sound, since this plugin is a single-patch instrument.
class FMSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

enum class OpWave { Sine = 0, Triangle, Saw, Square };

// Eight fixed modulation routings in the spirit of classic 4-operator FM algorithms.
// modSources[target] is a bitmask (bit0=A, bit1=B, bit2=C, bit3=D) of operators that
// modulate the phase of `target`. carrierMask is a bitmask of operators that are summed
// into the audible output. Operators are always processed D, C, B, A so a modulator's
// output is already known by the time it is needed.
struct FMAlgorithm
{
    int modSources[4];
    int carrierMask;
};

static constexpr FMAlgorithm kAlgorithms[8] =
{
    // 0: chain D->C->B->A, A is the only carrier
    { { 0b0010, 0b0100, 0b1000, 0b0000 }, 0b0001 },
    // 1: two parallel stacks, B->A and D->C
    { { 0b0010, 0b0000, 0b1000, 0b0000 }, 0b0101 },
    // 2: B, C and D all modulate A
    { { 0b1110, 0b0000, 0b0000, 0b0000 }, 0b0001 },
    // 3: pure additive, no FM at all
    { { 0b0000, 0b0000, 0b0000, 0b0000 }, 0b1111 },
    // 4: D modulates A, B and C in parallel
    { { 0b1000, 0b1000, 0b1000, 0b0000 }, 0b0111 },
    // 5: C->B->A chain, D is an independent carrier
    { { 0b0010, 0b0100, 0b0000, 0b0000 }, 0b1001 },
    // 6: C->A and D->C, B is an independent carrier
    { { 0b0100, 0b0000, 0b1000, 0b0000 }, 0b0011 },
    // 7: same chain as 0, intended to be used with feedback dialled in on D
    { { 0b0010, 0b0100, 0b1000, 0b0000 }, 0b0001 },
};

struct OperatorParams
{
    OpWave wave = OpWave::Sine;
    float coarseRatio = 1.0f;
    float fineSemitones = 0.0f;
    float levelDb = 0.0f;
    bool fixed = false;
    float fixedHz = 440.0f;
    float attackMs = 0.0f;
    float decayMs = 600.0f;
    float sustain = 0.0f;
    float releaseMs = 50.0f;
    float feedback = 0.0f;
};

struct VoiceParams
{
    OperatorParams ops[4];
    int algorithm = 0;

    int filterType = 0;     // 0 = LP, 1 = HP, 2 = BP
    int filterSlope = 0;    // 0 = 12dB, 1 = 24dB
    float filterCutoffHz = 8000.0f;
    float filterResonance = 0.5f;
    float filterEnvAmount = 0.0f; // bipolar, in octaves
    float filterAttackMs = 0.0f;
    float filterDecayMs = 400.0f;
    float filterSustain = 1.0f;
    float filterReleaseMs = 50.0f;

    int lfoShape = 0; // 0 sine 1 tri 2 square 3 saw 4 random
    float lfoRateHz = 4.0f;
    float lfoPitchSemitones = 0.0f;
    float lfoFilterAmount = 0.0f;
    float lfoAmpAmount = 0.0f;

    float pitchEnvSemitones = 0.0f;
    float pitchEnvTimeMs = 100.0f;

    float masterVolumeDb = -6.0f;
    float glideTimeMs = 0.0f;
};

class FMVoice : public juce::SynthesiserVoice
{
public:
    explicit FMVoice (const VoiceParams& paramsRef) : params (paramsRef) {}

    bool canPlaySound (juce::SynthesiserSound* sound) override
    {
        return dynamic_cast<FMSound*> (sound) != nullptr;
    }

    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    void setCurrentPlaybackSampleRate (double newRate) override;

private:
    const VoiceParams& params;

    double targetFrequencyHz = 440.0;
    double currentFrequencyHz = 440.0;
    float velocityLevel = 1.0f;
    bool noteOn = false;

    double opPhase[4] { 0, 0, 0, 0 };
    float opLastOutput[4] { 0, 0, 0, 0 };
    juce::ADSR opEnv[4];

    juce::ADSR filterEnv;
    juce::dsp::StateVariableTPTFilter<float> filterStage1, filterStage2;

    double lfoPhase = 0.0;
    juce::Random random;
    float lastRandomValue = 0.0f;

    double pitchEnvTime = 0.0;

    static float renderOsc (OpWave wave, double phase);
    float computeLfo (float rateHz);
    void updateFilterCoefficients (float cutoffHz, float resonance);
};
