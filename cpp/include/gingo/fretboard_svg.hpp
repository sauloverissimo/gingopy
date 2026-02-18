// Gingo — Music Theory Library
// FretboardSVG: visual SVG rendering of fretboard diagrams.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "fretboard.hpp"
#include "field.hpp"
#include "piano_svg.hpp"   // Layout enum

#include <string>
#include <vector>

namespace gingo {

/// Neck orientation for fretboard SVG rendering.
enum class Orientation {
    Horizontal = 0,  ///< Strings horizontal, frets vertical (fretboard view).
    Vertical   = 1   ///< Strings vertical, frets horizontal (chord box view).
};

/// Handedness for fretboard SVG rendering.
///
/// Handedness only mirrors the axis perpendicular to the strings:
///
///   **Horizontal orientation** (strings = Y, frets = X):
///     - Y axis (strings): always E2 (grave) on top, E4 (acute) on bottom.
///     - X axis (frets): RightHanded = nut left, frets grow right.
///                        LeftHanded  = nut right, frets grow left.
///
///   **Vertical orientation** (strings = X, frets = Y):
///     - X axis (strings): RightHanded = E4 right, E2 left.
///                          LeftHanded  = E4 left, E2 right.
///     - Y axis (frets): always nut on top, frets grow down.
enum class Handedness {
    RightHanded = 0,  ///< Standard right-handed layout.
    LeftHanded  = 1   ///< Mirrored left-handed layout.
};

// ---------------------------------------------------------------------------
// FretboardSVG — static SVG renderer for fretboard visualization
// ---------------------------------------------------------------------------

/// Generates SVG images of fretboard diagrams with highlighted positions.
///
/// Two rendering modes:
///   - **Chord diagram**: vertical strings, horizontal frets, dots for
///     fingers, X for muted, O for open. Classic "chord box" format.
///   - **Scale/fretboard**: horizontal strings, vertical frets, dots
///     for note positions across a fret range.
///
/// CSS classes:
///   `.str` string line, `.frt` fret line, `.nut` nut,
///   `.dot` finger dot, `.open` open marker, `.mute` muted marker,
///   `.lbl` text label, `.title` title text
///
/// Examples:
///   auto g = Fretboard::violao();
///   FretboardSVG::chord(g, Chord("CM"));
///   FretboardSVG::scale(g, Scale("C", ScaleType::Major), 0, 12);
///   FretboardSVG::field(g, Field("C", "major"));
class FretboardSVG {
public:
    /// Generate SVG for a chord fingering diagram.
    static std::string chord(const Fretboard& fb, const Chord& c,
                              int position = 0,
                              Orientation orientation = Orientation::Vertical,
                              Handedness handedness = Handedness::RightHanded);

    /// Generate SVG for a specific fingering.
    static std::string fingering(const Fretboard& fb, const Fingering& f,
                                  Orientation orientation = Orientation::Vertical,
                                  Handedness handedness = Handedness::RightHanded);

    /// Generate SVG for scale positions on the fretboard.
    static std::string scale(const Fretboard& fb, const Scale& s,
                              int fret_lo = 0, int fret_hi = 12,
                              Orientation orientation = Orientation::Horizontal,
                              Handedness handedness = Handedness::RightHanded);

    /// Generate SVG for arbitrary positions.
    static std::string positions(const Fretboard& fb,
                                  const std::vector<FretPosition>& highlighted,
                                  const std::string& title = "",
                                  Orientation orientation = Orientation::Horizontal,
                                  Handedness handedness = Handedness::RightHanded);

    /// Generate SVG for all positions of a note on the fretboard.
    static std::string note(const Fretboard& fb, const Note& n,
                             Orientation orientation = Orientation::Horizontal,
                             Handedness handedness = Handedness::RightHanded);

    /// Generate composite SVG for a harmonic field.
    static std::string field(const Fretboard& fb, const Field& f,
                              Layout layout = Layout::Vertical,
                              Orientation orientation = Orientation::Vertical,
                              Handedness handedness = Handedness::RightHanded);

    /// Generate composite SVG for a chord progression.
    static std::string progression(const Fretboard& fb, const Field& f,
                                    const std::vector<std::string>& branches,
                                    Layout layout = Layout::Vertical,
                                    Orientation orientation = Orientation::Vertical,
                                    Handedness handedness = Handedness::RightHanded);

    /// Generate SVG showing the full fretboard (all frets).
    static std::string full(const Fretboard& fb,
                             Orientation orientation = Orientation::Horizontal,
                             Handedness handedness = Handedness::RightHanded);

    /// Write an SVG string to a file.
    static void write(const std::string& svg, const std::string& path);

private:
    // Chord diagram constants (vertical orientation).
    static constexpr int CD_STR_SPACING = 20;   ///< Horizontal spacing between strings.
    static constexpr int CD_FRET_SPACING = 28;  ///< Vertical spacing between frets.
    static constexpr int CD_DOT_R = 7;          ///< Finger dot radius.
    static constexpr int CD_NUT_W = 4;          ///< Nut line width.
    static constexpr int CD_PAD = 16;           ///< Outer padding.
    static constexpr int CD_MARKER_H = 16;      ///< Height for X/O markers above nut.
    static constexpr int CD_TUNING_H = 14;      ///< Height for tuning labels above markers.
    static constexpr int CD_TITLE_H = 22;       ///< Title area height.
    static constexpr int CD_FRETS_SHOWN = 5;    ///< Number of frets in chord diagram.
    static constexpr int CD_FRET_LABEL_W = 20;  ///< Width for fret number label.

    // Scale/fretboard constants (horizontal orientation).
    static constexpr int FB_STR_SPACING = 28;   ///< Vertical spacing between strings.
    static constexpr int FB_FRET_SPACING = 50;  ///< Horizontal spacing between frets.
    static constexpr int FB_DOT_R = 9;          ///< Note dot radius.
    static constexpr int FB_NUT_W = 5;          ///< Nut line width.
    static constexpr int FB_PAD = 16;           ///< Outer padding.
    static constexpr int FB_MARKER_W = 16;      ///< Width for O markers before nut.
    static constexpr int FB_TITLE_H = 28;       ///< Title area height.
    static constexpr int FB_STR_LABEL_W = 32;   ///< Width for string name labels.

    // Composite layout constants.
    static constexpr int GAP_V = 8;
    static constexpr int GAP_H = 12;
    static constexpr int GRID_COLS = 4;
    static constexpr int COMP_PAD = 10;

    /// CSS for chord diagrams.
    static std::string css_chord();

    /// CSS for fretboard/scale diagrams.
    static std::string css_fretboard();

    /// Render a chord diagram (classic chord box format).
    static std::string render_diagram(const Fretboard& fb,
                                       const Fingering& f,
                                       const std::string& title,
                                       Handedness handedness = Handedness::RightHanded);

    /// Render a horizontal fretboard with highlighted positions.
    /// @param root_pc  Pitch class of the root note (-1 to skip root highlight).
    static std::string render_fretboard(const Fretboard& fb,
                                         const std::vector<FretPosition>& positions,
                                         int fret_lo, int fret_hi,
                                         const std::string& title,
                                         int root_pc = -1,
                                         const std::string& dot_color = "",
                                         Handedness handedness = Handedness::RightHanded);

    /// Render positions in vertical orientation (chord box extended).
    static std::string render_diagram_positions(
        const Fretboard& fb,
        const std::vector<FretPosition>& positions,
        int fret_lo, int fret_hi,
        const std::string& title,
        int root_pc = -1,
        const std::string& dot_color = "",
        Handedness handedness = Handedness::RightHanded);

    /// Convert a Fingering to FretPositions for fretboard rendering.
    static std::vector<FretPosition> fingering_to_positions(
        const Fretboard& fb, const Fingering& f,
        int& fret_lo, int& fret_hi);

    /// Degree color for composite views.
    static const char* degree_color(int index);
};

} // namespace gingo
