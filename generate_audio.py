#!/usr/bin/env python3
"""Generate audio assets (MP3) for documentation pages.

Generates MP3 files for:
- docs/guia/exemplos-praticos.md (practical examples)
- docs/guia/audio.md (audio guide)

Usage:
    python generate_audio.py

Requires: gingo, ffmpeg (for WAV->MP3 conversion)
"""

import os
import subprocess
import tempfile
from pathlib import Path

from gingo import (
    Note, Chord, Scale, Field, Sequence, NoteEvent, ChordEvent, Rest,
    Duration, Tempo, TimeSignature,
)
from gingo.audio import to_wav, Envelope

AUDIO_DIR = Path(__file__).parent / "docs" / "assets" / "audio"
AUDIO_DIR.mkdir(parents=True, exist_ok=True)


def wav_to_mp3(wav_path: str, mp3_path: str) -> None:
    """Convert WAV to MP3 using ffmpeg."""
    subprocess.run(
        ["ffmpeg", "-y", "-i", wav_path, "-codec:a", "libmp3lame",
         "-b:a", "128k", mp3_path],
        capture_output=True, check=True,
    )


def generate(name: str, obj, **kwargs) -> None:
    """Generate a WAV, convert to MP3, save in docs/assets/audio/."""
    mp3_path = str(AUDIO_DIR / f"{name}.mp3")
    with tempfile.NamedTemporaryFile(suffix=".wav", delete=False) as tmp:
        wav_path = tmp.name
    try:
        to_wav(obj, wav_path, **kwargs)
        wav_to_mp3(wav_path, mp3_path)
        size_kb = os.path.getsize(mp3_path) / 1024
        print(f"  {name}.mp3  ({size_kb:.0f} KB)")
    finally:
        os.unlink(wav_path)


def exemplos_praticos():
    """Audio assets for docs/guia/exemplos-praticos.md."""

    # Fretboard chords
    print("Fretboard chords:")
    generate("chord_am", Chord("Am"), waveform="triangle", strum=0.04)
    generate("chord_fm", Chord("FM"), waveform="triangle", strum=0.04)
    generate("chord_gm", Chord("GM"), waveform="triangle", strum=0.04)
    barre = ["FM", "BbM", "Bm", "F#m", "C#M", "AbM"]
    generate("barre_chords", barre, duration=0.6, waveform="triangle",
             strum=0.04)

    # Piano voicings
    print("\nPiano voicings:")
    generate("voicing_cm", Chord("CM"), waveform="sine")
    generate("voicing_dm7", Chord("Dm7"), waveform="sine")
    generate("voicing_g7", Chord("G7"), waveform="sine")

    # Scales
    print("\nScales:")
    generate("scale_c_major", Scale("C", "major"), duration=0.3, gap=0.05,
             waveform="triangle")
    generate("scale_a_blues", Scale("A", "blues"), duration=0.3, gap=0.05,
             waveform="sawtooth")
    generate("scale_c_penta", Scale("C", "major pentatonic"), duration=0.3,
             gap=0.05, waveform="triangle")

    # Progressions
    print("\nProgressions:")
    generate("prog_pop", ["CM", "GM", "Am", "FM"],
             duration=0.8, waveform="triangle", strum=0.04)
    generate("prog_turnaround", ["CM", "Am", "Dm", "G7"],
             duration=0.8, waveform="sine", strum=0.03)
    generate("prog_direct", ["CM", "G7", "CM"],
             duration=0.8, waveform="triangle", strum=0.03)

    # Neo-Riemannian pairs
    print("\nComparison pairs:")
    generate("neoR_P", ["CM", "Cm"], duration=1.2, waveform="sine")
    generate("neoR_L", ["CM", "Em"], duration=1.2, waveform="sine")
    generate("neoR_R", ["CM", "Am"], duration=1.2, waveform="sine")

    # Sequence
    print("\nSequence:")
    seq = Sequence(Tempo(120), TimeSignature(4, 4))
    seq.add(NoteEvent(Note("C"), Duration("quarter")))
    seq.add(NoteEvent(Note("E"), Duration("quarter")))
    seq.add(NoteEvent(Note("G"), Duration("quarter")))
    seq.add(Rest(Duration("quarter")))
    seq.add(ChordEvent(Chord("CM"), Duration("whole")))
    generate("sequence_ceg_cm", seq, waveform="triangle")

    seq251 = Sequence(Tempo(100))
    seq251.add(ChordEvent(Chord("Dm7"), Duration("whole")))
    seq251.add(ChordEvent(Chord("G7"), Duration("whole")))
    seq251.add(ChordEvent(Chord("C7M"), Duration("whole")))
    generate("sequence_251", seq251, waveform="sine")

    # Waveforms
    print("\nWaveforms (Note A):")
    for wf in ["sine", "square", "sawtooth", "triangle"]:
        generate(f"waveform_{wf}", Note("A"), duration=1.0, waveform=wf,
                 octave=4)

    # Case study: Let It Be
    print("\nLet It Be:")
    generate("letitbe", ["CM", "GM", "Am", "FM"],
             duration=0.8, waveform="triangle", strum=0.05)
    generate("letitbe_solo", Scale("C", "major pentatonic"), duration=0.3,
             gap=0.05, waveform="sawtooth")

    # Case study: Comparative tonalities
    print("\nComparative:")
    generate("field_c_major", Field("C", "major"), duration=0.6,
             waveform="triangle")
    generate("field_a_minor", Field("A", "natural minor"), duration=0.6,
             waveform="triangle")


def audio_guide():
    """Audio assets for docs/guia/audio.md."""

    # Play rapido
    print("\nPlay rapido:")
    generate("play_note_c", Note("C"), octave=4)
    generate("play_chord_am7", Chord("Am7"), waveform="sine", strum=0.03)
    generate("play_scale_cmaj", Scale("C", "major"), duration=0.3, gap=0.05)
    generate("play_field_cmaj", Field("C", "major"), duration=0.6)
    generate("play_list_pop", ["CM", "Am", "FM", "GM"], duration=0.6,
             waveform="triangle")

    # Strum comparison
    print("\nStrum:")
    for val, label in [(0.0, "0"), (0.03, "003"), (0.08, "008"),
                       (0.15, "015")]:
        generate(f"strum_{label}", Chord("Am7"), strum=val, duration=1.0,
                 waveform="triangle")

    # Gap comparison
    print("\nGap:")
    for val, label in [(0.0, "0"), (0.05, "005"), (0.15, "015")]:
        generate(f"gap_{label}", Scale("C", "major"), gap=val, duration=0.3,
                 waveform="triangle")

    # ADSR envelopes
    print("\nADSR:")
    pad = Envelope(attack=0.3, decay=0.1, sustain=0.8, release=0.5)
    generate("adsr_pad", Chord("Am7"), envelope=pad, duration=2.0,
             waveform="triangle")
    perc = Envelope(attack=0.005, decay=0.2, sustain=0.2, release=0.1)
    generate("adsr_perc", Note("C"), envelope=perc, duration=0.8,
             waveform="triangle")

    # Greek modes
    print("\nModos gregos:")
    for modo in ["ionian", "dorian", "phrygian", "lydian", "mixolydian",
                 "aeolian", "locrian"]:
        generate(f"mode_{modo}", Scale("C", modo), duration=0.25, gap=0.03,
                 waveform="triangle")

    # Scale families
    print("\nFamilias de escalas:")
    for name in ["major", "natural minor", "harmonic minor", "melodic minor",
                 "blues", "whole tone"]:
        safe = name.replace(" ", "_")
        generate(f"family_{safe}", Scale("C", name), duration=0.25, gap=0.03,
                 waveform="triangle")

    # Sequence
    print("\nSequence:")
    seq = Sequence(Tempo(100), TimeSignature(4, 4))
    seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))
    seq.add(NoteEvent(Note("E"), Duration("quarter"), 4))
    seq.add(NoteEvent(Note("G"), Duration("quarter"), 4))
    seq.add(Rest(Duration("quarter")))
    seq.add(ChordEvent(Chord("CM"), Duration("half"), 4))
    generate("seq_melody", seq, waveform="triangle")

    # Jazz progressions
    print("\nJazz:")
    generate("jazz_pop", ["CM", "GM", "Am", "FM"], duration=0.8,
             waveform="triangle")
    generate("jazz_251", ["Dm7", "G7", "C7M"], duration=1.0, strum=0.05,
             waveform="sine")


def main():
    exemplos_praticos()
    audio_guide()
    total = len(list(AUDIO_DIR.glob("*.mp3")))
    size_mb = sum(f.stat().st_size for f in AUDIO_DIR.glob("*.mp3")) / 1048576
    print(f"\nDone! {total} MP3 files ({size_mb:.1f} MB) in {AUDIO_DIR}")


if __name__ == "__main__":
    main()
