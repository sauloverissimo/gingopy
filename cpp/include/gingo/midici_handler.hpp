// Gingo — Music Theory Library
// MIDICIHandler: bidirectional MIDI-CI message handler.
//
// Parses incoming MIDI-CI SysEx and dispatches to virtual methods.
// Default implementations reply with gingo's profile and capabilities.
//
// MIDI-CI message flow:
//   Initiator                    Responder (gingo)
//   ─────────                    ─────────────────
//   Discovery Request  ──────►   onDiscovery()  → discoveryReply()
//   Profile Inquiry   ──────►   onProfileInquiry() → profileInquiryReply()
//   Property Get      ──────►   onPropertyGet() → capabilitiesJSON
//
// Usage:
//   MIDICIHandler handler;
//   // In your SysEx receive callback:
//   handler.receive(sysexBytes, length);
//   // Check if a reply was generated:
//   if (handler.hasReply()) {
//       sendSysEx(handler.reply(), handler.replyLength());
//       handler.clearReply();
//   }
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <cstring>

namespace gingo {

// ===========================================================================
// MIDI-CI sub-IDs (from MIDI-CI v2 specification)
// ===========================================================================

namespace midici {

constexpr uint8_t DISCOVERY_REQUEST     = 0x70;
constexpr uint8_t DISCOVERY_REPLY       = 0x71;
constexpr uint8_t PROFILE_INQUIRY       = 0x21;
constexpr uint8_t PROFILE_INQUIRY_REPLY = 0x22;
constexpr uint8_t PROPERTY_GET          = 0x34;
constexpr uint8_t PROPERTY_GET_REPLY    = 0x35;
constexpr uint8_t INVALIDATE_MUID       = 0x7E;

constexpr uint8_t SYSEX_START           = 0xF0;
constexpr uint8_t SYSEX_END             = 0xF7;
constexpr uint8_t UNIVERSAL_SYSEX       = 0x7E;
constexpr uint8_t MIDI_CI_ID            = 0x0D;
constexpr uint8_t CI_VERSION            = 0x02;

} // namespace midici

// ===========================================================================
// MIDICIHandler — bidirectional MIDI-CI responder
// ===========================================================================

/// Bidirectional MIDI-CI handler.
///
/// Parses incoming MIDI-CI SysEx messages and generates appropriate replies.
/// Virtual methods allow subclasses to customize behavior.
///
/// The reply buffer is internal (256 bytes) — no heap allocation needed
/// for standard MIDI-CI exchanges.
class MIDICIHandler {
public:
    static constexpr uint8_t MAX_REPLY = 255;

    MIDICIHandler();
    virtual ~MIDICIHandler() = default;

    /// Set the local MUID (28-bit, stored as 4 x 7-bit bytes).
    void setMUID(const uint8_t muid[4]);

    /// Process an incoming MIDI-CI SysEx message.
    /// @param data  Complete SysEx (including F0 and F7).
    /// @param len   Length in bytes.
    /// @returns     true if the message was recognized and handled.
    bool receive(const uint8_t* data, uint8_t len);

    /// Whether a reply is pending after receive().
    bool hasReply() const { return replyLen_ > 0; }

    /// Pointer to the reply buffer (valid until next receive() or clearReply()).
    const uint8_t* reply() const { return reply_; }

    /// Length of the pending reply.
    uint8_t replyLength() const { return replyLen_; }

    /// Clear the reply buffer.
    void clearReply() { replyLen_ = 0; }

protected:
    // ------------------------------------------------------------------
    // Virtual handlers — override for custom behavior
    // ------------------------------------------------------------------

    /// Called on Discovery Request. Default: generates Discovery Reply.
    /// @param srcMUID  Initiator's MUID (4 bytes).
    /// @param channel  Device ID from the message.
    virtual void onDiscovery(const uint8_t srcMUID[4], uint8_t channel);

    /// Called on Profile Inquiry. Default: replies with gingo profile.
    /// @param srcMUID  Initiator's MUID (4 bytes).
    /// @param channel  Target channel.
    virtual void onProfileInquiry(const uint8_t srcMUID[4], uint8_t channel);

    /// Called on Property Get request. Default: replies with capabilities JSON.
    /// @param srcMUID  Initiator's MUID (4 bytes).
    /// @param channel  Target channel.
    virtual void onPropertyGet(const uint8_t srcMUID[4], uint8_t channel);

    // ------------------------------------------------------------------
    // Reply helpers
    // ------------------------------------------------------------------

    /// Start building a reply SysEx.
    void beginReply_(uint8_t channel, uint8_t subId, const uint8_t destMUID[4]);

    /// Append raw bytes to the reply.
    void appendReply_(const uint8_t* data, uint8_t len);

    /// Append a single byte to the reply.
    void appendReply_(uint8_t b);

    /// Finalize the reply with SysEx end byte.
    void endReply_();

    // Local MUID
    uint8_t muid_[4];

private:
    uint8_t reply_[MAX_REPLY];
    uint8_t replyLen_;

    // Extract MUID from SysEx buffer at offset
    static void extractMUID_(const uint8_t* buf, uint8_t offset, uint8_t out[4]);
};

} // namespace gingo
