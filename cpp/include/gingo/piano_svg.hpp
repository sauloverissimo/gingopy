// Gingo — Music Theory Library
// PianoSVG: visual SVG rendering of piano keyboard states.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "piano.hpp"
#include "chord.hpp"
#include "field.hpp"
#include "progression.hpp"
#include "scale.hpp"

#include <string>
#include <utility>
#include <vector>

namespace gingo {

// Forward declaration — Tree is defined in tree.hpp.
class Tree;

/// Layout direction for composite piano visualizations.
enum class Layout {
    Vertical   = 0,  ///< Pianos stacked top-to-bottom.
    Horizontal = 1,  ///< Continuous strip with degree bands above the keys.
    Grid       = 2   ///< Mini pianos in a compact grid.
};

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
/// Composite methods (`field`, `progression`) render multiple pianos in
/// one SVG with shared CSS. Layout controls arrangement:
///   - Vertical: stacked pianos (default)
///   - Horizontal: degree bands above a shared piano
///   - Grid: mini pianos in rows
///
/// Examples:
///   Piano piano;
///   PianoSVG::note(piano, Note("C"), 4);
///   PianoSVG::chord(piano, Chord("Am7"), 4);
///   PianoSVG::field(piano, Field("C", "major"));
///   PianoSVG::write(svg, "output.svg");
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

    /// Generate composite SVG showing all chords in a harmonic field.
    static std::string field(const Piano& piano, const Field& f,
                             int octave = 4,
                             Layout layout = Layout::Vertical,
                             bool sevenths = false);

    /// Generate composite SVG showing chords in a progression.
    static std::string progression(const Piano& piano,
                                   const Field& f,
                                   const std::vector<std::string>& branches,
                                   int octave = 4,
                                   Layout layout = Layout::Vertical);

    /// Write an SVG string to a file.
    static void write(const std::string& svg, const std::string& path);

private:
    // Layout constants.
    static constexpr int WHITE_W = 23;
    static constexpr int WHITE_H = 120;
    static constexpr int BLACK_W = 14;
    static constexpr int BLACK_H = 78;
    static constexpr int TITLE_H = 28;

    // Composite layout constants.
    static constexpr int GAP_V    = 6;    ///< Vertical gap between stacked pianos.
    static constexpr int GAP_H    = 12;   ///< Horizontal gap in grid.
    static constexpr int PAD      = 10;   ///< Outer padding.
    static constexpr int BAND_H   = 20;   ///< Height of each degree band (Horizontal).
    static constexpr int BAND_GAP = 2;    ///< Gap between bands.
    static constexpr int GRID_COLS = 4;   ///< Columns in Grid layout.

    // Degree colors for composite views.
    static const char* degree_color(int index);

    /// Key layout info for rendering.
    struct KeyInfo {
        int midi;
        bool black;
        int x;
    };

    /// Build key layout for a MIDI range.
    static std::vector<KeyInfo> build_keys(int lo, int hi, int& white_count);

    /// Core renderer: takes a piano and a set of MIDI numbers to highlight.
    static std::string render(const Piano& piano,
                              const std::vector<int>& highlight_midi,
                              const std::string& title,
                              bool compact);

    /// Render a piano as a <g> fragment (no <svg>, no <style>).
    /// out_width/out_height report dimensions.
    static std::string render_group(int lo, int hi,
                                    const std::vector<int>& highlight_midi,
                                    const std::string& title,
                                    const std::string& color,
                                    int group_id,
                                    int& out_width, int& out_height);

    /// Compose multiple pianos (Vertical layout).
    static std::string render_vertical(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis,
        const std::vector<std::string>& titles,
        const std::vector<std::string>& colors);

    /// Compose as continuous strip with bands (Horizontal layout).
    static std::string render_horizontal(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis,
        const std::vector<std::string>& titles,
        const std::vector<std::string>& colors);

    /// Compose as compact grid (Grid layout).
    static std::string render_grid(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis,
        const std::vector<std::string>& titles,
        const std::vector<std::string>& colors);

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

    /// Compute unified compact range across multiple sets of MIDI numbers.
    static std::pair<int, int> unified_range(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis);

    /// Emit the CSS <style> block string.
    static std::string css_style();

    /// Emit extended CSS for composite views (degree colors).
    static std::string css_composite(const std::vector<std::string>& colors);
};

} // namespace gingo
