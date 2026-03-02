// Gingo — Music Theory Library
// MIDI2: UMP Flex Data generator + MIDI-CI capability announcement.
//
// Generates Universal MIDI Packet (UMP) Flex Data messages from gingo
// theory objects. Output is bit-for-bit compatible with cmidi2 and
// AM_MIDI2.0Lib because both follow the same MIDI 2.0 spec bit layout.
//
// UMP Flex Data (Message Type 0xD) — Word 0 layout:
//   bits 31-28: MT = 0xD
//   bits 27-24: Group (0-15)
//   bits 23-22: Format = 0b00 (complete in one UMP)
//   bits 21-20: Addressing = 0b01 (channel-addressed)
//   bits 19-16: Channel (0-15)
//   bits 15-8:  Status Bank
//   bits  7-0:  Status
//
// Chord type values match MIDI 2.0 spec and cmidi2 enums exactly:
//   CMIDI2_UMP_CHORD_TYPE_MAJOR=1, MINOR=7, DOMINANT=13, etc.
//
// Reference: MIDI 2.0 UMP spec v1.1.2, cmidi2.h (atsushieno/cmidi2)
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "note.hpp"
#include "chord.hpp"
#include "scale.hpp"
#include "note_context.hpp"

namespace gingo {

// ===========================================================================
// UMP — 128-bit Universal MIDI Packet (4 x 32-bit words)
// ===========================================================================

/// A 128-bit Universal MIDI Packet.
///
/// Stores four 32-bit UMP words in host byte order. Use toBytesBE() /
/// byteCount() to serialize to big-endian bytes for wire transmission.
///
/// Compatible with:
///   • cmidi2: two uint64_t pairs (words[0]<<32|words[1], words[2]<<32|words[3])
///   • AM_MIDI2.0Lib UMP struct: copy words[0..3] into UMP.word0..word3
///   • tusb_ump: tud_midi2_send(ump.words, ump.wordCount)
struct UMP {
    uint32_t words[4];
    uint8_t  wordCount;  ///< Number of valid words (4 for all Flex Data msgs)

    UMP() : wordCount(0) {
        words[0] = words[1] = words[2] = words[3] = 0;
    }

    /// Total byte count (wordCount * 4).
    uint8_t byteCount() const { return wordCount * 4; }

    /// Serialize word[i] as 4 big-endian bytes into dst.
    /// Returns number of bytes written (always 4 per word).
    uint8_t writeWordBE(uint8_t wordIdx, uint8_t* dst) const {
        if (wordIdx >= wordCount) return 0;
        uint32_t w = words[wordIdx];
        dst[0] = (uint8_t)(w >> 24);
        dst[1] = (uint8_t)(w >> 16);
        dst[2] = (uint8_t)(w >>  8);
        dst[3] = (uint8_t)(w      );
        return 4;
    }

    /// Serialize all words as big-endian bytes into buf (must hold byteCount bytes).
    /// Returns total bytes written.
    uint8_t toBytesBE(uint8_t* buf, uint8_t maxLen) const {
        uint8_t written = 0;
        for (uint8_t i = 0; i < wordCount && written + 4 <= maxLen; i++) {
            written += writeWordBE(i, buf + written);
        }
        return written;
    }
};

// ===========================================================================
// MIDI2 — UMP Flex Data factory (static methods, header-only)
// ===========================================================================

/// Generates MIDI 2.0 UMP Flex Data messages from gingo objects.
///
/// All methods are static — no instance required.
///
/// Examples:
///   Chord chord("Am7");
///   UMP ump = MIDI2::chordName(chord);
///   // ump.words[0] = 0xD010_0006 (Flex Data, channel 0, chord name)
///   // ump.words[1] = tonic=A, type=minor7, no bass/alterations
///
///   Scale scale("G", ScaleType::Major);
///   UMP keySig = MIDI2::keySignature(scale);
class MIDI2 {
public:
    // -----------------------------------------------------------------------
    // Flex Data Chord Name (StatusBank=0x00, Status=0x06)
    // Compatible with cmidi2_ump_flex_data_set_chord_name()
    // -----------------------------------------------------------------------

    /// Generate a Flex Data Chord Name UMP message.
    /// @param chord    Source chord (root + type used for tonic and chord type).
    /// @param group    MIDI 2.0 group (0-15, default 0).
    /// @param channel  MIDI 2.0 channel (0-15, default 0).
    static UMP chordName(const Chord& chord,
                        uint8_t group = 0, uint8_t channel = 0);

    // -----------------------------------------------------------------------
    // Flex Data Key Signature (StatusBank=0x00, Status=0x05)
    // -----------------------------------------------------------------------

    /// Generate a Flex Data Key Signature UMP message.
    /// @param scale    Source scale (tonic + key signature used).
    /// @param group    MIDI 2.0 group (0-15, default 0).
    /// @param channel  MIDI 2.0 channel (0-15, default 0).
    static UMP keySignature(const Scale& scale,
                            uint8_t group = 0, uint8_t channel = 0);

    // -----------------------------------------------------------------------
    // Per-note context as Assignable Per-Note Controller (MIDI 2.0 Type 0x4)
    // Encodes scale degree + harmonic function as 32-bit per-note RCC value.
    // -----------------------------------------------------------------------

    /// Generate a Per-Note Assignable Controller UMP for harmonic context.
    /// @param midiNoteNum  MIDI note number being annotated (0-127).
    /// @param ctx          Per-note context from Field::noteContext().
    /// @param group        MIDI 2.0 group (0-15, default 0).
    /// @param channel      MIDI 2.0 channel (0-15, default 0).
    static UMP perNoteController(uint8_t midiNoteNum,
                                const NoteContext& ctx,
                                uint8_t group = 0, uint8_t channel = 0);

    // -----------------------------------------------------------------------
    // UMP input dispatch → Monitor
    // -----------------------------------------------------------------------

    /// Dispatch an incoming UMP packet to a Monitor.
    ///
    /// Handles Note On, Note Off, Sustain Pedal (CC64), and All Notes Off
    /// (CC123) for both MIDI 1.0 (MT=0x2) and MIDI 2.0 (MT=0x4) messages.
    /// All groups and channels are accepted.
    ///
    /// This closes the integration loop with AM_MIDI2.0Lib and cmidi2:
    ///
    ///   // AM_MIDI2.0Lib — feed UMP directly into gingo:
    ///   processor.setChannelVoiceMessage([&mon](UMP& u) {
    ///       MIDI2::dispatch(u.getUMPData(), mon);
    ///   });
    ///
    ///   // cmidi2 / tusb_ump — raw word buffer:
    ///   uint32_t* words = ...; // from UMP stream
    ///   MIDI2::dispatch(words, mon);
    ///
    /// @param words  Pointer to the first word of the UMP packet.
    ///               Must point to at least 2 words when MT=0x4.
    /// @param mon    Monitor to receive the routed events.
    /// @returns      true if the packet was handled.
    static bool dispatch(const uint32_t* words, class Monitor& mon);

private:
    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    /// Build Flex Data Word 0.
    static uint32_t makeWord0_(uint8_t statusBank, uint8_t status,
                                uint8_t group, uint8_t channel) {
        return ((uint32_t)0xD         << 28)   // MT = Flex Data
             | ((uint32_t)(group   & 0xF) << 24)
             | ((uint32_t)0x0       << 22)   // Format = complete in one UMP
             | ((uint32_t)0x1       << 20)   // Addressing = channel
             | ((uint32_t)(channel & 0xF) << 16)
             | ((uint32_t)statusBank <<  8)
             | ((uint32_t)status);
    }

    /// Convert a Note to MIDI 2.0 tonic encoding.
    /// letter: A=1, B=2, C=3, D=4, E=5, F=6, G=7
    /// acc: natural=0, sharp=1, double_sharp=2, flat=0xF, double_flat=0xE
    static void noteToMIDI2Tonic_(const Note& note,
                                  uint8_t& letter, uint8_t& acc);

    /// Map gingo chord type string to MIDI 2.0 chord type value.
    /// Returns 0 (Unknown) for types without a direct MIDI 2.0 mapping.
    /// Values match CMIDI2_UMP_CHORD_TYPE_* in cmidi2.h exactly.
    static uint8_t chordTypeForName_(const std::string& type);

    /// Map gingo ScaleType to MIDI 2.0 mode byte for key signature.
    static uint8_t scaleTypeToMIDI2Mode_(ScaleType st);
};

// ===========================================================================
// MIDICI — MIDI-CI capability announcement (SysEx byte generators)
// ===========================================================================

/// Generates MIDI-CI SysEx message bytes for device discovery and
/// capability announcement. Output is spec-compliant MIDI-CI v2 SysEx.
///
/// MIDI-CI messages are transport-independent — the bytes can be sent
/// via any SysEx-capable transport (USB MIDI, BLE MIDI, DIN MIDI).
///
/// The gingo Profile ID (non-commercial, not registered):
///   [0x7D, 0x47, 0x49, 0x4E, 0x47] = non-commercial + "GING" (ASCII)
///
/// Usage with AM_MIDI2.0Lib:
///   uint8_t buf[64];
///   uint8_t len = MIDICI::profileInquiryReply(buf, sizeof(buf));
///   ump_processor.sendSysEx(buf, len);
namespace MIDICI {

/// gingo Profile ID for MIDI-CI.
/// Non-commercial bank (0x7D) + "GING" ASCII (educational use).
inline constexpr uint8_t PROFILE_ID[5] = { 0x7D, 0x47, 0x49, 0x4E, 0x47 };

/// Default source MUID (28-bit, 7-bit bytes).
/// Users should replace this with a device-specific value.
inline constexpr uint8_t DEFAULT_MUID[4] = { 0x47, 0x49, 0x4E, 0x47 };  // "GING"

/// Broadcast destination MUID (all devices).
inline constexpr uint8_t MUID_BROADCAST[4] = { 0x7F, 0x7F, 0x7F, 0x7F };

/// MIDI-CI SysEx header.
inline constexpr uint8_t MIDI_CI_SYSEX_ID = 0x0D;

/// Write a 28-bit MUID as 4 x 7-bit bytes into dst[0..3].
inline void writeMUID(uint8_t* dst, const uint8_t src[4]) {
    dst[0] = src[0] & 0x7F;
    dst[1] = src[1] & 0x7F;
    dst[2] = src[2] & 0x7F;
    dst[3] = src[3] & 0x7F;
}

/// Generate a MIDI-CI Discovery Request (sub-ID 0x70).
/// Used to discover MIDI-CI capable devices on the network.
/// @param buf     Output buffer.
/// @param maxLen  Buffer capacity.
/// @param srcMUID Source MUID (4 x 7-bit bytes), or nullptr for default.
/// @returns       Number of bytes written, 0 if buffer too small.
inline uint8_t discoveryRequest(uint8_t* buf, uint8_t maxLen,
                                const uint8_t* srcMUID = nullptr) {
    // F0 7E 7F 0D 70 02 <src_muid[4]> <dest_muid[4]> <mfr_id[3]> <fam[2]> <model[2]> <ver[4]> <cicat> <maxsysex[4]> F7
    // = 1+1+1+1+1+1+4+4+3+2+2+4+1+4+1 = 31 bytes
    static const uint8_t MSG_LEN = 31;
    if (maxLen < MSG_LEN) return 0;

    uint8_t i = 0;
    buf[i++] = 0xF0;  // SysEx start
    buf[i++] = 0x7E;  // Universal SysEx
    buf[i++] = 0x7F;  // Device ID: all
    buf[i++] = MIDI_CI_SYSEX_ID;  // MIDI-CI
    buf[i++] = 0x70;  // Sub-ID: Discovery Request
    buf[i++] = 0x02;  // MIDI-CI version 2
    // Source MUID (4 bytes)
    const uint8_t* src = srcMUID ? srcMUID : DEFAULT_MUID;
    writeMUID(buf + i, src); i += 4;
    // Destination MUID: broadcast
    writeMUID(buf + i, MUID_BROADCAST); i += 4;
    // Manufacturer ID (non-commercial)
    buf[i++] = 0x7D; buf[i++] = 0x00; buf[i++] = 0x00;
    // Device family + model (gingo = 0x0001, 0x0001)
    buf[i++] = 0x01; buf[i++] = 0x00;  // family
    buf[i++] = 0x01; buf[i++] = 0x00;  // model
    // Version (1.0.0.0)
    buf[i++] = 0x00; buf[i++] = 0x01; buf[i++] = 0x00; buf[i++] = 0x00;
    // MIDI-CI category support: 0x0E (profiles + property exchange + process inquiry)
    buf[i++] = 0x0E;
    // Max SysEx size: 128 bytes
    buf[i++] = 0x00; buf[i++] = 0x01; buf[i++] = 0x00; buf[i++] = 0x00;
    buf[i++] = 0xF7;  // SysEx end
    return i;
}

/// Generate a MIDI-CI Profile Inquiry Reply (sub-ID 0x22).
/// Announces the gingo profile (chord detection + harmonic analysis).
/// @param buf      Output buffer.
/// @param maxLen   Buffer capacity.
/// @param channel  MIDI channel (0-15, or 0x7F for all channels).
/// @param srcMUID  Source MUID (4 x 7-bit bytes), or nullptr for default.
/// @returns        Number of bytes written, 0 if buffer too small.
inline uint8_t profileInquiryReply(uint8_t* buf, uint8_t maxLen,
                                   uint8_t channel = 0x7F,
                                   const uint8_t* srcMUID = nullptr) {
    // F0 7E <ch> 0D 0x22 02 <src[4]> <dest[4]> <ch> <num_en=1> <profile_id[5]> <num_dis=0> F7
    // = 1+1+1+1+1+1+4+4+1+1+5+1+1 = 23 bytes
    static const uint8_t MSG_LEN = 23;
    if (maxLen < MSG_LEN) return 0;

    uint8_t i = 0;
    buf[i++] = 0xF0;
    buf[i++] = 0x7E;
    buf[i++] = channel & 0x7F;
    buf[i++] = MIDI_CI_SYSEX_ID;
    buf[i++] = 0x22;  // Sub-ID: Profile Inquiry Reply (enabled profiles)
    buf[i++] = 0x02;  // MIDI-CI version 2
    const uint8_t* src = srcMUID ? srcMUID : DEFAULT_MUID;
    writeMUID(buf + i, src); i += 4;
    writeMUID(buf + i, MUID_BROADCAST); i += 4;
    buf[i++] = channel & 0x7F;  // channel
    buf[i++] = 0x01;  // number of enabled profiles
    buf[i++] = PROFILE_ID[0];
    buf[i++] = PROFILE_ID[1];
    buf[i++] = PROFILE_ID[2];
    buf[i++] = PROFILE_ID[3];
    buf[i++] = PROFILE_ID[4];
    buf[i++] = 0x00;  // number of disabled profiles = 0
    buf[i++] = 0xF7;
    return i;
}

/// Generate a JSON capabilities string for MIDI-CI Property Exchange.
/// Compatible with the "ResourceList" property defined in the MIDI-CI spec.
/// @param buf     Output buffer.
/// @param maxLen  Buffer capacity.
/// @returns       Number of bytes written (not null-terminated).
inline uint8_t capabilitiesJSON(char* buf, uint8_t maxLen) {
    static const char JSON[] =
        "{\"name\":\"gingo\","
        "\"version\":\"1.2.0\","
        "\"scales\":[\"major\",\"minor\",\"modes\"],"
        "\"chords\":[\"triad\",\"seventh\",\"ninth\"],"
        "\"features\":[\"chord_detect\",\"key_sig\","
                      "\"harmonic_func\",\"per_note\","
                      "\"field_deduce\"]}";

    uint8_t len = 0;
    const char* src = JSON;
    while (*src && len < maxLen - 1) {
        buf[len++] = *src++;
    }
    buf[len] = '\0';
    return len;
}

} // namespace MIDICI

} // namespace gingo
