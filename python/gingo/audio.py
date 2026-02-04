# SPDX-License-Identifier: MIT
"""Audio synthesis and playback for gingo objects.

Renders Note, Chord, Scale, Field, Tree, Sequence, and event objects
to audio using simple waveform synthesis. Zero external dependencies
for synthesis and WAV export; ``simpleaudio`` (optional) for playback.

Usage::

    from gingo import Note, Chord, Scale
    from gingo.audio import play, to_wav, Waveform

    play(Note("C"))
    play(Chord("Am7"), waveform="square")
    play(Scale("C", "major"), duration=0.3)
    to_wav(Chord("G7"), "g7.wav")
"""

from __future__ import annotations

import math
import struct
import wave
from enum import Enum
from pathlib import Path
from typing import Any, List, Union


# ---------------------------------------------------------------------------
# Waveform
# ---------------------------------------------------------------------------

class Waveform(Enum):
    """Available waveform shapes for synthesis."""
    SINE = "sine"
    SQUARE = "square"
    SAWTOOTH = "sawtooth"
    TRIANGLE = "triangle"


def _sine(phase: float) -> float:
    return math.sin(2.0 * math.pi * phase)


def _square(phase: float) -> float:
    return 1.0 if (phase % 1.0) < 0.5 else -1.0


def _sawtooth(phase: float) -> float:
    return 2.0 * (phase % 1.0) - 1.0


def _triangle(phase: float) -> float:
    t = phase % 1.0
    return 4.0 * t - 1.0 if t < 0.5 else 3.0 - 4.0 * t


_WAVE_FUNCS = {
    Waveform.SINE: _sine,
    Waveform.SQUARE: _square,
    Waveform.SAWTOOTH: _sawtooth,
    Waveform.TRIANGLE: _triangle,
}


# ---------------------------------------------------------------------------
# Envelope
# ---------------------------------------------------------------------------

class Envelope:
    """ADSR amplitude envelope.

    Args:
        attack: Ramp-up time in seconds.
        decay: Time to drop from peak to sustain level.
        sustain: Sustain amplitude (0.0–1.0).
        release: Ramp-down time in seconds at note end.
    """

    def __init__(
        self,
        attack: float = 0.01,
        decay: float = 0.08,
        sustain: float = 0.6,
        release: float = 0.2,
    ) -> None:
        self.attack = attack
        self.decay = decay
        self.sustain = sustain
        self.release = release

    def amplitude(self, t: float, note_duration: float) -> float:
        """Return amplitude at time *t* for a note lasting *note_duration* s."""
        sustain_end = note_duration - self.release
        # Note too short for full ADSR — simple fade in/out
        if sustain_end < self.attack + self.decay:
            half = note_duration / 2.0
            if half <= 0:
                return 0.0
            if t < half:
                return t / half
            return max(0.0, 1.0 - (t - half) / half)

        if t < self.attack:
            return t / self.attack if self.attack > 0 else 1.0
        if t < self.attack + self.decay:
            frac = (t - self.attack) / self.decay if self.decay > 0 else 1.0
            return 1.0 - (1.0 - self.sustain) * frac
        if t < sustain_end:
            return self.sustain
        if t < note_duration:
            frac = (t - sustain_end) / self.release if self.release > 0 else 1.0
            return self.sustain * (1.0 - frac)
        return 0.0


# ---------------------------------------------------------------------------
# Internal rendering
# ---------------------------------------------------------------------------

def _render_tone(
    frequency: float,
    duration_sec: float,
    sample_rate: int,
    waveform: Waveform,
    amplitude: float,
    envelope: Envelope,
) -> list[float]:
    """Render a single-frequency tone."""
    n = int(duration_sec * sample_rate)
    fn = _WAVE_FUNCS[waveform]
    inv_sr = 1.0 / sample_rate
    samples = [0.0] * n
    for i in range(n):
        t = i * inv_sr
        samples[i] = fn(frequency * t) * amplitude * envelope.amplitude(t, duration_sec)
    return samples


def _render_chord_tones(
    frequencies: list[float],
    duration_sec: float,
    sample_rate: int,
    waveform: Waveform,
    amplitude: float,
    envelope: Envelope,
    strum: float = 0.0,
) -> list[float]:
    """Render several frequencies mixed together (chord).

    With *strum* > 0 each successive note starts a little later,
    producing a strummed / arpeggiated feel instead of a flat block.
    """
    if not frequencies:
        return [0.0] * int(duration_sec * sample_rate)
    fn = _WAVE_FUNCS[waveform]
    inv_sr = 1.0 / sample_rate
    per_note = amplitude / len(frequencies)

    # Total buffer includes the extra strum spread
    total_strum = strum * (len(frequencies) - 1)
    total_n = int((duration_sec + total_strum) * sample_rate)
    note_n = int(duration_sec * sample_rate)
    mixed = [0.0] * total_n

    for idx, freq in enumerate(frequencies):
        offset = int(idx * strum * sample_rate)
        for i in range(note_n):
            pos = offset + i
            if pos >= total_n:
                break
            t = i * inv_sr
            mixed[pos] += fn(freq * t) * per_note * envelope.amplitude(t, duration_sec)
    return mixed


def _render_silence(duration_sec: float, sample_rate: int) -> list[float]:
    return [0.0] * int(duration_sec * sample_rate)


# ---------------------------------------------------------------------------
# Chord voicing helpers
# ---------------------------------------------------------------------------

def _chord_frequencies(chord: Any, octave: int, tuning: float) -> list[float]:
    """Compute frequencies for each chord tone with ascending voicing.

    Notes above the root (by pitch-class) stay in *octave*; notes below
    the root move up one octave so the chord is voiced in close position.
    """
    root_semi = chord.root().semitone()
    freqs: list[float] = []
    for note in chord.notes():
        note_oct = octave if note.semitone() >= root_semi else octave + 1
        freqs.append(note.frequency(note_oct, tuning))
    return freqs


# ---------------------------------------------------------------------------
# Object → segments conversion
# ---------------------------------------------------------------------------

# Segment = ("tone", [freq], dur_sec)
#          | ("chord", [freqs], dur_sec)
#          | ("silence", dur_sec)

def _obj_to_segments(
    obj: Any,
    octave: int,
    duration: float,
    tuning: float,
) -> list[tuple]:
    """Convert any gingo object to a list of audio segments."""
    from gingo._gingo import (
        Note, Chord, Scale, Field, Tree, Sequence,
        NoteEvent, ChordEvent, Rest,
    )

    if isinstance(obj, Note):
        return [("tone", [obj.frequency(octave, tuning)], duration)]

    if isinstance(obj, NoteEvent):
        return [("tone", [obj.frequency(tuning)], duration)]

    if isinstance(obj, Chord):
        return [("chord", _chord_frequencies(obj, octave, tuning), duration)]

    if isinstance(obj, ChordEvent):
        freqs = [ne.frequency(tuning) for ne in obj.note_events()]
        return [("chord", freqs, duration)]

    if isinstance(obj, Rest):
        return [("silence", duration)]

    if isinstance(obj, Scale):
        segs: list[tuple] = []
        for note in obj:
            segs.append(("tone", [note.frequency(octave, tuning)], duration))
        # Complete with the tonic one octave up
        segs.append(("tone", [obj.tonic().frequency(octave + 1, tuning)], duration))
        return segs

    if isinstance(obj, Field):
        return [
            ("chord", _chord_frequencies(ch, octave, tuning), duration)
            for ch in obj
        ]

    if isinstance(obj, Tree):
        # Play the diatonic chords via the corresponding Field
        field = Field(obj.tonic().name(), str(obj.type()).rsplit(".", 1)[-1])
        return [
            ("chord", _chord_frequencies(ch, octave, tuning), duration)
            for ch in field
        ]

    if isinstance(obj, Sequence):
        tempo = obj.tempo()
        segs = []
        for i in range(len(obj)):
            event = obj[i]
            if isinstance(event, NoteEvent):
                segs.append((
                    "tone",
                    [event.frequency(tuning)],
                    tempo.seconds(event.duration()),
                ))
            elif isinstance(event, ChordEvent):
                segs.append((
                    "chord",
                    [ne.frequency(tuning) for ne in event.note_events()],
                    tempo.seconds(event.duration()),
                ))
            elif isinstance(event, Rest):
                segs.append(("silence", tempo.seconds(event.duration())))
        return segs

    if isinstance(obj, (list, tuple)):
        segs = []
        for item in obj:
            if isinstance(item, str):
                try:
                    ch = Chord(item)
                    segs.append(("chord", _chord_frequencies(ch, octave, tuning), duration))
                except (ValueError, RuntimeError):
                    try:
                        n = Note(item)
                        segs.append(("tone", [n.frequency(octave, tuning)], duration))
                    except (ValueError, RuntimeError):
                        pass
            else:
                segs.extend(_obj_to_segments(item, octave, duration, tuning))
        return segs

    raise TypeError(f"Cannot play object of type {type(obj).__name__}")


def _render_segments(
    segments: list[tuple],
    sample_rate: int,
    waveform: Waveform,
    amplitude: float,
    envelope: Envelope,
    strum: float = 0.0,
    gap: float = 0.0,
) -> list[float]:
    """Render all segments into a flat sample buffer.

    *gap* inserts a short silence between consecutive notes/chords,
    giving each one space to breathe (like lifting fingers between keys).
    """
    samples: list[float] = []
    gap_samples = int(gap * sample_rate) if gap > 0 else 0
    last = len(segments) - 1

    for i, seg in enumerate(segments):
        kind = seg[0]
        if kind == "tone":
            samples.extend(_render_tone(
                seg[1][0], seg[2], sample_rate, waveform, amplitude, envelope,
            ))
        elif kind == "chord":
            samples.extend(_render_chord_tones(
                seg[1], seg[2], sample_rate, waveform, amplitude, envelope,
                strum,
            ))
        elif kind == "silence":
            samples.extend(_render_silence(seg[1], sample_rate))

        # Insert gap between segments (not after the last one)
        if gap_samples and i < last and kind != "silence":
            samples.extend([0.0] * gap_samples)

    return samples


# ---------------------------------------------------------------------------
# WAV output
# ---------------------------------------------------------------------------

def _write_wav(path: str, samples: list[float], sample_rate: int) -> None:
    """Write float samples (–1…+1) to a 16-bit mono WAV file."""
    n = len(samples)
    int_samples = [0] * n
    for i in range(n):
        s = samples[i]
        if s > 1.0:
            s = 1.0
        elif s < -1.0:
            s = -1.0
        int_samples[i] = int(s * 32767)
    data = struct.pack(f"<{n}h", *int_samples)
    with wave.open(path, "w") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(sample_rate)
        wf.writeframes(data)


# ---------------------------------------------------------------------------
# Playback
# ---------------------------------------------------------------------------

def _play_samples(samples: list[float], sample_rate: int) -> None:
    """Play rendered samples through the system audio output."""
    n = len(samples)
    int_samples = [0] * n
    for i in range(n):
        s = samples[i]
        if s > 1.0:
            s = 1.0
        elif s < -1.0:
            s = -1.0
        int_samples[i] = int(s * 32767)
    audio_data = struct.pack(f"<{n}h", *int_samples)

    # Strategy 1: simpleaudio (cross-platform, preferred)
    try:
        import simpleaudio as sa
        play_obj = sa.play_buffer(audio_data, 1, 2, sample_rate)
        play_obj.wait_done()
        return
    except ImportError:
        pass

    # Strategy 2: temp WAV + system player
    import os
    import subprocess
    import sys
    import tempfile

    fd, tmp_path = tempfile.mkstemp(suffix=".wav")
    os.close(fd)
    try:
        _write_wav(tmp_path, samples, sample_rate)
        if sys.platform == "darwin":
            subprocess.run(["afplay", tmp_path], check=True, capture_output=True)
        elif sys.platform.startswith("linux"):
            for cmd in (
                ["aplay", tmp_path],
                ["paplay", tmp_path],
                ["ffplay", "-nodisp", "-autoexit", tmp_path],
            ):
                try:
                    subprocess.run(cmd, check=True, capture_output=True)
                    return
                except (FileNotFoundError, subprocess.CalledProcessError):
                    continue
            raise RuntimeError(
                "No audio player found. Install simpleaudio: "
                "pip install gingo[audio]"
            )
        elif sys.platform == "win32":
            import time
            os.startfile(tmp_path)  # type: ignore[attr-defined]
            time.sleep(n / sample_rate + 0.5)
    finally:
        try:
            os.unlink(tmp_path)
        except OSError:
            pass


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _parse_waveform(waveform: Any) -> Waveform:
    if isinstance(waveform, Waveform):
        return waveform
    if isinstance(waveform, str):
        try:
            return Waveform(waveform.lower())
        except ValueError:
            valid = ", ".join(w.value for w in Waveform)
            raise ValueError(f"Unknown waveform '{waveform}'. Valid: {valid}") from None
    raise TypeError(f"waveform must be Waveform or str, got {type(waveform).__name__}")


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def play(
    obj: Any,
    *,
    octave: int = 4,
    duration: float = 0.5,
    waveform: Union[Waveform, str] = Waveform.SINE,
    amplitude: float = 0.8,
    envelope: Envelope | None = None,
    strum: float = 0.03,
    gap: float = 0.05,
    tuning: float = 440.0,
    sample_rate: int = 44100,
) -> None:
    """Play a gingo object through the system audio output.

    Accepts Note, Chord, Scale, Field, Tree, Sequence, NoteEvent,
    ChordEvent, Rest, or a list of chord/note names.

    Args:
        obj: The gingo object to play.
        octave: Base octave (default 4 = middle-C octave).
        duration: Seconds per note/chord (default 0.5).
            Ignored for Sequence (uses its own tempo and durations).
        waveform: ``"sine"``, ``"square"``, ``"sawtooth"``, or ``"triangle"``.
        amplitude: Volume 0.0–1.0 (default 0.8).
        envelope: ADSR envelope. Default: attack=0.01, decay=0.08,
            sustain=0.6, release=0.2.
        strum: Delay in seconds between each note of a chord (default 0.03).
            Creates a strummed / arpeggiated feel. Set to 0 for simultaneous.
        gap: Silence in seconds between consecutive notes/chords (default 0.05).
            Gives each sound space to breathe. Set to 0 for legato.
        tuning: A4 reference in Hz (default 440).
        sample_rate: Samples per second (default 44100).
    """
    wf = _parse_waveform(waveform)
    env = envelope or Envelope()
    segments = _obj_to_segments(obj, octave=octave, duration=duration, tuning=tuning)
    samples = _render_segments(segments, sample_rate, wf, amplitude, env, strum, gap)
    _play_samples(samples, sample_rate)


def to_wav(
    obj: Any,
    path: Union[str, Path],
    *,
    octave: int = 4,
    duration: float = 0.5,
    waveform: Union[Waveform, str] = Waveform.SINE,
    amplitude: float = 0.8,
    envelope: Envelope | None = None,
    strum: float = 0.03,
    gap: float = 0.05,
    tuning: float = 440.0,
    sample_rate: int = 44100,
) -> None:
    """Render a gingo object to a WAV audio file.

    Same object types and parameters as :func:`play`.
    """
    wf = _parse_waveform(waveform)
    env = envelope or Envelope()
    segments = _obj_to_segments(obj, octave=octave, duration=duration, tuning=tuning)
    samples = _render_segments(segments, sample_rate, wf, amplitude, env, strum, gap)
    _write_wav(str(path), samples, sample_rate)
