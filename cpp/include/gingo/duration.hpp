// Gingo — Music Theory Library
// Duration: rhythmic durations and time values.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <vector>
#include <utility>
#include <cmath>

namespace gingo {

/// Represents a rhythmic duration (whole, half, quarter, eighth, etc.).
///
/// Durations are stored as rational values (numerator/denominator) to avoid
/// floating-point precision issues. They can be dotted, double-dotted, or
/// triplet/tuplet-based.
///
/// Examples:
///   Duration d("quarter");      // 1/4 note
///   d.beats();                  // 1.0
///   d.name();                   // "quarter"
///
///   Duration dotted("quarter", 1);  // dotted quarter = 1.5 beats
///   Duration triplet("eighth", 0, 3);  // eighth triplet
class Duration {
public:
    /// Construct from a duration name.
    /// Valid names: "whole", "half", "quarter", "eighth", "sixteenth", "thirty_second", "sixty_fourth"
    explicit Duration(const std::string& name, int dots = 0, int tuplet = 0);

    /// Construct from a rational value (numerator/denominator).
    /// Duration(1, 4) = quarter note, Duration(3, 8) = dotted quarter.
    Duration(int numerator, int denominator);

    /// Duration name: "whole", "half", "quarter", etc.
    const std::string& name() const { return name_; }

    /// Number of dots (0 = none, 1 = dotted, 2 = double-dotted).
    int dots() const { return dots_; }

    /// Tuplet division (0 = none, 3 = triplet, 5 = quintuplet, etc.).
    int tuplet() const { return tuplet_; }

    /// Duration in beats (quarter-note = 1.0 by default).
    /// Configurable via reference_value.
    double beats(double reference_value = 1.0) const;

    /// Duration as a rational fraction (numerator, denominator).
    /// quarter = (1, 4), dotted quarter = (3, 8).
    std::pair<int, int> rational() const { return {numerator_, denominator_}; }

    /// Numerator of the rational duration.
    int numerator() const { return numerator_; }

    /// Denominator of the rational duration.
    int denominator() const { return denominator_; }

    /// Add two durations (produces a new rational duration).
    Duration operator+(const Duration& other) const;

    /// Multiply duration by a scalar (for tuplets, augmentation).
    Duration operator*(double factor) const;

    bool operator==(const Duration& other) const {
        return numerator_ * other.denominator_ == other.numerator_ * denominator_;
    }
    bool operator!=(const Duration& other) const { return !(*this == other); }
    bool operator<(const Duration& other) const {
        return numerator_ * other.denominator_ < other.numerator_ * denominator_;
    }

    std::string to_string() const;

    // -- MIDI utilities ---------------------------------------------------------

    /// Convert this duration to MIDI ticks.
    /// Quarter note = ppqn ticks. Half = 2*ppqn. Eighth = ppqn/2.
    int midi_ticks(int ppqn = 480) const;

    /// Create a Duration from a MIDI tick count.
    /// @param ticks  Number of MIDI ticks.
    /// @param ppqn   Pulses per quarter note (default 480).
    static Duration from_ticks(int ticks, int ppqn = 480);

    // -- Static utilities ------------------------------------------------------

    /// All standard duration names.
    static const std::vector<std::string>& standard_names();

private:
    std::string name_;
    int dots_;
    int tuplet_;
    int numerator_;
    int denominator_;

    void compute_rational();

    /// Parse flexible notation into (name, dots) pair.
    /// Returns {"", -1} for fraction strings that should use rational path.
    static std::pair<std::string, int> parse_notation(const std::string& input);
};

}  // namespace gingo
