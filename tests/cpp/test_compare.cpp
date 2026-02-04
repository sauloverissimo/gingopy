// Gingo — Chord/Field comparison tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/chord.hpp>
#include <gingo/field.hpp>

using namespace gingo;

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------

static std::vector<std::string> note_names(const std::vector<Note>& notes) {
    std::vector<std::string> names;
    for (const auto& n : notes) names.push_back(n.natural());
    return names;
}

// ---------------------------------------------------------------------------
// ChordComparison tests
// ---------------------------------------------------------------------------

TEST_CASE("Compare identical chords", "[compare]") {
    auto r = Chord("CM").compare(Chord("CM"));

    REQUIRE(r.common_notes.size() == 3);
    REQUIRE(r.exclusive_a.empty());
    REQUIRE(r.exclusive_b.empty());
    REQUIRE(r.root_distance == 0);
    REQUIRE(r.root_direction == 0);
    REQUIRE(r.same_quality == true);
    REQUIRE(r.same_size == true);
    REQUIRE(r.enharmonic == true);
    REQUIRE(r.subset == "equal");
    REQUIRE(r.voice_leading == 0);
    REQUIRE(r.transformation == "");
    REQUIRE(r.inversion == false);
}

TEST_CASE("Compare CM and Am -- relative (R)", "[compare]") {
    auto r = Chord("CM").compare(Chord("Am"));

    // C-E-G vs A-C-E → common: C, E
    REQUIRE(r.common_notes.size() == 2);
    REQUIRE(r.exclusive_a.size() == 1);   // G
    REQUIRE(r.exclusive_b.size() == 1);   // A
    REQUIRE(r.root_distance == 3);
    REQUIRE(r.same_quality == false);
    REQUIRE(r.same_size == true);
    REQUIRE(r.transformation == "R");
}

TEST_CASE("Compare CM and Cm -- parallel (P)", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cm"));

    // C-E-G vs C-D#-G → common: C, G
    REQUIRE(r.common_notes.size() == 2);
    REQUIRE(r.root_distance == 0);
    REQUIRE(r.root_direction == 0);
    REQUIRE(r.same_quality == false);
    REQUIRE(r.transformation == "P");
}

TEST_CASE("Compare CM and Em -- leading-tone (L)", "[compare]") {
    auto r = Chord("CM").compare(Chord("Em"));

    // C→E = 4 semitones, both triads, major↔minor
    REQUIRE(r.root_distance == 4);
    REQUIRE(r.transformation == "L");
}

TEST_CASE("Compare different sizes -- voice leading undefined", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cm7"));

    REQUIRE(r.same_size == false);
    REQUIRE(r.voice_leading == -1);
}

TEST_CASE("Enharmonic equivalence", "[compare]") {
    auto r = Chord("C#M").compare(Chord("C#M"));

    REQUIRE(r.enharmonic == true);
    REQUIRE(r.inversion == false);
}

TEST_CASE("Subset relationship -- CM subset of CM7", "[compare]") {
    auto r = Chord("CM").compare(Chord("C7M"));

    // CM = {C,E,G}, C7M = {C,E,G,B}
    REQUIRE(r.subset == "a_subset_of_b");
}

TEST_CASE("Root direction signed", "[compare]") {
    auto r = Chord("CM").compare(Chord("FM"));

    // C→F = +5 semitones
    REQUIRE(r.root_direction == 5);
    REQUIRE(r.root_distance == 5);
}

TEST_CASE("Root direction negative (shortest path)", "[compare]") {
    auto r = Chord("CM").compare(Chord("BM"));

    // C→B = -1 (shortest path down)
    REQUIRE(r.root_direction == -1);
    REQUIRE(r.root_distance == 1);
}

TEST_CASE("Common interval labels", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cm"));

    // Both have P1 and 5J, different 3rd
    REQUIRE(r.common_intervals.size() == 2);
}

TEST_CASE("Voice leading CM to Dm", "[compare]") {
    auto r = Chord("CM").compare(Chord("Dm"));

    // C→D(2) + E→D(1) + G→F(1) = minimum is some permutation
    // Optimal: C→D(2), E→F(1), G→A(2) = 5? or C→F(5), E→D(1), G→A(2)...
    // Actually brute force finds the optimal pairing
    REQUIRE(r.voice_leading >= 0);
    REQUIRE(r.voice_leading <= 6);
}

TEST_CASE("No transformation for non-triads", "[compare]") {
    auto r = Chord("C7M").compare(Chord("Am7"));

    // 4-note chords — neo-Riemannian only for triads
    REQUIRE(r.transformation == "");
}

// ---------------------------------------------------------------------------
// FieldComparison tests
// ---------------------------------------------------------------------------

TEST_CASE("Field compare diatonic chords CM and GM", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("GM"));

    REQUIRE(r.degree_a.has_value());
    REQUIRE(*r.degree_a == 1);
    REQUIRE(r.degree_b.has_value());
    REQUIRE(*r.degree_b == 5);
    REQUIRE(r.function_a.has_value());
    REQUIRE(*r.function_a == HarmonicFunction::Tonic);
    REQUIRE(r.function_b.has_value());
    REQUIRE(*r.function_b == HarmonicFunction::Dominant);
    REQUIRE(r.same_function.has_value());
    REQUIRE(*r.same_function == false);
    REQUIRE(r.diatonic_a == true);
    REQUIRE(r.diatonic_b == true);
    REQUIRE(r.foreign_a.empty());
    REQUIRE(r.foreign_b.empty());
}

TEST_CASE("Field compare relative pair CM and Am", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("Am"));

    REQUIRE(r.relative == true);
    REQUIRE(r.same_function.has_value());
    REQUIRE(*r.same_function == true);
}

TEST_CASE("Field compare non-diatonic chord", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("F#M"));

    REQUIRE(r.diatonic_a == true);
    REQUIRE(r.diatonic_b == false);
    REQUIRE(!r.degree_b.has_value());
    REQUIRE(!r.function_b.has_value());
}

TEST_CASE("Field compare secondary dominant", "[compare]") {
    Field f("C", ScaleType::Major);
    // D7 root = D (semitone 2), G root = G (semitone 7)
    // D→G: (2-7+12)%12 = 7 → D7 is V7 of G
    auto r = f.compare(Chord("D7"), Chord("GM"));

    REQUIRE(r.secondary_dominant == "a_is_V7_of_b");
}

TEST_CASE("Field compare tritone substitution", "[compare]") {
    Field f("C", ScaleType::Major);
    // G7 and Db7: root distance = |7-1| = 6 (tritone)
    auto r = f.compare(Chord("G7"), Chord("C#7"));

    REQUIRE(r.tritone_sub == true);
}

TEST_CASE("Field compare pivot fields", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("Am"));

    REQUIRE(!r.pivot.empty());
    bool found_c_major = false;
    for (const auto& p : r.pivot) {
        if (p.tonic == "C" && p.scale_type == "Major") {
            found_c_major = true;
            REQUIRE(p.degree_a == 1);
            REQUIRE(p.degree_b == 6);
        }
    }
    REQUIRE(found_c_major);
}

TEST_CASE("Field compare borrowed chord Fm in C Major", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("Fm"));

    REQUIRE(r.diatonic_b == false);
    REQUIRE(r.borrowed_b.has_value());
    REQUIRE(r.borrowed_b->scale_type == "NaturalMinor");
    REQUIRE(r.borrowed_b->degree == 4);
}

TEST_CASE("Field compare foreign notes", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("Fm"));

    // CM notes are all in C major
    REQUIRE(r.foreign_a.empty());
    // Fm has Ab which is not in C major
    REQUIRE(!r.foreign_b.empty());
}

TEST_CASE("Field compare degree distance", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("GM"));

    REQUIRE(r.degree_distance.has_value());
    REQUIRE(*r.degree_distance == 4);  // |1 - 5| = 4
}

TEST_CASE("Field compare chromatic mediant", "[compare]") {
    Field f("C", ScaleType::Major);
    // C→E: root distance = 4 (major third), ascending
    auto r = f.compare(Chord("CM"), Chord("EM"));

    REQUIRE(r.chromatic_mediant == "upper");
}

// ---------------------------------------------------------------------------
// Interval Vector (Forte set theory)
// ---------------------------------------------------------------------------

TEST_CASE("Interval vector CM and Cm -- same vector (Z-relation)", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cm"));

    // CM = {0,4,7}: (0,4)=4, (0,7)=5, (4,7)=3 → [0,0,1,1,1,0]
    // Cm = {0,3,7}: (0,3)=3, (0,7)=5, (3,7)=4 → [0,0,1,1,1,0]
    REQUIRE(r.interval_vector_a == std::vector<int>{0, 0, 1, 1, 1, 0});
    REQUIRE(r.interval_vector_b == std::vector<int>{0, 0, 1, 1, 1, 0});
    REQUIRE(r.same_interval_vector == true);
}

TEST_CASE("Interval vector CM and Caug -- different vectors", "[compare]") {
    auto r = Chord("CM").compare(Chord("Caug"));

    // Caug = {0,4,8}: (0,4)=4, (0,8)=4, (4,8)=4 → [0,0,0,3,0,0]
    REQUIRE(r.interval_vector_a == std::vector<int>{0, 0, 1, 1, 1, 0});
    REQUIRE(r.interval_vector_b == std::vector<int>{0, 0, 0, 3, 0, 0});
    REQUIRE(r.same_interval_vector == false);
}

TEST_CASE("Interval vector C7M -- seventh chord", "[compare]") {
    auto r = Chord("C7M").compare(Chord("C7M"));

    // C7M = {0,4,7,11}: 6 pairs
    // (0,4)=4, (0,7)=5, (0,11)=1, (4,7)=3, (4,11)=5, (7,11)=4
    // → [1,0,1,2,2,0]
    REQUIRE(r.interval_vector_a == std::vector<int>{1, 0, 1, 2, 2, 0});
    REQUIRE(r.same_interval_vector == true);
}

// ---------------------------------------------------------------------------
// Transposition detection (Lewin T_n)
// ---------------------------------------------------------------------------

TEST_CASE("Transposition CM to DM is T2", "[compare]") {
    auto r = Chord("CM").compare(Chord("DM"));

    REQUIRE(r.transposition == 2);
}

TEST_CASE("Transposition CM to Cm is not transposition", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cm"));

    REQUIRE(r.transposition == -1);
}

TEST_CASE("Transposition identical chords is T0", "[compare]") {
    auto r = Chord("CM").compare(Chord("CM"));

    REQUIRE(r.transposition == 0);
}

TEST_CASE("Transposition different sizes is -1", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cm7"));

    REQUIRE(r.transposition == -1);
}

TEST_CASE("Transposition Am7 to Cm7 is T3", "[compare]") {
    auto r = Chord("Am7").compare(Chord("Cm7"));

    REQUIRE(r.transposition == 3);
}

// ---------------------------------------------------------------------------
// Neo-Riemannian 2-step compositions
// ---------------------------------------------------------------------------

TEST_CASE("Neo-Riemannian composition RP: CM to AM", "[compare]") {
    // R(CM) = Am (root 9, minor), P(Am) = AM (root 9, major)
    auto r = Chord("CM").compare(Chord("AM"));

    REQUIRE(r.transformation == "RP");
}

TEST_CASE("Neo-Riemannian composition LP: CM to EM", "[compare]") {
    // L(CM) = Em (root 4, minor), P(Em) = EM (root 4, major)
    auto r = Chord("CM").compare(Chord("EM"));

    REQUIRE(r.transformation == "LP");
}

TEST_CASE("Neo-Riemannian composition PL: CM to G#M", "[compare]") {
    // P(CM) = Cm (root 0, minor), L(Cm) = G#M (root 8, major)
    auto r = Chord("CM").compare(Chord("G#M"));

    REQUIRE(r.transformation == "PL");
}

TEST_CASE("Neo-Riemannian composition PR: CM to EbM", "[compare]") {
    // P(CM) = Cm (root 0, minor), R(Cm) = EbM (root 3, major)
    auto r = Chord("CM").compare(Chord("D#M"));

    REQUIRE(r.transformation == "PR");
}

TEST_CASE("Single P, L, R still work after composition extension", "[compare]") {
    REQUIRE(Chord("CM").compare(Chord("Cm")).transformation == "P");
    REQUIRE(Chord("CM").compare(Chord("Em")).transformation == "L");
    REQUIRE(Chord("CM").compare(Chord("Am")).transformation == "R");
}

// ---------------------------------------------------------------------------
// Dissonance (Sethares / Plomp-Levelt)
// ---------------------------------------------------------------------------

TEST_CASE("Dissonance scores are non-negative", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cm7"));

    REQUIRE(r.dissonance_a >= 0.0);
    REQUIRE(r.dissonance_b >= 0.0);
}

TEST_CASE("Dissonance identical chords have equal scores", "[compare]") {
    auto r = Chord("CM").compare(Chord("CM"));

    REQUIRE(r.dissonance_a == r.dissonance_b);
}

TEST_CASE("Dissonance diminished higher than major triad", "[compare]") {
    auto r = Chord("CM").compare(Chord("Cdim"));

    // dim has a tritone (ic6) — higher roughness expected
    REQUIRE(r.dissonance_b > r.dissonance_a);
}

// ---------------------------------------------------------------------------
// Root Motion (Field)
// ---------------------------------------------------------------------------

TEST_CASE("Field root motion descending fifth (V to I)", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("GM"), Chord("CM"));

    REQUIRE(r.root_motion == "descending_fifth");
}

TEST_CASE("Field root motion ascending fifth (I to V)", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("GM"));

    REQUIRE(r.root_motion == "ascending_fifth");
}

TEST_CASE("Field root motion ascending step (IV to V)", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("FM"), Chord("GM"));

    REQUIRE(r.root_motion == "ascending_step");
}

TEST_CASE("Field root motion descending third (I to vi)", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("Am"));

    REQUIRE(r.root_motion == "descending_third");
}

TEST_CASE("Field root motion unison", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("CM"));

    REQUIRE(r.root_motion == "unison");
}

TEST_CASE("Field root motion empty for non-diatonic", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("F#M"));

    REQUIRE(r.root_motion == "");
}

// ---------------------------------------------------------------------------
// Applied Diminished (Field)
// ---------------------------------------------------------------------------

TEST_CASE("Field applied diminished Bdim to CM", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("Bdim"), Chord("CM"));

    REQUIRE(r.applied_diminished == "a_is_viidim_of_b");
}

TEST_CASE("Field applied diminished reversed CM to Bdim", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("Bdim"));

    REQUIRE(r.applied_diminished == "b_is_viidim_of_a");
}

TEST_CASE("Field applied diminished m7b5", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("F#m7(b5)"), Chord("GM"));

    REQUIRE(r.applied_diminished == "a_is_viidim_of_b");
}

TEST_CASE("Field applied diminished no match", "[compare]") {
    Field f("C", ScaleType::Major);
    auto r = f.compare(Chord("CM"), Chord("GM"));

    REQUIRE(r.applied_diminished == "");
}

TEST_CASE("Field applied diminished non-dim chord rejected", "[compare]") {
    Field f("C", ScaleType::Major);
    // Bm is minor, not diminished — root 1 semitone below C
    auto r = f.compare(Chord("Bm"), Chord("CM"));

    REQUIRE(r.applied_diminished == "");
}
