// Gingo — Music Theory Library
// Field: harmonic field — the set of chords built from a scale.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "chord.hpp"
#include "harmonic_function.hpp"
#include "note.hpp"
#include "note_context.hpp"
#include "scale.hpp"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace gingo {

// Note: HarmonicFunction, harmonic_function_name, harmonic_function_short
// are defined in harmonic_function.hpp

/// Information about a borrowed chord — a chord found in a parallel
/// harmonic field (e.g. from the parallel minor when analyzing major).
struct BorrowedInfo {
    std::string scale_type;       ///< "NaturalMinor", "HarmonicMinor", etc.
    int degree;                   ///< 1-indexed degree in that field
    HarmonicFunction function;    ///< T/S/D in that field
    std::string role;             ///< role description in that field
};

/// Information about a pivot field — a harmonic field where both chords
/// are diatonic (coexist).
struct PivotInfo {
    std::string tonic;            ///< field tonic note name
    std::string scale_type;       ///< field type name
    int degree_a;                 ///< chord a's degree in this field
    int degree_b;                 ///< chord b's degree in this field
};

/// Result of comparing two chords within the context of a harmonic field.
///
/// Provides degree information, functional analysis, progression validity,
/// borrowed chord detection, pivot analysis, and chromatic relationships.
struct FieldComparison {
    /// Degrees of each chord (nullopt if not in field).
    std::optional<int> degree_a;
    std::optional<int> degree_b;

    /// Harmonic functions of each chord.
    std::optional<HarmonicFunction> function_a;
    std::optional<HarmonicFunction> function_b;

    /// Functional roles of each chord.
    std::optional<std::string> role_a;
    std::optional<std::string> role_b;

    /// Distance in degrees (nullopt if either chord is not in the field).
    std::optional<int> degree_distance;

    /// Whether both chords belong to the same functional group.
    std::optional<bool> same_function;

    /// Whether the two chords form a relative pair.
    bool relative;

    /// Whether a valid progression path exists from a to b.
    bool progression;

    /// Root motion type between the two chords (based on the interval between
    /// roots within the diatonic context).
    /// Values: "" (if either chord is not diatonic),
    /// "descending_fifth", "ascending_fifth",
    /// "descending_third", "ascending_third",
    /// "descending_step", "ascending_step",
    /// "tritone", "unison".
    std::string root_motion;

    /// Secondary dominant relationship:
    ///   "" | "a_is_V7_of_b" | "b_is_V7_of_a"
    std::string secondary_dominant;

    /// Applied diminished chord relationship (leading-tone diminished):
    ///   "" | "a_is_viidim_of_b" | "b_is_viidim_of_a"
    /// Detected when a chord is diminished (type "dim" or "m7(b5)")
    /// and its root is 1 semitone below the other chord's root.
    std::string applied_diminished;

    /// Whether each chord is diatonic in this field.
    bool diatonic_a;
    bool diatonic_b;

    /// Borrowed chord info (from parallel fields).
    std::optional<BorrowedInfo> borrowed_a;
    std::optional<BorrowedInfo> borrowed_b;

    /// Pivot fields where both chords coexist.
    std::vector<PivotInfo> pivot;

    /// Whether the pair forms a tritone substitution.
    bool tritone_sub;

    /// Chromatic mediant relationship: "" | "upper" | "lower"
    std::string chromatic_mediant;

    /// Foreign notes — notes not in the field's scale.
    std::vector<Note> foreign_a;
    std::vector<Note> foreign_b;
};

/// Represents a harmonic field — the diatonic chords built from each degree
/// of a scale.
///
/// A harmonic field takes a tonic and scale type, then constructs the chord
/// that naturally occurs on each scale degree.
///
/// Examples:
///   Field f("C", ScaleType::Major);
///   f.chords();           // {CM, Dm, Em, FM, GM, Am, Bdim}
///   f.chord(5);            // Chord("GM")
///   f.sevenths();          // {CM7, Dm7, Em7, FM7, G7, Am7, Bm7(b5)}
class Field {
public:
    Field(const std::string& tonic, ScaleType type);
    Field(const std::string& tonic, const std::string& type_name);

    /// The tonic note.
    Note tonic() const { return scale_.tonic(); }

    /// The underlying scale.
    const Scale& scale() const { return scale_; }

    /// Triads (3-note chords) for each scale degree.
    std::vector<Chord> chords() const;

    /// Seventh chords (4-note chords) for each scale degree.
    std::vector<Chord> sevenths() const;

    /// Get the triad chord at a specific degree (1-indexed).
    Chord chord(int degree) const;

    /// Get the seventh chord at a specific degree (1-indexed).
    Chord seventh(int degree) const;

    /// Applied chord (tonicization): build a chord as if a scale degree were
    /// a temporary tonic.
    ///
    /// Three forms:
    ///   applied("V7", 2)       — explicit quality, target by degree number
    ///   applied("V7", "IIm")   — explicit quality, target as Roman numeral
    ///   applied(5, 2)          — numeric shorthand, returns seventh chord
    ///
    /// The Roman numeral grammar is: [accidental] numeral quality
    ///   accidental = 'b' (flat) | '#' (sharp) — prefix, before the numeral
    ///   numeral    = I | II | III | IV | V | VI | VII
    ///   quality    = remaining characters (e.g. "7", "m7", "m7(b5)")
    ///
    /// Examples (Field("C", Major)):
    ///   applied("V7", 2)          → A7   (dominant of D)
    ///   applied("V7", "V")        → D7   (dominant of G)
    ///   applied("IIm7(b5)", 5)    → Am7(b5)
    ///   applied("bVII7", 5)       → F7   (flattened VII of G)
    ///   applied(5, 2)             → A7   (seventh at V of D major)
    Chord applied(const std::string& function, int target_degree) const;
    Chord applied(const std::string& function, const std::string& target) const;
    Chord applied(int function_degree, int target_degree) const;

    /// Harmonic function of a degree or chord.
    ///
    /// By degree (1-indexed): always returns a value, throws for out-of-range.
    /// By chord name or object: returns nullopt if the chord's root doesn't
    /// match any degree in the field (enharmonic comparison by semitone).
    ///
    /// Examples (Field("C", Major)):
    ///   function(1)     → HarmonicFunction::Tonic
    ///   function(5)     → HarmonicFunction::Dominant
    ///   function("FM")  → HarmonicFunction::Subdominant
    ///   function("F#M") → nullopt
    HarmonicFunction function(int degree) const;
    std::optional<HarmonicFunction> function(const std::string& chord_name) const;
    std::optional<HarmonicFunction> function(const Chord& chord) const;

    /// Role of a degree or chord within its harmonic function group.
    ///
    /// Returns a descriptive string: "primary", "relative of I",
    /// "relative of IV", "relative of V", or "transitive".
    ///
    /// By chord name or object: returns nullopt if not in the field.
    ///
    /// Examples (Field("C", Major)):
    ///   role(1)    → "primary"
    ///   role(6)    → "relative of I"
    ///   role("Am") → "relative of I"
    std::string role(int degree) const;
    std::optional<std::string> role(const std::string& chord_name) const;
    std::optional<std::string> role(const Chord& chord) const;

    /// Compare two chords within the context of this harmonic field.
    FieldComparison compare(const Chord& a, const Chord& b) const;

    /// Get per-note harmonic context for a given note.
    ///
    /// Returns NoteContext with scale degree, interval from tonic,
    /// harmonic function, and inScale flag.
    ///
    /// Examples (Field("C", Major)):
    ///   noteContext(Note("E"))  → degree=3, function=Tonic, interval=M3, inScale=true
    ///   noteContext(Note("F"))  → degree=4, function=Subdominant, interval=P4, inScale=true
    ///   noteContext(Note("C#")) → degree=0, function=Tonic, interval=m2, inScale=false
    NoteContext noteContext(const Note& note) const;

    /// Key signature (delegates to the underlying scale).
    int signature() const;

    /// Relative harmonic field (major ↔ natural minor).
    Field relative() const;

    /// Parallel harmonic field (same tonic, opposite quality).
    Field parallel() const;

    /// Adjacent harmonic fields on the circle of fifths.
    std::pair<Field, Field> neighbors() const;

    /// Number of degrees (typically 7).
    std::size_t size() const;

    std::string to_string() const;

private:
    explicit Field(Scale scale);

    Scale scale_;

    std::vector<Chord> build_chords(
        const std::vector<std::string>& degree_filter) const;

    /// Find which degree (1-indexed) a chord occupies by root semitone.
    /// Returns nullopt if no degree matches.
    std::optional<int> find_degree(const Chord& chord) const;
};

} // namespace gingo
