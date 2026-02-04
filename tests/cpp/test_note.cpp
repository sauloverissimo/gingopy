// Gingo — Note module tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <gingo/note.hpp>

using namespace gingo;
using Catch::Matchers::WithinAbs;

TEST_CASE("Note construction and basic properties", "[note]") {
    Note c("C");
    REQUIRE(c.name() == "C");
    REQUIRE(c.natural() == "C");
    REQUIRE(c.sound() == "C");
    REQUIRE(c.semitone() == 0);

    Note fsharp("F#");
    REQUIRE(fsharp.natural() == "F#");
    REQUIRE(fsharp.semitone() == 6);
}

TEST_CASE("Enharmonic resolution", "[note]") {
    REQUIRE(Note("Bb").natural() == "A#");
    REQUIRE(Note("Db").natural() == "C#");
    REQUIRE(Note("Eb").natural() == "D#");
    REQUIRE(Note("Gb").natural() == "F#");
    REQUIRE(Note("Ab").natural() == "G#");
}

TEST_CASE("Double accidentals", "[note]") {
    REQUIRE(Note("G##").natural() == "A");
    REQUIRE(Note("Bbb").natural() == "A");
    REQUIRE(Note("Abb").natural() == "G");
    REQUIRE(Note("C##").natural() == "D");
    REQUIRE(Note("E##").natural() == "F#");
}

TEST_CASE("Unicode notation", "[note]") {
    REQUIRE(Note::to_natural("B♭") == "A#");
    REQUIRE(Note::to_natural("E♭♭") == "D");
    REQUIRE(Note::to_natural("♭♭G") == "F");
    REQUIRE(Note::to_natural("♭C") == "B");
}

TEST_CASE("Special enharmonics", "[note]") {
    REQUIRE(Note("E#").natural() == "F");
    REQUIRE(Note("B#").natural() == "C");
    REQUIRE(Note("Fb").natural() == "E");
    REQUIRE(Note("Cb").natural() == "B");
}

TEST_CASE("Note frequency", "[note]") {
    REQUIRE_THAT(Note("A").frequency(4), WithinAbs(440.0, 0.01));
    REQUIRE_THAT(Note("C").frequency(4), WithinAbs(261.63, 0.1));
    REQUIRE_THAT(Note("A").frequency(3), WithinAbs(220.0, 0.01));
}

TEST_CASE("Enharmonic equivalence", "[note]") {
    REQUIRE(Note("Bb").is_enharmonic(Note("A#")));
    REQUIRE(Note("Db").is_enharmonic(Note("C#")));
    REQUIRE_FALSE(Note("C").is_enharmonic(Note("D")));
}

TEST_CASE("Note equality", "[note]") {
    REQUIRE(Note("C") == Note("C"));
    REQUIRE(Note("Bb") == Note("A#"));  // Same natural form
    REQUIRE(Note("C") != Note("D"));
}

TEST_CASE("Note transposition", "[note]") {
    REQUIRE(Note("C").transpose(7).natural() == "G");
    REQUIRE(Note("C").transpose(12).natural() == "C");
    REQUIRE(Note("A").transpose(-2).natural() == "G");
}

TEST_CASE("Extract root from chord name", "[note]") {
    REQUIRE(Note::extract_root("C") == "C");
    REQUIRE(Note::extract_root("C#m7") == "C#");
    REQUIRE(Note::extract_root("Bbdim") == "Bb");
    REQUIRE(Note::extract_root("A#7") == "A#");
}

TEST_CASE("Extract sound from chord name", "[note]") {
    REQUIRE(Note::extract_sound("G") == "G");
    REQUIRE(Note::extract_sound("Gb") == "G");
    REQUIRE(Note::extract_sound("G#m7") == "G");
}

TEST_CASE("Extract chord type", "[note]") {
    REQUIRE(Note::extract_type("A#7") == "7");
    REQUIRE(Note::extract_type("Cm7") == "m7");
    REQUIRE(Note::extract_type("Bbdim") == "dim");
    REQUIRE(Note::extract_type("C") == "");
    REQUIRE(Note::extract_type("F#m7(b5)") == "m7(b5)");
}

// ---------------------------------------------------------------------------
// Circle of fifths
// ---------------------------------------------------------------------------

TEST_CASE("Note fifths returns 12 notes in fifth order", "[note][fifths]") {
    const auto& f = Note::fifths();
    REQUIRE(f.size() == 12);
    REQUIRE(f[0] == "C");
    REQUIRE(f[1] == "G");
    REQUIRE(f[2] == "D");
    REQUIRE(f[3] == "A");
    REQUIRE(f[4] == "E");
    REQUIRE(f[5] == "B");
    REQUIRE(f[6] == "F#");
    REQUIRE(f[11] == "F");
}

TEST_CASE("Note distance on circle of fifths", "[note][fifths]") {
    // Same note
    REQUIRE(Note("C").distance(Note("C")) == 0);

    // Adjacent: perfect fifth
    REQUIRE(Note("C").distance(Note("G")) == 1);

    // Adjacent: perfect fourth (one fifth in the other direction)
    REQUIRE(Note("C").distance(Note("F")) == 1);

    // Two fifths apart
    REQUIRE(Note("C").distance(Note("D")) == 2);

    // Maximum distance: tritone
    REQUIRE(Note("C").distance(Note("F#")) == 6);

    // Enharmonic equivalence
    REQUIRE(Note("Bb").distance(Note("A#")) == 0);

    // Bb (=A#) to C: A# is at position 10, C at 0 → min(10, 2) = 2
    REQUIRE(Note("C").distance(Note("Bb")) == 2);
}
