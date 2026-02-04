// Gingo — Music Theory Library
// Note: the atomic unit of Western music.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>
#include <string>
#include <vector>

namespace gingo {

/// Represents a single musical pitch class (e.g. "C", "Bb", "F#").
///
/// Notes are stored in their original form and can be converted to a
/// canonical sharp-based representation (natural form).
///
/// Examples:
///   Note n("Bb");
///   n.name();          // "Bb"
///   n.natural();       // "A#"
///   n.sound();         // "B"
///   n.semitone();      // 10
///   n.frequency(4);    // 466.16 Hz
class Note {
public:
    /// Construct from any accepted notation: "C", "Bb", "F#", "Gb", "E♭", etc.
    explicit Note(const std::string& name);

    /// The name as given at construction time.
    const std::string& name() const { return name_; }

    /// Canonical sharp-based form: "Bb" → "A#", "Gb" → "F#", "C" → "C".
    const std::string& natural() const { return natural_; }

    /// The base letter only (no accidentals): "Bb" → "B", "F#" → "F".
    const std::string& sound() const { return sound_; }

    /// Position in the chromatic scale (C=0, C#=1, … B=11).
    int semitone() const { return semitone_; }

    /// Concert pitch frequency in Hz.
    /// @param octave  Octave number (default 4, where A4 = tuning reference).
    /// @param tuning  Reference frequency for A4 in Hz (default 440.0).
    double frequency(int octave = 4, double tuning = 440.0) const {
        return tuning * std::pow(2.0, (semitone_ - 9 + 12 * (octave - 4)) / 12.0);
    }

    /// Enharmonic equivalence: Note("Bb") == Note("A#").
    bool is_enharmonic(const Note& other) const { return semitone_ == other.semitone_; }

    /// Transpose by a number of semitones (positive = up, negative = down).
    Note transpose(int semitones) const;

    bool operator==(const Note& other) const { return natural_ == other.natural_; }
    bool operator!=(const Note& other) const { return !(*this == other); }
    std::string to_string() const;

    // -- Static utilities ported from the original note module ----------------

    /// Resolve any enharmonic spelling to sharp-based canonical form.
    static std::string to_natural(const std::string& note_name);

    /// Extract the root note from a chord name: "C#m7" → "C#".
    static std::string extract_root(const std::string& chord_name);

    /// Extract just the base letter: "C#m7" → "C".
    static std::string extract_sound(const std::string& name);

    /// Extract the chord-type suffix: "C#m7" → "m7".
    static std::string extract_type(const std::string& chord_name);

    /// Map any note name to its chromatic index 0–11.
    static int to_semitone(const std::string& note_name);

    /// The 12 chromatic pitch classes in sharp notation.
    static const std::vector<std::string>& chromatic();

    /// The 12 pitch classes ordered by ascending perfect fifths.
    /// C, G, D, A, E, B, F#, C#, G#, D#, A#, F
    static const std::vector<std::string>& fifths();

    /// Shortest distance to another note on the circle of fifths (0–6).
    int distance(const Note& other) const;

private:
    std::string name_;      // original input
    std::string natural_;   // sharp-based canonical
    std::string sound_;     // base letter
    int         semitone_;  // 0–11
};

} // namespace gingo
