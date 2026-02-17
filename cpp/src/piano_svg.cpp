// Gingo — Music Theory Library
// PianoSVG implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/piano_svg.hpp>
#include <gingo/tree.hpp>

#include <algorithm>
#include <fstream>
#include <iomanip>
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

const char* PianoSVG::degree_color(int index) {
    if (index < 0) index = 0;
    if (index >= 10) index = 7;
    return DEGREE_COLORS[index];
}

bool PianoSVG::is_black(int midi) {
    int pc = ((midi % 12) + 12) % 12;
    return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
}

int PianoSVG::white_key_x(int white_index) {
    return white_index * WHITE_W;
}

int PianoSVG::black_key_x(int midi, int prev_white_x) {
    return prev_white_x + WHITE_W - BLACK_W / 2;
}

std::string PianoSVG::midi_label(int midi) {
    int pc = ((midi % 12) + 12) % 12;
    int octave = midi / 12 - 1;
    return std::string(CHROMATIC[pc]) + std::to_string(octave);
}

std::vector<PianoSVG::KeyInfo> PianoSVG::build_keys(int lo, int hi,
                                                     int& white_count) {
    std::vector<KeyInfo> keys;
    keys.reserve(hi - lo + 1);
    white_count = 0;
    int last_white_x = 0;

    for (int m = lo; m <= hi; ++m) {
        if (is_black(m)) {
            int x = black_key_x(m, last_white_x);
            keys.push_back({m, true, x});
        } else {
            int x = white_key_x(white_count);
            last_white_x = x;
            keys.push_back({m, false, x});
            ++white_count;
        }
    }
    return keys;
}

// ---------------------------------------------------------------------------
// CSS
// ---------------------------------------------------------------------------

std::string PianoSVG::css_style() {
    return
        "<style>\n"
        ".w{width:23px;height:120px;fill:#fff;stroke:#333;stroke-width:1;ry:2}\n"
        ".b{width:14px;height:78px;fill:#111;stroke:#000;stroke-width:1;ry:2}\n"
        ".h.w{fill:#4A90D9}\n"
        ".h.b{fill:#2E6AB0}\n"
        ".t{font-family:Arial,Helvetica,sans-serif;font-weight:bold;"
        "fill:#fff;text-anchor:middle;pointer-events:none}\n"
        ".tw{font-size:9px}\n"
        ".tb{font-size:7px}\n"
        ".title{font-family:Arial,Helvetica,sans-serif;font-size:14px;"
        "font-weight:bold;fill:#333;text-anchor:middle}\n"
        "</style>\n";
}

std::string PianoSVG::css_composite(const std::vector<std::string>& colors) {
    std::ostringstream css;
    css << "<style>\n"
        << ".w{width:23px;height:120px;fill:#fff;stroke:#333;stroke-width:1;ry:2}\n"
        << ".b{width:14px;height:78px;fill:#111;stroke:#000;stroke-width:1;ry:2}\n"
        << ".t{font-family:Arial,Helvetica,sans-serif;font-weight:bold;"
           "fill:#fff;text-anchor:middle;pointer-events:none}\n"
        << ".tw{font-size:9px}\n"
        << ".tb{font-size:7px}\n"
        << ".title{font-family:Arial,Helvetica,sans-serif;font-size:12px;"
           "font-weight:bold;fill:#333;text-anchor:middle}\n"
        << ".band{font-family:Arial,Helvetica,sans-serif;font-size:9px;"
           "font-weight:bold;fill:#fff;text-anchor:middle;pointer-events:none}\n";

    for (std::size_t i = 0; i < colors.size(); ++i) {
        // Darken the color for black keys: subtract ~30 from each channel.
        int r = 74, g = 111, b2 = 165;
        if (colors[i].size() == 7 && colors[i][0] == '#') {
            r = static_cast<int>(std::stoul(colors[i].substr(1, 2), nullptr, 16));
            g = static_cast<int>(std::stoul(colors[i].substr(3, 2), nullptr, 16));
            b2 = static_cast<int>(std::stoul(colors[i].substr(5, 2), nullptr, 16));
        }
        int dr = std::max(0, r - 30);
        int dg = std::max(0, g - 30);
        int db = std::max(0, b2 - 30);

        std::ostringstream doss;
        doss << "#" << std::hex << std::uppercase;
        doss << (dr < 16 ? "0" : "") << dr;
        doss << (dg < 16 ? "0" : "") << dg;
        doss << (db < 16 ? "0" : "") << db;
        std::string dark = doss.str();

        css << ".d" << i << ".w{fill:" << colors[i] << "}\n"
            << ".d" << i << ".b{fill:" << dark << "}\n";
    }

    css << "</style>\n";
    return css.str();
}

// ---------------------------------------------------------------------------
// Compact range
// ---------------------------------------------------------------------------

std::pair<int, int> PianoSVG::compact_range(
        const Piano& piano,
        const std::vector<int>& highlight_midi) {
    int lo, hi;

    if (highlight_midi.empty()) {
        lo = 48;
        hi = 71;
    } else {
        int min_m = *std::min_element(highlight_midi.begin(),
                                       highlight_midi.end());
        int max_m = *std::max_element(highlight_midi.begin(),
                                       highlight_midi.end());
        lo = min_m - 12;
        hi = max_m + 12;
        lo = (lo / 12) * 12;
        hi = ((hi / 12) + 1) * 12 - 1;
    }

    lo = std::max(lo, piano.lowest().midi);
    hi = std::min(hi, piano.highest().midi);
    return {lo, hi};
}

std::pair<int, int> PianoSVG::unified_range(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis) {
    int global_min = 999, global_max = -1;
    for (const auto& midis : all_midis) {
        for (int m : midis) {
            if (m < global_min) global_min = m;
            if (m > global_max) global_max = m;
        }
    }
    if (global_max < 0) {
        global_min = 48;
        global_max = 71;
    }

    int lo = global_min - 7;
    int hi = global_max + 7;
    lo = (lo / 12) * 12;
    hi = ((hi / 12) + 1) * 12 - 1;

    lo = std::max(lo, piano.lowest().midi);
    hi = std::min(hi, piano.highest().midi);
    return {lo, hi};
}

// ---------------------------------------------------------------------------
// Core renderer (single piano, standalone SVG)
// ---------------------------------------------------------------------------

std::string PianoSVG::render(const Piano& piano,
                              const std::vector<int>& highlight_midi,
                              const std::string& title,
                              bool compact) {
    std::set<int> hl(highlight_midi.begin(), highlight_midi.end());

    int lo, hi;
    if (compact) {
        auto range = compact_range(piano, highlight_midi);
        lo = range.first;
        hi = range.second;
    } else {
        lo = piano.lowest().midi;
        hi = piano.highest().midi;
    }

    int white_count = 0;
    auto all_keys = build_keys(lo, hi, white_count);

    int total_w = white_count * WHITE_W;
    int title_offset = title.empty() ? 0 : TITLE_H;
    int total_h = WHITE_H + title_offset;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" "
        << "height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";

    svg << css_style();

    svg << "<rect width=\"" << total_w << "\" height=\"" << total_h
        << "\" fill=\"#F5F5F5\" rx=\"4\"/>\n";

    if (!title.empty()) {
        svg << "<text x=\"" << total_w / 2 << "\" y=\"18\" "
            << "class=\"title\">"
            << title << "</text>\n";
    }

    svg << "<g transform=\"translate(0," << title_offset << ")\">\n";

    for (const auto& ki : all_keys) {
        if (ki.black) continue;
        bool highlighted = hl.count(ki.midi) > 0;
        int oct = ki.midi / 12 - 1;
        int pc = ((ki.midi % 12) + 12) % 12;

        svg << "<rect id=\"key-" << ki.midi << "\" class=\"w"
            << (highlighted ? " h" : "") << "\" "
            << "data-midi=\"" << ki.midi << "\" "
            << "data-note=\"" << CHROMATIC[pc] << "\" "
            << "data-octave=\"" << oct << "\" "
            << "x=\"" << ki.x << "\" y=\"0\"/>\n";

        if (highlighted) {
            int tx = ki.x + WHITE_W / 2;
            int ty = WHITE_H - 8;
            svg << "<text x=\"" << tx << "\" y=\"" << ty
                << "\" class=\"t tw\">"
                << midi_label(ki.midi) << "</text>\n";
        }
    }

    for (const auto& ki : all_keys) {
        if (!ki.black) continue;
        bool highlighted = hl.count(ki.midi) > 0;
        int oct = ki.midi / 12 - 1;
        int pc = ((ki.midi % 12) + 12) % 12;

        svg << "<rect id=\"key-" << ki.midi << "\" class=\"b"
            << (highlighted ? " h" : "") << "\" "
            << "data-midi=\"" << ki.midi << "\" "
            << "data-note=\"" << CHROMATIC[pc] << "\" "
            << "data-octave=\"" << oct << "\" "
            << "x=\"" << ki.x << "\" y=\"0\"/>\n";

        if (highlighted) {
            int tx = ki.x + BLACK_W / 2;
            int ty = BLACK_H - 6;
            svg << "<text x=\"" << tx << "\" y=\"" << ty
                << "\" class=\"t tb\">"
                << midi_label(ki.midi) << "</text>\n";
        }
    }

    svg << "</g>\n";
    svg << "</svg>\n";

    return svg.str();
}

// ---------------------------------------------------------------------------
// render_group — fragment for composite views
// ---------------------------------------------------------------------------

std::string PianoSVG::render_group(int lo, int hi,
                                    const std::vector<int>& highlight_midi,
                                    const std::string& title,
                                    const std::string& color,
                                    int group_id,
                                    int& out_width, int& out_height) {
    std::set<int> hl(highlight_midi.begin(), highlight_midi.end());

    // Determine CSS class for highlight: "d{group_id}" instead of "h".
    std::string dcls = "d" + std::to_string(group_id);

    int white_count = 0;
    auto all_keys = build_keys(lo, hi, white_count);

    int total_w = white_count * WHITE_W;
    int title_offset = title.empty() ? 0 : TITLE_H;
    int total_h = WHITE_H + title_offset;

    out_width = total_w;
    out_height = total_h;

    std::ostringstream svg;

    // Background.
    svg << "<rect width=\"" << total_w << "\" height=\"" << total_h
        << "\" fill=\"#F0EDE8\" rx=\"3\"/>\n";

    // Title.
    if (!title.empty()) {
        svg << "<text x=\"" << total_w / 2 << "\" y=\"17\" "
            << "class=\"title\">"
            << title << "</text>\n";
    }

    // Keys group.
    svg << "<g transform=\"translate(0," << title_offset << ")\">\n";

    // White keys.
    for (const auto& ki : all_keys) {
        if (ki.black) continue;
        bool highlighted = hl.count(ki.midi) > 0;

        svg << "<rect id=\"p" << group_id << "-key-" << ki.midi
            << "\" class=\"w"
            << (highlighted ? " " + dcls : "") << "\" "
            << "x=\"" << ki.x << "\" y=\"0\"/>\n";

        if (highlighted) {
            int tx = ki.x + WHITE_W / 2;
            int ty = WHITE_H - 8;
            svg << "<text x=\"" << tx << "\" y=\"" << ty
                << "\" class=\"t tw\">"
                << midi_label(ki.midi) << "</text>\n";
        }
    }

    // Black keys.
    for (const auto& ki : all_keys) {
        if (!ki.black) continue;
        bool highlighted = hl.count(ki.midi) > 0;

        svg << "<rect id=\"p" << group_id << "-key-" << ki.midi
            << "\" class=\"b"
            << (highlighted ? " " + dcls : "") << "\" "
            << "x=\"" << ki.x << "\" y=\"0\"/>\n";

        if (highlighted) {
            int tx = ki.x + BLACK_W / 2;
            int ty = BLACK_H - 6;
            svg << "<text x=\"" << tx << "\" y=\"" << ty
                << "\" class=\"t tb\">"
                << midi_label(ki.midi) << "</text>\n";
        }
    }

    svg << "</g>\n";
    return svg.str();
}

// ---------------------------------------------------------------------------
// Vertical layout — stacked pianos
// ---------------------------------------------------------------------------

std::string PianoSVG::render_vertical(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis,
        const std::vector<std::string>& titles,
        const std::vector<std::string>& colors) {
    if (all_midis.empty()) return "";

    auto range = unified_range(piano, all_midis);
    int lo = range.first, hi = range.second;

    // Render each group.
    struct Sub { std::string svg; int w; int h; };
    std::vector<Sub> subs;
    subs.reserve(all_midis.size());

    for (std::size_t i = 0; i < all_midis.size(); ++i) {
        int w, h;
        auto g = render_group(lo, hi, all_midis[i], titles[i],
                              colors[i], static_cast<int>(i), w, h);
        subs.push_back({std::move(g), w, h});
    }

    int max_w = 0;
    int total_h = 0;
    for (const auto& s : subs) {
        if (s.w > max_w) max_w = s.w;
        total_h += s.h;
    }
    total_h += GAP_V * static_cast<int>(subs.size() - 1);
    total_h += PAD * 2;
    int total_w = max_w + PAD * 2;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";
    svg << css_composite(colors);

    int y = PAD;
    for (std::size_t i = 0; i < subs.size(); ++i) {
        int x = PAD + (max_w - subs[i].w) / 2;
        svg << "<g transform=\"translate(" << x << "," << y << ")\">\n"
            << subs[i].svg << "</g>\n";
        y += subs[i].h + GAP_V;
    }

    svg << "</svg>\n";
    return svg.str();
}

// ---------------------------------------------------------------------------
// Horizontal layout — continuous strip with degree bands
// ---------------------------------------------------------------------------

std::string PianoSVG::render_horizontal(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis,
        const std::vector<std::string>& titles,
        const std::vector<std::string>& colors) {
    if (all_midis.empty()) return "";

    auto range = unified_range(piano, all_midis);
    int lo = range.first, hi = range.second;

    int white_count = 0;
    auto all_keys = build_keys(lo, hi, white_count);
    int piano_w = white_count * WHITE_W;

    int n = static_cast<int>(all_midis.size());
    int bands_h = n * (BAND_H + BAND_GAP);
    int connector_h = 8;
    int piano_y = PAD + bands_h + connector_h;
    int total_w = piano_w + PAD * 2;
    int total_h = piano_y + WHITE_H + PAD;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";
    svg << css_composite(colors);

    // Degree bands.
    for (int i = 0; i < n; ++i) {
        std::set<int> hl(all_midis[i].begin(), all_midis[i].end());
        int band_y = PAD + i * (BAND_H + BAND_GAP);

        // Band background (light).
        svg << "<rect x=\"" << PAD << "\" y=\"" << band_y
            << "\" width=\"" << piano_w << "\" height=\"" << BAND_H
            << "\" rx=\"3\" fill=\"" << colors[i] << "\" opacity=\"0.12\"/>\n";

        // Band label on left.
        svg << "<text x=\"" << PAD + 14 << "\" y=\"" << band_y + 14
            << "\" class=\"band\" fill=\"" << colors[i] << "\">"
            << titles[i] << "</text>\n";

        // Highlight segments + connector lines.
        for (const auto& ki : all_keys) {
            if (hl.count(ki.midi) == 0) continue;
            int kw = ki.black ? BLACK_W : WHITE_W;
            int kx = PAD + ki.x;

            // Colored segment in the band.
            svg << "<rect x=\"" << kx << "\" y=\"" << band_y
                << "\" width=\"" << kw << "\" height=\"" << BAND_H
                << "\" rx=\"2\" fill=\"" << colors[i] << "\" opacity=\"0.85\"/>\n";

            // Note label in band.
            int pc = ((ki.midi % 12) + 12) % 12;
            svg << "<text x=\"" << kx + kw / 2 << "\" y=\"" << band_y + 14
                << "\" class=\"band\">" << CHROMATIC[pc] << "</text>\n";

            // Vertical connector line from band bottom to piano top.
            int cx = kx + kw / 2;
            svg << "<line x1=\"" << cx << "\" y1=\"" << band_y + BAND_H
                << "\" x2=\"" << cx << "\" y2=\"" << piano_y
                << "\" stroke=\"" << colors[i]
                << "\" stroke-width=\"0.7\" opacity=\"0.35\"/>\n";
        }
    }

    // Piano (unhighlighted base).
    svg << "<g transform=\"translate(" << PAD << "," << piano_y << ")\">\n";
    svg << "<rect width=\"" << piano_w << "\" height=\"" << WHITE_H
        << "\" fill=\"#F0EDE8\" rx=\"3\"/>\n";

    // White keys.
    for (const auto& ki : all_keys) {
        if (ki.black) continue;
        svg << "<rect class=\"w\" x=\"" << ki.x << "\" y=\"0\"/>\n";
    }

    // Black keys.
    for (const auto& ki : all_keys) {
        if (!ki.black) continue;
        svg << "<rect class=\"b\" x=\"" << ki.x << "\" y=\"0\"/>\n";
    }

    svg << "</g>\n";
    svg << "</svg>\n";
    return svg.str();
}

// ---------------------------------------------------------------------------
// Grid layout — mini pianos in rows
// ---------------------------------------------------------------------------

std::string PianoSVG::render_grid(
        const Piano& piano,
        const std::vector<std::vector<int>>& all_midis,
        const std::vector<std::string>& titles,
        const std::vector<std::string>& colors) {
    if (all_midis.empty()) return "";

    auto range = unified_range(piano, all_midis);
    int lo = range.first, hi = range.second;

    int n = static_cast<int>(all_midis.size());
    int cols = GRID_COLS;
    int rows = (n + cols - 1) / cols;

    // Render each group to get dimensions.
    struct Sub { std::string svg; int w; int h; };
    std::vector<Sub> subs;
    subs.reserve(n);

    for (int i = 0; i < n; ++i) {
        int w, h;
        auto g = render_group(lo, hi, all_midis[i], titles[i],
                              colors[i], i, w, h);
        subs.push_back({std::move(g), w, h});
    }

    // Scale to fit in grid.
    int cell_w = subs[0].w;
    int cell_h = subs[0].h;

    int total_w = PAD * 2 + cols * cell_w + (cols - 1) * GAP_H;
    int total_h = PAD * 2 + rows * cell_h + (rows - 1) * GAP_V;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";
    svg << css_composite(colors);

    for (int i = 0; i < n; ++i) {
        int col = i % cols;
        int row = i / cols;
        int x = PAD + col * (cell_w + GAP_H);
        int y = PAD + row * (cell_h + GAP_V);
        svg << "<g transform=\"translate(" << x << "," << y << ")\">\n"
            << subs[i].svg << "</g>\n";
    }

    svg << "</svg>\n";
    return svg.str();
}

// ---------------------------------------------------------------------------
// Public API — single piano methods
// ---------------------------------------------------------------------------

std::string PianoSVG::note(const Piano& piano, const Note& n,
                           int octave, bool compact) {
    auto k = piano.key(n, octave);
    std::string title = n.name() + std::to_string(octave);
    return render(piano, {k.midi}, title, compact);
}

std::string PianoSVG::chord(const Piano& piano, const Chord& c,
                             int octave, VoicingStyle style, bool compact) {
    auto v = piano.voicing(c, octave, style);
    std::vector<int> midis;
    midis.reserve(v.keys.size());
    for (const auto& k : v.keys) midis.push_back(k.midi);
    return render(piano, midis, c.name(), compact);
}

std::string PianoSVG::scale(const Piano& piano, const Scale& s,
                             int octave, bool compact) {
    auto sk = piano.scale_keys(s, octave);
    std::vector<int> midis;
    midis.reserve(sk.size());
    for (const auto& k : sk) midis.push_back(k.midi);
    std::string title = s.tonic().name() + " " + s.mode_name();
    return render(piano, midis, title, compact);
}

std::string PianoSVG::keys(const Piano& piano,
                            const std::vector<PianoKey>& highlighted,
                            const std::string& title, bool compact) {
    std::vector<int> midis;
    midis.reserve(highlighted.size());
    for (const auto& k : highlighted) midis.push_back(k.midi);
    return render(piano, midis, title, compact);
}

std::string PianoSVG::voicing(const Piano& piano, const PianoVoicing& v,
                               bool compact) {
    std::vector<int> midis;
    midis.reserve(v.keys.size());
    for (const auto& k : v.keys) midis.push_back(k.midi);
    std::string style_str;
    switch (v.style) {
        case VoicingStyle::Close: style_str = "close"; break;
        case VoicingStyle::Open:  style_str = "open";  break;
        case VoicingStyle::Shell: style_str = "shell"; break;
    }
    std::string title = v.chord_name + " (" + style_str + ")";
    return render(piano, midis, title, compact);
}

std::string PianoSVG::midi(const Piano& piano,
                            const std::vector<int>& midi_numbers,
                            bool compact) {
    return render(piano, midi_numbers, "", compact);
}

// ---------------------------------------------------------------------------
// Public API — composite methods
// ---------------------------------------------------------------------------

std::string PianoSVG::field(const Piano& piano, const Field& f,
                            int octave, Layout layout, bool sevenths) {
    auto chords = sevenths ? f.sevenths() : f.chords();
    int n = static_cast<int>(chords.size());

    std::vector<std::vector<int>> all_midis;
    std::vector<std::string> titles;
    std::vector<std::string> colors;
    all_midis.reserve(n);
    titles.reserve(n);
    colors.reserve(n);

    for (int i = 0; i < n; ++i) {
        auto v = piano.voicing(chords[i], octave, VoicingStyle::Close);
        std::vector<int> midis;
        midis.reserve(v.keys.size());
        for (const auto& k : v.keys) midis.push_back(k.midi);

        std::string roman_str = (i < 10) ? ROMAN[i] : std::to_string(i + 1);
        std::string title = roman_str + " - " + chords[i].name();

        all_midis.push_back(std::move(midis));
        titles.push_back(std::move(title));
        colors.push_back(degree_color(i));
    }

    switch (layout) {
        case Layout::Vertical:
            return render_vertical(piano, all_midis, titles, colors);
        case Layout::Horizontal:
            return render_horizontal(piano, all_midis, titles, colors);
        case Layout::Grid:
            return render_grid(piano, all_midis, titles, colors);
    }
    return render_vertical(piano, all_midis, titles, colors);
}

std::string PianoSVG::progression(const Piano& piano,
                                   const Field& f,
                                   const std::vector<std::string>& branches,
                                   int octave, Layout layout) {
    // Resolve each branch to a chord using the field.
    // Match branch label to degree: I→1, II→2, etc.
    // For applied chords (V7/IIm), use field.applied().
    auto field_chords = f.chords();
    auto field_sevenths = f.sevenths();
    int field_size = static_cast<int>(field_chords.size());

    // Map Roman numerals to degree index.
    auto roman_to_degree = [](const std::string& r) -> int {
        // Strip quality suffix to get the numeral.
        std::string upper;
        for (char c : r) {
            if (c >= 'A' && c <= 'Z') upper += c;
            else if (c == 'b' && upper.empty()) continue; // accidental prefix
            else if (c == '#' && upper.empty()) continue;
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

    std::vector<std::vector<int>> all_midis;
    std::vector<std::string> titles;
    std::vector<std::string> colors;

    for (const auto& branch : branches) {
        int deg = roman_to_degree(branch);
        if (deg < 0 || deg >= field_size) continue;

        // Use seventh chord if the branch contains "7", otherwise triad.
        bool is_seventh = branch.find('7') != std::string::npos;
        Chord c = is_seventh ? field_sevenths[deg] : field_chords[deg];

        auto v = piano.voicing(c, octave, VoicingStyle::Close);
        std::vector<int> midis;
        midis.reserve(v.keys.size());
        for (const auto& k : v.keys) midis.push_back(k.midi);

        std::string title = branch + " (" + c.name() + ")";

        all_midis.push_back(std::move(midis));
        titles.push_back(std::move(title));
        colors.push_back(degree_color(deg));
    }

    switch (layout) {
        case Layout::Vertical:
            return render_vertical(piano, all_midis, titles, colors);
        case Layout::Horizontal:
            return render_horizontal(piano, all_midis, titles, colors);
        case Layout::Grid:
            return render_grid(piano, all_midis, titles, colors);
    }
    return render_vertical(piano, all_midis, titles, colors);
}

void PianoSVG::write(const std::string& svg, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot write to file: " + path);
    file << svg;
}

} // namespace gingo
