// Gingo — Field module tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/field.hpp>

using namespace gingo;

TEST_CASE("Field construction", "[field]") {
    Field f("C", ScaleType::Major);
    REQUIRE(f.tonic().natural() == "C");
    REQUIRE(f.size() == 7);
}

TEST_CASE("Field from string", "[field]") {
    Field f("C", "major");
    REQUIRE(f.size() == 7);
}

TEST_CASE("Field triads count", "[field]") {
    Field f("C", ScaleType::Major);
    auto chords = f.chords();
    REQUIRE(chords.size() == 7);
}

TEST_CASE("Field sevenths count", "[field]") {
    Field f("C", ScaleType::Major);
    auto chords = f.sevenths();
    REQUIRE(chords.size() == 7);
}

TEST_CASE("Field chord at degree", "[field]") {
    Field f("C", ScaleType::Major);
    auto c1 = f.chord(1);
    REQUIRE(c1.root().natural() == "C");
}

TEST_CASE("Field chord at invalid degree throws", "[field]") {
    Field f("C", ScaleType::Major);
    REQUIRE_THROWS(f.chord(0));
    REQUIRE_THROWS(f.chord(8));
}

// ---------------------------------------------------------------------------
// Applied chords (tonicization)
// ---------------------------------------------------------------------------

TEST_CASE("Field applied string + int: secondary dominants", "[field]") {
    Field f("C", ScaleType::Major);

    // V7/II  → V7 of D major = A7
    auto c = f.applied("V7", 2);
    REQUIRE(c.root().natural() == "A");
    REQUIRE(c.type() == "7");

    // V7/IV  → V7 of F major = C7
    c = f.applied("V7", 4);
    REQUIRE(c.root().natural() == "C");
    REQUIRE(c.type() == "7");

    // V7/V   → V7 of G major = D7
    c = f.applied("V7", 5);
    REQUIRE(c.root().natural() == "D");
    REQUIRE(c.type() == "7");
}

TEST_CASE("Field applied string + int: complex quality", "[field]") {
    Field f("C", ScaleType::Major);

    // IIm7(b5)/V  → II of G major = A, quality m7(b5) → Am7(b5)
    auto c = f.applied("IIm7(b5)", 5);
    REQUIRE(c.root().natural() == "A");
    REQUIRE(c.type() == "m7(b5)");
}

TEST_CASE("Field applied string + int: no quality returns triad", "[field]") {
    Field f("C", ScaleType::Major);

    // V/II  → triad at V of D major = AM
    auto c = f.applied("V", 2);
    REQUIRE(c.root().natural() == "A");
}

TEST_CASE("Field applied with accidental", "[field]") {
    Field f("C", ScaleType::Major);

    // bVII7/V  → VII of G major = F#, flatten → F, quality 7 → F7
    auto c = f.applied("bVII7", 5);
    REQUIRE(c.root().natural() == "F");
    REQUIRE(c.type() == "7");

    // #IV7/I  → IV of C major = F, sharpen → F#, quality 7 → F#7
    c = f.applied("#IV7", 1);
    REQUIRE(c.root().natural() == "F#");
    REQUIRE(c.type() == "7");
}

TEST_CASE("Field applied string + string: Roman numeral target", "[field]") {
    Field f("C", ScaleType::Major);

    // V7 / IIm  → V7 of degree II = A7
    auto c = f.applied("V7", "IIm");
    REQUIRE(c.root().natural() == "A");
    REQUIRE(c.type() == "7");

    // V7 / V  → D7
    c = f.applied("V7", "V");
    REQUIRE(c.root().natural() == "D");
    REQUIRE(c.type() == "7");

    // IIm / IV  → IIm of degree IV = triad at II of F major = Gm
    c = f.applied("IIm", "IV");
    REQUIRE(c.root().natural() == "G");
}

TEST_CASE("Field applied numeric shorthand", "[field]") {
    Field f("C", ScaleType::Major);

    // applied(5, 2) = seventh at V of D major = A7
    auto c = f.applied(5, 2);
    REQUIRE(c.root().natural() == "A");
    REQUIRE(c.type() == "7");

    // applied(5, 5) = seventh at V of G major = D7
    c = f.applied(5, 5);
    REQUIRE(c.root().natural() == "D");
    REQUIRE(c.type() == "7");
}

TEST_CASE("Field applied out of range", "[field]") {
    Field f("C", ScaleType::Major);
    REQUIRE_THROWS(f.applied("V7", 0));
    REQUIRE_THROWS(f.applied("V7", 8));
    REQUIRE_THROWS(f.applied(5, 0));
}

TEST_CASE("Field applied invalid Roman numeral", "[field]") {
    Field f("C", ScaleType::Major);
    REQUIRE_THROWS(f.applied("X7", 2));
    REQUIRE_THROWS(f.applied("", 2));
}

// ---------------------------------------------------------------------------
// Harmonic function and role
// ---------------------------------------------------------------------------

TEST_CASE("Field function by degree: C major", "[field]") {
    Field f("C", ScaleType::Major);

    REQUIRE(f.function(1) == HarmonicFunction::Tonic);
    REQUIRE(f.function(2) == HarmonicFunction::Subdominant);
    REQUIRE(f.function(3) == HarmonicFunction::Tonic);
    REQUIRE(f.function(4) == HarmonicFunction::Subdominant);
    REQUIRE(f.function(5) == HarmonicFunction::Dominant);
    REQUIRE(f.function(6) == HarmonicFunction::Tonic);
    REQUIRE(f.function(7) == HarmonicFunction::Dominant);
}

TEST_CASE("Field function name and short", "[field]") {
    REQUIRE(harmonic_function_name(HarmonicFunction::Tonic) == "Tonic");
    REQUIRE(harmonic_function_name(HarmonicFunction::Subdominant) == "Subdominant");
    REQUIRE(harmonic_function_name(HarmonicFunction::Dominant) == "Dominant");

    REQUIRE(harmonic_function_short(HarmonicFunction::Tonic) == "T");
    REQUIRE(harmonic_function_short(HarmonicFunction::Subdominant) == "S");
    REQUIRE(harmonic_function_short(HarmonicFunction::Dominant) == "D");
}

TEST_CASE("Field role by degree: C major", "[field]") {
    Field f("C", ScaleType::Major);

    REQUIRE(f.role(1) == "primary");
    REQUIRE(f.role(2) == "relative of IV");
    REQUIRE(f.role(3) == "transitive");
    REQUIRE(f.role(4) == "primary");
    REQUIRE(f.role(5) == "primary");
    REQUIRE(f.role(6) == "relative of I");
    REQUIRE(f.role(7) == "relative of V");
}

TEST_CASE("Field function by chord name", "[field]") {
    Field f("C", ScaleType::Major);

    // FM is degree IV → Subdominant
    auto result = f.function("FM");
    REQUIRE(result.has_value());
    REQUIRE(*result == HarmonicFunction::Subdominant);

    // Am is degree VI → Tonic
    result = f.function("Am");
    REQUIRE(result.has_value());
    REQUIRE(*result == HarmonicFunction::Tonic);

    // GM is degree V → Dominant
    result = f.function("GM");
    REQUIRE(result.has_value());
    REQUIRE(*result == HarmonicFunction::Dominant);
}

TEST_CASE("Field function by Chord object", "[field]") {
    Field f("C", ScaleType::Major);
    Chord chord("Dm");

    auto result = f.function(chord);
    REQUIRE(result.has_value());
    REQUIRE(*result == HarmonicFunction::Subdominant);
}

TEST_CASE("Field function returns nullopt for non-field chord", "[field]") {
    Field f("C", ScaleType::Major);

    // F#M does not belong to C major
    auto result = f.function("F#M");
    REQUIRE(!result.has_value());
}

TEST_CASE("Field role by chord name", "[field]") {
    Field f("C", ScaleType::Major);

    auto result = f.role("Am");
    REQUIRE(result.has_value());
    REQUIRE(*result == "relative of I");

    result = f.role("FM");
    REQUIRE(result.has_value());
    REQUIRE(*result == "primary");

    // Non-field chord
    result = f.role("F#M");
    REQUIRE(!result.has_value());
}

TEST_CASE("Field function out of range", "[field]") {
    Field f("C", ScaleType::Major);
    REQUIRE_THROWS(f.function(0));
    REQUIRE_THROWS(f.function(8));
    REQUIRE_THROWS(f.role(0));
}

TEST_CASE("Field function natural minor", "[field]") {
    Field f("A", ScaleType::NaturalMinor);

    // i → T, ii → S, III → T, iv → S, v → D, VI → S, VII → D
    REQUIRE(f.function(1) == HarmonicFunction::Tonic);
    REQUIRE(f.function(3) == HarmonicFunction::Tonic);
    REQUIRE(f.function(5) == HarmonicFunction::Dominant);
    REQUIRE(f.function(6) == HarmonicFunction::Subdominant);
    REQUIRE(f.function(7) == HarmonicFunction::Dominant);
}

// ---------------------------------------------------------------------------
// Circle of fifths navigation
// ---------------------------------------------------------------------------

TEST_CASE("Field signature delegates to scale", "[field][fifths]") {
    REQUIRE(Field("C", "major").signature() == 0);
    REQUIRE(Field("G", "major").signature() == 1);
    REQUIRE(Field("D", "major").signature() == 2);
    REQUIRE(Field("F", "major").signature() == -1);
    REQUIRE(Field("Bb", "major").signature() == -2);
    REQUIRE(Field("A", "natural minor").signature() == 0);
}

TEST_CASE("Field relative major to minor", "[field][fifths]") {
    Field f("C", "major");
    Field rel = f.relative();
    REQUIRE(rel.tonic().natural() == "A");
    REQUIRE(rel.chords().size() == 7);
}

TEST_CASE("Field relative minor to major", "[field][fifths]") {
    Field f("A", "natural minor");
    Field rel = f.relative();
    REQUIRE(rel.tonic().natural() == "C");
    REQUIRE(rel.chords().size() == 7);
}

TEST_CASE("Field relative G major", "[field][fifths]") {
    Field rel = Field("G", "major").relative();
    REQUIRE(rel.tonic().natural() == "E");
}

TEST_CASE("Field parallel major to minor", "[field][fifths]") {
    Field f("C", "major");
    Field par = f.parallel();
    REQUIRE(par.tonic().natural() == "C");
    REQUIRE(par.chords().size() == 7);
    // Parallel should produce different chords
    REQUIRE(f.chord(3).name() != par.chord(3).name());
}

TEST_CASE("Field parallel minor to major", "[field][fifths]") {
    Field par = Field("C", "natural minor").parallel();
    REQUIRE(par.tonic().natural() == "C");
    // C major chord I = CM
    REQUIRE(par.chord(1).root().natural() == "C");
}

TEST_CASE("Field neighbors C major", "[field][fifths]") {
    auto [sub, dom] = Field("C", "major").neighbors();
    REQUIRE(sub.tonic().natural() == "F");
    REQUIRE(dom.tonic().natural() == "G");
    REQUIRE(sub.chords().size() == 7);
    REQUIRE(dom.chords().size() == 7);
}

TEST_CASE("Field neighbors A minor", "[field][fifths]") {
    auto [sub, dom] = Field("A", "natural minor").neighbors();
    REQUIRE(sub.tonic().natural() == "D");
    REQUIRE(dom.tonic().natural() == "E");
}
