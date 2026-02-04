// Gingo — Music Theory Library
// Scale: ordered sequence of notes following a tonal pattern.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "note.hpp"
#include "interval.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace gingo {

/// Scale type identifiers for parent scale patterns.
///
/// Each value represents a genuinely distinct interval formula (parent scale)
/// from which modes can be derived.  NaturalMinor is structurally mode 6 of
/// Major but kept for convenience.
enum class ScaleType {
    Major          = 0,
    NaturalMinor   = 1,
    HarmonicMinor  = 2,
    MelodicMinor   = 3,
    Diminished     = 4,
    HarmonicMajor  = 5,
    WholeTone      = 6,
    Augmented      = 7,
    Blues           = 8,
    Chromatic       = 9
};

/// Modality: how the scale is filtered (all 7 degrees vs pentatonic 5).
enum class Modality {
    Diatonic   = 0,   // 7-note scale (default)
    Pentatonic = 1    // 5-note scale
};

/// Represents a musical scale built from a tonic and a scale pattern.
///
/// A scale is identified by its tonic, parent scale type, mode number,
/// and optional pentatonic filter.  Notes and masks are lazily derived.
///
/// Examples:
///   Scale s("C", ScaleType::Major);
///   s.notes();                        // {C, D, E, F, G, A, B}
///   s.degree(5);                      // Note("G")
///   s.mode(2);                        // D Dorian
///   s.mode("dorian");                 // D Dorian (equivalent)
///   Scale("D", "dorian");             // D Dorian (direct)
///   Scale("C", "major").pentatonic(); // C Major Pentatonic
class Scale {
public:
    /// Construct from tonic and enum type (backward-compatible).
    Scale(const std::string& tonic, ScaleType type,
          Modality modality = Modality::Diatonic);

    /// Construct from tonic and string name (case-insensitive).
    /// Accepts parent names ("major", "harmonic minor", etc.),
    /// mode names ("dorian", "altered", "phrygian dominant", etc.),
    /// and pentatonic variants ("major pentatonic", "dorian pentatonic").
    Scale(const std::string& tonic, const std::string& type_name,
          const std::string& modality_name = "diatonic");

    /// The tonic (first degree) of the scale.
    Note tonic() const { return tonic_; }

    /// @deprecated Use parent() instead.
    ScaleType type() const { return parent_; }

    /// @deprecated Use is_pentatonic() instead.
    Modality modality() const;

    /// The parent scale type from which this mode is derived.
    ScaleType parent() const { return parent_; }

    /// Mode number within the parent (1-based).
    int mode_number() const { return mode_number_; }

    /// Whether the pentatonic filter is applied.
    bool is_pentatonic() const { return pentatonic_; }

    /// Mode name (e.g. "Dorian", "Phrygian Dominant", "Altered").
    /// For mode 1 of a parent, returns the parent name (e.g. "Ionian").
    std::string mode_name() const;

    /// Tonic quality: "major" or "minor".
    std::string quality() const;

    /// Brightness position in the mode continuum.
    /// For Major family: Lydian=7, Ionian=5, Mixolydian=3, etc.
    /// Returns 0 for non-Major families (not yet defined).
    int brightness() const;

    /// Color notes: notes that distinguish this mode from a reference.
    /// The reference can be a Scale object or a mode name string.
    /// E.g. Dorian vs Ionian → the color note is the natural 6th.
    std::vector<Note> colors(const Scale& reference) const;
    std::vector<Note> colors(const std::string& reference_mode) const;

    /// All notes in the scale.  Natural (sharp-based) representation.
    std::vector<Note> notes() const;

    /// All notes in formal notation (proper diatonic spelling).
    /// E.g. Gb major → {Gb, Ab, Bb, Cb, Db, Eb, F} instead of sharps.
    std::vector<Note> formal_notes() const;

    /// Get the note at a specific scale degree (1-indexed).
    /// degree(1) = tonic, degree(5) = dominant, etc.
    Note degree(int n) const;

    /// Chained degree navigation (1-indexed).
    /// degree({5, 5}) = "grau V do grau V" = note at degree II.
    /// Formula: pos = (pos-1 + arg-1) % size + 1, applied iteratively.
    Note degree(const std::vector<int>& degrees) const;

    /// Walk from a starting degree by successive steps (1-indexed).
    /// walk(5, {5}) = "from degree V, walk a fifth" — same arithmetic as
    /// chained degree(). Supports negative steps for backward navigation.
    Note walk(int start, const std::vector<int>& steps) const;

    /// Number of notes in the scale.
    std::size_t size() const;

    /// Whether a note belongs to this scale.
    bool contains(const Note& note) const;

    /// Find the scale degree of a note (1-indexed), or nullopt if not found.
    /// Scale("C", "major").degree_of(Note("G")) == 5.
    std::optional<int> degree_of(const Note& note) const;

    /// Build a new scale starting from a different mode/degree (int).
    /// mode(1) = same scale, mode(2) = Dorian of a major scale, etc.
    Scale mode(int degree_number) const;

    /// Build a new scale by mode name (e.g. "dorian", "altered").
    /// The mode must belong to the same parent scale.
    Scale mode(const std::string& mode_name) const;

    /// Return a pentatonic version of this scale.
    Scale pentatonic() const;

    /// Key signature: positive = sharps, negative = flats, 0 = none.
    /// Meaningful for Major and NaturalMinor families (all modes share
    /// the same signature).  Returns 0 for other families.
    int signature() const;

    /// Relative major/minor: same key signature, opposite quality.
    /// Only for Major (mode 1) and NaturalMinor (mode 1).
    /// C major → A natural minor, A natural minor → C major.
    Scale relative() const;

    /// Parallel major/minor: same tonic, opposite quality.
    /// C major → C natural minor, C minor → C major.
    Scale parallel() const;

    /// Adjacent keys on the circle of fifths: {subdominant, dominant}.
    /// Preserves scale type, mode number, and pentatonic flag.
    std::pair<Scale, Scale> neighbors() const;

    /// Decompose a scale into a 24-bit binary mask.
    /// Useful for comparing scale structures.
    std::vector<int> mask() const;

    std::string to_string() const;

    /// Convert a string like "major" to the corresponding ScaleType enum.
    static ScaleType parse_type(const std::string& name);

    /// Convert a string like "diatonic" to the corresponding Modality enum.
    static Modality parse_modality(const std::string& name);

private:
    Note      tonic_;
    ScaleType parent_;
    int       mode_number_  = 1;
    bool      pentatonic_   = false;

    // Lazily computed
    mutable std::vector<Note> cached_notes_;
    mutable std::vector<Note> cached_formal_;
    mutable bool              computed_ = false;

    void compute() const;

    // Internal constructor for mode/pentatonic chaining.
    Scale(Note tonic, ScaleType parent, int mode_number, bool pentatonic);
};

} // namespace gingo
