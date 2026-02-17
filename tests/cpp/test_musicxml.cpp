// Gingo — Music Theory Library
// Tests for MusicXML serializer.
//
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/musicxml.hpp>

#include <filesystem>

using namespace gingo;

// ---------------------------------------------------------------------------
// Note
// ---------------------------------------------------------------------------

TEST_CASE("MusicXML note C4", "[musicxml]") {
    auto xml = MusicXML::note(Note("C"), 4);
    REQUIRE(xml.find("<step>C</step>") != std::string::npos);
    REQUIRE(xml.find("<octave>4</octave>") != std::string::npos);
    REQUIRE(xml.find("<type>quarter</type>") != std::string::npos);
    REQUIRE(xml.find("<score-partwise") != std::string::npos);
    REQUIRE(xml.find("</score-partwise>") != std::string::npos);
}

TEST_CASE("MusicXML note with sharp", "[musicxml]") {
    auto xml = MusicXML::note(Note("C#"), 4);
    REQUIRE(xml.find("<step>C</step>") != std::string::npos);
    REQUIRE(xml.find("<alter>1</alter>") != std::string::npos);
}

TEST_CASE("MusicXML note with flat", "[musicxml]") {
    auto xml = MusicXML::note(Note("Bb"), 4);
    REQUIRE(xml.find("<step>B</step>") != std::string::npos);
    REQUIRE(xml.find("<alter>-1</alter>") != std::string::npos);
}

TEST_CASE("MusicXML note natural has no alter", "[musicxml]") {
    auto xml = MusicXML::note(Note("G"), 5);
    REQUIRE(xml.find("<alter>") == std::string::npos);
}

TEST_CASE("MusicXML note whole type", "[musicxml]") {
    auto xml = MusicXML::note(Note("E"), 3, "whole");
    REQUIRE(xml.find("<type>whole</type>") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Chord
// ---------------------------------------------------------------------------

TEST_CASE("MusicXML chord CM", "[musicxml]") {
    auto xml = MusicXML::chord(Chord("CM"), 4);
    // First note is NOT a chord member, subsequent are.
    // Count <note> elements.
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = xml.find("<note>", pos)) != std::string::npos) {
        count++;
        pos++;
    }
    REQUIRE(count == 3);  // C, E, G

    // Second and third notes should have <chord/>.
    std::size_t chord_count = 0;
    pos = 0;
    while ((pos = xml.find("<chord/>", pos)) != std::string::npos) {
        chord_count++;
        pos++;
    }
    REQUIRE(chord_count == 2);
}

TEST_CASE("MusicXML chord Am7 has 4 notes", "[musicxml]") {
    auto xml = MusicXML::chord(Chord("Am7"), 4);
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = xml.find("<note>", pos)) != std::string::npos) {
        count++;
        pos++;
    }
    REQUIRE(count == 4);
}

// ---------------------------------------------------------------------------
// Scale
// ---------------------------------------------------------------------------

TEST_CASE("MusicXML scale C major", "[musicxml]") {
    auto xml = MusicXML::scale(Scale("C", ScaleType::Major), 4);
    // 7 notes in the scale.
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = xml.find("<note>", pos)) != std::string::npos) {
        count++;
        pos++;
    }
    REQUIRE(count == 7);

    // No <chord/> tags (sequential notes).
    REQUIRE(xml.find("<chord/>") == std::string::npos);
}

// ---------------------------------------------------------------------------
// Field
// ---------------------------------------------------------------------------

TEST_CASE("MusicXML field C major has 7 measures", "[musicxml]") {
    auto xml = MusicXML::field(Field("C", ScaleType::Major), 4);
    std::size_t count = 0;
    std::size_t pos = 0;
    while ((pos = xml.find("<measure number=", pos)) != std::string::npos) {
        count++;
        pos++;
    }
    REQUIRE(count == 7);
}

// ---------------------------------------------------------------------------
// Sequence
// ---------------------------------------------------------------------------

TEST_CASE("MusicXML sequence basic", "[musicxml]") {
    Sequence seq(Tempo(120), TimeSignature(4, 4));
    seq.add(NoteEvent(Note("C"), Duration("quarter"), 4));
    seq.add(NoteEvent(Note("E"), Duration("quarter"), 4));
    seq.add(Rest(Duration("half")));

    auto xml = MusicXML::sequence(seq);
    REQUIRE(xml.find("<score-partwise") != std::string::npos);
    REQUIRE(xml.find("<step>C</step>") != std::string::npos);
    REQUIRE(xml.find("<step>E</step>") != std::string::npos);
    REQUIRE(xml.find("<rest/>") != std::string::npos);
}

// ---------------------------------------------------------------------------
// Write
// ---------------------------------------------------------------------------

TEST_CASE("MusicXML write to file", "[musicxml]") {
    auto xml = MusicXML::note(Note("C"), 4);
    std::string path = "/tmp/gingo_test_musicxml.xml";
    MusicXML::write(xml, path);
    REQUIRE(std::filesystem::exists(path));
    std::filesystem::remove(path);
}

// ---------------------------------------------------------------------------
// Header/footer structure
// ---------------------------------------------------------------------------

TEST_CASE("MusicXML valid document structure", "[musicxml]") {
    auto xml = MusicXML::note(Note("D"), 4);
    REQUIRE(xml.find("<?xml version=") != std::string::npos);
    REQUIRE(xml.find("<!DOCTYPE score-partwise") != std::string::npos);
    REQUIRE(xml.find("<software>Gingo</software>") != std::string::npos);
    REQUIRE(xml.find("<part-name>Piano</part-name>") != std::string::npos);
    REQUIRE(xml.find("<divisions>") != std::string::npos);
}
