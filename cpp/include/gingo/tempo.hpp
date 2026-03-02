// Gingo — Music Theory Library
// Tempo: beats per minute and time conversions.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace gingo {

class Duration;  // forward declaration

/// Represents a musical tempo in beats per minute (BPM).
///
/// Converts durations to absolute time (seconds) and provides
/// standard tempo marking names (Largo, Andante, Allegro, etc.).
///
/// Examples:
///   Tempo t(120);              // 120 BPM
///   t.bpm();                   // 120
///   t.marking();               // "Allegro"
///   t.seconds(Duration("quarter"));  // 0.5 seconds
class Tempo {
public:
    /// Construct from BPM value.
    explicit Tempo(double bpm);

    /// Construct from a standard tempo marking name.
    /// Valid: "Grave", "Largo", "Adagio", "Andante", "Moderato",
    ///        "Allegro", "Vivace", "Presto", "Prestissimo"
    explicit Tempo(const std::string& marking);

    /// Beats per minute.
    double bpm() const { return bpm_; }

    /// Standard tempo marking name (approximate).
    /// Returns the closest standard marking based on BPM.
    std::string marking() const;

    /// Convert a duration to seconds.
    /// Assumes quarter note = 1 beat by default.
    double seconds(const Duration& duration) const;

    /// Milliseconds per beat (useful for MIDI timing).
    double ms_per_beat() const { return 60000.0 / bpm_; }

    /// Microseconds per quarter-note beat (MIDI tempo meta-event format).
    int microseconds_per_beat() const;

    bool operator==(const Tempo& other) const { return bpm_ == other.bpm_; }
    bool operator!=(const Tempo& other) const { return !(*this == other); }

    std::string to_string() const;

    // -- Static utilities ------------------------------------------------------

    /// Map BPM to the closest standard tempo marking.
    static std::string bpm_to_marking(double bpm);

    /// Map tempo marking to typical BPM (midpoint of range).
    static double marking_to_bpm(const std::string& marking);

    /// Create a Tempo from microseconds per beat (MIDI tempo meta-event).
    static Tempo from_microseconds(int usec);

private:
    double bpm_;
};

}  // namespace gingo
