// Gingo — Music Theory Library
// MIDI1 implementation.
//
// SPDX-License-Identifier: MIT

#include "../include/gingo/midi1.hpp"
#include "../include/gingo/monitor.hpp"

#include <cstdint>

namespace gingo {

// ===========================================================================
// MIDI1 — stateless dispatcher
// ===========================================================================

bool MIDI1::dispatch(uint8_t status, uint8_t data1, uint8_t data2,
                     Monitor& mon) {
    uint8_t ch   = status & 0x0F;
    if (!mon.acceptsChannel(ch)) return false;

    uint8_t type = status & 0xF0;

    // Note On — vel=0 treated as Note Off (running-status convention)
    if (type == 0x90) {
        if (data2 > 0) { mon.noteOn(data1, data2); return true; }
        mon.noteOff(data1); return true;
    }

    // Note Off
    if (type == 0x80) {
        mon.noteOff(data1); return true;
    }

    // Control Change
    if (type == 0xB0) {
        if (data1 == 64) {
            (data2 >= 64) ? mon.sustainOn() : mon.sustainOff();
            return true;
        }
        if (data1 == 123) { mon.reset(); return true; }  // All Notes Off
    }

    return false;  // unhandled message type
}

// ===========================================================================
// MIDI1Parser — stateful parser
// ===========================================================================

uint8_t MIDI1Parser::dataLength_(uint8_t status) {
    switch (status & 0xF0) {
        case 0x80:  // Note Off
        case 0x90:  // Note On
        case 0xA0:  // Aftertouch (polyphonic)
        case 0xB0:  // Control Change
        case 0xE0:  // Pitch Bend
            return 2;
        case 0xC0:  // Program Change
        case 0xD0:  // Channel Pressure
            return 1;
        default:
            return 2;
    }
}

bool MIDI1Parser::feed(uint8_t b, Monitor& mon) {
    // Real-time bytes (0xF8-0xFF): ignore, no state change
    if (b >= 0xF8) return false;

    // SysEx end
    if (b == 0xF7) { inSysex_ = 0; return false; }

    // SysEx start
    if (b == 0xF0) { inSysex_ = 1; count_ = 0; return false; }

    // Inside SysEx: absorb until 0xF7
    if (inSysex_) return false;

    // System Common (0xF1-0xF6): clear running status, skip
    if (b >= 0xF1) { status_ = 0; count_ = 0; return false; }

    // Status byte (bit 7 set): start new message
    if (b & 0x80) {
        status_ = b;
        count_  = 0;
        return false;
    }

    // Data byte: accumulate
    if (!status_) return false;  // no running status yet — discard

    if (count_ == 0) {
        // First data byte
        uint8_t expected = dataLength_(status_);
        if (expected == 1) {
            // Single-data-byte message (e.g. Program Change, Channel Pressure)
            return MIDI1::dispatch(status_, b, 0, mon);
        }
        data1_  = b;
        count_  = 1;
        return false;
    }

    // Second (and final) data byte — dispatch
    bool handled = MIDI1::dispatch(status_, data1_, b, mon);
    count_ = 0;  // reset for running status: next data byte starts fresh
    return handled;
}

} // namespace gingo
