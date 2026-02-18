// Gingo — Music Theory Library
// FretboardSVG implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/fretboard_svg.hpp>

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gingo {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

static const char* CHROMATIC[] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

static const char* ROMAN[] = {
    "I","II","III","IV","V","VI","VII","VIII","IX","X"
};

static const char* DEGREE_COLORS[] = {
    "#4A6FA5",  // I   — steel blue
    "#D4793A",  // II  — burnt sienna
    "#5A8F5A",  // III — sage
    "#8B5A9E",  // IV  — plum
    "#C4453C",  // V   — vermillion
    "#C9963A",  // VI  — amber
    "#3A9E8F",  // VII — viridian
    "#7A6B5D",  // VIII+
    "#6B7A5D",
    "#5D6B7A"
};

const char* FretboardSVG::degree_color(int index) {
    if (index < 0) index = 0;
    if (index >= 10) index = 7;
    return DEGREE_COLORS[index];
}

static std::string midi_label(int midi) {
    int pc = ((midi % 12) + 12) % 12;
    int octave = midi / 12 - 1;
    return std::string(CHROMATIC[pc]) + std::to_string(octave);
}

// ---------------------------------------------------------------------------
// CSS
// ---------------------------------------------------------------------------

std::string FretboardSVG::css_chord() {
    return
        "<style>\n"
        ".str{stroke:#555;stroke-width:1}\n"
        ".frt{stroke:#555;stroke-width:1}\n"
        ".nut{stroke:#222;stroke-width:" + std::to_string(CD_NUT_W) + "}\n"
        ".dot{fill:#4A90D9}\n"
        ".dot.root{fill:#2E6AB0}\n"
        ".open{fill:none;stroke:#555;stroke-width:1.5}\n"
        ".mute{stroke:#555;stroke-width:2;stroke-linecap:round}\n"
        ".lbl{font-family:Arial,Helvetica,sans-serif;font-size:9px;"
        "fill:#fff;text-anchor:middle;pointer-events:none}\n"
        ".flbl{font-family:Arial,Helvetica,sans-serif;font-size:10px;"
        "fill:#666}\n"
        ".title{font-family:Arial,Helvetica,sans-serif;font-size:14px;"
        "font-weight:bold;fill:#333;text-anchor:middle}\n"
        "</style>\n";
}

std::string FretboardSVG::css_fretboard() {
    return
        "<style>\n"
        ".str{stroke:#999;stroke-width:1.2}\n"
        ".frt{stroke:#DDD;stroke-width:1}\n"
        ".nut{stroke:#333;stroke-width:" + std::to_string(FB_NUT_W) + "}\n"
        ".dot{fill:#5A9FD4;opacity:0.9}\n"
        ".rdot{fill:#2B6CA3}\n"
        ".open{fill:none;stroke:#555;stroke-width:1.5}\n"
        ".lbl{font-family:Arial,Helvetica,sans-serif;font-size:10px;"
        "fill:#fff;text-anchor:middle;pointer-events:none;font-weight:bold}\n"
        ".slbl{font-family:Arial,Helvetica,sans-serif;font-size:11px;"
        "fill:#555;text-anchor:end;font-weight:bold}\n"
        ".flbl{font-family:Arial,Helvetica,sans-serif;font-size:10px;"
        "fill:#AAA;text-anchor:middle}\n"
        ".inlay{fill:#E0E0E0}\n"
        ".title{font-family:Arial,Helvetica,sans-serif;font-size:15px;"
        "font-weight:bold;fill:#333;text-anchor:middle}\n"
        "</style>\n";
}

// ---------------------------------------------------------------------------
// Conversion helper
// ---------------------------------------------------------------------------

std::vector<FretPosition> FretboardSVG::fingering_to_positions(
    const Fretboard& fb, const Fingering& f,
    int& fret_lo, int& fret_hi) {

    std::vector<FretPosition> positions;
    int lo = 99, hi = 0;

    for (const auto& ss : f.strings) {
        if (ss.action == StringAction::Muted) continue;
        int fret = (ss.action == StringAction::Open) ? 0 : ss.fret;
        positions.push_back(fb.position(ss.string, fret));
        if (fret < lo) lo = fret;
        if (fret > hi) hi = fret;
    }

    if (positions.empty()) {
        fret_lo = 0;
        fret_hi = 5;
    } else {
        fret_lo = lo;
        fret_hi = std::max(hi + 1, lo + 4);
    }
    return positions;
}

// ---------------------------------------------------------------------------
// Chord diagram (vertical orientation)
// ---------------------------------------------------------------------------

std::string FretboardSVG::render_diagram(const Fretboard& fb,
                                           const Fingering& f,
                                           const std::string& title,
                                           Handedness handedness) {
    int n_str = fb.num_strings();
    int n_frets = CD_FRETS_SHOWN;
    bool lefty = (handedness == Handedness::LeftHanded);

    // Determine fret range to show.
    // If base_fret <= 1, show from fret 0 (nut visible).
    int start_fret = f.base_fret;
    bool show_nut = (start_fret <= 1);
    if (show_nut) start_fret = 1;

    // Diagram dimensions.
    int grid_w = (n_str - 1) * CD_STR_SPACING;
    int grid_h = n_frets * CD_FRET_SPACING;
    int fret_label_w = show_nut ? 0 : CD_FRET_LABEL_W;
    int total_w = CD_PAD * 2 + grid_w + fret_label_w;
    int title_h = title.empty() ? 0 : CD_TITLE_H;
    int total_h = CD_PAD * 2 + CD_TUNING_H + CD_MARKER_H + grid_h + title_h;

    int grid_x = CD_PAD + fret_label_w;
    int grid_y = CD_PAD + title_h + CD_TUNING_H + CD_MARKER_H;

    // Helper: map logical string index (0=1st/highest) to visual x position.
    auto str_x = [&](int idx) -> int {
        int visual = lefty ? idx : (n_str - 1 - idx);
        return grid_x + visual * CD_STR_SPACING;
    };

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";
    svg << css_chord();

    // Background.
    svg << "<rect width=\"" << total_w << "\" height=\"" << total_h
        << "\" fill=\"#F5F5F5\" rx=\"4\"/>\n";

    // Title.
    if (!title.empty()) {
        svg << "<text x=\"" << total_w / 2 << "\" y=\"" << CD_PAD + 14
            << "\" class=\"title\">" << title << "</text>\n";
    }

    // Nut or fret number.
    if (show_nut) {
        int nut_y = grid_y;
        svg << "<line x1=\"" << grid_x - 1 << "\" y1=\"" << nut_y
            << "\" x2=\"" << grid_x + grid_w + 1 << "\" y2=\"" << nut_y
            << "\" class=\"nut\"/>\n";
    } else {
        svg << "<text x=\"" << grid_x - 6 << "\" y=\"" << grid_y + CD_FRET_SPACING / 2 + 4
            << "\" class=\"flbl\" text-anchor=\"end\">" << start_fret << "</text>\n";
    }

    // Fret lines (horizontal).
    for (int i = 0; i <= n_frets; ++i) {
        int y = grid_y + i * CD_FRET_SPACING;
        svg << "<line x1=\"" << grid_x << "\" y1=\"" << y
            << "\" x2=\"" << grid_x + grid_w << "\" y2=\"" << y
            << "\" class=\"frt\"/>\n";
    }

    // String lines (vertical).
    for (int i = 0; i < n_str; ++i) {
        int x = grid_x + i * CD_STR_SPACING;
        svg << "<line x1=\"" << x << "\" y1=\"" << grid_y
            << "\" x2=\"" << x << "\" y2=\"" << grid_y + grid_h
            << "\" class=\"str\"/>\n";
    }

    // Barré indicator — rendered as a blue bar connecting the strings.
    // The barré spans all consecutive fretted strings (higher frets
    // are pressed on top of the barré finger, so they don't break it).
    // Track which strings are at the barré fret to suppress individual dots.
    std::vector<bool> in_barre(f.strings.size(), false);
    if (f.barre > 0) {
        int fret_offset = f.barre - start_fret;
        if (fret_offset >= 0 && fret_offset < n_frets) {
            // Find the longest consecutive run of fretted strings.
            int best_start = -1, best_len = 0;
            int run_start = -1, run_len = 0;
            for (std::size_t i = 0; i < f.strings.size(); ++i) {
                if (f.strings[i].action == StringAction::Fretted) {
                    if (run_len == 0) run_start = static_cast<int>(i);
                    run_len++;
                    if (run_len > best_len) {
                        best_start = run_start;
                        best_len = run_len;
                    }
                } else {
                    run_len = 0;
                }
            }
            if (best_len >= 2) {
                // Mark strings at the barré fret (they get the bar, not dots).
                for (int k = best_start; k < best_start + best_len; ++k) {
                    if (f.strings[k].fret == f.barre)
                        in_barre[k] = true;
                }

                int end_idx = best_start + best_len - 1;
                int x1 = str_x(end_idx);
                int x2 = str_x(best_start);
                if (x1 > x2) std::swap(x1, x2);
                int by = grid_y + fret_offset * CD_FRET_SPACING + CD_FRET_SPACING / 2;
                svg << "<line x1=\"" << x1 << "\" y1=\"" << by
                    << "\" x2=\"" << x2 << "\" y2=\"" << by
                    << "\" stroke=\"#4A90D9\" stroke-width=\""
                    << CD_DOT_R * 2 << "\" stroke-linecap=\"round\" opacity=\"0.4\"/>\n";
            }
        }
    }

    // String tuning labels above markers.
    for (std::size_t i = 0; i < f.strings.size(); ++i) {
        int x = str_x(static_cast<int>(i));
        int open_midi = fb.tuning().open_midi[static_cast<int>(i)];
        svg << "<text x=\"" << x << "\" y=\"" << grid_y - CD_MARKER_H - 2
            << "\" class=\"flbl\" text-anchor=\"middle\">"
            << midi_label(open_midi) << "</text>\n";
    }

    // X/O markers and dots.
    // Strings in the Fingering are ordered 1st (highest) to last (lowest).
    for (std::size_t i = 0; i < f.strings.size(); ++i) {
        const auto& ss = f.strings[i];
        int x = str_x(static_cast<int>(i));

        if (ss.action == StringAction::Muted) {
            // X marker above.
            int my = grid_y - CD_MARKER_H / 2;
            int s = 4;
            svg << "<line x1=\"" << x - s << "\" y1=\"" << my - s
                << "\" x2=\"" << x + s << "\" y2=\"" << my + s
                << "\" class=\"mute\"/>\n";
            svg << "<line x1=\"" << x + s << "\" y1=\"" << my - s
                << "\" x2=\"" << x - s << "\" y2=\"" << my + s
                << "\" class=\"mute\"/>\n";
        } else if (ss.action == StringAction::Open) {
            // O marker above.
            int my = grid_y - CD_MARKER_H / 2;
            svg << "<circle cx=\"" << x << "\" cy=\"" << my
                << "\" r=\"4\" class=\"open\"/>\n";
        } else {
            // Fretted: dot at the right fret.
            int fret_offset = ss.fret - start_fret;
            if (fret_offset >= 0 && fret_offset < n_frets) {
                int dy = grid_y + fret_offset * CD_FRET_SPACING + CD_FRET_SPACING / 2;
                // Draw individual dot only if NOT covered by barré bar.
                if (!in_barre[i]) {
                    svg << "<circle cx=\"" << x << "\" cy=\"" << dy
                        << "\" r=\"" << CD_DOT_R << "\" class=\"dot\"/>\n";
                }
                // Note label always shown (on top of barré bar or dot).
                int midi = fb.midi_at(static_cast<int>(i) + 1, ss.fret);
                int pc = ((midi % 12) + 12) % 12;
                svg << "<text x=\"" << x << "\" y=\"" << dy + 3
                    << "\" class=\"lbl\">" << CHROMATIC[pc] << "</text>\n";
            }
        }
    }

    svg << "</svg>\n";
    return svg.str();
}

// ---------------------------------------------------------------------------
// Scale/fretboard (horizontal orientation)
//
// Handedness layout:
//   Y axis (strings): always E2 (grave) top, E4 (acute) bottom.
//   X axis (frets):   RightHanded = nut left, frets grow right.
//                     LeftHanded  = nut right, frets grow left.
// ---------------------------------------------------------------------------

std::string FretboardSVG::render_fretboard(const Fretboard& fb,
                                             const std::vector<FretPosition>& pos,
                                             int fret_lo, int fret_hi,
                                             const std::string& title,
                                             int root_pc,
                                             const std::string& dot_color,
                                             Handedness handedness) {
    int n_str = fb.num_strings();
    int n_frets = fret_hi - fret_lo;
    if (n_frets < 1) n_frets = 1;
    bool show_nut = (fret_lo == 0);
    bool lefty = (handedness == Handedness::LeftHanded);

    // Helper: map logical string index to visual y position.
    // idx=0 = 1st string (E4), idx=5 = 6th string (E2).
    // Both righty and lefty: grave (E2) on top, acute (E4) on bottom.
    // Only X axis differs (nut side, fret direction).
    auto str_visual = [&](int idx) -> int {
        return n_str - 1 - idx;
    };

    // Dimensions.
    int grid_w = n_frets * FB_FRET_SPACING;
    int grid_h = (n_str - 1) * FB_STR_SPACING;
    int title_h = title.empty() ? 0 : FB_TITLE_H;
    int fret_label_h = 22;
    int marker_w = show_nut ? FB_MARKER_W : 0;
    int total_w = FB_PAD * 2 + FB_STR_LABEL_W + marker_w + grid_w;
    int total_h = FB_PAD * 2 + title_h + grid_h + fret_label_h;

    // For lefty: labels on the right side, nut on the right, frets grow leftward.
    // For righty: labels on the left side, nut on the left, frets grow rightward.
    int grid_x = lefty
        ? FB_PAD + grid_w   // grid_x = right edge of the grid (nut position)
        : FB_PAD + FB_STR_LABEL_W + marker_w;  // grid_x = left edge (nut position)
    int grid_y = FB_PAD + title_h;

    // Helper: map fret offset (0 = nut) to X coordinate.
    // Righty: fret 0 at grid_x, grows right. Lefty: fret 0 at grid_x, grows left.
    auto fret_x = [&](int offset) -> int {
        return lefty ? (grid_x - offset * FB_FRET_SPACING)
                     : (grid_x + offset * FB_FRET_SPACING);
    };

    // Grid left and right edges in canvas coordinates.
    int grid_left  = lefty ? fret_x(n_frets) : fret_x(0);
    int grid_right = lefty ? fret_x(0) : fret_x(n_frets);

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";
    svg << css_chord();

    // Background.
    svg << "<rect width=\"" << total_w << "\" height=\"" << total_h
        << "\" fill=\"#F5F5F5\" rx=\"4\"/>\n";

    // Title.
    if (!title.empty()) {
        svg << "<text x=\"" << total_w / 2 << "\" y=\"" << FB_PAD + 16
            << "\" class=\"title\">" << title << "</text>\n";
    }

    // Nut.
    if (show_nut) {
        int nut_x = fret_x(0);
        svg << "<line x1=\"" << nut_x << "\" y1=\"" << grid_y - 1
            << "\" x2=\"" << nut_x << "\" y2=\"" << grid_y + grid_h + 1
            << "\" class=\"nut\"/>\n";
    }

    // Fret lines (vertical).
    for (int i = 1; i <= n_frets; ++i) {
        int x = fret_x(i);
        svg << "<line x1=\"" << x << "\" y1=\"" << grid_y
            << "\" x2=\"" << x << "\" y2=\"" << grid_y + grid_h
            << "\" class=\"frt\"/>\n";
    }

    // String lines (horizontal).
    for (int i = 0; i < n_str; ++i) {
        int vi = str_visual(i);
        int y = grid_y + vi * FB_STR_SPACING;
        svg << "<line x1=\"" << grid_left << "\" y1=\"" << y
            << "\" x2=\"" << grid_right << "\" y2=\"" << y
            << "\" class=\"str\"/>\n";

        // String name label — on the side opposite to the nut.
        int open_midi = fb.tuning().open_midi[i];
        if (lefty) {
            // Labels on the right side.
            int lbl_x = show_nut ? (grid_right + FB_MARKER_W + 4) : (grid_right + 8);
            svg << "<text x=\"" << lbl_x << "\" y=\"" << y + 4
                << "\" class=\"flbl\" text-anchor=\"start\">"
                << midi_label(open_midi) << "</text>\n";
        } else {
            // Labels on the left side.
            int lbl_x = show_nut ? (grid_x - FB_MARKER_W - 4) : (grid_x - 8);
            svg << "<text x=\"" << lbl_x << "\" y=\"" << y + 4
                << "\" class=\"flbl\" text-anchor=\"end\">"
                << midi_label(open_midi) << "</text>\n";
        }
    }

    // Fret number labels below.
    for (int i = 0; i <= n_frets; ++i) {
        int fret_num = fret_lo + i;
        bool show = (fret_num == fret_lo) || (fret_num == fret_hi) ||
                    (fret_num == 3) || (fret_num == 5) || (fret_num == 7) ||
                    (fret_num == 9) || (fret_num == 12) || (fret_num == 15) ||
                    (fret_num == 17) || (fret_num == 19);
        if (show || n_frets <= 5) {
            int x = fret_x(i);
            svg << "<text x=\"" << x << "\" y=\"" << grid_y + grid_h + 16
                << "\" class=\"flbl\" text-anchor=\"middle\">"
                << fret_num << "</text>\n";
        }
    }

    // Highlighted dots.
    for (const auto& p : pos) {
        int fret_offset = p.fret - fret_lo;
        if (fret_offset < 0 || fret_offset > n_frets) continue;

        int x = fret_x(fret_offset);
        if (p.fret > 0) {
            // Center between frets: shift towards nut by half spacing.
            x += lefty ? (FB_FRET_SPACING / 2) : -(FB_FRET_SPACING / 2);
        }
        int vi = str_visual(p.string - 1);
        int y = grid_y + vi * FB_STR_SPACING;

        if (p.fret == 0) {
            // Open string: O marker (hollow circle) on the nut side.
            int ox = lefty ? (grid_right + FB_MARKER_W / 2)
                           : (grid_left - FB_MARKER_W / 2);
            svg << "<circle cx=\"" << ox << "\" cy=\"" << y
                << "\" r=\"4\" class=\"open\"/>\n";
        } else {
            // Fretted: filled dot.
            svg << "<circle cx=\"" << x << "\" cy=\"" << y
                << "\" r=\"" << CD_DOT_R << "\" class=\"dot\"/>\n";

            // Note label inside dot.
            svg << "<text x=\"" << x << "\" y=\"" << y + 3
                << "\" class=\"lbl\">" << p.note << "</text>\n";
        }
    }

    svg << "</svg>\n";
    return svg.str();
}

// ---------------------------------------------------------------------------
// Vertical positions renderer (chord box extended for scales/positions)
// ---------------------------------------------------------------------------

std::string FretboardSVG::render_diagram_positions(
    const Fretboard& fb,
    const std::vector<FretPosition>& pos,
    int fret_lo, int fret_hi,
    const std::string& title,
    int root_pc,
    const std::string& dot_color,
    Handedness handedness) {

    int n_str = fb.num_strings();
    int n_frets = fret_hi - fret_lo;
    if (n_frets < 1) n_frets = 1;
    bool show_nut = (fret_lo == 0);
    bool lefty = (handedness == Handedness::LeftHanded);

    // Helper: map logical string index (0=1st/highest) to visual x index.
    auto str_visual = [&](int idx) -> int {
        return lefty ? idx : (n_str - 1 - idx);
    };

    // Dimensions — use CD_ constants for vertical orientation.
    int grid_w = (n_str - 1) * CD_STR_SPACING;
    int grid_h = n_frets * CD_FRET_SPACING;
    int fret_label_w = CD_FRET_LABEL_W;
    int str_label_h = 18;
    int marker_h = show_nut ? CD_MARKER_H : 0;
    int title_h = title.empty() ? 0 : CD_TITLE_H;
    int total_w = CD_PAD * 2 + grid_w + fret_label_w;
    int total_h = CD_PAD * 2 + title_h + str_label_h + marker_h + grid_h;

    int grid_x = CD_PAD + fret_label_w;
    int grid_y = CD_PAD + title_h + str_label_h + marker_h;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";
    svg << css_chord();

    // Background.
    svg << "<rect width=\"" << total_w << "\" height=\"" << total_h
        << "\" fill=\"#F5F5F5\" rx=\"4\"/>\n";

    // Title.
    if (!title.empty()) {
        svg << "<text x=\"" << total_w / 2 << "\" y=\"" << CD_PAD + 14
            << "\" class=\"title\">" << title << "</text>\n";
    }

    // Nut or fret start number.
    if (show_nut) {
        svg << "<line x1=\"" << grid_x - 1 << "\" y1=\"" << grid_y
            << "\" x2=\"" << grid_x + grid_w + 1 << "\" y2=\"" << grid_y
            << "\" class=\"nut\"/>\n";
    }

    // Fret lines (horizontal).
    for (int i = 0; i <= n_frets; ++i) {
        int y = grid_y + i * CD_FRET_SPACING;
        svg << "<line x1=\"" << grid_x << "\" y1=\"" << y
            << "\" x2=\"" << grid_x + grid_w << "\" y2=\"" << y
            << "\" class=\"frt\"/>\n";
    }

    // String lines (vertical).
    for (int i = 0; i < n_str; ++i) {
        int vi = str_visual(i);
        int x = grid_x + vi * CD_STR_SPACING;
        svg << "<line x1=\"" << x << "\" y1=\"" << grid_y
            << "\" x2=\"" << x << "\" y2=\"" << grid_y + grid_h
            << "\" class=\"str\"/>\n";

        // String name label above.
        int open_midi = fb.tuning().open_midi[i];
        svg << "<text x=\"" << x << "\" y=\"" << grid_y - 4
            << "\" class=\"flbl\" text-anchor=\"middle\">"
            << midi_label(open_midi) << "</text>\n";
    }

    // Fret number labels on the left.
    for (int i = 0; i < n_frets; ++i) {
        int fret_num = fret_lo + i + 1;
        int y = grid_y + i * CD_FRET_SPACING + CD_FRET_SPACING / 2;
        svg << "<text x=\"" << grid_x - 6 << "\" y=\"" << y + 4
            << "\" class=\"flbl\" text-anchor=\"end\">" << fret_num << "</text>\n";
    }

    // Highlighted dots.
    for (const auto& p : pos) {
        int fret_offset = p.fret - fret_lo;
        if (fret_offset < 0 || fret_offset > n_frets) continue;

        int vi = str_visual(p.string - 1);
        int x = grid_x + vi * CD_STR_SPACING;

        if (p.fret == 0) {
            // Open string: O marker (hollow circle).
            int y = grid_y - CD_MARKER_H / 2;
            svg << "<circle cx=\"" << x << "\" cy=\"" << y
                << "\" r=\"4\" class=\"open\"/>\n";
        } else {
            int y = grid_y + fret_offset * CD_FRET_SPACING - CD_FRET_SPACING / 2;
            svg << "<circle cx=\"" << x << "\" cy=\"" << y
                << "\" r=\"" << CD_DOT_R << "\" class=\"dot\"/>\n";
            svg << "<text x=\"" << x << "\" y=\"" << y + 3
                << "\" class=\"lbl\">" << p.note << "</text>\n";
        }
    }

    svg << "</svg>\n";
    return svg.str();
}

// ---------------------------------------------------------------------------
// Public API — single methods
// ---------------------------------------------------------------------------

std::string FretboardSVG::chord(const Fretboard& fb, const Chord& c,
                                  int position,
                                  Orientation orientation,
                                  Handedness handedness) {
    auto f = fb.fingering(c, position);
    if (orientation == Orientation::Vertical) {
        return render_diagram(fb, f, c.name(), handedness);
    }
    // Horizontal: convert fingering to positions.
    int lo, hi;
    auto pos = fingering_to_positions(fb, f, lo, hi);
    int root_pc = c.notes().empty() ? -1 : c.notes()[0].semitone();
    return render_fretboard(fb, pos, lo, hi, c.name(), root_pc, "", handedness);
}

std::string FretboardSVG::fingering(const Fretboard& fb, const Fingering& f,
                                      Orientation orientation,
                                      Handedness handedness) {
    if (orientation == Orientation::Vertical) {
        return render_diagram(fb, f, f.chord_name, handedness);
    }
    int lo, hi;
    auto pos = fingering_to_positions(fb, f, lo, hi);
    return render_fretboard(fb, pos, lo, hi, f.chord_name, -1, "", handedness);
}

std::string FretboardSVG::scale(const Fretboard& fb, const Scale& s,
                                  int fret_lo, int fret_hi,
                                  Orientation orientation,
                                  Handedness handedness) {
    auto pos = fb.scale_positions(s, fret_lo, fret_hi);
    std::string title = s.tonic().name() + " " + s.mode_name();
    int root_pc = s.tonic().semitone();
    if (orientation == Orientation::Horizontal) {
        return render_fretboard(fb, pos, fret_lo, fret_hi, title, root_pc, "", handedness);
    }
    return render_diagram_positions(fb, pos, fret_lo, fret_hi, title, root_pc, "", handedness);
}

std::string FretboardSVG::positions(const Fretboard& fb,
                                      const std::vector<FretPosition>& highlighted,
                                      const std::string& title,
                                      Orientation orientation,
                                      Handedness handedness) {
    // Determine fret range from positions.
    int lo = 0, hi = 12;
    if (!highlighted.empty()) {
        lo = highlighted[0].fret;
        hi = highlighted[0].fret;
        for (const auto& p : highlighted) {
            if (p.fret < lo) lo = p.fret;
            if (p.fret > hi) hi = p.fret;
        }
        hi = std::min(hi + 2, fb.num_frets());
    }
    if (orientation == Orientation::Horizontal) {
        return render_fretboard(fb, highlighted, lo, hi, title, -1, "", handedness);
    }
    return render_diagram_positions(fb, highlighted, lo, hi, title, -1, "", handedness);
}

std::string FretboardSVG::note(const Fretboard& fb, const Note& n,
                                 Orientation orientation,
                                 Handedness handedness) {
    auto pos = fb.positions(n);
    int hi = std::min(12, fb.num_frets());
    if (orientation == Orientation::Horizontal) {
        return render_fretboard(fb, pos, 0, hi, n.name(), n.semitone(), "", handedness);
    }
    return render_diagram_positions(fb, pos, 0, hi, n.name(), n.semitone(), "", handedness);
}

// ---------------------------------------------------------------------------
// Composite helpers
// ---------------------------------------------------------------------------

/// Extract inner SVG content: everything after the first '>', but skip
/// <style>...</style> blocks (they'll be added once in the wrapper).
static std::string extract_inner(const std::string& svg) {
    auto start = svg.find('>');
    auto end = svg.rfind("</svg>");
    if (start == std::string::npos || end == std::string::npos) return "";

    std::string inner = svg.substr(start + 1, end - start - 1);

    // Remove <style>...</style> blocks.
    for (;;) {
        auto spos = inner.find("<style>");
        if (spos == std::string::npos) break;
        auto epos = inner.find("</style>", spos);
        if (epos == std::string::npos) break;
        inner.erase(spos, epos + 8 - spos);
    }
    return inner;
}

/// Parse width and height from an SVG string.
static bool parse_svg_dims(const std::string& svg, int& w, int& h) {
    auto wpos = svg.find("width=\"");
    auto hpos = svg.find("height=\"");
    if (wpos == std::string::npos || hpos == std::string::npos) return false;
    w = std::stoi(svg.substr(wpos + 7));
    h = std::stoi(svg.substr(hpos + 8));
    return true;
}

/// Compose multiple SVGs into a single SVG with layout.
static std::string compose_diagrams(const std::vector<std::string>& svgs,
                                      const std::string& css,
                                      Layout layout, int n,
                                      int gap_h, int gap_v,
                                      int grid_cols, int comp_pad) {
    int cell_w = 0, cell_h = 0;
    for (const auto& s : svgs) {
        if (!s.empty() && parse_svg_dims(s, cell_w, cell_h)) break;
    }
    if (cell_w == 0 || cell_h == 0) return "";

    int total_w = 0, total_h = 0;
    std::ostringstream out;

    if (layout == Layout::Grid) {
        int cols = grid_cols;
        int rows = (n + cols - 1) / cols;
        total_w = comp_pad * 2 + cols * cell_w + (cols - 1) * gap_h;
        total_h = comp_pad * 2 + rows * cell_h + (rows - 1) * gap_v;
    } else if (layout == Layout::Horizontal) {
        total_w = comp_pad * 2 + n * cell_w + (n - 1) * gap_h;
        total_h = comp_pad * 2 + cell_h;
    } else {
        total_w = comp_pad * 2 + cell_w;
        total_h = comp_pad * 2 + n * cell_h + (n - 1) * gap_v;
    }

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";
    out << css;

    for (int i = 0; i < n; ++i) {
        if (static_cast<std::size_t>(i) >= svgs.size() || svgs[i].empty())
            continue;

        int x = 0, y = 0;
        if (layout == Layout::Grid) {
            int col = i % grid_cols;
            int row = i / grid_cols;
            x = comp_pad + col * (cell_w + gap_h);
            y = comp_pad + row * (cell_h + gap_v);
        } else if (layout == Layout::Horizontal) {
            x = comp_pad + i * (cell_w + gap_h);
            y = comp_pad;
        } else {
            x = comp_pad;
            y = comp_pad + i * (cell_h + gap_v);
        }

        std::string inner = extract_inner(svgs[i]);
        out << "<g transform=\"translate(" << x << "," << y << ")\">\n"
            << inner << "</g>\n";
    }
    out << "</svg>\n";
    return out.str();
}

// ---------------------------------------------------------------------------
// Composite methods
// ---------------------------------------------------------------------------

std::string FretboardSVG::field(const Fretboard& fb, const Field& f,
                                  Layout layout,
                                  Orientation orientation,
                                  Handedness handedness) {
    auto chords = f.chords();
    int n = static_cast<int>(chords.size());

    std::vector<std::string> svgs;
    svgs.reserve(n);

    for (int i = 0; i < n; ++i) {
        std::string roman = (i < 10) ? ROMAN[i] : std::to_string(i + 1);
        std::string title = roman + std::string(" - ") + chords[i].name();

        try {
            auto fg = fb.fingering(chords[i]);
            if (orientation == Orientation::Vertical) {
                svgs.push_back(render_diagram(fb, fg, title, handedness));
            } else {
                int lo, hi;
                auto pos = fingering_to_positions(fb, fg, lo, hi);
                int rpc = chords[i].notes().empty() ? -1 : chords[i].notes()[0].semitone();
                svgs.push_back(render_fretboard(fb, pos, lo, hi, title, rpc, "", handedness));
            }
        } catch (...) {
            svgs.push_back("");
        }
    }

    std::string css = css_chord();
    return compose_diagrams(svgs, css, layout, n,
                             GAP_H, GAP_V, GRID_COLS, COMP_PAD);
}

std::string FretboardSVG::progression(const Fretboard& fb, const Field& f,
                                        const std::vector<std::string>& branches,
                                        Layout layout,
                                        Orientation orientation,
                                        Handedness handedness) {
    auto field_chords = f.chords();
    int field_size = static_cast<int>(field_chords.size());

    auto roman_to_degree = [](const std::string& r) -> int {
        std::string upper;
        for (char c : r) {
            if (c >= 'A' && c <= 'Z') upper += c;
            else if ((c == 'b' || c == '#') && upper.empty()) continue;
            else break;
        }
        if (upper == "I")    return 0;
        if (upper == "II")   return 1;
        if (upper == "III")  return 2;
        if (upper == "IV")   return 3;
        if (upper == "V")    return 4;
        if (upper == "VI")   return 5;
        if (upper == "VII")  return 6;
        return -1;
    };

    std::vector<std::string> svgs;
    int n = 0;

    for (const auto& branch : branches) {
        int deg = roman_to_degree(branch);
        if (deg < 0 || deg >= field_size) continue;

        Chord c = field_chords[deg];
        std::string title = branch + " (" + c.name() + ")";

        try {
            auto fg = fb.fingering(c);
            if (orientation == Orientation::Vertical) {
                svgs.push_back(render_diagram(fb, fg, title, handedness));
            } else {
                int lo, hi;
                auto pos = fingering_to_positions(fb, fg, lo, hi);
                int rpc = c.notes().empty() ? -1 : c.notes()[0].semitone();
                svgs.push_back(render_fretboard(fb, pos, lo, hi, title, rpc, "", handedness));
            }
        } catch (...) {
            svgs.push_back("");
        }
        n++;
    }

    if (n == 0) return "";

    std::string css = css_chord();
    return compose_diagrams(svgs, css, layout, n,
                             GAP_H, GAP_V, GRID_COLS, COMP_PAD);
}

// ---------------------------------------------------------------------------
// Full fretboard
// ---------------------------------------------------------------------------

std::string FretboardSVG::full(const Fretboard& fb,
                                 Orientation orientation,
                                 Handedness handedness) {
    // Empty positions — just the bare fretboard.
    std::vector<FretPosition> pos;
    int fret_hi = fb.num_frets();
    std::string title = fb.name();
    if (orientation == Orientation::Horizontal) {
        return render_fretboard(fb, pos, 0, fret_hi, title, -1, "", handedness);
    }
    return render_diagram_positions(fb, pos, 0, fret_hi, title, -1, "", handedness);
}

// ---------------------------------------------------------------------------
// Write
// ---------------------------------------------------------------------------

void FretboardSVG::write(const std::string& svg, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot write to file: " + path);
    file << svg;
}

} // namespace gingo
