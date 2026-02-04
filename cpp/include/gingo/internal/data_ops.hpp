// Gingo — Music Theory Library
// Internal data operations: rotate, spread, spin.
//
// These replace the utility functions from the external datahandler.h library.
//
// SPDX-License-Identifier: MIT

#pragma once

#include "types.hpp"
#include "table.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace gingo::internal {

/// Rotate a TypeVector so that it starts at `start_elem`, then extend it to
/// `length` elements by cycling.  This is used everywhere to rebase the
/// chromatic scale to a given tonic.
///
/// Example: rotate({"C","C#","D",...,"B"}, "D", 24)
///   → {"D","D#","E",...,"B","C","C#","D","D#",...}   (24 elements)
TypeVector rotate(const TypeVector& vec, const TypeElement& start_elem, int length = 12);

/// Create a TypeVector of `length` identical copies of vec[row].
/// Used to fill the "tonic" row in scale tables.
TypeVector spread(const TypeVector& vec, std::size_t row);

/// Circular-shift a TypeVector by `offset` positions.
/// Returns a new vector of the same size.
TypeVector spin(const TypeVector& vec, std::size_t offset);

/// Build a TypeTable containing all rotations of `vec`.
/// Row i is spin(vec, i).  Used for harmonic field construction.
TypeTable spin_all(const TypeVector& vec);

} // namespace gingo::internal
