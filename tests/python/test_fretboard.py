"""Tests for the Fretboard class."""

import pytest
from gingo import (
    Fretboard, FretPosition, StringAction, StringState, Fingering, Tuning,
    Note, Chord, Scale, ScaleType,
)


# ---------------------------------------------------------------------------
# Factories
# ---------------------------------------------------------------------------

class TestFretboardFactories:
    def test_violao_strings_and_frets(self):
        g = Fretboard.violao()
        assert g.num_strings() == 6
        assert g.num_frets() == 19
        assert g.name() == "violao"

    def test_cavaquinho_strings_and_frets(self):
        c = Fretboard.cavaquinho()
        assert c.num_strings() == 4
        assert c.num_frets() == 17
        assert c.name() == "cavaquinho"

    def test_bandolim_strings_and_frets(self):
        b = Fretboard.bandolim()
        assert b.num_strings() == 4
        assert b.num_frets() == 17
        assert b.name() == "bandolim"

    def test_custom_tuning(self):
        fb = Fretboard("test", [64, 59, 55], 12)
        assert fb.num_strings() == 3
        assert fb.num_frets() == 12
        assert fb.name() == "test"


# ---------------------------------------------------------------------------
# Open strings
# ---------------------------------------------------------------------------

class TestFretboardOpenStrings:
    def test_violao_string_1_is_E4(self):
        g = Fretboard.violao()
        assert g.midi_at(1, 0) == 64
        assert g.note_at(1, 0).name() == "E"

    def test_violao_string_6_is_E2(self):
        g = Fretboard.violao()
        assert g.midi_at(6, 0) == 40
        assert g.note_at(6, 0).name() == "E"

    def test_cavaquinho_string_1_is_D5(self):
        c = Fretboard.cavaquinho()
        assert c.midi_at(1, 0) == 74
        assert c.note_at(1, 0).name() == "D"

    def test_bandolim_string_1_is_E5(self):
        b = Fretboard.bandolim()
        assert b.midi_at(1, 0) == 76
        assert b.note_at(1, 0).name() == "E"


# ---------------------------------------------------------------------------
# Position
# ---------------------------------------------------------------------------

class TestFretboardPosition:
    def test_position_returns_correct_data(self):
        g = Fretboard.violao()
        p = g.position(1, 5)
        assert p.string == 1
        assert p.fret == 5
        assert p.midi == 69
        assert p.note == "A"
        assert p.octave == 4

    def test_fret_12_is_octave_higher(self):
        g = Fretboard.violao()
        p0 = g.position(1, 0)
        p12 = g.position(1, 12)
        assert p12.midi == p0.midi + 12
        assert p12.note == p0.note


# ---------------------------------------------------------------------------
# Positions (all occurrences)
# ---------------------------------------------------------------------------

class TestFretboardPositions:
    def test_finds_note_on_multiple_strings(self):
        g = Fretboard.violao()
        pos = g.positions(Note("E"))
        assert len(pos) >= 6

    def test_all_positions_have_correct_pitch_class(self):
        g = Fretboard.violao()
        for p in g.positions(Note("C")):
            assert p.note == "C"


# ---------------------------------------------------------------------------
# Scale positions
# ---------------------------------------------------------------------------

class TestFretboardScalePositions:
    def test_c_major_has_many_positions(self):
        g = Fretboard.violao()
        pos = g.scale_positions(Scale("C", ScaleType.Major))
        assert len(pos) > 20

    def test_fret_range_respected(self):
        g = Fretboard.violao()
        pos = g.scale_positions(Scale("C", ScaleType.Major), 0, 4)
        for p in pos:
            assert 0 <= p.fret <= 4


# ---------------------------------------------------------------------------
# Fingering
# ---------------------------------------------------------------------------

class TestFretboardFingering:
    def test_cm_on_violao(self):
        g = Fretboard.violao()
        f = g.fingering(Chord("CM"))
        assert f.chord_name == "CM"
        assert len(f.strings) > 0
        assert len(f.midi_notes) > 0

    def test_covers_all_chord_notes(self):
        g = Fretboard.violao()
        f = g.fingering(Chord("CM"))
        pcs = {m % 12 for m in f.midi_notes}
        assert 0 in pcs  # C
        assert 4 in pcs  # E
        assert 7 in pcs  # G

    def test_fingerings_returns_multiple(self):
        g = Fretboard.violao()
        fs = g.fingerings(Chord("CM"), 3)
        assert len(fs) >= 2

    def test_am_on_cavaquinho(self):
        c = Fretboard.cavaquinho()
        f = c.fingering(Chord("Am"))
        assert f.chord_name == "Am"


# ---------------------------------------------------------------------------
# Identify
# ---------------------------------------------------------------------------

class TestFretboardIdentify:
    def test_identify_cm_from_open_position(self):
        g = Fretboard.violao()
        chord = g.identify([(5, 3), (4, 2), (3, 0), (2, 1), (1, 0)])
        assert chord.name() == "CM"


# ---------------------------------------------------------------------------
# Capo
# ---------------------------------------------------------------------------

class TestFretboardCapo:
    def test_capo_raises_pitch(self):
        g = Fretboard.violao()
        gc = g.capo(2)
        assert gc.midi_at(1, 0) == 66
        assert gc.note_at(1, 0).name() == "F#"

    def test_capo_reduces_frets(self):
        g = Fretboard.violao()
        gc = g.capo(2)
        assert gc.num_frets() == 17

    def test_capo_name_includes_info(self):
        g = Fretboard.violao()
        gc = g.capo(3)
        assert "capo" in gc.name()


# ---------------------------------------------------------------------------
# Tuning
# ---------------------------------------------------------------------------

class TestTuning:
    def test_tuning_accessible(self):
        g = Fretboard.violao()
        t = g.tuning()
        assert t.name == "violao"
        assert len(t.open_midi) == 6
        assert t.num_frets == 19


# ---------------------------------------------------------------------------
# Display
# ---------------------------------------------------------------------------

class TestFretboardDisplay:
    def test_repr(self):
        g = Fretboard.violao()
        assert "violao" in repr(g)

    def test_str(self):
        g = Fretboard.violao()
        assert "Fretboard" in str(g)

    def test_fret_position_repr(self):
        g = Fretboard.violao()
        p = g.position(1, 0)
        assert "E" in repr(p)

    def test_fingering_repr(self):
        g = Fretboard.violao()
        f = g.fingering(Chord("CM"))
        assert "CM" in repr(f)


# ---------------------------------------------------------------------------
# Edge cases
# ---------------------------------------------------------------------------

class TestFretboardEdgeCases:
    def test_invalid_string_raises(self):
        g = Fretboard.violao()
        with pytest.raises(Exception):
            g.position(0, 0)
        with pytest.raises(Exception):
            g.position(7, 0)

    def test_invalid_fret_raises(self):
        g = Fretboard.violao()
        with pytest.raises(Exception):
            g.position(1, -1)
        with pytest.raises(Exception):
            g.position(1, 20)
