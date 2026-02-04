// Gingo — Music Theory Library
// Singleton holding all music theory lookup tables.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "table.hpp"
#include "types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace gingo::internal {

class LookupData {
public:
    static const LookupData& instance();

    // Note data
    const Table& notes() const { return notes_; }
    const std::unordered_map<std::string, std::string>& enharmonic_map() const { return enharmonic_map_; }

    // Interval data
    const Table& intervals() const { return intervals_; }

    // Scale data
    const Table& scales() const { return scales_; }
    const Table& modalities() const { return modalities_; }

    // Chord formula data (was "bulk")
    const Table& chord_formulas() const { return chord_formulas_; }

    LookupData(const LookupData&) = delete;
    LookupData& operator=(const LookupData&) = delete;

private:
    LookupData();

    void init_enharmonic_map();
    void init_notes();
    void init_intervals();
    void init_scales();
    void init_modalities();
    void init_chord_formulas();

    std::unordered_map<std::string, std::string> enharmonic_map_;
    Table notes_;
    Table intervals_;
    Table scales_;
    Table modalities_;
    Table chord_formulas_;
};

} // namespace gingo::internal
