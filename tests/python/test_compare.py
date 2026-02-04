# Gingo — Comparison tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import (
    Chord,
    ChordComparison,
    Field,
    FieldComparison,
    BorrowedInfo,
    PivotInfo,
    HarmonicFunction,
    ScaleType,
)


# ---------------------------------------------------------------------------
# ChordComparison tests
# ---------------------------------------------------------------------------


def test_compare_identical():
    r = Chord("CM").compare(Chord("CM"))
    assert isinstance(r, ChordComparison)
    assert len(r.common_notes) == 3
    assert len(r.exclusive_a) == 0
    assert len(r.exclusive_b) == 0
    assert r.root_distance == 0
    assert r.root_direction == 0
    assert r.same_quality is True
    assert r.same_size is True
    assert r.enharmonic is True
    assert r.subset == "equal"
    assert r.voice_leading == 0
    assert r.transformation == ""
    assert r.inversion is False


def test_compare_relative_r():
    r = Chord("CM").compare(Chord("Am"))
    assert len(r.common_notes) == 2
    assert r.root_distance == 3
    assert r.same_quality is False
    assert r.transformation == "R"


def test_compare_parallel_p():
    r = Chord("CM").compare(Chord("Cm"))
    assert len(r.common_notes) == 2
    assert r.root_distance == 0
    assert r.transformation == "P"


def test_compare_leading_tone_l():
    r = Chord("CM").compare(Chord("Em"))
    assert r.root_distance == 4
    assert r.transformation == "L"


def test_compare_different_size():
    r = Chord("CM").compare(Chord("Cm7"))
    assert r.same_size is False
    assert r.voice_leading == -1


def test_compare_subset():
    r = Chord("CM").compare(Chord("C7M"))
    assert r.subset == "a_subset_of_b"


def test_compare_root_direction_positive():
    r = Chord("CM").compare(Chord("FM"))
    assert r.root_direction == 5
    assert r.root_distance == 5


def test_compare_root_direction_negative():
    r = Chord("CM").compare(Chord("BM"))
    assert r.root_direction == -1
    assert r.root_distance == 1


def test_compare_common_intervals():
    r = Chord("CM").compare(Chord("Cm"))
    assert "P1" in r.common_intervals
    assert "5J" in r.common_intervals


def test_compare_voice_leading_same_chord():
    r = Chord("CM").compare(Chord("CM"))
    assert r.voice_leading == 0


def test_compare_no_transformation_non_triads():
    r = Chord("C7M").compare(Chord("Am7"))
    assert r.transformation == ""


def test_compare_repr():
    r = Chord("CM").compare(Chord("Am"))
    assert "ChordComparison" in repr(r)


# ---------------------------------------------------------------------------
# FieldComparison tests
# ---------------------------------------------------------------------------


def test_field_compare_diatonic():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("GM"))
    assert isinstance(r, FieldComparison)

    assert r.degree_a == 1
    assert r.degree_b == 5
    assert r.function_a == HarmonicFunction.Tonic
    assert r.function_b == HarmonicFunction.Dominant
    assert r.same_function is False
    assert r.diatonic_a is True
    assert r.diatonic_b is True
    assert len(r.foreign_a) == 0
    assert len(r.foreign_b) == 0


def test_field_compare_relative_pair():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Am"))

    assert r.relative is True
    assert r.same_function is True


def test_field_compare_non_diatonic():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("F#M"))

    assert r.diatonic_a is True
    assert r.diatonic_b is False
    assert r.degree_b is None
    assert r.function_b is None


def test_field_compare_secondary_dominant():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("D7"), Chord("GM"))

    assert r.secondary_dominant == "a_is_V7_of_b"


def test_field_compare_tritone_sub():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("G7"), Chord("C#7"))

    assert r.tritone_sub is True


def test_field_compare_pivot():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Am"))

    assert len(r.pivot) > 0
    c_major = [p for p in r.pivot if p.tonic == "C" and p.scale_type == "Major"]
    assert len(c_major) == 1
    assert c_major[0].degree_a == 1
    assert c_major[0].degree_b == 6


def test_field_compare_borrowed():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Fm"))

    assert r.diatonic_b is False
    assert r.borrowed_b is not None
    assert isinstance(r.borrowed_b, BorrowedInfo)
    assert r.borrowed_b.scale_type == "NaturalMinor"
    assert r.borrowed_b.degree == 4


def test_field_compare_foreign_notes():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Fm"))

    assert len(r.foreign_a) == 0
    assert len(r.foreign_b) > 0


def test_field_compare_degree_distance():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("GM"))

    assert r.degree_distance == 4


def test_field_compare_chromatic_mediant():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("EM"))

    assert r.chromatic_mediant == "upper"


def test_field_compare_repr():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("GM"))
    assert "FieldComparison" in repr(r)


# ---------------------------------------------------------------------------
# Interval Vector (Forte set theory)
# ---------------------------------------------------------------------------


def test_interval_vector_major_minor_triads():
    r = Chord("CM").compare(Chord("Cm"))
    assert r.interval_vector_a == [0, 0, 1, 1, 1, 0]
    assert r.interval_vector_b == [0, 0, 1, 1, 1, 0]
    assert r.same_interval_vector is True


def test_interval_vector_differs():
    r = Chord("CM").compare(Chord("Caug"))
    assert r.interval_vector_a == [0, 0, 1, 1, 1, 0]
    assert r.interval_vector_b == [0, 0, 0, 3, 0, 0]
    assert r.same_interval_vector is False


def test_interval_vector_seventh():
    r = Chord("C7M").compare(Chord("C7M"))
    assert r.interval_vector_a == [1, 0, 1, 2, 2, 0]
    assert r.same_interval_vector is True


# ---------------------------------------------------------------------------
# Transposition detection (Lewin T_n)
# ---------------------------------------------------------------------------


def test_transposition_cm_to_dm():
    r = Chord("CM").compare(Chord("DM"))
    assert r.transposition == 2


def test_transposition_not_related():
    r = Chord("CM").compare(Chord("Cm"))
    assert r.transposition == -1


def test_transposition_identical():
    r = Chord("CM").compare(Chord("CM"))
    assert r.transposition == 0


def test_transposition_different_sizes():
    r = Chord("CM").compare(Chord("Cm7"))
    assert r.transposition == -1


def test_transposition_seventh_chords():
    r = Chord("Am7").compare(Chord("Cm7"))
    assert r.transposition == 3


# ---------------------------------------------------------------------------
# Neo-Riemannian 2-step compositions
# ---------------------------------------------------------------------------


def test_neo_riemannian_composition_rp():
    r = Chord("CM").compare(Chord("AM"))
    assert r.transformation == "RP"


def test_neo_riemannian_composition_lp():
    r = Chord("CM").compare(Chord("EM"))
    assert r.transformation == "LP"


def test_neo_riemannian_composition_pl():
    r = Chord("CM").compare(Chord("G#M"))
    assert r.transformation == "PL"


def test_neo_riemannian_composition_pr():
    r = Chord("CM").compare(Chord("D#M"))
    assert r.transformation == "PR"


def test_neo_riemannian_single_still_works():
    assert Chord("CM").compare(Chord("Cm")).transformation == "P"
    assert Chord("CM").compare(Chord("Em")).transformation == "L"
    assert Chord("CM").compare(Chord("Am")).transformation == "R"


# ---------------------------------------------------------------------------
# Dissonance (Sethares / Plomp-Levelt)
# ---------------------------------------------------------------------------


def test_dissonance_non_negative():
    r = Chord("CM").compare(Chord("Cm7"))
    assert r.dissonance_a >= 0.0
    assert r.dissonance_b >= 0.0


def test_dissonance_identical():
    r = Chord("CM").compare(Chord("CM"))
    assert r.dissonance_a == r.dissonance_b


def test_dissonance_is_float():
    r = Chord("CM").compare(Chord("Am"))
    assert isinstance(r.dissonance_a, float)
    assert isinstance(r.dissonance_b, float)


def test_dissonance_diminished_higher_than_major():
    r = Chord("CM").compare(Chord("Cdim"))
    assert r.dissonance_b > r.dissonance_a


# ---------------------------------------------------------------------------
# Root Motion (Field)
# ---------------------------------------------------------------------------


def test_field_root_motion_descending_fifth():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("GM"), Chord("CM"))
    assert r.root_motion == "descending_fifth"


def test_field_root_motion_ascending_fifth():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("GM"))
    assert r.root_motion == "ascending_fifth"


def test_field_root_motion_ascending_step():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("FM"), Chord("GM"))
    assert r.root_motion == "ascending_step"


def test_field_root_motion_descending_third():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Am"))
    assert r.root_motion == "descending_third"


def test_field_root_motion_unison():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("CM"))
    assert r.root_motion == "unison"


def test_field_root_motion_non_diatonic():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("F#M"))
    assert r.root_motion == ""


# ---------------------------------------------------------------------------
# Applied Diminished (Field)
# ---------------------------------------------------------------------------


def test_field_applied_diminished_bdim_to_c():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("Bdim"), Chord("CM"))
    assert r.applied_diminished == "a_is_viidim_of_b"


def test_field_applied_diminished_reversed():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Bdim"))
    assert r.applied_diminished == "b_is_viidim_of_a"


def test_field_applied_diminished_m7b5():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("F#m7(b5)"), Chord("GM"))
    assert r.applied_diminished == "a_is_viidim_of_b"


def test_field_applied_diminished_no_match():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("GM"))
    assert r.applied_diminished == ""


def test_field_applied_diminished_non_dim():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("Bm"), Chord("CM"))
    assert r.applied_diminished == ""


# ---------------------------------------------------------------------------
# to_dict() serialization
# ---------------------------------------------------------------------------


def test_chord_comparison_to_dict_keys():
    d = Chord("CM").compare(Chord("Am")).to_dict()
    assert isinstance(d, dict)
    expected = {
        "common_notes", "exclusive_a", "exclusive_b",
        "root_distance", "root_direction", "same_quality", "same_size",
        "common_intervals", "enharmonic", "subset", "voice_leading",
        "transformation", "inversion",
        "interval_vector_a", "interval_vector_b", "same_interval_vector",
        "transposition", "dissonance_a", "dissonance_b",
    }
    assert set(d.keys()) == expected


def test_chord_comparison_to_dict_values():
    d = Chord("CM").compare(Chord("Am")).to_dict()
    assert d["common_notes"] == ["C", "E"]
    assert d["exclusive_a"] == ["G"]
    assert d["exclusive_b"] == ["A"]
    assert d["root_distance"] == 3
    assert d["transformation"] == "R"
    assert d["same_quality"] is False
    assert isinstance(d["dissonance_a"], float)
    assert d["interval_vector_a"] == [0, 0, 1, 1, 1, 0]


def test_chord_comparison_to_dict_notes_are_strings():
    d = Chord("CM").compare(Chord("Cm")).to_dict()
    for note in d["common_notes"]:
        assert isinstance(note, str)


def test_field_comparison_to_dict_keys():
    f = Field("C", ScaleType.Major)
    d = f.compare(Chord("CM"), Chord("GM")).to_dict()
    assert isinstance(d, dict)
    expected = {
        "degree_a", "degree_b", "function_a", "function_b",
        "role_a", "role_b", "degree_distance", "same_function",
        "relative", "progression", "root_motion",
        "secondary_dominant", "applied_diminished",
        "diatonic_a", "diatonic_b", "borrowed_a", "borrowed_b",
        "pivot", "tritone_sub", "chromatic_mediant",
        "foreign_a", "foreign_b",
    }
    assert set(d.keys()) == expected


def test_field_comparison_to_dict_values():
    f = Field("C", ScaleType.Major)
    d = f.compare(Chord("CM"), Chord("GM")).to_dict()
    assert d["degree_a"] == 1
    assert d["degree_b"] == 5
    assert d["function_a"] == "Tonic"
    assert d["function_b"] == "Dominant"
    assert d["same_function"] is False
    assert d["diatonic_a"] is True
    assert d["root_motion"] == "ascending_fifth"
    assert d["foreign_a"] == []


def test_field_comparison_to_dict_none_values():
    f = Field("C", ScaleType.Major)
    d = f.compare(Chord("CM"), Chord("F#M")).to_dict()
    assert d["degree_b"] is None
    assert d["function_b"] is None
    assert d["role_b"] is None


def test_field_comparison_to_dict_pivot_is_list_of_dicts():
    f = Field("C", ScaleType.Major)
    d = f.compare(Chord("CM"), Chord("Am")).to_dict()
    assert isinstance(d["pivot"], list)
    assert len(d["pivot"]) > 0
    p = d["pivot"][0]
    assert isinstance(p, dict)
    assert "tonic" in p
    assert "scale_type" in p
    assert "degree_a" in p
    assert "degree_b" in p


def test_field_comparison_to_dict_borrowed_is_dict():
    f = Field("C", ScaleType.Major)
    d = f.compare(Chord("CM"), Chord("Fm")).to_dict()
    assert d["borrowed_b"] is not None
    b = d["borrowed_b"]
    assert isinstance(b, dict)
    assert b["scale_type"] == "NaturalMinor"
    assert b["degree"] == 4
    assert b["function"] == "Subdominant"


def test_borrowed_info_to_dict():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Fm"))
    d = r.borrowed_b.to_dict()
    assert isinstance(d, dict)
    assert d["scale_type"] == "NaturalMinor"
    assert d["function"] == "Subdominant"


def test_pivot_info_to_dict():
    f = Field("C", ScaleType.Major)
    r = f.compare(Chord("CM"), Chord("Am"))
    p = [x for x in r.pivot if x.tonic == "C" and x.scale_type == "Major"][0]
    d = p.to_dict()
    assert isinstance(d, dict)
    assert d["tonic"] == "C"
    assert d["degree_a"] == 1
    assert d["degree_b"] == 6
