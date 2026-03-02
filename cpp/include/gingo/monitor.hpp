// Gingo — Music Theory Library
// Monitor: event-driven harmonic state tracker.
//
// Receives MIDI note-on/off events and fires callbacks when the harmonic
// state changes (new chord, new field, or any note-on with context).
//
// Two callback styles are provided:
//   • Function pointers (all platforms, heap-free)
//   • std::function (for lambda capture support)
//
// Both styles can be used simultaneously; when both are registered for the
// same event, both are fired (std::function first, then function pointer).
//
// Polling is always available alongside callbacks for backward compat.
//
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <optional>

#include "note.hpp"
#include "chord.hpp"
#include "scale.hpp"
#include "field.hpp"
#include "note_context.hpp"

namespace gingo {

// ===========================================================================
// Monitor — event-driven harmonic state tracker
// ===========================================================================

/// Event-driven harmonic state tracker.
///
/// Feed MIDI events via noteOn() / noteOff(). The monitor identifies
/// the current chord, deduces the most likely harmonic field, and fires
/// registered callbacks when state changes.
///
/// By default the monitor accepts events from any MIDI channel (omni mode).
/// Call setChannel() to restrict it to a specific channel so that
/// MIDI1::dispatch / MIDI2::dispatch automatically ignore other channels.
///
/// Examples:
///   Monitor mon;
///   mon.setChannel(0);  // listen to channel 0 only (omni = 0xFF, default)
///
///   // Lambda style (recommended):
///   mon.onChordDetected([](const Chord& c) {
///       std::cout << "Chord: " << c.name() << std::endl;
///   });
///
///   // Function pointer style:
///   mon.onChordDetected([](const Chord& c, void*) {
///       std::cout << "Chord: " << c.name() << std::endl;
///   }, nullptr);
///
///   mon.noteOn(60);  // C4 — triggers analysis
///   mon.noteOn(64);  // E4
///   mon.noteOn(67);  // G4 — onChordDetected fires with "CM"
class Monitor {
public:
    // ------------------------------------------------------------------
    // Callback types — function pointer style (heap-free)
    // ------------------------------------------------------------------

    /// Called when the identified chord changes.
    /// @param chord  The newly detected chord.
    /// @param ctx    User-provided context pointer (passed to onChordDetected).
    using ChordCallback = void (*)(const Chord& chord, void* ctx);

    /// Called when the deduced harmonic field changes.
    /// @param field  The best-matching harmonic field.
    /// @param ctx    User-provided context pointer.
    using FieldCallback = void (*)(const Field& field, void* ctx);

    /// Called on every noteOn, carrying per-note harmonic context.
    /// Fires even when no chord has been identified yet.
    /// @param ctx      Per-note context (degree, interval, function, inScale).
    /// @param userCtx  User-provided context pointer.
    using NoteCallback = void (*)(const NoteContext& ctx, void* userCtx);

    // ------------------------------------------------------------------
    // Constructor
    // ------------------------------------------------------------------

    Monitor();

    // ------------------------------------------------------------------
    // Callback registration — function pointer style
    // ------------------------------------------------------------------

    /// Register a callback for chord changes.
    /// Pass ctx=nullptr if no user data is needed.
    void onChordDetected(ChordCallback cb, void* ctx = nullptr);

    /// Register a callback for harmonic field changes.
    void onFieldChanged(FieldCallback cb, void* ctx = nullptr);

    /// Register a callback fired on every noteOn with per-note context.
    void onNoteOn(NoteCallback cb, void* ctx = nullptr);

    // ------------------------------------------------------------------
    // Callback registration — std::function style
    // ------------------------------------------------------------------

    /// Register a chord-change callback with lambda capture support.
    void onChordDetected(std::function<void(const Chord&)> fn);

    /// Register a field-change callback with lambda capture support.
    void onFieldChanged(std::function<void(const Field&)> fn);

    /// Register a per-note callback with lambda capture support.
    void onNoteOn(std::function<void(const NoteContext&)> fn);

    // ------------------------------------------------------------------
    // MIDI event feed
    // ------------------------------------------------------------------

    /// Process a MIDI Note On event.
    /// Adds the note, updates chord/field state, fires callbacks.
    /// @param midiNum   MIDI note number (0–127).
    /// @param velocity  MIDI velocity (1–127; ignored for state but stored).
    void noteOn(uint8_t midiNum, uint8_t velocity = 100);

    /// Process a MIDI Note Off event.
    /// Removes the note and re-evaluates state.
    /// @param midiNum  MIDI note number (0–127).
    void noteOff(uint8_t midiNum);

    /// Reset all held notes and clear chord/field state.
    void reset();

    // ------------------------------------------------------------------
    // Sustain pedal — called by the user or via CC64
    // ------------------------------------------------------------------

    /// Enable sustain. Notes released while sustain is active remain
    /// in the held list (contributing to chord/field detection) until
    /// sustainOff() is called.
    void sustainOn();

    /// Release sustain. Notes that were released while the pedal was
    /// held are removed and harmonic state is re-evaluated.
    void sustainOff();

    // ------------------------------------------------------------------
    // State access (polling — always available)
    // ------------------------------------------------------------------

    /// Number of currently held notes (includes sustained notes).
    uint8_t activeNoteCount() const { return static_cast<uint8_t>(held_.size()); }

    /// Whether the sustain pedal is active.
    bool hasSustain() const { return sustainHeld_; }

    /// Whether a chord has been identified from the held notes.
    bool hasChord() const { return chord_.has_value(); }

    /// Currently identified chord. Check hasChord() first.
    const Chord& currentChord() const { return chord_.value(); }

    /// Whether a harmonic field has been deduced.
    bool hasField() const { return field_.has_value(); }

    /// Currently deduced harmonic field. Check hasField() first.
    const Field& currentField() const { return field_.value(); }

    /// Get all currently held MIDI note numbers.
    const std::vector<uint8_t>& heldNotes() const { return held_; }

    // ------------------------------------------------------------------
    // Channel filter — used by MIDI1::dispatch / MIDI2::dispatch
    // ------------------------------------------------------------------

    /// Set the MIDI channel this monitor listens to (0-15).
    /// Pass 0xFF (default) to accept all channels (omni mode).
    void setChannel(uint8_t ch);

    /// Currently configured channel filter (0-15 or 0xFF for omni).
    uint8_t channel() const { return channel_; }

    /// Returns true if ch should be processed (omni or ch == channel_).
    /// Called internally by MIDI1::dispatch and MIDI2::dispatch.
    bool acceptsChannel(uint8_t ch) const {
        return channel_ == 0xFF || channel_ == ch;
    }

    // ------------------------------------------------------------------
    // Static helpers for MIDI dispatch
    // ------------------------------------------------------------------

    /// Convert MIDI note number to Note object.
    static Note noteFromMIDI(uint8_t midiNum);

private:
    // Channel filter (0xFF = omni)
    uint8_t channel_;

    // Held MIDI note numbers
    std::vector<uint8_t> held_;
    std::vector<uint8_t> velocities_;

    // Sustain pedal state
    bool sustainHeld_;
    std::vector<bool> sustained_;  // Per-slot: key was released while sustain active

    // Current harmonic state
    std::optional<Chord> chord_;
    std::optional<Field> field_;

    // Function pointer callbacks
    ChordCallback chordCb_;
    void*         chordCtx_;
    FieldCallback fieldCb_;
    void*         fieldCtx_;
    NoteCallback  noteCb_;
    void*         noteCtx_;

    // std::function callbacks
    std::function<void(const Chord&)>   chordFn_;
    std::function<void(const Field&)>   fieldFn_;
    std::function<void(const NoteContext&)> noteFn_;

    // Internal helpers
    void analyse_();
    std::optional<Chord> buildChordFromHeld_() const;
    std::optional<Field> deduceFieldFromHeld_() const;
    void fireChord_(const Chord& c);
    void fireField_(const Field& f);
    void fireNote_(const NoteContext& ctx);
};

} // namespace gingo
