// Gingo — Music Theory Library
// MIDI binary format utilities implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/internal/midi_format.hpp>
#include <stdexcept>

namespace gingo::internal {

// ---------------------------------------------------------------------------
// Writing

void write_vlq(std::vector<uint8_t>& out, uint32_t value) {
    // VLQ encoding: 7 bits per byte, MSB first, continuation bit 0x80
    if (value == 0) {
        out.push_back(0);
        return;
    }

    // Collect bytes in reverse order
    uint8_t buf[4];
    int count = 0;
    while (value > 0) {
        buf[count++] = value & 0x7F;
        value >>= 7;
    }

    // Write in MSB-first order with continuation bits
    for (int i = count - 1; i >= 0; --i) {
        uint8_t byte = buf[i];
        if (i > 0) byte |= 0x80;  // set continuation bit
        out.push_back(byte);
    }
}

void write_uint16(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(value & 0xFF));
}

void write_uint32(std::vector<uint8_t>& out, uint32_t value) {
    out.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(value & 0xFF));
}

// ---------------------------------------------------------------------------
// Reading

uint32_t read_vlq(const std::vector<uint8_t>& data, size_t& pos) {
    uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        if (pos >= data.size()) {
            throw std::runtime_error("Unexpected end of MIDI data in VLQ");
        }
        uint8_t byte = data[pos++];
        value = (value << 7) | (byte & 0x7F);
        if (!(byte & 0x80)) break;  // no continuation bit
    }
    return value;
}

uint16_t read_uint16(const std::vector<uint8_t>& data, size_t& pos) {
    if (pos + 2 > data.size()) {
        throw std::runtime_error("Unexpected end of MIDI data reading uint16");
    }
    uint16_t value = (static_cast<uint16_t>(data[pos]) << 8) |
                      static_cast<uint16_t>(data[pos + 1]);
    pos += 2;
    return value;
}

uint32_t read_uint32(const std::vector<uint8_t>& data, size_t& pos) {
    if (pos + 4 > data.size()) {
        throw std::runtime_error("Unexpected end of MIDI data reading uint32");
    }
    uint32_t value = (static_cast<uint32_t>(data[pos]) << 24) |
                     (static_cast<uint32_t>(data[pos + 1]) << 16) |
                     (static_cast<uint32_t>(data[pos + 2]) << 8) |
                      static_cast<uint32_t>(data[pos + 3]);
    pos += 4;
    return value;
}

}  // namespace gingo::internal
