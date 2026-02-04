// Gingo — Music Theory Library
// Implementation of the Chord class.
//
// SPDX-License-Identifier: MIT

#include "gingo/chord.hpp"
#include "gingo/internal/lookup_data.hpp"
#include "gingo/internal/data_ops.hpp"
#include "gingo/internal/notation_utils.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <numeric>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

Chord::Chord(const std::string& name)
    : name_(name)
    , root_(Note::extract_root(name))
    , type_(Note::extract_type(name))
    , computed_(false)
{}

// ---------------------------------------------------------------------------
// Lazy computation
// ---------------------------------------------------------------------------

void Chord::compute() const {
    if (computed_) return;

    // 1. Get the chord formula (interval labels) from chord_formulas table.
    const auto& formulas = internal::LookupData::instance().chord_formulas();
    if (!formulas.has_row(type_))
        throw std::invalid_argument("Unknown chord type: " + type_);
    const internal::TypeVector& formula = formulas.row(type_);

    // 2. Get the ilabel row from the interval table.
    const internal::TypeVector& ilabels =
        internal::LookupData::instance().intervals().row("ilabel");

    // 3. Get chromatic notes and rotate to the root.
    const internal::TypeVector& all_notes =
        internal::LookupData::instance().notes().row("note");
    std::string natural_root = Note::to_natural(Note::extract_root(name_));
    internal::TypeVector rotated =
        internal::rotate(all_notes, internal::TypeElement(natural_root), 24);

    // 4. For each interval in the formula, find its position in ilabels,
    //    then look up the note at that position in the rotated chromatic.
    for (const auto& iv : formula) {
        std::string label = internal::element_to_string(iv);
        for (std::size_t j = 0; j < ilabels.size(); ++j) {
            if (internal::element_to_string(ilabels[j]) == label) {
                cached_notes_.emplace_back(internal::element_to_string(rotated[j]));
                cached_intervals_.emplace_back(label);
                break;
            }
        }
    }

    // 5. Compute formal notes using notation utilities.
    std::string root_note  = Note::extract_root(name_);
    std::string note_sound = Note::extract_sound(name_);

    // Build degree list (0-indexed) from interval labels.
    const internal::TypeVector& degree_row =
        internal::LookupData::instance().intervals().row("degree");
    std::vector<int> chord_degrees;
    for (const auto& iv : formula) {
        std::string label = internal::element_to_string(iv);
        for (std::size_t j = 0; j < ilabels.size(); ++j) {
            if (internal::element_to_string(ilabels[j]) == label) {
                chord_degrees.push_back(internal::element_to_int(degree_row[j]) - 1);
                break;
            }
        }
    }

    // Build remove_indices for formal note computation: positions in the
    // rotated diatonic letter sequence that do NOT correspond to a chord degree.
    std::vector<std::string> diatonic = {"A", "B", "C", "D", "E", "F", "G"};
    std::vector<std::string> rotated_sounds =
        internal::rotate_remove(diatonic, note_sound, 14, {});

    std::vector<int> remove_indices;
    for (int i = 0; i < static_cast<int>(rotated_sounds.size()); ++i) {
        bool keep = false;
        for (int d : chord_degrees) {
            if (d == i) { keep = true; break; }
        }
        if (!keep) remove_indices.push_back(i);
    }

    // Convert natural notes to formal (proper diatonic) notes.
    internal::TypeVector natural_tv;
    natural_tv.reserve(cached_notes_.size());
    for (const auto& n : cached_notes_) {
        natural_tv.emplace_back(n.natural());
    }
    internal::TypeVector formal_tv =
        internal::get_formal_notes_tv(natural_tv, root_note, remove_indices);
    cached_formal_.reserve(formal_tv.size());
    for (const auto& e : formal_tv) {
        cached_formal_.emplace_back(internal::element_to_string(e));
    }

    computed_ = true;
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

std::vector<Note> Chord::notes() const {
    compute();
    // Use formal notes for correct enharmonic spelling
    return cached_formal_;
}

std::vector<Note> Chord::formal_notes() const {
    compute();
    return cached_formal_;
}

std::vector<Interval> Chord::intervals() const {
    compute();
    return cached_intervals_;
}

std::vector<std::string> Chord::interval_labels() const {
    compute();
    std::vector<std::string> labels;
    labels.reserve(cached_intervals_.size());
    for (const auto& iv : cached_intervals_) {
        labels.push_back(iv.label());
    }
    return labels;
}

std::size_t Chord::size() const {
    compute();
    return cached_notes_.size();
}

bool Chord::contains(const Note& note) const {
    compute();
    int st = note.semitone();
    return std::any_of(cached_notes_.begin(), cached_notes_.end(),
                       [st](const Note& n) { return n.semitone() == st; });
}

// ---------------------------------------------------------------------------
// Identification (reverse lookup)
// ---------------------------------------------------------------------------

Chord Chord::identify(const std::vector<Note>& notes) {
    if (notes.empty()) {
        throw std::invalid_argument("Cannot identify chord from empty note list");
    }

    // Preserve the original root name for the result.
    std::string original_root = notes[0].name();
    std::string natural_root  = Note::to_natural(original_root);

    // Convert all notes to their natural (sharp-based) form.
    std::vector<std::string> natural_names;
    natural_names.reserve(notes.size());
    for (const auto& n : notes) {
        natural_names.push_back(n.natural());
    }

    // Rotate chromatic scale from root, covering two octaves.
    const internal::TypeVector& all_notes =
        internal::LookupData::instance().notes().row("note");
    internal::TypeVector rotated =
        internal::rotate(all_notes, internal::TypeElement(natural_root), 24);

    // Get the interval label reference row.
    const internal::TypeVector& ilabels =
        internal::LookupData::instance().intervals().row("ilabel");

    // For each note, find it in the rotated chromatic and record the
    // corresponding interval label.  Use a running start index so that
    // notes are matched in ascending order through the two-octave range.
    internal::TypeVector detected_intervals;
    std::size_t start_idx = 0;
    for (const auto& name : natural_names) {
        for (std::size_t j = start_idx; j < rotated.size(); ++j) {
            if (internal::element_to_string(rotated[j]) == name) {
                detected_intervals.push_back(ilabels[j]);
                start_idx = j + 1;
                break;
            }
        }
    }

    // Compare detected intervals against every chord formula by numeric index
    // (deterministic order) rather than iterating the unordered name map.
    const internal::Table& formulas =
        internal::LookupData::instance().chord_formulas();
    for (std::size_t fi = 0; fi < formulas.row_count(); ++fi) {
        const internal::TypeVector& formula = formulas.data[fi];
        if (detected_intervals.size() != formula.size()) continue;

        bool match = true;
        for (std::size_t i = 0; i < detected_intervals.size(); ++i) {
            if (internal::element_to_string(detected_intervals[i]) !=
                internal::element_to_string(formula[i])) {
                match = false;
                break;
            }
        }
        if (match) {
            // Use the canonical (shortest) name for this formula index.
            std::string type_name =
                formulas.row_name_by_index(static_cast<int>(fi));
            return Chord(original_root + type_name);
        }
    }

    throw std::invalid_argument("Chord not found");
}

Chord Chord::identify(const std::vector<std::string>& note_names) {
    std::vector<Note> note_objects;
    note_objects.reserve(note_names.size());
    for (const auto& name : note_names) {
        note_objects.emplace_back(name);
    }
    return identify(note_objects);
}

// ---------------------------------------------------------------------------
// Detailed comparison
// ---------------------------------------------------------------------------

namespace {

/// Build a set of semitone offsets from the root for neo-Riemannian detection.
std::set<int> interval_set_from_root(const std::vector<Note>& notes) {
    std::set<int> result;
    int root_st = notes[0].semitone();
    for (const auto& n : notes) {
        result.insert((n.semitone() - root_st + 12) % 12);
    }
    return result;
}

/// Compute minimum semitone distance on the chromatic circle.
int chromatic_distance(int a, int b) {
    int d = std::abs(a - b);
    return std::min(d, 12 - d);
}

/// Compute the Forte interval-class vector for a set of notes.
/// Returns a 6-element vector: index 0 = ic1, index 5 = ic6.
std::vector<int> compute_interval_vector(const std::vector<Note>& notes) {
    std::vector<int> vec(6, 0);
    for (std::size_t i = 0; i < notes.size(); ++i) {
        for (std::size_t j = i + 1; j < notes.size(); ++j) {
            int ic = chromatic_distance(notes[i].semitone(), notes[j].semitone());
            if (ic >= 1 && ic <= 6)
                vec[static_cast<std::size_t>(ic - 1)]++;
        }
    }
    return vec;
}

/// Plomp-Levelt roughness between two frequencies (simplified Sethares model).
double plomp_levelt_roughness(double f1, double f2) {
    if (f1 > f2) std::swap(f1, f2);
    double s = 0.24 / (0.021 * f1 + 19.0);
    double d = f2 - f1;
    return std::exp(-3.5 * s * d) - std::exp(-5.75 * s * d);
}

/// Total dissonance score for a chord (sum of roughness for all note pairs).
double compute_dissonance(const std::vector<Note>& notes) {
    double total = 0.0;
    for (std::size_t i = 0; i < notes.size(); ++i) {
        double fi = notes[i].frequency(4);
        for (std::size_t j = i + 1; j < notes.size(); ++j) {
            double fj = notes[j].frequency(4);
            total += plomp_levelt_roughness(fi, fj);
        }
    }
    return total;
}

/// Triad identity for neo-Riemannian transformations.
struct TriadInfo {
    int root_st;
    bool is_major;
};

/// Apply a single neo-Riemannian transformation (P, L, or R) to a triad.
std::optional<TriadInfo> apply_neo_riemannian(TriadInfo t, char op) {
    if (t.is_major) {
        switch (op) {
            case 'P': return TriadInfo{t.root_st, false};
            case 'L': return TriadInfo{(t.root_st + 4) % 12, false};
            case 'R': return TriadInfo{(t.root_st + 9) % 12, false};
            default:  return std::nullopt;
        }
    } else {
        switch (op) {
            case 'P': return TriadInfo{t.root_st, true};
            case 'L': return TriadInfo{(t.root_st + 8) % 12, true};
            case 'R': return TriadInfo{(t.root_st + 3) % 12, true};
            default:  return std::nullopt;
        }
    }
}

/// Build a pitch-class set from root + quality.
std::set<int> triad_pc_set(int root_st, bool is_major) {
    if (is_major)
        return {root_st, (root_st + 4) % 12, (root_st + 7) % 12};
    else
        return {root_st, (root_st + 3) % 12, (root_st + 7) % 12};
}

} // anonymous namespace

ChordComparison Chord::compare(const Chord& other) const {
    ChordComparison r;

    auto a_notes = notes();
    auto b_notes = other.notes();

    // Build semitone sets
    std::set<int> a_set, b_set;
    for (const auto& n : a_notes) a_set.insert(n.semitone());
    for (const auto& n : b_notes) b_set.insert(n.semitone());

    // 1-2. Common and exclusive notes
    for (const auto& n : a_notes) {
        if (b_set.count(n.semitone()))
            r.common_notes.push_back(n);
        else
            r.exclusive_a.push_back(n);
    }
    for (const auto& n : b_notes) {
        if (!a_set.count(n.semitone()))
            r.exclusive_b.push_back(n);
    }

    // 3-4. Root distance and direction
    int a_st = root().semitone();
    int b_st = other.root().semitone();
    int diff = b_st - a_st;
    if (diff > 6)  diff -= 12;
    if (diff < -6) diff += 12;
    r.root_direction = diff;
    r.root_distance  = std::abs(diff);

    // 5-6. Quality and size
    r.same_quality = (type() == other.type());
    r.same_size    = (size() == other.size());

    // 7. Common intervals
    auto a_labels = interval_labels();
    auto b_labels = other.interval_labels();
    std::set<std::string> b_label_set(b_labels.begin(), b_labels.end());
    for (const auto& l : a_labels) {
        if (b_label_set.count(l))
            r.common_intervals.push_back(l);
    }

    // 8. Enharmonic equivalence
    std::vector<int> a_sorted(a_set.begin(), a_set.end());
    std::vector<int> b_sorted(b_set.begin(), b_set.end());
    r.enharmonic = (a_sorted == b_sorted);

    // 9. Subset relationship
    bool a_in_b = std::all_of(a_set.begin(), a_set.end(),
                              [&](int s) { return b_set.count(s) > 0; });
    bool b_in_a = std::all_of(b_set.begin(), b_set.end(),
                              [&](int s) { return a_set.count(s) > 0; });
    if (a_in_b && b_in_a)       r.subset = "equal";
    else if (a_in_b)            r.subset = "a_subset_of_b";
    else if (b_in_a)            r.subset = "b_subset_of_a";
    else                        r.subset = "";

    // 10. Voice leading (brute-force permutation for same-size chords)
    if (a_notes.size() != b_notes.size()) {
        r.voice_leading = -1;
    } else {
        std::vector<int> a_st_vec, b_st_vec;
        for (const auto& n : a_notes) a_st_vec.push_back(n.semitone());
        for (const auto& n : b_notes) b_st_vec.push_back(n.semitone());

        std::sort(b_st_vec.begin(), b_st_vec.end());
        int min_total = std::numeric_limits<int>::max();
        do {
            int total = 0;
            for (std::size_t i = 0; i < a_st_vec.size(); ++i) {
                total += chromatic_distance(a_st_vec[i], b_st_vec[i]);
            }
            min_total = std::min(min_total, total);
        } while (std::next_permutation(b_st_vec.begin(), b_st_vec.end()));

        r.voice_leading = min_total;
    }

    // 11. Neo-Riemannian transformation (P, L, R and 2-step compositions) — triads only
    r.transformation = "";
    if (a_notes.size() == 3 && b_notes.size() == 3) {
        auto a_ivs = interval_set_from_root(a_notes);
        auto b_ivs = interval_set_from_root(b_notes);

        bool a_major = (a_ivs == std::set<int>{0, 4, 7});
        bool a_minor = (a_ivs == std::set<int>{0, 3, 7});
        bool b_major = (b_ivs == std::set<int>{0, 4, 7});
        bool b_minor = (b_ivs == std::set<int>{0, 3, 7});

        if ((a_major || a_minor) && (b_major || b_minor)) {
            // Single transformation (one chord major, one minor)
            if ((a_major && b_minor) || (a_minor && b_major)) {
                int rd = r.root_distance;
                if (rd == 0)      r.transformation = "P";
                else if (rd == 3) r.transformation = "R";
                else if (rd == 4) r.transformation = "L";
            }

            // Two-step composition if single did not match.
            // Skip identity involutions (PP, LL, RR) — they are trivial.
            if (r.transformation.empty() && !r.enharmonic) {
                TriadInfo a_info{a_st, a_major};
                std::set<int> b_pc(b_set.begin(), b_set.end());
                bool found = false;
                const char ops[] = {'P', 'L', 'R'};
                for (char op1 : ops) {
                    if (found) break;
                    auto mid = apply_neo_riemannian(a_info, op1);
                    if (!mid) continue;
                    for (char op2 : ops) {
                        if (op1 == op2) continue;  // skip involutions
                        auto result = apply_neo_riemannian(*mid, op2);
                        if (!result) continue;
                        if (triad_pc_set(result->root_st, result->is_major) == b_pc) {
                            r.transformation = std::string(1, op1) + std::string(1, op2);
                            found = true;
                            break;
                        }
                    }
                }
            }
        }
    }

    // 12. Inversion (same pitch class set, different root)
    r.inversion = (a_sorted == b_sorted) &&
                  (root().semitone() != other.root().semitone());

    // 13. Interval vectors (Forte set theory)
    r.interval_vector_a = compute_interval_vector(a_notes);
    r.interval_vector_b = compute_interval_vector(b_notes);
    r.same_interval_vector = (r.interval_vector_a == r.interval_vector_b);

    // 14. Transposition detection (Lewin T_n)
    r.transposition = -1;
    if (a_sorted.size() == b_sorted.size()) {
        for (int n = 0; n < 12; ++n) {
            std::vector<int> transposed;
            transposed.reserve(a_sorted.size());
            for (int pc : a_sorted)
                transposed.push_back((pc + n) % 12);
            std::sort(transposed.begin(), transposed.end());
            if (transposed == b_sorted) {
                r.transposition = n;
                break;
            }
        }
    }

    // 15. Dissonance scores (Sethares / Plomp-Levelt model)
    r.dissonance_a = compute_dissonance(a_notes);
    r.dissonance_b = compute_dissonance(b_notes);

    return r;
}

// ---------------------------------------------------------------------------
// Transposition
// ---------------------------------------------------------------------------

Chord Chord::transpose(int semitones) const {
    Note new_root = root_.transpose(semitones);
    return Chord(new_root.name() + type_);
}

// ---------------------------------------------------------------------------
// Comparison & display
// ---------------------------------------------------------------------------

bool Chord::operator==(const Chord& other) const {
    return name_ == other.name_;
}

std::string Chord::to_string() const {
    return "Chord(\"" + name_ + "\")";
}

} // namespace gingo
