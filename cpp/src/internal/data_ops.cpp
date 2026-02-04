// SPDX-License-Identifier: MIT

#include "gingo/internal/data_ops.hpp"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

namespace gingo::internal {

TypeVector rotate(const TypeVector& vec, const TypeElement& start_elem, int length)
{
    if (vec.empty()) {
        return {};
    }

    // Find the position of start_elem in vec.
    auto it = std::find(vec.begin(), vec.end(), start_elem);
    if (it == vec.end()) {
        throw std::invalid_argument("rotate: start_elem not found in vec");
    }

    const auto start = static_cast<std::size_t>(std::distance(vec.begin(), it));
    const auto n     = vec.size();

    TypeVector result;
    result.reserve(static_cast<std::size_t>(length));

    for (int i = 0; i < length; ++i) {
        result.push_back(vec[(start + static_cast<std::size_t>(i)) % n]);
    }

    return result;
}

TypeVector spread(const TypeVector& vec, std::size_t row)
{
    constexpr std::size_t count = 24;
    return TypeVector(count, vec.at(row));
}

TypeVector spin(const TypeVector& vec, std::size_t offset)
{
    const auto n = vec.size();
    if (n == 0) {
        return {};
    }

    TypeVector result;
    result.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        result.push_back(vec[(i + offset) % n]);
    }

    return result;
}

TypeTable spin_all(const TypeVector& vec)
{
    const auto n = vec.size();

    TypeTable table;
    table.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        table.push_back(spin(vec, i));
    }

    return table;
}

} // namespace gingo::internal
