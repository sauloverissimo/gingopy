// Gingo — Music Theory Library
// Implementation of the Tree class.
//
// SPDX-License-Identifier: MIT

#include "gingo/tree.hpp"
#include "gingo/field.hpp"
#include "gingo/internal/lookup_data.hpp"
#include "gingo/internal/lookup_progression.hpp"
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
std::pair<std::string, std::string> parse_applied(const std::string& name) {
    size_t pos = name.find(" / ");
    if (pos == std::string::npos)
        return {"", ""};

    std::string function = name.substr(0, pos);
    std::string target = name.substr(pos + 3);
    return {function, target};
}

/// Resolve an applied chord using Field.applied().
Chord resolve_applied(const std::string& name, const Field& field) {
    auto [function, target] = parse_applied(name);
    if (function.empty())
        return Chord("C");

    return field.applied(function, target);
}

/// Resolve a diminished passing chord.
Chord resolve_diminished(const std::string& name, const Field& field) {
    std::string roman;
    if (name.size() >= 2 &&
        static_cast<unsigned char>(name[name.size()-2]) == 0xC2 &&
        static_cast<unsigned char>(name[name.size()-1]) == 0xB0) {
        roman = name.substr(0, name.length() - 2);
    } else if (name.size() >= 3 && name.substr(name.size() - 3) == "dim") {
        roman = name.substr(0, name.size() - 3);
    } else {
        return Chord("C");
    }

    int accidental = 0;
    size_t start = 0;
    std::string degree_part;

    if (!roman.empty() && roman[0] == 'b') {
        accidental = -1;
        start = 1;
        degree_part = roman.substr(start);
    } else if (!roman.empty() && roman[0] == '#') {
        accidental = 1;
        start = 1;
        degree_part = roman.substr(start);
    } else {
        degree_part = roman;
        if (!roman.empty() && roman.back() == '#') {
            accidental = 1;
            degree_part = roman.substr(0, roman.length() - 1);
        } else if (!roman.empty() && roman.back() == 'b') {
            accidental = -1;
            degree_part = roman.substr(0, roman.length() - 1);
        }
    }

    Note root = field.tonic();

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

    if (accidental != 0) {
        root = root.transpose(accidental);
    }

    return Chord(root.name() + "dim");
}

/// Resolve a tritone substitute chord.
Chord resolve_tritone_sub(const std::string& name, const Field& field) {
    if (name.find(" / ") != std::string::npos) {
        size_t pos = name.find(" / ");
        std::string target = name.substr(pos + 3);
        Chord dominant = field.applied("V7", target);
        Note tritone_root = dominant.root().transpose(6);
        return Chord(tritone_root.name() + "7");
    }

    Chord dominant = field.seventh(5);
    Note tritone_root = dominant.root().transpose(6);
    return Chord(tritone_root.name() + "7");
}

/// Resolve a diatonic chord.
Chord resolve_diatonic(const std::string& name, const Field& field) {
    int accidental = 0;
    size_t start = 0;

    if (!name.empty() && name[0] == 'b') {
        accidental = -1;
        start = 1;
    } else if (!name.empty() && name[0] == '#') {
        accidental = 1;
        start = 1;
    }

    std::string rest = name.substr(start);
    int degree = 0;

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

    bool has_minor = (rest.find("m") != std::string::npos && rest.find("dim") == std::string::npos);
    bool has_seventh = (rest.find("7") != std::string::npos);

    if ((has_minor && degree == 4) || (accidental == -1 && (degree == 6 || degree == 7))) {
        Field parallel_minor(field.tonic().name(), ScaleType::NaturalMinor);
        Chord borrowed = has_seventh ? parallel_minor.seventh(degree) : parallel_minor.chord(degree);
        return borrowed;
    }

    Note root = field.scale().degree(degree);

    if (accidental != 0) {
        root = root.transpose(accidental);
    }

    std::string chord_name = root.name();

    if (has_minor && has_seventh) {
        chord_name += "m7";
    } else if (has_minor) {
        chord_name += "m";
    } else if (has_seventh) {
        Chord diatonic = field.seventh(degree);
        std::string original = diatonic.name();
        size_t pos = original.find_first_not_of("ABCDEFG#b");
        if (pos != std::string::npos) {
            chord_name = root.name() + original.substr(pos);
        } else {
            chord_name += "7M";
        }
    } else {
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

/// Resolve a branch name to a Chord.
Chord resolve_branch_from_name(const std::string& branch_name,
                                const std::string& tonic,
                                ScaleType scale_type) {
    Field field(tonic, scale_type);

    if (branch_name.find(" / ") != std::string::npos) {
        return resolve_applied(branch_name, field);
    }

    bool has_degree_symbol = (branch_name.size() >= 2 &&
                              static_cast<unsigned char>(branch_name[branch_name.size()-2]) == 0xC2 &&
                              static_cast<unsigned char>(branch_name[branch_name.size()-1]) == 0xB0);
    bool has_dim_suffix = (branch_name.size() >= 3 &&
                           branch_name.substr(branch_name.size() - 3) == "dim");

    if (has_degree_symbol || has_dim_suffix) {
        return resolve_diminished(branch_name, field);
    }

    if (branch_name.find("SUBV") != std::string::npos) {
        return resolve_tritone_sub(branch_name, field);
    }

    return resolve_diatonic(branch_name, field);
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Tree::Tree(const std::string& tonic, ScaleType type, const std::string& tradition)
    : tonic_(tonic)
    , type_(type)
    , tradition_(tradition)
    , tonic_index_(tonic_.semitone())
{}

Tree::Tree(const std::string& tonic, const std::string& type_name,
           const std::string& tradition)
    : tonic_(tonic)
    , type_(Scale::parse_type(type_name))
    , tradition_(tradition)
    , tonic_index_(tonic_.semitone())
{}

// ---------------------------------------------------------------------------
// tradition()
// ---------------------------------------------------------------------------

Tradition Tree::tradition() const
{
    using namespace internal;
    const auto& lp = LookupProgression::instance();
    return Tradition{tradition_, lp.description(tradition_)};
}

// ---------------------------------------------------------------------------
// branches()
// ---------------------------------------------------------------------------

std::vector<std::string> Tree::branches() const
{
    using namespace internal;

    const auto& tb       = LookupProgression::instance().branches(tradition_);
    const int   scale_id = static_cast<int>(type_);

    std::vector<std::string> result;
    std::set<std::string>    seen;

    for (std::size_t i = 0; i < tb.row_count(); ++i) {
        const auto& row = tb.row(static_cast<int>(i));

        if (element_to_int(row[0]) != scale_id)
            continue;

        std::string name = element_to_string(row[1]);
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

    const auto& prog_data = LookupProgression::instance();
    const auto& tb        = prog_data.branches(tradition_);
    const auto& tp        = prog_data.paths(tradition_);
    const int   scale_id  = static_cast<int>(type_);

    std::vector<HarmonicPath> result;
    int path_id = 0;

    auto find_branch_row =
        [&](const std::string& name) -> const TypeVector* {
            for (std::size_t i = 0; i < tb.row_count(); ++i) {
                const auto& row = tb.row(static_cast<int>(i));
                if (element_to_int(row[0]) == scale_id &&
                    element_to_string(row[1]) == name)
                {
                    return &row;
                }
            }
            return nullptr;
        };

    auto emit_path =
        [&](const std::string& name) -> bool {
            Chord chord_obj("C");
            try {
                chord_obj = resolve_branch_from_name(name, tonic_.name(), type_);
            } catch (...) {
                return false;
            }

            std::vector<std::string> interval_labels;
            try {
                auto intervals = chord_obj.intervals();
                interval_labels.reserve(intervals.size());
                for (const auto& interval : intervals) {
                    interval_labels.push_back(interval.label());
                }
            } catch (...) {}

            std::vector<std::string> note_names;
            try {
                auto formal = chord_obj.formal_notes();
                note_names.reserve(formal.size());
                for (const auto& n : formal)
                    note_names.push_back(n.name());
            } catch (...) {
                try {
                    auto natural = chord_obj.notes();
                    note_names.reserve(natural.size());
                    for (const auto& n : natural)
                        note_names.push_back(n.name());
                } catch (...) {}
            }

            result.push_back(HarmonicPath{
                path_id++,
                name,
                chord_obj,
                std::move(interval_labels),
                std::move(note_names)
            });
            return true;
        };

    const TypeVector* origin_row = find_branch_row(branch_origin);
    if (origin_row) {
        emit_path(branch_origin);
    }

    // Paths table: {scaleIndex, branchOrigin, branchTarget}
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& prow = tp.row(static_cast<int>(i));

        if (element_to_int(prow[0]) != scale_id)
            continue;

        if (element_to_string(prow[1]) != branch_origin)
            continue;

        std::string target_name = element_to_string(prow[2]);

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
// shortest_path() — BFS
// ---------------------------------------------------------------------------

std::vector<std::string> Tree::shortest_path(const std::string& from,
                                              const std::string& to) const
{
    using namespace internal;

    const auto& tp       = LookupProgression::instance().paths(tradition_);
    const int   scale_id = static_cast<int>(type_);

    if (from == to) {
        return {from};
    }

    std::map<std::string, std::vector<std::string>> adjacency;
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));

        if (element_to_int(row[0]) != scale_id)
            continue;

        adjacency[element_to_string(row[1])].push_back(element_to_string(row[2]));
    }

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
            std::vector<std::string> path;
            std::string node = to;
            while (!node.empty()) {
                path.push_back(node);
                node = parent[node];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

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

    return {};
}

// ---------------------------------------------------------------------------
// is_valid()
// ---------------------------------------------------------------------------

bool Tree::is_valid(const std::vector<std::string>& branches) const
{
    using namespace internal;

    if (branches.empty() || branches.size() == 1) {
        return true;
    }

    const auto& tp       = LookupProgression::instance().paths(tradition_);
    const int   scale_id = static_cast<int>(type_);

    std::set<std::pair<std::string, std::string>> valid_transitions;
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));

        if (element_to_int(row[0]) != scale_id)
            continue;

        valid_transitions.insert({element_to_string(row[1]),
                                  element_to_string(row[2])});
    }

    for (std::size_t i = 0; i < branches.size() - 1; ++i) {
        if (valid_transitions.find({branches[i], branches[i + 1]}) ==
            valid_transitions.end()) {
            return false;
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// function()
// ---------------------------------------------------------------------------

HarmonicFunction Tree::function(const std::string& branch) const
{
    std::string primary = branch;

    if (branch.find(" / ") != std::string::npos) {
        primary = branch.substr(0, branch.find(" / "));
    }

    std::string degree_str = primary;
    if (!degree_str.empty() && (degree_str[0] == 'b' || degree_str[0] == '#')) {
        degree_str = degree_str.substr(1);
    }

    if (degree_str.find("VI") == 0 && degree_str.find("VII") != 0)
        return HarmonicFunction::Tonic;
    if (degree_str.find("III") == 0)
        return HarmonicFunction::Tonic;
    if (degree_str.find("I") == 0 && degree_str.find("II") != 0 &&
        degree_str.find("IV") != 0)
        return HarmonicFunction::Tonic;

    if (degree_str.find("IV") == 0)
        return HarmonicFunction::Subdominant;
    if (degree_str.find("II") == 0)
        return HarmonicFunction::Subdominant;

    if (degree_str.find("SUBV") == 0)
        return HarmonicFunction::Dominant;
    if (degree_str.find("VII") == 0)
        return HarmonicFunction::Dominant;
    if (degree_str.find("V") == 0)
        return HarmonicFunction::Dominant;

    return HarmonicFunction::Tonic;
}

// ---------------------------------------------------------------------------
// branches_with_function()
// ---------------------------------------------------------------------------

std::vector<std::string> Tree::branches_with_function(HarmonicFunction func) const
{
    auto all = branches();
    std::vector<std::string> result;
    for (const auto& b : all)
        if (function(b) == func)
            result.push_back(b);
    return result;
}

// ---------------------------------------------------------------------------
// schemas()
// ---------------------------------------------------------------------------

std::vector<Schema> Tree::schemas() const
{
    using namespace internal;

    const auto& ts       = LookupProgression::instance().schemas(tradition_);
    const int   scale_id = static_cast<int>(type_);

    std::vector<Schema> result;

    for (std::size_t i = 0; i < ts.row_count(); ++i) {
        const auto& row = ts.row(static_cast<int>(i));

        if (element_to_int(row[0]) != scale_id)
            continue;

        Schema s;
        s.name = element_to_string(row[1]);
        s.description = element_to_string(row[2]);

        if (auto* v = std::get_if<std::vector<std::string>>(&row[3]))
            s.branches = *v;

        result.push_back(std::move(s));
    }

    return result;
}

// ---------------------------------------------------------------------------
// to_dot()
// ---------------------------------------------------------------------------

std::string Tree::to_dot(bool show_functions) const
{
    using namespace internal;

    const auto& tp       = LookupProgression::instance().paths(tradition_);
    const int   scale_id = static_cast<int>(type_);

    std::ostringstream dot;
    dot << "digraph HarmonicTree {\n"
        << "  rankdir=LR;\n"
        << "  node [shape=box, style=filled];\n\n";

    std::set<std::string> all_branches;
    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));
        if (element_to_int(row[0]) != scale_id) continue;
        all_branches.insert(element_to_string(row[1]));
        all_branches.insert(element_to_string(row[2]));
    }

    auto escape = [](std::string s) {
        size_t pos = 0;
        while ((pos = s.find("\"", pos)) != std::string::npos) {
            s.replace(pos, 1, "\\\"");
            pos += 2;
        }
        return s;
    };

    for (const auto& branch : all_branches) {
        std::string color = "lightgray";
        if (show_functions) {
            auto func = function(branch);
            if (func == HarmonicFunction::Tonic) color = "lightblue";
            else if (func == HarmonicFunction::Subdominant) color = "lightgreen";
            else if (func == HarmonicFunction::Dominant) color = "lightyellow";
        }
        dot << "  \"" << escape(branch) << "\" [fillcolor=" << color << "];\n";
    }

    dot << "\n";

    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));
        if (element_to_int(row[0]) != scale_id) continue;
        std::string origin = element_to_string(row[1]);
        std::string target = element_to_string(row[2]);
        if (origin == target) continue;
        dot << "  \"" << escape(origin) << "\" -> \"" << escape(target) << "\";\n";
    }

    dot << "}\n";
    return dot.str();
}

// ---------------------------------------------------------------------------
// to_mermaid()
// ---------------------------------------------------------------------------

std::string Tree::to_mermaid() const
{
    using namespace internal;

    const auto& tp       = LookupProgression::instance().paths(tradition_);
    const int   scale_id = static_cast<int>(type_);

    std::ostringstream mmd;
    mmd << "graph LR\n";

    for (std::size_t i = 0; i < tp.row_count(); ++i) {
        const auto& row = tp.row(static_cast<int>(i));
        if (element_to_int(row[0]) != scale_id) continue;
        std::string origin = element_to_string(row[1]);
        std::string target = element_to_string(row[2]);
        if (origin == target) continue;

        auto sanitize = [](std::string s) {
            std::replace(s.begin(), s.end(), '/', '_');
            std::replace(s.begin(), s.end(), ' ', '_');
            return s;
        };

        mmd << "    " << sanitize(origin) << "[\"" << origin << "\"] --> "
            << sanitize(target) << "[\"" << target << "\"]\n";
    }

    return mmd.str();
}

// ---------------------------------------------------------------------------
// to_string()
// ---------------------------------------------------------------------------

std::string Tree::to_string() const
{
    return std::string("Tree(\"") + tonic_.name()
         + "\", \"" + scale_type_label(type_)
         + "\", \"" + tradition_ + "\")";
}

} // namespace gingo
