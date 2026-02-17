// Gingo — Music Theory Library
// Tests for Piano class.
//
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <gingo/piano.hpp>

using namespace gingo;

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

TEST_CASE("Piano default 88 keys", "[piano]") {
    Piano piano;
    REQUIRE(piano.num_keys() == 88);
    REQUIRE(piano.lowest().midi == 21);   // A0
    REQUIRE(piano.highest().midi == 108); // C8
}

TEST_CASE("Piano 61 keys", "[piano]") {
    Piano piano(61);
    REQUIRE(piano.num_keys() == 61);
    // Centered around middle C (60).
    REQUIRE(piano.in_range(60));
}

TEST_CASE("Piano invalid num_keys throws", "[piano]") {
    REQUIRE_THROWS(Piano(0));
    REQUIRE_THROWS(Piano(-1));
    REQUIRE_THROWS(Piano(129));
}

// ---------------------------------------------------------------------------
// PianoKey: key()
// ---------------------------------------------------------------------------

TEST_CASE("Piano key middle C", "[piano]") {
    Piano piano;
    auto k = piano.key(Note("C"), 4);
    REQUIRE(k.midi == 60);
    REQUIRE(k.octave == 4);
    REQUIRE(k.note == "C");
    REQUIRE(k.white == true);
}

TEST_CASE("Piano key A0 is first key", "[piano]") {
    Piano piano;
    auto k = piano.key(Note("A"), 0);
    REQUIRE(k.midi == 21);
    REQUIRE(k.position == 1);
}

TEST_CASE("Piano key C8 is last key", "[piano]") {
    Piano piano;
    auto k = piano.key(Note("C"), 8);
    REQUIRE(k.midi == 108);
    REQUIRE(k.position == 88);
}

TEST_CASE("Piano key black key", "[piano]") {
    Piano piano;
    auto k = piano.key(Note("C#"), 4);
    REQUIRE(k.midi == 61);
    REQUIRE(k.white == false);
}

TEST_CASE("Piano key Bb is black", "[piano]") {
    Piano piano;
    auto k = piano.key(Note("Bb"), 4);
    // Bb = A# = semitone 10, MIDI = 10 + 12*5 = 70
    REQUIRE(k.midi == 70);
    REQUIRE(k.white == false);
}

TEST_CASE("Piano key out of range throws", "[piano]") {
    Piano piano;
    REQUIRE_THROWS(piano.key(Note("C"), 9));  // C9 = 120, beyond 108
    REQUIRE_THROWS(piano.key(Note("C"), -1)); // C-1 = 0, below 21
}

// ---------------------------------------------------------------------------
// keys(): all octaves
// ---------------------------------------------------------------------------

TEST_CASE("Piano keys C across octaves", "[piano]") {
    Piano piano;
    auto ks = piano.keys(Note("C"));
    // C1=24, C2=36, ..., C8=108 → 8 keys
    REQUIRE(ks.size() == 8);
    REQUIRE(ks.front().midi == 24);  // C1
    REQUIRE(ks.back().midi == 108);  // C8
}

TEST_CASE("Piano keys A across octaves", "[piano]") {
    Piano piano;
    auto ks = piano.keys(Note("A"));
    // A0=21, A1=33, ..., A7=105 → 8 keys
    REQUIRE(ks.size() == 8);
    REQUIRE(ks.front().midi == 21);  // A0
}

// ---------------------------------------------------------------------------
// voicing()
// ---------------------------------------------------------------------------

TEST_CASE("Piano voicing CM close", "[piano]") {
    Piano piano;
    auto v = piano.voicing(Chord("CM"), 4, VoicingStyle::Close);
    REQUIRE(v.chord_name == "CM");
    REQUIRE(v.style == VoicingStyle::Close);
    REQUIRE(v.keys.size() == 3);
    // C4=60, E4=64, G4=67
    REQUIRE(v.keys[0].midi == 60);
    REQUIRE(v.keys[1].midi == 64);
    REQUIRE(v.keys[2].midi == 67);
}

TEST_CASE("Piano voicing CM open", "[piano]") {
    Piano piano;
    auto v = piano.voicing(Chord("CM"), 4, VoicingStyle::Open);
    REQUIRE(v.keys.size() == 3);
    // Root drops to C3=48, E4=64, G4=67
    REQUIRE(v.keys[0].midi == 48);
    REQUIRE(v.keys[1].midi == 64);
    REQUIRE(v.keys[2].midi == 67);
}

TEST_CASE("Piano voicing Am7 close", "[piano]") {
    Piano piano;
    auto v = piano.voicing(Chord("Am7"), 4, VoicingStyle::Close);
    REQUIRE(v.keys.size() == 4);
    // A4=69, C5=72, E5=76, G5=79
    REQUIRE(v.keys[0].midi == 69);
    REQUIRE(v.keys[1].midi == 72);
    REQUIRE(v.keys[2].midi == 76);
    REQUIRE(v.keys[3].midi == 79);
}

TEST_CASE("Piano voicing Am7 shell", "[piano]") {
    Piano piano;
    auto v = piano.voicing(Chord("Am7"), 4, VoicingStyle::Shell);
    // Shell: root + 3rd + 7th → A, C, G → 3 keys
    REQUIRE(v.keys.size() == 3);
}

// ---------------------------------------------------------------------------
// voicings()
// ---------------------------------------------------------------------------

TEST_CASE("Piano voicings returns three styles", "[piano]") {
    Piano piano;
    auto vs = piano.voicings(Chord("CM"), 4);
    REQUIRE(vs.size() == 3);
    REQUIRE(vs[0].style == VoicingStyle::Close);
    REQUIRE(vs[1].style == VoicingStyle::Open);
    REQUIRE(vs[2].style == VoicingStyle::Shell);
}

// ---------------------------------------------------------------------------
// scale_keys()
// ---------------------------------------------------------------------------

TEST_CASE("Piano scale_keys C major", "[piano]") {
    Piano piano;
    auto ks = piano.scale_keys(Scale("C", ScaleType::Major), 4);
    REQUIRE(ks.size() == 7);
    // C D E F G A B → all white keys
    for (const auto& k : ks) {
        REQUIRE(k.white == true);
    }
    REQUIRE(ks[0].midi == 60);  // C4
    REQUIRE(ks[6].midi == 71);  // B4
}

TEST_CASE("Piano scale_keys C major pentatonic", "[piano]") {
    Piano piano;
    Scale penta("C", "major pentatonic");
    auto ks = piano.scale_keys(penta, 4);
    REQUIRE(ks.size() == 5);
}

// ---------------------------------------------------------------------------
// Reverse: note_at()
// ---------------------------------------------------------------------------

TEST_CASE("Piano note_at middle C", "[piano]") {
    Piano piano;
    auto n = piano.note_at(60);
    REQUIRE(n.name() == "C");
}

TEST_CASE("Piano note_at A440", "[piano]") {
    Piano piano;
    auto n = piano.note_at(69);
    REQUIRE(n.name() == "A");
}

TEST_CASE("Piano note_at out of range throws", "[piano]") {
    Piano piano;
    REQUIRE_THROWS(piano.note_at(0));   // below A0
    REQUIRE_THROWS(piano.note_at(127)); // above C8
}

// ---------------------------------------------------------------------------
// Reverse: identify()
// ---------------------------------------------------------------------------

TEST_CASE("Piano identify CM from MIDI", "[piano]") {
    Piano piano;
    auto c = piano.identify({60, 64, 67});
    REQUIRE(c.name() == "CM");
}

TEST_CASE("Piano identify Am7 from MIDI", "[piano]") {
    Piano piano;
    auto c = piano.identify({69, 72, 76, 79});
    REQUIRE(c.name() == "Am7");
}

TEST_CASE("Piano identify with duplicate octaves", "[piano]") {
    Piano piano;
    // C3 + E4 + G4 + C5 → still CM
    auto c = piano.identify({48, 64, 67, 72});
    REQUIRE(c.name() == "CM");
}

TEST_CASE("Piano identify empty throws", "[piano]") {
    Piano piano;
    REQUIRE_THROWS(piano.identify({}));
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

TEST_CASE("Piano to_string", "[piano]") {
    Piano piano;
    auto s = piano.to_string();
    REQUIRE(s.find("88") != std::string::npos);
}

TEST_CASE("PianoKey to_string", "[piano]") {
    Piano piano;
    auto k = piano.key(Note("C"), 4);
    auto s = k.to_string();
    REQUIRE(s.find("C4") != std::string::npos);
    REQUIRE(s.find("60") != std::string::npos);
}

TEST_CASE("PianoVoicing to_string", "[piano]") {
    Piano piano;
    auto v = piano.voicing(Chord("CM"));
    auto s = v.to_string();
    REQUIRE(s.find("CM") != std::string::npos);
}
