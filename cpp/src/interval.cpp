// Gingo — Music Theory Library
// Implementation of the Interval class.
//
// SPDX-License-Identifier: MIT

#include "gingo/interval.hpp"
#include "gingo/note.hpp"
#include "gingo/internal/lookup_data.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

const std::vector<std::string>& Interval::all_labels()
{
    static const std::vector<std::string> labels = [] {
        const auto& row =
            internal::LookupData::instance().intervals().row("ilabel");
        std::vector<std::string> result;
        result.reserve(row.size());
        for (const auto& elem : row) {
            result.push_back(internal::element_to_string(elem));
        }
        return result;
    }();
    return labels;
}

int Interval::label_to_semitones(const std::string& label)
{
    const auto& labels = all_labels();
    for (int i = 0; i < static_cast<int>(labels.size()); ++i) {
        if (labels[i] == label) return i;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Interval::Interval(const std::string& label)
{
    const auto& table     = internal::LookupData::instance().intervals();
    const auto& ilabel_row = table.row("ilabel");

    int index = -1;
    for (int i = 0; i < static_cast<int>(ilabel_row.size()); ++i) {
        if (internal::element_to_string(ilabel_row[static_cast<std::size_t>(i)]) == label) {
            index = i;
            break;
        }
    }

    if (index < 0) {
        throw std::invalid_argument("Interval: unknown label: " + label);
    }

    const auto idx = static_cast<std::size_t>(index);

    label_       = label;
    semitones_   = index;
    anglo_saxon_ = internal::element_to_string(table.row("anglo_saxon")[idx]);
    degree_      = internal::element_to_int(table.row("degree")[idx]);
    octave_      = internal::element_to_int(table.row("octaveid")[idx]);
}

Interval::Interval(int semitones)
{
    if (semitones < 0 || semitones > 23) {
        throw std::invalid_argument(
            "Interval: semitones out of range (0-23): "
            + std::to_string(semitones));
    }

    const auto& table = internal::LookupData::instance().intervals();
    const auto  idx   = static_cast<std::size_t>(semitones);

    label_       = internal::element_to_string(table.row("ilabel")[idx]);
    semitones_   = semitones;
    anglo_saxon_ = internal::element_to_string(table.row("anglo_saxon")[idx]);
    degree_      = internal::element_to_int(table.row("degree")[idx]);
    octave_      = internal::element_to_int(table.row("octaveid")[idx]);
}

Interval::Interval(const Note& from, const Note& to)
    : Interval((to.semitone() - from.semitone() + 12) % 12)
{}

// ---------------------------------------------------------------------------
// Derived intervals
// ---------------------------------------------------------------------------

Interval Interval::simple() const
{
    return Interval(semitones_ % 12);
}

Interval Interval::invert() const
{
    const int r = semitones_ % 12;
    if (r == 0) {
        // P1 (0) -> P8 (12), P8 (12) -> P1 (0)
        return Interval(semitones_ == 0 ? 12 : 0);
    }
    return Interval(12 - r);
}

// ---------------------------------------------------------------------------
// Consonance
// ---------------------------------------------------------------------------

std::string Interval::consonance(bool include_fourth) const
{
    const int s = semitones_ % 12;
    switch (s) {
        case 0: case 7:
            return "perfect";
        case 5:
            return include_fourth ? "perfect" : "dissonant";
        case 3: case 4: case 8: case 9:
            return "imperfect";
        default:
            return "dissonant";
    }
}

bool Interval::is_consonant(bool include_fourth) const
{
    const std::string c = consonance(include_fourth);
    return c == "perfect" || c == "imperfect";
}

// ---------------------------------------------------------------------------
// Full names
// ---------------------------------------------------------------------------

std::string Interval::full_name() const
{
    static const char* names[] = {
        "Perfect Unison",       // 0
        "Minor Second",         // 1
        "Major Second",         // 2
        "Minor Third",          // 3
        "Major Third",          // 4
        "Perfect Fourth",       // 5
        "Tritone",              // 6
        "Perfect Fifth",        // 7
        "Minor Sixth",          // 8
        "Major Sixth",          // 9
        "Minor Seventh",        // 10
        "Major Seventh",        // 11
        "Perfect Octave",       // 12
        "Minor Ninth",          // 13
        "Major Ninth",          // 14
        "Minor Tenth",          // 15
        "Major Tenth",          // 16
        "Perfect Eleventh",     // 17
        "Augmented Eleventh",   // 18
        "Perfect Twelfth",      // 19
        "Minor Thirteenth",     // 20
        "Major Thirteenth",     // 21
        "Minor Fourteenth",     // 22
        "Major Fourteenth"      // 23
    };
    return names[static_cast<std::size_t>(semitones_)];
}

std::string Interval::full_name_pt() const
{
    static const char* names[] = {
        "Unissono Justo",           // 0
        "Segunda Menor",            // 1
        "Segunda Maior",            // 2
        "Terca Menor",              // 3
        "Terca Maior",              // 4
        "Quarta Justa",             // 5
        "Tritono",                  // 6
        "Quinta Justa",             // 7
        "Sexta Menor",              // 8
        "Sexta Maior",              // 9
        "Setima Menor",             // 10
        "Setima Maior",             // 11
        "Oitava Justa",             // 12
        "Nona Menor",               // 13
        "Nona Maior",               // 14
        "Decima Menor",             // 15
        "Decima Maior",             // 16
        "Decima Primeira Justa",    // 17
        "Decima Primeira Aumentada",// 18
        "Decima Segunda Justa",     // 19
        "Decima Terceira Menor",    // 20
        "Decima Terceira Maior",    // 21
        "Decima Quarta Menor",      // 22
        "Decima Quarta Maior"       // 23
    };
    return names[static_cast<std::size_t>(semitones_)];
}

// ---------------------------------------------------------------------------
// Arithmetic
// ---------------------------------------------------------------------------

Interval Interval::operator+(const Interval& other) const
{
    const int result = semitones_ + other.semitones_;
    if (result > 23) {
        throw std::overflow_error(
            "Interval: sum exceeds 23 semitones: "
            + std::to_string(semitones_) + " + "
            + std::to_string(other.semitones_));
    }
    return Interval(result);
}

Interval Interval::operator-(const Interval& other) const
{
    const int result = semitones_ - other.semitones_;
    if (result < 0) {
        throw std::underflow_error(
            "Interval: subtraction yields negative: "
            + std::to_string(semitones_) + " - "
            + std::to_string(other.semitones_));
    }
    return Interval(result);
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

std::string Interval::to_string() const
{
    return "Interval(\"" + label_ + "\")";
}

} // namespace gingo
