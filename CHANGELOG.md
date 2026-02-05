# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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
