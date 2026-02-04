// Gingo — Music Theory Library
// Implementation of the Note class.
//
// SPDX-License-Identifier: MIT

#include "gingo/note.hpp"
#include "gingo/internal/lookup_data.hpp"

#include <algorithm>
#include <string>

namespace gingo {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

const std::vector<std::string>& Note::chromatic() {
    static const std::vector<std::string> scale = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };
    return scale;
}

const std::vector<std::string>& Note::fifths() {
    static const std::vector<std::string> circle = {
        "C", "G", "D", "A", "E", "B", "F#", "C#", "G#", "D#", "A#", "F"
    };
    return circle;
}

int Note::distance(const Note& other) const {
    int pos_a = (semitone_ * 7) % 12;
    int pos_b = (other.semitone_ * 7) % 12;
    int raw = std::abs(pos_a - pos_b);
    return std::min(raw, 12 - raw);
}

std::string Note::to_natural(const std::string& note_name) {
    const auto& map = internal::LookupData::instance().enharmonic_map();
    auto it = map.find(note_name);
    if (it != map.end()) {
        return it->second;
    }
    return note_name;
}

int Note::to_semitone(const std::string& note_name) {
    const std::string natural = to_natural(note_name);
    const auto& scale = chromatic();
    for (int i = 0; i < static_cast<int>(scale.size()); ++i) {
        if (scale[i] == natural) {
            return i;
        }
    }
    return 0;
}

std::string Note::extract_sound(const std::string& name) {
    for (char c : name) {
        if (c >= 'A' && c <= 'G') {
            return std::string(1, c);
        }
    }
    return {};
}

// ---------------------------------------------------------------------------
// extract_root / extract_type
// ---------------------------------------------------------------------------

namespace {

/// Check whether the character at position @p pos in @p s is a single-byte
/// accidental ('b' or '#'), or whether a multi-byte Unicode accidental
/// (U+266D flat or U+266F sharp) starts at that position.
/// Returns the number of bytes consumed (0 if not an accidental).
std::size_t accidental_length(const std::string& s, std::size_t pos) {
    if (pos >= s.size()) return 0;

    char ch = s[pos];
    if (ch == 'b' || ch == '#') return 1;

    // UTF-8: U+266D (flat)  = 0xE2 0x99 0xAD
    //        U+266F (sharp) = 0xE2 0x99 0xAF
    if (pos + 2 < s.size() &&
        static_cast<unsigned char>(s[pos])     == 0xE2 &&
        static_cast<unsigned char>(s[pos + 1]) == 0x99) {
        unsigned char third = static_cast<unsigned char>(s[pos + 2]);
        if (third == 0xAD || third == 0xAF) return 3;
    }

    return 0;
}

} // anonymous namespace

std::string Note::extract_root(const std::string& chord_name) {
    // Locate the first uppercase letter A-G.
    std::size_t letter_pos = std::string::npos;
    for (std::size_t i = 0; i < chord_name.size(); ++i) {
        char c = chord_name[i];
        if (c >= 'A' && c <= 'G') {
            letter_pos = i;
            break;
        }
    }

    if (letter_pos == std::string::npos) {
        return {};
    }

    // Start building the root note.
    std::string root(1, chord_name[letter_pos]);

    // Check for accidentals immediately after the letter.
    // Supports single (#, b) and double (##, bb) accidentals.
    std::size_t after = letter_pos + 1;
    std::size_t acc_len = accidental_length(chord_name, after);
    if (acc_len > 0) {
        root += chord_name.substr(after, acc_len);
        // Check for a second accidental (e.g. F## or Bbb).
        std::size_t after2 = after + acc_len;
        std::size_t acc_len2 = accidental_length(chord_name, after2);
        if (acc_len2 > 0) {
            char first_acc = chord_name[after];
            char second_acc = chord_name[after2];
            // Only consume second if same type (## or bb, not #b or b#).
            if (first_acc == second_acc) {
                root += chord_name.substr(after2, acc_len2);
            }
        }
        return root;
    }

    // Check for accidentals immediately before the letter (e.g. "bD", "bbD").
    if (letter_pos > 0) {
        // Single-byte accidental right before the letter.
        char prev = chord_name[letter_pos - 1];
        if (prev == 'b' || prev == '#') {
            root += prev;
            // Check for double accidental (e.g. "bbD" or "##D").
            if (letter_pos >= 2) {
                char prev2 = chord_name[letter_pos - 2];
                if (prev2 == prev) {
                    root += prev2;
                }
            }
            return root;
        }

        // Multi-byte Unicode accidental ending right before the letter.
        if (letter_pos >= 3) {
            std::size_t start = letter_pos - 3;
            std::size_t ulen = accidental_length(chord_name, start);
            if (ulen == 3) {
                root += chord_name.substr(start, 3);
                return root;
            }
        }
    }

    return root;
}

std::string Note::extract_type(const std::string& chord_name) {
    // Locate the first uppercase letter A-G.
    std::size_t letter_pos = std::string::npos;
    for (std::size_t i = 0; i < chord_name.size(); ++i) {
        char c = chord_name[i];
        if (c >= 'A' && c <= 'G') {
            letter_pos = i;
            break;
        }
    }

    if (letter_pos == std::string::npos) {
        return {};
    }

    // Skip past the letter.
    std::size_t after = letter_pos + 1;

    // Skip trailing accidentals (single or double: #, b, ##, bb).
    std::size_t acc_len = accidental_length(chord_name, after);
    if (acc_len > 0) {
        char first_acc = chord_name[after];
        after += acc_len;
        // Check for second accidental of the same type.
        std::size_t acc_len2 = accidental_length(chord_name, after);
        if (acc_len2 > 0 && after < chord_name.size() &&
            chord_name[after] == first_acc) {
            after += acc_len2;
        }
    }

    // Everything remaining is the chord type.
    if (after >= chord_name.size()) {
        return {};
    }
    return chord_name.substr(after);
}

// ---------------------------------------------------------------------------
// Constructor & instance methods
// ---------------------------------------------------------------------------

Note::Note(const std::string& name)
    : name_(name)
    , natural_(to_natural(name))
    , sound_(extract_sound(name))
    , semitone_(to_semitone(name))
{}

Note Note::transpose(int semitones) const {
    int new_index = (semitone_ + semitones % 12 + 12) % 12;
    return Note(chromatic()[static_cast<std::size_t>(new_index)]);
}

std::string Note::to_string() const {
    return "Note(\"" + name_ + "\")";
}

} // namespace gingo
