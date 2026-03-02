// Gingo — Music Theory Library
// HarmonicFunction: the three primary harmonic functions.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>

namespace gingo {

/// The three primary harmonic functions in tonal music.
///
/// Every chord in a diatonic field belongs to one of three functional groups:
///   - Tonic (T):       stability, resolution, repose
///   - Subdominant (S): departure, tension away from tonic
///   - Dominant (D):    tension toward tonic, demands resolution
enum class HarmonicFunction {
    Tonic       = 0,
    Subdominant = 1,
    Dominant    = 2
};

/// Full name: "Tonic", "Subdominant", "Dominant".
std::string harmonic_function_name(HarmonicFunction f);

/// Short abbreviation: "T", "S", "D".
std::string harmonic_function_short(HarmonicFunction f);

} // namespace gingo
