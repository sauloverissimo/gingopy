// Gingo — Music Theory Library
// Sequence MIDI import/export implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/sequence.hpp>
#include <gingo/internal/midi_format.hpp>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <map>

namespace gingo {

using namespace gingo::internal;

// ---------------------------------------------------------------------------
// Export (to_midi)

void Sequence::to_midi(const std::string& path, int ppqn) const {
    std::vector<uint8_t> track_data;

    // Tempo meta-event: FF 51 03 tt tt tt
    write_vlq(track_data, 0);  // delta = 0
    track_data.push_back(0xFF);
    track_data.push_back(0x51);
    track_data.push_back(0x03);
    int usec = tempo_.microseconds_per_beat();
    track_data.push_back(static_cast<uint8_t>((usec >> 16) & 0xFF));
    track_data.push_back(static_cast<uint8_t>((usec >> 8) & 0xFF));
    track_data.push_back(static_cast<uint8_t>(usec & 0xFF));

    // Time signature meta-event: FF 58 04 nn dd cc bb
    write_vlq(track_data, 0);  // delta = 0
    track_data.push_back(0xFF);
    track_data.push_back(0x58);
    track_data.push_back(0x04);
    track_data.push_back(static_cast<uint8_t>(time_signature_.beats_per_bar()));
    int dd = 0;
    int bu = time_signature_.beat_unit();
    while (bu > 1) { bu >>= 1; dd++; }
    track_data.push_back(static_cast<uint8_t>(dd));
    track_data.push_back(24);   // MIDI clocks per metronome click
    track_data.push_back(8);    // 32nd notes per MIDI quarter note

    // Events
    uint8_t channel = 0;
    uint8_t velocity = 100;
    uint32_t pending_delta = 0;  // accumulated rest ticks

    for (const auto& event : events_) {
        std::visit([&](const auto& ev) {
            using T = std::decay_t<decltype(ev)>;

            if constexpr (std::is_same_v<T, NoteEvent>) {
                uint32_t ticks = static_cast<uint32_t>(ev.duration().midi_ticks(ppqn));
                uint8_t midi = static_cast<uint8_t>(ev.midi_number() & 0x7F);

                // Note On
                write_vlq(track_data, pending_delta);
                pending_delta = 0;
                track_data.push_back(0x90 | channel);
                track_data.push_back(midi);
                track_data.push_back(velocity);

                // Note Off after duration
                write_vlq(track_data, ticks);
                track_data.push_back(0x80 | channel);
                track_data.push_back(midi);
                track_data.push_back(0);
            }
            else if constexpr (std::is_same_v<T, ChordEvent>) {
                uint32_t ticks = static_cast<uint32_t>(ev.duration().midi_ticks(ppqn));
                auto note_events = ev.note_events();

                // All Note Ons
                for (size_t i = 0; i < note_events.size(); ++i) {
                    write_vlq(track_data, (i == 0) ? pending_delta : 0);
                    uint8_t midi = static_cast<uint8_t>(note_events[i].midi_number() & 0x7F);
                    track_data.push_back(0x90 | channel);
                    track_data.push_back(midi);
                    track_data.push_back(velocity);
                }
                pending_delta = 0;

                // All Note Offs after duration
                for (size_t i = 0; i < note_events.size(); ++i) {
                    write_vlq(track_data, (i == 0) ? ticks : 0);
                    uint8_t midi = static_cast<uint8_t>(note_events[i].midi_number() & 0x7F);
                    track_data.push_back(0x80 | channel);
                    track_data.push_back(midi);
                    track_data.push_back(0);
                }
            }
            else if constexpr (std::is_same_v<T, Rest>) {
                pending_delta += static_cast<uint32_t>(ev.duration().midi_ticks(ppqn));
            }
        }, event);
    }

    // End of Track: FF 2F 00
    write_vlq(track_data, pending_delta);
    track_data.push_back(0xFF);
    track_data.push_back(0x2F);
    track_data.push_back(0x00);

    // Build complete MIDI file
    std::vector<uint8_t> file_data;

    // Header chunk: MThd
    file_data.push_back('M'); file_data.push_back('T');
    file_data.push_back('h'); file_data.push_back('d');
    write_uint32(file_data, 6);                        // header length
    write_uint16(file_data, 0);                        // format 0
    write_uint16(file_data, 1);                        // 1 track
    write_uint16(file_data, static_cast<uint16_t>(ppqn));

    // Track chunk: MTrk
    file_data.push_back('M'); file_data.push_back('T');
    file_data.push_back('r'); file_data.push_back('k');
    write_uint32(file_data, static_cast<uint32_t>(track_data.size()));
    file_data.insert(file_data.end(), track_data.begin(), track_data.end());

    // Write to file
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Cannot open file for writing: " + path);
    }
    out.write(reinterpret_cast<const char*>(file_data.data()),
              static_cast<std::streamsize>(file_data.size()));
}

// ---------------------------------------------------------------------------
// Import (from_midi)

// Internal: MIDI note name from number
static std::string midi_note_name(int midi) {
    static const char* NAMES[] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
    return NAMES[midi % 12];
}

static int midi_octave(int midi) {
    return (midi / 12) - 1;
}

Sequence Sequence::from_midi(const std::string& path) {
    // Read file
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Cannot open MIDI file: " + path);
    }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)),
                               std::istreambuf_iterator<char>());

    if (data.size() < 14) {
        throw std::runtime_error("Invalid MIDI file: too small");
    }

    // Parse header chunk
    size_t pos = 0;
    if (data[0] != 'M' || data[1] != 'T' || data[2] != 'h' || data[3] != 'd') {
        throw std::runtime_error("Invalid MIDI file: bad header");
    }
    pos = 4;

    uint32_t hdr_len = read_uint32(data, pos);
    uint16_t format = read_uint16(data, pos);
    uint16_t num_tracks = read_uint16(data, pos);
    uint16_t division = read_uint16(data, pos);

    // Skip extra header bytes if any
    if (hdr_len > 6) {
        pos += (hdr_len - 6);
    }

    if (division & 0x8000) {
        throw std::runtime_error("SMPTE time division not supported");
    }

    int ppqn = division;

    // Collect note on/off events with absolute tick positions
    struct RawNote {
        uint32_t on_tick;
        uint32_t off_tick;
        int midi;
        int octave;
        std::string name;
    };

    std::vector<RawNote> raw_notes;
    double bpm = 120.0;
    int ts_num = 4;
    int ts_denom = 4;

    // Parse all tracks
    for (int t = 0; t < num_tracks; ++t) {
        if (pos + 8 > data.size()) break;

        // Verify MTrk
        if (data[pos] != 'M' || data[pos+1] != 'T' ||
            data[pos+2] != 'r' || data[pos+3] != 'k') {
            throw std::runtime_error("Invalid MIDI track header");
        }
        pos += 4;

        uint32_t track_len = read_uint32(data, pos);
        size_t track_end = pos + track_len;

        uint32_t abs_tick = 0;
        uint8_t running_status = 0;

        // Active notes: midi number -> on tick
        std::map<int, uint32_t> active_notes;

        while (pos < track_end) {
            uint32_t delta = read_vlq(data, pos);
            abs_tick += delta;

            if (pos >= track_end) break;

            uint8_t status = data[pos];

            // Meta event
            if (status == 0xFF) {
                pos++;
                if (pos >= track_end) break;
                uint8_t meta_type = data[pos++];
                uint32_t meta_len = read_vlq(data, pos);

                if (meta_type == 0x51 && meta_len == 3) {
                    // Tempo
                    int usec = (static_cast<int>(data[pos]) << 16) |
                               (static_cast<int>(data[pos+1]) << 8) |
                                static_cast<int>(data[pos+2]);
                    if (usec > 0) bpm = 60000000.0 / usec;
                }
                else if (meta_type == 0x58 && meta_len >= 2) {
                    // Time signature
                    ts_num = data[pos];
                    ts_denom = 1 << data[pos+1];
                }
                else if (meta_type == 0x2F) {
                    // End of track
                    pos += meta_len;
                    break;
                }

                pos += meta_len;
                continue;
            }

            // SysEx
            if (status == 0xF0 || status == 0xF7) {
                pos++;
                uint32_t sysex_len = read_vlq(data, pos);
                pos += sysex_len;
                continue;
            }

            // Channel message
            uint8_t cmd;
            uint8_t data1;

            if (status & 0x80) {
                // New status byte
                cmd = status;
                running_status = status;
                pos++;
                if (pos >= track_end) break;
                data1 = data[pos++];
            } else {
                // Running status
                cmd = running_status;
                data1 = status;
                pos++;
            }

            uint8_t msg_type = cmd & 0xF0;

            if (msg_type == 0x90 || msg_type == 0x80) {
                // Note On or Note Off (2 data bytes)
                if (pos >= track_end) break;
                uint8_t data2 = data[pos++];

                int midi_num = data1 & 0x7F;
                int vel = data2 & 0x7F;

                bool is_note_on = (msg_type == 0x90 && vel > 0);

                if (is_note_on) {
                    active_notes[midi_num] = abs_tick;
                } else {
                    // Note Off
                    auto it = active_notes.find(midi_num);
                    if (it != active_notes.end()) {
                        RawNote rn;
                        rn.on_tick = it->second;
                        rn.off_tick = abs_tick;
                        rn.midi = midi_num;
                        rn.octave = midi_octave(midi_num);
                        rn.name = midi_note_name(midi_num);
                        raw_notes.push_back(rn);
                        active_notes.erase(it);
                    }
                }
            }
            else if (msg_type == 0xA0 || msg_type == 0xB0 || msg_type == 0xE0) {
                // 2 data bytes: aftertouch, control change, pitch bend
                if (pos < track_end) pos++;
            }
            else if (msg_type == 0xC0 || msg_type == 0xD0) {
                // 1 data byte: program change, channel pressure
                // data1 already consumed
            }
        }

        pos = track_end;
    }

    // Sort notes by on_tick, then by midi number
    std::sort(raw_notes.begin(), raw_notes.end(),
        [](const RawNote& a, const RawNote& b) {
            if (a.on_tick != b.on_tick) return a.on_tick < b.on_tick;
            return a.midi < b.midi;
        });

    // Build Sequence: group simultaneous notes into ChordEvents
    Sequence seq(Tempo(bpm), TimeSignature(ts_num, ts_denom));
    uint32_t current_tick = 0;

    size_t i = 0;
    while (i < raw_notes.size()) {
        // Find all notes starting at the same tick
        uint32_t onset = raw_notes[i].on_tick;

        // Add rest if there's a gap
        if (onset > current_tick) {
            uint32_t gap_ticks = onset - current_tick;
            Duration rest_dur = Duration::from_ticks(static_cast<int>(gap_ticks), ppqn);
            seq.add(Rest(rest_dur));
        }

        // Collect simultaneous notes
        size_t j = i;
        while (j < raw_notes.size() && raw_notes[j].on_tick == onset) {
            ++j;
        }

        size_t count = j - i;

        if (count == 1) {
            // Single note
            const auto& rn = raw_notes[i];
            uint32_t dur_ticks = rn.off_tick - rn.on_tick;
            Duration dur = Duration::from_ticks(static_cast<int>(dur_ticks), ppqn);
            Note note(rn.name);
            seq.add(NoteEvent(note, dur, rn.octave));
            current_tick = rn.off_tick;
        } else {
            // Multiple simultaneous notes -> try to build chord
            // Use the shortest duration among simultaneous notes
            uint32_t min_dur = UINT32_MAX;
            std::vector<std::string> note_names;
            int base_octave = raw_notes[i].octave;

            for (size_t k = i; k < j; ++k) {
                uint32_t dur_ticks = raw_notes[k].off_tick - raw_notes[k].on_tick;
                if (dur_ticks < min_dur) min_dur = dur_ticks;
                note_names.push_back(raw_notes[k].name);
            }

            Duration dur = Duration::from_ticks(static_cast<int>(min_dur), ppqn);

            // Try to identify as a chord
            try {
                Chord chord = Chord::identify(note_names);
                seq.add(ChordEvent(chord, dur, base_octave));
            } catch (...) {
                // Cannot identify chord — add individual NoteEvents
                for (size_t k = i; k < j; ++k) {
                    const auto& rn = raw_notes[k];
                    uint32_t d = rn.off_tick - rn.on_tick;
                    Duration nd = Duration::from_ticks(static_cast<int>(d), ppqn);
                    seq.add(NoteEvent(Note(rn.name), nd, rn.octave));
                }
            }

            current_tick = onset + min_dur;
        }

        i = j;
    }

    return seq;
}

}  // namespace gingo
