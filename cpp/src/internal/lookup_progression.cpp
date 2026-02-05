// Gingo — Music Theory Library
// Implementation of LookupProgression singleton — progression data by tradition.
//
// SPDX-License-Identifier: MIT

#include "gingo/internal/lookup_progression.hpp"

#include <stdexcept>

namespace gingo::internal {

// ---------------------------------------------------------------------------
// Singleton (Meyer's pattern — thread-safe in C++11 and later)
// ---------------------------------------------------------------------------

const LookupProgression& LookupProgression::instance() {
    static LookupProgression prog;
    return prog;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

LookupProgression::LookupProgression() {
    init_harmonic_tree();
    init_jazz();
}

// ---------------------------------------------------------------------------
// Accessors
// ---------------------------------------------------------------------------

std::vector<std::string> LookupProgression::tradition_names() const {
    std::vector<std::string> names;
    names.reserve(data_.size());
    for (const auto& [name, _] : data_)
        names.push_back(name);
    return names;
}

bool LookupProgression::has_tradition(const std::string& name) const {
    return data_.count(name) > 0;
}

std::string LookupProgression::description(const std::string& tradition) const {
    auto it = data_.find(tradition);
    if (it == data_.end())
        throw std::invalid_argument("Unknown tradition: " + tradition);
    return it->second.description;
}

const Table& LookupProgression::branches(const std::string& tradition) const {
    auto it = data_.find(tradition);
    if (it == data_.end())
        throw std::invalid_argument("Unknown tradition: " + tradition);
    return it->second.branches;
}

const Table& LookupProgression::paths(const std::string& tradition) const {
    auto it = data_.find(tradition);
    if (it == data_.end())
        throw std::invalid_argument("Unknown tradition: " + tradition);
    return it->second.paths;
}

const Table& LookupProgression::schemas(const std::string& tradition) const {
    auto it = data_.find(tradition);
    if (it == data_.end())
        throw std::invalid_argument("Unknown tradition: " + tradition);
    return it->second.schemas;
}

// ---------------------------------------------------------------------------
// Harmonic Tree tradition (José de Alencar Soares)
// ---------------------------------------------------------------------------

void LookupProgression::init_harmonic_tree() {
    TraditionData td;
    td.description = "Harmonic Tree — José de Alencar Soares' model for "
                     "Brazilian Popular Music harmonic progressions.";

    // --- Branches -----------------------------------------------------------

    td.branches = Table({"scaleIndex", "branch"});

    td.branches.data = {
        // Major scale (scaleIndex = 0) — 29 entries
        {0, std::string("I")},
        {0, std::string("IIm / IV")},
        {0, std::string("IIm7(b5) / IIm")},
        {0, std::string("IIm7(11) / IV")},
        {0, std::string("SUBV7 / IV")},
        {0, std::string("V7 / IV")},
        {0, std::string("VIm")},
        {0, std::string("V7 / IIm")},
        {0, std::string("Idim")},
        {0, std::string("#Idim")},
        {0, std::string("bIIIdim")},
        {0, std::string("IV#dim")},
        {0, std::string("IV")},
        {0, std::string("V7 / V")},
        {0, std::string("IIm")},
        {0, std::string("IVm")},
        {0, std::string("bVI")},
        {0, std::string("bVII")},
        {0, std::string("IIm7(b5)")},
        {0, std::string("II#dim")},
        {0, std::string("SUBV7")},
        {0, std::string("V7")},
        {0, std::string("V7 / VI")},
        {0, std::string("V7 / Im")},
        {0, std::string("V7 / V")},
        {0, std::string("V7 / III")},
        {0, std::string("V7 / IV")},
        {0, std::string("V7 / bIII")},
        {0, std::string("V7 / bVI")},

        // Natural minor (scaleIndex = 1) — 16 entries
        {1, std::string("Im")},
        {1, std::string("IIm7(b5) / Ivm")},
        {1, std::string("IVm7 / IVm")},
        {1, std::string("V / IVm")},
        {1, std::string("V / V")},
        {1, std::string("bVI / Im")},
        {1, std::string("II / IIm")},
        {1, std::string("IVm")},
        {1, std::string("bV / V")},
        {1, std::string("IV#dim")},
        {1, std::string("V7 / I")},
        {1, std::string("V7 / III")},
        {1, std::string("V7 / V")},
        {1, std::string("bVII")},
        {1, std::string("bIII")},
        {1, std::string("Vm")},

        // Harmonic minor (scaleIndex = 2) — 15 entries
        {2, std::string("Im")},
        {2, std::string("IIm7(b5) / Ivm")},
        {2, std::string("IVm7 / IVm")},
        {2, std::string("V / IVm")},
        {2, std::string("V / V")},
        {2, std::string("bVI / Im")},
        {2, std::string("II / IIm")},
        {2, std::string("IVm")},
        {2, std::string("bV / V")},
        {2, std::string("IV#dim")},
        {2, std::string("V7 / I")},
        {2, std::string("V7 / III")},
        {2, std::string("V7 / V")},
        {2, std::string("bVII")},
        {2, std::string("bIII")},

        // Melodic minor (scaleIndex = 3) — 15 entries
        {3, std::string("Im")},
        {3, std::string("IIm7(b5) / Ivm")},
        {3, std::string("IVm7 / IVm")},
        {3, std::string("V / IVm")},
        {3, std::string("V / V")},
        {3, std::string("bVI / Im")},
        {3, std::string("II / IIm")},
        {3, std::string("IVm")},
        {3, std::string("bV / V")},
        {3, std::string("IV#dim")},
        {3, std::string("V7 / I")},
        {3, std::string("V7 / III")},
        {3, std::string("V7 / V")},
        {3, std::string("bVII")},
        {3, std::string("bIII")}
    };

    // --- Paths (simplified: no originIndex/targetIndex) ---------------------

    td.paths = Table({"scaleIndex", "branchOrigin", "branchTarget"});

    td.paths.data = {
        // Major scale (scaleIndex = 0)
        {0, std::string("I"), std::string("I")},
        {0, std::string("I"), std::string("IIm / IV")},
        {0, std::string("I"), std::string("IIm7(b5) / IIm")},
        {0, std::string("I"), std::string("IIm7(11) / IV")},
        {0, std::string("I"), std::string("VIm")},
        {0, std::string("I"), std::string("#Idim")},
        {0, std::string("I"), std::string("bIIIdim")},
        {0, std::string("I"), std::string("V7")},
        {0, std::string("I"), std::string("V7 / VI")},
        {0, std::string("IIm / IV"), std::string("V7 / IV")},
        {0, std::string("IIm7(b5) / IIm"), std::string("V7 / IIm")},
        {0, std::string("IIm7(11) / IV"), std::string("SUBV7 / IV")},
        {0, std::string("SUBV7 / IV"), std::string("IV")},
        {0, std::string("V7 / IV"), std::string("IV")},
        {0, std::string("VIm"), std::string("IV")},
        {0, std::string("V7 / IIm"), std::string("IIm")},
        {0, std::string("Idim"), std::string("IIm")},
        {0, std::string("#Idim"), std::string("IIm")},
        {0, std::string("bIIIdim"), std::string("IIm")},
        {0, std::string("IV"), std::string("IV#dim")},
        {0, std::string("IV#dim"), std::string("V7")},
        {0, std::string("IV#dim"), std::string("I")},
        {0, std::string("IV"), std::string("IVm")},
        {0, std::string("IV"), std::string("bVI")},
        {0, std::string("IV"), std::string("bVII")},
        {0, std::string("IV"), std::string("V7")},
        {0, std::string("V7 / V"), std::string("IIm")},
        {0, std::string("V7 / V"), std::string("V7")},
        {0, std::string("IIm"), std::string("V7")},
        {0, std::string("IVm"), std::string("V7")},
        {0, std::string("bVI"), std::string("I")},
        {0, std::string("bVI"), std::string("V7")},
        {0, std::string("bVII"), std::string("I")},
        {0, std::string("bVII"), std::string("V7")},
        {0, std::string("IIm7(b5)"), std::string("I")},
        {0, std::string("IIm7(b5)"), std::string("V7")},
        {0, std::string("SUBV7"), std::string("I")},
        {0, std::string("IV"), std::string("IIm7(b5)")},
        {0, std::string("I"), std::string("Idim")},
        {0, std::string("V7 / IIm"), std::string("V7 / V")},
        {0, std::string("Idim"), std::string("V7 / V")},
        {0, std::string("#Idim"), std::string("V7 / V")},
        {0, std::string("bIIIdim"), std::string("V7 / V")},
        {0, std::string("V7 / V"), std::string("SUBV7")},
        {0, std::string("IIm"), std::string("V7 / V")},
        {0, std::string("IIm"), std::string("IVm")},
        {0, std::string("IIm"), std::string("bVI")},
        {0, std::string("IIm"), std::string("bVII")},
        {0, std::string("IIm"), std::string("IIm7(b5)")},
        {0, std::string("V7 / V"), std::string("IVm")},
        {0, std::string("V7 / V"), std::string("bVI")},
        {0, std::string("V7 / V"), std::string("bVII")},
        {0, std::string("V7 / V"), std::string("IIm7(b5)")},
        {0, std::string("IVm"), std::string("I")},
        {0, std::string("V7"), std::string("I")},
        {0, std::string("IV#dim"), std::string("I")},
        {0, std::string("IIm"), std::string("SUBV7")},
        {0, std::string("IIm"), std::string("II#dim")},
        {0, std::string("II#dim"), std::string("I")},

        // Natural minor (scaleIndex = 1)
        {1, std::string("Im"), std::string("Im")},
        {1, std::string("Im"), std::string("IIm7(b5) / Ivm")},
        {1, std::string("Im"), std::string("II / IIm")},
        {1, std::string("Im"), std::string("V / V")},
        {1, std::string("Im"), std::string("bVI / Im")},
        {1, std::string("Im"), std::string("V7 / I")},
        {1, std::string("IIm7(b5) / Ivm"), std::string("IVm7 / IVm")},
        {1, std::string("IVm7 / IVm"), std::string("V / IVm")},
        {1, std::string("V / IVm"), std::string("IVm")},
        {1, std::string("V / V"), std::string("V7 / I")},
        {1, std::string("bVI / Im"), std::string("Im")},
        {1, std::string("II / IIm"), std::string("IVm")},
        {1, std::string("IVm"), std::string("V7 / I")},
        {1, std::string("IVm"), std::string("IV#dim")},
        {1, std::string("IV#dim"), std::string("V7 / I")},
        {1, std::string("IV#dim"), std::string("Im")},
        {1, std::string("V7 / I"), std::string("Im")},
        {1, std::string("V7 / III"), std::string("Im")},
        {1, std::string("V7 / V"), std::string("V7 / I")},
        {1, std::string("Im"), std::string("bVII")},
        {1, std::string("bVII"), std::string("bIII")},
        {1, std::string("bIII"), std::string("IVm")},
        {1, std::string("bIII"), std::string("V7 / I")},
        {1, std::string("bVII"), std::string("IVm")},
        {1, std::string("Vm"), std::string("Im")},

        // Harmonic minor (scaleIndex = 2)
        {2, std::string("Im"), std::string("Im")},
        {2, std::string("Im"), std::string("IIm7(b5) / Ivm")},
        {2, std::string("Im"), std::string("II / IIm")},
        {2, std::string("Im"), std::string("V / V")},
        {2, std::string("Im"), std::string("bVI / Im")},
        {2, std::string("Im"), std::string("V7 / I")},
        {2, std::string("IIm7(b5) / Ivm"), std::string("IVm7 / IVm")},
        {2, std::string("IVm7 / IVm"), std::string("V / IVm")},
        {2, std::string("V / IVm"), std::string("IVm")},
        {2, std::string("V / V"), std::string("V7 / I")},
        {2, std::string("bVI / Im"), std::string("Im")},
        {2, std::string("II / IIm"), std::string("IVm")},
        {2, std::string("IVm"), std::string("V7 / I")},
        {2, std::string("IVm"), std::string("IV#dim")},
        {2, std::string("IV#dim"), std::string("V7 / I")},
        {2, std::string("IV#dim"), std::string("Im")},
        {2, std::string("V7 / I"), std::string("Im")},
        {2, std::string("V7 / III"), std::string("Im")},
        {2, std::string("V7 / V"), std::string("V7 / I")},
        {2, std::string("Im"), std::string("bVII")},
        {2, std::string("bVII"), std::string("bIII")},
        {2, std::string("bIII"), std::string("IVm")},
        {2, std::string("bIII"), std::string("V7 / I")},
        {2, std::string("bVII"), std::string("IVm")},

        // Melodic minor (scaleIndex = 3)
        {3, std::string("Im"), std::string("Im")},
        {3, std::string("Im"), std::string("IIm7(b5) / Ivm")},
        {3, std::string("Im"), std::string("II / IIm")},
        {3, std::string("Im"), std::string("V / V")},
        {3, std::string("Im"), std::string("bVI / Im")},
        {3, std::string("Im"), std::string("V7 / I")},
        {3, std::string("IIm7(b5) / Ivm"), std::string("IVm7 / IVm")},
        {3, std::string("IVm7 / IVm"), std::string("V / IVm")},
        {3, std::string("V / IVm"), std::string("IVm")},
        {3, std::string("V / V"), std::string("V7 / I")},
        {3, std::string("bVI / Im"), std::string("Im")},
        {3, std::string("II / IIm"), std::string("IVm")},
        {3, std::string("IVm"), std::string("V7 / I")},
        {3, std::string("IVm"), std::string("IV#dim")},
        {3, std::string("IV#dim"), std::string("V7 / I")},
        {3, std::string("IV#dim"), std::string("Im")},
        {3, std::string("V7 / I"), std::string("Im")},
        {3, std::string("V7 / III"), std::string("Im")},
        {3, std::string("V7 / V"), std::string("V7 / I")},
        {3, std::string("Im"), std::string("bVII")},
        {3, std::string("bVII"), std::string("bIII")},
        {3, std::string("bIII"), std::string("IVm")},
        {3, std::string("bIII"), std::string("V7 / I")},
        {3, std::string("bVII"), std::string("IVm")}
    };

    // --- Schemas (named progression patterns) -------------------------------

    td.schemas = Table({"scaleIndex", "name", "description", "branches"});

    using SV = std::vector<std::string>;

    td.schemas.data = {
        // Major schemas
        {0, std::string("descending"),
            std::string("Main descending path (por baixo)"),
            SV{"I", "V7 / IIm", "IIm", "V7", "I"}},
        {0, std::string("ascending"),
            std::string("Ascending path through IV (por cima)"),
            SV{"I", "V7 / IV", "IV", "V7", "I"}},
        {0, std::string("direct"),
            std::string("Direct resolution I-V7-I"),
            SV{"I", "V7", "I"}},
        {0, std::string("extended_descending"),
            std::string("Extended descending with applied IIm7(b5)"),
            SV{"I", "IIm7(b5) / IIm", "V7 / IIm", "IIm", "V7", "I"}},
        {0, std::string("subdominant_prep"),
            std::string("Subdominant preparation via IIm/IV"),
            SV{"I", "IIm / IV", "V7 / IV", "IV", "V7", "I"}},
        {0, std::string("tritone_sub"),
            std::string("Tritone substitution resolution"),
            SV{"I", "IIm", "SUBV7", "I"}},
        {0, std::string("diminished_passing"),
            std::string("Diminished passing chord to IIm"),
            SV{"I", "#Idim", "IIm", "V7", "I"}},
        {0, std::string("modal_borrowing"),
            std::string("Modal interchange via borrowed IVm"),
            SV{"I", "IIm", "IVm", "V7", "I"}},

        // Minor schemas
        {1, std::string("minor_descending"),
            std::string("Minor descending: Im-V7-Im"),
            SV{"Im", "V7 / I", "Im"}},
        {1, std::string("minor_subdominant"),
            std::string("Full minor subdominant path"),
            SV{"Im", "IIm7(b5) / Ivm", "IVm7 / IVm", "V / IVm", "IVm",
               "V7 / I", "Im"}}
    };

    data_["harmonic_tree"] = std::move(td);
}

// ---------------------------------------------------------------------------
// Jazz tradition
// ---------------------------------------------------------------------------

void LookupProgression::init_jazz() {
    TraditionData td;
    td.description = "Jazz — Common chord progressions from the jazz tradition, "
                     "including ii-V-I patterns, turnarounds, and backdoor "
                     "progressions.";

    // --- Branches -----------------------------------------------------------

    td.branches = Table({"scaleIndex", "branch"});

    td.branches.data = {
        // Major (scaleIndex = 0)
        {0, std::string("I")},
        {0, std::string("IIm")},
        {0, std::string("IIIm")},
        {0, std::string("IV")},
        {0, std::string("V7")},
        {0, std::string("VIm")},
        {0, std::string("VIIdim")},
        {0, std::string("IVm")},
        {0, std::string("bVII")},
        {0, std::string("bVI")},
        {0, std::string("SUBV7")},
        {0, std::string("IIm7(b5)")},
        {0, std::string("V7 / IIm")},
        {0, std::string("V7 / IV")},
        {0, std::string("V7 / V")},
        {0, std::string("V7 / VI")},
        {0, std::string("#Idim")},
        {0, std::string("#IIdim")},

        // Minor (scaleIndex = 1)
        {1, std::string("Im")},
        {1, std::string("IIm7(b5)")},
        {1, std::string("bIII")},
        {1, std::string("IVm")},
        {1, std::string("V7")},
        {1, std::string("bVI")},
        {1, std::string("bVII")},
        {1, std::string("IV")},
        {1, std::string("V7 / V")}
    };

    // --- Paths --------------------------------------------------------------

    td.paths = Table({"scaleIndex", "branchOrigin", "branchTarget"});

    td.paths.data = {
        // Major (scaleIndex = 0) — common jazz voice leading
        {0, std::string("I"), std::string("I")},
        {0, std::string("I"), std::string("IIm")},
        {0, std::string("I"), std::string("IV")},
        {0, std::string("I"), std::string("VIm")},
        {0, std::string("I"), std::string("IIIm")},
        {0, std::string("I"), std::string("V7 / IIm")},
        {0, std::string("I"), std::string("V7 / IV")},
        {0, std::string("I"), std::string("V7 / VI")},

        // ii-V-I core
        {0, std::string("IIm"), std::string("V7")},
        {0, std::string("V7"), std::string("I")},

        // Turnaround: I-vi-ii-V
        {0, std::string("VIm"), std::string("IIm")},
        {0, std::string("IIm"), std::string("V7 / V")},

        // iii-vi-ii-V (extended turnaround)
        {0, std::string("IIIm"), std::string("VIm")},

        // IV paths
        {0, std::string("IV"), std::string("V7")},
        {0, std::string("IV"), std::string("IVm")},
        {0, std::string("IV"), std::string("IIm")},
        {0, std::string("I"), std::string("V7")},

        // Backdoor: IVm-bVII-I
        {0, std::string("IVm"), std::string("bVII")},
        {0, std::string("bVII"), std::string("I")},
        {0, std::string("IVm"), std::string("V7")},

        // Tritone sub
        {0, std::string("IIm"), std::string("SUBV7")},
        {0, std::string("SUBV7"), std::string("I")},

        // Secondary dominants
        {0, std::string("V7 / IIm"), std::string("IIm")},
        {0, std::string("V7 / IV"), std::string("IV")},
        {0, std::string("V7 / V"), std::string("V7")},
        {0, std::string("V7 / VI"), std::string("VIm")},

        // Diminished passing
        {0, std::string("#Idim"), std::string("IIm")},
        {0, std::string("#IIdim"), std::string("IIIm")},

        // Modal interchange
        {0, std::string("bVI"), std::string("I")},
        {0, std::string("bVI"), std::string("V7")},
        {0, std::string("IIm7(b5)"), std::string("V7")},
        {0, std::string("VIIdim"), std::string("I")},

        // Minor (scaleIndex = 1)
        {1, std::string("Im"), std::string("Im")},
        {1, std::string("Im"), std::string("IIm7(b5)")},
        {1, std::string("Im"), std::string("IVm")},
        {1, std::string("Im"), std::string("bVI")},
        {1, std::string("Im"), std::string("bVII")},
        {1, std::string("IIm7(b5)"), std::string("V7")},
        {1, std::string("V7"), std::string("Im")},
        {1, std::string("IVm"), std::string("V7")},
        {1, std::string("bVI"), std::string("bVII")},
        {1, std::string("bVII"), std::string("Im")},
        {1, std::string("bIII"), std::string("IVm")},
        {1, std::string("Im"), std::string("V7 / V")},
        {1, std::string("V7 / V"), std::string("V7")},
        {1, std::string("IV"), std::string("V7")},
        {1, std::string("Im"), std::string("IV")}
    };

    // --- Schemas ------------------------------------------------------------

    td.schemas = Table({"scaleIndex", "name", "description", "branches"});

    using SV = std::vector<std::string>;

    td.schemas.data = {
        // Major schemas
        {0, std::string("ii-V-I"),
            std::string("The fundamental jazz cadence"),
            SV{"IIm", "V7", "I"}},
        {0, std::string("turnaround"),
            std::string("Standard turnaround: I-vi-ii-V"),
            SV{"I", "VIm", "IIm", "V7"}},
        {0, std::string("backdoor"),
            std::string("Backdoor progression: IVm-bVII-I"),
            SV{"IVm", "bVII", "I"}},
        {0, std::string("tritone_sub"),
            std::string("Tritone substitution cadence"),
            SV{"IIm", "SUBV7", "I"}},
        {0, std::string("extended_turnaround"),
            std::string("Extended turnaround: iii-vi-ii-V"),
            SV{"IIIm", "VIm", "IIm", "V7"}},

        // Minor schemas
        {1, std::string("minor_ii-V-i"),
            std::string("Minor ii-V-i cadence"),
            SV{"IIm7(b5)", "V7", "Im"}},
        {1, std::string("minor_backdoor"),
            std::string("Minor backdoor: bVI-bVII-Im"),
            SV{"bVI", "bVII", "Im"}}
    };

    data_["jazz"] = std::move(td);
}

} // namespace gingo::internal
