// Gingo — Interval module tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/interval.hpp>
#include <gingo/note.hpp>

using namespace gingo;

TEST_CASE("Interval from label", "[interval]") {
    Interval p1("P1");
    REQUIRE(p1.label() == "P1");
    REQUIRE(p1.semitones() == 0);
    REQUIRE(p1.degree() == 1);

    Interval m3("3m");
    REQUIRE(m3.semitones() == 3);
    REQUIRE(m3.degree() == 3);

    Interval p5("5J");
    REQUIRE(p5.semitones() == 7);
    REQUIRE(p5.degree() == 5);

    Interval m7("7m");
    REQUIRE(m7.semitones() == 10);
    REQUIRE(m7.degree() == 7);
}

TEST_CASE("Interval from semitones", "[interval]") {
    Interval iv(7);
    REQUIRE(iv.label() == "5J");
    REQUIRE(iv.semitones() == 7);

    Interval iv2(0);
    REQUIRE(iv2.label() == "P1");
}

TEST_CASE("Interval anglo-saxon names", "[interval]") {
    REQUIRE(Interval("P1").anglo_saxon() == "P1");
    REQUIRE(Interval("3m").anglo_saxon() == "mi3");
    REQUIRE(Interval("3M").anglo_saxon() == "ma3");
    REQUIRE(Interval("5J").anglo_saxon() == "P5");
    REQUIRE(Interval("7M").anglo_saxon() == "ma7");
}

TEST_CASE("Interval octave", "[interval]") {
    REQUIRE(Interval("P1").octave() == 1);
    REQUIRE(Interval("7M").octave() == 1);
    REQUIRE(Interval("8J").octave() == 2);
    REQUIRE(Interval("9").octave() == 2);
}

TEST_CASE("Interval equality", "[interval]") {
    REQUIRE(Interval("5J") == Interval(7));
    REQUIRE(Interval("3m") != Interval("3M"));
}

TEST_CASE("All interval labels", "[interval]") {
    auto labels = Interval::all_labels();
    REQUIRE(labels.size() == 24);
    REQUIRE(labels[0] == "P1");
    REQUIRE(labels[7] == "5J");
    REQUIRE(labels[11] == "7M");
}

TEST_CASE("Invalid interval throws", "[interval]") {
    REQUIRE_THROWS_AS(Interval("INVALID"), std::invalid_argument);
    REQUIRE_THROWS_AS(Interval(-1), std::invalid_argument);
    REQUIRE_THROWS_AS(Interval(24), std::invalid_argument);
}

TEST_CASE("Interval from two notes", "[interval]") {
    REQUIRE(Interval(Note("C"), Note("G")).semitones() == 7);
    REQUIRE(Interval(Note("C"), Note("G")).label() == "5J");
    REQUIRE(Interval(Note("C"), Note("E")).semitones() == 4);
    REQUIRE(Interval(Note("G"), Note("C")).semitones() == 5);
    REQUIRE(Interval(Note("C"), Note("C")).semitones() == 0);
}

TEST_CASE("Interval simple and is_compound", "[interval]") {
    REQUIRE(Interval("5J").simple().semitones() == 7);
    REQUIRE_FALSE(Interval("5J").is_compound());
    REQUIRE(Interval("9").simple().semitones() == 2);
    REQUIRE(Interval("9").is_compound());
    REQUIRE(Interval("8J").simple().semitones() == 0);
    REQUIRE(Interval("8J").is_compound());
    REQUIRE(Interval("P1").simple().semitones() == 0);
    REQUIRE_FALSE(Interval("P1").is_compound());
}

TEST_CASE("Interval invert", "[interval]") {
    REQUIRE(Interval("5J").invert().semitones() == 5);     // P5 -> P4
    REQUIRE(Interval("3M").invert().semitones() == 8);     // M3 -> m6
    REQUIRE(Interval("4J").invert().semitones() == 7);     // P4 -> P5
    REQUIRE(Interval("d5").invert().semitones() == 6);     // tritone -> tritone
    REQUIRE(Interval("2m").invert().semitones() == 11);    // m2 -> M7
    REQUIRE(Interval("P1").invert().semitones() == 12);    // P1 -> P8
    REQUIRE(Interval("8J").invert().semitones() == 0);     // P8 -> P1
    // Compound: reduces first
    REQUIRE(Interval("9").invert().semitones() == 10);     // M9 -> m7
    REQUIRE(Interval("11").invert().semitones() == 7);     // P11 -> P5
}

TEST_CASE("Interval consonance", "[interval]") {
    REQUIRE(Interval("P1").consonance() == "perfect");
    REQUIRE(Interval("5J").consonance() == "perfect");
    REQUIRE(Interval("3m").consonance() == "imperfect");
    REQUIRE(Interval("3M").consonance() == "imperfect");
    REQUIRE(Interval("M6").consonance() == "imperfect");
    REQUIRE(Interval("2m").consonance() == "dissonant");
    REQUIRE(Interval("7M").consonance() == "dissonant");
    REQUIRE(Interval("d5").consonance() == "dissonant");
    // P4: dissonant by default, perfect with include_fourth
    REQUIRE(Interval("4J").consonance() == "dissonant");
    REQUIRE(Interval("4J").consonance(true) == "perfect");
}

TEST_CASE("Interval is_consonant", "[interval]") {
    REQUIRE(Interval("5J").is_consonant());
    REQUIRE(Interval("3M").is_consonant());
    REQUIRE_FALSE(Interval("2m").is_consonant());
    REQUIRE_FALSE(Interval("4J").is_consonant());
    REQUIRE(Interval("4J").is_consonant(true));
}

TEST_CASE("Interval full_name", "[interval]") {
    REQUIRE(Interval("P1").full_name() == "Perfect Unison");
    REQUIRE(Interval("3m").full_name() == "Minor Third");
    REQUIRE(Interval("5J").full_name() == "Perfect Fifth");
    REQUIRE(Interval("7M").full_name() == "Major Seventh");
    REQUIRE(Interval("d5").full_name() == "Tritone");
    REQUIRE(Interval("8J").full_name() == "Perfect Octave");
    REQUIRE(Interval("9").full_name() == "Major Ninth");
}

TEST_CASE("Interval full_name_pt", "[interval]") {
    REQUIRE(Interval("P1").full_name_pt() == "Unissono Justo");
    REQUIRE(Interval("3m").full_name_pt() == "Terca Menor");
    REQUIRE(Interval("5J").full_name_pt() == "Quinta Justa");
    REQUIRE(Interval("7M").full_name_pt() == "Setima Maior");
    REQUIRE(Interval("d5").full_name_pt() == "Tritono");
}

TEST_CASE("Interval addition", "[interval]") {
    auto result = Interval("3M") + Interval("3m");
    REQUIRE(result.semitones() == 7);
    REQUIRE(result.label() == "5J");
}

TEST_CASE("Interval subtraction", "[interval]") {
    auto result = Interval("5J") - Interval("3M");
    REQUIRE(result.semitones() == 3);
    REQUIRE(result.label() == "3m");
}

TEST_CASE("Interval addition overflow throws", "[interval]") {
    REQUIRE_THROWS_AS(Interval("8J") + Interval("8J"), std::overflow_error);
}

TEST_CASE("Interval subtraction underflow throws", "[interval]") {
    REQUIRE_THROWS_AS(Interval("3m") - Interval("5J"), std::underflow_error);
}
