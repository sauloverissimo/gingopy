// Gingo — Music Theory Library
// PianoSVG: visual SVG rendering of piano keyboard states.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "piano.hpp"
#include "chord.hpp"
#include "scale.hpp"

#include <string>
#include <utility>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// PianoSVG — static SVG renderer for piano keyboard visualization
// ---------------------------------------------------------------------------

/// Generates SVG images of a piano keyboard with highlighted keys.
///
/// Renders an 88-key (or smaller) piano as a standard SVG string.
/// Keys can be highlighted to show notes, chords, scales, or voicings.
///
/// The SVG uses CSS classes for compact output:
///   - `.w` = white key (23×120), `.b` = black key (14×78)
///   - `.h` = highlighted, `.t` = text label
///   - Each key carries `id`, `data-midi`, `data-note`, `data-octave`
///
/// When `compact` is true, only ~2 octaves around the highlighted notes
/// are rendered, producing SVGs under 3 KB.
///
/// Examples:
///   Piano piano;
///   PianoSVG::note(piano, Note("C"), 4);
///   PianoSVG::chord(piano, Chord("Am7"), 4);
///   PianoSVG::note(piano, Note("C"), 4, true);  // compact
///   PianoSVG::write(svg, "piano.svg");
class PianoSVG {
public:
    /// Generate SVG for a single highlighted note.
    static std::string note(const Piano& piano, const Note& n,
                            int octave = 4, bool compact = false);

    /// Generate SVG for a chord voicing (close position).
    static std::string chord(const Piano& piano, const Chord& c,
                             int octave = 4,
                             VoicingStyle style = VoicingStyle::Close,
                             bool compact = false);

    /// Generate SVG for a scale in a given octave.
    static std::string scale(const Piano& piano, const Scale& s,
                             int octave = 4, bool compact = false);

    /// Generate SVG from a list of PianoKeys (most flexible).
    static std::string keys(const Piano& piano,
                            const std::vector<PianoKey>& highlighted,
                            const std::string& title = "",
                            bool compact = false);

    /// Generate SVG from a PianoVoicing.
    static std::string voicing(const Piano& piano,
                               const PianoVoicing& v,
                               bool compact = false);

    /// Generate SVG from raw MIDI numbers.
    static std::string midi(const Piano& piano,
                            const std::vector<int>& midi_numbers,
                            bool compact = false);

    /// Write an SVG string to a file.
    static void write(const std::string& svg, const std::string& path);

private:
    // Layout constants.
    static constexpr int WHITE_W = 23;
    static constexpr int WHITE_H = 120;
    static constexpr int BLACK_W = 14;
    static constexpr int BLACK_H = 78;
    static constexpr int TITLE_H = 28;

    /// Core renderer: takes a piano and a set of MIDI numbers to highlight.
    static std::string render(const Piano& piano,
                              const std::vector<int>& highlight_midi,
                              const std::string& title,
                              bool compact);

    /// Check if a semitone class (0-11) is a black key.
    static bool is_black(int midi);

    /// Compute the x-offset for a white key by its index among white keys.
    static int white_key_x(int white_index);

    /// Compute the x-offset for a black key given the MIDI number
    /// and the x-position of its preceding white key.
    static int black_key_x(int midi, int prev_white_x);

    /// Get display label for a MIDI number (e.g., "C4", "F#5").
    static std::string midi_label(int midi);

    /// Compute the compact MIDI range around highlighted notes.
    static std::pair<int, int> compact_range(
        const Piano& piano, const std::vector<int>& highlight_midi);

    /// Emit the CSS <style> block string.
    static std::string css_style();
};

} // namespace gingo
