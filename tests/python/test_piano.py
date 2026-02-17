"""Tests for the Piano class."""

import pytest
from gingo import (
    Piano, PianoKey, PianoVoicing, VoicingStyle,
    Note, Chord, Scale, ScaleType,
)


# ---------------------------------------------------------------------------
# Construction
# ---------------------------------------------------------------------------

class TestPianoConstruction:
    def test_default_88_keys(self):
        piano = Piano()
        assert piano.num_keys() == 88

    def test_61_keys(self):
        piano = Piano(61)
        assert piano.num_keys() == 61
        assert piano.in_range(60)

    def test_lowest_highest_88(self):
        piano = Piano()
        assert piano.lowest().midi == 21   # A0
        assert piano.highest().midi == 108  # C8

    def test_invalid_raises(self):
        with pytest.raises(Exception):
            Piano(0)
        with pytest.raises(Exception):
            Piano(129)


# ---------------------------------------------------------------------------
# key()
# ---------------------------------------------------------------------------

class TestPianoKey:
    def test_middle_c(self):
        piano = Piano()
        k = piano.key(Note("C"), 4)
        assert k.midi == 60
        assert k.octave == 4
        assert k.note == "C"
        assert k.white is True

    def test_a0_first_key(self):
        piano = Piano()
        k = piano.key(Note("A"), 0)
        assert k.midi == 21
        assert k.position == 1

    def test_c8_last_key(self):
        piano = Piano()
        k = piano.key(Note("C"), 8)
        assert k.midi == 108
        assert k.position == 88

    def test_black_key(self):
        piano = Piano()
        k = piano.key(Note("C#"), 4)
        assert k.midi == 61
        assert k.white is False

    def test_bb_is_black(self):
        piano = Piano()
        k = piano.key(Note("Bb"), 4)
        assert k.midi == 70
        assert k.white is False

    def test_out_of_range_raises(self):
        piano = Piano()
        with pytest.raises(Exception):
            piano.key(Note("C"), 9)

    def test_to_dict(self):
        piano = Piano()
        d = piano.key(Note("C"), 4).to_dict()
        assert d["midi"] == 60
        assert d["note"] == "C"
        assert d["white"] is True


# ---------------------------------------------------------------------------
# keys()
# ---------------------------------------------------------------------------

class TestPianoKeys:
    def test_c_all_octaves(self):
        piano = Piano()
        ks = piano.keys(Note("C"))
        assert len(ks) == 8  # C1 to C8
        assert ks[0].midi == 24   # C1
        assert ks[-1].midi == 108  # C8

    def test_a_all_octaves(self):
        piano = Piano()
        ks = piano.keys(Note("A"))
        assert len(ks) == 8  # A0 to A7
        assert ks[0].midi == 21  # A0


# ---------------------------------------------------------------------------
# voicing()
# ---------------------------------------------------------------------------

class TestPianoVoicing:
    def test_cm_close(self):
        piano = Piano()
        v = piano.voicing(Chord("CM"), 4, VoicingStyle.Close)
        assert v.chord_name == "CM"
        assert v.style == VoicingStyle.Close
        assert len(v.keys) == 3
        midis = [k.midi for k in v.keys]
        assert midis == [60, 64, 67]

    def test_cm_open(self):
        piano = Piano()
        v = piano.voicing(Chord("CM"), 4, VoicingStyle.Open)
        assert len(v.keys) == 3
        assert v.keys[0].midi == 48  # C3 (root dropped)
        assert v.keys[1].midi == 64  # E4
        assert v.keys[2].midi == 67  # G4

    def test_am7_close(self):
        piano = Piano()
        v = piano.voicing(Chord("Am7"), 4, VoicingStyle.Close)
        assert len(v.keys) == 4

    def test_am7_shell(self):
        piano = Piano()
        v = piano.voicing(Chord("Am7"), 4, VoicingStyle.Shell)
        # Shell: root + 3rd + 7th = 3 keys
        assert len(v.keys) == 3

    def test_voicings_returns_three(self):
        piano = Piano()
        vs = piano.voicings(Chord("CM"), 4)
        assert len(vs) == 3
        assert vs[0].style == VoicingStyle.Close
        assert vs[1].style == VoicingStyle.Open
        assert vs[2].style == VoicingStyle.Shell

    def test_voicing_to_dict(self):
        piano = Piano()
        d = piano.voicing(Chord("CM")).to_dict()
        assert d["chord_name"] == "CM"
        assert len(d["keys"]) == 3


# ---------------------------------------------------------------------------
# scale_keys()
# ---------------------------------------------------------------------------

class TestPianoScaleKeys:
    def test_c_major(self):
        piano = Piano()
        ks = piano.scale_keys(Scale("C", ScaleType.Major), 4)
        assert len(ks) == 7
        assert ks[0].midi == 60  # C4
        assert ks[-1].midi == 71  # B4
        # All white keys in C major
        assert all(k.white for k in ks)

    def test_c_major_pentatonic(self):
        piano = Piano()
        ks = piano.scale_keys(Scale("C", "major pentatonic"), 4)
        assert len(ks) == 5


# ---------------------------------------------------------------------------
# Reverse: note_at()
# ---------------------------------------------------------------------------

class TestPianoNoteAt:
    def test_middle_c(self):
        piano = Piano()
        n = piano.note_at(60)
        assert n.name() == "C"

    def test_a440(self):
        piano = Piano()
        n = piano.note_at(69)
        assert n.name() == "A"

    def test_out_of_range_raises(self):
        piano = Piano()
        with pytest.raises(Exception):
            piano.note_at(0)
        with pytest.raises(Exception):
            piano.note_at(127)


# ---------------------------------------------------------------------------
# Reverse: identify()
# ---------------------------------------------------------------------------

class TestPianoIdentify:
    def test_cm(self):
        piano = Piano()
        c = piano.identify([60, 64, 67])
        assert c.name() == "CM"

    def test_am7(self):
        piano = Piano()
        c = piano.identify([69, 72, 76, 79])
        assert c.name() == "Am7"

    def test_with_duplicate_octaves(self):
        piano = Piano()
        c = piano.identify([48, 64, 67, 72])
        assert c.name() == "CM"

    def test_empty_raises(self):
        piano = Piano()
        with pytest.raises(Exception):
            piano.identify([])


# ---------------------------------------------------------------------------
# Display
# ---------------------------------------------------------------------------

class TestPianoDisplay:
    def test_piano_repr(self):
        piano = Piano()
        assert "88" in repr(piano)

    def test_piano_str(self):
        piano = Piano()
        assert "88" in str(piano)

    def test_key_str(self):
        piano = Piano()
        k = piano.key(Note("C"), 4)
        assert str(k) == "C4"

    def test_voicing_str(self):
        piano = Piano()
        v = piano.voicing(Chord("CM"))
        assert "CM" in str(v)
