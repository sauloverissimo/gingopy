// Gingo — Music Theory Library
// Mode name lookup table implementation.
//
// SPDX-License-Identifier: MIT

#include "gingo/internal/mode_data.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <unordered_map>

namespace gingo::internal {

// ---------------------------------------------------------------------------
// Mode table — all modes of all parent scales
// ---------------------------------------------------------------------------

static constexpr std::size_t kModeCount = 36;

// Primary mode entries.  Order: heptatonic families (7 each), then specials.
static const std::array<ModeInfo, kModeCount> kModes = {{
    // ── Major (7 modes) ────────────────────────────────────────────────
    {ScaleType::Major, 1, "Ionian",     "major", 6, 7},
    {ScaleType::Major, 2, "Dorian",     "minor", 3, 7},
    {ScaleType::Major, 3, "Phrygian",   "minor", 1, 7},
    {ScaleType::Major, 4, "Lydian",     "major", 7, 7},
    {ScaleType::Major, 5, "Mixolydian", "major", 5, 7},
    {ScaleType::Major, 6, "Aeolian",    "minor", 2, 7},
    {ScaleType::Major, 7, "Locrian",    "minor", 0, 7},

    // ── Harmonic Minor (7 modes) ──────────────────────────────────────
    {ScaleType::HarmonicMinor, 1, "Harmonic Minor",     "minor", 0, 7},
    {ScaleType::HarmonicMinor, 2, "Locrian nat6",       "minor", 0, 7},
    {ScaleType::HarmonicMinor, 3, "Ionian #5",          "major", 0, 7},
    {ScaleType::HarmonicMinor, 4, "Dorian #4",          "minor", 0, 7},
    {ScaleType::HarmonicMinor, 5, "Phrygian Dominant",  "major", 0, 7},
    {ScaleType::HarmonicMinor, 6, "Lydian #2",          "major", 0, 7},
    {ScaleType::HarmonicMinor, 7, "Ultralocrian",       "minor", 0, 7},

    // ── Melodic Minor (7 modes) ───────────────────────────────────────
    {ScaleType::MelodicMinor, 1, "Melodic Minor",    "minor", 0, 7},
    {ScaleType::MelodicMinor, 2, "Dorian b2",        "minor", 0, 7},
    {ScaleType::MelodicMinor, 3, "Lydian Augmented",  "major", 0, 7},
    {ScaleType::MelodicMinor, 4, "Lydian Dominant",   "major", 0, 7},
    {ScaleType::MelodicMinor, 5, "Mixolydian b6",     "major", 0, 7},
    {ScaleType::MelodicMinor, 6, "Locrian nat2",      "minor", 0, 7},
    {ScaleType::MelodicMinor, 7, "Altered",           "minor", 0, 7},

    // ── Harmonic Major (7 modes) ──────────────────────────────────────
    {ScaleType::HarmonicMajor, 1, "Harmonic Major",        "major", 0, 7},
    {ScaleType::HarmonicMajor, 2, "Dorian b5",             "minor", 0, 7},
    {ScaleType::HarmonicMajor, 3, "Phrygian b4",           "minor", 0, 7},
    {ScaleType::HarmonicMajor, 4, "Lydian Minor",          "minor", 0, 7},
    {ScaleType::HarmonicMajor, 5, "Mixolydian b2",         "major", 0, 7},
    {ScaleType::HarmonicMajor, 6, "Lydian Augmented #2",   "major", 0, 7},
    {ScaleType::HarmonicMajor, 7, "Locrian bb7",           "minor", 0, 7},

    // ── Special scales (no full mode hierarchy) ───────────────────────
    {ScaleType::Diminished, 1, "Diminished",           "minor", 0, 8},
    {ScaleType::Diminished, 2, "Dominant Diminished",   "major", 0, 8},
    {ScaleType::WholeTone,  1, "Whole Tone",           "major", 0, 6},
    {ScaleType::Augmented,  1, "Augmented",            "major", 0, 6},
    {ScaleType::Augmented,  2, "Augmented Inverse",    "minor", 0, 6},
    {ScaleType::Blues,      1, "Minor Blues",           "minor", 0, 6},
    {ScaleType::Blues,      2, "Major Blues",           "major", 0, 6},
    {ScaleType::Chromatic,  1, "Chromatic",            "major", 0, 12},
}};

// ---------------------------------------------------------------------------
// Alias map — lowercase string → index into kModes
// ---------------------------------------------------------------------------

namespace {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

struct AliasMap {
    std::unordered_map<std::string, std::size_t> map;

    AliasMap() {
        // Register primary names (lowercase)
        for (std::size_t i = 0; i < kModeCount; ++i) {
            map[to_lower(kModes[i].name)] = i;
        }

        // ── Major aliases ─────────────────────────────────────────────
        map["major"]         = 0;   // Ionian
        map["natural minor"] = 5;   // Aeolian
        map["minor natural"] = 5;

        // ── Harmonic Minor aliases ────────────────────────────────────
        map["aeolian nat7"]      = 7;   // Harmonic Minor
        map["augmented major"]   = 9;   // Ionian #5
        map["romanian"]          = 10;  // Dorian #4
        map["ukrainian dorian"]  = 10;
        map["phrygian nat3"]     = 11;  // Phrygian Dominant
        map["mixolydian b9 b13"] = 11;
        map["lydian #9"]         = 12;  // Lydian #2
        map["superlocrian bb7"]  = 13;  // Ultralocrian
        map["harmonic diminished"] = 13;

        // ── Melodic Minor aliases ─────────────────────────────────────
        map["jazz minor"]      = 14;  // Melodic Minor
        map["phrygian nat6"]   = 15;  // Dorian b2
        map["assyrian"]        = 15;
        map["lydian #5"]       = 16;  // Lydian Augmented
        map["lydian b7"]       = 17;  // Lydian Dominant
        map["acoustic"]        = 17;
        map["mixolydian #4"]   = 17;
        map["mixolydian b13"]  = 18;  // Mixolydian b6
        map["hindu"]           = 18;
        map["aeolian b5"]      = 19;  // Locrian nat2
        map["half-diminished"] = 19;
        map["super-locrian"]   = 20;  // Altered
        map["superlocrian"]    = 20;
        map["diminished whole-tone"] = 20;
        map["diminished whole tone"] = 20;

        // ── Harmonic Major aliases ────────────────────────────────────
        map["lydian b3"]       = 24;  // Lydian Minor
        map["mixolydian b9"]   = 25;  // Mixolydian b2
        map["lydian #5 #9"]    = 26;  // Lydian Augmented #2

        // ── Special scale aliases ─────────────────────────────────────
        map["diminished th"]       = 28;  // Diminished
        map["dominant diminished"] = 29;
        map["diminished ht"]       = 29;
        map["whole tone"]          = 30;  // WholeTone
        map["wholetone"]           = 30;
        map["minor blues"]         = 33;  // Blues
        map["major blues"]         = 34;
    }
};

const AliasMap& alias_map() {
    static const AliasMap m;
    return m;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Public lookup functions
// ---------------------------------------------------------------------------

const ModeInfo* find_mode_by_name(const std::string& name) {
    const std::string lower = to_lower(name);
    const auto& m = alias_map().map;
    auto it = m.find(lower);
    if (it != m.end()) {
        return &kModes[it->second];
    }
    return nullptr;
}

const ModeInfo* find_mode(ScaleType parent, int mode_number) {
    // NaturalMinor is structurally Major mode 6.  Map its mode numbers
    // to the equivalent Major mode so that mode_name() and quality()
    // resolve correctly:  NatMin mode n → Major mode ((n+4)%7 + 1).
    if (parent == ScaleType::NaturalMinor) {
        int mapped = ((mode_number + 4) % 7) + 1;
        return find_mode(ScaleType::Major, mapped);
    }

    for (const auto& m : kModes) {
        if (m.parent == parent && m.mode_number == mode_number) {
            return &m;
        }
    }
    return nullptr;
}

} // namespace gingo::internal
