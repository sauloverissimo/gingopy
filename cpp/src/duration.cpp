// Gingo — Music Theory Library
// Duration implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/duration.hpp>
#include <stdexcept>
#include <sstream>
#include <numeric>  // for std::gcd
#include <cmath>
#include <map>

namespace gingo {

// Static data
static const std::vector<std::string> DURATION_NAMES = {
    "whole", "half", "quarter", "eighth", "sixteenth", "thirty_second", "sixty_fourth"
};

static const std::map<std::string, int> NAME_TO_DENOM = {
    {"whole", 1},
    {"half", 2},
    {"quarter", 4},
    {"eighth", 8},
    {"sixteenth", 16},
    {"thirty_second", 32},
    {"sixty_fourth", 64}
};

// ---------------------------------------------------------------------------
// Constructor

Duration::Duration(const std::string& name, int dots, int tuplet)
    : name_(name), dots_(dots), tuplet_(tuplet), numerator_(1), denominator_(4)
{
    auto it = NAME_TO_DENOM.find(name);
    if (it == NAME_TO_DENOM.end()) {
        throw std::invalid_argument("Invalid duration name: " + name);
    }
    if (dots < 0 || dots > 2) {
        throw std::invalid_argument("Dots must be 0, 1, or 2");
    }
    if (tuplet < 0) {
        throw std::invalid_argument("Tuplet must be non-negative");
    }

    denominator_ = it->second;
    compute_rational();
}

Duration::Duration(int numerator, int denominator)
    : name_("custom"), dots_(0), tuplet_(0), numerator_(numerator), denominator_(denominator)
{
    if (denominator <= 0) {
        throw std::invalid_argument("Denominator must be positive");
    }
    if (numerator < 0) {
        throw std::invalid_argument("Numerator must be non-negative");
    }

    // Simplify fraction
    int g = std::gcd(numerator_, denominator_);
    numerator_ /= g;
    denominator_ /= g;

    // Try to match a standard name
    for (const auto& [n, d] : NAME_TO_DENOM) {
        if (denominator_ == d && numerator_ == 1 && dots_ == 0 && tuplet_ == 0) {
            name_ = n;
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Computation

void Duration::compute_rational() {
    // Base duration
    numerator_ = 1;
    // denominator_ already set

    // Apply dots: each dot adds half of the previous value
    // dotted quarter = 1/4 + 1/8 = 3/8
    // double-dotted = 1/4 + 1/8 + 1/16 = 7/16
    if (dots_ > 0) {
        // dotted: 1/d + 1/2d = 3/2d  →  multiply denom by 2^dots
        // double-dotted: 1/d + 1/2d + 1/4d = 7/4d  →  multiply denom by 4
        int denom_factor = 1 << dots_;  // 2^dots
        denominator_ *= denom_factor;
        // numerator: 2^dots + 2^(dots-1) + ... + 1 = 2^(dots+1) - 1
        numerator_ = (1 << (dots_ + 1)) - 1;
    }

    // Apply tuplet: divide by tuplet ratio
    // triplet eighth = (1/8) * (2/3) = 1/12
    if (tuplet_ > 0) {
        // Standard tuplet ratios: triplet = 2/3, quintuplet = 4/5, etc.
        // General formula: (tuplet-1) / tuplet
        int ratio_num = (tuplet_ == 3) ? 2 : (tuplet_ - 1);
        numerator_ *= ratio_num;
        denominator_ *= tuplet_;
    }

    // Simplify
    int g = std::gcd(numerator_, denominator_);
    numerator_ /= g;
    denominator_ /= g;
}

// ---------------------------------------------------------------------------
// Accessors

double Duration::beats(double reference_value) const {
    // Beats assuming quarter note = reference_value
    // quarter = 1/4, reference = 1.0 → 1.0 beats
    // half = 1/2, reference = 1.0 → 2.0 beats
    double base_quarter_beats = static_cast<double>(numerator_) / denominator_;
    return base_quarter_beats * 4.0 * reference_value;
}

// ---------------------------------------------------------------------------
// Operators

Duration Duration::operator+(const Duration& other) const {
    // Add two durations: find common denominator
    int common_denom = std::lcm(denominator_, other.denominator_);
    int new_num = numerator_ * (common_denom / denominator_) +
                  other.numerator_ * (common_denom / other.denominator_);
    return Duration(new_num, common_denom);
}

Duration Duration::operator*(double factor) const {
    // Multiply duration by scalar
    int new_num = static_cast<int>(numerator_ * factor);
    return Duration(new_num, denominator_);
}

// ---------------------------------------------------------------------------
// String representation

std::string Duration::to_string() const {
    std::ostringstream oss;
    oss << name_;
    if (dots_ > 0) {
        oss << " (";
        for (int i = 0; i < dots_; ++i) oss << ".";
        oss << ")";
    }
    if (tuplet_ > 0) {
        oss << " [" << tuplet_ << "]";
    }
    oss << " = " << numerator_ << "/" << denominator_;
    return oss.str();
}

// ---------------------------------------------------------------------------
// Static utilities

const std::vector<std::string>& Duration::standard_names() {
    return DURATION_NAMES;
}

}  // namespace gingo
