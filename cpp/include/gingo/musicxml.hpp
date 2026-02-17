// Gingo — Music Theory Library
// MusicXML: serialize music theory objects to MusicXML format.
//
// Generates valid MusicXML 4.0 partwise documents that can be opened
// in MuseScore, Finale, Sibelius, and other notation software.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "note.hpp"
#include "chord.hpp"
#include "scale.hpp"
#include "field.hpp"
#include "sequence.hpp"

#include <string>

namespace gingo {

/// Serialize Gingo objects to MusicXML 4.0 partwise format.
///
/// Each method returns a complete, self-contained MusicXML document
/// as a string.  Use write() to save to a file.
///
/// Examples:
///   std::string xml = MusicXML::note(Note("C"), 4);
///   MusicXML::write(xml, "note.musicxml");
///
///   std::string xml = MusicXML::chord(Chord("Am7"));
///   std::string xml = MusicXML::scale(Scale("C", ScaleType::Major));
///   std::string xml = MusicXML::field(Field("C", ScaleType::Major));
///   std::string xml = MusicXML::sequence(seq);
class MusicXML {
public:
    /// A single note as a one-measure score.
    /// @param n       The note to serialize.
    /// @param octave  Octave number (default 4).
    /// @param type    MusicXML note type: "whole", "half", "quarter", etc.
    static std::string note(const Note& n, int octave = 4,
                            const std::string& type = "quarter");

    /// A chord as a one-measure score (simultaneous notes).
    /// @param c       The chord to serialize.
    /// @param octave  Base octave for the root (default 4).
    /// @param type    MusicXML note type (default "whole").
    static std::string chord(const Chord& c, int octave = 4,
                             const std::string& type = "whole");

    /// A scale as a sequence of notes in one or more measures.
    /// @param s       The scale to serialize.
    /// @param octave  Base octave (default 4).
    /// @param type    MusicXML note type per note (default "quarter").
    static std::string scale(const Scale& s, int octave = 4,
                             const std::string& type = "quarter");

    /// A harmonic field as one chord per measure.
    /// @param f       The field to serialize (triads).
    /// @param octave  Base octave (default 4).
    /// @param type    MusicXML note type (default "whole").
    static std::string field(const Field& f, int octave = 4,
                             const std::string& type = "whole");

    /// A complete Sequence (notes, chords, rests with durations).
    /// Uses the Sequence's own tempo and time signature.
    static std::string sequence(const Sequence& seq);

    /// Write a MusicXML string to a file.
    /// @param xml     The MusicXML content.
    /// @param path    File path to write to.
    static void write(const std::string& xml, const std::string& path);

private:
    // XML document structure.
    static std::string header(const std::string& title = "Gingo");
    static std::string footer();

    // Note element builder.
    static std::string note_element(const Note& n, int octave,
                                    const std::string& type, int divisions,
                                    bool is_chord = false, bool is_rest = false);

    // Duration name → MusicXML divisions.
    static int type_to_divisions(const std::string& type);

    // Duration object → MusicXML type name.
    static std::string duration_to_type(const Duration& d);

    // Pitch helpers.
    static std::string pitch_step(const Note& n);
    static int pitch_alter(const Note& n);
};

} // namespace gingo
