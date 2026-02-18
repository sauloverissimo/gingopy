# Fretboard Fingering Algorithm — v1.0.3

## Overview

The `Fretboard` class generates realistic, playable guitar chord fingerings for any chord and tuning. The algorithm intelligently models CAGED shapes, barré chords, open forms, and physical comfort constraints.

**Result**: 22/24 standard chords match jguitar.com/fachords.com references; 672 total chords tested with 0 errors.

---

## Score Function Breakdown

### Penalties (score += value, higher = less preferred)

| Criterion | Formula | Notes |
|-----------|---------|-------|
| **Muted strings** | +3 per string | Inner mutes: +50 (impossible) |
| **Fret span** | +4×span | span > 3: +20×(span-3) additional |
| **Position** | +2×pos | Linear (not quadratic); allows barre at frets 1–7 |
| **Fingers > 3** | +15×(fingers-3) | 3 comfort, 4+ penalty |
| **Fret gap** | +15×(gap-2) | Only if gap ≥ 3 (hard to reach) |
| **Leapfrog** | +5×gap | Fretted-open-fretted at different frets |
| **Open+high** | +30 | If open strings AND max_fret ≥ 4 |
| **Non-root bass** | +20 | Weakens sound (inversions) |

### Rewards (score -= value, higher = more preferred)

| Criterion | Formula | Notes |
|-----------|---------|-------|
| **Open strings** | -2 per string | No finger needed |
| **Sounding strings** | -5 per string | Fuller chord (5–6 strings preferred) |

---

## Examples

### C Major — Standard open form
- **Fingering**: `X-3-2-0-1-0`
- **Score components**:
  - Muted: 1 × 3 = 3
  - Span: 3 × 4 = 12
  - Position: 0 × 2 = 0
  - Fingers: 4 → +15
  - Sounding: 5 × -5 = -25
  - **Total ≈ 5** ✓ Best form

### FM — E-shape barré at fret 1
- **Fingering**: `1-1-2-3-3-1` (6 cordas)
- **Score components**:
  - Muted: 0
  - Span: 2 × 4 = 8
  - Position: 1 × 2 = 2
  - Barré: 1 (pestana) + 2 (frets 2,3) = 3 fingers → 0
  - Sounding: 6 × -5 = -30
  - **Total ≈ -20** ✓ Standard pestana

### D#dim — Compact symmetric form
- **Fingering**: `2-0-1-2-X-X`
- **Score components**:
  - Muted: 2 × 3 = 6
  - Span: 2 × 4 = 8
  - Fingers: 3 → 0
  - Open+high: max_fret=2 < 4 → 0
  - Sounding: 4 × -5 = -20
  - **Total ≈ -6** ✓ Natural diminished form

---

## Search Strategy

### `fingering(chord)` — Simplified interface
```python
from gingo import Fretboard, Chord
g = Fretboard.violao()
chord = Chord("Am7")
best = g.fingering(chord)  # Returns Fingering object
```

**Under the hood**:
1. Scan entire fretboard: `frets 0 → 16`
2. For each position, call `generate_fingerings(chord, pos)`
3. Score all candidates using `score_fingering()`
4. Deduplicate by voicing signature (same notes = same voicing)
5. Return globally best-scored form

### `fingerings(chord, max_results)` — Multiple options
```python
options = g.fingerings(chord, 5)  # Top 5 fingerings
for i, fg in enumerate(options):
    print(f"{i+1}. {fg} (score: {score_fingering(fg)})")
```

---

## Validation Results

### Standard CAGED Chords (24 total)
- **Majors** (C, C#, D, Eb, E, F, F#, G, Ab, A, Bb, B)
- **Minors** (Cm, C#m, Dm, D#m, Em, Fm, F#m, Gm, G#m, Am, Bbm, Bm)

**Accuracy vs jguitar.com**:
- 22/24 **exact matches** (92%)
- 2 **acceptable alternatives** (C#M A-shape vs C-shape, EbM A-shape vs D-shape — both valid)

### Full Suite (672 chords)
- **12 tonics × 4 scales × (triads + 7ths)**
- **0 errors**: no impossible fingerings, no playability violations
- **22 special forms**: diminished/augmented (3–4 strings, symmetric)

### Test Coverage
- **C++ tests**: 426 (Catch2)
- **Python tests**: 676 (pytest)
- **Total**: 1,102 — **100% pass**

---

## Key Improvements in v1.0.3

### vs v1.0.2 (old algorithm)

| Aspect | Old | New | Impact |
|--------|-----|-----|--------|
| **Span penalty** | 8×span | 4×span | CAGED forms (span=2-3) now competitive |
| **Position** | 0.5×pos² + 2×pos | 2×pos | Linear instead of quadratic; barre chords frets 1–7 viable |
| **Sounding bonus** | -3×cordas | -5×cordas | Fuller chords (5–6 strings) strongly preferred |
| **Non-root bass** | +12 | +20 | More effective filtering of inversions |
| **Open+high** | – | +30 | New: prevents impractical mixing |
| **Fingering search** | partial (position=0) | **full fretboard** | Now finds pestanas in correct positions |

**Result**: Went from 14/24 standard chords correct → 22/24 (92% → +8 chords fixed)

---

## Barré Chord Model

**Definition**: One finger laid flat across ≥2 consecutive fretted strings at the lowest fret (pestana).

**Examples**:
- **FM** `1-1-2-3-3-1` = Barré at fret 1 (covers all 6 strings) + 2 higher frets = **3 fingers**
- **Gm** `3-5-5-3-3-3` = Barré at fret 3 (E-shape, 6 strings) + 1 higher fret = **2 fingers**
- **Bm** `2-3-4-4-2-X` = Barré at fret 2 (partial) + 1 higher fret = **2 fingers**

**Algorithm** (fretboard.cpp:476–499):
1. Find longest consecutive run of fretted strings
2. Count strings at `min_fret` within the run
3. If run ≥ 2 AND ≥ 2 strings at min_fret → barré detected
4. Fingers = 1 (barré) + distinct_higher_frets + outside_strings

---

## Physical Constraints Modeled

| Constraint | How modeled | Example |
|-----------|------------|---------|
| **4-finger limit** | Reject in `generate_fingerings()` | FM needs barré → 3 fingers ✓ |
| **Span ≤ 4 frets** | Penalty for span > 3 | Stretch > 3 frets costs 20+ points |
| **Inner mutes impossible** | Reject in `generate_fingerings()` | X between sounding strings → invalid |
| **Open+high impractical** | Penalty +30 | Open B + frets 4–5 → avoid |
| **Leapfrog uncomfortable** | Penalty +5×gap | Fretted-open-fretted pattern penalized |
| **Root bass preferred** | Penalty +20 | Inversions deprioritized |

---

## Configuration (if extending)

To adjust fingering preference, modify `score_fingering()` in `fretboard.cpp`:

```cpp
// Current weights
score += span * 4.0;                    // Lower = prefer wider spans
score += actual_pos * 2.0;              // Lower = prefer higher positions
score -= sounding * 5.0;                // Higher = prefer fuller chords
score += 30.0;  // open+high penalty    // Adjust open+high strictness
```

**Tuning philosophy**:
- Increase span penalty → favor compact forms
- Decrease position penalty → allow higher positions
- Increase sounding bonus → force 6-string chords
- Adjust open+high penalty → allow/disallow mixed open forms

---

## Visual Examples

All 48 field harmonic diagrams are in `output/`:
- `C_major_field.svg` — C Major (tríades + 7ths)
- `Cs_harmonic_minor_field.svg` — C# Harmonic Minor
- etc.

Each chord shows the optimal fingering with:
- **Dots**: fretted positions (colored by pitch)
- **Open circles**: open strings
- **X**: muted strings
- **Blue bar**: barré (if present)
- **Note labels**: pitch names on each dot

---

## References

- **CAGED system**: Standard 5 guitar shapes (C, A, G, E, D) transposed chromatically
- **Barré chords**: Simplified model assuming flat finger covers all consecutive strings
- **Validation**: jguitar.com, fachords.com, ChordBank, Riffhard
- **Comfort heuristics**: Based on standard guitar pedagogy (finger stretch, position preference)

---

## Future Improvements

- [ ] Thumb-over-neck forms (for E/A shapes on lower frets)
- [ ] Voicing varieties (shell, rootless, etc.)
- [ ] Left-handed optimizations
- [ ] Difficulty ratings (beginner → advanced)
- [ ] Voice leading (smooth transitions between chords)
- [ ] Custom tunings (dropped D, open G, etc.)
