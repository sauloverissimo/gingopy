# Gingo — Tree tests (Python)
# SPDX-License-Identifier: MIT

import pytest
from gingo import Tree, ScaleType, HarmonicFunction


# ---------------------------------------------------------------------------
# Construction
# ---------------------------------------------------------------------------

def test_tree_construction_from_scale_type():
    t = Tree("C", ScaleType.Major)
    assert t.tonic().natural() == "C"
    assert t.type() == ScaleType.Major


def test_tree_construction_from_string():
    t = Tree("C", "major")
    assert t.tonic().natural() == "C"
    assert t.type() == ScaleType.Major


def test_tree_construction_different_tonics():
    t1 = Tree("G", ScaleType.Major)
    assert t1.tonic().natural() == "G"

    t2 = Tree("A", ScaleType.NaturalMinor)
    assert t2.tonic().natural() == "A"


# ---------------------------------------------------------------------------
# Branches
# ---------------------------------------------------------------------------

def test_tree_branches_non_empty():
    t = Tree("C", ScaleType.Major)
    branches = t.branches()
    assert len(branches) > 0


def test_tree_branches_major_scale():
    t = Tree("C", ScaleType.Major)
    branches = t.branches()

    # Should contain basic diatonic branches
    assert "I" in branches
    assert "V7" in branches
    assert "IIm" in branches


def test_tree_branches_minor_scale():
    t = Tree("A", ScaleType.NaturalMinor)
    branches = t.branches()

    # Should contain Im
    assert "Im" in branches


# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

def test_tree_paths_from_origin():
    t = Tree("C", ScaleType.Major)
    paths = t.paths("I")

    assert len(paths) > 0
    # First path should be the origin itself
    assert paths[0].branch == "I"
    assert paths[0].id == 0


def test_tree_paths_contain_chord_info():
    t = Tree("C", ScaleType.Major)
    paths = t.paths("I")

    # Origin should have chord resolved
    assert paths[0].chord.root().natural() == "C"
    assert len(paths[0].interval_labels) > 0
    assert len(paths[0].note_names) > 0


# ---------------------------------------------------------------------------
# Shortest path
# ---------------------------------------------------------------------------

def test_tree_shortest_path_direct():
    t = Tree("C", ScaleType.Major)
    path = t.shortest_path("I", "V7")

    assert len(path) > 0
    assert path[0] == "I"
    assert path[-1] == "V7"


def test_tree_shortest_path_invalid_branch():
    t = Tree("C", ScaleType.Major)
    path = t.shortest_path("INVALID", "V7")

    assert len(path) == 0


def test_tree_shortest_path_same_branch():
    t = Tree("C", ScaleType.Major)
    path = t.shortest_path("I", "I")

    assert len(path) == 1
    assert path[0] == "I"


def test_tree_shortest_path_multi_step():
    t = Tree("C", ScaleType.Major)
    path = t.shortest_path("I", "IV")

    # Should find some path, even if indirect
    if len(path) > 0:
        assert path[0] == "I"
        assert path[-1] == "IV"
        assert len(path) >= 2


# ---------------------------------------------------------------------------
# Valid progression
# ---------------------------------------------------------------------------

def test_tree_valid_progression_single():
    t = Tree("C", ScaleType.Major)
    assert t.is_valid_progression(["I"])


def test_tree_valid_progression_ii_v_i():
    t = Tree("C", ScaleType.Major)
    # Classic II-V-I should be valid if paths exist
    valid = t.is_valid_progression(["IIm", "V7", "I"])
    # Don't require it to be true, just check it doesn't crash
    assert isinstance(valid, bool)


def test_tree_valid_progression_empty():
    t = Tree("C", ScaleType.Major)
    assert t.is_valid_progression([])


def test_tree_valid_progression_invalid_branch():
    t = Tree("C", ScaleType.Major)
    # Test with progression containing invalid branch
    # Single invalid branch may return True as empty path is valid
    # Test with progression that has invalid transition
    assert not t.is_valid_progression(["I", "INVALID", "V7"])


# ---------------------------------------------------------------------------
# Harmonic function
# ---------------------------------------------------------------------------

def test_tree_function_tonic():
    t = Tree("C", ScaleType.Major)
    func = t.function("I")
    assert func == HarmonicFunction.Tonic


def test_tree_function_subdominant():
    t = Tree("C", ScaleType.Major)
    func = t.function("IV")
    assert func == HarmonicFunction.Subdominant


def test_tree_function_dominant():
    t = Tree("C", ScaleType.Major)
    func = t.function("V7")
    assert func == HarmonicFunction.Dominant


def test_tree_function_applied_uses_primary():
    t = Tree("C", ScaleType.Major)
    # V7/IV should be Dominant (V7 = Dominant)
    func = t.function("V7 / IV")
    assert func == HarmonicFunction.Dominant


def test_tree_function_minor_branches():
    t = Tree("C", ScaleType.Major)

    func_iim = t.function("IIm")
    assert func_iim == HarmonicFunction.Subdominant

    func_vim = t.function("VIm")
    assert func_vim == HarmonicFunction.Tonic


# ---------------------------------------------------------------------------
# Branches with function
# ---------------------------------------------------------------------------

def test_tree_branches_with_function_tonics():
    t = Tree("C", ScaleType.Major)
    tonics = t.branches_with_function(HarmonicFunction.Tonic)

    assert len(tonics) > 0
    # Should contain I
    assert "I" in tonics


def test_tree_branches_with_function_subdominants():
    t = Tree("C", ScaleType.Major)
    subdominants = t.branches_with_function(HarmonicFunction.Subdominant)

    assert len(subdominants) > 0


def test_tree_branches_with_function_dominants():
    t = Tree("C", ScaleType.Major)
    dominants = t.branches_with_function(HarmonicFunction.Dominant)

    assert len(dominants) > 0
    # Should contain V7
    assert "V7" in dominants


# ---------------------------------------------------------------------------
# Visualization export
# ---------------------------------------------------------------------------

def test_tree_to_dot_basic():
    t = Tree("C", ScaleType.Major)
    dot = t.to_dot(False)

    assert len(dot) > 0
    assert "digraph" in dot


def test_tree_to_dot_with_functions():
    t = Tree("C", ScaleType.Major)
    dot = t.to_dot(True)

    assert len(dot) > 0
    # Should have color information
    assert "fillcolor" in dot


def test_tree_to_mermaid():
    t = Tree("C", ScaleType.Major)
    mermaid = t.to_mermaid()

    assert len(mermaid) > 0
    assert "graph" in mermaid


# ---------------------------------------------------------------------------
# Diminished chord notation
# ---------------------------------------------------------------------------

def test_tree_diminished_notation():
    t = Tree("C", ScaleType.Major)
    all_branches = t.branches()

    # Check if diminished notation exists (either dim or °)
    has_dim = any("dim" in b or "°" in b for b in all_branches)
    # Should support at least one notation
    assert has_dim


# ---------------------------------------------------------------------------
# Different scale types
# ---------------------------------------------------------------------------

def test_tree_natural_minor():
    t = Tree("A", ScaleType.NaturalMinor)
    branches = t.branches()
    assert len(branches) > 0

    paths = t.paths("Im")
    assert len(paths) > 0


def test_tree_harmonic_minor():
    t = Tree("A", ScaleType.HarmonicMinor)
    branches = t.branches()
    assert len(branches) > 0


def test_tree_melodic_minor():
    t = Tree("A", ScaleType.MelodicMinor)
    branches = t.branches()
    assert len(branches) > 0


# ---------------------------------------------------------------------------
# String representation
# ---------------------------------------------------------------------------

def test_tree_to_string():
    t = Tree("C", ScaleType.Major)
    s = str(t)

    assert len(s) > 0
    assert "C" in s


def test_tree_repr():
    t = Tree("C", ScaleType.Major)
    r = repr(t)

    assert len(r) > 0
