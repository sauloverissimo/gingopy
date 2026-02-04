# CLAUDE.md — 🪇 Gingo

Python music‑theory library (C++17 core, pybind11 bindings).
Used for study, analysis, and music‑software development.
Domain: Western harmony (12‑TET).

**Português (pt‑BR)**: https://sauloverissimo.github.io/gingo/

Gingo ships two interfaces:
- **Python library** (primary): `from gingo import Note, Chord, Scale, ...`
- **CLI** (exploration/testing): `gingo note C#`, `gingo chord Am7`, etc.

## Project layout

```
cpp/include/gingo/           # Public headers (.hpp)
cpp/include/gingo/internal/  # Internal infra (lookup, types, ops)
cpp/src/                     # Implementations (.cpp)
bindings/pybind_module.cpp   # C++ -> Python bridge (pybind11)
python/gingo/                # Python package (__init__.py, __init__.pyi, __main__.py, py.typed)
tests/cpp/                   # C++ tests (Catch2 v3.4)
tests/python/                # Python tests (pytest)
docs/                        # MkDocs site pages
```

## Build and test

### C++ (Catch2)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DGINGO_BUILD_TESTS=ON
cmake --build build --config Release
cd build && ctest --output-on-failure -C Release
```

### Python (pytest)
```bash
pip install -v ".[test]"
pytest tests/python -v
```

### Build library only (no tests)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Architecture

### Domain classes (namespace `gingo`)

| Class | Files | Responsibility |
|-------|-------|----------------|
| `Note` | note.hpp/cpp | Pitch class, enharmonic resolution, frequency, transposition |
| `Interval` | interval.hpp/cpp | Interval distance (0–23 semitones, 2 octaves) |
| `Chord` | chord.hpp/cpp | Root + intervals, 42 formulas, reverse identification, comparison (Neo‑Riemannian P/L/R + 2‑step compositions, voice leading, interval vector, T_n, dissonance) |
| `Scale` | scale.hpp/cpp | Tonic + Parent + Mode + Filter, 10 families, 36 modes, binary mask, pentatonic, colors |
| `Field` | field.hpp/cpp | Harmonic field: triads, sevenths, functions (T/S/D), applied chords, contextual comparison (modal borrowing, pivot, tritone sub, chromatic mediant, root motion, applied diminished), identify/deduce |

### Internal infrastructure (namespace `gingo::internal`)

- `LookupData` — Meyers singleton with all musical data (lazy, thread‑safe).
- `Table` — Generic lookup table for musical data.
- `TypeElement` = `std::variant<string, int, double, vector<string>, vector<int>>`
- `data_ops` — `rotate`, `spread`, `spin` for scale/chord manipulation.
- `notation_utils` — Formal notation helpers (Gb vs F#, etc).

### Design patterns

- **Lazy computation with mutable caching**: notes, intervals, and formal spelling are computed on demand and cached in `mutable` fields.
- **Singleton (Meyer's)**: `LookupData::instance()` loads data on first call.
- **Static library**: `gingo_core` built as a static lib, linked to the pybind11 module.

### Dependency chain

```
Note (base, no deps)
  -> Interval (uses semitones)
     -> Chord (uses Note + Interval + LookupData)
        -> Scale (uses Note + LookupData + data_ops)
           -> Field (uses Scale + Chord)
```

## Coding conventions

### General naming (C++ and Python)
- **Prefer single‑word public names** when possible (e.g., `colors()` instead of `color_notes()`).
- Use `snake_case` for multi‑word names only when a single word would be ambiguous.

### C++
- **Standard**: C++17 (required)
- **Namespace**: `gingo` (public), `gingo::internal` (infra)
- **Naming**: `snake_case` for methods/vars, `PascalCase` for classes/enums
- **Private members**: trailing `_` (e.g., `name_`, `cached_notes_`)
- **Header guards**: `#pragma once`
- **Includes**: public headers via `<gingo/...>`, internal via relative paths
- **License**: `SPDX-License-Identifier: MIT` at the top of each file
- **Section separators**: `// ---------------------------------------------------------------------------`
- **Docs**: `///` Doxygen style for public API
- **.cpp sections**: comments like `// Constructor`, `// Accessors`, etc.

### Python
- **Type stubs**: `__init__.pyi` (PEP 561)
- **Re‑exports**: `__init__.py` imports from `_gingo` (C++ module)
- **Tests**: pytest, `test_*.py` mirrors C++ tests

### Bindings (pybind11)
- Internal module: `_gingo` (underscore = private)
- Expose C++ methods with the same names
- `__repr__` maps to `to_string()`, `__str__` to the name/label
- Enums via `py::enum_<>` with named values
- Overloads via `py::overload_cast<>`

## Comparison structs

Comparison uses 4 structs (defined in `chord.hpp` and `field.hpp`):

| Struct | File | Responsibility |
|--------|------|----------------|
| `ChordComparison` | chord.hpp | Result of `Chord::compare()` (18 absolute dimensions). `to_dict()` for serialization. |
| `BorrowedInfo` | field.hpp | Modal borrowing info: origin scale, degree, function, role. `to_dict()`. |
| `PivotInfo` | field.hpp | Pivot key info: tonic, scale type, degrees in both contexts. `to_dict()`. |
| `FieldComparison` | field.hpp | Result of `Field::compare()` (21 contextual dimensions). `to_dict()` for serialization. |

Theoretical basis:
- **Neo‑Riemannian** (Cohn 2012): P/L/R for major↔minor triads; 2‑step compositions (RP, LP, PL, PR, LR, RL).
- **Voice leading** (Tymoczko 2011): minimal semitone sum (brute force for ≤7 notes).
- **Interval vector** (Forte 1973): 6‑element vector counting interval classes 1–6; detects Z‑relations.
- **Transposition T_n** (Lewin 1987): exact pitch‑class transposition (0–11), or ‑1.
- **Dissonance** (Plomp & Levelt 1965 / Sethares 1998): psychoacoustic roughness.
- **Root motion** (Kostka & Payne): diatonic root interval class.
- **Applied diminished** (Gauldin 1997): vii°/x detection (dim or m7(b5), root 1 semitone below target).
- **Modal borrowing**: searches 4 parallel ScaleTypes (same tonic).
- **Pivot**: sweep of 12 tonics × 4 types = 48 candidate fields.

Serialization:
- All 4 structs expose `to_dict()` returning `dict[str, Any]` (Notes -> strings, HarmonicFunction -> name string, optionals -> None, nested structs -> dicts).

## Embedded musical data

- 89 enharmonic spellings with canonical resolution (sharp‑based)
- 42 chord formulas (triads, sevenths, extended, altered)
- 24 interval labels (2 octaves)
- 12 chromatic pitch classes
- 24‑bit scale masks

## Current enums

```cpp
enum class ScaleType {
    Major=0, NaturalMinor=1, HarmonicMinor=2, MelodicMinor=3,
    Diminished=4, HarmonicMajor=5, WholeTone=6, Augmented=7,
    Blues=8, Chromatic=9
};
enum class Modality         { Diatonic=0, Pentatonic=1 };
enum class HarmonicFunction { Tonic=0, Subdominant=1, Dominant=2 };
```

`HarmonicFunction` exposes `.name` ("Tonic", "Subdominant", "Dominant")
and `.short` ("T", "S", "D"). Lives in `field.hpp` with free functions
`harmonic_function_name()` and `harmonic_function_short()`.

## Current state and next steps

### Version: 1.0.0

Scale uses a hierarchical Parent -> Mode -> Filter model:
- 10 parent families (ScaleType enum), 36 named modes
- Each `Scale` stores `parent_`, `mode_number_`, `pentatonic_`
- Accessors: `parent()`, `mode_number()`, `mode_name()`, `quality()`, `brightness()`
- Navigation: `mode(int)`, `mode(string)`, `pentatonic()`, `colors(ref)`
- Mode‑name construction: `Scale("D", "dorian")`, `Scale("C", "altered")`
- Pentatonic via suffix: `Scale("C", "major pentatonic")`
- `type()` and `modality()` kept for backward compatibility

Identification and deduction:
- `Chord(["C","E","G"])` — list constructor (builds from note names or Note objects)
- `Scale.identify(notes)` — detects scale from a complete note set
- `Field.identify(items)` — detects field from complete notes or chords
- `Field.deduce(items, limit=10)` — ranked inference from partial input, returns `list[FieldMatch]`
- CLI: `gingo scale ... --identify`, `gingo field ... --identify`, `gingo field ... --deduce [--limit N]`

Planned future scales (see `.old/scales_analysis.md` section 14.9):
- Bebop (6 types, 8 notes with chromatic passing tone)
- Named symmetric families (aliases of existing modes)

## Git and SSH

- **Remote URL**: `git@github-stacks:sauloverissimo/gingo.git` (SSH alias)
- **SSH host alias**: `github-stacks` (definido em `~/.ssh/config`, usa chave em `/home/saulo/stacks/.setup/github/id_ed25519_github`)
- Para push/pull, sempre usar o remote com o alias `github-stacks` (não `github.com` diretamente)
- **Commits**: manter autoria única (sem co-autoria). Autor/commit: `sauloverissimo` <sauloverissimo@gmail.com>.

## CI/CD

- **GitHub Actions**: `.github/workflows/ci.yml` — Ubuntu, macOS, Windows
- **Python**: tests 3.10, 3.11, 3.12, 3.13
- **Publishing**: `.github/workflows/publish.yml` via cibuildwheel to PyPI

## Project philosophy

Gingo is a **Python library** for musical study, analysis, and development.
The CLI is a convenience interface for exploration and quick testing, but
the primary goal is a programmable Python API.

The library models music theory in a **systemic, non‑biased** way. It does
not align with any single school or tradition (classical, jazz, popular).
The goal is to abstract structural principles of Western harmony so that
anyone — regardless of background — can understand and use them.

Design principles:

- **Library first, CLI second**: the core value is the Python API.
  The CLI exists for exploration and testing, not as a final product.
