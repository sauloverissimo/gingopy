// Gingo — Music Theory Library
// Interval: the distance between two notes.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>

namespace gingo {

class Note;  // forward declaration

/// Represents a musical interval — the distance between two pitches.
///
/// Intervals are identified by their short label (e.g. "P1", "3m", "5J")
/// or by their semitone distance (0–23, covering two octaves).
///
/// Examples:
///   Interval iv("3m");
///   iv.label();        // "3m"
///   iv.anglo_saxon();  // "mi3"
///   iv.semitones();    // 3
///   iv.degree();       // 3
class Interval {
public:
    /// Construct from a short label: "P1", "2m", "2M", "3m", "3M", "4J",
    /// "d5", "5J", "#5", "M6", "7m", "7M", "8J", "b9", "9", etc.
    explicit Interval(const std::string& label);

    /// Construct from a semitone count (0–23).
    explicit Interval(int semitones);

    /// Construct from two notes: ascending chromatic distance (mod 12).
    /// Interval(Note("C"), Note("G")) == Interval("5J") (7 semitones).
    Interval(const Note& from, const Note& to);

    /// Short label: "P1", "3m", "5J", "7M", etc.
    const std::string& label() const { return label_; }

    /// Anglo-Saxon formal name: "P1", "mi3", "P5", "ma7", etc.
    const std::string& anglo_saxon() const { return anglo_saxon_; }

    /// Number of semitones (0 = unison, 7 = perfect fifth, etc.)
    int semitones() const { return semitones_; }

    /// Diatonic degree number (1 = unison, 3 = third, 5 = fifth, etc.)
    int degree() const { return degree_; }

    /// Which octave the interval falls in (1 = first octave, 2 = second).
    int octave() const { return octave_; }

    /// Reduce to simple interval (first octave, 0–11 semitones).
    /// Interval("9").simple() == Interval("2M").
    Interval simple() const;

    /// Whether this interval spans more than one octave (semitones > 11).
    bool is_compound() const { return semitones_ > 11; }

    /// Complement within an octave.  Compound intervals are reduced first.
    /// Interval("5J").invert() == Interval("4J") (12 - 7 = 5).
    Interval invert() const;

    /// Consonance classification: "perfect", "imperfect", or "dissonant".
    /// Compound intervals are reduced to simple before classification.
    /// The perfect fourth is dissonant by default (contrapuntal convention);
    /// set include_fourth = true to treat it as a perfect consonance.
    std::string consonance(bool include_fourth = false) const;

    /// Whether this interval is consonant (perfect or imperfect).
    bool is_consonant(bool include_fourth = false) const;

    /// Full name in English: "Perfect Fifth", "Minor Third", etc.
    std::string full_name() const;

    /// Full name in Portuguese: "Quinta Justa", "Terça Menor", etc.
    std::string full_name_pt() const;

    Interval operator+(const Interval& other) const;
    Interval operator-(const Interval& other) const;

    bool operator==(const Interval& other) const { return semitones_ == other.semitones_; }
    bool operator!=(const Interval& other) const { return !(*this == other); }
    std::string to_string() const;

    /// Find an Interval by its label.  Returns semitone count, or -1 if unknown.
    static int label_to_semitones(const std::string& label);

    /// All 24 interval labels (two octaves).
    static const std::vector<std::string>& all_labels();

private:
    std::string label_;
    std::string anglo_saxon_;
    int         semitones_;
    int         degree_;
    int         octave_;
};

} // namespace gingo
