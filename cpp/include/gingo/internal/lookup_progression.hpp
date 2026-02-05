// Gingo — Music Theory Library
// Singleton holding progression data for all harmonic traditions.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "table.hpp"

#include <map>
#include <string>
#include <vector>

namespace gingo::internal {

/// Singleton providing access to harmonic progression data, organized by
/// tradition (e.g. "harmonic_tree", "jazz").
///
/// Each tradition stores its own branch definitions, path connections, and
/// named schemas. This replaces the former LookupTree singleton, which only
/// supported one tradition (Alencar's harmonic tree).
class LookupProgression {
public:
    static const LookupProgression& instance();

    /// List all registered tradition names.
    std::vector<std::string> tradition_names() const;

    /// Check if a tradition exists.
    bool has_tradition(const std::string& name) const;

    /// Human-readable description of a tradition.
    std::string description(const std::string& tradition) const;

    /// Branch definitions for a tradition.
    /// Columns: {scaleIndex (int), branch (string)}
    const Table& branches(const std::string& tradition) const;

    /// Path connections (directed graph edges) for a tradition.
    /// Columns: {scaleIndex (int), branchOrigin (string), branchTarget (string)}
    const Table& paths(const std::string& tradition) const;

    /// Named schemas (common progression patterns) for a tradition.
    /// Columns: {scaleIndex (int), name (string), description (string),
    ///           branches (vector<string>)}
    const Table& schemas(const std::string& tradition) const;

    LookupProgression(const LookupProgression&) = delete;
    LookupProgression& operator=(const LookupProgression&) = delete;

private:
    LookupProgression();

    void init_harmonic_tree();
    void init_jazz();

    struct TraditionData {
        std::string description;
        Table branches;
        Table paths;
        Table schemas;
    };

    std::map<std::string, TraditionData> data_;
};

} // namespace gingo::internal
