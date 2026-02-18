// Gingo — Music Theory Library
// Fretboard: string instrument mapping between theory and fret positions.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "note.hpp"
#include "chord.hpp"
#include "scale.hpp"

#include <string>
#include <utility>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// FretPosition — a single position on the fretboard
// ---------------------------------------------------------------------------

/// Represents one position on a fretboard (string + fret).
///
/// Maps between (string, fret) coordinates and pitch information.
/// String numbering follows standard tablature: 1 = highest pitch string.
///
/// Example:
///   FretPosition p = Fretboard::violao().position(1, 0);
///   p.string;   // 1
///   p.fret;     // 0 (open string)
///   p.midi;     // 64 (E4)
///   p.note;     // "E"
///   p.octave;   // 4
struct FretPosition {
    int         string;     ///< String number (1-based, 1 = highest pitch).
    int         fret;       ///< Fret number (0 = open string).
    int         midi;       ///< MIDI note number.
    std::string note;       ///< Pitch class name ("C", "Bb", etc.).
    int         octave;     ///< Octave number.

    std::string to_string() const;
};

// ---------------------------------------------------------------------------
// StringAction — how a string is played
// ---------------------------------------------------------------------------

/// How a single string is used in a fingering.
enum class StringAction {
    Open    = 0,  ///< Open string (no finger on fret).
    Fretted = 1,  ///< Finger pressed on a specific fret.
    Muted   = 2   ///< String is muted (not played).
};

// ---------------------------------------------------------------------------
// StringState — one string's contribution to a fingering
// ---------------------------------------------------------------------------

/// State of a single string within a chord fingering.
struct StringState {
    int          string;    ///< String number (1-based).
    StringAction action;    ///< Open, Fretted, or Muted.
    int          fret;      ///< Fret number (0 for open, -1 for muted).
    int          finger;    ///< Finger number (0=none, 1=index, 4=pinky).
};

// ---------------------------------------------------------------------------
// Fingering — a complete chord shape on the fretboard
// ---------------------------------------------------------------------------

/// A chord realized as specific string/fret positions.
///
/// Analogous to PianoVoicing: maps a chord to physical playing positions.
///
/// Example:
///   auto f = Fretboard::violao().fingering(Chord("CM"));
///   f.chord_name;  // "CM"
///   f.barre;       // 0 (no barré)
///   f.strings;     // [StringState(...), ...]
struct Fingering {
    std::vector<StringState> strings;       ///< State of each string (1st to last).
    std::string              chord_name;    ///< Original chord name ("Am7").
    int                      barre;         ///< Barré fret (0 = no barré).
    int                      base_fret;     ///< Lowest fret shown in diagram.
    int                      capo;          ///< Capo fret (0 = no capo).
    std::vector<int>         midi_notes;    ///< Resulting MIDI notes (low to high).

    std::string to_string() const;
};

// ---------------------------------------------------------------------------
// Tuning — string instrument tuning specification
// ---------------------------------------------------------------------------

/// Tuning specification for a string instrument.
///
/// Each entry in open_midi is the MIDI note for the open string.
/// Order: string 1 (highest pitch) to string N (lowest pitch).
struct Tuning {
    std::string      name;       ///< Tuning name ("standard", "drop D", etc.).
    std::vector<int> open_midi;  ///< MIDI notes for open strings (1st to last).
    int              num_frets;  ///< Number of frets on the instrument.
};

// ---------------------------------------------------------------------------
// Fretboard — the instrument
// ---------------------------------------------------------------------------

/// A fretted string instrument (guitar, cavaquinho, mandolin, etc.).
///
/// Maps between music theory objects (Note, Chord, Scale) and physical
/// fretboard positions identified by (string, fret) coordinates.
///
/// Use named constructors for common instruments:
///   Fretboard::violao()      // 6-string guitar, standard tuning
///   Fretboard::cavaquinho()  // 4-string cavaquinho
///   Fretboard::bandolim()    // 4-course mandolin
///
/// Or create custom tunings:
///   Fretboard("Drop D", {64,59,55,50,45,38}, 19)
///
/// Examples:
///   auto g = Fretboard::violao();
///   g.note_at(1, 0);          // Note("E")  — open 1st string
///   g.midi_at(6, 0);          // 40 (E2)
///   g.position(1, 5);         // FretPosition{string=1, fret=5, midi=69, ...}
///   g.fingering(Chord("CM")); // Fingering for C major
class Fretboard {
public:
    /// Construct from a tuning specification.
    explicit Fretboard(const Tuning& tuning);

    /// Construct from explicit parameters.
    Fretboard(const std::string& name, const std::vector<int>& open_midi,
              int num_frets = 19);

    // --- Named constructors (factories) ------------------------------------

    /// Standard 4-string cavaquinho (D5-B4-G4-D4, 17 frets).
    static Fretboard cavaquinho(int num_frets = 17);

    /// Standard 6-string guitar (E4-B3-G3-D3-A2-E2, 19 frets).
    static Fretboard violao(int num_frets = 19);

    /// Standard 4-course mandolin (E5-A4-D4-G3, 17 frets).
    static Fretboard bandolim(int num_frets = 17);

    // --- Info --------------------------------------------------------------

    /// Instrument name.
    const std::string& name() const { return tuning_.name; }

    /// Number of strings.
    int num_strings() const { return static_cast<int>(tuning_.open_midi.size()); }

    /// Number of frets.
    int num_frets() const { return tuning_.num_frets; }

    /// The tuning specification.
    const Tuning& tuning() const { return tuning_; }

    // --- Forward: theory → positions ---------------------------------------

    /// Get the position at a given string and fret.
    /// @throws std::out_of_range if string or fret is invalid.
    FretPosition position(int string, int fret) const;

    /// All positions where a note appears on the fretboard.
    std::vector<FretPosition> positions(const Note& note) const;

    /// The note at a given string and fret.
    /// @throws std::out_of_range if string or fret is invalid.
    Note note_at(int string, int fret) const;

    /// MIDI number at a given string and fret.
    /// @throws std::out_of_range if string or fret is invalid.
    int midi_at(int string, int fret) const;

    /// All scale positions across the entire fretboard.
    std::vector<FretPosition> scale_positions(const Scale& scale) const;

    /// Scale positions within a fret range.
    std::vector<FretPosition> scale_positions(const Scale& scale,
                                               int fret_lo, int fret_hi) const;

    // --- Fingerings --------------------------------------------------------

    /// Best fingering for a chord at a given position.
    /// @param position  Base fret position (0 = open position).
    Fingering fingering(const Chord& chord, int position = 0) const;

    /// Multiple fingerings ranked by playability.
    /// @param max_results  Maximum number of fingerings to return.
    std::vector<Fingering> fingerings(const Chord& chord,
                                       int max_results = 5) const;

    // --- Reverse: positions → theory ---------------------------------------

    /// Identify a chord from string/fret pairs.
    /// @param string_frets  Vector of (string, fret) pairs.
    /// @throws std::invalid_argument if no chord matches.
    Chord identify(const std::vector<std::pair<int, int>>& string_frets) const;

    // --- Capo --------------------------------------------------------------

    /// Return a new Fretboard with a capo at the given fret.
    Fretboard capo(int fret) const;

    // --- Display -----------------------------------------------------------

    std::string to_string() const;

private:
    Tuning tuning_;

    /// Validate string and fret bounds.
    void validate(int string, int fret) const;

    /// Compute MIDI from 0-based string index and fret.
    int compute_midi(int string_idx, int fret) const;

    /// Convert MIDI to pitch class name (sharp-based).
    static std::string midi_to_name(int midi);

    /// Convert MIDI to octave.
    static int midi_to_octave(int midi);

    /// Score a fingering for playability (lower = better).
    /// @param root_pc  Pitch class of chord root (0=C, ..., 11=B), or -1 to skip bass check.
    static double score_fingering(const Fingering& f, int root_pc = -1);

    /// Generate candidate fingerings within a fret window.
    std::vector<Fingering> generate_fingerings(
        const Chord& chord, int base_pos, int max_span = 4) const;
};

} // namespace gingo
