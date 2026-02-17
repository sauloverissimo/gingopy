// Gingo — Music Theory Library
// PianoSVG implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/piano_svg.hpp>

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gingo {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

// Chromatic names (sharp-based) for labeling.
static const char* CHROMATIC[] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

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

// ---------------------------------------------------------------------------
// CSS style block
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

// ---------------------------------------------------------------------------
// Compact range
// ---------------------------------------------------------------------------

std::pair<int, int> PianoSVG::compact_range(
        const Piano& piano,
        const std::vector<int>& highlight_midi) {
    int lo, hi;

    if (highlight_midi.empty()) {
        // Default: 2 octaves around middle C (C3-B4).
        lo = 48;
        hi = 71;
    } else {
        int min_m = *std::min_element(highlight_midi.begin(),
                                       highlight_midi.end());
        int max_m = *std::max_element(highlight_midi.begin(),
                                       highlight_midi.end());
        // Expand by 1 octave each direction.
        lo = min_m - 12;
        hi = max_m + 12;
        // Snap to octave boundaries (C..B).
        lo = (lo / 12) * 12;           // floor to nearest C
        hi = ((hi / 12) + 1) * 12 - 1; // ceil to nearest B
    }

    // Clamp to piano range.
    lo = std::max(lo, piano.lowest().midi);
    hi = std::min(hi, piano.highest().midi);
    return {lo, hi};
}

// ---------------------------------------------------------------------------
// Core renderer
// ---------------------------------------------------------------------------

std::string PianoSVG::render(const Piano& piano,
                              const std::vector<int>& highlight_midi,
                              const std::string& title,
                              bool compact) {
    // Build a set for O(1) lookup.
    std::set<int> hl(highlight_midi.begin(), highlight_midi.end());

    // Determine MIDI range.
    int lo, hi;
    if (compact) {
        auto range = compact_range(piano, highlight_midi);
        lo = range.first;
        hi = range.second;
    } else {
        lo = piano.lowest().midi;
        hi = piano.highest().midi;
    }

    // Build layout.
    struct KeyInfo {
        int midi;
        bool black;
        int x;
    };
    std::vector<KeyInfo> all_keys;
    all_keys.reserve(hi - lo + 1);

    int white_count = 0;
    int last_white_x = 0;

    for (int m = lo; m <= hi; ++m) {
        if (is_black(m)) {
            int x = black_key_x(m, last_white_x);
            all_keys.push_back({m, true, x});
        } else {
            int x = white_key_x(white_count);
            last_white_x = x;
            all_keys.push_back({m, false, x});
            ++white_count;
        }
    }

    int total_w = white_count * WHITE_W;
    int title_offset = title.empty() ? 0 : TITLE_H;
    int total_h = WHITE_H + title_offset;

    std::ostringstream svg;
    svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << total_w << "\" "
        << "height=\"" << total_h << "\" "
        << "viewBox=\"0 0 " << total_w << " " << total_h << "\">\n";

    // CSS style block.
    svg << css_style();

    // Background.
    svg << "<rect width=\"" << total_w << "\" height=\"" << total_h
        << "\" fill=\"#F5F5F5\" rx=\"4\"/>\n";

    // Title.
    if (!title.empty()) {
        svg << "<text x=\"" << total_w / 2 << "\" y=\"18\" "
            << "class=\"title\">"
            << title << "</text>\n";
    }

    // Group with title offset.
    svg << "<g transform=\"translate(0," << title_offset << ")\">\n";

    // Pass 1: white keys.
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

    // Pass 2: black keys (on top).
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
// Public API
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

void PianoSVG::write(const std::string& svg, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Cannot write to file: " + path);
    file << svg;
}

} // namespace gingo
