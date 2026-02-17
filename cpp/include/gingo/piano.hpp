// Gingo — Music Theory Library
// Piano: keyboard instrument mapping between theory and physical keys.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "note.hpp"
#include "chord.hpp"
#include "scale.hpp"

#include <string>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// PianoKey — a single key on the piano keyboard
// ---------------------------------------------------------------------------

/// Represents one physical key on a piano keyboard.
///
/// Maps between MIDI note numbers, pitch classes, and keyboard positions.
/// MIDI numbering: A0 = 21, C4 (middle C) = 60, C8 = 108.
///
/// Example:
///   PianoKey k = Piano().key(Note("C"), 4);
///   k.midi;      // 60
///   k.octave;    // 4
///   k.note;      // "C"
///   k.white;     // true
///   k.position;  // 40 (on 88-key piano)
struct PianoKey {
    int         midi;       ///< MIDI note number (21–108 for 88 keys).
    int         octave;     ///< Octave number (0–8 for standard piano).
    std::string note;       ///< Pitch class name ("C", "Bb", etc.).
    bool        white;      ///< True for white keys, false for black.
    int         position;   ///< 1-based position on the keyboard (1 = lowest).

    std::string to_string() const;
};

// ---------------------------------------------------------------------------
// VoicingStyle — how a chord is distributed across the keyboard
// ---------------------------------------------------------------------------

/// Voicing style for chord realization on the piano.
enum class VoicingStyle {
    Close = 0,  ///< All notes within one octave (close position).
    Open  = 1,  ///< Root drops one octave below (open position).
    Shell = 2   ///< Root + 3rd + 7th only (jazz shell voicing).
};

// ---------------------------------------------------------------------------
// PianoVoicing — a specific way to play a chord
// ---------------------------------------------------------------------------

/// A chord realized as specific piano keys.
///
/// Example:
///   auto v = Piano().voicing(Chord("Am7"));
///   v.chord_name;  // "Am7"
///   v.style;       // VoicingStyle::Close
///   v.inversion;   // 0
///   v.keys;        // [PianoKey(57), PianoKey(60), PianoKey(64), PianoKey(67)]
struct PianoVoicing {
    std::vector<PianoKey> keys;     ///< Keys to press, from lowest to highest.
    VoicingStyle          style;    ///< How the notes are distributed.
    std::string           chord_name; ///< Original chord name ("Am7").
    int                   inversion;  ///< 0 = root position, 1 = 1st, etc.

    std::string to_string() const;
};

// ---------------------------------------------------------------------------
// Piano — the instrument
// ---------------------------------------------------------------------------

/// A piano keyboard instrument.
///
/// Maps between music theory objects (Note, Chord, Scale) and physical
/// piano keys identified by MIDI numbers and keyboard positions.
///
/// Supports standard 88-key pianos and smaller keyboards (61, 76, etc.).
///
/// Examples:
///   Piano piano;                      // 88 keys (default)
///   Piano piano(61);                  // 61-key controller
///
///   // Forward: theory → keys
///   auto k = piano.key(Note("C"), 4); // PianoKey{midi=60, ...}
///   auto v = piano.voicing(Chord("Am7"));
///
///   // Reverse: keys → theory
///   auto n = piano.note_at(60);       // Note("C")
///   auto c = piano.identify({60, 64, 67}); // Chord("CM")
class Piano {
public:
    /// Construct a piano with a given number of keys.
    /// @param num_keys  Number of keys (default 88 = standard piano).
    ///                  Common values: 25, 37, 49, 61, 76, 88.
    explicit Piano(int num_keys = 88);

    // --- Info ---------------------------------------------------------------

    /// Number of keys on this piano.
    int num_keys() const { return num_keys_; }

    /// The lowest key on this piano.
    PianoKey lowest() const;

    /// The highest key on this piano.
    PianoKey highest() const;

    /// Whether a MIDI note number is within this piano's range.
    bool in_range(int midi) const;

    // --- Forward: theory → keys ---------------------------------------------

    /// Map a note and octave to a specific piano key.
    /// @throws std::out_of_range if the note is outside the piano's range.
    PianoKey key(const Note& note, int octave = 4) const;

    /// All keys for a given pitch class across all octaves on this piano.
    std::vector<PianoKey> keys(const Note& note) const;

    /// Realize a chord as a voicing on the piano.
    /// @param chord    The chord to voice.
    /// @param octave   Base octave for the root (default 4).
    /// @param style    Voicing style (default Close).
    PianoVoicing voicing(const Chord& chord, int octave = 4,
                         VoicingStyle style = VoicingStyle::Close) const;

    /// All available voicings for a chord (Close, Open, Shell).
    std::vector<PianoVoicing> voicings(const Chord& chord,
                                       int octave = 4) const;

    /// Map a scale to piano keys in a specific octave.
    std::vector<PianoKey> scale_keys(const Scale& scale,
                                     int octave = 4) const;

    // --- Reverse: keys → theory ---------------------------------------------

    /// Identify the note at a given MIDI number.
    /// @throws std::out_of_range if midi is outside the piano's range.
    Note note_at(int midi) const;

    /// Identify a chord from a set of MIDI note numbers.
    /// Notes are reduced to pitch classes and matched against chord formulas.
    /// @throws std::invalid_argument if no chord matches.
    Chord identify(const std::vector<int>& midi_numbers) const;

    // --- Display ------------------------------------------------------------

    std::string to_string() const;

private:
    int num_keys_;
    int lowest_midi_;
    int highest_midi_;

    /// Build a PianoKey from a MIDI number.
    PianoKey make_key(int midi) const;

    /// Whether a MIDI note falls on a white key.
    static bool is_white(int midi);

    /// Convert a Note + octave to a MIDI number.
    static int to_midi(const Note& note, int octave);

    /// Convert a MIDI number to a pitch class name (sharp-based).
    static std::string midi_to_name(int midi);

    /// Convert a MIDI number to an octave.
    static int midi_to_octave(int midi);
};

} // namespace gingo
