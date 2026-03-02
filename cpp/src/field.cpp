// Gingo — Music Theory Library
// Implementation of the Field class — harmonic field generation.
//
// A harmonic field is the set of chords naturally built from each degree
// of a scale by stacking thirds (or other intervals).  This implements the
// algorithm from the original Arduino library (harmonicfield / harmonics /
// getSpinFormalScale) in a clean, object-oriented form.
//
// SPDX-License-Identifier: MIT

#include "gingo/field.hpp"
#include "gingo/internal/lookup_data.hpp"
#include "gingo/internal/data_ops.hpp"
#include "gingo/internal/notation_utils.hpp"

#include <algorithm>
#include <cmath>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------------

namespace {

/// Map a ScaleType enum value to its display name for to_string().
std::string scale_type_name(ScaleType type) {
    switch (type) {
        case ScaleType::Major:         return "Major";
        case ScaleType::NaturalMinor:  return "NaturalMinor";
        case ScaleType::HarmonicMinor: return "HarmonicMinor";
        case ScaleType::MelodicMinor:  return "MelodicMinor";
        case ScaleType::Diminished:    return "Diminished";
        case ScaleType::HarmonicMajor: return "HarmonicMajor";
        case ScaleType::WholeTone:     return "WholeTone";
        case ScaleType::Augmented:     return "Augmented";
        case ScaleType::Blues:         return "Blues";
        case ScaleType::Chromatic:     return "Chromatic";
    }
    return "Unknown";
}

/// Convert a degree name ("1st", "3rd", "5th", "7th", "9th", etc.) to a
/// zero-based offset within the 7-note diatonic scale.
///
/// The extended degrees (9th, 11th, 13th) wrap back into the 7-note cycle:
///   9th  = 2nd of the next octave → offset 1
///   11th = 4th of the next octave → offset 3
///   13th = 6th of the next octave → offset 5
///
/// This matches the original spin-table construction in harmonics(), where
/// columns 9/11/13 were added as SpinTable[(i+1)%7][0], [(i+3)%7][0],
/// [(i+5)%7][0] — i.e. the same notes as the 2nd, 4th, and 6th degrees.
int degree_offset(const std::string& deg) {
    if (deg == "1st")  return 0;
    if (deg == "2nd")  return 1;
    if (deg == "3rd")  return 2;
    if (deg == "4th")  return 3;
    if (deg == "5th")  return 4;
    if (deg == "6th")  return 5;
    if (deg == "7th")  return 6;
    if (deg == "9th")  return 1;
    if (deg == "11th") return 3;
    if (deg == "13th") return 5;
    return 0;
}

/// Parsed representation of a Roman numeral expression (e.g. "bVII7", "IIm7(b5)").
struct ParsedRoman {
    int degree;           // 1–7
    int accidental;       // -1 (flat), 0 (natural), +1 (sharp)
    std::string quality;  // chord quality suffix (may be empty)
};

/// Parse a Roman numeral expression into degree, accidental, and quality.
///
/// Grammar: [accidental] roman_numeral quality
///   accidental    = 'b' | '#'   (prefix — before the numeral)
///   roman_numeral = VII | III | VI | IV | II | V | I   (greedy match)
///   quality       = remaining characters
///
/// Examples:
///   "V7"         → {5, 0, "7"}
///   "IIm7(b5)"   → {2, 0, "m7(b5)"}
///   "bVII7"      → {7, -1, "7"}
///   "#IVm7"      → {4, +1, "m7"}
///   "V"          → {5, 0, ""}
ParsedRoman parse_roman(const std::string& s) {
    if (s.empty())
        throw std::invalid_argument(
            "Field::applied: empty Roman numeral expression");

    ParsedRoman result{0, 0, ""};
    std::size_t pos = 0;

    // 1. Leading accidental
    if (s[0] == 'b') {
        result.accidental = -1;
        pos = 1;
    } else if (s[0] == '#') {
        result.accidental = 1;
        pos = 1;
    }

    // 2. Greedy-match Roman numeral (longer numerals first)
    auto rest = s.substr(pos);
    if (rest.size() >= 3 && rest.substr(0, 3) == "VII") {
        result.degree = 7; pos += 3;
    } else if (rest.size() >= 3 && rest.substr(0, 3) == "III") {
        result.degree = 3; pos += 3;
    } else if (rest.size() >= 2 && rest.substr(0, 2) == "VI") {
        result.degree = 6; pos += 2;
    } else if (rest.size() >= 2 && rest.substr(0, 2) == "IV") {
        result.degree = 4; pos += 2;
    } else if (rest.size() >= 2 && rest.substr(0, 2) == "II") {
        result.degree = 2; pos += 2;
    } else if (!rest.empty() && rest[0] == 'V') {
        result.degree = 5; pos += 1;
    } else if (!rest.empty() && rest[0] == 'I') {
        result.degree = 1; pos += 1;
    } else {
        throw std::invalid_argument(
            "Field::applied: could not parse Roman numeral from \""
            + s + "\"");
    }

    // 3. Quality = everything remaining
    result.quality = s.substr(pos);

    return result;
}

/// Resolve an applied chord given a parsed function and a target root note.
/// When quality is empty, returns the naturally occurring triad at the degree.
Chord resolve_applied_chord(const ParsedRoman& function,
                            const Note& target_note) {
    if (function.quality.empty()) {
        // No explicit quality → return naturally occurring chord
        Field intermediate_field(target_note.natural(), ScaleType::Major);
        Chord natural_chord = intermediate_field.chord(function.degree);

        if (function.accidental != 0) {
            Note root = Note(natural_chord.root().natural())
                            .transpose(function.accidental);
            return Chord(root.natural() + natural_chord.type());
        }
        return natural_chord;
    }

    // Explicit quality → build root from intermediate scale + quality
    Scale intermediate(target_note.natural(), ScaleType::Major);
    Note root = intermediate.degree(function.degree);
    if (function.accidental != 0)
        root = root.transpose(function.accidental);
    return Chord(root.natural() + function.quality);
}

// ---------------------------------------------------------------------------
// Harmonic function tables
// ---------------------------------------------------------------------------

/// Function assignment per degree (0-indexed) for each ScaleType.
///
/// Major:         I→T  II→S  III→T  IV→S  V→D  VI→T  VII→D
/// NaturalMinor:  i→T  ii→S  III→T  iv→S  v→D  VI→S  VII→D
/// HarmonicMinor: i→T  ii→S  III→T  iv→S  V→D  VI→S  vii→D
/// MelodicMinor:  i→T  ii→S  III→T  IV→S  V→D  vi→S  vii→D
/// Diminished:    symmetric — alternating T/D for 8 degrees
using HF = HarmonicFunction;

const HF kFunctionTable[][8] = {
    // Major (7)
    {HF::Tonic, HF::Subdominant, HF::Tonic, HF::Subdominant,
     HF::Dominant, HF::Tonic, HF::Dominant, HF::Tonic},
    // NaturalMinor (7)
    {HF::Tonic, HF::Subdominant, HF::Tonic, HF::Subdominant,
     HF::Dominant, HF::Subdominant, HF::Dominant, HF::Tonic},
    // HarmonicMinor (7)
    {HF::Tonic, HF::Subdominant, HF::Tonic, HF::Subdominant,
     HF::Dominant, HF::Subdominant, HF::Dominant, HF::Tonic},
    // MelodicMinor (7)
    {HF::Tonic, HF::Subdominant, HF::Tonic, HF::Subdominant,
     HF::Dominant, HF::Subdominant, HF::Dominant, HF::Tonic},
    // Diminished (8)
    {HF::Tonic, HF::Dominant, HF::Tonic, HF::Dominant,
     HF::Tonic, HF::Dominant, HF::Tonic, HF::Dominant},
};

/// Role description per degree for each ScaleType.
const char* kRoleTable[][8] = {
    // Major
    {"primary", "relative of IV", "transitive", "primary",
     "primary", "relative of I", "relative of V", ""},
    // NaturalMinor
    {"primary", "relative of IV", "relative of I", "primary",
     "primary", "relative of IV", "relative of V", ""},
    // HarmonicMinor
    {"primary", "relative of IV", "relative of I", "primary",
     "primary", "relative of IV", "relative of V", ""},
    // MelodicMinor
    {"primary", "relative of IV", "relative of I", "primary",
     "primary", "relative of IV", "relative of V", ""},
    // Diminished
    {"primary", "primary", "primary", "primary",
     "primary", "primary", "primary", "primary"},
};

} // anonymous namespace

// ---------------------------------------------------------------------------
// HarmonicFunction free functions
// ---------------------------------------------------------------------------

std::string harmonic_function_name(HarmonicFunction f) {
    switch (f) {
        case HarmonicFunction::Tonic:       return "Tonic";
        case HarmonicFunction::Subdominant: return "Subdominant";
        case HarmonicFunction::Dominant:    return "Dominant";
    }
    return "Unknown";
}

std::string harmonic_function_short(HarmonicFunction f) {
    switch (f) {
        case HarmonicFunction::Tonic:       return "T";
        case HarmonicFunction::Subdominant: return "S";
        case HarmonicFunction::Dominant:    return "D";
    }
    return "?";
}

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Field::Field(const std::string& tonic, ScaleType type)
    : scale_(tonic, type)
{}

Field::Field(const std::string& tonic, const std::string& type_name)
    : scale_(tonic, type_name)
{}

Field::Field(Scale s)
    : scale_(std::move(s))
{}

// ---------------------------------------------------------------------------
// Core algorithm
// ---------------------------------------------------------------------------

std::vector<Chord> Field::build_chords(
        const std::vector<std::string>& degree_filter) const {

    // Use formal (properly-spelled diatonic) notes so that chord
    // identification produces correct names — e.g. BbM rather than A#M.
    // Only keep unique pitch classes (first octave) to avoid duplicating
    // notes from the two-octave binary mask.
    auto all_notes = scale_.formal_notes();
    std::vector<Note> scale_notes;
    std::set<int> seen_semitones;
    for (const auto& note : all_notes) {
        if (seen_semitones.insert(note.semitone()).second) {
            scale_notes.push_back(note);
        }
    }
    const auto n = scale_notes.size();

    if (n == 0) {
        return {};
    }

    // Convert degree names to numeric offsets within the scale.
    std::vector<int> offsets;
    offsets.reserve(degree_filter.size());
    for (const auto& d : degree_filter) {
        offsets.push_back(degree_offset(d));
    }

    // Build one chord per scale degree.
    std::vector<Chord> result;
    result.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        // Collect chord tones by picking scale notes at the filtered
        // offsets, wrapping around modulo n.  For a triad with offsets
        // {0, 2, 4} on degree i this yields notes at indices
        // {i%n, (i+2)%n, (i+4)%n} — stacked thirds.
        std::vector<Note> chord_notes;
        chord_notes.reserve(offsets.size());

        for (int off : offsets) {
            chord_notes.push_back(
                scale_notes[(i + static_cast<std::size_t>(off)) % n]);
        }

        // Identify the chord from its constituent notes.
        try {
            result.push_back(Chord::identify(chord_notes));
        } catch (...) {
            // Fallback: construct a basic major chord from the root when
            // identification fails (should not happen for standard
            // diatonic scales, but guards against exotic scale types).
            result.push_back(Chord(chord_notes[0].natural() + "M"));
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Public accessors
// ---------------------------------------------------------------------------

std::vector<Chord> Field::chords() const {
    return build_chords({"1st", "3rd", "5th"});
}

std::vector<Chord> Field::sevenths() const {
    return build_chords({"1st", "3rd", "5th", "7th"});
}

Chord Field::chord(int degree) const {
    auto c = chords();
    if (degree < 1 || static_cast<std::size_t>(degree) > c.size()) {
        throw std::out_of_range(
            "Field::chord: degree " + std::to_string(degree) +
            " out of range [1, " + std::to_string(c.size()) + "]");
    }
    return c[static_cast<std::size_t>(degree - 1)];
}

Chord Field::seventh(int degree) const {
    auto c = sevenths();
    if (degree < 1 || static_cast<std::size_t>(degree) > c.size()) {
        throw std::out_of_range(
            "Field::seventh: degree " + std::to_string(degree) +
            " out of range [1, " + std::to_string(c.size()) + "]");
    }
    return c[static_cast<std::size_t>(degree - 1)];
}

// ---------------------------------------------------------------------------
// Applied chords (tonicization)
// ---------------------------------------------------------------------------

Chord Field::applied(const std::string& function, int target_degree) const {
    auto parsed = parse_roman(function);
    Note target_note = scale_.degree(target_degree);
    return resolve_applied_chord(parsed, target_note);
}

Chord Field::applied(const std::string& function,
                     const std::string& target) const {
    auto parsed_target = parse_roman(target);

    Note target_note = scale_.degree(parsed_target.degree);
    if (parsed_target.accidental != 0)
        target_note = target_note.transpose(parsed_target.accidental);

    auto parsed = parse_roman(function);
    return resolve_applied_chord(parsed, target_note);
}

Chord Field::applied(int function_degree, int target_degree) const {
    Note target_note = scale_.degree(target_degree);
    Field intermediate_field(target_note.natural(), ScaleType::Major);
    return intermediate_field.seventh(function_degree);
}

// ---------------------------------------------------------------------------
// Harmonic function and role
// ---------------------------------------------------------------------------

std::optional<int> Field::find_degree(const Chord& chord) const {
    auto triads = chords();
    int target = chord.root().semitone();
    for (std::size_t i = 0; i < triads.size(); ++i) {
        if (triads[i].root().semitone() == target)
            return static_cast<int>(i + 1);
    }
    return std::nullopt;
}

HarmonicFunction Field::function(int degree) const {
    auto n = size();
    if (degree < 1 || static_cast<std::size_t>(degree) > n)
        throw std::out_of_range(
            "Field::function: degree " + std::to_string(degree) +
            " out of range [1, " + std::to_string(n) + "]");
    int type_idx = static_cast<int>(scale_.type());
    return kFunctionTable[type_idx][degree - 1];
}

std::optional<HarmonicFunction> Field::function(const std::string& chord_name) const {
    try {
        Chord c(chord_name);
        return function(c);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<HarmonicFunction> Field::function(const Chord& chord) const {
    auto deg = find_degree(chord);
    if (!deg) return std::nullopt;
    return function(*deg);
}

NoteContext Field::noteContext(const Note& note) const {
    // Find degree by matching note pitch class to scale tones
    uint8_t degree = 0;
    bool inScale = false;
    HarmonicFunction func = HarmonicFunction::Tonic;

    for (int d = 1; d <= 7; d++) {
        if (chord(d).root().semitone() == note.semitone()) {
            degree = static_cast<uint8_t>(d);
            inScale = true;
            func = this->function(d);
            break;
        }
    }

    // Ascending interval from field tonic to this note (0-11 semitones)
    int semis = note.semitone() - scale_.tonic().semitone();
    if (semis < 0) semis += 12;

    NoteContext ctx;
    ctx.note     = note;
    ctx.degree   = degree;
    ctx.interval = Interval(semis);
    ctx.function = func;
    ctx.inScale  = inScale;
    return ctx;
}

std::string Field::role(int degree) const {
    auto n = size();
    if (degree < 1 || static_cast<std::size_t>(degree) > n)
        throw std::out_of_range(
            "Field::role: degree " + std::to_string(degree) +
            " out of range [1, " + std::to_string(n) + "]");
    int type_idx = static_cast<int>(scale_.type());
    return kRoleTable[type_idx][degree - 1];
}

std::optional<std::string> Field::role(const std::string& chord_name) const {
    try {
        Chord c(chord_name);
        return role(c);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<std::string> Field::role(const Chord& chord) const {
    auto deg = find_degree(chord);
    if (!deg) return std::nullopt;
    return role(*deg);
}

// ---------------------------------------------------------------------------
// Chord comparison in field context
// ---------------------------------------------------------------------------

namespace {

/// Relative chord pairs per ScaleType — degree pairs that share harmonic
/// function and overlap significantly in note content.
bool is_relative_pair(ScaleType type, int da, int db) {
    using ST = ScaleType;
    auto match = [&](int x, int y) {
        return (da == x && db == y) || (da == y && db == x);
    };
    switch (type) {
        case ST::Major:
            return match(1, 6) || match(2, 4) || match(3, 5);
        case ST::NaturalMinor:
        case ST::HarmonicMinor:
        case ST::MelodicMinor:
            return match(1, 3) || match(4, 6) || match(5, 7);
        case ST::Diminished:
        case ST::HarmonicMajor:
        case ST::WholeTone:
        case ST::Augmented:
        case ST::Blues:
        case ST::Chromatic:
            return false;
    }
    return false;
}

/// Map a degree (1-7) + chord type to a basic branch label.
/// Builds strings like "I", "IIm", "V7", "VIm", etc.
std::string degree_to_roman(int degree) {
    static const char* kNumerals[] = {
        "", "I", "II", "III", "IV", "V", "VI", "VII", "VIII"
    };
    if (degree < 1 || degree > 8) return "";
    return kNumerals[degree];
}

/// Scale type enum → display name.
const char* kScaleTypeNames[] = {
    "Major", "NaturalMinor", "HarmonicMinor", "MelodicMinor", "Diminished"
};

const ScaleType kParallelTypes[] = {
    ScaleType::Major, ScaleType::NaturalMinor,
    ScaleType::HarmonicMinor, ScaleType::MelodicMinor
};

} // anonymous namespace

FieldComparison Field::compare(const Chord& a, const Chord& b) const {
    FieldComparison r;

    // 13. Degrees
    r.degree_a = find_degree(a);
    r.degree_b = find_degree(b);

    // 14. Functions
    r.function_a = r.degree_a
        ? std::optional<HarmonicFunction>(function(*r.degree_a))
        : std::nullopt;
    r.function_b = r.degree_b
        ? std::optional<HarmonicFunction>(function(*r.degree_b))
        : std::nullopt;

    // 15. Roles
    r.role_a = r.degree_a
        ? std::optional<std::string>(role(*r.degree_a))
        : std::nullopt;
    r.role_b = r.degree_b
        ? std::optional<std::string>(role(*r.degree_b))
        : std::nullopt;

    // 16. Degree distance
    if (r.degree_a && r.degree_b)
        r.degree_distance = std::abs(*r.degree_a - *r.degree_b);

    // 17. Same function
    if (r.function_a && r.function_b)
        r.same_function = (*r.function_a == *r.function_b);

    // 18. Relative chord pair
    r.relative = false;
    if (r.degree_a && r.degree_b)
        r.relative = is_relative_pair(scale_.type(), *r.degree_a, *r.degree_b);

    // 19. Progression (reserved for future use)
    r.progression = false;

    // 19b. Root motion (objective root interval classification)
    r.root_motion = "";
    if (r.degree_a && r.degree_b) {
        int ar = a.root().semitone();
        int br = b.root().semitone();
        int motion = (br - ar + 12) % 12;

        if      (motion == 0)                r.root_motion = "unison";
        else if (motion == 1 || motion == 2) r.root_motion = "ascending_step";
        else if (motion == 3 || motion == 4) r.root_motion = "ascending_third";
        else if (motion == 5)                r.root_motion = "descending_fifth";
        else if (motion == 6)                r.root_motion = "tritone";
        else if (motion == 7)                r.root_motion = "ascending_fifth";
        else if (motion == 8 || motion == 9) r.root_motion = "descending_third";
        else                                 r.root_motion = "descending_step";
    }

    // 20. Secondary dominant
    r.secondary_dominant = "";
    {
        int a_root = a.root().semitone();
        int b_root = b.root().semitone();
        // V7 root is 7 semitones above the target root
        if (a.type() == "7" && (a_root - b_root + 12) % 12 == 7)
            r.secondary_dominant = "a_is_V7_of_b";
        else if (b.type() == "7" && (b_root - a_root + 12) % 12 == 7)
            r.secondary_dominant = "b_is_V7_of_a";
    }

    // 20b. Applied diminished (vii-dim/x leading-tone relationship)
    r.applied_diminished = "";
    {
        int a_root = a.root().semitone();
        int b_root = b.root().semitone();
        bool a_is_dim = (a.type() == "dim" || a.type() == "m7(b5)");
        bool b_is_dim = (b.type() == "dim" || b.type() == "m7(b5)");

        if (a_is_dim && (b_root - a_root + 12) % 12 == 1)
            r.applied_diminished = "a_is_viidim_of_b";
        else if (b_is_dim && (a_root - b_root + 12) % 12 == 1)
            r.applied_diminished = "b_is_viidim_of_a";
    }

    // 21. Diatonic — chord must exactly match a triad or seventh in the field,
    //     not just have its root on a scale degree.
    auto is_diatonic = [&](const Chord& chord) -> bool {
        auto triads = chords();
        auto sevs = sevenths();
        int target_root = chord.root().semitone();
        std::string target_type = chord.type();
        for (const auto& c : triads) {
            if (c.root().semitone() == target_root && c.type() == target_type)
                return true;
        }
        for (const auto& c : sevs) {
            if (c.root().semitone() == target_root && c.type() == target_type)
                return true;
        }
        return false;
    };
    r.diatonic_a = is_diatonic(a);
    r.diatonic_b = is_diatonic(b);

    // 22/26. Borrowed chord info (from parallel fields)
    auto check_borrowed = [&](const Chord& chord, bool is_diatonic)
            -> std::optional<BorrowedInfo> {
        if (is_diatonic) return std::nullopt;

        for (int i = 0; i < 4; ++i) {
            if (kParallelTypes[i] == scale_.type()) continue;
            try {
                Field alt(scale_.tonic().natural(), kParallelTypes[i]);
                auto deg = alt.find_degree(chord);
                if (deg) {
                    BorrowedInfo info;
                    info.scale_type = kScaleTypeNames[static_cast<int>(kParallelTypes[i])];
                    info.degree = *deg;
                    info.function = alt.function(*deg);
                    info.role = alt.role(*deg);
                    return info;
                }
            } catch (...) {}
        }
        return std::nullopt;
    };

    r.borrowed_a = check_borrowed(a, r.diatonic_a);
    r.borrowed_b = check_borrowed(b, r.diatonic_b);

    // 23. Pivot fields (fields where both chords coexist)
    const auto& chromatic = Note::chromatic();
    for (const auto& tonic_name : chromatic) {
        for (int ti = 0; ti < 4; ++ti) {
            try {
                Field pf(tonic_name, kParallelTypes[ti]);
                auto da = pf.find_degree(a);
                auto db = pf.find_degree(b);
                if (da && db) {
                    r.pivot.push_back(PivotInfo{
                        tonic_name,
                        kScaleTypeNames[static_cast<int>(kParallelTypes[ti])],
                        *da, *db
                    });
                }
            } catch (...) {}
        }
    }

    // 24. Tritone substitution
    int root_dist = std::abs(a.root().semitone() - b.root().semitone());
    root_dist = std::min(root_dist, 12 - root_dist);
    r.tritone_sub = (a.type() == "7" && b.type() == "7" && root_dist == 6);

    // 25. Chromatic mediant
    r.chromatic_mediant = "";
    if (root_dist == 3 || root_dist == 4) {
        int signed_diff = b.root().semitone() - a.root().semitone();
        if (signed_diff > 6)  signed_diff -= 12;
        if (signed_diff < -6) signed_diff += 12;
        r.chromatic_mediant = (signed_diff > 0) ? "upper" : "lower";
    }

    // 27. Foreign notes
    for (const auto& n : a.notes()) {
        if (!scale_.contains(n))
            r.foreign_a.push_back(n);
    }
    for (const auto& n : b.notes()) {
        if (!scale_.contains(n))
            r.foreign_b.push_back(n);
    }

    return r;
}

// ---------------------------------------------------------------------------
// Circle of fifths navigation
// ---------------------------------------------------------------------------

int Field::signature() const {
    return scale_.signature();
}

Field Field::relative() const {
    return Field(scale_.relative());
}

Field Field::parallel() const {
    return Field(scale_.parallel());
}

std::pair<Field, Field> Field::neighbors() const {
    auto [sub, dom] = scale_.neighbors();
    return {Field(std::move(sub)), Field(std::move(dom))};
}

// ---------------------------------------------------------------------------
// Size and display
// ---------------------------------------------------------------------------

std::size_t Field::size() const {
    return scale_.notes().size();
}

std::string Field::to_string() const {
    return "Field(\"" + scale_.tonic().name() + "\", \"" +
           scale_type_name(scale_.type()) + "\")";
}

} // namespace gingo
