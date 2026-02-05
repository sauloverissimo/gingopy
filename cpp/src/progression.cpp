// Gingo — Music Theory Library
// Implementation of the Progression class.
//
// SPDX-License-Identifier: MIT

#include "gingo/progression.hpp"
#include "gingo/tree.hpp"
#include "gingo/internal/lookup_progression.hpp"

#include <stdexcept>

namespace gingo {

// ---------------------------------------------------------------------------
// Constructors
// ---------------------------------------------------------------------------

Progression::Progression(const std::string& tonic, ScaleType type)
    : tonic_(tonic)
    , type_(type)
{}

Progression::Progression(const std::string& tonic, const std::string& type_name)
    : tonic_(tonic)
    , type_(Scale::parse_type(type_name))
{}

// ---------------------------------------------------------------------------
// traditions()
// ---------------------------------------------------------------------------

std::vector<Tradition> Progression::traditions()
{
    using namespace internal;

    const auto& lp = LookupProgression::instance();
    auto names = lp.tradition_names();

    std::vector<Tradition> result;
    result.reserve(names.size());

    for (const auto& name : names) {
        result.push_back(Tradition{name, lp.description(name)});
    }

    return result;
}

// ---------------------------------------------------------------------------
// tree()
// ---------------------------------------------------------------------------

Tree Progression::tree(const std::string& tradition) const
{
    using namespace internal;

    if (!LookupProgression::instance().has_tradition(tradition)) {
        throw std::invalid_argument("Unknown tradition: " + tradition);
    }

    return Tree(tonic_.name(), type_, tradition);
}

// ---------------------------------------------------------------------------
// to_string()
// ---------------------------------------------------------------------------

std::string Progression::to_string() const
{
    return std::string("Progression(\"") + tonic_.name() + "\")";
}

} // namespace gingo
