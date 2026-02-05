// Gingo — Music Theory Library
// Tree: directed graph of chord-function relationships for a single tradition.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "chord.hpp"
#include "field.hpp"
#include "interval.hpp"
#include "note.hpp"
#include "progression.hpp"
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
/// relationships within a key, for a specific tradition.
///
/// A Tree is the complete map of valid chord transitions in a tradition.
/// It is created via Progression::tree("tradition_name"), or directly
/// with a 3-argument constructor.
///
/// Examples:
///   // Via Progression (preferred):
///   Progression p("C", ScaleType::Major);
///   Tree ht = p.tree("harmonic_tree");
///
///   // Direct construction:
///   Tree t("C", ScaleType::Major, "harmonic_tree");
///
///   auto paths = t.paths("I");
///   // Returns: I → IIm/IV, I → V7, I → VIm, etc.
class Tree {
public:
    /// Construct a tree for a specific tradition.
    Tree(const std::string& tonic, ScaleType type, const std::string& tradition);
    Tree(const std::string& tonic, const std::string& type_name,
         const std::string& tradition);

    /// Get the tonic note.
    Note tonic() const { return tonic_; }

    /// Get the scale type.
    ScaleType type() const { return type_; }

    /// Get the tradition metadata.
    Tradition tradition() const;

    /// List all available branch names for this scale type and tradition.
    std::vector<std::string> branches() const;

    /// Get harmonic paths starting from a given branch origin.
    /// Returns the origin chord plus all chords reachable from it.
    std::vector<HarmonicPath> paths(const std::string& branch_origin) const;

    /// Find the shortest path between two branches.
    /// Returns empty vector if no path exists.
    std::vector<std::string> shortest_path(const std::string& from,
                                           const std::string& to) const;

    /// Check if a progression is valid in this tradition.
    bool is_valid(const std::vector<std::string>& branches) const;

    /// Get the harmonic function (T/S/D) of a branch.
    HarmonicFunction function(const std::string& branch) const;

    /// List all branches with a specific harmonic function.
    std::vector<std::string> branches_with_function(HarmonicFunction func) const;

    /// Get named schemas (common patterns) in this tradition.
    std::vector<Schema> schemas() const;

    /// Export the tree to Graphviz DOT format for visualization.
    std::string to_dot(bool show_functions = false) const;

    /// Export the tree to Mermaid diagram format for documentation.
    std::string to_mermaid() const;

    std::string to_string() const;

private:
    Note        tonic_;
    ScaleType   type_;
    std::string tradition_;
    int         tonic_index_;
};

} // namespace gingo
