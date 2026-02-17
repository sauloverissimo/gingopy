"""Tests for MusicXML serialization."""

import os
import pytest
from gingo import (
    MusicXML, Note, Chord, Scale, ScaleType, Field,
    Sequence, Tempo, TimeSignature, NoteEvent, ChordEvent, Rest, Duration,
)


# ---------------------------------------------------------------------------
# Note
# ---------------------------------------------------------------------------

class TestMusicXMLNote:
    def test_basic_note(self):
        xml = MusicXML.note(Note("C"), 4)
        assert "<step>C</step>" in xml
        assert "<octave>4</octave>" in xml
        assert "<type>quarter</type>" in xml

    def test_sharp(self):
        xml = MusicXML.note(Note("C#"), 4)
        assert "<step>C</step>" in xml
        assert "<alter>1</alter>" in xml

    def test_flat(self):
        xml = MusicXML.note(Note("Bb"), 4)
        assert "<step>B</step>" in xml
        assert "<alter>-1</alter>" in xml

    def test_natural_no_alter(self):
        xml = MusicXML.note(Note("G"), 5)
        assert "<alter>" not in xml

    def test_whole_type(self):
        xml = MusicXML.note(Note("E"), 3, "whole")
        assert "<type>whole</type>" in xml

    def test_document_structure(self):
        xml = MusicXML.note(Note("D"), 4)
        assert '<?xml version=' in xml
        assert '<!DOCTYPE score-partwise' in xml
        assert '<score-partwise' in xml
        assert '</score-partwise>' in xml
        assert '<software>Gingo</software>' in xml
        assert '<part-name>Piano</part-name>' in xml


# ---------------------------------------------------------------------------
# Chord
# ---------------------------------------------------------------------------

class TestMusicXMLChord:
    def test_cm_three_notes(self):
        xml = MusicXML.chord(Chord("CM"), 4)
        assert xml.count("<note>") == 3

    def test_cm_chord_tags(self):
        xml = MusicXML.chord(Chord("CM"), 4)
        # 2nd and 3rd notes get <chord/>
        assert xml.count("<chord/>") == 2

    def test_am7_four_notes(self):
        xml = MusicXML.chord(Chord("Am7"), 4)
        assert xml.count("<note>") == 4


# ---------------------------------------------------------------------------
# Scale
# ---------------------------------------------------------------------------

class TestMusicXMLScale:
    def test_c_major_seven_notes(self):
        xml = MusicXML.scale(Scale("C", ScaleType.Major), 4)
        assert xml.count("<note>") == 7

    def test_no_chord_tags(self):
        xml = MusicXML.scale(Scale("C", ScaleType.Major), 4)
        assert "<chord/>" not in xml


# ---------------------------------------------------------------------------
# Field
# ---------------------------------------------------------------------------

class TestMusicXMLField:
    def test_seven_measures(self):
        xml = MusicXML.field(Field("C", ScaleType.Major), 4)
        assert xml.count("<measure number=") == 7


# ---------------------------------------------------------------------------
# Sequence
# ---------------------------------------------------------------------------

class TestMusicXMLSequence:
    def test_basic_sequence(self):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))
        seq.add(NoteEvent(Note("E"), Duration("quarter"), 4))
        seq.add(Rest(Duration("half")))

        xml = MusicXML.sequence(seq)
        assert "<step>C</step>" in xml
        assert "<step>E</step>" in xml
        assert "<rest/>" in xml

    def test_chord_event(self):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(ChordEvent(Chord("CM"), Duration("whole"), 4))

        xml = MusicXML.sequence(seq)
        assert xml.count("<note>") == 3
        assert xml.count("<chord/>") == 2

    def test_tempo_marking(self):
        seq = Sequence(Tempo(120), TimeSignature(4, 4))
        seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))

        xml = MusicXML.sequence(seq)
        assert "<per-minute>120</per-minute>" in xml


# ---------------------------------------------------------------------------
# Write
# ---------------------------------------------------------------------------

class TestMusicXMLWrite:
    def test_write_file(self, tmp_path):
        xml = MusicXML.note(Note("C"), 4)
        path = str(tmp_path / "test.musicxml")
        MusicXML.write(xml, path)
        assert os.path.exists(path)
        with open(path) as f:
            content = f.read()
        assert "<step>C</step>" in content

    def test_write_chord(self, tmp_path):
        xml = MusicXML.chord(Chord("Am7"), 4)
        path = str(tmp_path / "chord.musicxml")
        MusicXML.write(xml, path)
        assert os.path.exists(path)
