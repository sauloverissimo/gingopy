// Gingo — Music Theory Library
// Implementation of the Tree class.
//
// SPDX-License-Identifier: MIT

#include "gingo/tree.hpp"
#include "gingo/field.hpp"
#include "gingo/internal/lookup_data.hpp"
#include "gingo/internal/lookup_tree.hpp"
#include "gingo/internal/data_ops.hpp"
#include "gingo/internal/notation_utils.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace gingo {

// ---------------------------------------------------------------------------
// Anonymous helpers
// ---------------------------------------------------------------------------

namespace {

/// Map a ScaleType enum value to its canonical display name.
const char* scale_type_label(ScaleType t) {
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

/// Parse an applied chord notation "X / Y" into function and target.
/// Returns {function, target} or {"", ""} if not applied format.
std::pair<std::string, std::string> parse_applied(const std::string& name) {
    size_t pos = name.find(" / ");
    if (pos == std::string::npos)
        return {"", ""};

    std::string function = name.substr(0, pos);
    std::string target = name.substr(pos + 3);
    return {function, target};
}

/// Resolve an applied chord using Field.applied().
/// Examples: "IIm / IV", "V7 / IIm", "SUBV7 / IV"
Chord resolve_applied(const std::string& name, const Field& field) {
    auto [function, target] = parse_applied(name);
    if (function.empty())
        return Chord("C");  // Invalid, should not happen

    return field.applied(function, target);
}

/// Resolve a diminished passing chord.
/// Examples: "I°" → C°, "#I°" → C#°, "bIII°" → Eb°, "II#dim" → D#dim
Chord resolve_diminished(const std::string& name, const Field& field) {
    // Remove the diminished suffix (either '°' or "dim")
    std::string roman;
    // Check for '°' symbol (UTF-8: 0xC2 0xB0)
    if (name.size() >= 2 &&
        static_cast<unsigned char>(name[name.size()-2]) == 0xC2 &&
        static_cast<unsigned char>(name[name.size()-1]) == 0xB0) {
        roman = name.substr(0, name.length() - 2);  // Remove '°' (2 bytes)
    } else if (name.size() >= 3 && name.substr(name.size() - 3) == "dim") {
        roman = name.substr(0, name.size() - 3);  // Remove "dim"
    } else {
        // Invalid format
        return Chord("C");
    }

    // Parse accidentals - can be before OR after the roman numeral
    // Examples: "#II", "II#", "bIII"
    int accidental = 0;
    size_t start = 0;
    std::string degree_part;

    // Check for accidental BEFORE numeral (#II, bIII)
    if (!roman.empty() && roman[0] == 'b') {
        accidental = -1;
        start = 1;
        degree_part = roman.substr(start);
    } else if (!roman.empty() && roman[0] == '#') {
        accidental = 1;
        start = 1;
        degree_part = roman.substr(start);
    } else {
        // Check for accidental AFTER numeral (II#, IV#)
        degree_part = roman;
        if (!roman.empty() && roman.back() == '#') {
            accidental = 1;
            degree_part = roman.substr(0, roman.length() - 1);
        } else if (!roman.empty() && roman.back() == 'b') {
            accidental = -1;
            degree_part = roman.substr(0, roman.length() - 1);
        }
    }

    // Get the root note based on the degree
    Note root = field.tonic();  // Default

    if (degree_part == "I") {
        root = field.tonic();
    } else if (degree_part == "II") {
        root = field.scale().degree(2);
    } else if (degree_part == "III") {
        root = field.scale().degree(3);
    } else if (degree_part == "IV") {
        root = field.scale().degree(4);
    } else if (degree_part == "V") {
        root = field.scale().degree(5);
    } else if (degree_part == "VI") {
        root = field.scale().degree(6);
    } else if (degree_part == "VII") {
        root = field.scale().degree(7);
    }

    // Apply accidental
    if (accidental != 0) {
        root = root.transpose(accidental);
    }

    // Build diminished chord (dim triad)
    // Use "dim" suffix instead of "°" for better compatibility
    return Chord(root.name() + "dim");
}

/// Resolve a tritone substitute chord.
/// Examples: "SUBV7" → tritone sub of V7, "SUBV7 / IV" → tritone sub of V7/IV
Chord resolve_tritone_sub(const std::string& name, const Field& field) {
    // Check if it's an applied tritone sub
    if (name.find(" / ") != std::string::npos) {
        // "SUBV7 / IV" → get V7/IV first, then apply tritone
        size_t pos = name.find(" / ");
        std::string target = name.substr(pos + 3);

        // Get the dominant of the target
        Chord dominant = field.applied("V7", target);

        // Tritone substitute is 6 semitones away
        Note tritone_root = dominant.root().transpose(6);
        return Chord(tritone_root.name() + "7");
    }

    // Simple "SUBV7" → tritone sub of V7
    Chord dominant = field.seventh(5);
    Note tritone_root = dominant.root().transpose(6);
    return Chord(tritone_root.name() + "7");
}

/// Resolve a diatonic chord (graus diretos do campo harmônico).
/// Examples: "I", "IIm", "IV", "VIm", "bVI", "bVII", "Im", "IVm"
Chord resolve_diatonic(const std::string& name, const Field& field) {
    // Parse accidentals
    int accidental = 0;
    size_t start = 0;

    if (!name.empty() && name[0] == 'b') {
        accidental = -1;
        start = 1;
    } else if (!name.empty() && name[0] == '#') {
        accidental = 1;
        start = 1;
    }

    // Extract degree
    std::string rest = name.substr(start);
    int degree = 0;

    // Parse roman numeral
    if (rest.substr(0, 3) == "VII") {
        degree = 7;
        rest = rest.substr(3);
    } else if (rest.substr(0, 2) == "VI") {
        degree = 6;
        rest = rest.substr(2);
    } else if (rest.substr(0, 2) == "IV") {
        degree = 4;
        rest = rest.substr(2);
    } else if (rest.substr(0, 3) == "III") {
        degree = 3;
        rest = rest.substr(3);
    } else if (rest.substr(0, 2) == "II") {
        degree = 2;
        rest = rest.substr(2);
    } else if (rest.substr(0, 1) == "V") {
        degree = 5;
        rest = rest.substr(1);
    } else if (rest.substr(0, 1) == "I") {
        degree = 1;
        rest = rest.substr(1);
    }

    // Check for explicit quality indicators in the remaining part
    bool has_minor = (rest.find("m") != std::string::npos && rest.find("dim") == std::string::npos);
    bool has_seventh = (rest.find("7") != std::string::npos);

    // Modal interchange (borrowed chords): use parallel minor field
    // IVm, bVI, bVII in major → borrow from parallel minor
    if ((has_minor && degree == 4) || (accidental == -1 && (degree == 6 || degree == 7))) {
        // Get parallel minor field
        Field parallel_minor(field.tonic().name(), ScaleType::NaturalMinor);
        Chord borrowed = has_seventh ? parallel_minor.seventh(degree) : parallel_minor.chord(degree);
        return borrowed;
    }

    // Get root note from scale degree
    Note root = field.scale().degree(degree);

    // Apply accidental if needed (for other borrowed chords)
    if (accidental != 0) {
        root = root.transpose(accidental);
    }

    // Build chord name
    std::string chord_name = root.name();

    if (has_minor && has_seventh) {
        chord_name += "m7";
    } else if (has_minor) {
        chord_name += "m";
    } else if (has_seventh) {
        // Get quality from diatonic field
        Chord diatonic = field.seventh(degree);
        std::string original = diatonic.name();
        size_t pos = original.find_first_not_of("ABCDEFG#b");
        if (pos != std::string::npos) {
            chord_name = root.name() + original.substr(pos);
        } else {
            chord_name += "7M";
        }
    } else {
        // Get quality from diatonic field
        Chord diatonic = field.chord(degree);
        std::string original = diatonic.name();
        size_t pos = original.find_first_not_of("ABCDEFG#b");
        if (pos != std::string::npos) {
            chord_name = root.name() + original.substr(pos);
        } else {
            chord_name += "M";
        }
    }

    return Chord(chord_name);
}

/// Resolve a branch name to a Chord using Field.applied() and helpers.
Chord resolve_branch_from_name(const std::string& branch_name,
                                const std::string& tonic,
                                ScaleType scale_type) {
    Field field(tonic, scale_type);

    // 1. Applied chords: "X / Y"
    if (branch_name.find(" / ") != std::string::npos) {
        return resolve_applied(branch_name, field);
    }

    // 2. Diminished chords: "I°", "#I°", "bIII°", "Idim", "#Idim", "bIIIdim"
    // Check for '°' symbol (UTF-8: 0xC2 0xB0) or "dim" suffix
    bool has_degree_symbol = (branch_name.size() >= 2 &&
                              static_cast<unsigned char>(branch_name[branch_name.size()-2]) == 0xC2 &&
                              static_cast<unsigned char>(branch_name[branch_name.size()-1]) == 0xB0);
    bool has_dim_suffix = (branch_name.size() >= 3 &&
                           branch_name.substr(branch_name.size() - 3) == "dim");

    if (has_degree_symbol || has_dim_suffix) {
        return resolve_diminished(branch_name, field);
    }

    // 3. Tritone substitutes: "SUBV7", "SUBV7 / IV"
    if (branch_name.find("SUBV") != std::string::npos) {
        return resolve_tritone_sub(branch_name, field);
    }

    // 4. Diatonic chords: "I", "IIm", "IV", "VIm", etc.
    return resolve_diatonic(branch_name, field);
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Tree::Tree(const std::string& tonic, ScaleType type)
    : tonic_(tonic)
    , type_(type)
    , tonic_index_(tonic_.semitone())
{}

Tree::Tree(const std::string& tonic, const std::string& type_name)
    : tonic_(tonic)
    , type_(Scale::parse_type(type_name))
    , tonic_index_(tonic_.semitone())
{}

// ---------------------------------------------------------------------------
// branches()
// ---------------------------------------------------------------------------

std::vector<std::string> Tree::branches() const
{
    using namespace internal;

    const auto& tb       = LookupTree::instance().branches();
    const int   scale_id = static_cast<int>(type_);

    // Collect unique branch names while preserving insertion order.
    std::vector<std::string> result;
    std::set<std::string>    seen;

    for (std::size_t i = 0; i < tb.row_count(); ++i) {
        const auto& row = tb.row(static_cast<int>(i));

        if (element_to_int(row[0]) != scale_id)
            continue;

        std::string name = element_to_string(row[1]);  // Now branch is at index 1
        if (!name.empty() && seen.insert(name).second)
            result.push_back(name);
    }

    return result;
}

// ---------------------------------------------------------------------------
// paths()
// ---------------------------------------------------------------------------

std::vector<HarmonicPath> Tree::paths(const std::string& branch_origin) const
{
    using namespace internal;

    const auto& chord_data = LookupData::instance();
    const auto& tree_data  = LookupTree::instance();
    const auto& tb         = tree_data.branches();
    const auto& tp         = tree_data.paths();
    const int   scale_id   = static_cast<int>(type_);

    std::vector<HarmonicPath> result;
    int path_id = 0;

    // -- Helper: locate the first tree-branch row matching (scale, name). ----

    auto find_branch_row =
        [&](const std::string& name) -> const TypeVector* {
            for (std::size_t i = 0; i < tb.row_count(); ++i) {
                const auto& row = tb.row(static_cast<int>(i));
                if (element_to_int(row[0]) == scale_id &&
                    element_to_string(row[1]) == name)  // Now branch is at index 1
                {
                    return &row;
                }
            }
            return nullptr;
        };

    // -- Helper: resolve a branch row into a HarmonicPath and append it. -----

    auto emit_path =
        [&](const std::string& name) -> bool {
            // Build the Chord object using the new resolve_branch_from_name.
            Chord chord_obj("C");   // placeholder — immediately reassigned
            try {
                chord_obj = resolve_branch_from_name(name, tonic_.name(), type_);
            } catch (...) {
                return false;
            }

            // Get interval labels from the chord's intervals.
            std::vector<std::string> interval_labels;
            try {
                auto intervals = chord_obj.intervals();
                interval_labels.reserve(intervals.size());
                for (const auto& interval : intervals) {
                    interval_labels.push_back(interval.label());
                }
            } catch (...) {
                // Unable to resolve intervals — leave empty.
            }

            // Formal (diatonic-correct) note names.
            std::vector<std::string> note_names;
            try {
                auto formal = chord_obj.formal_notes();
                note_names.reserve(formal.size());
                for (const auto& n : formal)
                    note_names.push_back(n.name());
            } catch (...) {
                // Fallback: use the natural (sharp-based) note names.
                try {
                    auto natural = chord_obj.notes();
                    note_names.reserve(natural.size());
                    for (const auto& n : natural)
                        note_names.push_back(n.name());
                } catch (...) {
                    // Unable to resolve notes — leave empty.
                }
            }

            result.push_back(HarmonicPath{
                path_id++,
                name,                           // branch
                chord_obj,
                std::move(interval_labels),
                std::move(note_names)
            });
            return true;
        };

    // -- Step 1: emit the origin chord itself. -------------------------------

    const TypeVector* origin_row = find_branch_row(branch_origin);
    if (origin_row) {
        emit_path(branch_origin);
    }

    // -- Step 2: emit every target reachable from the origin. ----------------

    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& prow = tp.row(static_cast<int>(i));

        if (element_to_string(prow[3]) != branch_origin)
            continue;

        std::string target_name = element_to_string(prow[4]);

        // Skip the self-referencing edge (already emitted as the origin).
        if (target_name == branch_origin)
            continue;

        const TypeVector* target_row = find_branch_row(target_name);
        if (target_row) {
            emit_path(target_name);
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// shortest_path() — BFS algorithm
// ---------------------------------------------------------------------------

std::vector<std::string> Tree::shortest_path(const std::string& from,
                                              const std::string& to) const
{
    using namespace internal;

    const auto& tree_data = LookupTree::instance();
    const auto& tp        = tree_data.paths();
    const int   scale_id  = static_cast<int>(type_);

    // Special case: same node
    if (from == to) {
        return {from};
    }

    // Build adjacency list for this scale
    std::map<std::string, std::vector<std::string>> adjacency;
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));

        if (element_to_int(row[0]) != scale_id)
            continue;

        std::string origin = element_to_string(row[3]);
        std::string target = element_to_string(row[4]);

        adjacency[origin].push_back(target);
    }

    // BFS to find shortest path
    std::queue<std::string> queue;
    std::map<std::string, std::string> parent;
    std::set<std::string> visited;

    queue.push(from);
    visited.insert(from);
    parent[from] = "";

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();

        if (current == to) {
            // Reconstruct path
            std::vector<std::string> path;
            std::string node = to;
            while (!node.empty()) {
                path.push_back(node);
                node = parent[node];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        // Explore neighbors
        if (adjacency.find(current) != adjacency.end()) {
            for (const auto& neighbor : adjacency[current]) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    queue.push(neighbor);
                }
            }
        }
    }

    // No path found
    return {};
}

// ---------------------------------------------------------------------------
// is_valid_progression()
// ---------------------------------------------------------------------------

bool Tree::is_valid_progression(const std::vector<std::string>& branches) const
{
    using namespace internal;

    if (branches.empty() || branches.size() == 1) {
        return true;  // Empty or single branch is trivially valid
    }

    const auto& tree_data = LookupTree::instance();
    const auto& tp        = tree_data.paths();
    const int   scale_id  = static_cast<int>(type_);

    // Build set of valid transitions for quick lookup
    std::set<std::pair<std::string, std::string>> valid_transitions;
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));

        if (element_to_int(row[0]) != scale_id)
            continue;

        std::string origin = element_to_string(row[3]);
        std::string target = element_to_string(row[4]);

        valid_transitions.insert({origin, target});
    }

    // Check each consecutive transition
    for (std::size_t i = 0; i < branches.size() - 1; ++i) {
        std::pair<std::string, std::string> transition = {branches[i], branches[i + 1]};

        if (valid_transitions.find(transition) == valid_transitions.end()) {
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// function() — Classify branch by harmonic function
// ---------------------------------------------------------------------------

HarmonicFunction Tree::function(const std::string& branch) const
{
    // Parse the primary degree from the branch name
    // For applied chords like "V7 / IV", we classify by the function itself (V7 = Dominant)
    // For diatonic chords like "I", "IIm", "IV", we classify by degree

    std::string primary = branch;

    // For applied chords, extract the function part
    if (branch.find(" / ") != std::string::npos) {
        size_t pos = branch.find(" / ");
        primary = branch.substr(0, pos);
    }

    // Remove accidentals and suffixes to get the degree
    std::string degree_str = primary;
    if (!degree_str.empty() && (degree_str[0] == 'b' || degree_str[0] == '#')) {
        degree_str = degree_str.substr(1);
    }

    // Check the degree (case-insensitive for Roman numerals)
    // Tonic: I, III, VI (and their minor variants)
    if (degree_str.find("VI") == 0 && degree_str.find("VII") != 0) {
        return HarmonicFunction::Tonic;
    }
    if (degree_str.find("III") == 0) {
        return HarmonicFunction::Tonic;
    }
    if (degree_str.find("I") == 0 && degree_str.find("II") != 0 && degree_str.find("IV") != 0) {
        return HarmonicFunction::Tonic;
    }

    // Subdominant: II, IV
    if (degree_str.find("IV") == 0) {
        return HarmonicFunction::Subdominant;
    }
    if (degree_str.find("II") == 0) {
        return HarmonicFunction::Subdominant;
    }

    // Dominant: V, VII, SUBV (tritone substitute)
    if (degree_str.find("SUBV") == 0) {
        return HarmonicFunction::Dominant;
    }
    if (degree_str.find("VII") == 0) {
        return HarmonicFunction::Dominant;
    }
    if (degree_str.find("V") == 0) {
        return HarmonicFunction::Dominant;
    }

    // Default: Tonic
    return HarmonicFunction::Tonic;
}

// ---------------------------------------------------------------------------
// branches_with_function()
// ---------------------------------------------------------------------------

std::vector<std::string> Tree::branches_with_function(HarmonicFunction func) const
{
    auto all_branches = branches();
    std::vector<std::string> result;

    for (const auto& branch : all_branches) {
        if (function(branch) == func) {
            result.push_back(branch);
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// to_dot() — Export to Graphviz DOT format
// ---------------------------------------------------------------------------

std::string Tree::to_dot(bool show_functions) const
{
    using namespace internal;

    const auto& tree_data = LookupTree::instance();
    const auto& tp        = tree_data.paths();
    const int   scale_id  = static_cast<int>(type_);

    std::ostringstream dot;

    // Header
    dot << "digraph HarmonicTree {\n";
    dot << "  rankdir=LR;\n";
    dot << "  node [shape=box, style=filled];\n\n";

    // Collect all branches
    std::set<std::string> all_branches;
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));
        if (element_to_int(row[0]) != scale_id)
            continue;

        std::string origin = element_to_string(row[3]);
        std::string target = element_to_string(row[4]);

        all_branches.insert(origin);
        all_branches.insert(target);
    }

    // Define nodes with colors based on function
    for (const auto& branch : all_branches) {
        std::string color = "lightgray";
        if (show_functions) {
            HarmonicFunction func = function(branch);
            if (func == HarmonicFunction::Tonic) {
                color = "lightblue";
            } else if (func == HarmonicFunction::Subdominant) {
                color = "lightgreen";
            } else if (func == HarmonicFunction::Dominant) {
                color = "lightyellow";
            }
        }

        // Escape quotes in branch names
        std::string escaped_branch = branch;
        size_t pos = 0;
        while ((pos = escaped_branch.find("\"", pos)) != std::string::npos) {
            escaped_branch.replace(pos, 1, "\\\"");
            pos += 2;
        }

        dot << "  \"" << escaped_branch << "\" [fillcolor=" << color << "];\n";
    }

    dot << "\n";

    // Define edges
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));
        if (element_to_int(row[0]) != scale_id)
            continue;

        std::string origin = element_to_string(row[3]);
        std::string target = element_to_string(row[4]);

        // Skip self-loops
        if (origin == target)
            continue;

        // Escape quotes
        std::string escaped_origin = origin;
        std::string escaped_target = target;
        size_t pos;

        pos = 0;
        while ((pos = escaped_origin.find("\"", pos)) != std::string::npos) {
            escaped_origin.replace(pos, 1, "\\\"");
            pos += 2;
        }

        pos = 0;
        while ((pos = escaped_target.find("\"", pos)) != std::string::npos) {
            escaped_target.replace(pos, 1, "\\\"");
            pos += 2;
        }

        dot << "  \"" << escaped_origin << "\" -> \"" << escaped_target << "\";\n";
    }

    dot << "}\n";

    return dot.str();
}

// ---------------------------------------------------------------------------
// to_mermaid() — Export to Mermaid diagram format
// ---------------------------------------------------------------------------

std::string Tree::to_mermaid() const
{
    using namespace internal;

    const auto& tree_data = LookupTree::instance();
    const auto& tp        = tree_data.paths();
    const int   scale_id  = static_cast<int>(type_);

    std::ostringstream mmd;

    // Header
    mmd << "graph LR\n";

    // Define edges (Mermaid creates nodes automatically)
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));
        if (element_to_int(row[0]) != scale_id)
            continue;

        std::string origin = element_to_string(row[3]);
        std::string target = element_to_string(row[4]);

        // Skip self-loops
        if (origin == target)
            continue;

        // Sanitize node names for Mermaid (replace spaces and special chars)
        std::string origin_id = origin;
        std::string target_id = target;

        // Replace "/" with "_" for node IDs
        std::replace(origin_id.begin(), origin_id.end(), '/', '_');
        std::replace(origin_id.begin(), origin_id.end(), ' ', '_');
        std::replace(target_id.begin(), target_id.end(), '/', '_');
        std::replace(target_id.begin(), target_id.end(), ' ', '_');

        mmd << "    " << origin_id << "[\"" << origin << "\"] --> "
            << target_id << "[\"" << target << "\"]\n";
    }

    return mmd.str();
}

// ---------------------------------------------------------------------------
// to_string()
// ---------------------------------------------------------------------------

std::string Tree::to_string() const
{
    return std::string("Tree(\"") + tonic_.name()
         + "\", \"" + scale_type_label(type_) + "\")";
}

} // namespace gingo
