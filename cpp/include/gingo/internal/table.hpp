// Gingo — Music Theory Library
// Internal Table class for structured lookup data.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "types.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace gingo::internal {

/// A simple 2-D table used internally to store music-theory lookup data.
///
/// Each row is a TypeVector (heterogeneous array of TypeElements).
/// Rows can be accessed by numeric index or by name through rowNameToIndex.
/// Column names are stored for documentation / debug purposes only.
class Table {
public:
    TypeTable data;
    std::vector<std::string> column_names;
    std::unordered_map<std::string, int> row_name_to_index;

    Table() = default;

    explicit Table(const std::vector<std::string>& columns)
        : column_names(columns) {}

    Table(TypeTable init_data, const std::vector<std::string>& columns)
        : data(std::move(init_data)), column_names(columns) {}

    // --- Row access ----------------------------------------------------------

    const TypeVector& row(int index) const {
        return data.at(static_cast<size_t>(index));
    }

    const TypeVector& row(const std::string& name) const {
        auto it = row_name_to_index.find(name);
        if (it == row_name_to_index.end())
            throw std::invalid_argument("Table: row name not found: " + name);
        return data.at(static_cast<size_t>(it->second));
    }

    const TypeVector& operator[](int index) const { return row(index); }
    const TypeVector& operator[](const std::string& name) const { return row(name); }

    // --- Mutation -------------------------------------------------------------

    void add_row(TypeVector r) { data.push_back(std::move(r)); }

    // --- Query ---------------------------------------------------------------

    std::size_t row_count() const { return data.size(); }

    bool has_row(const std::string& name) const {
        return row_name_to_index.count(name) > 0;
    }

    int row_index(const std::string& name) const {
        auto it = row_name_to_index.find(name);
        if (it == row_name_to_index.end()) return -1;
        return it->second;
    }

    /// Reverse lookup: find the canonical name for a given row index.
    /// Selection priority:
    ///   1. Reject purely-symbolic names (e.g. "+") when an alphanumeric
    ///      alternative exists (e.g. "aug").  A name is "purely symbolic"
    ///      if it contains no letters and no digits.
    ///   2. Among remaining candidates, prefer the shortest.
    ///   3. Tie-break alphabetically.
    std::string row_name_by_index(int index) const {
        std::string best;
        bool best_has_alnum = false;
        for (const auto& [name, idx] : row_name_to_index) {
            if (idx != index) continue;
            bool has_alnum = false;
            for (char c : name) {
                if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                    (c >= '0' && c <= '9')) {
                    has_alnum = true;
                    break;
                }
            }
            if (best.empty() ||
                (has_alnum && !best_has_alnum) ||
                (has_alnum == best_has_alnum &&
                 (name.size() < best.size() ||
                  (name.size() == best.size() && name < best)))) {
                best = name;
                best_has_alnum = has_alnum;
            }
        }
        return best;
    }
};

} // namespace gingo::internal
