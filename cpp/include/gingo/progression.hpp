// Gingo — Music Theory Library
// Progression: coordinator for harmonic progressions across traditions.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "note.hpp"
#include "scale.hpp"

#include <string>
#include <vector>

namespace gingo {

// Forward declaration — Tree is defined in tree.hpp
class Tree;

// ---------------------------------------------------------------------------
// Tradition — metadata about a harmonic tradition
// ---------------------------------------------------------------------------

/// Identifies a harmonic tradition (e.g. "harmonic_tree", "jazz").
struct Tradition {
    std::string name;         ///< Machine-readable identifier
    std::string description;  ///< Human-readable description
};

// ---------------------------------------------------------------------------
// Schema — a named progression pattern within a tradition
// ---------------------------------------------------------------------------

/// A named chord progression pattern.
///
/// Examples:
///   Schema{"descending", "Main descending path", {"I","V7/IIm","IIm","V7","I"}}
///   Schema{"ii-V-I", "The fundamental jazz cadence", {"IIm","V7","I"}}
struct Schema {
    std::string name;                    ///< Pattern name
    std::string description;             ///< Human-readable description
    std::vector<std::string> branches;   ///< Sequence of branch labels
};

// ---------------------------------------------------------------------------
// Progression — cross-tradition coordinator
// ---------------------------------------------------------------------------

/// Coordinates harmonic progression analysis across multiple traditions.
///
/// Progression is the main entry point for working with harmonic progressions.
/// It provides access to tradition-specific trees and cross-tradition analysis
/// (identify, deduce, predict — implemented in the binding layer).
///
/// Examples:
///   Progression p("C", ScaleType::Major);
///   auto tree = p.tree("harmonic_tree");
///   tree.branches();  // All branches in Alencar's tree
///
///   auto jazz = p.tree("jazz");
///   jazz.schemas();   // ["ii-V-I", "turnaround", ...]
class Progression {
public:
    Progression(const std::string& tonic, ScaleType type);
    Progression(const std::string& tonic, const std::string& type_name);

    /// Get the tonic note.
    Note tonic() const { return tonic_; }

    /// Get the scale type.
    ScaleType type() const { return type_; }

    /// List all registered traditions.
    static std::vector<Tradition> traditions();

    /// Get a Tree (harmonic graph) for a specific tradition.
    /// Throws if tradition is unknown.
    Tree tree(const std::string& tradition) const;

    std::string to_string() const;

private:
    Note      tonic_;
    ScaleType type_;
};

} // namespace gingo
