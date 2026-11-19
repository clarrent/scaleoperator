#pragma once
#include <array>
#include <juce_core/juce_core.h>

// Reimplementation of the behaviour of Ableton's "Scale" MIDI effect:
// every incoming note is mapped, by pitch class, to a chosen outgoing
// pitch class. Notes can be deleted (muted), folded back into a +/-6
// semitone window, transposed, and limited to a note range.
class ScaleMapper
{
public:
    static constexpr int kNumNotes = 12;

    // mapping[inputPitchClass] = outputPitchClass, or -1 if that input is "deleted"
    std::array<int, kNumNotes> mapping { 0,1,2,3,4,5,6,7,8,9,10,11 };

    int base = 0;                 // root note, 0 = C .. 11 = B
    int transposeSemitones = 0;   // -36 .. +36
    bool fold = false;
    int lowestNote = 0;           // 0..127, bottom of the active range
    int rangeSemitones = 127;     // width of the active range

    // Returns the mapped MIDI note number, or -1 if the note should be dropped entirely.
    // Notes outside [lowestNote, lowestNote + rangeSemitones] pass through unaltered,
    // matching Scale's behaviour of leaving out-of-range notes untouched.
    int mapNote (int noteNumber) const
    {
        if (noteNumber < lowestNote || noteNumber > lowestNote + rangeSemitones)
            return noteNumber;

        const int pitchClass = ((noteNumber - base) % kNumNotes + kNumNotes) % kNumNotes;
        const int octave = (noteNumber - base - pitchClass) / kNumNotes;

        const int mappedClass = mapping[(size_t) pitchClass];
        if (mappedClass < 0)
            return -1;

        int outNote = base + octave * kNumNotes + mappedClass;

        if (fold)
        {
            int offset = outNote - noteNumber;
            while (offset > 6)  { outNote -= kNumNotes; offset -= kNumNotes; }
            while (offset < -6) { outNote += kNumNotes; offset += kNumNotes; }
        }

        outNote += transposeSemitones;
        return juce::jlimit (0, 127, outNote);
    }
};
