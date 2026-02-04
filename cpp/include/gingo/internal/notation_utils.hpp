// Gingo — Music Theory Library
// Internal notation utility functions: chromatic/diatonic conversions,
// formal note spelling, and related helpers.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "types.hpp"

#include <string>
#include <vector>

namespace gingo::internal {

/// Compute the minimum semitone difference between @p target_note and
/// @p start_note on the chromatic circle (C=0, C#=1, ... B=11).
/// When @p range is "min" the result is wrapped to [-6, +6].
int half_tones(const std::string& target_note,
               const std::string& start_note,
               const std::string& range = "min");

/// Convert a diatonic @p base_note (e.g. "D") to formal notation given the
/// chromatic @p target_note (e.g. "C#").  Produces the shortest accidental
/// spelling: sharps when the target is higher, flats when lower.
std::string natural_to_formal(const std::string& base_note,
                              const std::string& target_note);

/// Rotate the diatonic @p base vector so that it starts at @p tonic, extend
/// by cycling to @p total_length entries, then erase the positions listed in
/// @p remove_indices.
std::vector<std::string> rotate_remove(const std::vector<std::string>& base,
                                       const std::string& tonic,
                                       int total_length,
                                       const std::vector<int>& remove_indices);

/// Convert a vector of chromatic note names to formal (proper diatonic letter)
/// notation relative to @p tonic_original.  Empty strings and "0" entries are
/// preserved as-is.
std::vector<std::string> get_formal_notes(
    const std::vector<std::string>& chromatic_notes,
    const std::string& tonic_original,
    const std::vector<int>& remove_indices = {7, 9, 11, 13});

/// Like get_formal_notes but operates on a TypeVector (extracts strings,
/// processes them, and re-packs the results).
TypeVector get_formal_notes_tv(
    const TypeVector& chromatic_notes,
    const std::string& tonic_original,
    const std::vector<int>& remove_indices = {7, 9, 11, 13});

/// Convert a scale's note vector to formal (proper diatonic) notation.
/// The first non-empty note is used as the tonic reference.
TypeVector get_natural_notes(
    const TypeVector& scale_notes,
    const std::vector<int>& remove_indices = {7, 9, 11, 13});

} // namespace gingo::internal
