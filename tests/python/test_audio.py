# SPDX-License-Identifier: MIT
"""Tests for gingo.audio — synthesis, WAV export, and playback."""

import math
import struct
import wave
from pathlib import Path
from unittest.mock import patch

import pytest

from gingo import (
    Note, Chord, Scale, Field, Tree, Sequence,
    NoteEvent, ChordEvent, Rest, Duration, Tempo, TimeSignature,
)
from gingo.audio import (
    Envelope, Waveform,
    play, to_wav,
    _obj_to_segments, _render_tone, _render_chord_tones,
    _render_segments, _render_silence, _parse_waveform,
)


# ---------------------------------------------------------------------------
# Waveform
# ---------------------------------------------------------------------------

class TestWaveform:
    def test_enum_values(self):
        assert Waveform.SINE.value == "sine"
        assert Waveform.SQUARE.value == "square"
        assert Waveform.SAWTOOTH.value == "sawtooth"
        assert Waveform.TRIANGLE.value == "triangle"

    def test_parse_string(self):
        assert _parse_waveform("sine") == Waveform.SINE
        assert _parse_waveform("SQUARE") == Waveform.SQUARE

    def test_parse_enum(self):
        assert _parse_waveform(Waveform.TRIANGLE) == Waveform.TRIANGLE

    def test_parse_invalid(self):
        with pytest.raises(ValueError, match="Unknown waveform"):
            _parse_waveform("wobble")
        with pytest.raises(TypeError):
            _parse_waveform(42)


# ---------------------------------------------------------------------------
# Envelope
# ---------------------------------------------------------------------------

class TestEnvelope:
    def test_default(self):
        env = Envelope()
        assert env.attack == 0.01
        assert env.decay == 0.08
        assert env.sustain == 0.6
        assert env.release == 0.2

    def test_attack_ramp(self):
        env = Envelope(attack=0.1, decay=0.0, sustain=1.0, release=0.0)
        assert env.amplitude(0.0, 1.0) == pytest.approx(0.0)
        assert env.amplitude(0.05, 1.0) == pytest.approx(0.5)
        assert env.amplitude(0.1, 1.0) == pytest.approx(1.0)

    def test_sustain_level(self):
        env = Envelope(attack=0.01, decay=0.01, sustain=0.5, release=0.01)
        # In the sustain region
        assert env.amplitude(0.5, 1.0) == pytest.approx(0.5)

    def test_release_ramp(self):
        env = Envelope(attack=0.0, decay=0.0, sustain=1.0, release=0.1)
        # At note_duration - release, amplitude = sustain
        assert env.amplitude(0.9, 1.0) == pytest.approx(1.0)
        # Halfway through release
        assert env.amplitude(0.95, 1.0) == pytest.approx(0.5)

    def test_after_note_duration(self):
        env = Envelope()
        assert env.amplitude(2.0, 1.0) == 0.0

    def test_very_short_note(self):
        env = Envelope(attack=0.5, decay=0.5, sustain=0.7, release=0.5)
        # Note shorter than attack+decay+release → uses simplified fade
        amp = env.amplitude(0.0, 0.1)
        assert amp == pytest.approx(0.0)
        # Peak at midpoint
        amp = env.amplitude(0.05, 0.1)
        assert amp == pytest.approx(1.0)


# ---------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------

class TestRendering:
    def test_render_tone_length(self):
        env = Envelope()
        samples = _render_tone(440.0, 0.5, 44100, Waveform.SINE, 0.8, env)
        assert len(samples) == 22050

    def test_render_tone_sine_shape(self):
        """A pure 1 Hz sine at t=0.25 should be at peak."""
        env = Envelope(attack=0.0, decay=0.0, sustain=1.0, release=0.0)
        samples = _render_tone(1.0, 1.0, 1000, Waveform.SINE, 1.0, env)
        # At sample 250 (t=0.25), sin(2π*0.25) = 1.0
        assert samples[250] == pytest.approx(1.0, abs=0.01)

    def test_render_chord_tones_mixing(self):
        env = Envelope(attack=0.0, decay=0.0, sustain=1.0, release=0.0)
        samples = _render_chord_tones([440.0, 550.0], 0.1, 44100,
                                       Waveform.SINE, 0.8, env, strum=0.0)
        assert len(samples) == 4410
        # No sample should exceed amplitude (divided by note count)
        assert all(-1.0 <= s <= 1.0 for s in samples)

    def test_render_chord_strum_extends_duration(self):
        env = Envelope(attack=0.0, decay=0.0, sustain=1.0, release=0.0)
        # 3 notes, strum=0.05 → total = 0.5 + 2*0.05 = 0.6s
        samples = _render_chord_tones([440.0, 550.0, 660.0], 0.5, 1000,
                                       Waveform.SINE, 0.8, env, strum=0.05)
        assert len(samples) == 600  # 0.6s * 1000 Hz

    def test_render_chord_strum_second_note_delayed(self):
        env = Envelope(attack=0.0, decay=0.0, sustain=1.0, release=0.0)
        # strum=0.05 at sr=1000 → second note starts at sample 50
        samples = _render_chord_tones([440.0, 550.0], 0.1, 1000,
                                       Waveform.SQUARE, 0.8, env, strum=0.05)
        # Before second note starts (sample 10), only first note contributes
        only_first = samples[10]
        # After second note starts (sample 60), both contribute
        both = samples[60]
        assert only_first != both

    def test_render_silence(self):
        samples = _render_silence(0.5, 44100)
        assert len(samples) == 22050
        assert all(s == 0.0 for s in samples)


# ---------------------------------------------------------------------------
# Object → segments
# ---------------------------------------------------------------------------

class TestObjToSegments:
    def test_note(self):
        segs = _obj_to_segments(Note("A"), octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 1
        assert segs[0][0] == "tone"
        assert segs[0][1][0] == pytest.approx(440.0)
        assert segs[0][2] == 0.5

    def test_chord(self):
        segs = _obj_to_segments(Chord("CM"), octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 1
        assert segs[0][0] == "chord"
        assert len(segs[0][1]) == 3  # C, E, G

    def test_scale(self):
        segs = _obj_to_segments(Scale("C", "major"), octave=4, duration=0.3, tuning=440.0)
        # 7 notes + tonic octave up = 8
        assert len(segs) == 8
        assert all(s[0] == "tone" for s in segs)

    def test_field(self):
        segs = _obj_to_segments(Field("C", "major"), octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 7
        assert all(s[0] == "chord" for s in segs)

    def test_tree(self):
        segs = _obj_to_segments(Tree("C", "major", "harmonic_tree"), octave=4, duration=0.5, tuning=440.0)
        assert len(segs) > 0
        assert all(s[0] == "chord" for s in segs)

    def test_note_event(self):
        ne = NoteEvent(Note("C"), Duration("quarter"), 4)
        segs = _obj_to_segments(ne, octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 1
        assert segs[0][0] == "tone"

    def test_chord_event(self):
        ce = ChordEvent(Chord("Am"), Duration("half"), 4)
        segs = _obj_to_segments(ce, octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 1
        assert segs[0][0] == "chord"

    def test_rest(self):
        r = Rest(Duration("quarter"))
        segs = _obj_to_segments(r, octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 1
        assert segs[0][0] == "silence"

    def test_sequence(self):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))
        seq.add(Rest(Duration("quarter")))
        seq.add(ChordEvent(Chord("CM"), Duration("half"), 4))
        segs = _obj_to_segments(seq, octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 3
        assert segs[0][0] == "tone"
        assert segs[1][0] == "silence"
        assert segs[2][0] == "chord"

    def test_list_of_chord_names(self):
        segs = _obj_to_segments(["CM", "Am", "FM", "GM"],
                                 octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 4
        assert all(s[0] == "chord" for s in segs)

    def test_list_of_note_names(self):
        segs = _obj_to_segments(["C", "D", "E"],
                                 octave=4, duration=0.5, tuning=440.0)
        assert len(segs) == 3
        assert all(s[0] == "tone" for s in segs)

    def test_unknown_type_raises(self):
        with pytest.raises(TypeError, match="Cannot play"):
            _obj_to_segments(42, octave=4, duration=0.5, tuning=440.0)


# ---------------------------------------------------------------------------
# WAV export
# ---------------------------------------------------------------------------

class TestToWav:
    def test_wav_header(self, tmp_path):
        path = tmp_path / "test.wav"
        to_wav(Note("A"), str(path), duration=0.1, sample_rate=8000)
        with wave.open(str(path), "r") as wf:
            assert wf.getnchannels() == 1
            assert wf.getsampwidth() == 2
            assert wf.getframerate() == 8000

    def test_wav_frame_count(self, tmp_path):
        path = tmp_path / "test.wav"
        to_wav(Note("A"), str(path), duration=0.5, sample_rate=8000)
        with wave.open(str(path), "r") as wf:
            assert wf.getnframes() == 4000

    def test_wav_chord(self, tmp_path):
        path = tmp_path / "chord.wav"
        to_wav(Chord("Am7"), str(path), duration=0.2, sample_rate=8000)
        assert path.exists()
        with wave.open(str(path), "r") as wf:
            assert wf.getnframes() > 0

    def test_wav_scale(self, tmp_path):
        path = tmp_path / "scale.wav"
        to_wav(Scale("C", "major"), str(path), duration=0.1, sample_rate=8000, gap=0.0)
        with wave.open(str(path), "r") as wf:
            # 8 notes * 0.1s * 8000 = 6400 frames (gap=0)
            assert wf.getnframes() == 6400

    def test_wav_waveform_square(self, tmp_path):
        path = tmp_path / "sq.wav"
        to_wav(Note("A"), str(path), duration=0.1, sample_rate=8000,
               waveform="square")
        assert path.exists()

    def test_wav_path_as_pathlib(self, tmp_path):
        path = tmp_path / "pathlib.wav"
        to_wav(Note("C"), path, duration=0.1, sample_rate=8000)
        assert path.exists()


# ---------------------------------------------------------------------------
# Monkey-patch (.play / .to_wav on classes)
# ---------------------------------------------------------------------------

class TestMonkeyPatch:
    def test_note_has_play(self):
        assert hasattr(Note("C"), "play")

    def test_chord_has_play(self):
        assert hasattr(Chord("Am7"), "play")

    def test_scale_has_play(self):
        assert hasattr(Scale("C", "major"), "play")

    def test_note_has_to_wav(self):
        assert hasattr(Note("C"), "to_wav")

    def test_note_to_wav(self, tmp_path):
        path = tmp_path / "note.wav"
        Note("C").to_wav(str(path), duration=0.1, sample_rate=8000)
        assert path.exists()

    def test_chord_to_wav(self, tmp_path):
        path = tmp_path / "chord.wav"
        Chord("Am7").to_wav(str(path), duration=0.1, sample_rate=8000)
        assert path.exists()


# ---------------------------------------------------------------------------
# Sequence __getitem__
# ---------------------------------------------------------------------------

class TestSequenceGetitem:
    def test_getitem_note_event(self):
        seq = Sequence()
        ne = NoteEvent(Note("C"), Duration("quarter"), 4)
        seq.add(ne)
        got = seq[0]
        assert isinstance(got, NoteEvent)

    def test_getitem_chord_event(self):
        seq = Sequence()
        ce = ChordEvent(Chord("CM"), Duration("half"), 4)
        seq.add(ce)
        assert isinstance(seq[0], ChordEvent)

    def test_getitem_rest(self):
        seq = Sequence()
        seq.add(Rest(Duration("quarter")))
        assert isinstance(seq[0], Rest)

    def test_getitem_negative(self):
        seq = Sequence()
        seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))
        seq.add(NoteEvent(Note("D"), Duration("quarter"), 4))
        last = seq[-1]
        assert isinstance(last, NoteEvent)

    def test_getitem_out_of_range(self):
        seq = Sequence()
        with pytest.raises(IndexError):
            seq[0]
