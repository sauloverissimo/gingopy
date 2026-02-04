// Gingo — Music Theory Library
// Implementation of internal notation utility functions.
//
// SPDX-License-Identifier: MIT

#include "gingo/internal/notation_utils.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace gingo::internal {

// ---------------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------------

namespace {

/// The twelve-tone chromatic scale used as the reference for all semitone
/// calculations throughout this translation unit.
const std::vector<std::string>& chromatic_scale() {
    static const std::vector<std::string> scale = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };
    return scale;
}

/// Find the index of @p note inside the chromatic scale.  Returns -1 when
/// the note is not found.
int chromatic_index(const std::string& note) {
    const auto& scale = chromatic_scale();
    for (int i = 0; i < static_cast<int>(scale.size()); ++i) {
        if (scale[i] == note) return i;
    }
    return -1;
}

/// Extract the diatonic sound letter (A-G) from a note name.
/// For example: "C#" -> "C", "Db" -> "D", "F##" -> "F".
std::string extract_sound(const std::string& note) {
    for (char c : note) {
        if (c >= 'A' && c <= 'G') {
            return std::string(1, c);
        }
    }
    return {};
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

int half_tones(const std::string& target_note,
               const std::string& start_note,
               const std::string& range) {
    int target_idx = chromatic_index(target_note);
    int start_idx  = chromatic_index(start_note);

    int diff = target_idx - start_idx;

    if (range == "min") {
        if (diff > 6)  diff -= 12;
        if (diff < -6) diff += 12;
    }

    return diff;
}

std::string natural_to_formal(const std::string& base_note,
                              const std::string& target_note) {
    int base_idx   = chromatic_index(base_note);
    int target_idx = chromatic_index(target_note);

    int diff = target_idx - base_idx;
    if (diff > 6)  diff -= 12;
    if (diff < -6) diff += 12;

    std::string result = base_note;
    if (diff > 0) {
        result.append(static_cast<std::string::size_type>(diff), '#');
    } else if (diff < 0) {
        result.append(static_cast<std::string::size_type>(-diff), 'b');
    }

    return result;
}

std::vector<std::string> rotate_remove(const std::vector<std::string>& base,
                                       const std::string& tonic,
                                       int total_length,
                                       const std::vector<int>& remove_indices) {
    // Locate the tonic inside the base vector.
    int start_index = 0;
    for (int i = 0; i < static_cast<int>(base.size()); ++i) {
        if (base[i] == tonic) {
            start_index = i;
            break;
        }
    }

    const int n = static_cast<int>(base.size());

    // Build the rotated & extended vector.
    std::vector<std::string> extended;
    extended.reserve(static_cast<std::size_t>(total_length));
    for (int i = 0; i < total_length; ++i) {
        extended.push_back(base[static_cast<std::size_t>((start_index + i) % n)]);
    }

    // Remove entries at the requested positions (erase from back to front so
    // that earlier removals do not shift later indices).
    std::vector<int> sorted_indices = remove_indices;
    std::sort(sorted_indices.rbegin(), sorted_indices.rend());
    for (int idx : sorted_indices) {
        if (idx >= 0 && idx < static_cast<int>(extended.size())) {
            extended.erase(extended.begin() + idx);
        }
    }

    return extended;
}

std::vector<std::string> get_formal_notes(
        const std::vector<std::string>& chromatic_notes,
        const std::string& tonic_original,
        const std::vector<int>& remove_indices) {

    const std::string diatonic_tonic = extract_sound(tonic_original);

    static const std::vector<std::string> base_diatonic = {
        "C", "D", "E", "F", "G", "A", "B"
    };

    // Build the rotated diatonic letter sequence.
    std::vector<std::string> rotate_scale =
        rotate_remove(base_diatonic, diatonic_tonic, 14, remove_indices);

    // Assign one diatonic letter to each non-empty chromatic entry.
    const std::size_t count = chromatic_notes.size();
    std::vector<std::string> diatonic_scale(count);
    std::size_t letter_idx = 0;

    for (std::size_t i = 0; i < count; ++i) {
        const std::string& note = chromatic_notes[i];
        if (!note.empty() && note != "0") {
            if (letter_idx < rotate_scale.size()) {
                diatonic_scale[i] = rotate_scale[letter_idx];
                ++letter_idx;
            }
        }
    }

    // Convert each pair (diatonic letter, chromatic name) to formal notation.
    std::vector<std::string> final_scale(count);
    for (std::size_t i = 0; i < count; ++i) {
        const std::string& note = chromatic_notes[i];
        if (note.empty() || note == "0") {
            final_scale[i] = note;
        } else {
            final_scale[i] = natural_to_formal(diatonic_scale[i], note);
        }
    }

    return final_scale;
}

TypeVector get_formal_notes_tv(
        const TypeVector& chromatic_notes,
        const std::string& tonic_original,
        const std::vector<int>& remove_indices) {

    // Extract plain strings from the TypeVector.
    std::vector<std::string> strings;
    strings.reserve(chromatic_notes.size());
    for (const auto& elem : chromatic_notes) {
        strings.push_back(element_to_string(elem));
    }

    // Process through the string-based overload.
    std::vector<std::string> formal =
        get_formal_notes(strings, tonic_original, remove_indices);

    // Re-pack into a TypeVector.
    TypeVector result;
    result.reserve(formal.size());
    for (auto& s : formal) {
        result.emplace_back(std::move(s));
    }

    return result;
}

TypeVector get_natural_notes(
        const TypeVector& scale_notes,
        const std::vector<int>& remove_indices) {

    // Extract plain strings.
    std::vector<std::string> strings;
    strings.reserve(scale_notes.size());
    for (const auto& elem : scale_notes) {
        strings.push_back(element_to_string(elem));
    }

    // Use the first non-empty entry as the tonic reference.
    std::string tonic;
    for (const auto& s : strings) {
        if (!s.empty() && s != "0") {
            tonic = s;
            break;
        }
    }

    if (tonic.empty()) {
        return scale_notes;  // nothing to convert
    }

    // Convert and re-pack.
    std::vector<std::string> formal =
        get_formal_notes(strings, tonic, remove_indices);

    TypeVector result;
    result.reserve(formal.size());
    for (auto& s : formal) {
        result.emplace_back(std::move(s));
    }

    return result;
}

} // namespace gingo::internal
