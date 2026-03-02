// Gingo — Music Theory Library
// NoteContext: per-note harmonic context (degree, interval, function).
//
// Returned by Field::noteContext(). Contains the harmonic context
// of a single note within a field: its scale degree, interval from the
// tonic, and harmonic function (Tonic / Subdominant / Dominant).
//
// MIDI 2.0 use: encode degree and function as per-note RCC values
// via MIDI2::perNoteController().
//
// SPDX-License-Identifier: MIT

#pragma once

#include "chord.hpp"
#include "harmonic_function.hpp"
#include "interval.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace gingo {

// ---------------------------------------------------------------------------
// NoteContext — per-note harmonic context within a Field
// ---------------------------------------------------------------------------

/// Per-note harmonic context within a harmonic field.
///
/// Examples:
///   Field field("C", ScaleType::Major);
///   NoteContext ctx = field.noteContext(Note("E"));
///   // ctx.degree   = 3          (third degree of C major)
///   // ctx.inScale  = true
///   // ctx.function = HarmonicFunction::Tonic
///   // ctx.interval = Interval(4)  (major third, 4 semitones)
struct NoteContext {
    Note              note;      ///< The note
    uint8_t           degree;    ///< Scale degree 1–7 (0 = not in scale)
    Interval          interval;  ///< Ascending interval from field tonic to this note
    HarmonicFunction  function;  ///< Tonic, Subdominant, or Dominant
    bool              inScale;   ///< true when degree > 0
    
    // Default constructor - must initialize all members
    NoteContext() 
        : note("C")
        , degree(0)
        , interval(0)
        , function(HarmonicFunction::Tonic)
        , inScale(false) 
    {}
};

} // namespace gingo
