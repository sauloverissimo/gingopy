// Gingo — Music Theory Library
// Scale implementation: ordered sequence of notes following a tonal pattern.
//
// SPDX-License-Identifier: MIT

#include "gingo/scale.hpp"
#include "gingo/internal/lookup_data.hpp"
#include "gingo/internal/data_ops.hpp"
#include "gingo/internal/notation_utils.hpp"
#include "gingo/internal/mode_data.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

namespace gingo {

using internal::TypeElement;
using internal::TypeVector;
using internal::element_to_int;
using internal::element_to_string;
using internal::LookupData;
using internal::get_formal_notes_tv;
using internal::spin;
using internal::ModeInfo;
using internal::find_mode;
using internal::find_mode_by_name;

// ---------------------------------------------------------------------------
// Local helpers
// ---------------------------------------------------------------------------

namespace {

std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

bool ends_with(const std::string& s, const std::string& suffix) {
    if (suffix.size() > s.size()) return false;
    return s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0;
}

const char* type_to_cstr(ScaleType t) {
    switch (t) {
        case ScaleType::Major:         return "major";
        case ScaleType::NaturalMinor:  return "natural minor";
        case ScaleType::HarmonicMinor: return "harmonic minor";
        case ScaleType::MelodicMinor:  return "melodic minor";
        case ScaleType::Diminished:    return "diminished";
        case ScaleType::HarmonicMajor: return "harmonic major";
        case ScaleType::WholeTone:     return "whole tone";
        case ScaleType::Augmented:     return "augmented";
        case ScaleType::Blues:         return "blues";
        case ScaleType::Chromatic:     return "chromatic";
    }
    return "unknown";
}

/// Find the semitone offset of the nth scale degree (1-based) within a
/// parent mask's first octave (positions 0-11).
int mode_offset(const TypeVector& parent_mask, int mode_number) {
    int count = 0;
    for (int i = 0; i < 12; ++i) {
        if (element_to_int(parent_mask[i]) == 1) {
            ++count;
            if (count == mode_number) return i;
        }
    }
    return 0;
}

/// How many notes have value 1 in the first octave of a mask.
int note_count_in_mask(const TypeVector& mask) {
    int count = 0;
    for (int i = 0; i < 12; ++i) {
        if (element_to_int(mask[i]) == 1) ++count;
    }
    return count;
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// parse_type / parse_modality
// ---------------------------------------------------------------------------

ScaleType Scale::parse_type(const std::string& name) {
    const std::string lower = to_lower(name);

    if (lower == "major" || lower == "maj" || lower == "0")
        return ScaleType::Major;
    if (lower == "natural minor" || lower == "minor natural" ||
        lower == "m"             || lower == "1")
        return ScaleType::NaturalMinor;
    if (lower == "minor" || lower == "harmonic minor" || lower == "2")
        return ScaleType::HarmonicMinor;
    if (lower == "melodic minor" || lower == "jazz minor" || lower == "3")
        return ScaleType::MelodicMinor;
    if (lower == "diminished" || lower == "dim" || lower == "4")
        return ScaleType::Diminished;
    if (lower == "harmonic major" || lower == "5")
        return ScaleType::HarmonicMajor;
    if (lower == "whole tone" || lower == "wholetone" || lower == "6")
        return ScaleType::WholeTone;
    if (lower == "augmented" || lower == "aug" || lower == "7")
        return ScaleType::Augmented;
    if (lower == "blues" || lower == "8")
        return ScaleType::Blues;
    if (lower == "chromatic" || lower == "9")
        return ScaleType::Chromatic;

    throw std::invalid_argument("Scale::parse_type: unknown scale type: " + name);
}

Modality Scale::parse_modality(const std::string& name) {
    const std::string lower = to_lower(name);

    if (lower == "diatonic")   return Modality::Diatonic;
    if (lower == "pentatonic") return Modality::Pentatonic;

    throw std::invalid_argument("Scale::parse_modality: unknown modality: " + name);
}

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Scale::Scale(const std::string& tonic, ScaleType type, Modality modality)
    : tonic_(tonic),
      parent_(type),
      mode_number_(1),
      pentatonic_(modality == Modality::Pentatonic)
{}

Scale::Scale(const std::string& tonic, const std::string& type_name,
             const std::string& modality_name)
    : tonic_(tonic),
      parent_(ScaleType::Major),
      mode_number_(1),
      pentatonic_(false)
{
    const std::string lower = to_lower(type_name);

    // Pentatonic flag from modality_name parameter
    if (to_lower(modality_name) == "pentatonic") {
        pentatonic_ = true;
    }

    // Special aliases for common pentatonic names (before suffix strip)
    if (lower == "minor pentatonic") {
        parent_ = ScaleType::NaturalMinor;
        pentatonic_ = true;
        return;
    }
    if (lower == "major pentatonic") {
        parent_ = ScaleType::Major;
        pentatonic_ = true;
        return;
    }

    // Check for " pentatonic" suffix in type_name
    std::string base = lower;
    if (ends_with(lower, " pentatonic")) {
        pentatonic_ = true;
        base = lower.substr(0, lower.size() - 11);
    }

    // 1) Try parent scale type (e.g. "major", "harmonic minor")
    try {
        parent_ = parse_type(base);
        mode_number_ = 1;
        return;
    } catch (...) {
        // Not a parent type — fall through to mode name lookup
    }

    // 2) Try mode name (e.g. "dorian", "altered", "phrygian dominant")
    const ModeInfo* info = find_mode_by_name(base);
    if (info) {
        parent_ = info->parent;
        mode_number_ = info->mode_number;
        return;
    }

    throw std::invalid_argument(
        "Scale: unknown scale type or mode name: " + type_name);
}

Scale::Scale(Note tonic, ScaleType parent, int mode_number, bool pentatonic)
    : tonic_(std::move(tonic)),
      parent_(parent),
      mode_number_(mode_number),
      pentatonic_(pentatonic)
{}

// ---------------------------------------------------------------------------
// Deprecated accessor
// ---------------------------------------------------------------------------

Modality Scale::modality() const {
    return pentatonic_ ? Modality::Pentatonic : Modality::Diatonic;
}

// ---------------------------------------------------------------------------
// New accessors (mode_name, quality, brightness, colors)
// ---------------------------------------------------------------------------

std::string Scale::mode_name() const {
    const ModeInfo* info = find_mode(parent_, mode_number_);
    if (info) return info->name;
    return type_to_cstr(parent_);
}

std::string Scale::quality() const {
    const ModeInfo* info = find_mode(parent_, mode_number_);
    if (info) return info->quality;
    return "major";
}

int Scale::brightness() const {
    const ModeInfo* info = find_mode(parent_, mode_number_);
    return info ? info->brightness : 0;
}

std::vector<Note> Scale::colors(const Scale& reference) const {
    compute();
    const auto ref_notes = reference.notes();

    // Build a set of reference semitones.
    std::vector<bool> ref_set(12, false);
    for (const auto& n : ref_notes) {
        ref_set[static_cast<std::size_t>(n.semitone())] = true;
    }

    // Color notes = notes in this scale that are NOT in the reference,
    // or that occupy the same scale degree but at a different semitone.
    // Simplified: notes whose semitone is not in the reference set.
    std::vector<Note> result;
    for (const auto& n : cached_notes_) {
        if (!ref_set[static_cast<std::size_t>(n.semitone())]) {
            result.push_back(n);
        }
    }
    return result;
}

std::vector<Note> Scale::colors(const std::string& reference_mode) const {
    // Build the reference scale with the same tonic.
    const ModeInfo* ref_info = find_mode_by_name(reference_mode);
    if (!ref_info) {
        // Try as a parent type name (e.g. "major", "harmonic minor")
        try {
            ScaleType ref_type = parse_type(reference_mode);
            Scale ref(tonic_.name(), ref_type);
            return colors(ref);
        } catch (...) {
            throw std::invalid_argument(
                "Scale::colors: unknown reference: " + reference_mode);
        }
    }
    // Construct a scale with the same tonic but the reference mode's intervals.
    Scale ref(tonic_, ref_info->parent, ref_info->mode_number, false);
    return colors(ref);
}

// ---------------------------------------------------------------------------
// Core computation (lazy, mutable-cached)
// ---------------------------------------------------------------------------

void Scale::compute() const {
    if (computed_) return;

    const int row = tonic_.semitone();  // chromatic index 0-11

    // 1. Rotated chromatic notes: spin the interval matrix by the tonic
    //    position so that index 0 is the tonic.
    const TypeVector& matrix = LookupData::instance().intervals().row("matrix");
    TypeVector rotated_notes = spin(matrix, static_cast<std::size_t>(row));

    // 2. Get parent's binary mask (24 positions).
    const TypeVector& parent_mask =
        LookupData::instance().scales().row(static_cast<int>(parent_));

    // 3. Compute effective mask: rotate by mode offset.
    //    Each 12-position octave is rotated independently.
    const int offset = mode_offset(parent_mask, mode_number_);

    std::vector<int> effective_mask(24);
    for (int i = 0; i < 12; ++i) {
        effective_mask[i]      = element_to_int(parent_mask[(i + offset) % 12]);
        effective_mask[i + 12] = element_to_int(parent_mask[((i + offset) % 12) + 12]);
    }

    // 4. Build the natural notes list from the first octave (positions
    //    0-11).  The second octave holds extended degrees used only
    //    internally for chord construction.
    for (std::size_t i = 0; i < 12; ++i) {
        if (effective_mask[i] == 1) {
            cached_notes_.emplace_back(element_to_string(rotated_notes[i]));
        }
    }

    // 5. Build formal notes (proper diatonic-letter spelling).
    TypeVector scale_notes_tv(24);
    for (std::size_t i = 0; i < 24; ++i) {
        if (effective_mask[i] == 1) {
            scale_notes_tv[i] = rotated_notes[i];
        } else {
            scale_notes_tv[i] = std::string("");
        }
    }

    const std::string& tonic_name = tonic_.name();
    TypeVector formal_tv =
        get_formal_notes_tv(scale_notes_tv, tonic_name, {7, 9, 11, 13});

    for (std::size_t i = 0; i < 12; ++i) {
        if (effective_mask[i] == 1) {
            std::string fn = element_to_string(formal_tv[i]);
            if (!fn.empty() && fn != "0") {
                cached_formal_.emplace_back(fn);
            }
        }
    }

    // 6. Pentatonic filter: remove avoid-note degrees.
    //    Major quality → remove degrees 4 and 7 (0-indexed: 3, 6)
    //    Minor quality → remove degrees 2 and 6 (0-indexed: 1, 5)
    if (pentatonic_ && cached_notes_.size() >= 7) {
        const ModeInfo* info = find_mode(parent_, mode_number_);
        bool is_minor = (info && std::string(info->quality) == "minor");

        auto filter_penta = [&](std::vector<Note>& notes) {
            std::vector<Note> filtered;
            for (std::size_t i = 0; i < notes.size(); ++i) {
                bool skip = is_minor ? (i == 1 || i == 5)
                                     : (i == 3 || i == 6);
                if (!skip) filtered.push_back(notes[i]);
            }
            notes = std::move(filtered);
        };

        filter_penta(cached_notes_);
        filter_penta(cached_formal_);
    }

    computed_ = true;
}

// ---------------------------------------------------------------------------
// Public accessors
// ---------------------------------------------------------------------------

std::vector<Note> Scale::notes() const {
    compute();
    // Use formal notes for correct enharmonic spelling
    return cached_formal_;
}

std::vector<Note> Scale::formal_notes() const {
    compute();
    return cached_formal_;
}

Note Scale::degree(int n) const {
    compute();
    // Use formal notes for correct enharmonic spelling
    if (n < 1 || static_cast<std::size_t>(n) > cached_formal_.size()) {
        throw std::out_of_range(
            "Scale::degree: degree " + std::to_string(n) +
            " out of range [1, " + std::to_string(cached_formal_.size()) + "]");
    }
    return cached_formal_[static_cast<std::size_t>(n - 1)];
}

Note Scale::degree(const std::vector<int>& degrees) const {
    if (degrees.empty())
        throw std::invalid_argument(
            "Scale::degree: requires at least one degree");
    compute();
    // Use formal notes for correct enharmonic spelling
    const int n = static_cast<int>(cached_formal_.size());

    int pos = degrees[0];
    if (pos < 1 || pos > n)
        throw std::out_of_range(
            "Scale::degree: degree " + std::to_string(pos) +
            " out of range [1, " + std::to_string(n) + "]");

    for (std::size_t i = 1; i < degrees.size(); ++i) {
        pos = ((pos - 1 + degrees[i] - 1) % n + n) % n + 1;
    }
    return cached_formal_[static_cast<std::size_t>(pos - 1)];
}

Note Scale::walk(int start, const std::vector<int>& steps) const {
    if (steps.empty())
        throw std::invalid_argument(
            "Scale::walk: requires at least one step");
    compute();
    // Use formal notes for correct enharmonic spelling
    const int n = static_cast<int>(cached_formal_.size());

    if (start < 1 || start > n)
        throw std::out_of_range(
            "Scale::walk: start degree " + std::to_string(start) +
            " out of range [1, " + std::to_string(n) + "]");

    int pos = start;
    for (int step : steps) {
        pos = ((pos - 1 + step - 1) % n + n) % n + 1;
    }
    return cached_formal_[static_cast<std::size_t>(pos - 1)];
}

std::size_t Scale::size() const {
    compute();
    return cached_notes_.size();
}

bool Scale::contains(const Note& note) const {
    compute();
    const int target = note.semitone();
    return std::any_of(cached_notes_.begin(), cached_notes_.end(),
                       [target](const Note& n) { return n.semitone() == target; });
}

std::optional<int> Scale::degree_of(const Note& note) const {
    compute();
    const int target = note.semitone();
    for (std::size_t i = 0; i < cached_notes_.size(); ++i) {
        if (cached_notes_[i].semitone() == target)
            return static_cast<int>(i) + 1;
    }
    return std::nullopt;
}

// ---------------------------------------------------------------------------
// Mode construction
// ---------------------------------------------------------------------------

Scale Scale::mode(int degree_number) const {
    compute();
    if (degree_number < 1 ||
        static_cast<std::size_t>(degree_number) > cached_notes_.size()) {
        throw std::out_of_range(
            "Scale::mode: degree " + std::to_string(degree_number) +
            " out of range [1, " + std::to_string(cached_notes_.size()) + "]");
    }
    const Note& target = cached_notes_[static_cast<std::size_t>(degree_number - 1)];

    // Compute the absolute mode number within the parent.
    // E.g. Dorian (mode 2) → mode(3) → absolute mode ((2-1)+(3-1)) % 7 + 1 = 4 (Lydian).
    const int total = static_cast<int>(cached_notes_.size());
    int abs_mode = ((mode_number_ - 1) + (degree_number - 1)) % total + 1;

    return Scale(Note(target.natural()), parent_, abs_mode, pentatonic_);
}

Scale Scale::mode(const std::string& mode_name) const {
    const ModeInfo* info = find_mode_by_name(mode_name);
    if (!info) {
        throw std::invalid_argument(
            "Scale::mode: unknown mode name: " + mode_name);
    }

    // Resolve NaturalMinor to Major for parent comparison.
    ScaleType my_effective = parent_;
    int my_effective_mode = mode_number_;
    if (my_effective == ScaleType::NaturalMinor) {
        my_effective = ScaleType::Major;
        my_effective_mode = ((mode_number_ + 4) % 7) + 1;
    }

    if (info->parent != my_effective) {
        throw std::invalid_argument(
            "Scale::mode: \"" + mode_name +
            "\" is not a mode of this scale's parent");
    }

    // Find the target tonic: the note at the relative degree offset.
    compute();
    const int total = static_cast<int>(cached_notes_.size());
    int idx = ((info->mode_number - my_effective_mode) % total + total) % total;
    const Note& target = cached_notes_[static_cast<std::size_t>(idx)];

    return Scale(Note(target.natural()), info->parent, info->mode_number, pentatonic_);
}

// ---------------------------------------------------------------------------
// Pentatonic
// ---------------------------------------------------------------------------

Scale Scale::pentatonic() const {
    return Scale(tonic_, parent_, mode_number_, true);
}

// ---------------------------------------------------------------------------
// Circle of fifths: signature, relative, parallel, neighbors
// ---------------------------------------------------------------------------

int Scale::signature() const {
    // Only meaningful for Major and NaturalMinor families.
    if (parent_ != ScaleType::Major && parent_ != ScaleType::NaturalMinor)
        return 0;

    // Find the Ionian (mode 1) tonic of this key.
    // For Major: the Ionian tonic is mode_offset semitones below the current tonic.
    // For NaturalMinor: NaturalMinor is structurally mode 6 of Major,
    //   so the Ionian tonic is mode_offset(parent_mask, 6+mode-1) below,
    //   but simpler: NaturalMinor mode 1 tonic + 3 semitones = relative Major tonic.
    const TypeVector& parent_mask =
        LookupData::instance().scales().row(static_cast<int>(parent_));

    int offset = mode_offset(parent_mask, mode_number_);

    int ionian_semitone;
    if (parent_ == ScaleType::NaturalMinor) {
        // NaturalMinor mode N: first find the NaturalMinor mode 1 tonic,
        // then add 3 to get the Major tonic.
        int nm_mode1_semitone = (tonic_.semitone() - offset + 12) % 12;
        ionian_semitone = (nm_mode1_semitone + 3) % 12;
    } else {
        // Major mode N: subtract the mode offset to get Ionian tonic.
        ionian_semitone = (tonic_.semitone() - offset + 12) % 12;
    }

    // Position on the circle of fifths: (semitone * 7) % 12
    int pos = (ionian_semitone * 7) % 12;
    return pos <= 6 ? pos : pos - 12;
}

Scale Scale::relative() const {
    if (parent_ == ScaleType::Major && mode_number_ == 1) {
        // Major → relative NaturalMinor (degree 6)
        compute();
        const Note& rel_tonic = cached_notes_[5];
        return Scale(rel_tonic.natural(), ScaleType::NaturalMinor);
    }
    if (parent_ == ScaleType::NaturalMinor && mode_number_ == 1) {
        // NaturalMinor → relative Major (degree 3)
        compute();
        const Note& rel_tonic = cached_notes_[2];
        return Scale(rel_tonic.natural(), ScaleType::Major);
    }
    throw std::invalid_argument(
        "Scale::relative: only defined for Major or NaturalMinor (mode 1)");
}

Scale Scale::parallel() const {
    if (parent_ == ScaleType::Major && mode_number_ == 1) {
        return Scale(tonic_.natural(), ScaleType::NaturalMinor);
    }
    if (parent_ == ScaleType::NaturalMinor && mode_number_ == 1) {
        return Scale(tonic_.natural(), ScaleType::Major);
    }
    throw std::invalid_argument(
        "Scale::parallel: only defined for Major or NaturalMinor (mode 1)");
}

std::pair<Scale, Scale> Scale::neighbors() const {
    // Subdominant: 5 semitones up (= 7 down on circle)
    Note sub_tonic = tonic_.transpose(5);
    // Dominant: 7 semitones up
    Note dom_tonic = tonic_.transpose(7);

    return {
        Scale(Note(sub_tonic.natural()), parent_, mode_number_, pentatonic_),
        Scale(Note(dom_tonic.natural()), parent_, mode_number_, pentatonic_)
    };
}

// ---------------------------------------------------------------------------
// Mask
// ---------------------------------------------------------------------------

std::vector<int> Scale::mask() const {
    const TypeVector& parent_mask =
        LookupData::instance().scales().row(static_cast<int>(parent_));

    const int offset = mode_offset(parent_mask, mode_number_);

    std::vector<int> result(24);
    for (int i = 0; i < 12; ++i) {
        result[i]      = element_to_int(parent_mask[(i + offset) % 12]);
        result[i + 12] = element_to_int(parent_mask[((i + offset) % 12) + 12]);
    }
    return result;
}

// ---------------------------------------------------------------------------
// to_string
// ---------------------------------------------------------------------------

std::string Scale::to_string() const {
    std::string type_str;
    if (mode_number_ == 1) {
        type_str = type_to_cstr(parent_);
    } else {
        const ModeInfo* info = find_mode(parent_, mode_number_);
        type_str = info ? info->name : type_to_cstr(parent_);
    }
    if (pentatonic_) type_str += " pentatonic";

    return std::string("Scale(\"") + tonic_.name() + "\", \"" +
           type_str + "\")";
}

} // namespace gingo
