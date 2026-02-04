// Gingo — Music Theory Library
// TimeSignature: meter and bar structure.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <utility>

namespace gingo {

class Duration;  // forward declaration

/// Represents a time signature (meter): beats per bar and beat unit.
///
/// Examples: 4/4, 3/4, 6/8, 7/8, 5/4, etc.
///
/// Examples:
///   TimeSignature ts(4, 4);    // common time
///   ts.beats_per_bar();        // 4
///   ts.beat_unit();            // 4 (quarter note)
///   ts.bar_duration();         // Duration(4, 4) = whole note
class TimeSignature {
public:
    /// Construct from beats per bar and beat unit.
    /// TimeSignature(4, 4) = 4/4, TimeSignature(6, 8) = 6/8.
    TimeSignature(int beats_per_bar, int beat_unit);

    /// Number of beats per bar (numerator).
    int beats_per_bar() const { return beats_per_bar_; }

    /// Beat unit (denominator): 2=half, 4=quarter, 8=eighth, etc.
    int beat_unit() const { return beat_unit_; }

    /// As a pair (numerator, denominator).
    std::pair<int, int> signature() const { return {beats_per_bar_, beat_unit_}; }

    /// Total duration of one complete bar.
    Duration bar_duration() const;

    /// Classification: "simple" or "compound" (based on beat grouping).
    /// Compound: beats_per_bar divisible by 3 and beat_unit = 8 (6/8, 9/8, 12/8).
    std::string classification() const;

    /// Common names: "4/4" → "common time", "2/2" → "cut time".
    std::string common_name() const;

    bool operator==(const TimeSignature& other) const {
        return beats_per_bar_ == other.beats_per_bar_ && beat_unit_ == other.beat_unit_;
    }
    bool operator!=(const TimeSignature& other) const { return !(*this == other); }

    std::string to_string() const;

private:
    int beats_per_bar_;
    int beat_unit_;
};

}  // namespace gingo
