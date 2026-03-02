# Gingo — Rhythm tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import (
    Duration, Tempo, TimeSignature,
    NoteEvent, ChordEvent, Rest, Sequence,
    Note, Chord,
)


# ===========================================================================
# Duration
# ===========================================================================

class TestDuration:
    def test_quarter(self):
        d = Duration("quarter")
        assert d.name() == "quarter"
        assert d.beats() == pytest.approx(1.0)
        assert d.numerator() == 1
        assert d.denominator() == 4

    def test_whole(self):
        assert Duration("whole").beats() == pytest.approx(4.0)

    def test_half(self):
        assert Duration("half").beats() == pytest.approx(2.0)

    def test_eighth(self):
        assert Duration("eighth").beats() == pytest.approx(0.5)

    def test_sixteenth(self):
        assert Duration("sixteenth").beats() == pytest.approx(0.25)

    def test_from_rational(self):
        d = Duration(1, 4)
        assert d.name() == "quarter"
        assert d.beats() == pytest.approx(1.0)

    def test_dotted(self):
        d = Duration("quarter", dots=1)
        assert d.beats() == pytest.approx(1.5)
        assert d.numerator() == 3
        assert d.denominator() == 8

    def test_double_dotted(self):
        d = Duration("quarter", dots=2)
        assert d.beats() == pytest.approx(1.75)

    def test_triplet(self):
        d = Duration("eighth", tuplet=3)
        assert d.beats() == pytest.approx(1.0 / 3.0)

    def test_addition(self):
        q = Duration("quarter")
        e = Duration("eighth")
        s = q + e
        assert s.beats() == pytest.approx(1.5)

    def test_comparison(self):
        q = Duration("quarter")
        h = Duration("half")
        assert q == Duration("quarter")
        assert q != h
        assert q < h

    def test_standard_names(self):
        names = Duration.standard_names()
        assert len(names) == 7
        assert "quarter" in names

    def test_invalid_name(self):
        with pytest.raises(ValueError):
            Duration("invalid")

    def test_rational(self):
        d = Duration("quarter")
        assert d.rational() == (1, 4)

    def test_str_repr(self):
        d = Duration("quarter")
        assert str(d) == "quarter"
        assert "quarter" in repr(d)

    def test_abbreviation(self):
        assert Duration("q").name() == "quarter"
        assert Duration("h").name() == "half"
        assert Duration("w").name() == "whole"
        assert Duration("e").name() == "eighth"
        assert Duration("s").name() == "sixteenth"
        assert Duration("q").beats() == pytest.approx(1.0)

    def test_dotted_abbreviation(self):
        d = Duration("q.")
        assert d.name() == "quarter"
        assert d.dots() == 1
        assert d.beats() == pytest.approx(1.5)

        d2 = Duration("h..")
        assert d2.name() == "half"
        assert d2.dots() == 2
        assert d2.beats() == pytest.approx(3.5)

    def test_lilypond_notation(self):
        assert Duration("4").name() == "quarter"
        assert Duration("8").name() == "eighth"
        assert Duration("2").name() == "half"
        assert Duration("1").name() == "whole"
        assert Duration("16").name() == "sixteenth"
        assert Duration("32").name() == "thirty_second"
        assert Duration("64").name() == "sixty_fourth"

    def test_dotted_lilypond(self):
        d = Duration("4.")
        assert d.name() == "quarter"
        assert d.dots() == 1
        assert d.beats() == pytest.approx(1.5)

    def test_fraction_string(self):
        d = Duration("1/4")
        assert d.numerator() == 1
        assert d.denominator() == 4
        assert d.name() == "quarter"

        d2 = Duration("3/8")
        assert d2.numerator() == 3
        assert d2.denominator() == 8
        assert d2.beats() == pytest.approx(1.5)

    def test_explicit_dots_override(self):
        d = Duration("q", dots=1)
        assert d.dots() == 1
        assert d.beats() == pytest.approx(1.5)

        d2 = Duration("q.", dots=2)
        assert d2.dots() == 2
        assert d2.beats() == pytest.approx(1.75)

    def test_midi_ticks(self):
        assert Duration("quarter").midi_ticks() == 480
        assert Duration("half").midi_ticks() == 960
        assert Duration("whole").midi_ticks() == 1920
        assert Duration("eighth").midi_ticks() == 240
        assert Duration("quarter", dots=1).midi_ticks() == 720
        # Custom ppqn
        assert Duration("quarter").midi_ticks(96) == 96

    def test_from_ticks(self):
        d = Duration.from_ticks(480, 480)
        assert d.name() == "quarter"
        assert d.numerator() == 1
        assert d.denominator() == 4

        d2 = Duration.from_ticks(960, 480)
        assert d2.name() == "half"

        d3 = Duration.from_ticks(720, 480)
        assert d3.beats() == pytest.approx(1.5)


# ===========================================================================
# Tempo
# ===========================================================================

class TestTempo:
    def test_from_bpm(self):
        t = Tempo(120)
        assert t.bpm() == pytest.approx(120.0)

    def test_from_marking(self):
        t = Tempo("Allegro")
        assert t.bpm() > 100

    def test_marking(self):
        assert Tempo(60).marking() == "Adagio"
        assert Tempo(140).marking() == "Allegro"

    def test_seconds(self):
        t = Tempo(120)
        q = Duration("quarter")
        assert t.seconds(q) == pytest.approx(0.5)

    def test_ms_per_beat(self):
        assert Tempo(120).ms_per_beat() == pytest.approx(500.0)

    def test_invalid_bpm(self):
        with pytest.raises(ValueError):
            Tempo(0)

    def test_float_int(self):
        t = Tempo(120)
        assert float(t) == pytest.approx(120.0)
        assert int(t) == 120

    def test_str(self):
        assert "120" in str(Tempo(120))

    def test_static_conversions(self):
        assert Tempo.bpm_to_marking(140) == "Allegro"
        assert Tempo.marking_to_bpm("Allegro") > 0

    def test_microseconds_per_beat(self):
        assert Tempo(120).microseconds_per_beat() == 500000
        assert Tempo(60).microseconds_per_beat() == 1000000

    def test_from_microseconds(self):
        t = Tempo.from_microseconds(500000)
        assert t.bpm() == pytest.approx(120.0)

        t2 = Tempo.from_microseconds(1000000)
        assert t2.bpm() == pytest.approx(60.0)

        with pytest.raises(ValueError):
            Tempo.from_microseconds(0)


# ===========================================================================
# TimeSignature
# ===========================================================================

class TestTimeSignature:
    def test_common_time(self):
        ts = TimeSignature(4, 4)
        assert ts.beats_per_bar() == 4
        assert ts.beat_unit() == 4
        assert ts.classification() == "simple"
        assert ts.common_name() == "common time"

    def test_compound(self):
        ts = TimeSignature(6, 8)
        assert ts.classification() == "compound"

    def test_cut_time(self):
        ts = TimeSignature(2, 2)
        assert ts.common_name() == "cut time"

    def test_bar_duration(self):
        ts = TimeSignature(3, 4)
        assert ts.bar_duration().beats() == pytest.approx(3.0)

    def test_signature(self):
        ts = TimeSignature(4, 4)
        assert ts.signature() == (4, 4)

    def test_equality(self):
        assert TimeSignature(4, 4) == TimeSignature(4, 4)
        assert TimeSignature(3, 4) != TimeSignature(4, 4)

    def test_str(self):
        assert str(TimeSignature(4, 4)) == "4/4"

    def test_invalid(self):
        with pytest.raises(ValueError):
            TimeSignature(4, 3)


# ===========================================================================
# NoteEvent
# ===========================================================================

class TestNoteEvent:
    def test_construction(self):
        e = NoteEvent(Note("C"), Duration("quarter"))
        assert e.note().name() == "C"
        assert e.octave() == 4
        assert e.duration().name() == "quarter"

    def test_midi_number(self):
        assert NoteEvent(Note("C"), Duration("quarter"), 4).midi_number() == 60
        assert NoteEvent(Note("A"), Duration("quarter"), 4).midi_number() == 69

    def test_frequency(self):
        e = NoteEvent(Note("A"), Duration("quarter"), 4)
        assert e.frequency() == pytest.approx(440.0, abs=0.01)

    def test_str(self):
        e = NoteEvent(Note("C"), Duration("quarter"), 4)
        assert str(e) == "C4"


# ===========================================================================
# ChordEvent
# ===========================================================================

class TestChordEvent:
    def test_construction(self):
        e = ChordEvent(Chord("CM"), Duration("whole"))
        assert e.chord().name() == "CM"
        assert e.duration().name() == "whole"

    def test_note_events(self):
        e = ChordEvent(Chord("CM"), Duration("quarter"), 4)
        notes = e.note_events()
        assert len(notes) == 3

    def test_str(self):
        e = ChordEvent(Chord("Am7"), Duration("half"))
        assert str(e) == "Am7"


# ===========================================================================
# Rest
# ===========================================================================

class TestRest:
    def test_construction(self):
        r = Rest(Duration("quarter"))
        assert r.duration().name() == "quarter"

    def test_equality(self):
        assert Rest(Duration("quarter")) == Rest(Duration("quarter"))
        assert Rest(Duration("quarter")) != Rest(Duration("half"))

    def test_str(self):
        assert str(Rest(Duration("quarter"))) == "Rest"


# ===========================================================================
# Sequence
# ===========================================================================

class TestSequence:
    def test_empty(self):
        seq = Sequence()
        assert len(seq) == 0

    def test_add_notes(self):
        seq = Sequence()
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        seq.add(NoteEvent(Note("E"), Duration("quarter")))
        seq.add(Rest(Duration("half")))
        assert len(seq) == 3
        assert seq.total_duration() == pytest.approx(4.0)

    def test_total_seconds(self):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        seq.add(NoteEvent(Note("E"), Duration("quarter")))
        assert seq.total_seconds() == pytest.approx(1.0)

    def test_bar_count(self):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        for n in ["C", "D", "E", "F"]:
            seq.add(NoteEvent(Note(n), Duration("quarter")))
        assert seq.bar_count() == 1

    def test_transpose(self):
        seq = Sequence()
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        t = seq.transpose(7)
        assert len(t) == 1

    def test_remove(self):
        seq = Sequence()
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        seq.add(NoteEvent(Note("E"), Duration("quarter")))
        seq.remove(0)
        assert len(seq) == 1

    def test_clear(self):
        seq = Sequence()
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        seq.clear()
        assert len(seq) == 0

    def test_with_chords(self):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(ChordEvent(Chord("CM"), Duration("half")))
        seq.add(ChordEvent(Chord("GM"), Duration("half")))
        assert len(seq) == 2
        assert seq.total_duration() == pytest.approx(4.0)
        assert seq.bar_count() == 1

    def test_tempo_and_time_signature(self):
        seq = Sequence(Tempo(90), TimeSignature(3, 4))
        assert seq.tempo().bpm() == pytest.approx(90.0)
        assert seq.time_signature().beats_per_bar() == 3

    def test_set_tempo(self):
        seq = Sequence()
        seq.set_tempo(Tempo(60))
        assert seq.tempo().bpm() == pytest.approx(60.0)

    def test_str(self):
        seq = Sequence()
        seq.add(NoteEvent(Note("C"), Duration("quarter")))
        assert "1 events" in str(seq)
