// Gingo — Music Theory Library
// Mode name lookup table: maps mode names to parent scale + mode number.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "gingo/scale.hpp"

#include <string>

namespace gingo::internal {

/// Metadata for a single mode of a parent scale.
struct ModeInfo {
    ScaleType   parent;
    int         mode_number;   // 1-based
    const char* name;          // primary name (e.g. "Dorian")
    const char* quality;       // "major" or "minor"
    int         brightness;    // 1-7 for Major modes, 0 otherwise
    int         note_count;    // 7 for heptatonic, 6/8/12 for others
};

/// Look up mode info by name (case-insensitive).
/// Searches primary names and all aliases.
/// Returns nullptr if not found.
const ModeInfo* find_mode_by_name(const std::string& name);

/// Look up mode info by parent + mode number.
/// Returns nullptr if the combination is invalid.
const ModeInfo* find_mode(ScaleType parent, int mode_number);

} // namespace gingo::internal
