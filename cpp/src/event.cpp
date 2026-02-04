// Gingo — Music Theory Library
// Event implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/event.hpp>
#include <sstream>

namespace gingo {

// ---------------------------------------------------------------------------
// NoteEvent

NoteEvent::NoteEvent(const Note& note, const Duration& duration, int octave)
    : note_(note), octave_(octave), duration_(duration)
{
    if (octave < 0 || octave > 10) {
        throw std::invalid_argument("Octave must be between 0 and 10");
    }
}

std::string NoteEvent::to_string() const {
    std::ostringstream oss;
    oss << note_.name() << octave_ << " [" << duration_.name() << "]";
    return oss.str();
}

// ---------------------------------------------------------------------------
// ChordEvent

ChordEvent::ChordEvent(const Chord& chord, const Duration& duration, int octave)
    : chord_(chord), octave_(octave), duration_(duration)
{
    if (octave < 0 || octave > 10) {
        throw std::invalid_argument("Octave must be between 0 and 10");
    }
}

std::vector<NoteEvent> ChordEvent::note_events() const {
    std::vector<NoteEvent> result;
    const auto& notes = chord_.notes();

    for (const auto& note : notes) {
        result.emplace_back(note, duration_, octave_);
    }

    return result;
}

std::string ChordEvent::to_string() const {
    std::ostringstream oss;
    oss << chord_.name() << " [" << duration_.name() << "]";
    return oss.str();
}

// ---------------------------------------------------------------------------
// Rest

Rest::Rest(const Duration& duration) : duration_(duration) {}

std::string Rest::to_string() const {
    std::ostringstream oss;
    oss << "Rest [" << duration_.name() << "]";
    return oss.str();
}

}  // namespace gingo
