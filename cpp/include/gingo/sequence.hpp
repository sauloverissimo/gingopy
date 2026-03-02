// Gingo — Music Theory Library
// Sequence: timeline of musical events.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <gingo/event.hpp>
#include <gingo/tempo.hpp>
#include <gingo/time_signature.hpp>
#include <vector>
#include <variant>
#include <string>

namespace gingo {

/// A musical event: NoteEvent, ChordEvent, or Rest.
using Event = std::variant<NoteEvent, ChordEvent, Rest>;

/// Represents a sequence of musical events with tempo and time signature.
///
/// A sequence is a timeline of notes, chords, and rests with rhythmic
/// information. It can be used for:
/// - Melodic lines
/// - Harmonic progressions
/// - Complete musical phrases
/// - Export to MIDI, MusicXML, etc. (via optional modules)
///
/// Examples:
///   Sequence seq(Tempo(120), TimeSignature(4, 4));
///   seq.add(NoteEvent(Note("C"), Duration("quarter")));
///   seq.add(NoteEvent(Note("E"), Duration("quarter")));
///   seq.add(Rest(Duration("half")));
///   seq.total_duration();  // 1.0 (whole note in beats)
class Sequence {
public:
    /// Construct an empty sequence.
    Sequence(const Tempo& tempo = Tempo(120),
             const TimeSignature& time_signature = TimeSignature(4, 4));

    /// Construct from a vector of events.
    Sequence(const std::vector<Event>& events,
             const Tempo& tempo = Tempo(120),
             const TimeSignature& time_signature = TimeSignature(4, 4));

    /// Tempo (BPM).
    const Tempo& tempo() const { return tempo_; }

    /// Set tempo.
    void set_tempo(const Tempo& tempo) { tempo_ = tempo; }

    /// Time signature.
    const TimeSignature& time_signature() const { return time_signature_; }

    /// Set time signature.
    void set_time_signature(const TimeSignature& ts) { time_signature_ = ts; }

    /// All events in the sequence.
    const std::vector<Event>& events() const { return events_; }

    /// Number of events.
    size_t size() const { return events_.size(); }

    /// Check if empty.
    bool empty() const { return events_.empty(); }

    /// Add an event to the sequence.
    void add(const Event& event);

    /// Add a note event.
    void add(const NoteEvent& event) { add(Event(event)); }

    /// Add a chord event.
    void add(const ChordEvent& event) { add(Event(event)); }

    /// Add a rest.
    void add(const Rest& rest) { add(Event(rest)); }

    /// Remove event at index.
    void remove(size_t index);

    /// Clear all events.
    void clear() { events_.clear(); }

    /// Total duration in beats (sum of all event durations).
    double total_duration() const;

    /// Total duration in seconds (based on tempo).
    double total_seconds() const;

    /// Number of complete bars (based on time signature).
    int bar_count() const;

    /// Get event at index.
    const Event& at(size_t index) const { return events_.at(index); }

    /// Operator[] for event access.
    const Event& operator[](size_t index) const { return events_[index]; }

    /// Transpose all note events by semitones.
    Sequence transpose(int semitones) const;

    // -- MIDI I/O ---------------------------------------------------------------

    /// Export this sequence to a Standard MIDI File (format 0, single track).
    /// @param path  File path to write to (should end in .mid).
    /// @param ppqn  Pulses per quarter note (default 480).
    void to_midi(const std::string& path, int ppqn = 480) const;

    /// Import a Sequence from a Standard MIDI File.
    /// Reads format 0 or format 1 (merges tracks).
    /// @param path  Path to a .mid file.
    static Sequence from_midi(const std::string& path);

    bool operator==(const Sequence& other) const;
    bool operator!=(const Sequence& other) const { return !(*this == other); }

    std::string to_string() const;

private:
    std::vector<Event> events_;
    Tempo tempo_;
    TimeSignature time_signature_;
};

}  // namespace gingo
