# Gingo — Field tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import Field, ScaleType, Chord, HarmonicFunction


def test_field_construction():
    f = Field("C", ScaleType.Major)
    assert f.size() >= 7


def test_field_triads():
    f = Field("C", ScaleType.Major)
    triads = f.chords()
    assert len(triads) >= 7
    # All returned elements should be Chord objects
    assert all(isinstance(c, Chord) for c in triads)


def test_field_sevenths():
    f = Field("C", ScaleType.Major)
    sevenths = f.sevenths()
    assert len(sevenths) >= 7
    assert all(isinstance(c, Chord) for c in sevenths)


def test_field_chord():
    f = Field("C", ScaleType.Major)
    first = f.chord(1)
    assert isinstance(first, Chord)


def test_field_chord_out_of_range():
    f = Field("C", ScaleType.Major)
    with pytest.raises(IndexError):
        f.chord(0)
    with pytest.raises(IndexError):
        f.chord(100)


def test_field_string_name():
    f = Field("C", ScaleType.Major)
    assert "Field" in repr(f) or "C" in str(f)


# ---------------------------------------------------------------------------
# Applied chords (tonicization)
# ---------------------------------------------------------------------------

def test_applied_string_int():
    f = Field("C", ScaleType.Major)
    # V7/II = A7
    c = f.applied("V7", 2)
    assert c.root().natural() == "A"
    assert c.type() == "7"
    # V7/IV = C7
    c = f.applied("V7", 4)
    assert c.root().natural() == "C"
    assert c.type() == "7"
    # V7/V = D7
    c = f.applied("V7", 5)
    assert c.root().natural() == "D"
    assert c.type() == "7"


def test_applied_complex_quality():
    f = Field("C", ScaleType.Major)
    # IIm7(b5)/V = Am7(b5)
    c = f.applied("IIm7(b5)", 5)
    assert c.root().natural() == "A"
    assert c.type() == "m7(b5)"


def test_applied_no_quality():
    f = Field("C", ScaleType.Major)
    # V/II = triad at V of D major = A
    c = f.applied("V", 2)
    assert c.root().natural() == "A"


def test_applied_accidental():
    f = Field("C", ScaleType.Major)
    # bVII7/V = F7
    c = f.applied("bVII7", 5)
    assert c.root().natural() == "F"
    assert c.type() == "7"
    # #IV7/I = F#7
    c = f.applied("#IV7", 1)
    assert c.root().natural() == "F#"
    assert c.type() == "7"


def test_applied_string_string():
    f = Field("C", ScaleType.Major)
    # V7 / IIm = A7
    c = f.applied("V7", "IIm")
    assert c.root().natural() == "A"
    assert c.type() == "7"
    # V7 / V = D7
    c = f.applied("V7", "V")
    assert c.root().natural() == "D"
    assert c.type() == "7"


def test_applied_numeric():
    f = Field("C", ScaleType.Major)
    # applied(5, 2) = seventh at V of D major = A7
    c = f.applied(5, 2)
    assert c.root().natural() == "A"
    assert c.type() == "7"
    # applied(5, 5) = D7
    c = f.applied(5, 5)
    assert c.root().natural() == "D"
    assert c.type() == "7"


def test_applied_out_of_range():
    f = Field("C", ScaleType.Major)
    with pytest.raises(IndexError):
        f.applied("V7", 0)
    with pytest.raises(IndexError):
        f.applied("V7", 8)
    with pytest.raises(IndexError):
        f.applied(5, 0)


def test_applied_invalid_roman():
    f = Field("C", ScaleType.Major)
    with pytest.raises(ValueError):
        f.applied("X7", 2)
    with pytest.raises(ValueError):
        f.applied("", 2)


# ---------------------------------------------------------------------------
# Harmonic function and role
# ---------------------------------------------------------------------------

def test_function_by_degree():
    f = Field("C", ScaleType.Major)

    assert f.function(1) == HarmonicFunction.Tonic
    assert f.function(2) == HarmonicFunction.Subdominant
    assert f.function(3) == HarmonicFunction.Tonic
    assert f.function(4) == HarmonicFunction.Subdominant
    assert f.function(5) == HarmonicFunction.Dominant
    assert f.function(6) == HarmonicFunction.Tonic
    assert f.function(7) == HarmonicFunction.Dominant


def test_function_name_and_short():
    assert HarmonicFunction.Tonic.name == "Tonic"
    assert HarmonicFunction.Subdominant.name == "Subdominant"
    assert HarmonicFunction.Dominant.name == "Dominant"

    assert HarmonicFunction.Tonic.short == "T"
    assert HarmonicFunction.Subdominant.short == "S"
    assert HarmonicFunction.Dominant.short == "D"


def test_role_by_degree():
    f = Field("C", ScaleType.Major)

    assert f.role(1) == "primary"
    assert f.role(2) == "relative of IV"
    assert f.role(3) == "transitive"
    assert f.role(4) == "primary"
    assert f.role(5) == "primary"
    assert f.role(6) == "relative of I"
    assert f.role(7) == "relative of V"


def test_function_by_chord_name():
    f = Field("C", ScaleType.Major)

    # FM is degree IV → Subdominant
    result = f.function("FM")
    assert result == HarmonicFunction.Subdominant

    # Am is degree VI → Tonic
    result = f.function("Am")
    assert result == HarmonicFunction.Tonic

    # GM is degree V → Dominant
    result = f.function("GM")
    assert result == HarmonicFunction.Dominant


def test_function_by_chord_object():
    f = Field("C", ScaleType.Major)
    chord = Chord("Dm")

    result = f.function(chord)
    assert result == HarmonicFunction.Subdominant


def test_function_returns_none_for_non_field_chord():
    f = Field("C", ScaleType.Major)

    # F#M does not belong to C major
    result = f.function("F#M")
    assert result is None


def test_role_by_chord_name():
    f = Field("C", ScaleType.Major)

    result = f.role("Am")
    assert result == "relative of I"

    result = f.role("FM")
    assert result == "primary"

    # Non-field chord
    result = f.role("F#M")
    assert result is None


def test_function_out_of_range():
    f = Field("C", ScaleType.Major)
    with pytest.raises(IndexError):
        f.function(0)
    with pytest.raises(IndexError):
        f.function(8)
    with pytest.raises(IndexError):
        f.role(0)


def test_function_natural_minor():
    f = Field("A", ScaleType.NaturalMinor)

    assert f.function(1) == HarmonicFunction.Tonic
    assert f.function(3) == HarmonicFunction.Tonic
    assert f.function(5) == HarmonicFunction.Dominant
    assert f.function(6) == HarmonicFunction.Subdominant
    assert f.function(7) == HarmonicFunction.Dominant


# ---------------------------------------------------------------------------
# Circle of fifths navigation
# ---------------------------------------------------------------------------


def test_field_signature():
    assert Field("C", "major").signature() == 0
    assert Field("G", "major").signature() == 1
    assert Field("D", "major").signature() == 2
    assert Field("F", "major").signature() == -1
    assert Field("Bb", "major").signature() == -2
    assert Field("A", "natural minor").signature() == 0


def test_field_relative_major():
    rel = Field("C", "major").relative()
    assert rel.tonic().natural() == "A"
    assert len(rel.chords()) == 7


def test_field_relative_minor():
    rel = Field("A", "natural minor").relative()
    assert rel.tonic().natural() == "C"
    assert len(rel.chords()) == 7


def test_field_relative_g_major():
    rel = Field("G", "major").relative()
    assert rel.tonic().natural() == "E"


def test_field_parallel_major():
    f = Field("C", "major")
    par = f.parallel()
    assert par.tonic().natural() == "C"
    assert len(par.chords()) == 7


def test_field_parallel_minor():
    par = Field("C", "natural minor").parallel()
    assert par.tonic().natural() == "C"


def test_field_neighbors_c_major():
    sub, dom = Field("C", "major").neighbors()
    assert sub.tonic().natural() == "F"
    assert dom.tonic().natural() == "G"
    assert len(sub.chords()) == 7
    assert len(dom.chords()) == 7


def test_field_neighbors_a_minor():
    sub, dom = Field("A", "natural minor").neighbors()
    assert sub.tonic().natural() == "D"
    assert dom.tonic().natural() == "E"
