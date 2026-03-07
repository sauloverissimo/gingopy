// Gingo — Music Theory Library
// MIDICIHandler implementation.
//
// SPDX-License-Identifier: MIT

#include "../include/gingo/midici_handler.hpp"
#include "../include/gingo/midi2.hpp"

#include <cstring>

namespace gingo {

// ===========================================================================
// Constructor
// ===========================================================================

MIDICIHandler::MIDICIHandler() : replyLen_(0) {
    std::memcpy(muid_, MIDICI::DEFAULT_MUID, 4);
    std::memset(reply_, 0, MAX_REPLY);
}

void MIDICIHandler::setMUID(const uint8_t muid[4]) {
    std::memcpy(muid_, muid, 4);
}

// ===========================================================================
// receive — parse incoming MIDI-CI SysEx
// ===========================================================================

bool MIDICIHandler::receive(const uint8_t* data, uint8_t len) {
    replyLen_ = 0;

    // Minimum MIDI-CI SysEx: F0 7E <ch> 0D <subId> 02 <srcMUID[4]> <dstMUID[4]> ... F7
    // = at least 16 bytes
    if (len < 16) return false;
    if (data[0] != midici::SYSEX_START) return false;
    if (data[1] != midici::UNIVERSAL_SYSEX) return false;
    if (data[3] != midici::MIDI_CI_ID) return false;

    uint8_t channel = data[2];
    uint8_t subId   = data[4];
    // uint8_t version = data[5];  // CI version — currently always 0x02

    uint8_t srcMUID[4];
    extractMUID_(data, 6, srcMUID);

    uint8_t dstMUID[4];
    extractMUID_(data, 10, dstMUID);

    // Check destination: must be broadcast or our MUID
    bool isBroadcast = (dstMUID[0] == 0x7F && dstMUID[1] == 0x7F &&
                        dstMUID[2] == 0x7F && dstMUID[3] == 0x7F);
    bool isForUs = (std::memcmp(dstMUID, muid_, 4) == 0);
    if (!isBroadcast && !isForUs) return false;

    switch (subId) {
        case midici::DISCOVERY_REQUEST:
            onDiscovery(srcMUID, channel);
            return true;

        case midici::PROFILE_INQUIRY:
            onProfileInquiry(srcMUID, channel);
            return true;

        case midici::PROPERTY_GET:
            onPropertyGet(srcMUID, channel);
            return true;

        default:
            return false;
    }
}

// ===========================================================================
// Default virtual handlers
// ===========================================================================

void MIDICIHandler::onDiscovery(const uint8_t srcMUID[4], uint8_t channel) {
    // Discovery Reply (sub-ID 0x71)
    // Format: F0 7E <ch> 0D 71 02 <src[4]> <dst[4]> <mfr[3]> <fam[2]> <model[2]>
    //         <ver[4]> <cicat> <maxsysex[4]> F7
    beginReply_(channel, midici::DISCOVERY_REPLY, srcMUID);

    // Manufacturer ID (non-commercial)
    uint8_t mfr[] = { 0x7D, 0x00, 0x00 };
    appendReply_(mfr, 3);

    // Device family + model (gingo = 0x0001, 0x0001)
    uint8_t fam[] = { 0x01, 0x00, 0x01, 0x00 };
    appendReply_(fam, 4);

    // Version (1.2.0.0)
    uint8_t ver[] = { 0x00, 0x02, 0x01, 0x00 };
    appendReply_(ver, 4);

    // MIDI-CI category: profiles + property exchange + process inquiry
    appendReply_(0x0E);

    // Max SysEx size: 256 bytes (0x00 0x02 0x00 0x00)
    uint8_t maxSysex[] = { 0x00, 0x02, 0x00, 0x00 };
    appendReply_(maxSysex, 4);

    endReply_();
}

void MIDICIHandler::onProfileInquiry(const uint8_t srcMUID[4], uint8_t channel) {
    beginReply_(channel, midici::PROFILE_INQUIRY_REPLY, srcMUID);

    // Channel
    appendReply_(channel & 0x7F);

    // Number of enabled profiles: 1
    appendReply_(0x01);

    // Profile ID: [0x7D, 0x47, 0x49, 0x4E, 0x47] = non-commercial + "GING"
    appendReply_(MIDICI::PROFILE_ID, 5);

    // Number of disabled profiles: 0
    appendReply_(0x00);

    endReply_();
}

void MIDICIHandler::onPropertyGet(const uint8_t srcMUID[4], uint8_t channel) {
    // Property Get Reply (sub-ID 0x35)
    // Simplified: return capabilities JSON as the property value
    beginReply_(channel, midici::PROPERTY_GET_REPLY, srcMUID);

    // Request ID (echo back 0x01 — simplified)
    appendReply_(0x01);

    // Header length (2 bytes, little-endian) — empty header
    appendReply_(0x00);
    appendReply_(0x00);

    // Number of chunks / this chunk (simplified: 1 of 1)
    appendReply_(0x01);
    appendReply_(0x01);

    // Property data: capabilities JSON
    char json[200];
    uint8_t jsonLen = MIDICI::capabilitiesJSON(json, sizeof(json));

    // Data length (2 bytes, little-endian)
    appendReply_(jsonLen & 0x7F);
    appendReply_((jsonLen >> 7) & 0x7F);

    // JSON payload (7-bit safe — all printable ASCII)
    for (uint8_t i = 0; i < jsonLen; i++) {
        appendReply_(static_cast<uint8_t>(json[i]));
    }

    endReply_();
}

// ===========================================================================
// Reply helpers
// ===========================================================================

void MIDICIHandler::beginReply_(uint8_t channel, uint8_t subId,
                                 const uint8_t destMUID[4]) {
    replyLen_ = 0;
    appendReply_(midici::SYSEX_START);
    appendReply_(midici::UNIVERSAL_SYSEX);
    appendReply_(channel & 0x7F);
    appendReply_(midici::MIDI_CI_ID);
    appendReply_(subId);
    appendReply_(midici::CI_VERSION);

    // Source MUID (ours)
    for (int i = 0; i < 4; i++) appendReply_(muid_[i] & 0x7F);

    // Destination MUID
    for (int i = 0; i < 4; i++) appendReply_(destMUID[i] & 0x7F);
}

void MIDICIHandler::appendReply_(const uint8_t* data, uint8_t len) {
    for (uint8_t i = 0; i < len && replyLen_ < MAX_REPLY; i++) {
        reply_[replyLen_++] = data[i];
    }
}

void MIDICIHandler::appendReply_(uint8_t b) {
    if (replyLen_ < MAX_REPLY) {
        reply_[replyLen_++] = b;
    }
}

void MIDICIHandler::endReply_() {
    appendReply_(midici::SYSEX_END);
}

// ===========================================================================
// Static helpers
// ===========================================================================

void MIDICIHandler::extractMUID_(const uint8_t* buf, uint8_t offset,
                                  uint8_t out[4]) {
    out[0] = buf[offset]     & 0x7F;
    out[1] = buf[offset + 1] & 0x7F;
    out[2] = buf[offset + 2] & 0x7F;
    out[3] = buf[offset + 3] & 0x7F;
}

} // namespace gingo
