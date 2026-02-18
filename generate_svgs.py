#!/usr/bin/env python3
"""Generate SVG files for all harmonic fields across all tonics and scale types."""

import os
from gingo import (
    Fretboard, Chord, Scale, Field, FretboardSVG,
    ScaleType, Layout
)

OUTPUT = "/home/saulo/Libraries/gingo/output"
os.makedirs(OUTPUT, exist_ok=True)

guitar = Fretboard.violao()

tonics = ["C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B"]
scale_types = [
    ("major", ScaleType.Major),
    ("natural_minor", ScaleType.NaturalMinor),
    ("harmonic_minor", ScaleType.HarmonicMinor),
    ("melodic_minor", ScaleType.MelodicMinor),
]

for tonic in tonics:
    for stype_name, stype in scale_types:
        safe_tonic = tonic.replace("#", "s").replace("b", "f")

        try:
            field = Field(tonic, stype)
        except Exception as e:
            print(f"SKIP {tonic} {stype_name}: {e}")
            continue

        # Field with horizontal layout (all chords side by side)
        try:
            svg = FretboardSVG.field(guitar, field, Layout.Horizontal)
            fname = f"{safe_tonic}_{stype_name}_field.svg"
            FretboardSVG.write(svg, os.path.join(OUTPUT, fname))
            print(f"OK {tonic} {stype_name}")
        except Exception as e:
            print(f"ERROR {tonic} {stype_name}: {e}")

print("\nDone! Files in:", OUTPUT)
