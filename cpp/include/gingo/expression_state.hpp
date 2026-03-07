// Gingo — Music Theory Library
// ExpressionState: per-note and per-channel performance expression data.
//
// Captures MIDI expression state that lives alongside harmonic analysis:
// velocity (7/16-bit), pitch bend, aftertouch, and control changes.
//
// Design rationale:
//   Harmonic analysis (chord detection, field deduction) is pitch-class
//   based and independent of expression. ExpressionState tracks the
//   performance layer so that consumers (visualizers, MIDI 2.0 Flex Data
//   generators, DAW bridges) have access to the full picture without
//   polluting the harmonic core.
//
// All values use MIDI 2.0 native resolution internally. MIDI 1.0 inputs
// are scaled up on ingress so that no precision is lost.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

namespace gingo {

// ---------------------------------------------------------------------------
// Resolution scaling — MIDI 1.0 ↔ MIDI 2.0
// ---------------------------------------------------------------------------

namespace midi {

/// Scale 7-bit MIDI 1.0 value (0–127) to 32-bit MIDI 2.0 range.
/// Uses the bit-replication method from the MIDI 2.0 spec (M2-104-UM §4.4).
constexpr uint32_t scale7to32(uint8_t v7) {
    uint32_t v = v7;
    return (v << 25) | (v << 18) | (v << 11) | (v << 4) | (v >> 3);
}

/// Scale 16-bit MIDI 2.0 velocity to 32-bit range.
constexpr uint32_t scale16to32(uint16_t v16) {
    uint32_t v = v16;
    return (v << 16) | v;
}

/// Scale 14-bit MIDI 1.0 pitch bend to signed 32-bit MIDI 2.0 range.
/// Input: 0–16383 (center = 8192). Output: INT32_MIN–INT32_MAX (center = 0).
constexpr int32_t scale14to32(uint16_t pb14) {
    int32_t centered = static_cast<int32_t>(pb14) - 8192;
    return centered << 18;
}

/// Compress 32-bit value to 7-bit (for backward compat).
constexpr uint8_t scale32to7(uint32_t v32) {
    return static_cast<uint8_t>(v32 >> 25);
}

/// Compress 32-bit velocity to 16-bit.
constexpr uint16_t scale32to16(uint32_t v32) {
    return static_cast<uint16_t>(v32 >> 16);
}

} // namespace midi

// ---------------------------------------------------------------------------
// NoteExpression — per-note expression snapshot
// ---------------------------------------------------------------------------

/// Per-note expression state at MIDI 2.0 resolution.
/// Returned by Monitor::noteExpression() for consumers that need
/// the full performance picture alongside harmonic context.
struct NoteExpression {
    uint32_t velocity;     ///< Note-on velocity (32-bit, MIDI 2.0 scale)
    int32_t  pitchBend;    ///< Per-note pitch bend (signed 32-bit, 0 = center)
    uint32_t pressure;     ///< Per-note aftertouch / poly pressure (32-bit)

    NoteExpression()
        : velocity(0), pitchBend(0), pressure(0) {}

    /// Velocity as 7-bit (MIDI 1.0 compat).
    uint8_t velocity7() const { return midi::scale32to7(velocity); }

    /// Velocity as 16-bit (MIDI 2.0 Note On).
    uint16_t velocity16() const { return midi::scale32to16(velocity); }
};

// ---------------------------------------------------------------------------
// ChannelExpression — per-channel expression state
// ---------------------------------------------------------------------------

/// Per-channel expression state at MIDI 2.0 resolution.
/// Tracks pitch bend, channel pressure, and the 128 CCs.
struct ChannelExpression {
    int32_t  pitchBend;    ///< Channel pitch bend (signed 32-bit, 0 = center)
    uint32_t pressure;     ///< Channel pressure / aftertouch (32-bit)

    ChannelExpression() : pitchBend(0), pressure(0) {}

    /// Access a CC value. Returns 0 for unset CCs.
    uint32_t cc(uint8_t index) const {
        auto it = cc_.find(index);
        return it != cc_.end() ? it->second : 0;
    }

    /// Set a CC value (32-bit MIDI 2.0 resolution).
    void setCC(uint8_t index, uint32_t value) {
        cc_[index] = value;
    }

    /// Reset all expression state to defaults.
    void reset() {
        pitchBend = 0;
        pressure  = 0;
        cc_.clear();
    }

private:
    std::unordered_map<uint8_t, uint32_t> cc_;
};

} // namespace gingo
