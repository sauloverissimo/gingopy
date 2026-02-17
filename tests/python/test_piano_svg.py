"""Tests for the PianoSVG class."""

import os
import tempfile

import pytest
from gingo import (
    Piano, PianoKey, PianoVoicing, PianoSVG, VoicingStyle, Layout,
    Note, Chord, Scale, ScaleType, Field,
)


# ---------------------------------------------------------------------------
# Note
# ---------------------------------------------------------------------------

class TestPianoSVGNote:
    def test_generates_valid_svg(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert "<svg" in svg
        assert "</svg>" in svg
        assert 'xmlns="http://www.w3.org/2000/svg"' in svg

    def test_contains_title(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert "C4" in svg

    def test_highlighted_white_key(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'class="w h"' in svg

    def test_highlighted_black_key(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C#"), 4)
        assert 'class="b h"' in svg
        assert "C#4" in svg

    def test_different_octaves(self):
        piano = Piano()
        svg3 = PianoSVG.note(piano, Note("A"), 3)
        svg5 = PianoSVG.note(piano, Note("A"), 5)
        assert "A3" in svg3
        assert "A5" in svg5


# ---------------------------------------------------------------------------
# CSS style block
# ---------------------------------------------------------------------------

class TestPianoSVGCSS:
    def test_has_style_block(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert "<style>" in svg
        assert "</style>" in svg

    def test_has_css_classes(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert ".w{" in svg
        assert ".b{" in svg
        assert ".h.w{" in svg
        assert ".h.b{" in svg
        assert ".t{" in svg

    def test_highlight_colors_in_css(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert "#4A90D9" in svg
        assert "#2E6AB0" in svg

    def test_pointer_events_in_css(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert "pointer-events:none" in svg


# ---------------------------------------------------------------------------
# Chord
# ---------------------------------------------------------------------------

class TestPianoSVGChord:
    def test_generates_svg_with_name(self):
        piano = Piano()
        svg = PianoSVG.chord(piano, Chord("CM"), 4)
        assert "<svg" in svg
        assert "CM" in svg

    def test_multiple_highlighted_keys(self):
        piano = Piano()
        svg = PianoSVG.chord(piano, Chord("CM"), 4)
        # CM = C, E, G → 3 white keys highlighted
        assert svg.count('class="w h"') >= 3

    def test_with_voicing_style(self):
        piano = Piano()
        svg = PianoSVG.chord(piano, Chord("Am7"), 4, VoicingStyle.Shell)
        assert "<svg" in svg
        assert "Am7" in svg

    def test_open_voicing(self):
        piano = Piano()
        svg = PianoSVG.chord(piano, Chord("CM"), 4, VoicingStyle.Open)
        assert "<svg" in svg


# ---------------------------------------------------------------------------
# Scale
# ---------------------------------------------------------------------------

class TestPianoSVGScale:
    def test_generates_svg_with_name(self):
        piano = Piano()
        svg = PianoSVG.scale(piano, Scale("C", ScaleType.Major), 4)
        assert "<svg" in svg

    def test_c_major_highlights_7_keys(self):
        piano = Piano()
        svg = PianoSVG.scale(piano, Scale("C", ScaleType.Major), 4)
        # C major has 7 white notes → 7 highlighted with class "w h"
        assert svg.count('class="w h"') >= 7

    def test_chromatic_scale(self):
        piano = Piano()
        svg = PianoSVG.scale(piano, Scale("C", ScaleType.Blues), 4)
        assert "<svg" in svg


# ---------------------------------------------------------------------------
# Keys (custom)
# ---------------------------------------------------------------------------

class TestPianoSVGKeys:
    def test_custom_title(self):
        piano = Piano()
        k1 = piano.key(Note("C"), 4)
        k2 = piano.key(Note("E"), 4)
        svg = PianoSVG.keys(piano, [k1, k2], "My Chord")
        assert "My Chord" in svg
        assert "C4" in svg
        assert "E4" in svg

    def test_empty_title_no_text(self):
        piano = Piano()
        k1 = piano.key(Note("C"), 4)
        svg = PianoSVG.keys(piano, [k1])
        assert "<svg" in svg

    def test_empty_keys_list(self):
        piano = Piano()
        svg = PianoSVG.keys(piano, [], "Empty")
        assert "Empty" in svg
        assert 'class="w h"' not in svg
        assert 'class="b h"' not in svg


# ---------------------------------------------------------------------------
# Voicing
# ---------------------------------------------------------------------------

class TestPianoSVGVoicing:
    def test_shows_style_in_title(self):
        piano = Piano()
        v = piano.voicing(Chord("CM"), 4, VoicingStyle.Open)
        svg = PianoSVG.voicing(piano, v)
        assert "CM" in svg
        assert "open" in svg

    def test_close_voicing(self):
        piano = Piano()
        v = piano.voicing(Chord("Am7"), 4, VoicingStyle.Close)
        svg = PianoSVG.voicing(piano, v)
        assert "Am7" in svg
        assert "close" in svg

    def test_shell_voicing(self):
        piano = Piano()
        v = piano.voicing(Chord("Dm7"), 4, VoicingStyle.Shell)
        svg = PianoSVG.voicing(piano, v)
        assert "Dm7" in svg
        assert "shell" in svg


# ---------------------------------------------------------------------------
# MIDI
# ---------------------------------------------------------------------------

class TestPianoSVGMidi:
    def test_generates_svg(self):
        piano = Piano()
        svg = PianoSVG.midi(piano, [60, 64, 67])
        assert "<svg" in svg
        assert "C4" in svg
        assert "E4" in svg
        assert "G4" in svg

    def test_empty_list(self):
        piano = Piano()
        svg = PianoSVG.midi(piano, [])
        assert "<svg" in svg
        assert 'class="w h"' not in svg
        assert 'class="b h"' not in svg


# ---------------------------------------------------------------------------
# Write
# ---------------------------------------------------------------------------

class TestPianoSVGWrite:
    def test_writes_file(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, "test.svg")
            PianoSVG.write(svg, path)
            assert os.path.exists(path)
            with open(path) as f:
                content = f.read()
            assert content == svg

    def test_file_size_reasonable(self):
        piano = Piano()
        svg = PianoSVG.chord(piano, Chord("CM"), 4)
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, "chord.svg")
            PianoSVG.write(svg, path)
            size = os.path.getsize(path)
            assert size > 100  # Not empty
            assert size < 500_000  # Not unreasonably large


# ---------------------------------------------------------------------------
# Interactive attributes (data-*, id, class)
# ---------------------------------------------------------------------------

class TestPianoSVGInteractive:
    def test_key_has_id(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'id="key-60"' in svg

    def test_key_has_data_midi(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'data-midi="60"' in svg

    def test_key_has_data_note(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'data-note="C"' in svg

    def test_key_has_data_octave(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'data-octave="4"' in svg

    def test_white_key_has_class_w(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'class="w"' in svg
        assert 'class="b"' in svg

    def test_highlighted_white_key_class(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'class="w h"' in svg

    def test_highlighted_black_key_class(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C#"), 4)
        assert 'class="b h"' in svg

    def test_white_label_css_class(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert 'class="t tw"' in svg

    def test_black_label_css_class(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C#"), 4)
        assert 'class="t tb"' in svg

    def test_all_keys_have_ids(self):
        """Every key on the 88-key piano should have a unique id."""
        piano = Piano()
        svg = PianoSVG.midi(piano, [])
        for midi in range(21, 109):
            assert f'id="key-{midi}"' in svg


# ---------------------------------------------------------------------------
# Small piano
# ---------------------------------------------------------------------------

class TestPianoSVGSmallPiano:
    def test_61_key_piano(self):
        piano = Piano(61)
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert "<svg" in svg
        assert "C4" in svg

    def test_25_key_piano(self):
        piano = Piano(25)
        svg = PianoSVG.note(piano, Note("C"), 4)
        assert "<svg" in svg


# ---------------------------------------------------------------------------
# Compact mode
# ---------------------------------------------------------------------------

class TestPianoSVGCompact:
    def test_compact_note_smaller_than_full(self):
        piano = Piano()
        full = PianoSVG.note(piano, Note("C"), 4, compact=False)
        compact = PianoSVG.note(piano, Note("C"), 4, compact=True)
        assert len(compact) < len(full)

    def test_compact_note_contains_highlighted_key(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4, compact=True)
        assert 'class="w h"' in svg
        assert "C4" in svg

    def test_compact_omits_distant_keys(self):
        piano = Piano()
        svg = PianoSVG.note(piano, Note("C"), 4, compact=True)
        # A0 (MIDI 21) is far from C4 (MIDI 60)
        assert 'id="key-21"' not in svg
        # C8 (MIDI 108) also far
        assert 'id="key-108"' not in svg

    def test_compact_chord(self):
        piano = Piano()
        svg = PianoSVG.chord(piano, Chord("CM"), 4, VoicingStyle.Close, compact=True)
        assert "<svg" in svg
        assert "CM" in svg
        assert svg.count('class="w h"') >= 3

    def test_compact_scale(self):
        piano = Piano()
        svg = PianoSVG.scale(piano, Scale("C", ScaleType.Major), 4, compact=True)
        assert "<svg" in svg
        assert svg.count('class="w h"') >= 7

    def test_compact_midi(self):
        piano = Piano()
        svg = PianoSVG.midi(piano, [60, 64, 67], compact=True)
        assert "C4" in svg
        assert "E4" in svg
        assert "G4" in svg

    def test_compact_empty_highlights_default_range(self):
        piano = Piano()
        svg = PianoSVG.midi(piano, [], compact=True)
        assert "<svg" in svg
        # Default range C3-B4 (MIDI 48-71), should contain key-60
        assert 'id="key-60"' in svg
        # Should NOT contain A0
        assert 'id="key-21"' not in svg

    def test_compact_voicing(self):
        piano = Piano()
        v = piano.voicing(Chord("CM"), 4, VoicingStyle.Close)
        svg = PianoSVG.voicing(piano, v, compact=True)
        assert "<svg" in svg
        assert "CM" in svg


# ---------------------------------------------------------------------------
# Field composite
# ---------------------------------------------------------------------------

class TestPianoSVGField:
    def test_vertical_generates_composite(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.field(piano, f)
        assert "<svg" in svg
        assert "</svg>" in svg
        assert svg.count("<style>") == 1

    def test_has_all_degree_labels(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.field(piano, f)
        assert "I - CM" in svg
        assert "II - Dm" in svg
        assert "VII - Bdim" in svg

    def test_sevenths(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.field(piano, f, sevenths=True)
        assert "I - C7M" in svg
        assert "V - G7" in svg

    def test_horizontal_layout(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.field(piano, f, layout=Layout.Horizontal)
        assert "<svg" in svg
        assert "I - CM" in svg
        assert "VII - Bdim" in svg

    def test_grid_layout(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.field(piano, f, layout=Layout.Grid)
        assert "<svg" in svg
        assert "I - CM" in svg

    def test_write_field_svg(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.field(piano, f)
        with tempfile.TemporaryDirectory() as tmpdir:
            path = os.path.join(tmpdir, "field.svg")
            PianoSVG.write(svg, path)
            assert os.path.exists(path)


# ---------------------------------------------------------------------------
# Progression composite
# ---------------------------------------------------------------------------

class TestPianoSVGProgression:
    def test_generates_composite(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.progression(piano, f, ["I", "IIm", "V", "I"])
        assert "<svg" in svg
        assert svg.count("<style>") == 1

    def test_shows_branch_labels(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.progression(piano, f, ["I", "V"])
        assert "I (CM)" in svg
        assert "V (GM)" in svg

    def test_horizontal_layout(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.progression(piano, f, ["I", "IIm", "V"],
                                   layout=Layout.Horizontal)
        assert "<svg" in svg
        assert "I (CM)" in svg

    def test_grid_layout(self):
        piano = Piano()
        f = Field("C", "major")
        svg = PianoSVG.progression(piano, f, ["I", "IIm", "V", "I"],
                                   layout=Layout.Grid)
        assert "<svg" in svg
