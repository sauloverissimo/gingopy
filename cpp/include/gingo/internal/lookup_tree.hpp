// Gingo — Music Theory Library
// Singleton holding harmonic tree lookup tables.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "table.hpp"

namespace gingo::internal {

/// Singleton providing access to harmonic tree data.
///
/// Contains branch definitions (68 entries) and path connections (57 entries)
/// that encode the harmonic progression rules from Alencar's theory.
class LookupTree {
public:
    static const LookupTree& instance();

    /// Table of branch definitions (68 rows).
    /// Columns: scaleIndex, targetDegree, startDegree, targetAccident,
    ///          bulkIndex, branch
    const Table& branches() const { return branches_; }

    /// Table of path connections (57 rows for major scale).
    /// Columns: scaleIndex, originIndex, targetIndex, branchOrigin,
    ///          branchTarget
    const Table& paths() const { return paths_; }

    LookupTree(const LookupTree&) = delete;
    LookupTree& operator=(const LookupTree&) = delete;

private:
    LookupTree();

    void init_branches();
    void init_paths();

    Table branches_;
    Table paths_;
};

} // namespace gingo::internal
