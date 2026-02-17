// Gingo — Music Theory Library
// Piano implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/piano.hpp>

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

// White keys: C, D, E, F, G, A, B  →  semitone % 12 in {0,2,4,5,7,9,11}
static constexpr bool WHITE_KEY_MAP[12] = {
    true,  false, true,  false, true,  true,   // C  C# D  D# E  F
    false, true,  false, true,  false, true     // F# G  G# A  A# B
};

bool Piano::is_white(int midi) {
    int semitone = ((midi % 12) + 12) % 12;
    return WHITE_KEY_MAP[semitone];
}

int Piano::to_midi(const Note& note, int octave) {
    return note.semitone() + 12 * (octave + 1);
}

std::string Piano::midi_to_name(int midi) {
    return Note::chromatic()[((midi % 12) + 12) % 12];
}

int Piano::midi_to_octave(int midi) {
    return midi / 12 - 1;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

Piano::Piano(int num_keys)
    : num_keys_(num_keys)
{
    if (num_keys < 1 || num_keys > 128)
        throw std::invalid_argument("num_keys must be between 1 and 128");

    // Standard 88-key piano: A0 (21) to C8 (108).
    // Smaller keyboards are centered around middle C (MIDI 60).
    if (num_keys == 88) {
        lowest_midi_  = 21;   // A0
        highest_midi_ = 108;  // C8
    } else {
        // Center around middle C (60), biased slightly downward.
        int half = num_keys / 2;
        lowest_midi_  = 60 - half;
        highest_midi_ = lowest_midi_ + num_keys - 1;

        // Clamp to valid MIDI range.
        if (lowest_midi_ < 0)   { lowest_midi_ = 0; highest_midi_ = num_keys - 1; }
        if (highest_midi_ > 127) { highest_midi_ = 127; lowest_midi_ = 128 - num_keys; }
    }
}

// ---------------------------------------------------------------------------
// Info
// ---------------------------------------------------------------------------

PianoKey Piano::lowest() const {
    return make_key(lowest_midi_);
}

PianoKey Piano::highest() const {
    return make_key(highest_midi_);
}

bool Piano::in_range(int midi) const {
    return midi >= lowest_midi_ && midi <= highest_midi_;
}

// ---------------------------------------------------------------------------
// Forward: theory → keys
// ---------------------------------------------------------------------------

PianoKey Piano::key(const Note& note, int octave) const {
    int midi = to_midi(note, octave);
    if (!in_range(midi))
        throw std::out_of_range(
            "Note " + note.name() + " in octave " + std::to_string(octave) +
            " (MIDI " + std::to_string(midi) + ") is outside piano range [" +
            std::to_string(lowest_midi_) + ", " + std::to_string(highest_midi_) + "]");
    return make_key(midi);
}

std::vector<PianoKey> Piano::keys(const Note& note) const {
    std::vector<PianoKey> result;
    int semitone = note.semitone();

    // Find first MIDI in range with matching pitch class.
    int start = lowest_midi_ + ((semitone - (lowest_midi_ % 12)) % 12 + 12) % 12;
    if (start < lowest_midi_) start += 12;

    for (int midi = start; midi <= highest_midi_; midi += 12) {
        result.push_back(make_key(midi));
    }
    return result;
}

PianoVoicing Piano::voicing(const Chord& chord, int octave,
                            VoicingStyle style) const {
    auto notes = chord.notes();
    PianoVoicing v;
    v.style      = style;
    v.chord_name = chord.name();
    v.inversion  = 0;

    if (notes.empty()) return v;

    int root_semi = notes[0].semitone();

    switch (style) {
    case VoicingStyle::Close: {
        // All notes in the same octave, starting from root.
        for (auto& n : notes) {
            int oct = octave;
            if (n.semitone() < root_semi) oct += 1;
            int midi = to_midi(n, oct);
            if (in_range(midi))
                v.keys.push_back(make_key(midi));
        }
        break;
    }
    case VoicingStyle::Open: {
        // Root one octave lower, remaining notes in base octave.
        int root_midi = to_midi(notes[0], octave - 1);
        if (in_range(root_midi))
            v.keys.push_back(make_key(root_midi));

        for (std::size_t i = 1; i < notes.size(); ++i) {
            int oct = octave;
            if (notes[i].semitone() < root_semi) oct += 1;
            int midi = to_midi(notes[i], oct);
            if (in_range(midi))
                v.keys.push_back(make_key(midi));
        }
        break;
    }
    case VoicingStyle::Shell: {
        // Jazz shell: root + 3rd + 7th (for tetrads), or root + 3rd + 5th (triads).
        std::vector<std::size_t> indices;
        indices.push_back(0);  // root

        auto labels = chord.interval_labels();
        // Find 3rd (minor or major).
        for (std::size_t i = 1; i < labels.size(); ++i) {
            if (labels[i] == "3m" || labels[i] == "3M") {
                indices.push_back(i);
                break;
            }
        }

        // Find 7th (minor or major) for tetrads.
        bool found_seventh = false;
        for (std::size_t i = 1; i < labels.size(); ++i) {
            if (labels[i] == "7m" || labels[i] == "7M") {
                indices.push_back(i);
                found_seventh = true;
                break;
            }
        }

        // No 7th? Use 5th instead.
        if (!found_seventh) {
            for (std::size_t i = 1; i < labels.size(); ++i) {
                if (labels[i] == "5J" || labels[i] == "5dim" || labels[i] == "5aug") {
                    indices.push_back(i);
                    break;
                }
            }
        }

        for (auto idx : indices) {
            if (idx >= notes.size()) continue;
            int oct = octave;
            if (notes[idx].semitone() < root_semi) oct += 1;
            int midi = to_midi(notes[idx], oct);
            if (in_range(midi))
                v.keys.push_back(make_key(midi));
        }
        break;
    }
    }

    // Sort by MIDI (low to high).
    std::sort(v.keys.begin(), v.keys.end(),
              [](const PianoKey& a, const PianoKey& b) { return a.midi < b.midi; });

    return v;
}

std::vector<PianoVoicing> Piano::voicings(const Chord& chord,
                                          int octave) const {
    return {
        voicing(chord, octave, VoicingStyle::Close),
        voicing(chord, octave, VoicingStyle::Open),
        voicing(chord, octave, VoicingStyle::Shell),
    };
}

std::vector<PianoKey> Piano::scale_keys(const Scale& scale,
                                        int octave) const {
    std::vector<PianoKey> result;
    auto notes = scale.notes();
    int tonic_semi = scale.tonic().semitone();

    for (auto& n : notes) {
        int oct = octave;
        if (n.semitone() < tonic_semi) oct += 1;
        int midi = to_midi(n, oct);
        if (in_range(midi))
            result.push_back(make_key(midi));
    }
    return result;
}

// ---------------------------------------------------------------------------
// Reverse: keys → theory
// ---------------------------------------------------------------------------

Note Piano::note_at(int midi) const {
    if (!in_range(midi))
        throw std::out_of_range(
            "MIDI " + std::to_string(midi) + " is outside piano range [" +
            std::to_string(lowest_midi_) + ", " + std::to_string(highest_midi_) + "]");
    return Note(midi_to_name(midi));
}

Chord Piano::identify(const std::vector<int>& midi_numbers) const {
    if (midi_numbers.empty())
        throw std::invalid_argument("Cannot identify chord from empty MIDI list");

    // Convert MIDI numbers to note names (pitch classes, deduplicated).
    std::vector<std::string> names;
    std::vector<int> seen_semitones;

    // Sort by MIDI to get lowest note as root.
    auto sorted = midi_numbers;
    std::sort(sorted.begin(), sorted.end());

    for (int midi : sorted) {
        int semi = ((midi % 12) + 12) % 12;
        bool dup = false;
        for (int s : seen_semitones) {
            if (s == semi) { dup = true; break; }
        }
        if (!dup) {
            seen_semitones.push_back(semi);
            names.push_back(midi_to_name(midi));
        }
    }

    return Chord::identify(names);
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

std::string PianoKey::to_string() const {
    return "PianoKey(" + note + std::to_string(octave) +
           ", midi=" + std::to_string(midi) +
           ", " + (white ? "white" : "black") +
           ", pos=" + std::to_string(position) + ")";
}

std::string PianoVoicing::to_string() const {
    std::ostringstream os;
    os << "PianoVoicing(" << chord_name << ", ";
    switch (style) {
        case VoicingStyle::Close: os << "close"; break;
        case VoicingStyle::Open:  os << "open";  break;
        case VoicingStyle::Shell: os << "shell"; break;
    }
    os << ", inv=" << inversion << ", keys=[";
    for (std::size_t i = 0; i < keys.size(); ++i) {
        if (i > 0) os << ", ";
        os << keys[i].note << keys[i].octave;
    }
    os << "])";
    return os.str();
}

std::string Piano::to_string() const {
    return "Piano(" + std::to_string(num_keys_) + " keys, " +
           midi_to_name(lowest_midi_) + std::to_string(midi_to_octave(lowest_midi_)) +
           "–" +
           midi_to_name(highest_midi_) + std::to_string(midi_to_octave(highest_midi_)) +
           ")";
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

PianoKey Piano::make_key(int midi) const {
    return PianoKey{
        midi,
        midi_to_octave(midi),
        midi_to_name(midi),
        is_white(midi),
        midi - lowest_midi_ + 1
    };
}

} // namespace gingo
