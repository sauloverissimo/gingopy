# 🪇 Gingo

An expressive music theory and rhythm toolkit for Python, powered by a C++17 core.

[![PyPI version](https://badge.fury.io/py/gingo.svg)](https://badge.fury.io/py/gingo)
[![Python 3.10+](https://img.shields.io/badge/python-3.10+-blue.svg)](https://www.python.org/downloads/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

From pitch classes to harmonic trees and rhythmic grids — with audio playback and a friendly CLI.

Notes, intervals, chords, scales, and harmonic fields are just the beginning: Gingo also ships with durations, tempo markings (nomes de tempo), time signatures, and sequence playback.

**Português (pt-BR)**: https://sauloverissimo.github.io/gingo/ (guia e referência completos)

---

## About

Gingo is a pragmatic library for analysis, composition, and teaching. It prioritizes correctness, ergonomics, and speed, while keeping the API compact and consistent across concepts.

**Highlights**

- **C++17 core + Python API** — fast and deterministic, with full type hints.
- **Pitch & harmony** — `Note`, `Interval`, `Chord`, `Scale`, `Field`, and `Tree` (beta) with identification, deduction, and comparison utilities.
- **Rhythm & time** — `Duration`, `Tempo` (BPM + nomes de tempo), `TimeSignature`, and `Sequence` with note/chord events.
- **Audio** — `.play()` and `.to_wav()` on musical objects, plus CLI `--play` / `--wav` with waveform and strum controls.
- **CLI-first exploration** — query and inspect theory concepts without leaving the terminal.

---

---

## Installation

```bash
pip install gingo
```

Optional audio playback dependency:

```bash
pip install "gingo[audio]"
```

Requires Python 3.10+. Pre-built binary wheels are available for Linux, macOS, and Windows — no C++17 compiler needed. If no wheel is available for your platform, pip will build from source automatically.

---

## Quick Start

```python
from gingo import (
    Note, Interval, Chord, Scale, Field, Tree, ScaleType,
    Duration, Tempo, TimeSignature, Sequence,
    NoteEvent, ChordEvent, Rest,
)

# Notes
note = Note("Bb")
note.natural()      # "A#"
note.semitone()     # 10
note.frequency(4)   # 466.16 Hz
note.play(octave=4) # Listen to Bb4

# Intervals
iv = Interval("5J")
iv.semitones()      # 7
iv.anglo_saxon()    # "P5"

# Chords
chord = Chord("Cm7")
chord.root()        # Note("C")
chord.type()        # "m7"
chord.notes()       # [Note("C"), Note("Eb"), Note("G"), Note("Bb")]
chord.interval_labels()  # ["P1", "3m", "5J", "7m"]
chord.play()        # Listen to Cm7

# Identify a chord from notes
Chord.identify(["C", "E", "G"])  # Chord("CM")

# Identify a scale or field from a full note/chord set
Scale.identify(["C", "D", "E", "F", "G", "A", "B"])  # Scale("C", "major")
Field.identify(["CM", "Dm", "Em", "FM", "GM", "Am"])  # Field("C", "major")

# Deduce likely fields from partial evidence (ranked)
matches = Field.deduce(["CM", "FM"])
matches[0].field   # Field("C", "major") or Field("F", "major")
matches[0].score   # 1.0

# Compare two chords (absolute, context-free)
r = Chord("CM").compare(Chord("Am"))
r.common_notes       # [Note("C"), Note("E")]
r.root_distance      # 3
r.transformation     # "R" (neo-Riemannian Relative)
r.transposition      # -1 (not related by transposition)
r.dissonance_a       # 0.057... (psychoacoustic roughness)
r.to_dict()          # full dict serialization

# Scales
scale = Scale("C", ScaleType.Major)
[n.natural() for n in scale.notes()]  # ["C", "D", "E", "F", "G", "A", "B"]
scale.degree(5)     # Note("G")
scale.play()        # Listen to C major scale

# Harmonic fields
field = Field("C", ScaleType.Major)
[c.name() for c in field.chords()]
# ["CM", "Dm", "Em", "FM", "GM", "Am", "Bdim"]

# Compare two chords within a harmonic field (contextual)
r = field.compare(Chord("CM"), Chord("GM"))
r.degree_a           # 1 (I)
r.degree_b           # 5 (V)
r.function_a         # HarmonicFunction.Tonic
r.function_b         # HarmonicFunction.Dominant
r.root_motion        # "ascending_fifth"
r.to_dict()          # full dict serialization

# Harmonic trees (progressions and voice leading)
tree = Tree("C", ScaleType.Major)
# Tree is currently beta (content & references are still growing).
tree.branches()      # All available harmonic branches
tree.paths("I")      # All progressions from tonic
tree.shortest_path("I", "V7")  # ["I", "V7"]
tree.is_valid_progression(["IIm", "V7", "I"])  # True
tree.function("V7")  # HarmonicFunction.Dominant
tree.to_dot()        # Export to Graphviz
tree.to_mermaid()    # Export to Mermaid diagram

# Rhythm
q = Duration("quarter")
dotted = Duration("eighth", dots=1)
triplet = Duration("eighth", tuplet=3)
Tempo("Allegro").bpm()         # 140.0
Tempo(120).marking()           # "Allegretto"
TimeSignature(6, 8).classification()  # "compound"

# Sequence (events in time)
seq = Sequence(Tempo(120), TimeSignature(4, 4))
seq.add(NoteEvent(Note("C"), Duration("quarter"), octave=4))
seq.add(ChordEvent(Chord("G7"), Duration("half"), octave=4))
seq.add(Rest(Duration("quarter")))
seq.total_seconds()

# Audio
Note("C").play()
Chord("Am7").play(waveform="square")
Scale("C", "major").to_wav("c_major.wav")
```

---

## CLI (quick exploration)

```bash
gingo note C#
gingo note C --fifths
gingo interval 7 --all
gingo scale "C major" --degree 5 5
gingo scale "C,D,E,F,G,A,B" --identify
gingo field "C major" --functions
gingo field "CM,FM,G7" --identify
gingo field "CM,FM" --deduce
gingo compare CM GM --field "C major"
gingo note C --play --waveform triangle
gingo chord Am7 --play --strum 0.05
gingo chord Am7 --wav am7.wav
gingo duration quarter --tempo 120
gingo tempo Allegro --all
gingo timesig 6 8 --tempo 120
```

Audio flags:

- `--play` outputs to the system audio device
- `--wav FILE` exports a WAV file
- `--waveform` (`sine`, `square`, `sawtooth`, `triangle`)
- `--strum` and `--gap` control timing between chord tones and events

---

## Detailed Guide

### Note

The `Note` class is the atomic unit of the library. It represents a single pitch class (C, D, E, F, G, A, B) with optional accidentals.

```python
from gingo import Note

# Construction — accepts any common notation
c  = Note("C")      # Natural
bb = Note("Bb")     # Flat
fs = Note("F#")     # Sharp
eb = Note("E♭")     # Unicode flat
gs = Note("G##")    # Double sharp

# Core properties
bb.name()           # "Bb"   — the original input
bb.natural()        # "A#"   — canonical sharp-based form
bb.sound()          # "B"    — base letter (no accidentals)
bb.semitone()       # 10     — chromatic position (C=0, C#=1, ..., B=11)

# Frequency calculation (A4 = 440 Hz standard tuning)
Note("A").frequency(4)    # 440.0 Hz
Note("A").frequency(3)    # 220.0 Hz
Note("C").frequency(4)    # 261.63 Hz
Note("A").frequency(5)    # 880.0 Hz

# Enharmonic equivalence
Note("Bb").is_enharmonic(Note("A#"))   # True
Note("Db").is_enharmonic(Note("C#"))   # True
Note("C").is_enharmonic(Note("D"))     # False

# Equality (compares natural forms)
Note("Bb") == Note("A#")   # True — same natural form
Note("C") == Note("C")     # True
Note("C") != Note("D")     # True

# Transposition
Note("C").transpose(7)     # Note("G")  — up a perfect fifth
Note("C").transpose(12)    # Note("C")  — up an octave
Note("A").transpose(-2)    # Note("G")  — down a whole step
Note("E").transpose(1)     # Note("F")  — up a semitone

# Audio playback (requires gingo[audio])
Note("A").play(octave=4)                    # A4 (440 Hz)
Note("C").play(octave=5, waveform="square") # C5 with square wave
Note("Eb").to_wav("eb.wav", octave=4)       # Export to WAV file

# Static utilities
Note.to_natural("Bb")              # "A#"
Note.to_natural("G##")             # "A"
Note.to_natural("Bbb")             # "A"
Note.extract_root("C#m7")          # "C#"
Note.extract_root("Bbdim")         # "Bb"
Note.extract_sound("Gb")           # "G"
Note.extract_type("C#m7")          # "m7"
Note.extract_type("F#m7(b5)")      # "m7(b5)"
Note.extract_type("C")             # ""
```

#### Enharmonic Resolution Table

Gingo resolves 89 enharmonic spellings to a canonical sharp-based form:

| Input | Natural | Category |
|-------|---------|----------|
| `Bb` | `A#` | Standard flat |
| `Db` | `C#` | Standard flat |
| `Eb` | `D#` | Standard flat |
| `Gb` | `F#` | Standard flat |
| `Ab` | `G#` | Standard flat |
| `E#` | `F` | Special sharp (no sharp exists) |
| `B#` | `C` | Special sharp (no sharp exists) |
| `Fb` | `E` | Special flat (no flat exists) |
| `Cb` | `B` | Special flat (no flat exists) |
| `G##` | `A` | Double sharp |
| `C##` | `D` | Double sharp |
| `E##` | `F#` | Double sharp |
| `Bbb` | `A` | Double flat |
| `Abb` | `G` | Double flat |
| `B♭` | `A#` | Unicode flat symbol |
| `E♭♭` | `D` | Unicode double flat |
| `♭♭G` | `F` | Prefix accidentals |

---

### Interval

The `Interval` class represents the distance between two pitches, covering two full octaves (24 semitones).

```python
from gingo import Interval

# Construction — from label or semitone count
p1 = Interval("P1")     # Perfect unison
m3 = Interval("3m")     # Minor third
M3 = Interval("3M")     # Major third
p5 = Interval("5J")     # Perfect fifth
m7 = Interval("7m")     # Minor seventh

# From semitone count
iv = Interval(7)         # Same as Interval("5J")

# Properties
m3.label()        # "3m"
m3.anglo_saxon()  # "mi3"
m3.semitones()    # 3
m3.degree()       # 3
m3.octave()       # 1

# Second octave intervals
b9 = Interval("b9")
b9.semitones()    # 13
b9.octave()       # 2

# Equality (by semitone distance)
Interval("P1") == Interval(0)    # True
Interval("5J") == Interval(7)    # True
```

#### All 24 Interval Labels

| Semitones | Label | Anglo-Saxon | Degree |
|-----------|-------|-------------|--------|
| 0 | P1 | P1 | 1 |
| 1 | 2m | mi2 | 2 |
| 2 | 2M | ma2 | 2 |
| 3 | 3m | mi3 | 3 |
| 4 | 3M | ma3 | 3 |
| 5 | 4J | P4 | 4 |
| 6 | d5 | d5 | 5 |
| 7 | 5J | P5 | 5 |
| 8 | #5 | mi6 | 6 |
| 9 | M6 | ma6 | 6 |
| 10 | 7m | mi7 | 7 |
| 11 | 7M | ma7 | 7 |
| 12 | 8J | P8 | 8 |
| 13 | b9 | mi9 | 9 |
| 14 | 9 | ma9 | 9 |
| 15 | #9 | mi10 | 10 |
| 16 | b11 | ma10 | 10 |
| 17 | 11 | P11 | 11 |
| 18 | #11 | d11 | 11 |
| 19 | 5 | P12 | 12 |
| 20 | b13 | mi13 | 13 |
| 21 | 13 | ma13 | 13 |
| 22 | #13 | mi14 | 14 |
| 23 | bI | ma14 | 14 |

---

### Chord

The `Chord` class represents a musical chord — a root note plus a set of intervals from a database of 42 chord formulas.

```python
from gingo import Chord, Note

# Construction from name
cm   = Chord("CM")           # C major
dm7  = Chord("Dm7")          # D minor seventh
bb7m = Chord("Bb7M")         # Bb major seventh
fsdim = Chord("F#dim")       # F# diminished

# Root, type, and name
cm.root()                    # Note("C")
cm.root().natural()          # "C"
cm.type()                    # "M"
cm.name()                    # "CM"

# Notes — with correct enharmonic spelling
[n.name() for n in Chord("CM").notes()]
# ["C", "E", "G"]

[n.name() for n in Chord("Am7").notes()]
# ["A", "C", "E", "G"]

[n.name() for n in Chord("Dbm7").notes()]
# ["Db", "Fb", "Ab", "Cb"]  — proper flat spelling

# Notes can also be accessed as natural (sharp-based) canonical form
[n.natural() for n in Chord("Dbm7").notes()]
# ["C#", "E", "G#", "B"]

# Interval structure
Chord("Am7").interval_labels()
# ["P1", "3m", "5J", "7m"]

Chord("CM").interval_labels()
# ["P1", "3M", "5J"]

Chord("Bdim").interval_labels()
# ["P1", "3m", "d5"]

# Size
Chord("CM").size()      # 3 (triad)
Chord("Am7").size()     # 4 (seventh chord)
Chord("G7").size()      # 4

# Contains — check if a note belongs to the chord
Chord("CM").contains(Note("E"))    # True
Chord("CM").contains(Note("F"))    # False

# Identify chord from notes (reverse lookup)
c = Chord.identify(["C", "E", "G"])
c.name()                # "CM"
c.type()                # "M"

c2 = Chord.identify(["D", "F#", "A", "C#", "E"])
c2.type()               # "9"

# Equality
Chord("CM") == Chord("CM")     # True
Chord("CM") != Chord("Cm")     # True

# Audio playback (requires gingo[audio])
Chord("Am7").play()                          # Play Am7 chord
Chord("G7").play(waveform="sawtooth")       # Custom waveform
Chord("Dm").play(strum=0.05)                 # Arpeggiated/strummed
Chord("CM").to_wav("cmajor.wav", octave=4)  # Export to WAV file
```

#### Supported Chord Types (42 formulas)

**Triads (7):** M, m, dim, aug, sus2, sus4, 5

**Seventh chords (10):** 7, m7, 7M, m7M, dim7, m7(b5), 7(b5), 7(#5), 7M(#5), sus7

**Sixth chords (3):** 6, m6, 6(9)

**Ninth chords (4):** 9, m9, M9, sus9

**Extended chords (6):** 11, m11, m7(11), 13, m13, M13

**Altered chords (6):** 7(b9), 7(#9), 7(#11), 13(#11), (b9), (b13)

**Add chords (4):** add9, add2, add11, add4

**Other (2):** sus, 7+5

---

### Scale

The `Scale` class builds a scale from a tonic note and a scale pattern. It supports 10 parent families, mode names, pentatonic filters, and a chainable API.

```python
from gingo import Scale, ScaleType, Note

# Construction — from enum, string, or mode name
s1 = Scale("C", ScaleType.Major)
s2 = Scale("C", "major")              # string form
s3 = Scale("D", "dorian")             # mode name → Major, mode 2
s4 = Scale("E", "phrygian dominant")  # mode name → HarmonicMinor, mode 5
s5 = Scale("C", "altered")            # mode name → MelodicMinor, mode 7

# Scale identity
d = Scale("D", "dorian")
d.parent()        # ScaleType.Major
d.mode_number()   # 2
d.mode_name()     # "Dorian"
d.quality()       # "minor"
d.brightness()    # 3

# Scale notes (with correct enharmonic spelling)
[n.name() for n in Scale("C", "major").notes()]
# ["C", "D", "E", "F", "G", "A", "B"]

[n.name() for n in Scale("D", "dorian").notes()]
# ["D", "E", "F", "G", "A", "B", "C"]

[n.name() for n in Scale("Gb", "major").notes()]
# ["Gb", "Ab", "Bb", "Cb", "Db", "Eb", "F"]

# Natural form (canonical sharp-based) also available
[n.natural() for n in Scale("Gb", "major").notes()]
# ["F#", "G#", "A#", "B", "C#", "D#", "F"]

# Degree access (1-indexed, supports chaining)
s = Scale("C", "major")
s.degree(1)        # Note("C")  — tonic
s.degree(5)        # Note("G")  — dominant
s.degree(5, 5)     # Note("D")  — V of V
s.degree(5, 5, 3)  # Note("F")  — III of V of V

# Walk: navigate along the scale
s.walk(1, 4)       # Note("F")  — from I, a fourth = IV
s.walk(5, 5)       # Note("D")  — from V, a fifth = II

# Modes by number or name
s.mode(2)                 # D Dorian
s.mode("lydian")          # F Lydian

# Pentatonic
s.pentatonic()                        # C major pentatonic (5 notes)
Scale("C", "major pentatonic")        # same thing
Scale("A", "minor pentatonic")        # A C D E G

# Color notes (what distinguishes this mode from a reference)
Scale("C", "dorian").colors("ionian")  # [Eb, Bb]

# Other families
Scale("C", "whole tone").size()   # 6
Scale("A", "blues").size()        # 6
Scale("C", "chromatic").size()    # 12
Scale("C", "diminished").size()   # 8

# Audio playback (requires gingo[audio])
Scale("C", "major").play()                      # Play C major scale
Scale("D", "dorian").play(waveform="triangle") # Custom waveform
Scale("A", "minor").to_wav("a_minor.wav")      # Export to WAV file
```

#### Scale Types (10 parent families)

| Type | Notes | Pattern | Description |
|------|:-----:|---------|-------------|
| `Major` | 7 | W-W-H-W-W-W-H | Ionian mode, the most common Western scale |
| `NaturalMinor` | 7 | W-H-W-W-H-W-W | Aeolian mode, relative minor |
| `HarmonicMinor` | 7 | W-H-W-W-H-A2-H | Raised 7th degree, characteristic V7 chord |
| `MelodicMinor` | 7 | W-H-W-W-W-W-H | Raised 6th and 7th degrees (ascending) |
| `HarmonicMajor` | 7 | W-W-H-W-H-A2-H | Major with lowered 6th degree |
| `Diminished` | 8 | W-H-W-H-W-H-W-H | Symmetric octatonic scale |
| `WholeTone` | 6 | W-W-W-W-W-W | Symmetric whole-tone scale |
| `Augmented` | 6 | A2-H-A2-H-A2-H | Symmetric augmented scale |
| `Blues` | 6 | m3-W-H-H-m3-W | Minor pentatonic + blue note |
| `Chromatic` | 12 | H-H-H-H-H-H-H-H-H-H-H-H | All 12 pitch classes |

W = whole step, H = half step, A2 = augmented second, m3 = minor third

---

### Field (Harmonic Field)

The `Field` class generates the diatonic chords built from each degree of a scale — the harmonic field.

```python
from gingo import Field, ScaleType, HarmonicFunction

# Construction
f = Field("C", ScaleType.Major)

# Triads (3-note chords on each degree)
triads = f.chords()
[c.name() for c in triads]
# ["CM", "Dm", "Em", "FM", "GM", "Am", "Bdim"]
#   I     ii   iii    IV    V    vi   vii°

# Seventh chords (4-note chords on each degree)
sevenths = f.sevenths()
[c.name() for c in sevenths]
# ["CM7", "Dm7", "Em7", "FM7", "G7", "Am7", "Bm7(b5)"]
#  Imaj7  ii7   iii7  IVmaj7  V7   vi7   vii-7(b5)

# Access by degree (1-indexed)
f.chord(1)                  # Chord("CM")
f.chord(5)                  # Chord("GM")
f.seventh(5)                # Chord("G7")

# Harmonic function (Tonic / Subdominant / Dominant)
f.function(1)               # HarmonicFunction.Tonic
f.function(5)               # HarmonicFunction.Dominant
f.function(5).name          # "Dominant"
f.function(5).short         # "D"

# Role within function group
f.role(1)                   # "primary"
f.role(6)                   # "relative of I"

# Query by chord name or object
f.function("FM")            # HarmonicFunction.Subdominant
f.function("F#M")           # None (not in the field)
f.role("Am")                # "relative of I"

# Applied chords (tonicization)
f.applied("V7", 2)          # Chord("A7")  — V7 of degree II
f.applied("V7", "V")        # Chord("D7")  — V7 of degree V
f.applied("IIm7(b5)", 5)    # Chord("Am7(b5)")
f.applied(5, 2)             # Chord("A7")  — numeric shorthand

# Number of degrees
f.size()                    # 7

# Works with any scale type
f_minor = Field("A", ScaleType.HarmonicMinor)
[c.name() for c in f_minor.chords()]
# Harmonic minor field: Am, Bdim, Caug, Dm, EM, FM, G#dim
```

---

### Tree (Harmonic Tree / Progressions) — beta

The `Tree` class represents harmonic progressions and voice leading paths within a scale's harmonic field. Based on José de Alencar's harmonic tree theory.

**⚠️ Status: beta** — This feature is under active study and development. Due to limited bibliographic references available, the current implementation may contain errors or incomplete patterns. Use with caution and validate results against your harmonic analysis needs.

```python
from gingo import Tree, ScaleType, HarmonicFunction

# Construction
tree = Tree("C", ScaleType.Major)

# List all available harmonic branches
branches = tree.branches()
# ["I", "IIm", "IIIm", "IV", "V7", "VIm", "VIIdim", "V7/IV", "IVm", "bVI", "bVII", ...]

# Get all possible paths from a branch
paths = tree.paths("I")
for path in paths[:3]:
    print(f"{path.id}: {path.branch} → {path.chord.name()}")
# 0: I → CM
# 1: IIm / IV → Dm
# 2: VIm → Am

# Path information
path = paths[1]
path.branch              # "IIm / IV"
path.chord               # Chord object
path.chord.name()        # "Dm"
path.interval_labels     # ["P1", "3m", "5J"]
path.note_names          # ["D", "F", "A"]

# Find shortest path between two branches
path = tree.shortest_path("I", "V7")
# ["I", "V7"]

path = tree.shortest_path("I", "IV")
# ["I", "VIm", "IV"] or another valid path

# Validate a progression
tree.is_valid_progression(["IIm", "V7", "I"])     # True (II-V-I)
tree.is_valid_progression(["I", "IV", "V7"])      # True
tree.is_valid_progression(["I", "INVALID"])       # False

# Harmonic function classification
tree.function("I")       # HarmonicFunction.Tonic
tree.function("IV")      # HarmonicFunction.Subdominant
tree.function("V7")      # HarmonicFunction.Dominant
tree.function("VIm")     # HarmonicFunction.Tonic (relative)

# Get all branches with a specific function
tonics = tree.branches_with_function(HarmonicFunction.Tonic)
# ["I", "VIm", ...]

dominants = tree.branches_with_function(HarmonicFunction.Dominant)
# ["V7", "VIIdim", ...]

# Export to visualization formats
dot = tree.to_dot(show_functions=True)
# Graphviz DOT format with color-coded functions

mermaid = tree.to_mermaid()
# Mermaid diagram format

# Works with minor scales
tree_minor = Tree("A", ScaleType.NaturalMinor)
tree_minor.branches()
# ["Im", "IIdim", "bIII", "IVm", "Vm", "bVI", "bVII", ...]
```

---

## API Reference Summary

### Note

| Method | Returns | Description |
|--------|---------|-------------|
| `Note(name)` | `Note` | Construct from any notation |
| `.name()` | `str` | Original input name |
| `.natural()` | `str` | Canonical sharp form |
| `.sound()` | `str` | Base letter only |
| `.semitone()` | `int` | Chromatic index 0-11 |
| `.frequency(octave=4)` | `float` | Concert pitch in Hz |
| `.is_enharmonic(other)` | `bool` | Same pitch class? |
| `.transpose(semitones)` | `Note` | Shifted note |
| `Note.to_natural(name)` | `str` | Static: resolve spelling |
| `Note.extract_root(name)` | `str` | Static: root from chord name |
| `Note.extract_sound(name)` | `str` | Static: base letter from name |
| `Note.extract_type(name)` | `str` | Static: chord type suffix |

### Interval

| Method | Returns | Description |
|--------|---------|-------------|
| `Interval(label)` | `Interval` | From label string |
| `Interval(semitones)` | `Interval` | From semitone count |
| `.label()` | `str` | Short label |
| `.anglo_saxon()` | `str` | Anglo-Saxon formal name |
| `.semitones()` | `int` | Semitone distance |
| `.degree()` | `int` | Diatonic degree number |
| `.octave()` | `int` | Octave (1 or 2) |

### Chord

| Method | Returns | Description |
|--------|---------|-------------|
| `Chord(name)` | `Chord` | From chord name |
| `.name()` | `str` | Full chord name |
| `.root()` | `Note` | Root note |
| `.type()` | `str` | Quality suffix |
| `.notes()` | `list[Note]` | Chord tones (natural) |
| `.formal_notes()` | `list[Note]` | Chord tones (diatonic spelling) |
| `.intervals()` | `list[Interval]` | Interval objects |
| `.interval_labels()` | `list[str]` | Interval label strings |
| `.size()` | `int` | Number of notes |
| `.contains(note)` | `bool` | Note membership test |
| `.compare(other)` | `ChordComparison` | Detailed comparison (18 dimensions) |
| `Chord.identify(names)` | `Chord` | Static: reverse lookup |

### Scale

| Method | Returns | Description |
|--------|---------|-------------|
| `Scale(tonic, type)` | `Scale` | From tonic + ScaleType/string/mode name |
| `.tonic()` | `Note` | Tonic note |
| `.parent()` | `ScaleType` | Parent family (Major, HarmonicMinor, ...) |
| `.mode_number()` | `int` | Mode number (1-7) |
| `.mode_name()` | `str` | Mode name (Ionian, Dorian, ...) |
| `.quality()` | `str` | Tonal quality ("major" / "minor") |
| `.brightness()` | `int` | Brightness (1=Locrian, 7=Lydian) |
| `.is_pentatonic()` | `bool` | Whether pentatonic filter is active |
| `.type()` | `ScaleType` | Scale type enum (backward compat, = parent) |
| `.modality()` | `Modality` | Modality enum (backward compat) |
| `.notes()` | `list[Note]` | Scale notes (natural) |
| `.formal_notes()` | `list[Note]` | Scale notes (diatonic) |
| `.degree(*degrees)` | `Note` | Chained degree: `degree(5, 5)` = V of V |
| `.walk(start, *steps)` | `Note` | Walk: `walk(1, 4)` = IV |
| `.size()` | `int` | Number of notes |
| `.contains(note)` | `bool` | Note membership |
| `.mode(n_or_name)` | `Scale` | Mode by number (int) or name (str) |
| `.pentatonic()` | `Scale` | Pentatonic version of the scale |
| `.colors(reference)` | `list[Note]` | Notes differing from a reference mode |
| `.mask()` | `list[int]` | 24-bit active positions |
| `Scale.parse_type(name)` | `ScaleType` | Static: string to enum |
| `Scale.parse_modality(name)` | `Modality` | Static: string to enum |
| `Scale.identify(notes)` | `Scale` | Static: detect scale from full note set |

### Field

| Method | Returns | Description |
|--------|---------|-------------|
| `Field(tonic, type)` | `Field` | From tonic + ScaleType/string |
| `.tonic()` | `Note` | Tonic note |
| `.scale()` | `Scale` | Underlying scale |
| `.chords()` | `list[Chord]` | Triads per degree |
| `.sevenths()` | `list[Chord]` | Seventh chords per degree |
| `.chord(degree)` | `Chord` | Triad at degree N |
| `.seventh(degree)` | `Chord` | 7th chord at degree N |
| `.applied(func, target)` | `Chord` | Applied chord (tonicization) |
| `.function(degree)` | `HarmonicFunction` | Harmonic function (T/S/D) |
| `.function(chord)` | `HarmonicFunction?` | Function by chord (None if not in field) |
| `.role(degree)` | `str` | Role: "primary", "relative of I", etc. |
| `.role(chord)` | `str?` | Role by chord (None if not in field) |
| `.compare(a, b)` | `FieldComparison` | Contextual comparison (21 dimensions) |
| `.size()` | `int` | Number of degrees |
| `Field.identify(items)` | `Field` | Static: detect field from full notes/chords |
| `Field.deduce(items, limit=10)` | `list[FieldMatch]` | Static: ranked candidates from partial input |

### Tree

| Method | Returns | Description |
|--------|---------|-------------|
| `Tree(tonic, type)` | `Tree` | From tonic + ScaleType/string |
| `.tonic()` | `Note` | Tonic note |
| `.type()` | `ScaleType` | Scale type |
| `.branches()` | `list[str]` | All harmonic branches |
| `.paths(branch)` | `list[HarmonicPath]` | All paths from a branch |
| `.shortest_path(from, to)` | `list[str]` | Shortest progression |
| `.is_valid_progression(branches)` | `bool` | Validate progression |
| `.function(branch)` | `HarmonicFunction` | Harmonic function (T/S/D) |
| `.branches_with_function(func)` | `list[str]` | Branches with function |
| `.to_dot(show_functions=False)` | `str` | Graphviz DOT export |
| `.to_mermaid()` | `str` | Mermaid diagram export |

### HarmonicPath (struct)

Returned by `Tree.paths()`. Represents a harmonic progression step.

| Field | Type | Description |
|-------|------|-------------|
| `.id` | `int` | Path identifier |
| `.branch` | `str` | Target branch name |
| `.chord` | `Chord` | Resolved chord |
| `.interval_labels` | `list[str]` | Chord intervals |
| `.note_names` | `list[str]` | Chord note names |

### ChordComparison (struct)

Returned by `Chord.compare()`. Absolute (context-free) comparison of two chords.

| Field | Type | Description |
|-------|------|-------------|
| `.common_notes` | `list[Note]` | Notes present in both chords |
| `.exclusive_a` | `list[Note]` | Notes only in chord A |
| `.exclusive_b` | `list[Note]` | Notes only in chord B |
| `.root_distance` | `int` | Root distance in semitones (0-6, shortest arc) |
| `.root_direction` | `int` | Signed root direction (-6 to +6) |
| `.same_quality` | `bool` | Same chord type (M, m, dim, etc.) |
| `.same_size` | `bool` | Same number of notes |
| `.common_intervals` | `list[str]` | Interval labels present in both |
| `.enharmonic` | `bool` | Same pitch class set |
| `.subset` | `str` | `""`, `"a_subset_of_b"`, `"b_subset_of_a"`, `"equal"` |
| `.voice_leading` | `int` | Optimal voice pairing in semitones (Tymoczko 2011). -1 if different sizes |
| `.transformation` | `str` | Neo-Riemannian transformation (Cohn 2012): `""`, `"P"`, `"L"`, `"R"`, `"RP"`, `"LP"`, `"PL"`, `"PR"`, `"LR"`, `"RL"` (triads only) |
| `.inversion` | `bool` | Same notes, different root |
| `.interval_vector_a` | `list[int]` | Interval-class vector (Forte 1973): 6 elements counting ic1-6 for chord A |
| `.interval_vector_b` | `list[int]` | Interval-class vector (Forte 1973) for chord B |
| `.same_interval_vector` | `bool` | Same vector = Z-relation candidate (Forte 1973) |
| `.transposition` | `int` | Transposition index T_n (Lewin 1987): 0-11, or -1 if not related |
| `.dissonance_a` | `float` | Psychoacoustic roughness (Plomp & Levelt 1965 / Sethares 1998) for chord A |
| `.dissonance_b` | `float` | Psychoacoustic roughness (Plomp & Levelt 1965 / Sethares 1998) for chord B |

| Method | Returns | Description |
|--------|---------|-------------|
| `.to_dict()` | `dict` | Serialize all fields to a plain Python dict (Notes as strings) |

### FieldComparison (struct)

Returned by `Field.compare()`. Contextual comparison within a harmonic field.

| Field | Type | Description |
|-------|------|-------------|
| `.degree_a`, `.degree_b` | `int?` | Scale degree (None if non-diatonic) |
| `.function_a`, `.function_b` | `HarmonicFunction?` | Harmonic function |
| `.role_a`, `.role_b` | `str?` | Role within function group |
| `.degree_distance` | `int?` | Distance between degrees |
| `.same_function` | `bool?` | Same harmonic function |
| `.relative` | `bool` | Relative chord pair |
| `.progression` | `bool` | Reserved for future use |
| `.root_motion` | `str` | Diatonic root motion (Kostka & Payne): `""`, `"ascending_fifth"`, `"descending_fifth"`, `"ascending_third"`, `"descending_third"`, `"ascending_step"`, `"descending_step"`, `"tritone"`, `"unison"` |
| `.secondary_dominant` | `str` | Secondary dominant (Kostka & Payne): `""`, `"a_is_V7_of_b"`, `"b_is_V7_of_a"` |
| `.applied_diminished` | `str` | Applied diminished vii/x (Gauldin 1997): `""`, `"a_is_viidim_of_b"`, `"b_is_viidim_of_a"` |
| `.diatonic_a`, `.diatonic_b` | `bool` | Belongs to the field |
| `.borrowed_a`, `.borrowed_b` | `BorrowedInfo?` | Modal borrowing origin |
| `.pivot` | `list[PivotInfo]` | Keys where both chords have a degree |
| `.tritone_sub` | `bool` | Tritone substitution (Kostka & Payne): both dom7, roots 6 st apart |
| `.chromatic_mediant` | `str` | Chromatic mediant (Cohn 2012): `""`, `"upper"`, `"lower"` |
| `.foreign_a`, `.foreign_b` | `list[Note]` | Notes outside the scale |

| Method | Returns | Description |
|--------|---------|-------------|
| `.to_dict()` | `dict` | Serialize all fields to a plain Python dict |

### FieldMatch (struct)

Returned by `Field.deduce()`. Ranked candidate field match.

| Field | Type | Description |
|-------|------|-------------|
| `.field` | `Field` | Candidate field |
| `.score` | `float` | Match ratio (0.0–1.0) |
| `.matched` | `int` | Number of matched items |
| `.total` | `int` | Total items in input |
| `.roles` | `list[str]` | Roles for each item (Roman numerals) |

| Method | Returns | Description |
|--------|---------|-------------|
| `.to_dict()` | `dict` | Serialize to dict |

### BorrowedInfo (struct)

| Field | Type | Description |
|-------|------|-------------|
| `.scale_type` | `str` | Origin scale type ("NaturalMinor", etc.) |
| `.degree` | `int` | Degree in that scale |
| `.function` | `HarmonicFunction` | Function in that scale |
| `.role` | `str` | Role in that scale |

| Method | Returns | Description |
|--------|---------|-------------|
| `.to_dict()` | `dict` | Serialize to dict (function as string name) |

### PivotInfo (struct)

| Field | Type | Description |
|-------|------|-------------|
| `.tonic` | `str` | Tonic of the pivot key |
| `.scale_type` | `str` | Scale type |
| `.degree_a` | `int` | Degree of chord A in that key |
| `.degree_b` | `int` | Degree of chord B in that key |

| Method | Returns | Description |
|--------|---------|-------------|
| `.to_dict()` | `dict` | Serialize to dict |

### HarmonicFunction (enum)

| Property | Returns | Description |
|----------|---------|-------------|
| `.name` | `str` | Full name: "Tonic", "Subdominant", "Dominant" |
| `.short` | `str` | Abbreviation: "T", "S", "D" |

---

## Rhythm & Time

Gingo models rhythm with first-class objects that match standard music notation.

### Duration

Durations can be created by name (e.g., `quarter`, `eighth`) or as rational values. Dots and tuplets are built in.

```python
from gingo import Duration

Duration("quarter")
Duration("eighth", dots=1)   # dotted eighth
Duration("eighth", tuplet=3) # triplet eighth
Duration(3, 16)               # 3/16
```

### Tempo (nomos de tempo)

Tempo accepts either BPM or traditional tempo markings (nomos/nomes de tempo) such as Allegro or Adagio, and converts between them.

```python
from gingo import Tempo, Duration

Tempo(120).marking()     # "Allegretto"
Tempo("Adagio").bpm()   # 60
Tempo("Allegro").seconds(Duration("quarter"))
```

### Time Signature

Time signatures provide beats-per-bar, beat unit, classification, and bar duration.

```python
from gingo import TimeSignature, Tempo

ts = TimeSignature(6, 8)
ts.classification()      # "compound"
ts.bar_duration().beats()
Tempo(120).seconds(ts.bar_duration())
```

### Sequence & Events

Build a timeline of note/chord events with a tempo and time signature. Sequences can be transposed and played back.

```python
from gingo import (
    Sequence, Tempo, TimeSignature, NoteEvent, ChordEvent, Rest,
    Note, Chord, Duration,
)

seq = Sequence(Tempo(96), TimeSignature(4, 4))
seq.add(NoteEvent(Note("C"), Duration("quarter"), octave=4))
seq.add(ChordEvent(Chord("G7"), Duration("half"), octave=4))
seq.add(Rest(Duration("quarter")))
seq.total_seconds()
```

CLI helpers for rhythm:

- `gingo duration quarter --tempo 120`
- `gingo tempo Allegro --all`
- `gingo timesig 6 8 --tempo 120`

---

## Audio & Playback

Any musical object can be rendered to audio with `.play()` or `.to_wav()` (monophonic synthesis). Playback uses `simpleaudio` when available; install the optional dependency with `pip install gingo[audio]` for the best cross-platform experience.

```python
from gingo import Note, Chord, Scale

Note("C").play(waveform="sine")
Chord("Am7").play(waveform="square", strum=0.04)
Scale("C", "major").to_wav("c_major.wav", waveform="triangle")
```

CLI audio flags are available on `note`, `chord`, `scale`, and `field`:

- `--play` outputs to speakers
- `--wav FILE` exports a WAV file
- `--waveform sine|square|sawtooth|triangle`
- `--strum` and `--gap` for timing feel

---

## Architecture

```
gingo/
├── cpp/                        # C++17 core library
│   ├── include/gingo/     # Public headers
│   │   ├── note.hpp           # Note class
│   │   ├── interval.hpp       # Interval class
│   │   ├── chord.hpp          # Chord class (42 formulas)
│   │   ├── scale.hpp          # Scale class (10 families, modes, pentatonic)
│   │   ├── field.hpp          # Harmonic field
│   │   ├── tree.hpp           # Harmonic tree (beta)
│   │   ├── duration.hpp       # Duration class (rhythm)
│   │   ├── tempo.hpp          # Tempo class (BPM + markings)
│   │   ├── time_signature.hpp # TimeSignature class
│   │   ├── event.hpp          # NoteEvent, ChordEvent, Rest
│   │   ├── sequence.hpp       # Sequence class (timeline)
│   │   ├── gingo.hpp          # Umbrella include
│   │   └── internal/          # Internal infrastructure
│   │       ├── types.hpp      # TypeElement, TypeVector, TypeTable
│   │       ├── table.hpp      # Lookup table class
│   │       ├── data_ops.hpp   # rotate, spread, spin operations
│   │       ├── notation_utils.hpp  # Formal notation helpers
│   │       ├── lookup_data.hpp     # Singleton with all music data
│   │       ├── lookup_tree.hpp     # Singleton with tree data
│   │       └── mode_data.hpp       # Mode metadata
│   └── src/                   # All implementations
├── bindings/
│   └── pybind_module.cpp      # pybind11 Python bridge
├── python/gingo/
│   ├── __init__.py            # Public API re-exports
│   ├── __init__.pyi           # Type stubs (PEP 561)
│   ├── __main__.py            # CLI entry point
│   ├── audio.py               # Audio playback (requires simpleaudio)
│   └── py.typed               # PEP 561 marker
├── tests/
│   ├── cpp/                   # Catch2 test suite
│   └── python/                # pytest suite
├── CMakeLists.txt             # CMake build system
├── pyproject.toml             # scikit-build-core packaging
├── MANIFEST.in                # Source distribution manifest
└── .github/workflows/
    ├── ci.yml                 # Cross-platform CI
    └── publish.yml            # PyPI publishing via cibuildwheel
```

**Design decisions:**

- **C++ core** — All music theory computation runs in compiled C++17 for performance. This is critical for real-time MIDI, FFT, and machine learning workloads.
- **pybind11 bridge** — Exposes the C++ types to Python with zero-copy where possible and full type stub support for IDE autocompletion.
- **Lazy computation** — Chord notes, scale notes, and formal spellings are computed on first access and cached internally using mutable fields.
- **Meyer's singleton** — All lookup data (enharmonic maps, chord formulas, scale masks) is initialized once on first use, with no manual setup required.
- **Domain types over generic tables** — Instead of the original generic `Table` data structure, the new API uses dedicated `Note`, `Interval`, `Chord`, `Scale`, and `Field` types with clear, discoverable methods.

---

## Building from Source

### Python package

```bash
pip install -v .
```

This triggers scikit-build-core, which runs CMake, compiles the C++ core, links the pybind11 module, and installs the Python package.

### C++ only

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### C++ with tests

```bash
cmake -B build -DGINGO_BUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

### Run Python tests

```bash
pip install -v ".[test]"
pytest tests/python -v
```

---

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run both C++ and Python test suites
5. Submit a pull request

---

## License

MIT
