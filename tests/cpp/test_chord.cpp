// Gingo — Chord module tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/chord.hpp>

#include <algorithm>
#include <string>
#include <vector>

using namespace gingo;

static std::vector<std::string> note_names(const std::vector<Note>& notes) {
    std::vector<std::string> names;
    for (const auto& n : notes) names.push_back(n.natural());
    return names;
}

TEST_CASE("Major triad", "[chord]") {
    Chord c("CM");
    REQUIRE(c.root().natural() == "C");
    REQUIRE(c.type() == "M");
    auto names = note_names(c.notes());
    REQUIRE(names == std::vector<std::string>{"C", "E", "G"});
}

TEST_CASE("Minor seventh chord", "[chord]") {
    Chord c("Am7");
    REQUIRE(c.root().natural() == "A");
    REQUIRE(c.type() == "m7");
    REQUIRE(c.size() == 4);
    auto labels = c.interval_labels();
    REQUIRE(labels == std::vector<std::string>{"P1", "3m", "5J", "7m"});
}

TEST_CASE("Chord with flat root", "[chord]") {
    Chord c("Dbm7");
    REQUIRE(c.root().natural() == "C#");
    auto names = note_names(c.notes());
    REQUIRE(names == std::vector<std::string>{"C#", "E", "G#", "B"});
}

TEST_CASE("Chord with sharp root", "[chord]") {
    Chord c("A#m7");
    auto names = note_names(c.notes());
    REQUIRE(names == std::vector<std::string>{"A#", "C#", "F", "G#"});
}

TEST_CASE("Diminished chord", "[chord]") {
    Chord c("Bdim");
    REQUIRE(c.type() == "dim");
    auto labels = c.interval_labels();
    REQUIRE(labels == std::vector<std::string>{"P1", "3m", "d5"});
}

TEST_CASE("Augmented chord", "[chord]") {
    Chord c("Caug");
    auto labels = c.interval_labels();
    REQUIRE(labels == std::vector<std::string>{"P1", "3M", "#5"});
}

TEST_CASE("Dominant seventh", "[chord]") {
    Chord c("G7");
    REQUIRE(c.type() == "7");
    REQUIRE(c.size() == 4);
}

TEST_CASE("Chord contains note", "[chord]") {
    Chord c("CM");
    REQUIRE(c.contains(Note("E")));
    REQUIRE(c.contains(Note("G")));
    REQUIRE_FALSE(c.contains(Note("F")));
}

TEST_CASE("Identify chord from notes", "[chord]") {
    auto c = Chord::identify({"C", "E", "G"});
    REQUIRE(c.type() == "M");
    REQUIRE(c.root().natural() == "C");
}

TEST_CASE("Chord equality", "[chord]") {
    REQUIRE(Chord("CM") == Chord("CM"));
    REQUIRE(Chord("CM") != Chord("Cm"));
}
