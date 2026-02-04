// Gingo — Music Theory Library
// Tree: harmonic progression tree — paths between chord functions.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "chord.hpp"
#include "field.hpp"
#include "interval.hpp"
#include "note.hpp"
#include "scale.hpp"

#include <string>
#include <vector>

namespace gingo {

/// A single node in a harmonic path, representing one chord and its context.
struct HarmonicPath {
    int         id;             // Sequential index in the path
    std::string branch;         // Functional label ("I", "V7", "IIm", etc.)
    Chord       chord;          // The resolved chord
    std::vector<std::string> interval_labels;  // Interval formula
    std::vector<std::string> note_names;       // Formal note names
};

/// Represents a harmonic tree — a directed graph of chord-function
/// relationships within a key.
///
/// The tree encodes how chords connect to each other in tonal harmony:
/// which chords can follow which, and what their functional labels are.
///
/// Examples:
///   Tree t("C", ScaleType::Major);
///   auto paths = t.paths("I");
///   // Returns: I → IIm/IV, I → V7, I → VIm, etc.
class Tree {
public:
    Tree(const std::string& tonic, ScaleType type);
    Tree(const std::string& tonic, const std::string& type_name);

    /// Get the tonic note.
    Note tonic() const { return tonic_; }

    /// Get the scale type.
    ScaleType type() const { return type_; }

    /// List all available branch names for this scale type.
    std::vector<std::string> branches() const;

    /// Get harmonic paths starting from a given branch origin.
    /// Returns the origin chord plus all chords reachable from it.
    std::vector<HarmonicPath> paths(const std::string& branch_origin) const;

    /// Find the shortest path between two branches.
    /// Returns empty vector if no path exists.
    /// Example: shortest_path("I", "V7") might return ["I", "IIm", "V7"]
    std::vector<std::string> shortest_path(const std::string& from,
                                           const std::string& to) const;

    /// Check if a progression is valid according to the harmonic tree.
    /// Returns true if each transition exists in the paths table.
    bool is_valid_progression(const std::vector<std::string>& branches) const;

    /// Get the harmonic function (T/S/D) of a branch.
    /// Returns the function based on the primary degree.
    /// Examples: "I" → Tonic, "V7" → Dominant, "IIm / IV" → Subdominant
    HarmonicFunction function(const std::string& branch) const;

    /// List all branches with a specific harmonic function.
    std::vector<std::string> branches_with_function(HarmonicFunction func) const;

    /// Export the tree to Graphviz DOT format for visualization.
    /// Set show_functions=true to color-code by harmonic function.
    std::string to_dot(bool show_functions = false) const;

    /// Export the tree to Mermaid diagram format for documentation.
    std::string to_mermaid() const;

    std::string to_string() const;

private:
    Note      tonic_;
    ScaleType type_;
    int       tonic_index_;
};

} // namespace gingo
