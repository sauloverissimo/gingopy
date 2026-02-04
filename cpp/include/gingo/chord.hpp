// Gingo — Music Theory Library
// Chord: a combination of notes with harmonic identity.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "note.hpp"
#include "interval.hpp"

#include <string>
#include <utility>
#include <vector>

namespace gingo {

/// Result of comparing two chords in absolute (context-free) terms.
///
/// Covers note overlap, root geometry, quality match, voice leading,
/// neo-Riemannian transformations, and set-theory relationships.
struct ChordComparison {
    /// Notes present in both chords (by pitch class / semitone).
    std::vector<Note> common_notes;

    /// Notes only in the first chord (a).
    std::vector<Note> exclusive_a;

    /// Notes only in the second chord (b).
    std::vector<Note> exclusive_b;

    /// Unsigned root distance in semitones (0-6, shortest arc).
    int root_distance;

    /// Signed root direction on the chromatic circle (-6 to +6).
    /// Positive = ascending, negative = descending.
    int root_direction;

    /// Whether both chords share the same type/quality suffix.
    bool same_quality;

    /// Whether both chords have the same number of notes.
    bool same_size;

    /// Interval labels common to both chord formulas.
    std::vector<std::string> common_intervals;

    /// Whether the chords are enharmonically equivalent
    /// (same pitch class set, possibly different spellings).
    bool enharmonic;

    /// Subset relationship:
    ///   "" = no subset, "a_subset_of_b", "b_subset_of_a", "equal".
    std::string subset;

    /// Minimum total semitone movement for voice leading.
    /// -1 when chords have different numbers of notes.
    int voice_leading;

    /// Neo-Riemannian transformation label:
    ///   "" = none, "P" = parallel, "L" = leading-tone, "R" = relative.
    std::string transformation;

    /// Whether the chords are inversions of each other
    /// (same pitch class set, different root).
    bool inversion;

    /// Interval-class vector (Forte) for chord a: 6 elements counting
    /// interval classes 1-6 across all note pairs.
    std::vector<int> interval_vector_a;

    /// Interval-class vector (Forte) for chord b.
    std::vector<int> interval_vector_b;

    /// Whether both chords share the same interval vector (Z-relation
    /// candidate when pitch class sets differ).
    bool same_interval_vector;

    /// Transposition index T_n (Lewin): if the pitch-class set of b equals
    /// the set of a shifted by n semitones, returns n (0-11).
    /// -1 means the chords are not related by transposition.
    int transposition;

    /// Psychoacoustic roughness score for chord a (Sethares model, sum of
    /// Plomp-Levelt roughness over all fundamental pairs).  Uses A4 = 440 Hz
    /// in octave 4.
    double dissonance_a;

    /// Psychoacoustic roughness score for chord b.
    double dissonance_b;
};

/// Represents a musical chord — a root note plus a set of intervals.
///
/// Chords are constructed from a name string (e.g. "Cm7", "Db7M", "A#m")
/// and automatically resolve their notes and intervals from the internal
/// chord formula database (42+ types).
///
/// Examples:
///   Chord c("Cm7");
///   c.root();       // Note("C")
///   c.type();       // "m7"
///   c.notes();      // {Note("C"), Note("D#"), Note("G"), Note("A#")}
///   c.intervals();  // {Interval("P1"), Interval("3m"), Interval("5J"), Interval("7m")}
class Chord {
public:
    /// Construct from a chord name: "CM", "Dm7", "Bb7M(#5)", etc.
    explicit Chord(const std::string& name);

    /// The full chord name as given.
    const std::string& name() const { return name_; }

    /// Root note of the chord.
    Note root() const { return root_; }

    /// Chord type/quality suffix: "m7", "7M", "dim", "aug", etc.
    const std::string& type() const { return type_; }

    /// Chord tones as natural (sharp-based) notes.
    std::vector<Note> notes() const;

    /// Chord tones in formal (proper diatonic) notation.
    /// For example, Bbm7 → {Bb, Db, F, Ab} instead of {A#, C#, F, G#}.
    std::vector<Note> formal_notes() const;

    /// The interval structure of this chord.
    std::vector<Interval> intervals() const;

    /// The interval labels as strings: {"P1", "3m", "5J", "7m"}.
    std::vector<std::string> interval_labels() const;

    /// How many notes in the chord.
    std::size_t size() const;

    /// Whether a given note (by semitone) is part of this chord.
    bool contains(const Note& note) const;

    /// Identify a chord from a set of notes (reverse lookup).
    /// The first note is treated as the root.
    static Chord identify(const std::vector<Note>& notes);
    static Chord identify(const std::vector<std::string>& note_names);

    /// Transpose the chord by a number of semitones.
    /// Returns a new chord with the same type/quality, rooted on the transposed root.
    Chord transpose(int semitones) const;

    /// Compare this chord with another, returning a detailed analysis.
    ChordComparison compare(const Chord& other) const;

    bool operator==(const Chord& other) const;
    bool operator!=(const Chord& other) const { return !(*this == other); }
    std::string to_string() const;

private:
    std::string name_;
    Note        root_;
    std::string type_;

    // Cached computed data
    mutable std::vector<Note>     cached_notes_;
    mutable std::vector<Note>     cached_formal_;
    mutable std::vector<Interval> cached_intervals_;
    mutable bool                  computed_ = false;

    void compute() const;
};

} // namespace gingo
