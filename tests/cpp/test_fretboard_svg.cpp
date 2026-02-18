// Gingo — Music Theory Library
// Tests for FretboardSVG class.
//
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/fretboard_svg.hpp>
#include <gingo/field.hpp>

#include <filesystem>
#include <fstream>
#include <string>

using namespace gingo;

// ---------------------------------------------------------------------------
// Helper
// ---------------------------------------------------------------------------

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

// ---------------------------------------------------------------------------
// Chord diagram
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG chord generates valid SVG", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("CM"));
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
    REQUIRE(contains(svg, "xmlns=\"http://www.w3.org/2000/svg\""));
}

TEST_CASE("FretboardSVG chord contains title", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("Am7"));
    REQUIRE(contains(svg, "Am7"));
}

TEST_CASE("FretboardSVG chord has CSS style", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("CM"));
    REQUIRE(contains(svg, "<style>"));
    REQUIRE(contains(svg, "</style>"));
}

TEST_CASE("FretboardSVG chord has string lines", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("CM"));
    REQUIRE(contains(svg, "class=\"str\""));
}

TEST_CASE("FretboardSVG chord has fret lines", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("CM"));
    REQUIRE(contains(svg, "class=\"frt\""));
}

TEST_CASE("FretboardSVG chord has dots", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("CM"));
    REQUIRE(contains(svg, "class=\"dot\""));
}

TEST_CASE("FretboardSVG chord on cavaquinho", "[fretboard_svg]") {
    auto c = Fretboard::cavaquinho();
    auto svg = FretboardSVG::chord(c, Chord("CM"));
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "CM"));
}

TEST_CASE("FretboardSVG chord on bandolim", "[fretboard_svg]") {
    auto b = Fretboard::bandolim();
    auto svg = FretboardSVG::chord(b, Chord("CM"));
    REQUIRE(contains(svg, "<svg"));
}

// ---------------------------------------------------------------------------
// Fingering
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG fingering renders specific fingering", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto f = g.fingering(Chord("Am"));
    auto svg = FretboardSVG::fingering(g, f);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "Am"));
}

// ---------------------------------------------------------------------------
// Scale
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG scale generates valid SVG", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::scale(g, Scale("C", ScaleType::Major), 0, 12);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
}

TEST_CASE("FretboardSVG scale contains mode name", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::scale(g, Scale("C", ScaleType::Major), 0, 5);
    REQUIRE(contains(svg, "Ionian"));
}

TEST_CASE("FretboardSVG scale has string labels", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::scale(g, Scale("C", ScaleType::Major), 0, 5);
    // Should contain open string labels like E4, B3, etc.
    REQUIRE(contains(svg, "E4"));
}

// ---------------------------------------------------------------------------
// Note
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG note shows all positions", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::note(g, Note("C"));
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, ">C<"));
}

// ---------------------------------------------------------------------------
// Positions
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG positions with custom title", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto pos = g.positions(Note("E"));
    auto svg = FretboardSVG::positions(g, pos, "All E notes");
    REQUIRE(contains(svg, "All E notes"));
}

// ---------------------------------------------------------------------------
// Field composite
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG field vertical generates SVG", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::field(g, f);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
}

TEST_CASE("FretboardSVG field has degree labels", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::field(g, f);
    REQUIRE(contains(svg, "I - CM"));
    REQUIRE(contains(svg, "VII - Bdim"));
}

TEST_CASE("FretboardSVG field grid layout", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::field(g, f, Layout::Grid);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "I - CM"));
}

TEST_CASE("FretboardSVG field horizontal layout", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::field(g, f, Layout::Horizontal);
    REQUIRE(contains(svg, "<svg"));
}

// ---------------------------------------------------------------------------
// Progression composite
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG progression generates SVG", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::progression(g, f, {"I", "IV", "V", "I"});
    REQUIRE(contains(svg, "<svg"));
}

TEST_CASE("FretboardSVG progression shows labels", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::progression(g, f, {"I", "V"});
    REQUIRE(contains(svg, "I (CM)"));
    REQUIRE(contains(svg, "V (GM)"));
}

// ---------------------------------------------------------------------------
// Write
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Orientation
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG chord horizontal orientation", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("CM"), 0, Orientation::Horizontal);
    REQUIRE(contains(svg, "<svg"));
    // Horizontal fretboard uses slbl class for string labels.
    REQUIRE(contains(svg, "E4"));
}

TEST_CASE("FretboardSVG scale vertical orientation", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::scale(g, Scale("C", ScaleType::Major), 0, 5,
                                     Orientation::Vertical);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "Ionian"));
}

TEST_CASE("FretboardSVG note vertical orientation", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::note(g, Note("C"), Orientation::Vertical);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, ">C<"));
}

// ---------------------------------------------------------------------------
// Handedness
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG chord left-handed", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg_r = FretboardSVG::chord(g, Chord("CM"), 0,
                     Orientation::Vertical, Handedness::RightHanded);
    auto svg_l = FretboardSVG::chord(g, Chord("CM"), 0,
                     Orientation::Vertical, Handedness::LeftHanded);
    // Both are valid SVGs.
    REQUIRE(contains(svg_r, "<svg"));
    REQUIRE(contains(svg_l, "<svg"));
    // They should differ (mirrored strings).
    REQUIRE(svg_r != svg_l);
}

TEST_CASE("FretboardSVG scale left-handed", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg_r = FretboardSVG::scale(g, Scale("C", ScaleType::Major), 0, 5,
                     Orientation::Horizontal, Handedness::RightHanded);
    auto svg_l = FretboardSVG::scale(g, Scale("C", ScaleType::Major), 0, 5,
                     Orientation::Horizontal, Handedness::LeftHanded);
    REQUIRE(contains(svg_r, "<svg"));
    REQUIRE(contains(svg_l, "<svg"));
    REQUIRE(svg_r != svg_l);
}

TEST_CASE("FretboardSVG chord horizontal left-handed", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("Am"), 0,
                   Orientation::Horizontal, Handedness::LeftHanded);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
}

// ---------------------------------------------------------------------------
// Composite with orientation/handedness
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG field horizontal orientation", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::field(g, f, Layout::Vertical,
                                     Orientation::Horizontal);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "E4"));
}

TEST_CASE("FretboardSVG progression left-handed", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    Field f("C", ScaleType::Major);
    auto svg = FretboardSVG::progression(g, f, {"I", "V"},
                   Layout::Vertical, Orientation::Vertical,
                   Handedness::LeftHanded);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "I (CM)"));
}

// ---------------------------------------------------------------------------
// Default backward compatibility
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG defaults produce same output", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    // chord() defaults: Vertical, RightHanded — same as explicit.
    auto svg1 = FretboardSVG::chord(g, Chord("CM"));
    auto svg2 = FretboardSVG::chord(g, Chord("CM"), 0,
                    Orientation::Vertical, Handedness::RightHanded);
    REQUIRE(svg1 == svg2);
}

// ---------------------------------------------------------------------------
// Full fretboard
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG full generates SVG", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::full(g);
    REQUIRE(contains(svg, "<svg"));
    REQUIRE(contains(svg, "</svg>"));
    REQUIRE(contains(svg, "class=\"nut\""));
}

TEST_CASE("FretboardSVG full shows instrument name", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::full(g);
    REQUIRE(contains(svg, g.name()));
}

TEST_CASE("FretboardSVG full vertical", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::full(g, Orientation::Vertical);
    REQUIRE(contains(svg, "<svg"));
}

// ---------------------------------------------------------------------------
// Write
// ---------------------------------------------------------------------------

TEST_CASE("FretboardSVG write creates file", "[fretboard_svg]") {
    auto g = Fretboard::violao();
    auto svg = FretboardSVG::chord(g, Chord("CM"));
    std::string path = (std::filesystem::temp_directory_path() / "gingo_test_fretboard_svg.svg").string();
    FretboardSVG::write(svg, path);

    std::ifstream in(path);
    REQUIRE(in.is_open());
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    REQUIRE(content == svg);
    std::remove(path.c_str());
}
