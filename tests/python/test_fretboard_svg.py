"""Tests for the FretboardSVG class."""

import os
import tempfile

import pytest
from gingo import (
    Fretboard, FretboardSVG, Chord, Scale, ScaleType, Note, Field, Layout,
    Orientation, Handedness,
)


# ---------------------------------------------------------------------------
# Chord diagram
# ---------------------------------------------------------------------------

class TestFretboardSVGChord:
    def test_generates_valid_svg(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("CM"))
        assert "<svg" in svg
        assert "</svg>" in svg
        assert 'xmlns="http://www.w3.org/2000/svg"' in svg

    def test_contains_title(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("Am7"))
        assert "Am7" in svg

    def test_has_style_block(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("CM"))
        assert "<style>" in svg
        assert "</style>" in svg

    def test_has_string_lines(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("CM"))
        assert 'class="str"' in svg

    def test_has_fret_lines(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("CM"))
        assert 'class="frt"' in svg

    def test_has_dots(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("CM"))
        assert 'class="dot"' in svg

    def test_on_cavaquinho(self):
        c = Fretboard.cavaquinho()
        svg = FretboardSVG.chord(c, Chord("CM"))
        assert "<svg" in svg
        assert "CM" in svg

    def test_on_bandolim(self):
        b = Fretboard.bandolim()
        svg = FretboardSVG.chord(b, Chord("CM"))
        assert "<svg" in svg


# ---------------------------------------------------------------------------
# Fingering
# ---------------------------------------------------------------------------

class TestFretboardSVGFingering:
    def test_renders_fingering(self):
        g = Fretboard.violao()
        f = g.fingering(Chord("Am"))
        svg = FretboardSVG.fingering(g, f)
        assert "<svg" in svg
        assert "Am" in svg


# ---------------------------------------------------------------------------
# Scale
# ---------------------------------------------------------------------------

class TestFretboardSVGScale:
    def test_generates_valid_svg(self):
        g = Fretboard.violao()
        svg = FretboardSVG.scale(g, Scale("C", ScaleType.Major), 0, 12)
        assert "<svg" in svg
        assert "</svg>" in svg

    def test_has_string_labels(self):
        g = Fretboard.violao()
        svg = FretboardSVG.scale(g, Scale("C", ScaleType.Major), 0, 5)
        assert "E4" in svg


# ---------------------------------------------------------------------------
# Note
# ---------------------------------------------------------------------------

class TestFretboardSVGNote:
    def test_shows_positions(self):
        g = Fretboard.violao()
        svg = FretboardSVG.note(g, Note("C"))
        assert "<svg" in svg
        assert ">C<" in svg


# ---------------------------------------------------------------------------
# Positions
# ---------------------------------------------------------------------------

class TestFretboardSVGPositions:
    def test_custom_title(self):
        g = Fretboard.violao()
        pos = g.positions(Note("E"))
        svg = FretboardSVG.positions(g, pos, "All E notes")
        assert "All E notes" in svg


# ---------------------------------------------------------------------------
# Field composite
# ---------------------------------------------------------------------------

class TestFretboardSVGField:
    def test_vertical_generates_svg(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.field(g, f)
        assert "<svg" in svg
        assert "</svg>" in svg

    def test_has_degree_labels(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.field(g, f)
        assert "I - CM" in svg
        assert "VII - Bdim" in svg

    def test_grid_layout(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.field(g, f, Layout.Grid)
        assert "<svg" in svg
        assert "I - CM" in svg

    def test_horizontal_layout(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.field(g, f, Layout.Horizontal)
        assert "<svg" in svg


# ---------------------------------------------------------------------------
# Progression composite
# ---------------------------------------------------------------------------

class TestFretboardSVGProgression:
    def test_generates_svg(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.progression(g, f, ["I", "IV", "V", "I"])
        assert "<svg" in svg

    def test_shows_labels(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.progression(g, f, ["I", "V"])
        assert "I (CM)" in svg
        assert "V (GM)" in svg


# ---------------------------------------------------------------------------
# Orientation
# ---------------------------------------------------------------------------

class TestFretboardSVGOrientation:
    def test_chord_horizontal(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("CM"), 0, Orientation.Horizontal)
        assert "<svg" in svg
        assert "E4" in svg

    def test_scale_vertical(self):
        g = Fretboard.violao()
        svg = FretboardSVG.scale(g, Scale("C", ScaleType.Major), 0, 5,
                                  Orientation.Vertical)
        assert "<svg" in svg
        assert "Ionian" in svg

    def test_note_vertical(self):
        g = Fretboard.violao()
        svg = FretboardSVG.note(g, Note("C"), Orientation.Vertical)
        assert "<svg" in svg
        assert ">C<" in svg


# ---------------------------------------------------------------------------
# Handedness
# ---------------------------------------------------------------------------

class TestFretboardSVGHandedness:
    def test_chord_left_handed_differs(self):
        g = Fretboard.violao()
        svg_r = FretboardSVG.chord(g, Chord("CM"), 0,
                    Orientation.Vertical, Handedness.RightHanded)
        svg_l = FretboardSVG.chord(g, Chord("CM"), 0,
                    Orientation.Vertical, Handedness.LeftHanded)
        assert "<svg" in svg_r
        assert "<svg" in svg_l
        assert svg_r != svg_l

    def test_scale_left_handed_differs(self):
        g = Fretboard.violao()
        svg_r = FretboardSVG.scale(g, Scale("C", ScaleType.Major), 0, 5,
                    Orientation.Horizontal, Handedness.RightHanded)
        svg_l = FretboardSVG.scale(g, Scale("C", ScaleType.Major), 0, 5,
                    Orientation.Horizontal, Handedness.LeftHanded)
        assert "<svg" in svg_r
        assert "<svg" in svg_l
        assert svg_r != svg_l

    def test_chord_horizontal_left_handed(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("Am"), 0,
                  Orientation.Horizontal, Handedness.LeftHanded)
        assert "<svg" in svg
        assert "</svg>" in svg


# ---------------------------------------------------------------------------
# Composite with orientation/handedness
# ---------------------------------------------------------------------------

class TestFretboardSVGCompositeParams:
    def test_field_horizontal(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.field(g, f, Layout.Vertical, Orientation.Horizontal)
        assert "<svg" in svg
        assert "E4" in svg

    def test_progression_left_handed(self):
        g = Fretboard.violao()
        f = Field("C", "major")
        svg = FretboardSVG.progression(g, f, ["I", "V"],
                  Layout.Vertical, Orientation.Vertical, Handedness.LeftHanded)
        assert "<svg" in svg
        assert "I (CM)" in svg


# ---------------------------------------------------------------------------
# Default backward compatibility
# ---------------------------------------------------------------------------

class TestFretboardSVGBackwardCompat:
    def test_defaults_match_explicit(self):
        g = Fretboard.violao()
        svg1 = FretboardSVG.chord(g, Chord("CM"))
        svg2 = FretboardSVG.chord(g, Chord("CM"), 0,
                   Orientation.Vertical, Handedness.RightHanded)
        assert svg1 == svg2


# ---------------------------------------------------------------------------
# Full fretboard
# ---------------------------------------------------------------------------

class TestFretboardSVGFull:
    def test_generates_svg(self):
        g = Fretboard.violao()
        svg = FretboardSVG.full(g)
        assert "<svg" in svg
        assert "</svg>" in svg
        assert 'class="nut"' in svg

    def test_shows_instrument_name(self):
        g = Fretboard.violao()
        svg = FretboardSVG.full(g)
        assert g.name() in svg

    def test_vertical(self):
        g = Fretboard.violao()
        svg = FretboardSVG.full(g, Orientation.Vertical)
        assert "<svg" in svg


# ---------------------------------------------------------------------------
# Write
# ---------------------------------------------------------------------------

class TestFretboardSVGWrite:
    def test_writes_file(self):
        g = Fretboard.violao()
        svg = FretboardSVG.chord(g, Chord("CM"))
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, "test.svg")
            FretboardSVG.write(svg, path)
            assert os.path.exists(path)
            with open(path) as f:
                content = f.read()
            assert content == svg
