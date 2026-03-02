// Gingo — MIDI import/export + Monitor/MIDI1/MIDI2 tests
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <gingo/sequence.hpp>
#include <gingo/internal/midi_format.hpp>
#include <gingo/monitor.hpp>
#include <gingo/midi1.hpp>
#include <gingo/midi2.hpp>
#include <fstream>
#include <cstdio>

using namespace gingo;
using namespace gingo::internal;
using Catch::Matchers::WithinAbs;

// ===========================================================================
// VLQ encoding/decoding
// ===========================================================================

TEST_CASE("VLQ encode zero", "[midi]") {
    std::vector<uint8_t> buf;
    write_vlq(buf, 0);
    REQUIRE(buf.size() == 1);
    REQUIRE(buf[0] == 0x00);
}

TEST_CASE("VLQ encode small value", "[midi]") {
    std::vector<uint8_t> buf;
    write_vlq(buf, 127);
    REQUIRE(buf.size() == 1);
    REQUIRE(buf[0] == 0x7F);
}

TEST_CASE("VLQ encode two bytes", "[midi]") {
    std::vector<uint8_t> buf;
    write_vlq(buf, 128);
    REQUIRE(buf.size() == 2);
    REQUIRE(buf[0] == 0x81);
    REQUIRE(buf[1] == 0x00);
}

TEST_CASE("VLQ encode 480", "[midi]") {
    std::vector<uint8_t> buf;
    write_vlq(buf, 480);
    // 480 = 0x1E0 -> 0b 11 1100000 -> 0x83, 0x60
    REQUIRE(buf.size() == 2);
    REQUIRE(buf[0] == 0x83);
    REQUIRE(buf[1] == 0x60);
}

TEST_CASE("VLQ roundtrip", "[midi]") {
    for (uint32_t val : {0u, 1u, 127u, 128u, 480u, 960u, 1920u, 16383u, 65535u}) {
        std::vector<uint8_t> buf;
        write_vlq(buf, val);
        size_t pos = 0;
        uint32_t result = read_vlq(buf, pos);
        REQUIRE(result == val);
        REQUIRE(pos == buf.size());
    }
}

// ===========================================================================
// Big-endian integers
// ===========================================================================

TEST_CASE("uint16 roundtrip", "[midi]") {
    std::vector<uint8_t> buf;
    write_uint16(buf, 480);
    size_t pos = 0;
    REQUIRE(read_uint16(buf, pos) == 480);
}

TEST_CASE("uint32 roundtrip", "[midi]") {
    std::vector<uint8_t> buf;
    write_uint32(buf, 123456);
    size_t pos = 0;
    REQUIRE(read_uint32(buf, pos) == 123456);
}

// ===========================================================================
// Sequence.to_midi()
// ===========================================================================

TEST_CASE("to_midi writes valid MIDI header", "[midi]") {
    Sequence seq(Tempo(120), TimeSignature(4, 4));
    seq.add(NoteEvent(Note("C"), Duration("quarter"), 4));

    std::string path = "/tmp/gingo_test_header.mid";
    seq.to_midi(path);

    std::ifstream f(path, std::ios::binary);
    REQUIRE(f.good());

    char hdr[4];
    f.read(hdr, 4);
    REQUIRE(std::string(hdr, 4) == "MThd");

    // Header length = 6
    char len[4];
    f.read(len, 4);
    uint32_t hdr_len = (static_cast<uint8_t>(len[0]) << 24) |
                       (static_cast<uint8_t>(len[1]) << 16) |
                       (static_cast<uint8_t>(len[2]) << 8) |
                        static_cast<uint8_t>(len[3]);
    REQUIRE(hdr_len == 6);

    // Format 0
    char fmt[2];
    f.read(fmt, 2);
    REQUIRE(static_cast<uint8_t>(fmt[1]) == 0);

    // 1 track
    char trk[2];
    f.read(trk, 2);
    REQUIRE(static_cast<uint8_t>(trk[1]) == 1);

    std::remove(path.c_str());
}

TEST_CASE("to_midi with notes", "[midi]") {
    Sequence seq(Tempo(120), TimeSignature(4, 4));
    seq.add(NoteEvent(Note("C"), Duration("quarter"), 4));
    seq.add(NoteEvent(Note("E"), Duration("quarter"), 4));
    seq.add(Rest(Duration("half")));

    std::string path = "/tmp/gingo_test_notes.mid";
    REQUIRE_NOTHROW(seq.to_midi(path));

    // File should exist and be non-empty
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    REQUIRE(f.good());
    REQUIRE(f.tellg() > 14);

    std::remove(path.c_str());
}

TEST_CASE("to_midi with chords", "[midi]") {
    Sequence seq(Tempo(100), TimeSignature(4, 4));
    seq.add(ChordEvent(Chord("CM"), Duration("whole"), 4));

    std::string path = "/tmp/gingo_test_chord.mid";
    REQUIRE_NOTHROW(seq.to_midi(path));

    std::ifstream f(path, std::ios::binary | std::ios::ate);
    REQUIRE(f.good());
    REQUIRE(f.tellg() > 14);

    std::remove(path.c_str());
}

TEST_CASE("to_midi custom ppqn", "[midi]") {
    Sequence seq(Tempo(120), TimeSignature(4, 4));
    seq.add(NoteEvent(Note("C"), Duration("quarter"), 4));

    std::string path = "/tmp/gingo_test_ppqn.mid";
    REQUIRE_NOTHROW(seq.to_midi(path, 96));

    // Read ppqn from header
    std::ifstream f(path, std::ios::binary);
    f.seekg(12);  // skip to ppqn field
    char ppqn_bytes[2];
    f.read(ppqn_bytes, 2);
    uint16_t ppqn = (static_cast<uint8_t>(ppqn_bytes[0]) << 8) |
                     static_cast<uint8_t>(ppqn_bytes[1]);
    REQUIRE(ppqn == 96);

    std::remove(path.c_str());
}

// ===========================================================================
// MIDI roundtrip
// ===========================================================================

TEST_CASE("MIDI roundtrip single notes", "[midi]") {
    Sequence original(Tempo(120), TimeSignature(4, 4));
    original.add(NoteEvent(Note("C"), Duration("quarter"), 4));
    original.add(NoteEvent(Note("E"), Duration("quarter"), 4));
    original.add(NoteEvent(Note("G"), Duration("half"), 4));

    std::string path = "/tmp/gingo_roundtrip.mid";
    original.to_midi(path);

    Sequence loaded = Sequence::from_midi(path);
    REQUIRE(loaded.size() == 3);
    REQUIRE_THAT(loaded.total_duration(), WithinAbs(4.0, 0.1));
    REQUIRE_THAT(loaded.tempo().bpm(), WithinAbs(120.0, 1.0));

    std::remove(path.c_str());
}

TEST_CASE("MIDI roundtrip with rest", "[midi]") {
    Sequence original(Tempo(90), TimeSignature(4, 4));
    original.add(NoteEvent(Note("A"), Duration("quarter"), 4));
    original.add(Rest(Duration("quarter")));
    original.add(NoteEvent(Note("B"), Duration("half"), 4));

    std::string path = "/tmp/gingo_roundtrip_rest.mid";
    original.to_midi(path);

    Sequence loaded = Sequence::from_midi(path);
    // Should have 3 events (note, rest, note)
    REQUIRE(loaded.size() == 3);
    REQUIRE_THAT(loaded.total_duration(), WithinAbs(4.0, 0.1));
    REQUIRE_THAT(loaded.tempo().bpm(), WithinAbs(90.0, 1.0));

    std::remove(path.c_str());
}

TEST_CASE("MIDI roundtrip with chord", "[midi]") {
    Sequence original(Tempo(120), TimeSignature(4, 4));
    original.add(ChordEvent(Chord("CM"), Duration("whole"), 4));

    std::string path = "/tmp/gingo_roundtrip_chord.mid";
    original.to_midi(path);

    Sequence loaded = Sequence::from_midi(path);
    // Chord with 3 notes should be reconstructed as either a ChordEvent or NoteEvents
    REQUIRE(loaded.size() >= 1);
    REQUIRE_THAT(loaded.total_duration(), WithinAbs(4.0, 0.1));

    std::remove(path.c_str());
}

TEST_CASE("from_midi invalid file throws", "[midi]") {
    REQUIRE_THROWS_AS(Sequence::from_midi("/nonexistent/file.mid"), std::runtime_error);
}

TEST_CASE("from_midi invalid content throws", "[midi]") {
    std::string path = "/tmp/gingo_invalid.mid";
    std::ofstream f(path, std::ios::binary);
    f << "not a midi file";
    f.close();

    REQUIRE_THROWS_AS(Sequence::from_midi(path), std::runtime_error);

    std::remove(path.c_str());
}

// ===========================================================================
// Monitor
// ===========================================================================

TEST_CASE("Monitor starts empty", "[monitor]") {
    Monitor mon;
    REQUIRE(mon.activeNoteCount() == 0);
    REQUIRE_FALSE(mon.hasChord());
    REQUIRE_FALSE(mon.hasField());
    REQUIRE_FALSE(mon.hasSustain());
}

TEST_CASE("Monitor detects CM chord", "[monitor]") {
    Monitor mon;
    mon.noteOn(60);  // C4
    mon.noteOn(64);  // E4
    mon.noteOn(67);  // G4

    REQUIRE(mon.activeNoteCount() == 3);
    REQUIRE(mon.hasChord());
    REQUIRE(mon.currentChord().name() == "CM");
}

TEST_CASE("Monitor detects Am chord (bass note A3 below C4 E4)", "[monitor]") {
    Monitor mon;
    // A3(57) < C4(60) < E4(64) — sorted by MIDI preserves A as bass → Am
    mon.noteOn(57);  // A3
    mon.noteOn(60);  // C4
    mon.noteOn(64);  // E4

    REQUIRE(mon.hasChord());
    REQUIRE(mon.currentChord().name() == "Am");
}

TEST_CASE("Monitor chord callback fires", "[monitor]") {
    Monitor mon;
    std::string detected;
    mon.onChordDetected([&](const Chord& c) { detected = c.name(); });

    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);  // CM

    REQUIRE(detected == "CM");
}

TEST_CASE("Monitor chord callback fires (function pointer)", "[monitor]") {
    Monitor mon;
    static std::string detected;
    detected.clear();
    mon.onChordDetected([](const Chord& c, void*) { detected = c.name(); }, nullptr);

    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);  // CM

    REQUIRE(detected == "CM");
}

TEST_CASE("Monitor fires both callbacks when both registered", "[monitor]") {
    Monitor mon;
    int fnCount = 0, cbCount = 0;
    mon.onChordDetected([&](const Chord&) { fnCount++; });
    mon.onChordDetected([](const Chord&, void* ctx) {
        (*static_cast<int*>(ctx))++;
    }, &cbCount);

    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);  // CM

    REQUIRE(fnCount == 1);
    REQUIRE(cbCount == 1);
}

TEST_CASE("Monitor noteOff removes note", "[monitor]") {
    Monitor mon;
    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);
    REQUIRE(mon.activeNoteCount() == 3);

    mon.noteOff(67);
    REQUIRE(mon.activeNoteCount() == 2);
}

TEST_CASE("Monitor reset clears state", "[monitor]") {
    Monitor mon;
    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);
    mon.reset();

    REQUIRE(mon.activeNoteCount() == 0);
    REQUIRE_FALSE(mon.hasChord());
    REQUIRE_FALSE(mon.hasField());
}

TEST_CASE("Monitor sustain pedal holds notes", "[monitor]") {
    Monitor mon;
    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);  // CM
    mon.sustainOn();
    REQUIRE(mon.hasSustain());

    mon.noteOff(60); mon.noteOff(64); mon.noteOff(67);
    REQUIRE(mon.activeNoteCount() == 3);  // still held by sustain

    mon.sustainOff();
    REQUIRE(mon.activeNoteCount() == 0);
}

TEST_CASE("Monitor noteFromMIDI middle C", "[monitor]") {
    Note n = Monitor::noteFromMIDI(60);
    REQUIRE(n.semitone() == 0);  // C
}

TEST_CASE("Monitor noteFromMIDI A4 = 69", "[monitor]") {
    Note n = Monitor::noteFromMIDI(69);
    REQUIRE(n.semitone() == 9);  // A
}

TEST_CASE("Monitor deduces field from CM chord", "[monitor]") {
    Monitor mon;
    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);  // CM

    // CM is diatonic to C major (and others); monitor deduces first match
    REQUIRE(mon.hasField());
}

TEST_CASE("Monitor per-note callback fires with context", "[monitor]") {
    Monitor mon;
    // Build field first (need 3 notes)
    mon.noteOn(60); mon.noteOn(64); mon.noteOn(67);  // CM → C major field

    bool firedFn = false, firedCb = false;
    mon.onNoteOn([&](const NoteContext&) { firedFn = true; });
    mon.onNoteOn([](const NoteContext&, void* ctx) {
        *static_cast<bool*>(ctx) = true;
    }, &firedCb);

    mon.noteOn(62);  // D4 — triggers per-note callback

    REQUIRE(firedFn);
    REQUIRE(firedCb);
}

// ===========================================================================
// MIDI1
// ===========================================================================

TEST_CASE("MIDI1 dispatch note on", "[midi1]") {
    Monitor mon;
    bool handled = MIDI1::dispatch(0x90, 60, 100, mon);
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 1);
}

TEST_CASE("MIDI1 dispatch note off", "[midi1]") {
    Monitor mon;
    MIDI1::dispatch(0x90, 60, 100, mon);
    bool handled = MIDI1::dispatch(0x80, 60, 0, mon);
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 0);
}

TEST_CASE("MIDI1 note on with vel=0 treated as note off", "[midi1]") {
    Monitor mon;
    MIDI1::dispatch(0x90, 60, 100, mon);
    MIDI1::dispatch(0x90, 60, 0, mon);  // vel=0 → note off
    REQUIRE(mon.activeNoteCount() == 0);
}

TEST_CASE("MIDI1 CC64 sustain on/off", "[midi1]") {
    Monitor mon;
    MIDI1::dispatch(0xB0, 64, 127, mon);  // sustain on
    REQUIRE(mon.hasSustain());
    MIDI1::dispatch(0xB0, 64, 0, mon);    // sustain off
    REQUIRE_FALSE(mon.hasSustain());
}

TEST_CASE("MIDI1 CC123 all notes off", "[midi1]") {
    Monitor mon;
    MIDI1::dispatch(0x90, 60, 100, mon);
    MIDI1::dispatch(0x90, 64, 100, mon);
    MIDI1::dispatch(0xB0, 123, 0, mon);  // all notes off
    REQUIRE(mon.activeNoteCount() == 0);
}

TEST_CASE("MIDI1 channel nibble ignored", "[midi1]") {
    Monitor mon;
    MIDI1::dispatch(0x95, 60, 100, mon);  // channel 6, still note on
    REQUIRE(mon.activeNoteCount() == 1);
}

TEST_CASE("MIDI1 unhandled message returns false", "[midi1]") {
    Monitor mon;
    bool handled = MIDI1::dispatch(0xC0, 42, 0, mon);  // Program Change
    REQUIRE_FALSE(handled);
}

TEST_CASE("MIDI1Parser feed note on", "[midi1]") {
    Monitor mon;
    MIDI1Parser parser;
    parser.feed(0x90, mon);  // status
    parser.feed(60,   mon);  // data1
    bool handled = parser.feed(100, mon);  // data2 → complete
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 1);
}

TEST_CASE("MIDI1Parser running status", "[midi1]") {
    Monitor mon;
    MIDI1Parser parser;
    parser.feed(0x90, mon); parser.feed(60, mon); parser.feed(100, mon);  // C on
    parser.feed(64, mon);   parser.feed(100, mon);  // E on (running status)
    parser.feed(67, mon);   parser.feed(100, mon);  // G on (running status)
    REQUIRE(mon.activeNoteCount() == 3);
    REQUIRE(mon.hasChord());
    REQUIRE(mon.currentChord().name() == "CM");
}

TEST_CASE("MIDI1Parser sysex absorbed", "[midi1]") {
    Monitor mon;
    MIDI1Parser parser;
    parser.feed(0xF0, mon); parser.feed(0x41, mon); parser.feed(0xF7, mon);
    REQUIRE(mon.activeNoteCount() == 0);
}

TEST_CASE("MIDI1Parser real-time bytes ignored", "[midi1]") {
    Monitor mon;
    MIDI1Parser parser;
    parser.feed(0x90, mon);
    parser.feed(0xF8, mon);  // MIDI clock — ignore, no state change
    parser.feed(60,   mon);
    parser.feed(100,  mon);
    REQUIRE(mon.activeNoteCount() == 1);
}

TEST_CASE("MIDI1Parser reset clears state", "[midi1]") {
    Monitor mon;
    MIDI1Parser parser;
    parser.feed(0x90, mon); parser.feed(60, mon);  // partial
    parser.reset();
    REQUIRE(parser.status_ == 0);
    REQUIRE(parser.count_  == 0);
}

// ===========================================================================
// MIDI2 — UMP generation
// ===========================================================================

TEST_CASE("MIDI2 chordName CM produces 4-word UMP", "[midi2]") {
    Chord chord("CM");
    UMP ump = MIDI2::chordName(chord);
    REQUIRE(ump.wordCount == 4);
    // Message Type = 0xD (Flex Data)
    REQUIRE(((ump.words[0] >> 28) & 0xF) == 0xD);
    // Status = 0x06 (chord name)
    REQUIRE((ump.words[0] & 0xFF) == 0x06);
}

TEST_CASE("MIDI2 chordName encodes tonic correctly", "[midi2]") {
    Chord chord("AM");
    UMP ump = MIDI2::chordName(chord);
    // Tonic letter for A = 1, no accidental = 0
    uint8_t letter = (ump.words[1] >> 24) & 0x0F;
    uint8_t acc    = (ump.words[1] >> 28) & 0x0F;
    REQUIRE(letter == 1);  // A
    REQUIRE(acc    == 0);  // natural
}

TEST_CASE("MIDI2 chordName minor chord type", "[midi2]") {
    Chord chord("Am");
    UMP ump = MIDI2::chordName(chord);
    uint8_t chordType = (ump.words[1] >> 16) & 0xFF;
    REQUIRE(chordType == 7);  // MIDI 2.0 minor = 7
}

TEST_CASE("MIDI2 keySignature produces 4-word UMP", "[midi2]") {
    Scale scale("C", ScaleType::Major);
    UMP ump = MIDI2::keySignature(scale);
    REQUIRE(ump.wordCount == 4);
    REQUIRE(((ump.words[0] >> 28) & 0xF) == 0xD);
    REQUIRE((ump.words[0] & 0xFF) == 0x05);  // key signature status
}

TEST_CASE("MIDI2 perNoteController produces 2-word UMP", "[midi2]") {
    Field field("C", ScaleType::Major);
    NoteContext ctx = field.noteContext(Note("E"));

    UMP ump = MIDI2::perNoteController(64, ctx);
    REQUIRE(ump.wordCount == 2);
    // MT = 0x4, opcode = 0x1 (assignable per-note controller)
    REQUIRE(((ump.words[0] >> 28) & 0xF) == 0x4);
    REQUIRE(((ump.words[0] >> 20) & 0xF) == 0x1);
    // degree = 3 (E is 3rd degree of C major)
    REQUIRE(((ump.words[1] >> 24) & 0xFF) == 3);
}

TEST_CASE("MIDI2 UMP toBytesBE serializes big-endian", "[midi2]") {
    Chord chord("CM");
    UMP ump = MIDI2::chordName(chord);
    uint8_t buf[16];
    uint8_t written = ump.toBytesBE(buf, sizeof(buf));
    REQUIRE(written == 16);
    // First nibble of first byte = 0xD (Flex Data MT)
    REQUIRE((buf[0] >> 4) == 0xD);
}

TEST_CASE("MIDI2 dispatch MT=0x2 note on", "[midi2]") {
    Monitor mon;
    // Build a MIDI 1.0-in-UMP note on: MT=0x2, opcode=0x9, note=60, vel=100
    uint32_t words[2];
    words[0] = ((uint32_t)0x2 << 28) | ((uint32_t)0x9 << 20) | (60 << 8) | 100;
    words[1] = 0;
    bool handled = MIDI2::dispatch(words, mon);
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 1);
}

TEST_CASE("MIDI2 dispatch MT=0x4 note on", "[midi2]") {
    Monitor mon;
    // MT=0x4, opcode=0x9, note=60, vel16>0
    uint32_t words[2];
    words[0] = ((uint32_t)0x4 << 28) | ((uint32_t)0x9 << 20) | (60 << 8);
    words[1] = (uint32_t)0x8000 << 16;  // vel16 = 0x8000 → vel7 = 64
    bool handled = MIDI2::dispatch(words, mon);
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 1);
}

TEST_CASE("MIDI2 dispatch MT=0x4 note off", "[midi2]") {
    Monitor mon;
    uint32_t on[2];
    on[0] = ((uint32_t)0x4 << 28) | ((uint32_t)0x9 << 20) | (60 << 8);
    on[1] = (uint32_t)0x8000 << 16;
    MIDI2::dispatch(on, mon);

    uint32_t off[2];
    off[0] = ((uint32_t)0x4 << 28) | ((uint32_t)0x8 << 20) | (60 << 8);
    off[1] = 0;
    bool handled = MIDI2::dispatch(off, mon);
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 0);
}

// ===========================================================================
// Monitor channel filter
// ===========================================================================

TEST_CASE("Monitor default channel is omni (0xFF)", "[monitor]") {
    Monitor mon;
    REQUIRE(mon.channel() == 0xFF);
    REQUIRE(mon.acceptsChannel(0));
    REQUIRE(mon.acceptsChannel(15));
}

TEST_CASE("Monitor acceptsChannel specific channel", "[monitor]") {
    Monitor mon;
    mon.setChannel(3);
    REQUIRE(!mon.acceptsChannel(0));
    REQUIRE(!mon.acceptsChannel(2));
    REQUIRE( mon.acceptsChannel(3));
    REQUIRE(!mon.acceptsChannel(4));
}

TEST_CASE("MIDI1 dispatch filters by Monitor channel", "[midi1]") {
    Monitor mon;
    mon.setChannel(1);  // channel 1 only (0-indexed)

    // Channel 0 → discarded
    bool handled = MIDI1::dispatch(0x90, 60, 100, mon);
    REQUIRE(!handled);
    REQUIRE(mon.activeNoteCount() == 0);

    // Channel 1 → accepted
    handled = MIDI1::dispatch(0x91, 60, 100, mon);
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 1);
}

TEST_CASE("MIDI1 dispatch omni mode accepts all channels", "[midi1]") {
    Monitor mon;  // omni by default

    // Notes from three different channels — all accepted
    MIDI1::dispatch(0x91, 60, 100, mon);  // ch 1 — C
    MIDI1::dispatch(0x92, 64, 100, mon);  // ch 2 — E
    MIDI1::dispatch(0x93, 67, 100, mon);  // ch 3 — G
    REQUIRE(mon.activeNoteCount() == 3);
    REQUIRE(mon.hasChord());
    REQUIRE(mon.currentChord().name() == "CM");
}

TEST_CASE("MIDI1 channel filter builds chord from single channel", "[midi1]") {
    Monitor mon;
    mon.setChannel(1);

    // Ch 0 notes (wrong channel) — ignored
    MIDI1::dispatch(0x90, 60, 100, mon);
    MIDI1::dispatch(0x90, 64, 100, mon);
    MIDI1::dispatch(0x90, 67, 100, mon);
    REQUIRE(!mon.hasChord());

    // Ch 1 notes (correct channel) — accepted
    MIDI1::dispatch(0x91, 60, 100, mon);
    MIDI1::dispatch(0x91, 64, 100, mon);
    MIDI1::dispatch(0x91, 67, 100, mon);
    REQUIRE(mon.hasChord());
    REQUIRE(mon.currentChord().name() == "CM");
}

TEST_CASE("MIDI2 dispatch MT=0x2 filters by channel", "[midi2]") {
    Monitor mon;
    mon.setChannel(2);

    // Channel 0: MT=0x2, opcode=NoteOn, ch=0, note=60, vel=100
    uint32_t w0[2];
    w0[0] = ((uint32_t)0x2 << 28) | ((uint32_t)0x9 << 20) | ((uint32_t)0 << 16)
          | (60 << 8) | 100;
    w0[1] = 0;
    bool handled = MIDI2::dispatch(w0, mon);
    REQUIRE(!handled);
    REQUIRE(mon.activeNoteCount() == 0);

    // Channel 2: MT=0x2, opcode=NoteOn, ch=2, note=60, vel=100
    uint32_t w2[2];
    w2[0] = ((uint32_t)0x2 << 28) | ((uint32_t)0x9 << 20) | ((uint32_t)2 << 16)
          | (60 << 8) | 100;
    w2[1] = 0;
    handled = MIDI2::dispatch(w2, mon);
    REQUIRE(handled);
    REQUIRE(mon.activeNoteCount() == 1);
}

// ===========================================================================
// MIDICI — SysEx generators
// ===========================================================================

TEST_CASE("MIDICI discoveryRequest writes correct header", "[midici]") {
    uint8_t buf[64];
    uint8_t len = MIDICI::discoveryRequest(buf, sizeof(buf));
    REQUIRE(len == 31);
    REQUIRE(buf[0]  == 0xF0);  // SysEx start
    REQUIRE(buf[1]  == 0x7E);  // Universal SysEx
    REQUIRE(buf[3]  == 0x0D);  // MIDI-CI
    REQUIRE(buf[4]  == 0x70);  // Discovery Request
    REQUIRE(buf[30] == 0xF7);  // SysEx end
}

TEST_CASE("MIDICI profileInquiryReply writes correct header", "[midici]") {
    uint8_t buf[64];
    uint8_t len = MIDICI::profileInquiryReply(buf, sizeof(buf));
    REQUIRE(len == 23);
    REQUIRE(buf[0] == 0xF0);
    REQUIRE(buf[4] == 0x22);  // Profile Inquiry Reply
    REQUIRE(buf[len - 1] == 0xF7);
    // Profile ID: buf[16..20] (after 2x MUID + channel + num_enabled)
    REQUIRE(buf[16] == MIDICI::PROFILE_ID[0]);
    REQUIRE(buf[17] == MIDICI::PROFILE_ID[1]);
}

TEST_CASE("MIDICI discoveryRequest buffer too small returns 0", "[midici]") {
    uint8_t buf[10];
    uint8_t len = MIDICI::discoveryRequest(buf, sizeof(buf));
    REQUIRE(len == 0);
}

TEST_CASE("MIDICI capabilitiesJSON writes non-empty string", "[midici]") {
    char buf[255];
    uint8_t len = MIDICI::capabilitiesJSON(buf, static_cast<uint8_t>(sizeof(buf)));
    REQUIRE(len > 0);
    REQUIRE(buf[0] == '{');
    REQUIRE(buf[len - 1] == '}');
}
