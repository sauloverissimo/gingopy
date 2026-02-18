# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2026-02-18

### Added

- **Fretboard class**: Guitar/string instrument fingering engine
  - `Fretboard.violao()`, `.cavaquinho()`, `.bandolim()` — factory methods for standard instruments
  - `Fretboard(tuning, num_frets)` — custom instruments with any tuning
  - `fretboard.fingering(chord, position)` — generates optimal CAGED-based fingering
  - `fretboard.scale_positions(scale, fret_lo, fret_hi)` — all scale positions on the neck
  - `fretboard.positions(note)` — all occurrences of a note on the fretboard
  - `fretboard.position(string, fret)` — single position lookup
  - Multi-criteria scoring algorithm: span, position, finger count, sounding strings, barre detection
  - 22/24 standard CAGED chord shapes match reference (jguitar.com)

- **FretboardSVG class**: SVG renderer for fretboard diagrams
  - `FretboardSVG.chord(fb, chord)` — chord diagram (vertical chord box)
  - `FretboardSVG.fingering(fb, fingering)` — render specific fingering
  - `FretboardSVG.scale(fb, scale, fret_lo, fret_hi)` — scale on fretboard
  - `FretboardSVG.note(fb, note)` — all positions of a note
  - `FretboardSVG.positions(fb, positions, title)` — custom highlighted positions
  - `FretboardSVG.field(fb, field, layout)` — all chords in a harmonic field
  - `FretboardSVG.progression(fb, field, branches, layout)` — chord progression
  - `FretboardSVG.full(fb)` — full open fretboard visualization
  - `FretboardSVG.write(svg, path)` — save SVG to file

- **Orientation enum**: `Horizontal` (fretboard view), `Vertical` (chord box view)
  - All FretboardSVG methods accept `orientation` parameter
  - Smart defaults: chord/fingering default to Vertical, scale/note default to Horizontal

- **Handedness enum**: `RightHanded`, `LeftHanded`
  - All FretboardSVG methods accept `handedness` parameter
  - Left-handed mirrors the perpendicular axis (frets for horizontal, strings for vertical)

- **Fretboard structs**:
  - `Tuning` — string tuning (name, open MIDI values)
  - `FretPosition` — single position on the fretboard (string, fret, note, MIDI)
  - `Fingering` — complete chord fingering (strings, barre, base_fret, chord_name)
  - `StringState` — per-string state (string number, fret, action)
  - `StringAction` enum — `Open`, `Fretted`, `Muted`

- **CLI commands**:
  - `gingo fretboard chord CM` — show chord fingering
  - `gingo fretboard chord CM --svg chord.svg` — export SVG
  - `gingo fretboard scale "C major"` — show scale positions
  - `gingo fretboard field "C major"` — show full harmonic field
  - `gingo fretboard --left` — left-handed diagrams
  - `gingo fretboard --horizontal` / `--vertical` — orientation control

### Changed

- **Version**: Bumped to 1.1.0 (new instrument module = minor version)
- **Architecture**: Added `fretboard.hpp`, `fretboard_svg.hpp` to public headers
- **Dependency chain**: Fretboard uses Note + Chord + Scale (same level as Piano)

## [1.0.2] - 2026-02-05

### Added

- **Progression class**: New coordinator for cross-tradition harmonic analysis
  - `Progression("C", "major")` — constructor with tonic and scale type
  - `Progression.traditions()` — lists available traditions (harmonic_tree, jazz)
  - `prog.tree("harmonic_tree")` — returns Tree for a specific tradition
  - `prog.identify(branches)` — identifies tradition and schema from a progression
  - `prog.deduce(branches, limit)` — ranked matches across traditions
  - `prog.predict(branches)` — suggests next chords, cross-tradition

- **Tradition struct**: Metadata for harmonic traditions (name, description)

- **Schema struct**: Named progression patterns (name, description, branches)
  - Harmonic tree schemas: descending, ascending, direct, extended_descending, etc.
  - Jazz schemas: ii-V-I, turnaround, backdoor, tritone_sub, etc.

- **ProgressionMatch struct**: Result of identify/deduce operations
  - Fields: tradition, schema, score, matched, total, branches

- **ProgressionRoute struct**: Result of predict operation
  - Fields: next, tradition, schema, path, confidence

- **Tree enhancements**:
  - 3-argument constructor: `Tree("C", ScaleType.Major, "harmonic_tree")`
  - `tree.tradition()` — returns Tradition metadata
  - `tree.schemas()` — returns named patterns for the tradition
  - `tree.is_valid(branches)` — shorter alias for is_valid_progression

- **Jazz tradition**: New harmonic tradition with classic jazz patterns
  - Branches: I, IIm, IIIm, IV, V7, VIm, VIIdim, IVm, bVII, SUBV7, etc.
  - Schemas: ii-V-I, turnaround, backdoor, tritone_sub, minor_ii-V-i

- **CLI commands**:
  - `gingo tree "C major" harmonic_tree` — explore harmonic trees
  - `gingo progression "C major" --identify IIm V7 I` — identify progressions
  - `gingo progression "C major" --deduce IIm V7` — deduce traditions
  - `gingo progression "C major" --predict I IIm` — predict next chords
  - `gingo progression --traditions` — list available traditions

### Changed

- **Tree constructor**: Now requires 3 arguments (tonic, type, tradition)
  - Old: `Tree("C", "major")` — still works, defaults to "harmonic_tree"
  - New: `Tree("C", "major", "harmonic_tree")` — explicit tradition

- **LookupProgression**: Replaces LookupTree internally
  - Tradition-keyed data structure for branches, paths, and schemas
  - Supports multiple traditions (harmonic_tree, jazz)

### Deprecated

- `tree.is_valid_progression()` — use `tree.is_valid()` instead (still works)

## [1.0.1] - 2026-02-04

### Added

- Initial release of Tree class (beta) for harmonic progressions
- Audio playback and WAV export for all musical objects
- Duration, Tempo, TimeSignature, and Sequence classes for rhythm
- NoteEvent, ChordEvent, and Rest for sequencing

## [1.0.0] - 2026-02-03

### Added

- Core music theory classes: Note, Interval, Chord, Scale, Field
- 42 chord formulas with identification and comparison
- 10 scale families with 36 named modes
- Harmonic field generation with functions (T/S/D)
- ChordComparison with 18 dimensions (Neo-Riemannian, voice leading, etc.)
- FieldComparison with 21 contextual dimensions
- Field.identify() and Field.deduce() for harmonic analysis
- CLI for exploration: note, interval, chord, scale, field, compare
- Python type stubs (PEP 561)
- Cross-platform wheels (Linux, macOS, Windows)
