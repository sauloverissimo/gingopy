// Gingo — Music Theory Library
// Tests for PianoSVG class.
//
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/piano_svg.hpp>
#include <gingo/field.hpp>

#include <fstream>
#include <string>

using namespace gingo;

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

static int count(const std::string& haystack, const std::string& needle) {
    int n = 0;
    std::string::size_type pos = 0;
    while ((pos = haystack.find(needle, pos)) != std::string::npos) {
        ++n;
        pos += needle.size();
    }
    return n;
}

// ---------------------------------------------------------------------------
// Basic structure
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG note generates valid SVG", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
    REQUIRE(contains(svg, "xmlns=\"http://www.w3.org/2000/svg\""));
}

TEST_CASE("PianoSVG note contains title", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "C4"));
}

TEST_CASE("PianoSVG note highlights white key", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    // Highlighted white key has class "w h"
    REQUIRE(contains(svg, "class=\"w h\""));
}

TEST_CASE("PianoSVG sharp note highlights black key", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C#"), 4);
    // Highlighted black key has class "b h"
    REQUIRE(contains(svg, "class=\"b h\""));
    REQUIRE(contains(svg, "C#4"));
}

// ---------------------------------------------------------------------------
// CSS style block
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG has CSS style block", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "<style>"));
    REQUIRE(contains(svg, "</style>"));
    REQUIRE(contains(svg, ".w{"));
    REQUIRE(contains(svg, ".b{"));
    REQUIRE(contains(svg, ".h.w{"));
    REQUIRE(contains(svg, ".h.b{"));
    REQUIRE(contains(svg, ".t{"));
}

TEST_CASE("PianoSVG CSS contains highlight colors", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    // Colors appear only once, in the CSS block
    REQUIRE(contains(svg, "#4A90D9"));
    REQUIRE(contains(svg, "#2E6AB0"));
}

TEST_CASE("PianoSVG CSS contains pointer-events", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "pointer-events:none"));
}

// ---------------------------------------------------------------------------
// Chord
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG chord generates SVG with chord name", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::chord(piano, Chord("CM"), 4);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "CM"));
}

TEST_CASE("PianoSVG chord has multiple highlighted keys", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::chord(piano, Chord("CM"), 4);
    // CM = C, E, G → 3 white keys highlighted
    REQUIRE(count(svg, "class=\"w h\"") >= 3);
}

TEST_CASE("PianoSVG chord with voicing style", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::chord(piano, Chord("Am7"), 4, VoicingStyle::Shell);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "Am7"));
}

// ---------------------------------------------------------------------------
// Scale
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG scale generates SVG with scale name", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::scale(piano, Scale("C", ScaleType::Major), 4);
    REQUIRE(contains(svg, "<svg"));
    // mode_name() returns "Ionian" for Major scale
    REQUIRE(contains(svg, "Ionian"));
}

TEST_CASE("PianoSVG scale C major highlights 7 keys", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::scale(piano, Scale("C", ScaleType::Major), 4);
    // C major has 7 notes, all white → 7 highlighted with class "w h"
    REQUIRE(count(svg, "class=\"w h\"") >= 7);
}

// ---------------------------------------------------------------------------
// Keys (custom)
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG keys with custom title", "[piano_svg]") {
    Piano piano;
    auto k1 = piano.key(Note("C"), 4);
    auto k2 = piano.key(Note("E"), 4);
    auto svg = PianoSVG::keys(piano, {k1, k2}, "Custom");
    REQUIRE(contains(svg, "Custom"));
    REQUIRE(contains(svg, "C4"));
    REQUIRE(contains(svg, "E4"));
}

TEST_CASE("PianoSVG keys with empty title", "[piano_svg]") {
    Piano piano;
    auto k1 = piano.key(Note("C"), 4);
    auto svg = PianoSVG::keys(piano, {k1});
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
}

// ---------------------------------------------------------------------------
// Voicing
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG voicing shows style in title", "[piano_svg]") {
    Piano piano;
    auto v = piano.voicing(Chord("CM"), 4, VoicingStyle::Open);
    auto svg = PianoSVG::voicing(piano, v);
    REQUIRE(contains(svg, "CM"));
    REQUIRE(contains(svg, "open"));
}

// ---------------------------------------------------------------------------
// MIDI
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG midi generates SVG", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::midi(piano, {60, 64, 67});
    REQUIRE(contains(svg, "<svg"));
    // Labels for the highlighted keys
    REQUIRE(contains(svg, "C4"));
    REQUIRE(contains(svg, "E4"));
    REQUIRE(contains(svg, "G4"));
}

// ---------------------------------------------------------------------------
// Write
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG write creates file", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    std::string path = "/tmp/gingo_test_piano_svg.svg";
    PianoSVG::write(svg, path);

    std::ifstream in(path);
    REQUIRE(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    REQUIRE(content == svg);
    std::remove(path.c_str());
}

// ---------------------------------------------------------------------------
// Small piano
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG works with 61-key piano", "[piano_svg]") {
    Piano piano(61);
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "C4"));
}

// ---------------------------------------------------------------------------
// Interactive attributes (data-*, id, class)
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG keys have id attributes", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "id=\"key-60\""));
}

TEST_CASE("PianoSVG keys have data-midi attribute", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "data-midi=\"60\""));
}

TEST_CASE("PianoSVG keys have data-note attribute", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "data-note=\"C\""));
}

TEST_CASE("PianoSVG keys have data-octave attribute", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "data-octave=\"4\""));
}

TEST_CASE("PianoSVG white keys have class w", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "class=\"w\""));
    REQUIRE(contains(svg, "class=\"b\""));
}

TEST_CASE("PianoSVG highlighted white key has class w h", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "class=\"w h\""));
}

TEST_CASE("PianoSVG highlighted black key has class b h", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C#"), 4);
    REQUIRE(contains(svg, "class=\"b h\""));
}

TEST_CASE("PianoSVG labels use CSS text classes", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4);
    REQUIRE(contains(svg, "class=\"t tw\""));
}

TEST_CASE("PianoSVG black key label uses CSS text class", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C#"), 4);
    REQUIRE(contains(svg, "class=\"t tb\""));
}

// ---------------------------------------------------------------------------
// Edge cases
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG empty highlight list", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::midi(piano, {});
    REQUIRE(contains(svg, "<svg"));
    // No highlighted keys → no "w h" or "b h" classes
    REQUIRE(!contains(svg, "class=\"w h\""));
    REQUIRE(!contains(svg, "class=\"b h\""));
}

TEST_CASE("PianoSVG keys with empty vector", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::keys(piano, {}, "Empty");
    REQUIRE(contains(svg, "Empty"));
    REQUIRE(!contains(svg, "class=\"w h\""));
    REQUIRE(!contains(svg, "class=\"b h\""));
}

// ---------------------------------------------------------------------------
// Compact mode
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG compact note is smaller than full", "[piano_svg]") {
    Piano piano;
    auto full = PianoSVG::note(piano, Note("C"), 4, false);
    auto compact = PianoSVG::note(piano, Note("C"), 4, true);
    REQUIRE(compact.size() < full.size());
}

TEST_CASE("PianoSVG compact note contains highlighted key", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4, true);
    REQUIRE(contains(svg, "class=\"w h\""));
    REQUIRE(contains(svg, "C4"));
}

TEST_CASE("PianoSVG compact omits distant keys", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::note(piano, Note("C"), 4, true);
    // A0 (MIDI 21) is far from C4 (MIDI 60), should not appear
    REQUIRE(!contains(svg, "id=\"key-21\""));
    // C8 (MIDI 108) also far
    REQUIRE(!contains(svg, "id=\"key-108\""));
}

TEST_CASE("PianoSVG compact chord works", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::chord(piano, Chord("CM"), 4, VoicingStyle::Close, true);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "CM"));
    REQUIRE(count(svg, "class=\"w h\"") >= 3);
}

TEST_CASE("PianoSVG compact scale works", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::scale(piano, Scale("C", ScaleType::Major), 4, true);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(count(svg, "class=\"w h\"") >= 7);
}

TEST_CASE("PianoSVG compact midi works", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::midi(piano, {60, 64, 67}, true);
    REQUIRE(contains(svg, "C4"));
    REQUIRE(contains(svg, "E4"));
    REQUIRE(contains(svg, "G4"));
}

TEST_CASE("PianoSVG compact empty highlights uses default range", "[piano_svg]") {
    Piano piano;
    auto svg = PianoSVG::midi(piano, {}, true);
    REQUIRE(contains(svg, "<svg"));
    // Default range C3-B4 (MIDI 48-71), should contain key-60
    REQUIRE(contains(svg, "id=\"key-60\""));
    // Should NOT contain A0
    REQUIRE(!contains(svg, "id=\"key-21\""));
}

// ---------------------------------------------------------------------------
// Field composite
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG field vertical generates composite SVG", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::field(piano, f);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
    REQUIRE(count(svg, "<style>") == 1);
}

TEST_CASE("PianoSVG field has 7 degree labels", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::field(piano, f);
    REQUIRE(contains(svg, "I - CM"));
    REQUIRE(contains(svg, "II - Dm"));
    REQUIRE(contains(svg, "VII - Bdim"));
}

TEST_CASE("PianoSVG field sevenths", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::field(piano, f, 4, Layout::Vertical, true);
    REQUIRE(contains(svg, "I - C7M"));
    REQUIRE(contains(svg, "V - G7"));
}

TEST_CASE("PianoSVG field horizontal layout", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::field(piano, f, 4, Layout::Horizontal);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "I - CM"));
    REQUIRE(contains(svg, "VII - Bdim"));
}

TEST_CASE("PianoSVG field grid layout", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::field(piano, f, 4, Layout::Grid);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "I - CM"));
}

// ---------------------------------------------------------------------------
// Progression composite
// ---------------------------------------------------------------------------

TEST_CASE("PianoSVG progression generates composite SVG", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::progression(piano, f, {"I", "IIm", "V", "I"});
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(count(svg, "<style>") == 1);
}

TEST_CASE("PianoSVG progression shows branch and chord labels", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::progression(piano, f, {"I", "V"});
    REQUIRE(contains(svg, "I (CM)"));
    REQUIRE(contains(svg, "V (GM)"));
}

TEST_CASE("PianoSVG progression horizontal layout", "[piano_svg]") {
    Piano piano;
    Field f("C", ScaleType::Major);
    auto svg = PianoSVG::progression(piano, f, {"I", "IIm", "V"},
                                      4, Layout::Horizontal);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "I (CM)"));
}
