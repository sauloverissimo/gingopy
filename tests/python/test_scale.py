# Gingo — Scale tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import Scale, ScaleType, Modality, Note


def test_major_scale_notes():
    s = Scale("C", ScaleType.Major)
    names = [n.natural() for n in s.notes()]
    assert names[:7] == ["C", "D", "E", "F", "G", "A", "B"]


def test_natural_minor_scale():
    s = Scale("A", ScaleType.NaturalMinor)
    names = [n.natural() for n in s.notes()]
    assert names[:7] == ["A", "B", "C", "D", "E", "F", "G"]


def test_harmonic_minor_scale():
    s = Scale("A", ScaleType.HarmonicMinor)
    names = [n.natural() for n in s.notes()]
    # Harmonic minor raises the 7th degree: G -> G#
    assert "G#" in names


def test_scale_degree():
    s = Scale("C", ScaleType.Major)
    assert s.degree(1).natural() == "C"
    assert s.degree(5).natural() == "G"


def test_scale_degree_out_of_range():
    s = Scale("C", ScaleType.Major)
    with pytest.raises(IndexError):
        s.degree(0)
    with pytest.raises(IndexError):
        s.degree(100)


def test_degree_varargs():
    """degree(5, 5) = grau V do grau V = D."""
    s = Scale("C", ScaleType.Major)
    assert s.degree(5).natural() == "G"           # retrocompativel
    assert s.degree(5, 5).natural() == "D"        # grau V do grau V
    assert s.degree(5, 5, 3).natural() == "F"     # encadeado


def test_degree_varargs_wraps():
    s = Scale("C", ScaleType.Major)
    assert s.degree(7, 2).natural() == "C"        # VII→II wraps → I
    assert s.degree(4, 5).natural() == "C"        # IV→V wraps → I


def test_degree_varargs_out_of_range():
    s = Scale("C", ScaleType.Major)
    with pytest.raises(IndexError):
        s.degree(0, 5)
    with pytest.raises(IndexError):
        s.degree(100)


def test_walk():
    """walk(start, *steps) navega pela escala."""
    s = Scale("C", ScaleType.Major)
    assert s.walk(1, 4).natural() == "F"          # I→IV = F
    assert s.walk(5, 5).natural() == "D"          # V→V = II = D


def test_walk_multistep():
    s = Scale("C", ScaleType.Major)
    # from III, third (→V), then second (→VI)
    assert s.walk(3, 3, 2).natural() == "A"


def test_walk_negative():
    s = Scale("C", ScaleType.Major)
    result = s.walk(5, -2)
    assert result.natural() == "D"


def test_walk_wraps():
    s = Scale("C", ScaleType.Major)
    assert s.walk(7, 2).natural() == "C"          # VII→II wraps → I


def test_walk_validation():
    s = Scale("C", ScaleType.Major)
    with pytest.raises((ValueError, IndexError)):
        s.walk(1)                                  # needs at least 1 step
    with pytest.raises(IndexError):
        s.walk(0, 5)                               # invalid start


def test_scale_size():
    s = Scale("C", ScaleType.Major)
    assert s.size() >= 7


def test_scale_contains():
    s = Scale("C", ScaleType.Major)
    assert s.contains(Note("G"))


def test_parse_type():
    assert Scale.parse_type("major") == ScaleType.Major
    assert Scale.parse_type("minor") == ScaleType.HarmonicMinor
    assert Scale.parse_type("dim") == ScaleType.Diminished


def test_parse_modality():
    assert Scale.parse_modality("diatonic") == Modality.Diatonic
    assert Scale.parse_modality("pentatonic") == Modality.Pentatonic


def test_mask():
    s = Scale("C", ScaleType.Major)
    mask = s.mask()
    assert isinstance(mask, list)
    assert len(mask) == 24
    assert mask[0] == 1  # Root is always present


def test_string_repr():
    s = Scale("C", ScaleType.Major)
    assert "C" in str(s)


def test_mode():
    s = Scale("C", ScaleType.Major)
    mode2 = s.mode(2)
    assert mode2.degree(1).natural() == "D"


# ---------------------------------------------------------------------------
# New scale hierarchy tests
# ---------------------------------------------------------------------------


def test_parse_new_scale_types():
    assert Scale.parse_type("harmonic major") == ScaleType.HarmonicMajor
    assert Scale.parse_type("whole tone") == ScaleType.WholeTone
    assert Scale.parse_type("wholetone") == ScaleType.WholeTone
    assert Scale.parse_type("augmented") == ScaleType.Augmented
    assert Scale.parse_type("aug") == ScaleType.Augmented
    assert Scale.parse_type("blues") == ScaleType.Blues
    assert Scale.parse_type("chromatic") == ScaleType.Chromatic


def test_new_scale_type_construction():
    wt = Scale("C", ScaleType.WholeTone)
    assert wt.size() == 6

    bl = Scale("A", ScaleType.Blues)
    assert bl.size() == 6

    ch = Scale("C", ScaleType.Chromatic)
    assert ch.size() == 12

    dim = Scale("C", ScaleType.Diminished)
    assert dim.size() == 8


def test_construct_by_mode_name():
    d = Scale("D", "dorian")
    assert d.parent() == ScaleType.Major
    assert d.mode_number() == 2
    assert d.tonic().natural() == "D"
    notes = [n.natural() for n in d.notes()]
    assert notes == ["D", "E", "F", "G", "A", "B", "C"]


def test_construct_altered_scale():
    a = Scale("C", "altered")
    assert a.parent() == ScaleType.MelodicMinor
    assert a.mode_number() == 7


def test_construct_phrygian_dominant():
    pd = Scale("E", "phrygian dominant")
    assert pd.parent() == ScaleType.HarmonicMinor
    assert pd.mode_number() == 5
    assert pd.tonic().natural() == "E"


def test_parent_and_mode_number():
    s = Scale("C", ScaleType.Major)
    assert s.parent() == ScaleType.Major
    assert s.mode_number() == 1
    assert s.type() == ScaleType.Major  # backward compat


def test_mode_name_accessor():
    s = Scale("C", ScaleType.Major)
    assert s.mode_name() == "Ionian"

    d = Scale("D", "dorian")
    assert d.mode_name() == "Dorian"

    nm = Scale("A", ScaleType.NaturalMinor)
    assert nm.mode_name() == "Aeolian"


def test_quality_accessor():
    maj = Scale("C", "major")
    assert maj.quality() == "major"

    dor = Scale("D", "dorian")
    assert dor.quality() == "minor"

    lyd = Scale("F", "lydian")
    assert lyd.quality() == "major"

    loc = Scale("B", "locrian")
    assert loc.quality() == "minor"


def test_brightness_accessor():
    lyd = Scale("F", "lydian")
    assert lyd.brightness() == 7

    ion = Scale("C", "major")
    assert ion.brightness() == 6

    loc = Scale("B", "locrian")
    assert loc.brightness() == 0


def test_mode_int_tracks():
    s = Scale("C", ScaleType.Major)
    dorian = s.mode(2)
    assert dorian.tonic().natural() == "D"
    assert dorian.parent() == ScaleType.Major
    assert dorian.mode_number() == 2
    assert dorian.mode_name() == "Dorian"


def test_mode_string_by_name():
    s = Scale("C", "major")
    lyd = s.mode("lydian")
    assert lyd.tonic().natural() == "F"
    assert lyd.mode_number() == 4
    assert lyd.mode_name() == "Lydian"


def test_mode_string_from_natural_minor():
    nm = Scale("A", "natural minor")
    dor = nm.mode("dorian")
    assert dor.tonic().natural() == "D"
    assert dor.mode_name() == "Dorian"


def test_mode_string_cross_parent_throws():
    s = Scale("C", "major")
    with pytest.raises(ValueError):
        s.mode("altered")
    with pytest.raises(ValueError):
        s.mode("phrygian dominant")


def test_mode_chaining():
    s = Scale("C", "major")
    dorian = s.mode(2)
    from_dorian = dorian.mode(3)
    assert from_dorian.tonic().natural() == "F"
    assert from_dorian.mode_name() == "Lydian"


def test_pentatonic_method():
    s = Scale("C", "major")
    p = s.pentatonic()
    assert p.is_pentatonic()
    assert p.size() == 5
    notes = [n.natural() for n in p.notes()]
    assert notes == ["C", "D", "E", "G", "A"]


def test_pentatonic_minor():
    s = Scale("A", ScaleType.NaturalMinor)
    p = s.pentatonic()
    assert p.size() == 5
    notes = [n.natural() for n in p.notes()]
    assert notes == ["A", "C", "D", "E", "G"]


def test_construct_with_pentatonic_suffix():
    mp = Scale("C", "major pentatonic")
    assert mp.is_pentatonic()
    assert mp.size() == 5
    assert [n.natural() for n in mp.notes()] == ["C", "D", "E", "G", "A"]

    mnp = Scale("A", "minor pentatonic")
    assert mnp.is_pentatonic()
    assert mnp.parent() == ScaleType.NaturalMinor
    assert mnp.size() == 5
    # A minor pentatonic = A C D E G (natural minor, not harmonic)
    assert [n.natural() for n in mnp.notes()] == ["A", "C", "D", "E", "G"]


def test_dorian_pentatonic_via_suffix():
    dp = Scale("D", "dorian pentatonic")
    assert dp.is_pentatonic()
    assert dp.parent() == ScaleType.Major
    assert dp.mode_number() == 2
    assert dp.size() == 5


def test_colors_against_reference():
    dor = Scale("C", "dorian")
    cols = dor.colors("ionian")
    assert len(cols) == 2


def test_colors_lydian_vs_ionian():
    lyd = Scale("C", "lydian")
    cols = lyd.colors("ionian")
    assert len(cols) == 1
    assert cols[0].natural() == "F#"


def test_to_string_mode_name():
    s = Scale("D", "dorian")
    assert repr(s) == 'Scale("D", "Dorian")'


def test_to_string_pentatonic():
    s = Scale("C", "major pentatonic")
    assert repr(s) == 'Scale("C", "major pentatonic")'


def test_backward_compat_enum_constructor():
    s = Scale("C", ScaleType.Major, Modality.Diatonic)
    assert s.type() == ScaleType.Major
    assert s.modality() == Modality.Diatonic
    assert s.parent() == ScaleType.Major
    assert s.mode_number() == 1
    assert not s.is_pentatonic()


def test_backward_compat_natural_minor():
    s = Scale("A", ScaleType.NaturalMinor)
    assert s.parent() == ScaleType.NaturalMinor
    names = [n.natural() for n in s.notes()]
    assert names[0] == "A"
    assert len(names) == 7


def test_harmonic_minor_modes():
    hm = Scale("A", "harmonic minor")
    pd = hm.mode(5)
    assert pd.tonic().natural() == "E"
    assert pd.mode_name() == "Phrygian Dominant"


def test_melodic_minor_modes():
    mm = Scale("C", "melodic minor")
    alt = mm.mode(7)
    assert alt.mode_name() == "Altered"
    assert alt.quality() == "minor"


def test_harmonic_major_construction():
    hm = Scale("C", "harmonic major")
    assert hm.parent() == ScaleType.HarmonicMajor
    assert hm.size() == 7
    notes = [n.natural() for n in hm.notes()]
    assert notes[0] == "C"
    assert notes[5] == "G#"  # Ab = G# in natural notation


# ---------------------------------------------------------------------------
# Circle of fifths: signature, relative, parallel, neighbors
# ---------------------------------------------------------------------------


def test_signature_major_keys():
    assert Scale("C", "major").signature() == 0
    assert Scale("G", "major").signature() == 1
    assert Scale("D", "major").signature() == 2
    assert Scale("F", "major").signature() == -1
    assert Scale("Bb", "major").signature() == -2


def test_signature_natural_minor():
    assert Scale("A", "natural minor").signature() == 0   # = C major
    assert Scale("E", "natural minor").signature() == 1   # = G major
    assert Scale("D", "natural minor").signature() == -1  # = F major


def test_signature_modes():
    # D Dorian (mode 2 of C major) = 0
    assert Scale("D", "dorian").signature() == 0
    # A Mixolydian (mode 5 of D major) = 2
    assert Scale("A", "mixolydian").signature() == 2


def test_signature_non_diatonic():
    assert Scale("A", "harmonic minor").signature() == 0
    assert Scale("C", "whole tone").signature() == 0


def test_relative_major_to_minor():
    rel = Scale("C", "major").relative()
    assert rel.tonic().natural() == "A"
    assert rel.parent() == ScaleType.NaturalMinor


def test_relative_minor_to_major():
    rel = Scale("A", "natural minor").relative()
    assert rel.tonic().natural() == "C"
    assert rel.parent() == ScaleType.Major


def test_relative_g_major():
    rel = Scale("G", "major").relative()
    assert rel.tonic().natural() == "E"


def test_relative_throws_for_modes():
    with pytest.raises(ValueError):
        Scale("D", "dorian").relative()


def test_parallel_major_to_minor():
    par = Scale("C", "major").parallel()
    assert par.tonic().natural() == "C"
    assert par.parent() == ScaleType.NaturalMinor


def test_parallel_minor_to_major():
    par = Scale("C", "natural minor").parallel()
    assert par.tonic().natural() == "C"
    assert par.parent() == ScaleType.Major


def test_parallel_throws_for_modes():
    with pytest.raises(ValueError):
        Scale("D", "dorian").parallel()


def test_neighbors_c_major():
    sub, dom = Scale("C", "major").neighbors()
    assert sub.tonic().natural() == "F"
    assert sub.parent() == ScaleType.Major
    assert dom.tonic().natural() == "G"
    assert dom.parent() == ScaleType.Major


def test_neighbors_a_minor():
    sub, dom = Scale("A", "natural minor").neighbors()
    assert sub.tonic().natural() == "D"
    assert sub.parent() == ScaleType.NaturalMinor
    assert dom.tonic().natural() == "E"
    assert dom.parent() == ScaleType.NaturalMinor


def test_neighbors_preserves_pentatonic():
    s = Scale("C", "major pentatonic")
    sub, dom = s.neighbors()
    assert sub.is_pentatonic()
    assert dom.is_pentatonic()


def test_neighbors_preserves_mode():
    dor = Scale("D", "dorian")
    sub, dom = dor.neighbors()
    assert sub.mode_number() == 2
    assert dom.mode_number() == 2
    assert sub.tonic().natural() == "G"
    assert dom.tonic().natural() == "A"
