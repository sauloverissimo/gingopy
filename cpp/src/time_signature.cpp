// Gingo — Music Theory Library
// TimeSignature implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/time_signature.hpp>
#include <gingo/duration.hpp>
#include <stdexcept>
#include <sstream>

namespace gingo {

// ---------------------------------------------------------------------------
// Constructor

TimeSignature::TimeSignature(int beats_per_bar, int beat_unit)
    : beats_per_bar_(beats_per_bar), beat_unit_(beat_unit)
{
    if (beats_per_bar <= 0) {
        throw std::invalid_argument("Beats per bar must be positive");
    }
    if (beat_unit != 1 && beat_unit != 2 && beat_unit != 4 &&
        beat_unit != 8 && beat_unit != 16 && beat_unit != 32) {
        throw std::invalid_argument("Beat unit must be 1, 2, 4, 8, 16, or 32");
    }
}

// ---------------------------------------------------------------------------
// Accessors

Duration TimeSignature::bar_duration() const {
    // Total duration of one bar
    // 4/4 = 4 quarter notes = 4/4 = whole note
    // 6/8 = 6 eighth notes = 6/8 = 3/4
    return Duration(beats_per_bar_, beat_unit_);
}

std::string TimeSignature::classification() const {
    // Compound: divisible by 3 and beat_unit = 8
    if (beats_per_bar_ % 3 == 0 && beat_unit_ == 8) {
        return "compound";
    }
    return "simple";
}

std::string TimeSignature::common_name() const {
    if (beats_per_bar_ == 4 && beat_unit_ == 4) {
        return "common time";
    }
    if (beats_per_bar_ == 2 && beat_unit_ == 2) {
        return "cut time";
    }
    return to_string();
}

// ---------------------------------------------------------------------------
// String representation

std::string TimeSignature::to_string() const {
    std::ostringstream oss;
    oss << beats_per_bar_ << "/" << beat_unit_;
    return oss.str();
}

}  // namespace gingo
