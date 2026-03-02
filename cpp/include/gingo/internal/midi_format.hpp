// Gingo — Music Theory Library
// Internal MIDI binary format utilities.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <vector>

namespace gingo::internal {

// ---------------------------------------------------------------------------
// Writing (for MIDI export)

/// Write a variable-length quantity (VLQ) to a byte vector.
void write_vlq(std::vector<uint8_t>& out, uint32_t value);

/// Write a big-endian 16-bit integer.
void write_uint16(std::vector<uint8_t>& out, uint16_t value);

/// Write a big-endian 32-bit integer.
void write_uint32(std::vector<uint8_t>& out, uint32_t value);

// ---------------------------------------------------------------------------
// Reading (for MIDI import)

/// Read a variable-length quantity from a byte stream. Advances pos.
uint32_t read_vlq(const std::vector<uint8_t>& data, size_t& pos);

/// Read a big-endian 16-bit integer. Advances pos.
uint16_t read_uint16(const std::vector<uint8_t>& data, size_t& pos);

/// Read a big-endian 32-bit integer. Advances pos.
uint32_t read_uint32(const std::vector<uint8_t>& data, size_t& pos);

}  // namespace gingo::internal
