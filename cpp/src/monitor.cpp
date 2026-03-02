// Gingo — Music Theory Library
// Monitor implementation.
//
// SPDX-License-Identifier: MIT

#include "../include/gingo/monitor.hpp"
#include "../include/gingo/note_context.hpp"
#include "../include/gingo/field.hpp"
#include "../include/gingo/chord.hpp"

#include <algorithm>
#include <optional>

namespace gingo {

// ===========================================================================
// Monitor — constructor
// ===========================================================================

Monitor::Monitor()
    : channel_(0xFF),
      sustainHeld_(false),
      chordCb_(nullptr), chordCtx_(nullptr),
      fieldCb_(nullptr), fieldCtx_(nullptr),
      noteCb_(nullptr), noteCtx_(nullptr) {
    sustained_.reserve(16);
}

void Monitor::setChannel(uint8_t ch) {
    channel_ = ch;
}

// ===========================================================================
// Callback registration — function pointer style
// ===========================================================================

void Monitor::onChordDetected(ChordCallback cb, void* ctx) {
    chordCb_ = cb;
    chordCtx_ = ctx;
}

void Monitor::onFieldChanged(FieldCallback cb, void* ctx) {
    fieldCb_ = cb;
    fieldCtx_ = ctx;
}

void Monitor::onNoteOn(NoteCallback cb, void* ctx) {
    noteCb_ = cb;
    noteCtx_ = ctx;
}

// ===========================================================================
// Callback registration — std::function style
// ===========================================================================

void Monitor::onChordDetected(std::function<void(const Chord&)> fn) {
    chordFn_ = std::move(fn);
}

void Monitor::onFieldChanged(std::function<void(const Field&)> fn) {
    fieldFn_ = std::move(fn);
}

void Monitor::onNoteOn(std::function<void(const NoteContext&)> fn) {
    noteFn_ = std::move(fn);
}

// ===========================================================================
// MIDI event feed
// ===========================================================================

void Monitor::noteOn(uint8_t midiNum, uint8_t velocity) {
    // Add note if not already held
    auto it = std::find(held_.begin(), held_.end(), midiNum);
    if (it == held_.end()) {
        held_.push_back(midiNum);
        velocities_.push_back(velocity);
        if (sustainHeld_) {
            sustained_.push_back(false);
        }
    }

    // Analyze and fire callbacks
    analyse_();

    // Fire per-note callback with context
    if (hasField()) {
        Note note = noteFromMIDI(midiNum);
        fireNote_(field_->noteContext(note));
    }
}

void Monitor::noteOff(uint8_t midiNum) {
    auto it = std::find(held_.begin(), held_.end(), midiNum);
    if (it == held_.end()) return;

    size_t idx = std::distance(held_.begin(), it);

    if (sustainHeld_) {
        // Mark as sustained, but keep in held list
        if (idx < sustained_.size()) {
            sustained_[idx] = true;
        }
    } else {
        // Remove immediately
        held_.erase(it);
        velocities_.erase(velocities_.begin() + idx);
        if (sustained_.size() > idx) {
            sustained_.erase(sustained_.begin() + idx);
        }
        analyse_();
    }
}

void Monitor::reset() {
    held_.clear();
    velocities_.clear();
    sustained_.clear();
    sustainHeld_ = false;
    chord_ = std::nullopt;
    field_ = std::nullopt;
}

// ===========================================================================
// Sustain pedal
// ===========================================================================

void Monitor::sustainOn() {
    sustainHeld_ = true;
    // Resize sustained_ to match held_ if needed
    while (sustained_.size() < held_.size()) {
        sustained_.push_back(false);
    }
}

void Monitor::sustainOff() {
    sustainHeld_ = false;
    // Remove notes that were released while sustain was active
    std::vector<uint8_t> newHeld;
    std::vector<uint8_t> newVels;
    for (size_t i = 0; i < held_.size(); i++) {
        if (!sustained_[i]) {
            newHeld.push_back(held_[i]);
            newVels.push_back(velocities_[i]);
        }
    }
    held_ = std::move(newHeld);
    velocities_ = std::move(newVels);
    sustained_.clear();
    analyse_();
}

// ===========================================================================
// Internal helpers
// ===========================================================================

void Monitor::analyse_() {
    // Build chord from held notes
    std::optional<Chord> newChord = buildChordFromHeld_();

    if (newChord.has_value()) {
        if (!chord_.has_value() || chord_.value().name() != newChord.value().name()) {
            chord_ = newChord;
            fireChord_(*chord_);

            // Deduce field from chord
            std::optional<Field> newField = deduceFieldFromHeld_();
            if (newField.has_value()) {
                if (!field_.has_value() || field_.value().tonic().semitone() != newField->tonic().semitone() ||
                    field_.value().scale().type() != newField->scale().type()) {
                    field_ = newField;
                    fireField_(*field_);
                }
            } else if (field_.has_value()) {
                field_ = std::nullopt;
            }
        }
    } else if (chord_.has_value()) {
        chord_ = std::nullopt;
        field_ = std::nullopt;
    }
}

std::optional<Chord> Monitor::buildChordFromHeld_() const {
    if (held_.size() < 2) {
        return std::nullopt;
    }

    // Sort by MIDI number to preserve bass note ordering, then deduplicate
    // by pitch class (keep lowest occurrence of each pitch class).
    // This ensures [A3, C4, E4] → [A, C, E] → Am (not [C, E, A] → C6).
    std::vector<uint8_t> sorted = held_;
    std::sort(sorted.begin(), sorted.end());

    std::vector<Note> notes;
    for (uint8_t midi : sorted) {
        uint8_t pc = midi % 12;
        bool dup = false;
        for (const auto& n : notes) {
            if (n.semitone() == pc) { dup = true; break; }
        }
        if (!dup) notes.push_back(noteFromMIDI(midi));
    }
    if (notes.size() < 2) return std::nullopt;

    try {
        return Chord::identify(notes);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<Field> Monitor::deduceFieldFromHeld_() const {
    if (!chord_.has_value()) {
        return std::nullopt;
    }

    const Chord& c = chord_.value();

    // Try to deduce field from chord
    // Try major and natural minor
    for (int mode = 0; mode <= 1; mode++) {
        ScaleType type = (mode == 0) ? ScaleType::Major : ScaleType::NaturalMinor;
        for (int tonicSemitone = 0; tonicSemitone < 12; tonicSemitone++) {
            const auto& chromatic = Note::chromatic();
            std::string tonicName = chromatic[tonicSemitone];
            Field f(tonicName, type);

            // Check if chord is in this field
            auto func = f.function(c);
            if (func.has_value()) {
                return f;
            }
        }
    }

    return std::nullopt;
}

void Monitor::fireChord_(const Chord& c) {
    if (chordFn_) { chordFn_(c); }
    if (chordCb_) { chordCb_(c, chordCtx_); }
}

void Monitor::fireField_(const Field& f) {
    if (fieldFn_) { fieldFn_(f); }
    if (fieldCb_) { fieldCb_(f, fieldCtx_); }
}

void Monitor::fireNote_(const NoteContext& ctx) {
    if (noteFn_) { noteFn_(ctx); }
    if (noteCb_) { noteCb_(ctx, noteCtx_); }
}

// ===========================================================================
// Static helpers
// ===========================================================================

Note Monitor::noteFromMIDI(uint8_t midiNum) {
    const auto& chromatic = Note::chromatic();
    return Note(chromatic[midiNum % 12]);
}

} // namespace gingo
