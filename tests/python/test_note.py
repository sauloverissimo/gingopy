# Gingo — Note tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import Note


def test_basic_construction():
    n = Note("C")
    assert n.name() == "C"
    assert n.natural() == "C"
    assert n.sound() == "C"
    assert n.semitone() == 0


def test_enharmonic_resolution():
    assert Note("Bb").natural() == "A#"
    assert Note("Db").natural() == "C#"
    assert Note("Eb").natural() == "D#"
    assert Note("Gb").natural() == "F#"
    assert Note("Ab").natural() == "G#"


def test_double_accidentals():
    assert Note("G##").natural() == "A"
    assert Note("Bbb").natural() == "A"
    assert Note("Abb").natural() == "G"


def test_special_enharmonics():
    assert Note("E#").natural() == "F"
    assert Note("B#").natural() == "C"
    assert Note("Fb").natural() == "E"
    assert Note("Cb").natural() == "B"


def test_frequency():
    assert abs(Note("A").frequency(4) - 440.0) < 0.01
    assert abs(Note("C").frequency(4) - 261.63) < 0.1


def test_enharmonic_check():
    assert Note("Bb").is_enharmonic(Note("A#"))
    assert not Note("C").is_enharmonic(Note("D"))


def test_equality():
    assert Note("C") == Note("C")
    assert Note("Bb") == Note("A#")
    assert Note("C") != Note("D")


def test_transpose():
    assert Note("C").transpose(7).natural() == "G"
    assert Note("C").transpose(12).natural() == "C"


def test_string_repr():
    n = Note("C")
    assert "C" in str(n)
    assert "Note" in repr(n)


# ---------------------------------------------------------------------------
# Circle of fifths
# ---------------------------------------------------------------------------


def test_fifths_returns_12_notes():
    f = Note.fifths()
    assert len(f) == 12
    assert f[0] == "C"
    assert f[1] == "G"
    assert f[5] == "B"
    assert f[6] == "F#"
    assert f[11] == "F"


def test_distance_same_note():
    assert Note("C").distance(Note("C")) == 0


def test_distance_perfect_fifth():
    assert Note("C").distance(Note("G")) == 1


def test_distance_perfect_fourth():
    assert Note("C").distance(Note("F")) == 1


def test_distance_two_fifths():
    assert Note("C").distance(Note("D")) == 2


def test_distance_tritone_max():
    assert Note("C").distance(Note("F#")) == 6


def test_distance_enharmonic():
    assert Note("Bb").distance(Note("A#")) == 0


def test_distance_symmetry():
    assert Note("C").distance(Note("Bb")) == Note("Bb").distance(Note("C"))
