// Gingo — Music Theory Library
// Tests for Fretboard class.
//
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/fretboard.hpp>

#include <set>

using namespace gingo;

// ---------------------------------------------------------------------------
// Factories
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard violao has 6 strings and 19 frets", "[fretboard]") {
    auto g = Fretboard::violao();
    REQUIRE(g.num_strings() == 6);
    REQUIRE(g.num_frets() == 19);
    REQUIRE(g.name() == "violao");
}

TEST_CASE("Fretboard cavaquinho has 4 strings and 17 frets", "[fretboard]") {
    auto c = Fretboard::cavaquinho();
    REQUIRE(c.num_strings() == 4);
    REQUIRE(c.num_frets() == 17);
    REQUIRE(c.name() == "cavaquinho");
}

TEST_CASE("Fretboard bandolim has 4 strings and 17 frets", "[fretboard]") {
    auto b = Fretboard::bandolim();
    REQUIRE(b.num_strings() == 4);
    REQUIRE(b.num_frets() == 17);
    REQUIRE(b.name() == "bandolim");
}

TEST_CASE("Fretboard custom tuning", "[fretboard]") {
    Fretboard fb("test", {64, 59, 55}, 12);
    REQUIRE(fb.num_strings() == 3);
    REQUIRE(fb.num_frets() == 12);
    REQUIRE(fb.name() == "test");
}

// ---------------------------------------------------------------------------
// Open strings (MIDI values)
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard violao open string 1 is E4", "[fretboard]") {
    auto g = Fretboard::violao();
    REQUIRE(g.midi_at(1, 0) == 64);
    REQUIRE(g.note_at(1, 0).name() == "E");
}

TEST_CASE("Fretboard violao open string 6 is E2", "[fretboard]") {
    auto g = Fretboard::violao();
    REQUIRE(g.midi_at(6, 0) == 40);
    REQUIRE(g.note_at(6, 0).name() == "E");
}

TEST_CASE("Fretboard cavaquinho open string 1 is D5", "[fretboard]") {
    auto c = Fretboard::cavaquinho();
    REQUIRE(c.midi_at(1, 0) == 74);
    REQUIRE(c.note_at(1, 0).name() == "D");
}

TEST_CASE("Fretboard bandolim open string 1 is E5", "[fretboard]") {
    auto b = Fretboard::bandolim();
    REQUIRE(b.midi_at(1, 0) == 76);
    REQUIRE(b.note_at(1, 0).name() == "E");
}

// ---------------------------------------------------------------------------
// Position
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard position returns correct data", "[fretboard]") {
    auto g = Fretboard::violao();
    auto p = g.position(1, 5);
    REQUIRE(p.string == 1);
    REQUIRE(p.fret == 5);
    REQUIRE(p.midi == 69);    // E4(64) + 5 = A4(69)
    REQUIRE(p.note == "A");
    REQUIRE(p.octave == 4);
}

TEST_CASE("Fretboard position at fret 12 is octave higher", "[fretboard]") {
    auto g = Fretboard::violao();
    auto open = g.position(1, 0);
    auto f12 = g.position(1, 12);
    REQUIRE(f12.midi == open.midi + 12);
    REQUIRE(f12.note == open.note);
    REQUIRE(f12.octave == open.octave + 1);
}

// ---------------------------------------------------------------------------
// Positions (all occurrences of a note)
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard positions finds note on multiple strings", "[fretboard]") {
    auto g = Fretboard::violao();
    auto pos = g.positions(Note("E"));
    // E appears on every string at various frets
    REQUIRE(pos.size() >= 6);
}

TEST_CASE("Fretboard positions all have correct pitch class", "[fretboard]") {
    auto g = Fretboard::violao();
    auto pos = g.positions(Note("C"));
    for (const auto& p : pos) {
        REQUIRE(p.note == "C");
    }
}

// ---------------------------------------------------------------------------
// Scale positions
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard scale_positions returns positions for C major", "[fretboard]") {
    auto g = Fretboard::violao();
    auto pos = g.scale_positions(Scale("C", ScaleType::Major));
    REQUIRE(pos.size() > 20);  // Many positions across all frets
}

TEST_CASE("Fretboard scale_positions within fret range", "[fretboard]") {
    auto g = Fretboard::violao();
    auto pos = g.scale_positions(Scale("C", ScaleType::Major), 0, 4);
    for (const auto& p : pos) {
        REQUIRE(p.fret >= 0);
        REQUIRE(p.fret <= 4);
    }
}

// ---------------------------------------------------------------------------
// Fingering
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard fingering for CM on violao", "[fretboard]") {
    auto g = Fretboard::violao();
    auto f = g.fingering(Chord("CM"));
    REQUIRE(f.chord_name == "CM");
    REQUIRE(!f.strings.empty());
    REQUIRE(!f.midi_notes.empty());
}

TEST_CASE("Fretboard fingering covers all chord notes", "[fretboard]") {
    auto g = Fretboard::violao();
    auto f = g.fingering(Chord("CM"));
    // CM = C, E, G → pitch classes 0, 4, 7
    std::set<int> pcs;
    for (int m : f.midi_notes) pcs.insert(((m % 12) + 12) % 12);
    REQUIRE(pcs.find(0) != pcs.end());   // C
    REQUIRE(pcs.find(4) != pcs.end());   // E
    REQUIRE(pcs.find(7) != pcs.end());   // G
}

TEST_CASE("Fretboard fingerings returns multiple results", "[fretboard]") {
    auto g = Fretboard::violao();
    auto fs = g.fingerings(Chord("CM"), 3);
    REQUIRE(fs.size() >= 2);
    for (const auto& f : fs) {
        REQUIRE(f.chord_name == "CM");
    }
}

TEST_CASE("Fretboard fingering for Am on cavaquinho", "[fretboard]") {
    auto c = Fretboard::cavaquinho();
    auto f = c.fingering(Chord("Am"));
    REQUIRE(f.chord_name == "Am");
    REQUIRE(!f.midi_notes.empty());
}

// ---------------------------------------------------------------------------
// Identify
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard identify CM from open position", "[fretboard]") {
    auto g = Fretboard::violao();
    // Standard C chord: string 5 fret 3 (C), string 4 fret 2 (E),
    // string 3 fret 0 (G), string 2 fret 1 (C), string 1 fret 0 (E)
    auto chord = g.identify({{5, 3}, {4, 2}, {3, 0}, {2, 1}, {1, 0}});
    REQUIRE(chord.name() == "CM");
}

// ---------------------------------------------------------------------------
// Capo
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard capo raises pitch by fret count", "[fretboard]") {
    auto g = Fretboard::violao();
    auto gc = g.capo(2);
    // Open string 1 was E4(64), now F#4(66)
    REQUIRE(gc.midi_at(1, 0) == 66);
    REQUIRE(gc.note_at(1, 0).name() == "F#");
}

TEST_CASE("Fretboard capo reduces fret count", "[fretboard]") {
    auto g = Fretboard::violao();
    auto gc = g.capo(2);
    REQUIRE(gc.num_frets() == 17);  // 19 - 2
}

TEST_CASE("Fretboard capo name includes capo info", "[fretboard]") {
    auto g = Fretboard::violao();
    auto gc = g.capo(3);
    REQUIRE(gc.name().find("capo") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST_CASE("Fretboard invalid string throws", "[fretboard]") {
    auto g = Fretboard::violao();
    REQUIRE_THROWS(g.position(0, 0));
    REQUIRE_THROWS(g.position(7, 0));
}

TEST_CASE("Fretboard invalid fret throws", "[fretboard]") {
    auto g = Fretboard::violao();
    REQUIRE_THROWS(g.position(1, -1));
    REQUIRE_THROWS(g.position(1, 20));
}

TEST_CASE("Fretboard to_string contains instrument name", "[fretboard]") {
    auto g = Fretboard::violao();
    REQUIRE(g.to_string().find("violao") != std::string::npos);
}

TEST_CASE("FretPosition to_string contains note", "[fretboard]") {
    auto g = Fretboard::violao();
    auto p = g.position(1, 0);
    REQUIRE(p.to_string().find("E") != std::string::npos);
}

TEST_CASE("Fingering to_string contains chord name", "[fretboard]") {
    auto g = Fretboard::violao();
    auto f = g.fingering(Chord("CM"));
    REQUIRE(f.to_string().find("CM") != std::string::npos);
}
