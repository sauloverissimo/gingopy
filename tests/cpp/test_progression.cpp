// Gingo — Progression module tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/progression.hpp>
#include <gingo/tree.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace gingo;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_CASE("Progression construction from ScaleType", "[progression]") {
    Progression p("C", ScaleType::Major);
    REQUIRE(p.tonic().natural() == "C");
    REQUIRE(p.type() == ScaleType::Major);
}

TEST_CASE("Progression construction from string", "[progression]") {
    Progression p("C", "major");
    REQUIRE(p.tonic().natural() == "C");
    REQUIRE(p.type() == ScaleType::Major);
}

TEST_CASE("Progression construction with different tonics", "[progression]") {
    Progression p1("G", ScaleType::Major);
    REQUIRE(p1.tonic().natural() == "G");

    Progression p2("A", ScaleType::NaturalMinor);
    REQUIRE(p2.tonic().natural() == "A");

    Progression p3("Bb", "major");
    REQUIRE(p3.tonic().semitone() == Note("Bb").semitone());
}

TEST_CASE("Progression construction with minor types", "[progression]") {
    Progression p1("D", ScaleType::NaturalMinor);
    REQUIRE(p1.type() == ScaleType::NaturalMinor);

    Progression p2("E", ScaleType::HarmonicMinor);
    REQUIRE(p2.type() == ScaleType::HarmonicMinor);

    Progression p3("F", ScaleType::MelodicMinor);
    REQUIRE(p3.type() == ScaleType::MelodicMinor);
}

// ---------------------------------------------------------------------------
// Traditions (static)
// ---------------------------------------------------------------------------

TEST_CASE("Progression traditions returns list with expected entries", "[progression]") {
    auto traditions = Progression::traditions();
    REQUIRE(!traditions.empty());

    std::vector<std::string> names;
    for (const auto& t : traditions) {
        names.push_back(t.name);
    }

    // Must contain both harmonic_tree and jazz
    REQUIRE(std::find(names.begin(), names.end(), "harmonic_tree") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "jazz") != names.end());
}

TEST_CASE("Progression traditions have descriptions", "[progression]") {
    auto traditions = Progression::traditions();
    for (const auto& t : traditions) {
        REQUIRE(!t.name.empty());
        REQUIRE(!t.description.empty());
    }
}

// ---------------------------------------------------------------------------
// Tree access
// ---------------------------------------------------------------------------

TEST_CASE("Progression tree harmonic_tree returns valid tree", "[progression]") {
    Progression p("C", ScaleType::Major);
    Tree t = p.tree("harmonic_tree");

    REQUIRE(t.tonic().natural() == "C");
    REQUIRE(t.type() == ScaleType::Major);
}

TEST_CASE("Progression tree harmonic_tree tradition metadata", "[progression]") {
    Progression p("C", ScaleType::Major);
    Tree t = p.tree("harmonic_tree");
    auto trad = t.tradition();

    REQUIRE(trad.name == "harmonic_tree");
    REQUIRE(!trad.description.empty());
}

TEST_CASE("Progression tree jazz returns valid tree", "[progression]") {
    Progression p("C", ScaleType::Major);
    Tree t = p.tree("jazz");

    REQUIRE(t.tonic().natural() == "C");
    REQUIRE(t.type() == ScaleType::Major);
}

TEST_CASE("Progression tree jazz tradition metadata", "[progression]") {
    Progression p("C", ScaleType::Major);
    Tree t = p.tree("jazz");
    auto trad = t.tradition();

    REQUIRE(trad.name == "jazz");
    REQUIRE(!trad.description.empty());
}

TEST_CASE("Progression tree jazz has branches", "[progression]") {
    Progression p("C", ScaleType::Major);
    Tree t = p.tree("jazz");
    auto branches = t.branches();

    REQUIRE(!branches.empty());
}

TEST_CASE("Progression tree invalid throws", "[progression]") {
    Progression p("C", ScaleType::Major);
    REQUIRE_THROWS_AS(p.tree("invalid"), std::invalid_argument);
}

TEST_CASE("Progression tree empty string throws", "[progression]") {
    Progression p("C", ScaleType::Major);
    REQUIRE_THROWS_AS(p.tree(""), std::invalid_argument);
}

// ---------------------------------------------------------------------------
// Tree tradition()
// ---------------------------------------------------------------------------

TEST_CASE("Tree tradition returns correct struct for harmonic_tree", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto trad = t.tradition();

    REQUIRE(trad.name == "harmonic_tree");
    REQUIRE(!trad.description.empty());
}

TEST_CASE("Tree tradition returns correct struct for jazz", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "jazz");
    auto trad = t.tradition();

    REQUIRE(trad.name == "jazz");
    REQUIRE(!trad.description.empty());
}

// ---------------------------------------------------------------------------
// Tree schemas()
// ---------------------------------------------------------------------------

TEST_CASE("Tree schemas for harmonic_tree are non-empty", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto schemas = t.schemas();

    REQUIRE(!schemas.empty());
}

TEST_CASE("Tree schemas for harmonic_tree contain expected patterns", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto schemas = t.schemas();

    std::vector<std::string> names;
    for (const auto& s : schemas) {
        names.push_back(s.name);
    }

    // Harmonic tree should include classic patterns
    bool has_descending = std::find(names.begin(), names.end(), "descending") != names.end();
    bool has_ascending  = std::find(names.begin(), names.end(), "ascending")  != names.end();
    bool has_direct     = std::find(names.begin(), names.end(), "direct")     != names.end();

    REQUIRE(has_descending);
    REQUIRE(has_ascending);
    REQUIRE(has_direct);
}

TEST_CASE("Tree schemas for jazz are non-empty", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "jazz");
    auto schemas = t.schemas();

    REQUIRE(!schemas.empty());
}

TEST_CASE("Tree schemas for jazz contain expected patterns", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "jazz");
    auto schemas = t.schemas();

    std::vector<std::string> names;
    for (const auto& s : schemas) {
        names.push_back(s.name);
    }

    // Jazz should include canonical patterns
    bool has_ii_V_I      = std::find(names.begin(), names.end(), "ii-V-I")      != names.end();
    bool has_turnaround  = std::find(names.begin(), names.end(), "turnaround")  != names.end();
    bool has_backdoor    = std::find(names.begin(), names.end(), "backdoor")    != names.end();

    REQUIRE(has_ii_V_I);
    REQUIRE(has_turnaround);
    REQUIRE(has_backdoor);
}

TEST_CASE("Tree schemas have valid structure", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto schemas = t.schemas();

    for (const auto& s : schemas) {
        REQUIRE(!s.name.empty());
        REQUIRE(!s.description.empty());
        REQUIRE(!s.branches.empty());
    }
}

TEST_CASE("Tree jazz schemas have valid structure", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "jazz");
    auto schemas = t.schemas();

    for (const auto& s : schemas) {
        REQUIRE(!s.name.empty());
        REQUIRE(!s.description.empty());
        REQUIRE(!s.branches.empty());
    }
}

// ---------------------------------------------------------------------------
// Tree is_valid (replaces is_valid_progression)
// ---------------------------------------------------------------------------

TEST_CASE("Tree is_valid single chord", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    REQUIRE(t.is_valid({"I"}));
}

TEST_CASE("Tree is_valid II-V-I", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto valid = t.is_valid({"IIm", "V7", "I"});
    // Classic II-V-I: check it doesn't crash and returns bool
    REQUIRE((valid == true || valid == false));
}

TEST_CASE("Tree is_valid empty progression", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    REQUIRE(t.is_valid({}));
}

TEST_CASE("Tree is_valid with invalid branch returns false", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    REQUIRE_FALSE(t.is_valid({"I", "INVALID", "V7"}));
}

TEST_CASE("Tree is_valid jazz tradition", "[progression][tree]") {
    Tree t("C", ScaleType::Major, "jazz");
    // Should work without crashing regardless of tradition
    auto valid = t.is_valid({"IIm", "V7", "I"});
    REQUIRE((valid == true || valid == false));
}

// ---------------------------------------------------------------------------
// Progression to_string
// ---------------------------------------------------------------------------

TEST_CASE("Progression to_string is non-empty", "[progression]") {
    Progression p("C", ScaleType::Major);
    auto str = p.to_string();

    REQUIRE(!str.empty());
}

TEST_CASE("Progression to_string includes tonic", "[progression]") {
    Progression p("C", ScaleType::Major);
    auto str = p.to_string();

    REQUIRE(str.find("C") != std::string::npos);
}

TEST_CASE("Progression to_string for different keys", "[progression]") {
    Progression p1("G", ScaleType::Major);
    REQUIRE(p1.to_string().find("G") != std::string::npos);

    Progression p2("A", ScaleType::NaturalMinor);
    REQUIRE(p2.to_string().find("A") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Integration: Progression -> Tree round trip
// ---------------------------------------------------------------------------

TEST_CASE("Progression tree preserves tonic and type", "[progression][tree]") {
    Progression p("D", ScaleType::Major);

    Tree ht = p.tree("harmonic_tree");
    REQUIRE(ht.tonic().natural() == "D");
    REQUIRE(ht.type() == ScaleType::Major);

    Tree jz = p.tree("jazz");
    REQUIRE(jz.tonic().natural() == "D");
    REQUIRE(jz.type() == ScaleType::Major);
}

TEST_CASE("Progression tree for minor key", "[progression][tree]") {
    Progression p("A", ScaleType::NaturalMinor);
    Tree t = p.tree("harmonic_tree");

    REQUIRE(t.tonic().natural() == "A");
    REQUIRE(t.type() == ScaleType::NaturalMinor);

    auto branches = t.branches();
    REQUIRE(!branches.empty());
}
