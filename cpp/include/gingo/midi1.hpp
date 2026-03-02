// Gingo — Music Theory Library
// MIDI1: raw MIDI 1.0 byte stream dispatcher and parser.
//
// Two components:
//
//   MIDI1 (static class)
//     Stateless dispatcher for pre-parsed MIDI 1.0 messages.
//     Mirrors MIDI2::dispatch() for the raw-byte world.
//     Use when your transport already splits bytes into (status, data1, data2),
//     e.g. Arduino MIDI Library callbacks.
//
//       void handleNoteOn(byte ch, byte note, byte vel) {
//           MIDI1::dispatch(0x90 | (ch - 1), note, vel, mon);
//       }
//       void handleNoteOff(byte ch, byte note, byte vel) {
//           MIDI1::dispatch(0x80 | (ch - 1), note, vel, mon);
//       }
//       void handleCC(byte ch, byte cc, byte val) {
//           MIDI1::dispatch(0xB0 | (ch - 1), cc, val, mon);
//       }
//
//   MIDI1Parser (struct)
//     Stateful parser for raw byte streams (DIN MIDI, BLE MIDI, USB MIDI 1.0).
//     Handles running status, SysEx absorption, and real-time bytes (0xF8-0xFF).
//     Feed one byte at a time via feed(); call reset() to restart cleanly.
//
//       // DIN MIDI at 31250 baud (UART ISR or polling loop):
//       MIDI1Parser parser;
//       while (Serial2.available()) {
//           parser.feed(Serial2.read(), mon);
//       }
//
//       // BLE MIDI (strip the 2-byte BLE header, feed payload bytes):
//       parser.feed(bleByte, mon);
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <functional>

namespace gingo {

// Forward declaration — Monitor is defined in monitor.hpp
class Monitor;

// ===========================================================================
// MIDI1 — stateless dispatcher for pre-parsed MIDI 1.0 messages
// ===========================================================================

/// Stateless MIDI 1.0 dispatcher.
///
/// Accepts pre-parsed (status, data1, data2) tuples and routes them to a
/// Monitor. The channel nibble is extracted and checked against
/// Monitor::acceptsChannel() — events from filtered channels are silently
/// discarded (returns false). In omni mode (default) all channels pass.
///
/// Handled messages:
///   • 0x9n Note On  — vel > 0 → noteOn, vel == 0 → noteOff (running-status trick)
///   • 0x8n Note Off — noteOff
///   • 0xBn CC 64    — sustain pedal (val >= 64 → on, val < 64 → off)
///   • 0xBn CC 123   — All Notes Off → reset()
class MIDI1 {
public:
    /// Dispatch a pre-parsed MIDI 1.0 message to a Monitor.
    ///
    /// @param status  Status byte (e.g. 0x90, 0x80, 0xB0). Channel nibble ignored.
    /// @param data1   First data byte (note number or CC number).
    /// @param data2   Second data byte (velocity or CC value).
    /// @param mon     Monitor to receive the routed event.
    /// @returns       true if the message was handled.
    static bool dispatch(uint8_t status, uint8_t data1, uint8_t data2,
                         Monitor& mon);
};

// ===========================================================================
// MIDI1Parser — stateful raw byte stream parser
// ===========================================================================

/// Stateful MIDI 1.0 byte stream parser.
///
/// Handles the full MIDI 1.0 serial protocol including:
///   • Running status — status byte reused until a new one arrives
///   • SysEx (0xF0 … 0xF7) — absorbed silently, parser state preserved
///   • Real-time bytes (0xF8-0xFF) — ignored without disrupting parser state
///
/// All state fits in 4 bytes — safe for stack allocation on any platform.
///
/// Usage:
///   MIDI1Parser parser;
///   // in loop or ISR:
///   while (Serial.available()) parser.feed(Serial.read(), mon);
struct MIDI1Parser {
    uint8_t status_;   ///< Current running status byte (0 = none)
    uint8_t data1_;    ///< First data byte accumulated
    uint8_t count_;    ///< Number of data bytes received for current message
    uint8_t inSysex_;  ///< 1 while absorbing SysEx bytes

    MIDI1Parser() : status_(0), data1_(0), count_(0), inSysex_(0) {}

    /// Reset parser to initial state (e.g. after a MIDI port reconnection).
    void reset() {
        status_ = 0;
        data1_  = 0;
        count_  = 0;
        inSysex_= 0;
    }

    /// Feed one byte from the raw MIDI stream.
    ///
    /// Internally accumulates bytes and calls MIDI1::dispatch() when a
    /// complete message is ready.
    ///
    /// @param b    Raw byte from the MIDI serial stream.
    /// @param mon  Monitor to receive routed events.
    /// @returns    true if feeding this byte completed a handled message.
    bool feed(uint8_t b, Monitor& mon);

private:
    /// Expected number of data bytes for a given status byte.
    /// Returns 1 for single-data messages, 2 for two-data messages.
    static uint8_t dataLength_(uint8_t status);
};

} // namespace gingo
