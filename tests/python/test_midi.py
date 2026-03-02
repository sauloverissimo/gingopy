# Gingo — MIDI import/export tests (Python)
# SPDX-License-Identifier: MIT

import os
import pytest
from gingo import (
    Sequence, Tempo, TimeSignature,
    NoteEvent, ChordEvent, Rest, Duration,
    Note, Chord,
)


class TestSequenceToMidi:
    def test_to_midi_creates_file(self, tmp_path):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        seq.add(NoteEvent(Note("E"), Duration("quarter")))

        path = str(tmp_path / "test.mid")
        seq.to_midi(path)

        assert os.path.exists(path)
        with open(path, "rb") as f:
            assert f.read(4) == b"MThd"

    def test_to_midi_with_chords(self, tmp_path):
        seq = Sequence(Tempo(100), TimeSignature(4, 4))
        seq.add(ChordEvent(Chord("CM"), Duration("whole")))

        path = str(tmp_path / "chord.mid")
        seq.to_midi(path)

        assert os.path.exists(path)
        assert os.path.getsize(path) > 14

    def test_to_midi_with_rests(self, tmp_path):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        seq.add(Rest(Duration("quarter")))
        seq.add(NoteEvent(Note("E"), Duration("half")))

        path = str(tmp_path / "rest.mid")
        seq.to_midi(path)

        assert os.path.exists(path)

    def test_to_midi_custom_ppqn(self, tmp_path):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(NoteEvent(Note("C"), Duration("quarter")))

        path = str(tmp_path / "ppqn96.mid")
        seq.to_midi(path, ppqn=96)

        with open(path, "rb") as f:
            data = f.read()
            # ppqn is at bytes 12-13 (big-endian)
            ppqn = (data[12] << 8) | data[13]
            assert ppqn == 96


class TestSequenceFromMidi:
    def test_from_midi_invalid_path(self):
        with pytest.raises(RuntimeError):
            Sequence.from_midi("/nonexistent/file.mid")

    def test_from_midi_invalid_content(self, tmp_path):
        path = str(tmp_path / "invalid.mid")
        with open(path, "wb") as f:
            f.write(b"not a midi file")

        with pytest.raises(RuntimeError):
            Sequence.from_midi(path)


class TestMidiRoundtrip:
    def test_roundtrip_notes(self, tmp_path):
        original = Sequence(Tempo(120), TimeSignature(4, 4))
        original.add(NoteEvent(Note("C"), Duration("quarter")))
        original.add(NoteEvent(Note("E"), Duration("quarter")))
        original.add(NoteEvent(Note("G"), Duration("half")))

        path = str(tmp_path / "roundtrip.mid")
        original.to_midi(path)

        loaded = Sequence.from_midi(path)
        assert len(loaded) == 3
        assert loaded.total_duration() == pytest.approx(4.0, abs=0.1)
        assert loaded.tempo().bpm() == pytest.approx(120.0, abs=1.0)

    def test_roundtrip_with_rest(self, tmp_path):
        original = Sequence(Tempo(90), TimeSignature(4, 4))
        original.add(NoteEvent(Note("A"), Duration("quarter")))
        original.add(Rest(Duration("quarter")))
        original.add(NoteEvent(Note("B"), Duration("half")))

        path = str(tmp_path / "roundtrip_rest.mid")
        original.to_midi(path)

        loaded = Sequence.from_midi(path)
        assert len(loaded) == 3
        assert loaded.total_duration() == pytest.approx(4.0, abs=0.1)
        assert loaded.tempo().bpm() == pytest.approx(90.0, abs=1.0)

    def test_roundtrip_with_chord(self, tmp_path):
        original = Sequence(Tempo(120), TimeSignature(4, 4))
        original.add(ChordEvent(Chord("CM"), Duration("whole")))

        path = str(tmp_path / "roundtrip_chord.mid")
        original.to_midi(path)

        loaded = Sequence.from_midi(path)
        assert len(loaded) >= 1
        assert loaded.total_duration() == pytest.approx(4.0, abs=0.1)

    def test_roundtrip_tempo_preserved(self, tmp_path):
        for bpm in [60, 90, 120, 180]:
            original = Sequence(Tempo(bpm), TimeSignature(4, 4))
            original.add(NoteEvent(Note("C"), Duration("quarter")))

            path = str(tmp_path / f"tempo_{bpm}.mid")
            original.to_midi(path)

            loaded = Sequence.from_midi(path)
            assert loaded.tempo().bpm() == pytest.approx(bpm, abs=1.0)

    def test_roundtrip_time_signature(self, tmp_path):
        original = Sequence(Tempo(120), TimeSignature(3, 4))
        original.add(NoteEvent(Note("C"), Duration("quarter")))

        path = str(tmp_path / "ts_34.mid")
        original.to_midi(path)

        loaded = Sequence.from_midi(path)
        assert loaded.time_signature().beats_per_bar() == 3
        assert loaded.time_signature().beat_unit() == 4
