// Gingo — Music Theory Library
// Internal type definitions replacing Arduino-specific datahandler.h
//
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <variant>
#include <vector>

namespace gingo::internal {

// A musical element can be a string (note name, label), an integer (index,
// degree, flag), a double (ratio, frequency), or a compound vector.
using TypeElement = std::variant<
    std::string,
    int,
    double,
    std::vector<std::string>,
    std::vector<int>>;

using TypeVector = std::vector<TypeElement>;
using TypeTable  = std::vector<TypeVector>;

// ---------------------------------------------------------------------------
// Convenience helpers for extracting values from TypeElement
// ---------------------------------------------------------------------------

/// Return the string inside a TypeElement, or empty string if not a string.
inline std::string element_to_string(const TypeElement& e) {
    if (auto* s = std::get_if<std::string>(&e)) return *s;
    return {};
}

/// Return the int inside a TypeElement, or 0 if not an int.
inline int element_to_int(const TypeElement& e) {
    if (auto* v = std::get_if<int>(&e)) return *v;
    return 0;
}

/// Return the double inside a TypeElement, or 0.0 if not a double.
inline double element_to_double(const TypeElement& e) {
    if (auto* v = std::get_if<double>(&e)) return *v;
    return 0.0;
}

/// Check whether a TypeElement holds a string.
inline bool is_string(const TypeElement& e) {
    return std::holds_alternative<std::string>(e);
}

/// Check whether a TypeElement holds an int.
inline bool is_int(const TypeElement& e) {
    return std::holds_alternative<int>(e);
}

} // namespace gingo::internal
