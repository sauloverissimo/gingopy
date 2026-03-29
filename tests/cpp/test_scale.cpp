// Gingo — Scale module tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/scale.hpp>

#include <string>
#include <vector>

using namespace gingo;

static std::vector<std::string> naturals(const std::vector<Note>& notes) {
    std::vector<std::string> names;
    for (const auto& n : notes) names.push_back(n.natural());
    return names;
}

TEST_CASE("C major scale", "[scale]") {
    Scale s("C", ScaleType::Major);
    auto names = naturals(s.notes());
    REQUIRE(names.size() >= 7);
    REQUIRE(names[0] == "C");
}

TEST_CASE("Scale from string type", "[scale]") {
    Scale s("C", "major");
    REQUIRE(s.type() == ScaleType::Major);
    REQUIRE(s.tonic().natural() == "C");
}

TEST_CASE("Scale degree access", "[scale]") {
    Scale s("C", ScaleType::Major);
    REQUIRE(s.degree(1).natural() == "C");
    REQUIRE(s.degree(5).natural() == "G");
}

TEST_CASE("Scale size", "[scale]") {
    Scale diatonic("C", ScaleType::Major, Modality::Diatonic);
    REQUIRE(diatonic.size() >= 7);
}

TEST_CASE("Scale contains note", "[scale]") {
    Scale s("C", ScaleType::Major);
    REQUIRE(s.contains(Note("C")));
    REQUIRE(s.contains(Note("G")));
}

TEST_CASE("Minor scale", "[scale]") {
    Scale s("A", ScaleType::NaturalMinor);
    auto names = naturals(s.notes());
    REQUIRE(names[0] == "A");
}

TEST_CASE("Parse scale type", "[scale]") {
    REQUIRE(Scale::parse_type("major") == ScaleType::Major);
    REQUIRE(Scale::parse_type("natural minor") == ScaleType::NaturalMinor);
    REQUIRE(Scale::parse_type("harmonic minor") == ScaleType::HarmonicMinor);
    REQUIRE(Scale::parse_type("melodic minor") == ScaleType::MelodicMinor);
    REQUIRE(Scale::parse_type("diminished") == ScaleType::Diminished);
}

TEST_CASE("Parse modality", "[scale]") {
    REQUIRE(Scale::parse_modality("diatonic") == Modality::Diatonic);
    REQUIRE(Scale::parse_modality("pentatonic") == Modality::Pentatonic);
}

TEST_CASE("Invalid scale type throws", "[scale]") {
    REQUIRE_THROWS_AS(Scale::parse_type("invalid"), std::invalid_argument);
}

TEST_CASE("Scale chained degree", "[scale]") {
    Scale s("C", ScaleType::Major);
    // Single-arg backward compatibility
    REQUIRE(s.degree(5).natural() == "G");
    // degree({5, 5}) = grau V do grau V = D
    REQUIRE(s.degree({5, 5}).natural() == "D");
    // degree({5, 5, 3}) = grau III do grau V do grau V = F
    REQUIRE(s.degree({5, 5, 3}).natural() == "F");
    // Single element vector = same as scalar
    REQUIRE(s.degree({1}).natural() == "C");
    REQUIRE(s.degree({7}).natural() == "B");
}

TEST_CASE("Scale chained degree wraps around", "[scale]") {
    Scale s("C", ScaleType::Major);
    // degree({7, 2}) = grau II do grau VII: (7-1+2-1)%7+1 = 1 → C
    REQUIRE(s.degree({7, 2}).natural() == "C");
    // degree({4, 5}) = grau V do grau IV: (4-1+5-1)%7+1 = 1 → C
    REQUIRE(s.degree({4, 5}).natural() == "C");
}

TEST_CASE("Scale chained degree validation", "[scale]") {
    Scale s("C", ScaleType::Major);
    REQUIRE_THROWS_AS(s.degree(std::vector<int>{}), std::invalid_argument);
    REQUIRE_THROWS_AS(s.degree({0, 5}), std::out_of_range);
    REQUIRE_THROWS_AS(s.degree({100}), std::out_of_range);
}

TEST_CASE("Scale walk", "[scale]") {
    Scale s("C", ScaleType::Major);
    // walk(1, {4}) = from I, a fourth = IV = F
    REQUIRE(s.walk(1, {4}).natural() == "F");
    // walk(5, {5}) = from V, a fifth = II = D
    REQUIRE(s.walk(5, {5}).natural() == "D");
    // walk wraps: from VII, a second = I = C
    REQUIRE(s.walk(7, {2}).natural() == "C");
}

TEST_CASE("Scale walk multi-step", "[scale]") {
    Scale s("C", ScaleType::Major);
    // walk(3, {3, 2}) = from III, a third (→V), then a second (→VI)
    // Step 1: (3-1+3-1)%7+1 = 5 → G
    // Step 2: (5-1+2-1)%7+1 = 6 → A
    REQUIRE(s.walk(3, {3, 2}).natural() == "A");
}

TEST_CASE("Scale walk negative steps", "[scale]") {
    Scale s("C", ScaleType::Major);
    // Negative steps use the same formula: (pos-1 + step-1)
    // walk(5, {-2}): (5-1 + (-2)-1) % 7 = 1 % 7 = 1 → pos 2 = D
    Note result = s.walk(5, {-2});
    REQUIRE(result.natural() == "D");
}

TEST_CASE("Scale walk validation", "[scale]") {
    Scale s("C", ScaleType::Major);
    REQUIRE_THROWS_AS(s.walk(1, {}), std::invalid_argument);
    REQUIRE_THROWS_AS(s.walk(0, {5}), std::out_of_range);
    REQUIRE_THROWS_AS(s.walk(100, {1}), std::out_of_range);
}

TEST_CASE("Scale mask", "[scale]") {
    Scale s("C", ScaleType::Major);
    auto mask = s.mask();
    REQUIRE(mask.size() == 24);
    REQUIRE(mask[0] == 1);   // P1 (tonic)
    REQUIRE(mask[7] == 1);   // 5J (dominant)
}

// ---------------------------------------------------------------------------
// New scale hierarchy tests
// ---------------------------------------------------------------------------

TEST_CASE("Parse new scale types", "[scale][hierarchy]") {
    REQUIRE(Scale::parse_type("harmonic major") == ScaleType::HarmonicMajor);
    REQUIRE(Scale::parse_type("whole tone") == ScaleType::WholeTone);
    REQUIRE(Scale::parse_type("wholetone") == ScaleType::WholeTone);
    REQUIRE(Scale::parse_type("augmented") == ScaleType::Augmented);
    REQUIRE(Scale::parse_type("aug") == ScaleType::Augmented);
    REQUIRE(Scale::parse_type("blues") == ScaleType::Blues);
    REQUIRE(Scale::parse_type("chromatic") == ScaleType::Chromatic);
}

TEST_CASE("New scale type construction", "[scale][hierarchy]") {
    Scale wt("C", ScaleType::WholeTone);
    REQUIRE(wt.size() == 6);
    auto wtn = naturals(wt.notes());
    REQUIRE(wtn[0] == "C");

    Scale bl("A", ScaleType::Blues);
    REQUIRE(bl.size() == 6);

    Scale ch("C", ScaleType::Chromatic);
    REQUIRE(ch.size() == 12);

    Scale dim("C", ScaleType::Diminished);
    REQUIRE(dim.size() == 8);
}

TEST_CASE("Construct by mode name", "[scale][hierarchy]") {
    Scale d("D", "dorian");
    REQUIRE(d.parent() == ScaleType::Major);
    REQUIRE(d.mode_number() == 2);
    REQUIRE(d.tonic().natural() == "D");
    auto notes = naturals(d.notes());
    // D Dorian = D E F G A B C
    REQUIRE(notes == std::vector<std::string>{"D","E","F","G","A","B","C"});
}

TEST_CASE("Construct altered scale", "[scale][hierarchy]") {
    Scale a("C", "altered");
    REQUIRE(a.parent() == ScaleType::MelodicMinor);
    REQUIRE(a.mode_number() == 7);
}

TEST_CASE("Construct phrygian dominant", "[scale][hierarchy]") {
    Scale pd("E", "phrygian dominant");
    REQUIRE(pd.parent() == ScaleType::HarmonicMinor);
    REQUIRE(pd.mode_number() == 5);
    REQUIRE(pd.tonic().natural() == "E");
}

TEST_CASE("Parent and mode_number accessors", "[scale][hierarchy]") {
    Scale s("C", ScaleType::Major);
    REQUIRE(s.parent() == ScaleType::Major);
    REQUIRE(s.mode_number() == 1);
    REQUIRE(s.type() == ScaleType::Major);  // backward compat
}

TEST_CASE("mode_name accessor", "[scale][hierarchy]") {
    Scale s("C", ScaleType::Major);
    REQUIRE(s.mode_name() == "Ionian");

    Scale d("D", "dorian");
    REQUIRE(d.mode_name() == "Dorian");

    Scale nm("A", ScaleType::NaturalMinor);
    REQUIRE(nm.mode_name() == "Aeolian");
}

TEST_CASE("quality accessor", "[scale][hierarchy]") {
    Scale maj("C", "major");
    REQUIRE(maj.quality() == "major");

    Scale dor("D", "dorian");
    REQUIRE(dor.quality() == "minor");

    Scale lyd("F", "lydian");
    REQUIRE(lyd.quality() == "major");

    Scale loc("B", "locrian");
    REQUIRE(loc.quality() == "minor");
}

TEST_CASE("brightness accessor", "[scale][hierarchy]") {
    Scale lyd("F", "lydian");
    REQUIRE(lyd.brightness() == 7);

    Scale ion("C", "major");
    REQUIRE(ion.brightness() == 6);

    Scale loc("B", "locrian");
    REQUIRE(loc.brightness() == 0);
}

TEST_CASE("mode(int) tracks mode number", "[scale][hierarchy]") {
    Scale s("C", ScaleType::Major);
    Scale dorian = s.mode(2);
    REQUIRE(dorian.tonic().natural() == "D");
    REQUIRE(dorian.parent() == ScaleType::Major);
    REQUIRE(dorian.mode_number() == 2);
    REQUIRE(dorian.mode_name() == "Dorian");
}

TEST_CASE("mode(string) by name", "[scale][hierarchy]") {
    Scale s("C", "major");
    Scale lyd = s.mode("lydian");
    REQUIRE(lyd.tonic().natural() == "F");
    REQUIRE(lyd.mode_number() == 4);
    REQUIRE(lyd.mode_name() == "Lydian");
}

TEST_CASE("mode(string) from natural minor", "[scale][hierarchy]") {
    Scale nm("A", "natural minor");
    Scale dor = nm.mode("dorian");
    REQUIRE(dor.tonic().natural() == "D");
    REQUIRE(dor.mode_name() == "Dorian");
}

TEST_CASE("mode(string) cross-parent throws", "[scale][hierarchy]") {
    Scale s("C", "major");
    REQUIRE_THROWS_AS(s.mode("altered"), std::invalid_argument);
    REQUIRE_THROWS_AS(s.mode("phrygian dominant"), std::invalid_argument);
}

TEST_CASE("mode chaining", "[scale][hierarchy]") {
    // C Major -> mode(2) = D Dorian -> mode(3) = F Lydian
    Scale s("C", "major");
    Scale dorian = s.mode(2);
    Scale from_dorian = dorian.mode(3);  // 3rd degree of Dorian
    REQUIRE(from_dorian.tonic().natural() == "F");
    REQUIRE(from_dorian.mode_name() == "Lydian");
}

TEST_CASE("pentatonic method", "[scale][hierarchy]") {
    Scale s("C", "major");
    Scale p = s.pentatonic();
    REQUIRE(p.is_pentatonic());
    REQUIRE(p.size() == 5);
    auto notes = naturals(p.notes());
    // C Major pentatonic: C D E G A (remove 4th=F and 7th=B)
    REQUIRE(notes == std::vector<std::string>{"C","D","E","G","A"});
}

TEST_CASE("pentatonic minor", "[scale][hierarchy]") {
    Scale s("A", ScaleType::NaturalMinor);
    Scale p = s.pentatonic();
    REQUIRE(p.size() == 5);
    auto notes = naturals(p.notes());
    // A minor pentatonic: A C D E G (remove 2nd=B and 6th=F)
    REQUIRE(notes == std::vector<std::string>{"A","C","D","E","G"});
}

TEST_CASE("Construct with pentatonic suffix", "[scale][hierarchy]") {
    Scale mp("C", "major pentatonic");
    REQUIRE(mp.is_pentatonic());
    REQUIRE(mp.size() == 5);
    auto mp_notes = naturals(mp.notes());
    REQUIRE(mp_notes == std::vector<std::string>{"C","D","E","G","A"});

    Scale mnp("A", "minor pentatonic");
    REQUIRE(mnp.is_pentatonic());
    REQUIRE(mnp.parent() == ScaleType::NaturalMinor);
    REQUIRE(mnp.size() == 5);
    auto mnp_notes = naturals(mnp.notes());
    // A minor pentatonic = A C D E G (natural minor, not harmonic)
    REQUIRE(mnp_notes == std::vector<std::string>{"A","C","D","E","G"});
}

TEST_CASE("Dorian pentatonic via suffix", "[scale][hierarchy]") {
    Scale dp("D", "dorian pentatonic");
    REQUIRE(dp.is_pentatonic());
    REQUIRE(dp.parent() == ScaleType::Major);
    REQUIRE(dp.mode_number() == 2);
    REQUIRE(dp.size() == 5);
}

TEST_CASE("colors against reference", "[scale][hierarchy]") {
    // Dorian vs Ionian with same tonic: the color is the natural 6th
    Scale dor("C", "dorian");
    auto cols = dor.colors("ionian");
    // C Dorian = C D Eb F G A Bb
    // C Ionian = C D E  F G A B
    // Differences: Eb (not in Ionian), Bb (not in Ionian)
    REQUIRE(cols.size() == 2);
}

TEST_CASE("colors Lydian vs Ionian", "[scale][hierarchy]") {
    Scale lyd("C", "lydian");
    auto cols = lyd.colors("ionian");
    // C Lydian  = C D E F# G A B
    // C Ionian  = C D E F  G A B
    // Color note: F# (not in Ionian)
    REQUIRE(cols.size() == 1);
    REQUIRE(cols[0].natural() == "F#");
}

TEST_CASE("to_string shows mode name", "[scale][hierarchy]") {
    Scale s("D", "dorian");
    REQUIRE(s.to_string() == "Scale(\"D\", \"Dorian\")");
}

TEST_CASE("to_string for pentatonic", "[scale][hierarchy]") {
    Scale s("C", "major pentatonic");
    REQUIRE(s.to_string() == "Scale(\"C\", \"major pentatonic\")");
}

TEST_CASE("Backward compat: enum constructor", "[scale][hierarchy]") {
    Scale s("C", ScaleType::Major, Modality::Diatonic);
    REQUIRE(s.type() == ScaleType::Major);
    REQUIRE(s.modality() == Modality::Diatonic);
    REQUIRE(s.parent() == ScaleType::Major);
    REQUIRE(s.mode_number() == 1);
    REQUIRE(!s.is_pentatonic());
}

TEST_CASE("Backward compat: NaturalMinor via enum", "[scale][hierarchy]") {
    Scale s("A", ScaleType::NaturalMinor);
    REQUIRE(s.parent() == ScaleType::NaturalMinor);
    auto notes = naturals(s.notes());
    REQUIRE(notes[0] == "A");
    REQUIRE(notes.size() == 7);
}

TEST_CASE("Harmonic minor modes", "[scale][hierarchy]") {
    Scale hm("A", "harmonic minor");
    Scale pd = hm.mode(5);
    REQUIRE(pd.tonic().natural() == "E");
    REQUIRE(pd.mode_name() == "Phrygian Dominant");
}

TEST_CASE("Melodic minor modes", "[scale][hierarchy]") {
    Scale mm("C", "melodic minor");
    Scale alt = mm.mode(7);
    REQUIRE(alt.mode_name() == "Altered");
    REQUIRE(alt.quality() == "minor");
}

TEST_CASE("Harmonic major construction", "[scale][hierarchy]") {
    Scale hm("C", "harmonic major");
    REQUIRE(hm.parent() == ScaleType::HarmonicMajor);
    REQUIRE(hm.size() == 7);
    auto notes = naturals(hm.notes());
    // C Harmonic Major: C D E F G Ab B
    REQUIRE(notes[0] == "C");
    REQUIRE(notes[5] == "G#");  // Ab = G# in natural notation
}

// ---------------------------------------------------------------------------
// Circle of fifths: signature, relative, parallel, neighbors
// ---------------------------------------------------------------------------

TEST_CASE("Signature for major keys", "[scale][fifths]") {
    REQUIRE(Scale("C",  "major").signature() == 0);
    REQUIRE(Scale("G",  "major").signature() == 1);
    REQUIRE(Scale("D",  "major").signature() == 2);
    REQUIRE(Scale("A",  "major").signature() == 3);
    REQUIRE(Scale("E",  "major").signature() == 4);
    REQUIRE(Scale("B",  "major").signature() == 5);
    REQUIRE(Scale("F#", "major").signature() == 6);
    REQUIRE(Scale("F",  "major").signature() == -1);
    REQUIRE(Scale("Bb", "major").signature() == -2);
    REQUIRE(Scale("Eb", "major").signature() == -3);
    REQUIRE(Scale("Ab", "major").signature() == -4);
    REQUIRE(Scale("Db", "major").signature() == -5);
}

TEST_CASE("Signature for natural minor keys", "[scale][fifths]") {
    // A minor = relative of C major = 0
    REQUIRE(Scale("A",  "natural minor").signature() == 0);
    // E minor = relative of G major = 1
    REQUIRE(Scale("E",  "natural minor").signature() == 1);
    // D minor = relative of F major = -1
    REQUIRE(Scale("D",  "natural minor").signature() == -1);
}

TEST_CASE("Signature for modes shares parent key", "[scale][fifths]") {
    // D Dorian (mode 2 of C major) = 0 sharps/flats
    REQUIRE(Scale("D", "dorian").signature() == 0);
    // E Phrygian (mode 3 of C major) = 0
    REQUIRE(Scale("E", "phrygian").signature() == 0);
    // A Mixolydian (mode 5 of D major) = 2 sharps
    REQUIRE(Scale("A", "mixolydian").signature() == 2);
}

TEST_CASE("Signature for non-diatonic returns 0", "[scale][fifths]") {
    REQUIRE(Scale("A", "harmonic minor").signature() == 0);
    REQUIRE(Scale("C", "melodic minor").signature() == 0);
    REQUIRE(Scale("C", "whole tone").signature() == 0);
}

TEST_CASE("Relative major and minor", "[scale][fifths]") {
    // C major -> A natural minor
    Scale c_rel = Scale("C", "major").relative();
    REQUIRE(c_rel.tonic().natural() == "A");
    REQUIRE(c_rel.parent() == ScaleType::NaturalMinor);

    // A natural minor -> C major
    Scale a_rel = Scale("A", "natural minor").relative();
    REQUIRE(a_rel.tonic().natural() == "C");
    REQUIRE(a_rel.parent() == ScaleType::Major);

    // G major -> E minor
    Scale g_rel = Scale("G", "major").relative();
    REQUIRE(g_rel.tonic().natural() == "E");
}

TEST_CASE("Relative throws for modes", "[scale][fifths]") {
    REQUIRE_THROWS_AS(Scale("D", "dorian").relative(), std::invalid_argument);
    REQUIRE_THROWS_AS(Scale("A", "harmonic minor").relative(), std::invalid_argument);
}

TEST_CASE("Parallel major and minor", "[scale][fifths]") {
    // C major -> C natural minor
    Scale c_par = Scale("C", "major").parallel();
    REQUIRE(c_par.tonic().natural() == "C");
    REQUIRE(c_par.parent() == ScaleType::NaturalMinor);

    // C natural minor -> C major
    Scale cm_par = Scale("C", "natural minor").parallel();
    REQUIRE(cm_par.tonic().natural() == "C");
    REQUIRE(cm_par.parent() == ScaleType::Major);
}

TEST_CASE("Parallel throws for modes", "[scale][fifths]") {
    REQUIRE_THROWS_AS(Scale("D", "dorian").parallel(), std::invalid_argument);
}

TEST_CASE("Neighbors on circle of fifths", "[scale][fifths]") {
    // C major -> (F major, G major)
    auto [sub, dom] = Scale("C", "major").neighbors();
    REQUIRE(sub.tonic().natural() == "F");
    REQUIRE(sub.parent() == ScaleType::Major);
    REQUIRE(dom.tonic().natural() == "G");
    REQUIRE(dom.parent() == ScaleType::Major);
}

TEST_CASE("Neighbors for minor key", "[scale][fifths]") {
    // A minor -> (D minor, E minor)
    auto [sub, dom] = Scale("A", "natural minor").neighbors();
    REQUIRE(sub.tonic().natural() == "D");
    REQUIRE(sub.parent() == ScaleType::NaturalMinor);
    REQUIRE(dom.tonic().natural() == "E");
    REQUIRE(dom.parent() == ScaleType::NaturalMinor);
}

TEST_CASE("Neighbors preserves pentatonic", "[scale][fifths]") {
    Scale s("C", "major pentatonic");
    auto [sub, dom] = s.neighbors();
    REQUIRE(sub.is_pentatonic());
    REQUIRE(dom.is_pentatonic());
}

TEST_CASE("Neighbors preserves mode", "[scale][fifths]") {
    Scale dor("D", "dorian");
    auto [sub, dom] = dor.neighbors();
    REQUIRE(sub.mode_number() == 2);
    REQUIRE(dom.mode_number() == 2);
    REQUIRE(sub.tonic().natural() == "G");   // D - 7 = G
    REQUIRE(dom.tonic().natural() == "A");   // D + 7 = A
}

TEST_CASE("degree_of finds correct degree", "[scale]") {
    Scale cmaj("C", "major");
    REQUIRE(cmaj.degree_of(Note("C")) == 1);
    REQUIRE(cmaj.degree_of(Note("D")) == 2);
    REQUIRE(cmaj.degree_of(Note("E")) == 3);
    REQUIRE(cmaj.degree_of(Note("G")) == 5);
    REQUIRE(cmaj.degree_of(Note("B")) == 7);
}

TEST_CASE("degree_of returns nullopt for non-member", "[scale]") {
    Scale cmaj("C", "major");
    REQUIRE_FALSE(cmaj.degree_of(Note("F#")).has_value());
    REQUIRE_FALSE(cmaj.degree_of(Note("Bb")).has_value());
}

TEST_CASE("degree_of enharmonic match", "[scale]") {
    Scale cmaj("C", "major");
    // Fb is enharmonic to E (semitone 4), so it matches degree 3
    REQUIRE(cmaj.degree_of(Note("Fb")) == 3);
}
