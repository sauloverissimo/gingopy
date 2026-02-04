// Gingo — Music Theory Library
// Tempo implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/tempo.hpp>
#include <gingo/duration.hpp>
#include <stdexcept>
#include <sstream>
#include <map>
#include <vector>

namespace gingo {

// Standard tempo markings with BPM ranges (midpoint used for conversion)
static const std::vector<std::pair<std::string, std::pair<double, double>>> TEMPO_MARKINGS = {
    {"Grave",       {25,  45}},
    {"Largo",       {40,  60}},
    {"Lento",       {45,  60}},
    {"Adagio",      {55,  75}},
    {"Andante",     {76,  108}},
    {"Moderato",    {108, 120}},
    {"Allegro",     {120, 156}},
    {"Vivace",      {156, 176}},
    {"Presto",      {168, 200}},
    {"Prestissimo", {200, 240}}
};

// ---------------------------------------------------------------------------
// Constructor

Tempo::Tempo(double bpm) : bpm_(bpm) {
    if (bpm <= 0) {
        throw std::invalid_argument("BPM must be positive");
    }
}

Tempo::Tempo(const std::string& marking) {
    double bpm_value = marking_to_bpm(marking);
    if (bpm_value <= 0) {
        throw std::invalid_argument("Unknown tempo marking: " + marking);
    }
    bpm_ = bpm_value;
}

// ---------------------------------------------------------------------------
// Accessors

std::string Tempo::marking() const {
    return bpm_to_marking(bpm_);
}

double Tempo::seconds(const Duration& duration) const {
    // Quarter note = 1 beat by default
    // Duration in beats × (60 / BPM) = seconds
    double beats = duration.beats();
    return beats * (60.0 / bpm_);
}

// ---------------------------------------------------------------------------
// String representation

std::string Tempo::to_string() const {
    std::ostringstream oss;
    oss << static_cast<int>(bpm_) << " BPM (" << marking() << ")";
    return oss.str();
}

// ---------------------------------------------------------------------------
// Static utilities

std::string Tempo::bpm_to_marking(double bpm) {
    // Find the closest marking
    std::string closest = "Moderato";
    double min_distance = 1000.0;

    for (const auto& [name, range] : TEMPO_MARKINGS) {
        double midpoint = (range.first + range.second) / 2.0;
        double distance = std::abs(bpm - midpoint);
        if (distance < min_distance) {
            min_distance = distance;
            closest = name;
        }
    }

    return closest;
}

double Tempo::marking_to_bpm(const std::string& marking) {
    for (const auto& [name, range] : TEMPO_MARKINGS) {
        if (name == marking) {
            return (range.first + range.second) / 2.0;
        }
    }
    return -1.0;  // Invalid
}

}  // namespace gingo
