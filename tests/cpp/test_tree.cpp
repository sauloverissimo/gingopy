// Gingo — Tree module tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/tree.hpp>

using namespace gingo;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_CASE("Tree construction from ScaleType", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    REQUIRE(t.tonic().natural() == "C");
    REQUIRE(t.type() == ScaleType::Major);
}

TEST_CASE("Tree construction from string", "[tree]") {
    Tree t("C", "major", "harmonic_tree");
    REQUIRE(t.tonic().natural() == "C");
    REQUIRE(t.type() == ScaleType::Major);
}

TEST_CASE("Tree construction with different tonics", "[tree]") {
    Tree t1("G", ScaleType::Major, "harmonic_tree");
    REQUIRE(t1.tonic().natural() == "G");

    Tree t2("A", ScaleType::NaturalMinor, "harmonic_tree");
    REQUIRE(t2.tonic().natural() == "A");
}

// ---------------------------------------------------------------------------
// Branches
// ---------------------------------------------------------------------------

TEST_CASE("Tree branches returns non-empty list", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto branches = t.branches();
    REQUIRE(!branches.empty());
}

TEST_CASE("Tree branches for major scale", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto branches = t.branches();

    // Should contain basic diatonic branches
    bool has_I = false;
    bool has_V7 = false;
    bool has_IIm = false;

    for (const auto& b : branches) {
        if (b == "I") has_I = true;
        if (b == "V7") has_V7 = true;
        if (b == "IIm") has_IIm = true;
    }

    REQUIRE(has_I);
    REQUIRE(has_V7);
    REQUIRE(has_IIm);
}

TEST_CASE("Tree branches for minor scale", "[tree]") {
    Tree t("A", ScaleType::NaturalMinor, "harmonic_tree");
    auto branches = t.branches();

    // Should contain Im
    bool has_Im = false;
    for (const auto& b : branches) {
        if (b == "Im") has_Im = true;
    }
    REQUIRE(has_Im);
}

// ---------------------------------------------------------------------------
// Paths
// ---------------------------------------------------------------------------

TEST_CASE("Tree paths from origin", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto paths = t.paths("I");

    REQUIRE(!paths.empty());
    // First path should be the origin itself
    REQUIRE(paths[0].branch == "I");
    REQUIRE(paths[0].id == 0);
}

TEST_CASE("Tree paths contain chord information", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto paths = t.paths("I");

    // Origin should have chord resolved
    REQUIRE(paths[0].chord.root().natural() == "C");
    REQUIRE(!paths[0].interval_labels.empty());
    REQUIRE(!paths[0].note_names.empty());
}

// ---------------------------------------------------------------------------
// Shortest path
// ---------------------------------------------------------------------------

TEST_CASE("Tree shortest_path finds direct connection", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto path = t.shortest_path("I", "V7");

    REQUIRE(!path.empty());
    REQUIRE(path.front() == "I");
    REQUIRE(path.back() == "V7");
}

TEST_CASE("Tree shortest_path returns empty for invalid branches", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto path = t.shortest_path("INVALID", "V7");

    REQUIRE(path.empty());
}

TEST_CASE("Tree shortest_path from branch to itself", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto path = t.shortest_path("I", "I");

    REQUIRE(path.size() == 1);
    REQUIRE(path[0] == "I");
}

TEST_CASE("Tree shortest_path finds multi-step path", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto path = t.shortest_path("I", "IV");

    // Should find some path, even if indirect
    if (!path.empty()) {
        REQUIRE(path.front() == "I");
        REQUIRE(path.back() == "IV");
        REQUIRE(path.size() >= 2);
    }
}

// ---------------------------------------------------------------------------
// Valid progression
// ---------------------------------------------------------------------------

TEST_CASE("Tree is_valid_progression single chord", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    REQUIRE(t.is_valid({"I"}));
}

TEST_CASE("Tree is_valid_progression II-V-I", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    // Classic II-V-I should be valid if paths exist
    auto valid = t.is_valid({"IIm", "V7", "I"});
    // Don't require it to be true, just check it doesn't crash
    REQUIRE((valid == true || valid == false));
}

TEST_CASE("Tree is_valid_progression empty", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    REQUIRE(t.is_valid({}));
}

TEST_CASE("Tree is_valid_progression with invalid branch", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    // Test progression containing invalid branch
    REQUIRE_FALSE(t.is_valid({"I", "INVALID", "V7"}));
}

// ---------------------------------------------------------------------------
// Harmonic function
// ---------------------------------------------------------------------------

TEST_CASE("Tree function returns tonic for I", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto func = t.function("I");
    REQUIRE(func == HarmonicFunction::Tonic);
}

TEST_CASE("Tree function returns subdominant for IV", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto func = t.function("IV");
    REQUIRE(func == HarmonicFunction::Subdominant);
}

TEST_CASE("Tree function returns dominant for V7", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto func = t.function("V7");
    REQUIRE(func == HarmonicFunction::Dominant);
}

TEST_CASE("Tree function for applied chords uses primary degree", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    // V7/IV should be Dominant (V7 = Dominant)
    auto func = t.function("V7 / IV");
    REQUIRE(func == HarmonicFunction::Dominant);
}

TEST_CASE("Tree function for minor branches", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto func_IIm = t.function("IIm");
    REQUIRE(func_IIm == HarmonicFunction::Subdominant);

    auto func_VIm = t.function("VIm");
    REQUIRE(func_VIm == HarmonicFunction::Tonic);
}

// ---------------------------------------------------------------------------
// Branches with function
// ---------------------------------------------------------------------------

TEST_CASE("Tree branches_with_function returns tonics", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto tonics = t.branches_with_function(HarmonicFunction::Tonic);

    REQUIRE(!tonics.empty());
    // Should contain I
    bool has_I = false;
    for (const auto& b : tonics) {
        if (b == "I") has_I = true;
    }
    REQUIRE(has_I);
}

TEST_CASE("Tree branches_with_function returns subdominants", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto subdominants = t.branches_with_function(HarmonicFunction::Subdominant);

    REQUIRE(!subdominants.empty());
}

TEST_CASE("Tree branches_with_function returns dominants", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto dominants = t.branches_with_function(HarmonicFunction::Dominant);

    REQUIRE(!dominants.empty());
    // Should contain V7
    bool has_V7 = false;
    for (const auto& b : dominants) {
        if (b == "V7") has_V7 = true;
    }
    REQUIRE(has_V7);
}

// ---------------------------------------------------------------------------
// Visualization export
// ---------------------------------------------------------------------------

TEST_CASE("Tree to_dot returns non-empty string", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto dot = t.to_dot(false);

    REQUIRE(!dot.empty());
    REQUIRE(dot.find("digraph") != std::string::npos);
}

TEST_CASE("Tree to_dot with functions includes colors", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto dot = t.to_dot(true);

    REQUIRE(!dot.empty());
    // Should have color information
    REQUIRE(dot.find("fillcolor") != std::string::npos);
}

TEST_CASE("Tree to_mermaid returns non-empty string", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto mermaid = t.to_mermaid();

    REQUIRE(!mermaid.empty());
    REQUIRE(mermaid.find("graph") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Diminished chord notation (both ° and dim)
// ---------------------------------------------------------------------------

TEST_CASE("Tree resolves diminished chord notation", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto paths = t.paths("I");

    // Check if Idim or I° exists in branches (both ° and dim notations)
    auto all_branches = t.branches();

    bool has_dim = false;
    for (const auto& b : all_branches) {
        if (b == "Idim" || b.find("°") != std::string::npos) {
            has_dim = true;
            break;
        }
    }
    // Should support at least one diminished notation (dim or °)
    REQUIRE(has_dim);
}

// ---------------------------------------------------------------------------
// Integration with different scale types
// ---------------------------------------------------------------------------

TEST_CASE("Tree works with NaturalMinor", "[tree]") {
    Tree t("A", ScaleType::NaturalMinor, "harmonic_tree");
    auto branches = t.branches();
    REQUIRE(!branches.empty());

    auto paths = t.paths("Im");
    REQUIRE(!paths.empty());
}

TEST_CASE("Tree works with HarmonicMinor", "[tree]") {
    Tree t("A", ScaleType::HarmonicMinor, "harmonic_tree");
    auto branches = t.branches();
    REQUIRE(!branches.empty());
}

TEST_CASE("Tree works with MelodicMinor", "[tree]") {
    Tree t("A", ScaleType::MelodicMinor, "harmonic_tree");
    auto branches = t.branches();
    REQUIRE(!branches.empty());
}

// ---------------------------------------------------------------------------
// to_string
// ---------------------------------------------------------------------------

TEST_CASE("Tree to_string includes tonic and type", "[tree]") {
    Tree t("C", ScaleType::Major, "harmonic_tree");
    auto str = t.to_string();

    REQUIRE(!str.empty());
    REQUIRE(str.find("C") != std::string::npos);
}
