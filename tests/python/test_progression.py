# Gingo — Progression tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import (
    Progression, Tree, Tradition, Schema,
    ProgressionMatch, ProgressionRoute,
    ScaleType, HarmonicFunction,
)


# ---------------------------------------------------------------------------
# Construction
# ---------------------------------------------------------------------------


def test_construction_from_scale_type():
    p = Progression("C", ScaleType.Major)
    assert p.tonic().natural() == "C"
    assert p.type() == ScaleType.Major


def test_construction_from_string():
    p = Progression("C", "major")
    assert p.tonic().natural() == "C"
    assert p.type() == ScaleType.Major


def test_construction_different_tonics():
    p1 = Progression("G", ScaleType.Major)
    assert p1.tonic().natural() == "G"

    p2 = Progression("A", ScaleType.NaturalMinor)
    assert p2.tonic().natural() == "A"
    assert p2.type() == ScaleType.NaturalMinor


def test_construction_minor_string():
    p = Progression("A", "natural minor")
    assert p.type() == ScaleType.NaturalMinor


# ---------------------------------------------------------------------------
# Traditions
# ---------------------------------------------------------------------------


def test_traditions_returns_list():
    traditions = Progression.traditions()
    assert isinstance(traditions, list)
    assert len(traditions) >= 2


def test_traditions_contains_harmonic_tree():
    traditions = Progression.traditions()
    names = [t.name for t in traditions]
    assert "harmonic_tree" in names


def test_traditions_contains_jazz():
    traditions = Progression.traditions()
    names = [t.name for t in traditions]
    assert "jazz" in names


def test_traditions_elements_are_tradition_objects():
    traditions = Progression.traditions()
    for t in traditions:
        assert isinstance(t, Tradition)


# ---------------------------------------------------------------------------
# Tree access
# ---------------------------------------------------------------------------


def test_tree_harmonic_tree():
    p = Progression("C", ScaleType.Major)
    t = p.tree("harmonic_tree")
    assert isinstance(t, Tree)
    branches = t.branches()
    assert len(branches) > 0
    assert "I" in branches


def test_tree_jazz():
    p = Progression("C", ScaleType.Major)
    t = p.tree("jazz")
    assert isinstance(t, Tree)
    branches = t.branches()
    assert len(branches) > 0
    assert "I" in branches
    assert "IIm" in branches
    assert "V7" in branches


def test_tree_invalid_tradition():
    p = Progression("C", ScaleType.Major)
    with pytest.raises(ValueError):
        p.tree("invalid")


# ---------------------------------------------------------------------------
# Tradition struct
# ---------------------------------------------------------------------------


def test_tradition_name_and_description():
    p = Progression("C", ScaleType.Major)
    t = p.tree("harmonic_tree")
    trad = t.tradition()
    assert isinstance(trad, Tradition)
    assert trad.name == "harmonic_tree"
    assert len(trad.description) > 0


def test_tradition_jazz_description():
    p = Progression("C", ScaleType.Major)
    t = p.tree("jazz")
    trad = t.tradition()
    assert trad.name == "jazz"
    assert len(trad.description) > 0


def test_tradition_to_dict():
    p = Progression("C", ScaleType.Major)
    trad = p.tree("harmonic_tree").tradition()
    d = trad.to_dict()
    assert isinstance(d, dict)
    assert set(d.keys()) == {"name", "description"}
    assert d["name"] == "harmonic_tree"
    assert isinstance(d["description"], str)
    assert len(d["description"]) > 0


# ---------------------------------------------------------------------------
# Schema struct
# ---------------------------------------------------------------------------


def test_schemas_harmonic_tree():
    p = Progression("C", ScaleType.Major)
    t = p.tree("harmonic_tree")
    schemas = t.schemas()
    assert len(schemas) > 0
    assert all(isinstance(s, Schema) for s in schemas)


def test_schemas_jazz():
    p = Progression("C", ScaleType.Major)
    t = p.tree("jazz")
    schemas = t.schemas()
    assert len(schemas) > 0

    names = [s.name for s in schemas]
    assert "ii-V-I" in names
    assert "turnaround" in names


def test_schema_attributes():
    p = Progression("C", ScaleType.Major)
    schemas = p.tree("jazz").schemas()
    ii_v_i = [s for s in schemas if s.name == "ii-V-I"][0]

    assert ii_v_i.name == "ii-V-I"
    assert len(ii_v_i.description) > 0
    assert ii_v_i.branches == ["IIm", "V7", "I"]


def test_schema_turnaround_branches():
    p = Progression("C", ScaleType.Major)
    schemas = p.tree("jazz").schemas()
    turnaround = [s for s in schemas if s.name == "turnaround"][0]

    assert turnaround.branches == ["I", "VIm", "IIm", "V7"]


def test_schema_to_dict():
    p = Progression("C", ScaleType.Major)
    schemas = p.tree("jazz").schemas()
    s = schemas[0]
    d = s.to_dict()
    assert isinstance(d, dict)
    assert set(d.keys()) == {"name", "description", "branches"}
    assert isinstance(d["name"], str)
    assert isinstance(d["description"], str)
    assert isinstance(d["branches"], list)


# ---------------------------------------------------------------------------
# Tree.is_valid()
# ---------------------------------------------------------------------------


def test_tree_is_valid_empty():
    p = Progression("C", ScaleType.Major)
    t = p.tree("harmonic_tree")
    assert t.is_valid([])


def test_tree_is_valid_single():
    p = Progression("C", ScaleType.Major)
    t = p.tree("harmonic_tree")
    assert t.is_valid(["I"])


def test_tree_is_valid_ii_v_i():
    p = Progression("C", ScaleType.Major)
    t = p.tree("jazz")
    valid = t.is_valid(["IIm", "V7", "I"])
    assert isinstance(valid, bool)


def test_tree_is_valid_invalid_branch():
    p = Progression("C", ScaleType.Major)
    t = p.tree("harmonic_tree")
    assert not t.is_valid(["I", "INVALID", "V7"])


# ---------------------------------------------------------------------------
# identify()
# ---------------------------------------------------------------------------


def test_identify_exact_ii_v_i():
    p = Progression("C", ScaleType.Major)
    m = p.identify(["IIm", "V7", "I"])
    assert isinstance(m, ProgressionMatch)
    assert m.score > 0.0
    assert m.branches == ["IIm", "V7", "I"]


def test_identify_turnaround():
    p = Progression("C", ScaleType.Major)
    m = p.identify(["I", "VIm", "IIm", "V7"])
    assert isinstance(m, ProgressionMatch)
    assert m.score > 0.0
    assert len(m.tradition) > 0


def test_identify_harmonic_tree_descending():
    p = Progression("C", ScaleType.Major)
    m = p.identify(["I", "V7 / IIm", "IIm", "V7", "I"])
    assert isinstance(m, ProgressionMatch)
    assert m.score > 0.0
    assert m.tradition == "harmonic_tree"
    assert m.schema == "descending"


def test_identify_returns_best_match():
    p = Progression("C", ScaleType.Major)
    m = p.identify(["I", "V7", "I"])
    assert isinstance(m, ProgressionMatch)
    assert m.score > 0.0
    assert isinstance(m.matched, int)
    assert isinstance(m.total, int)


def test_identify_empty_raises():
    p = Progression("C", ScaleType.Major)
    with pytest.raises(ValueError):
        p.identify([])


# ---------------------------------------------------------------------------
# deduce()
# ---------------------------------------------------------------------------


def test_deduce_returns_list():
    p = Progression("C", ScaleType.Major)
    results = p.deduce(["I", "IIm"])
    assert isinstance(results, list)
    assert len(results) > 0


def test_deduce_elements_are_progression_match():
    p = Progression("C", ScaleType.Major)
    results = p.deduce(["I", "IIm"])
    for r in results:
        assert isinstance(r, ProgressionMatch)


def test_deduce_sorted_by_score():
    p = Progression("C", ScaleType.Major)
    results = p.deduce(["I", "IIm"])
    for i in range(len(results) - 1):
        assert results[i].score >= results[i + 1].score


def test_deduce_limit():
    p = Progression("C", ScaleType.Major)
    results = p.deduce(["I", "IIm"], limit=2)
    assert len(results) <= 2


def test_deduce_partial_ii_v():
    p = Progression("C", ScaleType.Major)
    results = p.deduce(["IIm", "V7"])
    assert len(results) > 0
    # Should find ii-V-I or related patterns
    has_jazz = any(r.tradition == "jazz" for r in results)
    assert has_jazz


def test_deduce_empty_raises():
    p = Progression("C", ScaleType.Major)
    with pytest.raises(ValueError):
        p.deduce([])


# ---------------------------------------------------------------------------
# predict()
# ---------------------------------------------------------------------------


def test_predict_returns_list():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    assert isinstance(results, list)
    assert len(results) > 0


def test_predict_elements_are_progression_route():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    for r in results:
        assert isinstance(r, ProgressionRoute)


def test_predict_sorted_by_confidence():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    for i in range(len(results) - 1):
        assert results[i].confidence >= results[i + 1].confidence


def test_predict_has_next_branch():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    for r in results:
        assert len(r.next) > 0


def test_predict_v7_after_iim():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["IIm"])
    # V7 should appear as a likely next branch from IIm
    nexts = [r.next for r in results]
    assert "V7" in nexts


def test_predict_with_tradition_filter():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"], tradition="jazz")
    assert len(results) > 0
    for r in results:
        assert r.tradition == "jazz"


def test_predict_path_extends_input():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    for r in results:
        assert len(r.path) == 3
        assert r.path[0] == "I"
        assert r.path[1] == "IIm"
        assert r.path[2] == r.next


def test_predict_empty_raises():
    p = Progression("C", ScaleType.Major)
    with pytest.raises(ValueError):
        p.predict([])


# ---------------------------------------------------------------------------
# ProgressionMatch.to_dict()
# ---------------------------------------------------------------------------


def test_progression_match_to_dict_keys():
    p = Progression("C", ScaleType.Major)
    m = p.identify(["IIm", "V7", "I"])
    d = m.to_dict()
    assert isinstance(d, dict)
    expected = {"tradition", "schema", "score", "matched", "total", "branches"}
    assert set(d.keys()) == expected


def test_progression_match_to_dict_values():
    p = Progression("C", ScaleType.Major)
    m = p.identify(["IIm", "V7", "I"])
    d = m.to_dict()
    assert isinstance(d["tradition"], str)
    assert isinstance(d["score"], float)
    assert isinstance(d["matched"], int)
    assert isinstance(d["total"], int)
    assert isinstance(d["branches"], list)
    assert d["branches"] == ["IIm", "V7", "I"]


# ---------------------------------------------------------------------------
# ProgressionRoute.to_dict()
# ---------------------------------------------------------------------------


def test_progression_route_to_dict_keys():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    assert len(results) > 0
    d = results[0].to_dict()
    assert isinstance(d, dict)
    expected = {"next", "tradition", "schema", "path", "confidence"}
    assert set(d.keys()) == expected


def test_progression_route_to_dict_values():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    d = results[0].to_dict()
    assert isinstance(d["next"], str)
    assert isinstance(d["tradition"], str)
    assert isinstance(d["schema"], str)
    assert isinstance(d["path"], list)
    assert isinstance(d["confidence"], float)


# ---------------------------------------------------------------------------
# repr() and str()
# ---------------------------------------------------------------------------


def test_progression_repr():
    p = Progression("C", ScaleType.Major)
    r = repr(p)
    assert len(r) > 0
    assert "C" in r


def test_progression_str():
    p = Progression("C", ScaleType.Major)
    s = str(p)
    assert len(s) > 0
    assert "C" in s


def test_tradition_repr():
    trad = Progression("C", ScaleType.Major).tree("jazz").tradition()
    r = repr(trad)
    assert "Tradition" in r
    assert "jazz" in r


def test_tradition_str():
    trad = Progression("C", ScaleType.Major).tree("jazz").tradition()
    s = str(trad)
    assert s == "jazz"


def test_schema_repr():
    schemas = Progression("C", ScaleType.Major).tree("jazz").schemas()
    r = repr(schemas[0])
    assert "Schema" in r


def test_schema_str():
    schemas = Progression("C", ScaleType.Major).tree("jazz").schemas()
    s = str(schemas[0])
    assert len(s) > 0


def test_progression_match_repr():
    p = Progression("C", ScaleType.Major)
    m = p.identify(["IIm", "V7", "I"])
    r = repr(m)
    assert "ProgressionMatch" in r


def test_progression_route_repr():
    p = Progression("C", ScaleType.Major)
    results = p.predict(["I", "IIm"])
    assert len(results) > 0
    r = repr(results[0])
    assert "ProgressionRoute" in r


# ---------------------------------------------------------------------------
# Cross-tradition coverage
# ---------------------------------------------------------------------------


def test_identify_across_traditions():
    """identify() should search all traditions and return the best match."""
    p = Progression("C", ScaleType.Major)

    # Jazz ii-V-I exact match
    m_jazz = p.identify(["IIm", "V7", "I"])
    assert m_jazz.tradition == "jazz"
    assert m_jazz.schema == "ii-V-I"
    assert m_jazz.score == 1.0

    # Harmonic tree descending
    m_ht = p.identify(["I", "V7 / IIm", "IIm", "V7", "I"])
    assert m_ht.tradition == "harmonic_tree"
    assert m_ht.schema == "descending"


def test_deduce_across_traditions():
    """deduce() should return matches from multiple traditions."""
    p = Progression("C", ScaleType.Major)
    results = p.deduce(["I", "IIm", "V7"])
    traditions = {r.tradition for r in results}
    # Should appear in at least one tradition
    assert len(traditions) >= 1


def test_minor_key_progression():
    """Progression should work with minor keys."""
    p = Progression("A", ScaleType.NaturalMinor)
    t = p.tree("harmonic_tree")
    branches = t.branches()
    assert "Im" in branches


def test_minor_key_jazz():
    p = Progression("A", ScaleType.NaturalMinor)
    t = p.tree("jazz")
    branches = t.branches()
    assert "Im" in branches
    assert "V7" in branches
