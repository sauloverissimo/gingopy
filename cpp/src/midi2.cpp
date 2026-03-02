// Gingo — Music Theory Library
// MIDI2 implementation.
//
// SPDX-License-Identifier: MIT

#include "../include/gingo/midi2.hpp"
#include "../include/gingo/monitor.hpp"

#include <cstring>

namespace gingo {

// ===========================================================================
// MIDI2 — UMP Flex Data factory
// ===========================================================================

UMP MIDI2::chordName(const Chord& chord,
                     uint8_t group, uint8_t channel) {
    UMP ump;
    ump.wordCount = 4;
    ump.words[0] = makeWord0_(0x00, 0x06, group, channel);

    uint8_t tonicLetter, tonicAcc;
    noteToMIDI2Tonic_(chord.root(), tonicLetter, tonicAcc);

    uint8_t chordType = chordTypeForName_(chord.type());

    ump.words[1] = ((uint32_t)tonicAcc   << 28)
                 | ((uint32_t)tonicLetter << 24)
                 | ((uint32_t)chordType   << 16);
    ump.words[2] = 0;
    ump.words[3] = 0;
    return ump;
}

UMP MIDI2::keySignature(const Scale& scale,
                        uint8_t group, uint8_t channel) {
    UMP ump;
    ump.wordCount = 4;
    ump.words[0] = makeWord0_(0x00, 0x05, group, channel);

    uint8_t tonicLetter, tonicAcc;
    noteToMIDI2Tonic_(scale.tonic(), tonicLetter, tonicAcc);

    uint8_t scaleMode = scaleTypeToMIDI2Mode_(scale.type());

    ump.words[1] = ((uint32_t)tonicAcc    << 28)
                 | ((uint32_t)tonicLetter  << 24)
                 | ((uint32_t)scaleMode    << 16);
    ump.words[2] = 0;
    ump.words[3] = 0;
    return ump;
}

UMP MIDI2::perNoteController(uint8_t midiNoteNum,
                             const NoteContext& ctx,
                             uint8_t group, uint8_t channel) {
    UMP ump;
    ump.wordCount = 2;

    ump.words[0] = ((uint32_t)0x4 << 28)
                 | ((uint32_t)(group   & 0xF) << 24)
                 | ((uint32_t)0x1              << 20)  // opcode: assignable per-note ctrl
                 | ((uint32_t)(channel & 0xF) << 16)
                 | ((uint32_t)midiNoteNum       <<  8)
                 | ((uint32_t)0x00);  // controller index 0 = gingo degree/function

    ump.words[1] = ((uint32_t)ctx.degree              << 24)
                 | ((uint32_t)(static_cast<uint8_t>(ctx.function) & 0xFF)   << 16)
                 | ((uint32_t)ctx.interval.semitones() <<  8)
                 | ((uint32_t)(ctx.inScale ? 1 : 0));
    ump.words[2] = 0;
    ump.words[3] = 0;
    return ump;
}

bool MIDI2::dispatch(const uint32_t* words, Monitor& mon) {
    uint8_t mt = (uint8_t)((words[0] >> 28) & 0xF);
    uint8_t ch = (uint8_t)((words[0] >> 16) & 0xF);
    if (!mon.acceptsChannel(ch)) return false;

    // MIDI 1.0 Channel Voice (MT=0x2, 1 word)
    if (mt == 0x2) {
        uint8_t opcode = (uint8_t)((words[0] >> 20) & 0xF);
        uint8_t data1  = (uint8_t)((words[0] >>  8) & 0x7F);
        uint8_t data2  = (uint8_t)(words[0] & 0x7F);

        if (opcode == 0x9 && data2 > 0) { mon.noteOn(data1, data2);  return true; }
        if (opcode == 0x8 || (opcode == 0x9 && data2 == 0)) { mon.noteOff(data1); return true; }
        if (opcode == 0xB) {
            if (data1 == 64) { (data2 >= 64) ? mon.sustainOn() : mon.sustainOff(); return true; }
            if (data1 == 123) { mon.reset(); return true; }
        }
        return false;
    }

    // MIDI 2.0 Channel Voice (MT=0x4, 2 words)
    if (mt == 0x4) {
        uint8_t opcode = (uint8_t)((words[0] >> 20) & 0xF);
        uint8_t index  = (uint8_t)((words[0] >>  8) & 0x7F);

        if (opcode == 0x9 || opcode == 0x8) {
            uint16_t vel16 = (uint16_t)((words[1] >> 16) & 0xFFFF);
            uint8_t  vel7  = (uint8_t)(vel16 >> 9);
            if (opcode == 0x9 && vel16 > 0) { mon.noteOn(index, vel7);  return true; }
            mon.noteOff(index); return true;
        }
        if (opcode == 0xB) {
            uint32_t val = words[1];
            if (index == 64) { (val >= 0x80000000U) ? mon.sustainOn() : mon.sustainOff(); return true; }
            if (index == 123) { mon.reset(); return true; }
        }
        return false;
    }

    return false;
}

// ===========================================================================
// Internal helpers
// ===========================================================================

void MIDI2::noteToMIDI2Tonic_(const Note& note,
                               uint8_t& letter, uint8_t& acc) {
    const std::string& name = note.name();
    char first = name[0];
    switch (first) {
        case 'A': letter = 1; break;
        case 'B': letter = 2; break;
        case 'C': letter = 3; break;
        case 'D': letter = 4; break;
        case 'E': letter = 5; break;
        case 'F': letter = 6; break;
        case 'G': letter = 7; break;
        default:  letter = 0; break;
    }
    if (name.length() >= 2) {
        if (name[1] == '#' && name.length() >= 3 && name[2] == '#') acc = 2;
        else if (name[1] == '#') acc = 1;
        else if (name[1] == 'b' && name.length() >= 3 && name[2] == 'b') acc = 0xE;
        else if (name[1] == 'b') acc = 0xF;
        else acc = 0;
    } else {
        acc = 0;
    }
}

uint8_t MIDI2::chordTypeForName_(const std::string& type) {
    if (type.empty()) return 1;  // empty = Major

    struct Entry { const char* name; uint8_t midi2Type; };
    static const Entry TABLE[] = {
        { "M",        1  },  // Major triad
        { "6",        2  },  // Major 6th
        { "7M",       3  },  // Major 7th (maj7)
        { "M9",       4  },  // Major 9th
        { "maj13",    6  },  // Major 13th
        { "m",        7  },  // Minor triad
        { "m6",       8  },  // Minor 6th
        { "m7",       9  },  // Minor 7th
        { "m9",       10 },  // Minor 9th
        { "m11",      11 },  // Minor 11th
        { "m13",      12 },  // Minor 13th
        { "7",        13 },  // Dominant 7th
        { "9",        14 },  // Dominant 9th
        { "11",       15 },  // Dominant 11th
        { "13",       16 },  // Dominant 13th
        { "aug",      17 },  // Augmented triad
        { "7#5",      18 },  // Augmented 7th
        { "7+5",      18 },  // Augmented 7th (alt notation)
        { "M7#5",     18 },  // Major 7th augmented
        { "dim",      19 },  // Diminished triad
        { "dim7",     20 },  // Diminished 7th
        { "m7(b5)",   21 },  // Half-diminished (m7♭5)
        { "mM7",      22 },  // Minor-Major 7th
        { "5",        24 },  // Power chord (no third)
        { "sus2",     25 },  // Suspended 2nd
        { "sus4",     26 },  // Suspended 4th
        { "sus",      26 },  // Suspended 4th (alt notation)
        { "sus7",     27 },  // 7th suspended 4th
    };

    static const uint8_t TABLE_SIZE = static_cast<uint8_t>(sizeof(TABLE) / sizeof(TABLE[0]));

    for (uint8_t i = 0; i < TABLE_SIZE; i++) {
        if (type == TABLE[i].name) return TABLE[i].midi2Type;
    }
    return 0;  // Unknown
}

uint8_t MIDI2::scaleTypeToMIDI2Mode_(ScaleType st) {
    switch (st) {
        case ScaleType::Major:          return 0;
        case ScaleType::NaturalMinor:  return 1;
        case ScaleType::HarmonicMinor: return 2;
        case ScaleType::MelodicMinor:  return 3;
        default:                        return 0;
    }
}

} // namespace gingo
