// Gingo — Music Theory Library
// Event: musical events with pitch and duration.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <gingo/note.hpp>
#include <gingo/chord.hpp>
#include <gingo/duration.hpp>
#include <string>
#include <vector>
#include <optional>

namespace gingo {

/// Represents a musical event: a note or chord with a duration and position.
///
/// Events are the building blocks of musical sequences. They combine pitch
/// (Note or Chord) with rhythmic information (Duration).
///
/// Examples:
///   NoteEvent(Note("C4"), Duration("quarter"));
///   ChordEvent(Chord("Cmaj7"), Duration("whole"));
class NoteEvent {
public:
    /// Construct from note and duration.
    NoteEvent(const Note& note, const Duration& duration, int octave = 4);

    /// The note (pitch class).
    const Note& note() const { return note_; }

    /// Octave number (default 4).
    int octave() const { return octave_; }

    /// MIDI note number (C4 = 60).
    int midi_number() const { return 12 * (octave_ + 1) + note_.semitone(); }

    /// Duration of this event.
    const Duration& duration() const { return duration_; }

    /// Frequency in Hz.
    /// @param tuning  Reference frequency for A4 in Hz (default 440.0).
    double frequency(double tuning = 440.0) const { return note_.frequency(octave_, tuning); }

    bool operator==(const NoteEvent& other) const {
        return note_ == other.note_ && octave_ == other.octave_ && duration_ == other.duration_;
    }
    bool operator!=(const NoteEvent& other) const { return !(*this == other); }

    std::string to_string() const;

private:
    Note note_;
    int octave_;
    Duration duration_;
};

// ---------------------------------------------------------------------------

/// Represents a chord event: multiple simultaneous notes with duration.
class ChordEvent {
public:
    /// Construct from chord and duration.
    ChordEvent(const Chord& chord, const Duration& duration, int octave = 4);

    /// The chord.
    const Chord& chord() const { return chord_; }

    /// Base octave for the chord root.
    int octave() const { return octave_; }

    /// Duration of this event.
    const Duration& duration() const { return duration_; }

    /// All notes in the chord (with octave information).
    std::vector<NoteEvent> note_events() const;

    bool operator==(const ChordEvent& other) const {
        return chord_ == other.chord_ && octave_ == other.octave_ && duration_ == other.duration_;
    }
    bool operator!=(const ChordEvent& other) const { return !(*this == other); }

    std::string to_string() const;

private:
    Chord chord_;
    int octave_;
    Duration duration_;
};

// ---------------------------------------------------------------------------

/// Represents a rest (silence) with duration.
class Rest {
public:
    /// Construct a rest with a duration.
    explicit Rest(const Duration& duration);

    /// Duration of the rest.
    const Duration& duration() const { return duration_; }

    bool operator==(const Rest& other) const { return duration_ == other.duration_; }
    bool operator!=(const Rest& other) const { return !(*this == other); }

    std::string to_string() const;

private:
    Duration duration_;
};

}  // namespace gingo
