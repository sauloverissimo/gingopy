# Gingo — Chord tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import Chord, Note


def test_major_triad():
    c = Chord("CM")
    assert c.root().natural() == "C"
    assert c.type() == "M"
    names = [n.natural() for n in c.notes()]
    assert names == ["C", "E", "G"]


def test_minor_seventh():
    c = Chord("Am7")
    assert c.size() == 4
    assert c.interval_labels() == ["P1", "3m", "5J", "7m"]


def test_flat_root():
    c = Chord("Dbm7")
    names = [n.natural() for n in c.notes()]
    assert names == ["C#", "E", "G#", "B"]


def test_dominant_seventh():
    c = Chord("G7")
    assert c.type() == "7"
    assert c.size() == 4


def test_contains():
    c = Chord("CM")
    assert c.contains(Note("E"))
    assert not c.contains(Note("F"))


def test_identify_from_strings():
    c = Chord.identify(["C", "E", "G"])
    assert c.root().natural() == "C"
    assert "M" in c.type() or c.type() == "M"


def test_equality():
    assert Chord("CM") == Chord("CM")
    assert Chord("CM") != Chord("Cm")


def test_string_repr():
    c = Chord("Cm7")
    assert "Cm7" in str(c)
