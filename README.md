# Scale Operator — FL Studio-compatible VST3

A single instrument plugin that mimics two Ableton devices:

- **Page 1: Scale** — remaps incoming MIDI notes using a 12x12 pitch-class
  map (click a cell to remap that input note, click it again to delete/mute
  it), with Base, Transpose (+/-36 st), Fold, and a Range/Lowest window.
  Notes outside the range pass through unaltered, same as the original.
- **Page 2: Operator** — a 4-operator FM synth (A/B/C/D), each with its own
  waveform, coarse ratio, fine tune, level, fixed-frequency mode, and ADSR
  envelope, 8 selectable FM routing algorithms, a resonant filter (LP/HP/BP,
  12/24 dB, with its own envelope), an LFO (5 shapes, routable to pitch,
  filter and amp), a pitch envelope, and global volume/voices/glide.

The Scale stage remaps MIDI before it reaches the Operator-style engine, so
this is one plugin, not two — exactly what you asked for ("page 1, page 2").

This is a functional recreation built to match the *behavior* described in
the Ableton manual pages you shared, not a byte-for-byte clone of Ableton's
DSP or UI.

## What you're getting

A complete JUCE/C++ source project (not a pre-built binary — I can't compile
a Windows VST3 in this sandbox, and you'll get a plugin that's actually
signed/built for your machine this way). You build it once with a free
toolchain and FL Studio will see it like any other VST3.

## Build instructions (Windows, for FL Studio)

**One-time setup:** install **Visual Studio 2022 Community** (free) with
the "Desktop development with C++" workload checked — that workload
includes CMake, so you don't need to install anything else separately.
https://visualstudio.microsoft.com/vs/community/

**Then, every time you want to (re)build:**

1. Unzip this project anywhere, e.g. `C:\dev\ScaleOperator`.
2. Double-click **`build.bat`** in the project folder.

That's it — the script checks for CMake, downloads JUCE the first time
(needs internet, takes a few minutes), compiles the plugin, and copies
`Scale Operator.vst3` straight into your system VST3 folder
(`C:\Program Files\Common Files\VST3`). It'll print clear errors and wait
for a keypress if anything goes wrong, so it won't just flash and vanish.
3. In FL Studio: **Options → Manage Plugins → Find Plugins** (rescan). The
   plugin will show up as **Scale Operator** under Generators/Instruments.

(If you'd rather run it by hand, the two commands `build.bat` runs are
`cmake -B build -G "Visual Studio 17 2022" -A x64` then
`cmake --build build --config Release`.)

## Build instructions (macOS)

```
cmake -B build -G Xcode
cmake --build build --config Release
```
Copies to `~/Library/Audio/Plug-Ins/VST3`. (FL Studio for Mac reads VST3
from the same location as other hosts.)

## Notes / honest limitations

- **Voices** is currently advisory — all 16 voices are always allocated;
  it doesn't yet hard-limit polyphony. Easy to tighten up if you want true
  voice stealing at a lower count.
- Filter modulation (envelope + LFO) is applied as a semitone-ish shift
  for a musical sweep rather than a precisely calibrated octave curve —
  tweak the scaling constants in `FMVoice.cpp` (`updateFilterCoefficients`
  call site) if you want it steeper/shallower.
- If you change the Scale map while a note is still held, that note's
  note-off is recomputed with the *current* map, which can occasionally
  leave a note ringing if you repatch mid-note. Rare in practice, but
  worth knowing.
- No preset browser yet — DAW-level save/load (FL's own preset slots /
  project save) works fine via `getStateInformation`/`setStateInformation`.

## Where to look if you want to extend it

- `Source/ScaleMapper.h` — the note-remap logic (Page 1)
- `Source/FMVoice.h/.cpp` — the FM engine, algorithms, filter, LFO (Page 2)
- `Source/PluginProcessor.cpp` — all parameters are defined in
  `createParameterLayout()` near the top — add/rename params there
- `Source/ScalePage.cpp` / `Source/OperatorPage.cpp` — the two UI pages
