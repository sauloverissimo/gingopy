# Gingo — Pythonic protocol tests
# SPDX-License-Identifier: MIT

import pytest
from gingo import Note, Interval, Chord, Scale, ScaleType, Field


# ---------------------------------------------------------------------------
# Note: hash, ordering, arithmetic, int
# ---------------------------------------------------------------------------


def test_note_hash_equal():
    assert hash(Note("C")) == hash(Note("C"))
    assert hash(Note("Bb")) == hash(Note("A#"))


def test_note_in_set():
    s = {Note("C"), Note("E"), Note("G")}
    assert Note("C") in s
    assert Note("D") not in s


def test_note_as_dict_key():
    d = {Note("C"): "tonic", Note("G"): "dominant"}
    assert d[Note("C")] == "tonic"


def test_note_ordering():
    assert Note("C") < Note("D")
    assert Note("B") > Note("A")
    assert Note("C") <= Note("C")
    assert Note("G") >= Note("E")


def test_note_sorted():
    notes = [Note("G"), Note("C"), Note("E")]
    result = sorted(notes)
    assert [n.natural() for n in result] == ["C", "E", "G"]


def test_note_int():
    assert int(Note("C")) == 0
    assert int(Note("G")) == 7
    assert int(Note("A#")) == 10


def test_note_add():
    assert (Note("C") + 7).natural() == "G"
    assert (Note("G") + 5).natural() == "C"


def test_note_sub_int():
    assert (Note("G") - 7).natural() == "C"
    assert (Note("C") - 1).natural() == "B"


def test_note_sub_note():
    iv = Note("G") - Note("C")
    assert isinstance(iv, Interval)
    assert iv.semitones() == 7
    assert iv.label() == "5J"
    assert (Note("C") - Note("G")).semitones() == 5
    assert (Note("C") - Note("C")).semitones() == 0


# ---------------------------------------------------------------------------
# Interval: hash, ordering, int
# ---------------------------------------------------------------------------


def test_interval_hash():
    assert hash(Interval("5J")) == hash(Interval(7))


def test_interval_in_set():
    s = {Interval("5J"), Interval("3M")}
    assert Interval(7) in s


def test_interval_ordering():
    assert Interval("3m") < Interval("5J")
    assert Interval("7M") > Interval("3M")
    assert Interval("5J") <= Interval("5J")


def test_interval_sorted():
    ivs = [Interval("5J"), Interval("3m"), Interval("P1")]
    result = sorted(ivs)
    assert [iv.semitones() for iv in result] == [0, 3, 7]


def test_interval_int():
    assert int(Interval("5J")) == 7
    assert int(Interval("3M")) == 4


# ---------------------------------------------------------------------------
# Chord: hash, len, contains, iter, getitem
# ---------------------------------------------------------------------------


def test_chord_hash():
    assert hash(Chord("CM")) == hash(Chord("CM"))


def test_chord_in_set():
    s = {Chord("CM"), Chord("Am"), Chord("GM")}
    assert Chord("CM") in s
    assert Chord("Dm") not in s


def test_chord_len():
    assert len(Chord("CM")) == 3
    assert len(Chord("C7")) == 4


def test_chord_contains():
    c = Chord("CM")
    assert Note("C") in c
    assert Note("E") in c
    assert Note("G") in c
    assert Note("D") not in c


def test_chord_iter():
    notes = [n.natural() for n in Chord("CM")]
    assert notes == ["C", "E", "G"]


def test_chord_iter_unpack():
    root, third, fifth = Chord("CM")
    assert root.natural() == "C"
    assert third.natural() == "E"
    assert fifth.natural() == "G"


def test_chord_getitem():
    c = Chord("CM")
    assert c[0].natural() == "C"
    assert c[1].natural() == "E"
    assert c[2].natural() == "G"


def test_chord_getitem_negative():
    c = Chord("CM")
    assert c[-1].natural() == "G"
    assert c[-2].natural() == "E"
    assert c[-3].natural() == "C"


def test_chord_getitem_out_of_range():
    c = Chord("CM")
    with pytest.raises(IndexError):
        c[3]
    with pytest.raises(IndexError):
        c[-4]


# ---------------------------------------------------------------------------
# Scale: eq, hash, str, contains, iter, getitem, reversed
# ---------------------------------------------------------------------------


def test_scale_eq():
    assert Scale("C", "major") == Scale("C", "major")
    assert Scale("C", "major") != Scale("G", "major")
    assert Scale("C", "major") != Scale("C", "natural minor")


def test_scale_hash():
    assert hash(Scale("C", "major")) == hash(Scale("C", "major"))


def test_scale_in_set():
    s = {Scale("C", "major"), Scale("G", "major")}
    assert Scale("C", "major") in s
    assert Scale("D", "major") not in s


def test_scale_str():
    assert str(Scale("C", "major")) == "C major"
    assert str(Scale("A", "natural minor")) == "A natural minor"
    assert str(Scale("D", "dorian")) == "D dorian"
    assert str(Scale("C", "harmonic minor")) == "C harmonic minor"


def test_scale_str_pentatonic():
    assert "pentatonic" in str(Scale("C", "major pentatonic"))


def test_scale_contains():
    s = Scale("C", "major")
    assert Note("C") in s
    assert Note("E") in s
    assert Note("F#") not in s


def test_scale_iter():
    notes = [n.natural() for n in Scale("C", "major")]
    assert notes == ["C", "D", "E", "F", "G", "A", "B"]


def test_scale_getitem():
    s = Scale("C", "major")
    assert s[0].natural() == "C"
    assert s[6].natural() == "B"


def test_scale_getitem_negative():
    s = Scale("C", "major")
    assert s[-1].natural() == "B"
    assert s[-7].natural() == "C"


def test_scale_getitem_out_of_range():
    s = Scale("C", "major")
    with pytest.raises(IndexError):
        s[7]
    with pytest.raises(IndexError):
        s[-8]


def test_scale_reversed():
    notes = [n.natural() for n in reversed(Scale("C", "major"))]
    assert notes == ["B", "A", "G", "F", "E", "D", "C"]


# ---------------------------------------------------------------------------
# Field: eq, hash, str, contains, iter, getitem
# ---------------------------------------------------------------------------


def test_field_eq():
    assert Field("C", "major") == Field("C", "major")
    assert Field("C", "major") != Field("G", "major")


def test_field_hash():
    assert hash(Field("C", "major")) == hash(Field("C", "major"))


def test_field_in_set():
    s = {Field("C", "major"), Field("G", "major")}
    assert Field("C", "major") in s


def test_field_str():
    assert str(Field("C", "major")) == "C major"
    assert str(Field("A", "natural minor")) == "A natural minor"


def test_field_contains_chord():
    f = Field("C", "major")
    assert Chord("CM") in f
    assert Chord("Am") in f
    assert Chord("F#M") not in f


def test_field_contains_string():
    f = Field("C", "major")
    assert "CM" in f
    assert "F#M" not in f


def test_field_iter():
    chords = [str(c) for c in Field("C", "major")]
    assert len(chords) == 7
    assert chords[0] == "CM"


def test_field_getitem():
    f = Field("C", "major")
    assert str(f[0]) == "CM"
    assert f[4].root().natural() == "G"


def test_field_getitem_negative():
    f = Field("C", "major")
    last = f[-1]
    assert last.root().natural() == "B"


def test_field_getitem_out_of_range():
    f = Field("C", "major")
    with pytest.raises(IndexError):
        f[7]
    with pytest.raises(IndexError):
        f[-8]


# ---------------------------------------------------------------------------
# Interval: from Notes, simple, invert, consonance, full_name, arithmetic
# ---------------------------------------------------------------------------


def test_interval_from_notes():
    iv = Interval(Note("C"), Note("G"))
    assert iv.semitones() == 7
    assert iv.label() == "5J"


def test_interval_from_notes_reversed():
    iv = Interval(Note("G"), Note("C"))
    assert iv.semitones() == 5
    assert iv.label() == "4J"


def test_interval_from_notes_unison():
    assert Interval(Note("C"), Note("C")).semitones() == 0


def test_interval_simple():
    assert Interval("5J").simple().semitones() == 7
    assert Interval("9").simple().semitones() == 2
    assert Interval("8J").simple().semitones() == 0


def test_interval_is_compound():
    assert not Interval("5J").is_compound()
    assert Interval("9").is_compound()
    assert Interval("8J").is_compound()


def test_interval_invert():
    assert Interval("5J").invert().semitones() == 5
    assert Interval("3M").invert().semitones() == 8
    assert Interval("4J").invert().semitones() == 7
    assert Interval("d5").invert().semitones() == 6
    assert Interval("P1").invert().semitones() == 12
    assert Interval("8J").invert().semitones() == 0


def test_interval_invert_compound():
    assert Interval("9").invert().semitones() == 10
    assert Interval("11").invert().semitones() == 7


def test_interval_consonance():
    assert Interval("P1").consonance() == "perfect"
    assert Interval("5J").consonance() == "perfect"
    assert Interval("3m").consonance() == "imperfect"
    assert Interval("3M").consonance() == "imperfect"
    assert Interval("M6").consonance() == "imperfect"
    assert Interval("2m").consonance() == "dissonant"
    assert Interval("7M").consonance() == "dissonant"
    assert Interval("d5").consonance() == "dissonant"


def test_interval_consonance_fourth():
    assert Interval("4J").consonance() == "dissonant"
    assert Interval("4J").consonance(include_fourth=True) == "perfect"


def test_interval_is_consonant():
    assert Interval("5J").is_consonant()
    assert Interval("3M").is_consonant()
    assert not Interval("2m").is_consonant()
    assert not Interval("4J").is_consonant()
    assert Interval("4J").is_consonant(include_fourth=True)


def test_interval_full_name():
    assert Interval("P1").full_name() == "Perfect Unison"
    assert Interval("3m").full_name() == "Minor Third"
    assert Interval("5J").full_name() == "Perfect Fifth"
    assert Interval("d5").full_name() == "Tritone"
    assert Interval("8J").full_name() == "Perfect Octave"
    assert Interval("9").full_name() == "Major Ninth"


def test_interval_full_name_pt():
    assert Interval("P1").full_name_pt() == "Unissono Justo"
    assert Interval("3m").full_name_pt() == "Terca Menor"
    assert Interval("5J").full_name_pt() == "Quinta Justa"
    assert Interval("d5").full_name_pt() == "Tritono"


def test_interval_add():
    result = Interval("3M") + Interval("3m")
    assert result.semitones() == 7
    assert result.label() == "5J"


def test_interval_sub():
    result = Interval("5J") - Interval("3M")
    assert result.semitones() == 3
    assert result.label() == "3m"


def test_interval_add_overflow():
    with pytest.raises(OverflowError):
        Interval("8J") + Interval("8J")  # 12 + 12 = 24 > 23


def test_interval_sub_underflow():
    with pytest.raises(RuntimeError):
        Interval("3m") - Interval("5J")


def test_interval_eq_int():
    assert Interval("5J") == 7
    assert Interval("3M") == 4
    assert not (Interval("5J") == 8)
    assert Interval("5J") != 8
    assert not (Interval("5J") != 7)


# ---------------------------------------------------------------------------
# Note + Interval, Note - Interval
# ---------------------------------------------------------------------------


def test_note_add_interval():
    assert (Note("C") + Interval("5J")).natural() == "G"
    assert (Note("C") + Interval("3M")).natural() == "E"


def test_note_sub_interval():
    assert (Note("G") - Interval("5J")).natural() == "C"
    assert (Note("E") - Interval("3M")).natural() == "C"


# ---------------------------------------------------------------------------
# Scale: degree_of
# ---------------------------------------------------------------------------


def test_scale_degree_of():
    s = Scale("C", "major")
    assert s.degree_of(Note("C")) == 1
    assert s.degree_of(Note("D")) == 2
    assert s.degree_of(Note("E")) == 3
    assert s.degree_of(Note("G")) == 5
    assert s.degree_of(Note("B")) == 7


def test_scale_degree_of_not_found():
    s = Scale("C", "major")
    assert s.degree_of(Note("F#")) is None
    assert s.degree_of(Note("Bb")) is None


def test_scale_degree_of_enharmonic():
    s = Scale("C", "major")
    assert s.degree_of(Note("Fb")) == 3  # Fb == E


# ---------------------------------------------------------------------------
# Phase 3: Note.__float__
# ---------------------------------------------------------------------------


def test_note_float():
    assert float(Note("A")) == 440.0
    assert isinstance(float(Note("C")), float)


def test_note_float_all_notes():
    assert float(Note("C")) == pytest.approx(261.626, rel=1e-3)
    assert float(Note("E")) == pytest.approx(329.628, rel=1e-3)


# ---------------------------------------------------------------------------
# Phase 3: Chord slicing, reversed, contains(str)
# ---------------------------------------------------------------------------


def test_chord_slice():
    c = Chord("CM")
    result = c[0:2]
    assert isinstance(result, list)
    assert len(result) == 2
    assert result[0].natural() == "C"
    assert result[1].natural() == "E"


def test_chord_slice_step():
    c = Chord("C7")  # C E G Bb
    result = c[::2]
    assert len(result) == 2
    assert result[0].natural() == "C"
    assert result[1].natural() == "G"


def test_chord_slice_negative():
    c = Chord("CM")
    result = c[::-1]
    assert [n.natural() for n in result] == ["G", "E", "C"]


def test_chord_reversed():
    notes = [n.natural() for n in reversed(Chord("CM"))]
    assert notes == ["G", "E", "C"]


def test_chord_contains_string():
    c = Chord("CM")
    assert "C" in c
    assert "E" in c
    assert "G" in c
    assert "D" not in c


# ---------------------------------------------------------------------------
# Phase 3: Scale slicing, contains(str)
# ---------------------------------------------------------------------------


def test_scale_slice():
    s = Scale("C", "major")
    result = s[0:3]
    assert isinstance(result, list)
    assert len(result) == 3
    assert [n.natural() for n in result] == ["C", "D", "E"]


def test_scale_slice_step():
    s = Scale("C", "major")
    result = s[::2]
    assert [n.natural() for n in result] == ["C", "E", "G", "B"]


def test_scale_slice_negative():
    s = Scale("C", "major")
    result = s[::-1]
    assert [n.natural() for n in result] == ["B", "A", "G", "F", "E", "D", "C"]


def test_scale_contains_string():
    s = Scale("C", "major")
    assert "C" in s
    assert "E" in s
    assert "F#" not in s


# ---------------------------------------------------------------------------
# Phase 3: Field slicing, reversed
# ---------------------------------------------------------------------------


def test_field_slice():
    f = Field("C", "major")
    result = f[0:3]
    assert isinstance(result, list)
    assert len(result) == 3
    assert str(result[0]) == "CM"


def test_field_slice_step():
    f = Field("C", "major")
    result = f[::2]
    assert len(result) == 4  # 7 chords, every other: 0,2,4,6


def test_field_reversed():
    chords = [str(c) for c in reversed(Field("C", "major"))]
    assert len(chords) == 7
    assert chords[-1] == "CM"  # first chord is now last


# ---------------------------------------------------------------------------
# Phase 4: Field roman numeral access
# ---------------------------------------------------------------------------


def test_field_roman_triads():
    f = Field("C", "major")
    assert str(f["I"]) == "CM"
    assert str(f["II"]) == "Dm"
    assert str(f["III"]) == "Em"
    assert str(f["IV"]) == "FM"
    assert str(f["V"]) == "GM"
    assert str(f["VI"]) == "Am"
    assert str(f["VII"]) == "Bdim"


def test_field_roman_sevenths():
    f = Field("C", "major")
    assert str(f["I7"]) == "C7M"
    assert str(f["V7"]) == "G7"
    assert str(f["II7"]) == "Dm7"
    assert str(f["IV7"]) == "F7M"


def test_field_roman_accidental():
    f = Field("C", "major")
    assert str(f["bVII"]) == "BbM"
    assert str(f["bIII"]) == "EbM"
    assert str(f["bVI"]) == "AbM"


def test_field_roman_accidental_quality():
    f = Field("C", "major")
    assert str(f["bVII7"]) == "Bb7"
    assert str(f["#IVm7(b5)"]) == "F#m7(b5)"


def test_field_roman_minor_field():
    f = Field("A", "natural minor")
    assert str(f["I"]) == "Am"
    assert str(f["III"]) == "CM"
    assert str(f["V"]) == "Em"
    assert str(f["V7"]) == "Em7"


def test_field_roman_invalid():
    f = Field("C", "major")
    with pytest.raises(KeyError):
        f["VIII"]
    with pytest.raises(KeyError):
        f["X"]
    with pytest.raises(KeyError):
        f[""]


def test_field_roman_coexists_with_int():
    f = Field("C", "major")
    assert str(f["I"]) == str(f[0])
    assert str(f["V"]) == str(f[4])


# ---------------------------------------------------------------------------
# Phase 5: Field applied chord via tuple syntax
# ---------------------------------------------------------------------------


def test_field_applied_tuple_str_str():
    """f["V7", "IV"] → applied("V7", "IV") — secondary dominant."""
    f = Field("C", "major")
    assert str(f["V7", "IV"]) == "C7"
    assert str(f["V7", "V"]) == "D7"
    assert str(f["V7", "II"]) == "A7"
    assert str(f["V7", "VI"]) == "E7"


def test_field_applied_tuple_str_int():
    """f["V7", 4] → applied("V7", 4)."""
    f = Field("C", "major")
    assert str(f["V7", 4]) == "C7"
    assert str(f["V7", 5]) == "D7"


def test_field_applied_tuple_int_int():
    """f[5, 4] → applied(5, 4)."""
    f = Field("C", "major")
    assert str(f[5, 4]) == "C7"
    assert str(f[5, 5]) == "D7"


def test_field_applied_tuple_accidentals():
    """Accidentals work naturally in tuple elements."""
    f = Field("C", "major")
    assert str(f["V7", "bVII"]) == "F7"
    assert str(f["bVII7", "IV"]) == str(f.applied("bVII7", "IV"))


def test_field_applied_tuple_quality():
    """Explicit quality in function element."""
    f = Field("C", "major")
    assert str(f["IIm7(b5)", "V"]) == "Am7(b5)"


def test_field_applied_tuple_minor_field():
    """Applied chords in minor field."""
    f = Field("A", "natural minor")
    assert str(f["V7", "I"]) == "E7"
    assert str(f["V7", "IV"]) == "A7"


def test_field_applied_tuple_matches_method():
    """Tuple syntax produces same result as explicit applied() call."""
    f = Field("C", "major")
    assert str(f["V7", "IV"]) == str(f.applied("V7", "IV"))
    assert str(f["V7", 4]) == str(f.applied("V7", 4))
    assert str(f[5, 4]) == str(f.applied(5, 4))


def test_field_applied_tuple_wrong_size():
    """Tuple with != 2 elements raises KeyError."""
    f = Field("C", "major")
    with pytest.raises(KeyError):
        f["V7", "IV", "II"]
    with pytest.raises(KeyError):
        f["V7",]


def test_field_applied_tuple_wrong_types():
    """Tuple with invalid type combination raises TypeError."""
    f = Field("C", "major")
    with pytest.raises(TypeError):
        f[5, "IV"]  # int, str not supported (no C++ overload)


# ---------------------------------------------------------------------------
# Phase 6: Flexible construction
# ---------------------------------------------------------------------------


def test_chord_from_list_of_strings():
    """Chord(["C", "E", "G"]) identifies as CM."""
    c = Chord(["C", "E", "G"])
    assert str(c) == "CM"


def test_chord_from_list_of_notes():
    """Chord([Note("C"), Note("E"), Note("G")]) identifies as CM."""
    c = Chord([Note("C"), Note("E"), Note("G")])
    assert str(c) == "CM"


def test_chord_from_list_matches_identify():
    """Constructor list form produces same result as Chord.identify()."""
    assert str(Chord(["C", "E", "G"])) == str(Chord.identify(["C", "E", "G"]))
    assert str(Chord(["A", "C", "E"])) == str(Chord.identify(["A", "C", "E"]))


def test_chord_from_list_seventh():
    """Chord from 4 notes identifies a seventh chord."""
    c = Chord(["C", "E", "G", "Bb"])
    assert str(c) == "C7"


def test_chord_from_list_minor():
    """Chord from minor triad notes."""
    c = Chord(["A", "C", "E"])
    assert str(c) == "Am"


def test_chord_from_tuple():
    """Chord(("C", "E", "G")) also works (tuple input)."""
    c = Chord(("C", "E", "G"))
    assert str(c) == "CM"


def test_chord_from_list_invalid():
    """Chord from unrecognizable notes raises."""
    with pytest.raises(Exception):
        Chord(["C", "C#", "D"])


def test_chord_string_still_works():
    """Original string constructor is unchanged."""
    assert str(Chord("CM")) == "CM"
    assert str(Chord("Am7")) == "Am7"


def test_scale_identify_major():
    """Scale.identify detects C major."""
    s = Scale.identify(["C", "D", "E", "F", "G", "A", "B"])
    assert s.tonic().name() == "C"
    assert [n.natural() for n in s] == ["C", "D", "E", "F", "G", "A", "B"]


def test_scale_identify_natural_minor():
    """Scale.identify detects A natural minor (= A aeolian)."""
    s = Scale.identify(["A", "B", "C", "D", "E", "F", "G"])
    assert s.tonic().name() == "A"
    assert s.mode_name() in ("Aeolian", "NaturalMinor")


def test_scale_identify_dorian():
    """Scale.identify detects D dorian."""
    s = Scale.identify(["D", "E", "F", "G", "A", "B", "C"])
    assert s.tonic().name() == "D"
    assert s.mode_name() == "Dorian"


def test_scale_identify_harmonic_minor():
    """Scale.identify detects A harmonic minor."""
    s = Scale.identify(["A", "B", "C", "D", "E", "F", "G#"])
    assert s.tonic().name() == "A"


def test_scale_identify_pentatonic():
    """Scale.identify detects C major pentatonic."""
    s = Scale.identify(["C", "D", "E", "G", "A"])
    assert s.tonic().name() == "C"
    assert s.is_pentatonic()
    assert s.size() == 5


def test_scale_identify_minor_pentatonic():
    """Scale.identify detects A minor pentatonic."""
    s = Scale.identify(["A", "C", "D", "E", "G"])
    assert s.tonic().name() == "A"
    assert s.is_pentatonic()


def test_scale_identify_from_note_objects():
    """Scale.identify accepts Note objects."""
    notes = [Note("C"), Note("D"), Note("E"), Note("F"),
             Note("G"), Note("A"), Note("B")]
    s = Scale.identify(notes)
    assert s.tonic().name() == "C"


def test_scale_identify_not_found():
    """Scale.identify raises for unrecognizable note set."""
    with pytest.raises(ValueError):
        Scale.identify(["C", "C#", "D"])


# ---------------------------------------------------------------------------
# Phase 7: Field.identify (key detection)
# ---------------------------------------------------------------------------


# --- Note-based ---


def test_field_identify_major_notes():
    """Field.identify from C major scale notes."""
    f = Field.identify(["C", "D", "E", "F", "G", "A", "B"])
    assert f.tonic().name() == "C"
    assert "major" in str(f).lower()


def test_field_identify_minor_notes():
    """Field.identify from A natural minor scale notes."""
    f = Field.identify(["A", "B", "C", "D", "E", "F", "G"])
    assert f.tonic().name() == "A"
    assert "minor" in str(f).lower()


def test_field_identify_dorian_notes():
    """D dorian notes → parent field C major (dorian has no own field type)."""
    f = Field.identify(["D", "E", "F", "G", "A", "B", "C"])
    assert f.tonic().name() == "C"
    assert "major" in str(f).lower()


def test_field_identify_harmonic_minor_notes():
    """Field.identify from A harmonic minor notes."""
    f = Field.identify(["A", "B", "C", "D", "E", "F", "G#"])
    assert f.tonic().name() == "A"


def test_field_identify_note_objects():
    """Field.identify accepts Note objects."""
    notes = [Note("C"), Note("D"), Note("E"), Note("F"),
             Note("G"), Note("A"), Note("B")]
    f = Field.identify(notes)
    assert f.tonic().name() == "C"


def test_field_identify_pentatonic_notes():
    """Field.identify from pentatonic scale notes."""
    f = Field.identify(["C", "D", "E", "G", "A"])
    assert f.tonic().name() == "C"


def test_field_identify_notes_not_found():
    """Field.identify raises for unrecognizable notes."""
    with pytest.raises(ValueError):
        Field.identify(["C", "C#", "D"])


# --- Chord-based ---


def test_field_identify_major_chords():
    """Field.identify from C major diatonic triads."""
    f = Field.identify(["CM", "Dm", "Em", "FM", "GM", "Am"])
    assert f.tonic().name() == "C"
    assert "major" in str(f).lower()


def test_field_identify_subset_chords():
    """Field.identify from subset of chords (I, IV, V7)."""
    f = Field.identify(["CM", "FM", "G7"])
    assert f.tonic().name() == "C"


def test_field_identify_harmonic_minor_chords():
    """E7 chord signals A harmonic minor over A natural minor."""
    f = Field.identify(["Am", "Dm", "E7"])
    assert f.tonic().name() == "A"
    # E7 is diatonic in harmonic minor, not natural minor
    assert f.function(Chord("E7")) is not None


def test_field_identify_chord_objects():
    """Field.identify accepts Chord objects."""
    f = Field.identify([Chord("CM"), Chord("FM"), Chord("GM")])
    assert f.tonic().name() == "C"


def test_field_identify_f_major_chords():
    """Field.identify detects F major from its triads."""
    f = Field.identify(["FM", "Gm", "Am", "BbM", "CM"])
    assert f.tonic().name() == "F"


def test_field_identify_empty():
    """Field.identify raises on empty input."""
    with pytest.raises(ValueError):
        Field.identify([])


def test_field_identify_usable():
    """Identified field supports full API (function, degree_of, etc.)."""
    f = Field.identify(["CM", "FM", "G7"])
    # Can query functions
    func = f.function(Chord("GM"))
    assert func is not None
    # Can query degrees
    deg = f.scale().degree_of(Note("G"))
    assert deg == 5


# ---------------------------------------------------------------------------
# Phase 8: Field.deduce (ranked key inference)
# ---------------------------------------------------------------------------


# --- Chord-based ---


def test_deduce_ambiguous_two_chords():
    """CM + FM are diatonic in both C major and F major."""
    results = Field.deduce(["CM", "FM"])
    assert len(results) >= 2
    assert results[0].score == 1.0
    tonics = {r.field.tonic().name() for r in results if r.score == 1.0}
    assert "C" in tonics and "F" in tonics


def test_deduce_three_chords_c_major():
    """CM + FM + GM uniquely identifies C major as top result."""
    results = Field.deduce(["CM", "FM", "GM"])
    assert results[0].field.tonic().name() == "C"
    assert results[0].score == 1.0
    assert results[0].matched == 3
    assert results[0].total == 3


def test_deduce_roles_correct():
    """Roles reflect the degree of each chord in the field."""
    results = Field.deduce(["CM", "FM", "GM"])
    r = [r for r in results
         if r.field.tonic().name() == "C"
         and "major" in str(r.field).lower()][0]
    assert r.roles == ["I", "IV", "V"]


def test_deduce_harmonic_minor():
    """E7 chord signals A harmonic minor with perfect score."""
    results = Field.deduce(["Am", "Dm", "E7"])
    hm = [r for r in results if r.score == 1.0
          and r.field.tonic().name() == "A"]
    assert len(hm) > 0


def test_deduce_chord_objects():
    """Chord objects accepted as input."""
    results = Field.deduce([Chord("CM"), Chord("FM")])
    assert len(results) >= 2


def test_deduce_single_chord():
    """Single chord matches multiple fields, all with score 1.0."""
    results = Field.deduce(["CM"])
    assert len(results) >= 1
    assert all(r.score == 1.0 for r in results)


def test_deduce_seventh_roles():
    """Seventh chords get role with '7' suffix."""
    results = Field.deduce(["CM", "G7"])
    r = [r for r in results
         if r.field.tonic().name() == "C"
         and "major" in str(r.field).lower()][0]
    assert "I" in r.roles
    assert "V7" in r.roles


# --- Note-based ---


def test_deduce_partial_notes():
    """C, D, E belong to multiple scales; results are ranked."""
    results = Field.deduce(["C", "D", "E"])
    assert len(results) >= 1
    assert results[0].score > 0
    tonics = {r.field.tonic().name() for r in results[:10]}
    assert "C" in tonics


def test_deduce_note_objects():
    """Note objects accepted as input."""
    results = Field.deduce([Note("C"), Note("E"), Note("G")])
    assert len(results) >= 1


def test_deduce_note_roles():
    """Note roles are Roman numerals for the scale degree."""
    results = Field.deduce(["C", "E", "G"])
    r = [r for r in results
         if r.field.tonic().name() == "C"
         and "major" in str(r.field).lower()][0]
    assert r.roles == ["I", "III", "V"]


# --- FieldMatch struct ---


def test_deduce_to_dict():
    """FieldMatch.to_dict() contains all expected keys."""
    results = Field.deduce(["CM", "FM"])
    d = results[0].to_dict()
    assert "field" in d
    assert "score" in d
    assert "matched" in d
    assert "total" in d
    assert "roles" in d


def test_deduce_repr():
    """FieldMatch repr contains score."""
    results = Field.deduce(["CM", "FM"])
    assert "score=" in repr(results[0])


# --- Limit parameter ---


def test_deduce_limit():
    """limit parameter caps the number of results."""
    results = Field.deduce(["CM"], limit=3)
    assert len(results) <= 3


def test_deduce_limit_zero_returns_all():
    """limit=0 returns all candidates."""
    limited = Field.deduce(["CM"], limit=3)
    full = Field.deduce(["CM"], limit=0)
    assert len(full) >= len(limited)


# --- Errors ---


def test_deduce_empty_raises():
    """Empty input raises ValueError."""
    with pytest.raises(ValueError):
        Field.deduce([])


def test_deduce_no_match_returns_list():
    """Unrecognizable chords return a list (possibly with partial matches)."""
    results = Field.deduce(["CM", "F#m", "Bb7", "Edim", "G#aug"])
    assert isinstance(results, list)
