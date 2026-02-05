"""🪇 Gingo — Music theory library for Python.

A comprehensive toolset for working with music theory programmatically:
notes, intervals, chords, scales, and harmonic fields.

Examples:
    >>> from gingo import Note, Chord, Scale, Field
    >>> Note("Bb").natural()
    'A#'
    >>> Chord("Cm7").notes()
    [Note("C"), Note("D#"), Note("G"), Note("A#")]
    >>> Scale("C", "major").degree(5)
    Note("G")
    >>> [c.name() for c in Field("C", "major").chords()]
    ['CM', 'Dm', 'Em', 'FM', 'GM', 'Am', 'Bdim']
"""

from gingo._gingo import (
    Note,
    Interval,
    Chord,
    ChordComparison,
    Scale,
    ScaleType,
    Modality,
    Field,
    HarmonicFunction,
    BorrowedInfo,
    PivotInfo,
    FieldComparison,
    FieldMatch,
    Tradition,
    Schema,
    Tree,
    HarmonicPath,
    Progression,
    ProgressionMatch,
    ProgressionRoute,
    Duration,
    Tempo,
    TimeSignature,
    NoteEvent,
    ChordEvent,
    Rest,
    Sequence,
)

try:
    from importlib.metadata import version as _version
    __version__ = _version("gingo")
except Exception:
    __version__ = "0.0.0"
__author__ = "Saulo Verissimo"

__all__ = [
    "Note",
    "Interval",
    "Chord",
    "ChordComparison",
    "Scale",
    "ScaleType",
    "Modality",
    "Field",
    "HarmonicFunction",
    "BorrowedInfo",
    "PivotInfo",
    "FieldComparison",
    "FieldMatch",
    "Tradition",
    "Schema",
    "Tree",
    "HarmonicPath",
    "Progression",
    "ProgressionMatch",
    "ProgressionRoute",
    "Duration",
    "Tempo",
    "TimeSignature",
    "NoteEvent",
    "ChordEvent",
    "Rest",
    "Sequence",
]

# ---------------------------------------------------------------------------
# Audio monkey-patch: .play() and .to_wav() on domain classes
# Available when gingo.audio can be imported (always — zero-dep synthesis).
# ---------------------------------------------------------------------------

try:
    from gingo.audio import play as _play_fn, to_wav as _to_wav_fn

    def _make_play(_cls):  # noqa: N802
        def play(self, **kwargs):
            """Play this object through the system audio output.

            See :func:`gingo.audio.play` for keyword arguments.
            """
            return _play_fn(self, **kwargs)
        return play

    def _make_to_wav(_cls):  # noqa: N802
        def to_wav(self, path, **kwargs):
            """Render this object to a WAV file.

            See :func:`gingo.audio.to_wav` for keyword arguments.
            """
            return _to_wav_fn(self, path, **kwargs)
        return to_wav

    for _cls in (Note, Chord, Scale, Field, Tree, Sequence,
                 NoteEvent, ChordEvent, Rest):
        _cls.play = _make_play(_cls)
        _cls.to_wav = _make_to_wav(_cls)

    del _make_play, _make_to_wav, _cls
except Exception:
    pass
