# SPDX-License-Identifier: MIT
"""Gingo CLI — explore music theory from the terminal.

Usage:
    python -m gingo note C#
    python -m gingo interval 5J
    python -m gingo chord Cmaj7
    python -m gingo scale "C major"
    python -m gingo field "C major"
"""

import argparse
import sys
import textwrap

from gingo import (
    Note,
    Interval,
    Chord,
    Scale,
    Field,
    FieldComparison,
    HarmonicFunction,
    Tree,
    Progression,
    Duration,
    Tempo,
    TimeSignature,
    Piano,
    VoicingStyle,
    MusicXML,
    PianoSVG,
    __version__,
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _join(items, sep="  "):
    return sep.join(str(x) for x in items)


def _header(text):
    print(f"\n  {text}")
    print(f"  {'—' * len(text)}")


def _row(label, value, indent=4):
    print(f"{' ' * indent}{label:.<20s} {value}")


def _add_audio_args(parser):
    """Add --play, --wav, --waveform, and --strum arguments to a subparser."""
    parser.add_argument("--play", action="store_true",
                        help="play through audio output")
    parser.add_argument("--wav", metavar="FILE",
                        help="export to WAV file")
    parser.add_argument("--waveform",
                        choices=["sine", "square", "sawtooth", "triangle"],
                        default="sine",
                        help="waveform for --play / --wav (default: sine)")
    parser.add_argument("--strum", type=float, default=0.03, metavar="SEC",
                        help="delay between chord notes in seconds (default: 0.03)")
    parser.add_argument("--gap", type=float, default=0.05, metavar="SEC",
                        help="silence between consecutive notes/chords (default: 0.05)")


def _handle_audio(obj, args):
    """Handle --play and --wav flags if present."""
    has_play = getattr(args, "play", False)
    wav_path = getattr(args, "wav", None)
    if not has_play and not wav_path:
        return
    from gingo.audio import play as audio_play, to_wav
    kw = {"waveform": args.waveform, "strum": args.strum, "gap": args.gap}
    if hasattr(args, "octave") and args.octave is not None:
        kw["octave"] = args.octave
    if wav_path:
        to_wav(obj, wav_path, **kw)
        print(f"    Saved: {wav_path}")
    if has_play:
        audio_play(obj, **kw)


# ---------------------------------------------------------------------------
# Subcommands
# ---------------------------------------------------------------------------

def cmd_note(args):
    """Show note properties."""
    n = Note(args.name)

    _header(f"Note: {n.name()}")
    _row("Semitone index", str(n.semitone()))
    tuning = args.tuning or 440.0
    octave = args.octave if args.octave is not None else 4
    freq = n.frequency(octave, tuning)
    label = f"Frequency ({n.name()}{octave}"
    if tuning != 440.0:
        label += f", A4={tuning:.0f}"
    label += ")"
    _row(label, f"{freq:.2f} Hz")
    _row("Natural name", n.natural())
    _row("Sound", n.sound())

    if args.transpose:
        t = n.transpose(args.transpose)
        _row(f"Transpose ({args.transpose:+d} st)", str(t))

    if args.enharmonic:
        other = Note(args.enharmonic)
        eq = n.is_enharmonic(other)
        _row(f"Enharmonic to {args.enharmonic}", "yes" if eq else "no")

    if args.distance:
        other = Note(args.distance)
        d = n.distance(other)
        _row(f"Distance to {args.distance}", f"{d} fifth{'s' if d != 1 else ''}")

    if args.fifths:
        print()
        _header("Circle of fifths")
        fifths = Note.fifths()
        # Find current note's position on the circle
        pos = (n.semitone() * 7) % 12
        items = []
        for i, name in enumerate(fifths):
            marker = f"[{name}]" if i == pos else f" {name} "
            items.append(marker)
        # Print in a circular layout: top row, then two lines
        print(f"    {_join(items, '  ')}")

    _handle_audio(n, args)
    print()


def cmd_interval(args):
    """Show interval properties."""
    try:
        iv = Interval(args.label)
    except (ValueError, RuntimeError):
        try:
            iv = Interval(int(args.label))
        except (ValueError, RuntimeError):
            print(f"  Error: unknown interval '{args.label}'")
            print(f"  Try a label (5J, 3M, 7m) or semitone count (0-23)")
            sys.exit(1)

    _header(f"Interval: {iv.label()}")
    _row("Semitones", str(iv.semitones()))
    _row("Anglo-saxon", iv.anglo_saxon())
    _row("Degree", str(iv.degree()))
    _row("Octave", str(iv.octave()))

    if args.all:
        print()
        _header("All 24 intervals (2 octaves)")
        print(f"    {'ST':>3s}  {'Label':<6s}  {'Anglo-saxon':<10s}  {'Deg':>3s}  {'Oct':>3s}")
        print(f"    {'---':>3s}  {'------':<6s}  {'----------':<10s}  {'---':>3s}  {'---':>3s}")
        for i in range(24):
            x = Interval(i)
            print(f"    {x.semitones():>3d}  {x.label():<6s}  {x.anglo_saxon():<10s}  {x.degree():>3d}  {x.octave():>3d}")

    print()


def cmd_chord(args):
    """Show chord properties."""
    if args.identify:
        notes = [n.strip() for n in args.name.split(",")]
        c = Chord.identify(notes)
        print(f"\n  Identified: {c.name()}")
        print(f"  From notes: {', '.join(notes)}\n")
    else:
        c = Chord(args.name)

    _header(f"Chord: {c.name()}")
    _row("Root", str(c.root()))
    _row("Type", c.type())
    _row("Notes", _join(c.notes(), ", "))
    _row("Formal notes", _join(c.formal_notes(), ", "))
    _row("Intervals", _join(c.intervals(), ", "))
    _row("Size", str(c.size()))
    _handle_audio(c, args)
    print()


def cmd_scale(args):
    """Show scale properties, modes, and pentatonic."""
    if args.identify:
        notes = [n.strip() for n in args.name.split(",")]
        s = Scale.identify(notes)
        print(f"\n  Identified: {s}")
        print(f"  From notes: {', '.join(notes)}")
    else:
        parts = args.name.strip().split(maxsplit=1)
        if len(parts) < 2:
            print(f"  Error: provide tonic and type, e.g. 'C major' or 'A natural minor'")
            sys.exit(1)

        tonic, stype = parts[0], parts[1]

        # Handle --pentatonic flag: append "pentatonic" suffix if not already present
        if args.pentatonic and "pentatonic" not in stype.lower():
            s = Scale(tonic, stype + " pentatonic")
        else:
            s = Scale(tonic, stype)

    _header(f"Scale: {s}")
    _row("Tonic", str(s.tonic()))
    _row("Parent", str(s.parent()).replace("ScaleType.", ""))
    _row("Mode", f"{s.mode_name()} (mode {s.mode_number()})")
    _row("Quality", s.quality())
    brightness = s.brightness()
    if brightness > 0:
        _row("Brightness", str(brightness))
    if s.is_pentatonic():
        _row("Pentatonic", "yes")
    sig = s.signature()
    if sig != 0 or s.parent().name in ("Major", "NaturalMinor"):
        if sig > 0:
            _row("Key signature", f"{sig} sharp{'s' if sig != 1 else ''}")
        elif sig < 0:
            _row("Key signature", f"{-sig} flat{'s' if sig != -1 else ''}")
        else:
            _row("Key signature", "none")
    _row("Size", str(s.size()))
    _row("Notes", _join(s.notes(), "  "))
    _row("Formal notes", _join(s.formal_notes(), "  "))
    _row("Binary mask", "".join(str(x) for x in s.mask()))

    if args.colors:
        cols = s.colors(args.colors)
        if cols:
            _row("Colors vs " + args.colors, _join(cols, "  "))
        else:
            _row("Colors vs " + args.colors, "(none)")

    if args.modes:
        print()
        _header("Modes")
        base = Scale(tonic, stype) if s.is_pentatonic() else s
        # Build the base parent scale (mode 1) for mode listing
        parent_tonic = base.tonic()
        if base.mode_number() != 1:
            # Reconstruct parent scale at mode 1 to list all modes
            parent_name = str(base.parent()).replace("ScaleType.", "").lower()
            base = Scale(str(parent_tonic), parent_name)
        for i in range(1, base.size() + 1):
            m = base.mode(i)
            notes_str = _join(m.notes(), "  ")
            print(f"    {i}. {m.mode_name():<22s} {str(m.tonic()):<4s} {notes_str}")

    if args.degrees:
        print()
        _header("Degrees")
        for i in range(1, s.size() + 1):
            d = s.degree(i)
            print(f"    {i}: {d}")

    if args.degree:
        result = s.degree(*args.degree)
        degs = " → ".join(str(d) for d in args.degree)
        chain = " of ".join(
            ["I", "II", "III", "IV", "V", "VI", "VII"][d - 1]
            if 1 <= d <= 7 else str(d)
            for d in reversed(args.degree)
        )
        print()
        _header(f"Chained Degree: {chain}")
        _row("Input", degs)
        _row("Result", str(result))
        _row("Natural", result.natural())

    if args.walk:
        start = args.walk[0]
        steps = args.walk[1:]
        if not steps:
            print("  Error: --walk requires start + at least one step")
            sys.exit(1)
        result = s.walk(start, *steps)
        print()
        _header(f"Walk: start={start}, steps={', '.join(str(x) for x in steps)}")
        _row("Result", str(result))
        _row("Natural", result.natural())

    if args.relative:
        try:
            rel = s.relative()
            print()
            _header("Relative key")
            _row("Scale", str(rel))
            _row("Notes", _join(rel.notes(), "  "))
        except ValueError as e:
            print(f"\n  {e}")

    if args.parallel:
        try:
            par = s.parallel()
            print()
            _header("Parallel key")
            _row("Scale", str(par))
            _row("Notes", _join(par.notes(), "  "))
        except ValueError as e:
            print(f"\n  {e}")

    if args.neighbors:
        sub, dom = s.neighbors()
        print()
        _header("Neighbors (circle of fifths)")
        _row("Subdominant", f"{sub}  {_join(sub.notes(), '  ')}")
        _row("Dominant", f"{dom}  {_join(dom.notes(), '  ')}")

    _handle_audio(s, args)
    print()


def _parse_applied_target(target_str):
    """Parse the target part of an applied expression.

    Returns an int if it's a number, or a string for Roman numerals.
    """
    try:
        return int(target_str)
    except ValueError:
        return target_str


def cmd_field(args):
    """Show harmonic field (triads and sevenths)."""
    if args.deduce:
        items = [i.strip() for i in args.name.split(",")]
        limit = args.limit
        results = Field.deduce(items, limit=limit)

        _header(f"Deduce from: {', '.join(items)}")
        if not results:
            print("    (no matches found)")
            print()
            return
        print()
        print(f"    {'#':>3}  {'Field':<25s}  {'Score':>6s}  {'Match':>5s}  Roles")
        print(f"    {'---':>3}  {'-----':<25s}  {'-----':>6s}  {'-----':>5s}  -----")
        for i, r in enumerate(results, 1):
            roles_str = ", ".join(x for x in r.roles if x) if r.roles else ""
            print(f"    {i:>3}  {str(r.field):<25s}  {r.score:>6.2f}  {r.matched}/{r.total:<3}  {roles_str}")
        print()
        return

    if args.identify:
        items = [i.strip() for i in args.name.split(",")]
        f = Field.identify(items)
        print(f"\n  Identified: {f}")
        print(f"  From: {', '.join(items)}")
    else:
        parts = args.name.strip().split(maxsplit=1)
        if len(parts) < 2:
            print(f"  Error: provide tonic and type, e.g. 'C major'")
            sys.exit(1)

        tonic, stype = parts[0], parts[1]
        f = Field(tonic, stype)

    scale = f.scale()
    size = scale.size()
    stype = str(scale.type()).replace("ScaleType.", "").lower()
    if scale.mode_name() in ("Aeolian", "NaturalMinor"):
        stype = "natural minor"

    if args.applied:
        # Parse X/Y notation
        if "/" not in args.applied:
            print("  Error: use X/Y notation, e.g. V7/II or 5/2")
            sys.exit(1)

        func_str, target_str = args.applied.rsplit("/", 1)
        target = _parse_applied_target(target_str)

        # Detect numeric function
        try:
            func = int(func_str)
        except ValueError:
            func = func_str

        c = f.applied(func, target)

        _header(f"Applied Chord: {func_str}/{target_str} in {f.tonic()} {stype}")
        _row("Chord", str(c))
        _row("Root", str(c.root()))
        _row("Type", c.type())
        _row("Notes", _join(c.notes(), ", "))
        _row("Formal notes", _join(c.formal_notes(), ", "))
        _row("Intervals", _join(c.intervals(), ", "))
        print()
        return

    sig = f.signature()
    sig_str = (f"{sig}#" if sig > 0 else f"{abs(sig)}b" if sig < 0 else "0")
    _header(f"Harmonic Field: {f.tonic()} {stype}  (sig: {sig_str})")

    roman = ["I", "II", "III", "IV", "V", "VI", "VII", "VIII"]

    if args.functions:
        print()
        print(f"    {'':3s}  {'Triad':<12s}  {'Seventh':<12s}  {'Function':<14s}  {'Role'}")
        print(f"    {'':3s}  {'-----':<12s}  {'-------':<12s}  {'--------':<14s}  {'----'}")

        triads = f.chords()
        sevenths = f.sevenths()
        for i in range(size):
            deg = roman[i] if i < len(roman) else str(i + 1)
            t = triads[i]
            s = sevenths[i]
            hf = f.function(i + 1)
            rl = f.role(i + 1)
            print(f"    {deg:>3s}  {str(t):<12s}  {str(s):<12s}  {hf.name:<14s}  {rl}")
    else:
        print()
        print(f"    {'':3s}  {'Triad':<12s}  {'Seventh':<12s}  {'Notes (triad)':<20s}  {'Notes (7th)'}")
        print(f"    {'':3s}  {'-----':<12s}  {'-------':<12s}  {'-------------':<20s}  {'-----------'}")

        triads = f.chords()
        sevenths = f.sevenths()
        for i in range(size):
            deg = roman[i] if i < len(roman) else str(i + 1)
            t = triads[i]
            s = sevenths[i]
            t_notes = _join(t.notes(), " ")
            s_notes = _join(s.notes(), " ")
            print(f"    {deg:>3s}  {str(t):<12s}  {str(s):<12s}  {t_notes:<20s}  {s_notes}")

    if args.relative:
        try:
            rel = f.relative()
            print()
            _header("Relative field")
            _row("Field", repr(rel))
            _row("Triads", _join(rel.chords(), "  "))
            _row("Sevenths", _join(rel.sevenths(), "  "))
        except ValueError as e:
            print(f"\n  {e}")

    if args.parallel:
        try:
            par = f.parallel()
            print()
            _header("Parallel field")
            _row("Field", repr(par))
            _row("Triads", _join(par.chords(), "  "))
            _row("Sevenths", _join(par.sevenths(), "  "))
        except ValueError as e:
            print(f"\n  {e}")

    if args.neighbors:
        sub, dom = f.neighbors()
        print()
        _header("Neighbors (circle of fifths)")
        _row("Subdominant", f"{repr(sub)}")
        _row("  Triads", _join(sub.chords(), "  "))
        _row("Dominant", f"{repr(dom)}")
        _row("  Triads", _join(dom.chords(), "  "))

    _handle_audio(f, args)
    print()


def cmd_compare(args):
    """Compare two chords (absolute or within a harmonic field)."""
    a = Chord(args.chord_a)
    b = Chord(args.chord_b)

    # Absolute comparison (always shown)
    r = a.compare(b)

    _header(f"Compare: {a.name()} vs {b.name()}")
    _row("Common notes", _join(r.common_notes, ", ") if r.common_notes else "(none)")
    if r.exclusive_a:
        _row("Exclusive A", _join(r.exclusive_a, ", "))
    if r.exclusive_b:
        _row("Exclusive B", _join(r.exclusive_b, ", "))
    _row("Root distance", str(r.root_distance))
    _row("Root direction", f"{r.root_direction:+d}")
    _row("Same quality", "yes" if r.same_quality else "no")
    _row("Same size", "yes" if r.same_size else "no")
    if r.common_intervals:
        _row("Common intervals", _join(r.common_intervals, ", "))
    _row("Enharmonic", "yes" if r.enharmonic else "no")
    if r.subset:
        _row("Subset", r.subset)
    if r.voice_leading >= 0:
        _row("Voice leading", f"{r.voice_leading} st")
    else:
        _row("Voice leading", "N/A (different sizes)")
    if r.transformation:
        names = {
            "P": "Parallel", "L": "Leading-tone", "R": "Relative",
            "RP": "Relative + Parallel", "LP": "Leading-tone + Parallel",
            "PL": "Parallel + Leading-tone", "PR": "Parallel + Relative",
            "LR": "Leading-tone + Relative", "RL": "Relative + Leading-tone",
        }
        label = f"{r.transformation} ({names.get(r.transformation, r.transformation)})"
        _row("Transformation", label)
    if r.inversion:
        _row("Inversion", "yes")

    _row("Interval vector A", str(r.interval_vector_a))
    _row("Interval vector B", str(r.interval_vector_b))
    if r.same_interval_vector:
        _row("Same interval vector", "yes")

    if r.transposition >= 0:
        _row("Transposition", f"T{r.transposition}")

    _row("Dissonance A", f"{r.dissonance_a:.4f}")
    _row("Dissonance B", f"{r.dissonance_b:.4f}")

    # Contextual comparison (if --field is given)
    if args.field:
        parts = args.field.strip().split(maxsplit=1)
        if len(parts) < 2:
            print(f"  Error: --field requires 'tonic type', e.g. 'C major'")
            sys.exit(1)

        tonic, stype = parts[0], parts[1]
        f = Field(tonic, stype)
        fc = f.compare(a, b)

        print()
        _header(f"Context: {f.tonic()} {stype}")

        def _deg_label(deg):
            roman = ["I", "II", "III", "IV", "V", "VI", "VII"]
            return roman[deg - 1] if deg and 1 <= deg <= 7 else str(deg)

        if fc.degree_a is not None:
            _row("Degree A", _deg_label(fc.degree_a))
        else:
            _row("Degree A", "non-diatonic")
        if fc.degree_b is not None:
            _row("Degree B", _deg_label(fc.degree_b))
        else:
            _row("Degree B", "non-diatonic")

        if fc.function_a is not None:
            _row("Function A", fc.function_a.name)
        if fc.function_b is not None:
            _row("Function B", fc.function_b.name)
        if fc.same_function is not None:
            _row("Same function", "yes" if fc.same_function else "no")

        _row("Diatonic A", "yes" if fc.diatonic_a else "no")
        _row("Diatonic B", "yes" if fc.diatonic_b else "no")

        if fc.degree_distance is not None:
            _row("Degree distance", str(fc.degree_distance))
        if fc.relative:
            _row("Relative pair", "yes")
        if fc.root_motion:
            _row("Root motion", fc.root_motion)
        if fc.secondary_dominant:
            _row("Secondary dominant", fc.secondary_dominant)
        if fc.applied_diminished:
            _row("Applied diminished", fc.applied_diminished)
        if fc.tritone_sub:
            _row("Tritone sub", "yes")
        if fc.chromatic_mediant:
            _row("Chromatic mediant", fc.chromatic_mediant)

        if fc.borrowed_a is not None:
            _row("Borrowed A", f"deg {fc.borrowed_a.degree} of {fc.borrowed_a.scale_type}")
        if fc.borrowed_b is not None:
            _row("Borrowed B", f"deg {fc.borrowed_b.degree} of {fc.borrowed_b.scale_type}")

        if fc.foreign_a:
            _row("Foreign notes A", _join(fc.foreign_a, ", "))
        if fc.foreign_b:
            _row("Foreign notes B", _join(fc.foreign_b, ", "))

        if fc.pivot:
            print()
            _header("Pivot fields")
            for p in fc.pivot:
                print(f"    {p.tonic} {p.scale_type}: "
                      f"{_deg_label(p.degree_a)} / {_deg_label(p.degree_b)}")

    print()


def cmd_tree(args):
    """Show harmonic tree for a tradition."""
    parts = args.name.strip().split(maxsplit=1)
    if len(parts) < 2:
        print(f"  Error: provide tonic and type, e.g. 'C major'")
        sys.exit(1)

    tonic, stype = parts[0], parts[1]
    t = Tree(tonic, stype, args.tradition)

    if args.dot:
        print(t.to_dot(args.functions))
        return

    if args.mermaid:
        print(t.to_mermaid())
        return

    trad = t.tradition()
    _header(f"Tree: {t.tonic()} {stype} [{trad.name}]")
    _row("Tradition", trad.name)
    _row("Description", trad.description)

    branches = t.branches()
    _row("Branches", str(len(branches)))

    if args.validate:
        items = [b.strip() for b in args.validate.split(",")]
        valid = t.is_valid(items)
        print()
        _header(f"Validate: {' -> '.join(items)}")
        _row("Valid", "yes" if valid else "no")
        print()
        return

    if args.paths:
        paths = t.paths(args.paths)
        print()
        _header(f"Paths from: {args.paths}")
        for p in paths:
            chord_notes = _join(p.note_names, " ")
            print(f"    [{p.id}] {p.branch:<16s} {str(p.chord):<12s} {chord_notes}")
        print()
        return

    if args.shortest:
        if len(args.shortest) != 2:
            print("  Error: --shortest requires exactly 2 branches")
            sys.exit(1)
        path = t.shortest_path(args.shortest[0], args.shortest[1])
        print()
        _header(f"Shortest path: {args.shortest[0]} -> {args.shortest[1]}")
        if path:
            print(f"    {' -> '.join(path)}  ({len(path)} steps)")
        else:
            print("    (no path found)")
        print()
        return

    if args.schemas:
        schemas = t.schemas()
        print()
        _header("Schemas")
        for s in schemas:
            prog = " -> ".join(s.branches)
            print(f"    {s.name:<24s} {prog}")
            if s.description:
                print(f"    {'':24s} {s.description}")
        print()
        return

    # Default: show branches with optional functions
    print()
    if args.functions:
        _header("Branches (with harmonic function)")
        print(f"    {'Branch':<16s}  {'Function':<14s}")
        print(f"    {'------':<16s}  {'--------':<14s}")
        for b in branches:
            func = t.function(b)
            print(f"    {b:<16s}  {func.name}")
    else:
        _header("Branches")
        # Print in columns
        cols = 4
        for i in range(0, len(branches), cols):
            row = branches[i:i + cols]
            print(f"    {'  '.join(f'{b:<16s}' for b in row)}")

    _handle_audio(t, args)
    print()


def cmd_progression(args):
    """Show progression analysis."""
    if args.traditions:
        traditions = Progression.traditions()
        _header("Available traditions")
        for trad in traditions:
            print(f"    {trad.name:<20s} {trad.description}")
        print()
        return

    parts = args.name.strip().split(maxsplit=1)
    if len(parts) < 2:
        print(f"  Error: provide tonic and type, e.g. 'C major'")
        sys.exit(1)

    tonic, stype = parts[0], parts[1]
    p = Progression(tonic, stype)

    if args.identify:
        items = [b.strip() for b in args.identify.split(",")]
        m = p.identify(items)
        _header(f"Identify: {' -> '.join(items)}")
        _row("Tradition", m.tradition)
        if m.schema:
            _row("Schema", m.schema)
        _row("Score", f"{m.score:.2f}")
        _row("Transitions", f"{m.matched}/{m.total}")
        print()
        return

    if args.deduce:
        items = [b.strip() for b in args.deduce.split(",")]
        results = p.deduce(items, limit=args.limit)
        _header(f"Deduce from: {' -> '.join(items)}")
        if not results:
            print("    (no matches found)")
            print()
            return
        print()
        print(f"    {'#':>3}  {'Tradition':<16s}  {'Schema':<20s}  {'Score':>6s}  {'Match':>5s}")
        print(f"    {'---':>3}  {'---':<16s}  {'---':<20s}  {'---':>6s}  {'---':>5s}")
        for i, r in enumerate(results, 1):
            print(f"    {i:>3}  {r.tradition:<16s}  {r.schema or '—':<20s}  {r.score:>6.2f}  {r.matched}/{r.total}")
        print()
        return

    if args.predict:
        items = [b.strip() for b in args.predict.split(",")]
        tradition = args.tradition if args.tradition else ""
        routes = p.predict(items, tradition)
        _header(f"Predict after: {' -> '.join(items)}")
        if not routes:
            print("    (no suggestions)")
            print()
            return
        print()
        print(f"    {'Next':<16s}  {'Tradition':<16s}  {'Schema':<20s}  {'Conf':>5s}  Path")
        print(f"    {'----':<16s}  {'---':<16s}  {'---':<20s}  {'----':>5s}  ----")
        for r in routes:
            path_str = " -> ".join(r.path) if r.path else ""
            print(f"    {r.next:<16s}  {r.tradition:<16s}  {r.schema or '—':<20s}  {r.confidence:>5.2f}  {path_str}")
        print()
        return

    # Default: show info
    _header(f"Progression: {p.tonic()} {stype}")
    _row("Tonic", str(p.tonic()))
    _row("Type", str(p.type()).replace("ScaleType.", ""))

    traditions = Progression.traditions()
    print()
    _header("Traditions")
    for trad in traditions:
        tree = p.tree(trad.name)
        n_branches = len(tree.branches())
        n_schemas = len(tree.schemas())
        print(f"    {trad.name:<20s} {n_branches} branches, {n_schemas} schemas")

    print()


def _write_svg(svg, args):
    """Write SVG to file if --svg flag is set."""
    svg_path = getattr(args, "svg", None)
    if not svg_path:
        return
    PianoSVG.write(svg, svg_path)
    print(f"    SVG: {svg_path}")


def cmd_piano(args):
    """Show piano key info, voicings, and reverse identification."""
    piano = Piano(args.keys)

    if args.identify:
        midi_nums = args.identify
        chord = piano.identify(midi_nums)
        _header(f"Identify MIDI: {', '.join(str(m) for m in midi_nums)}")
        _row("Chord", chord.name())
        _row("Root", str(chord.root()))
        _row("Notes", _join(chord.notes(), ", "))
        _write_svg(PianoSVG.midi(piano, midi_nums, compact=args.compact), args)
        _handle_audio(chord, args)
        print()
        return

    if args.scale:
        parts = args.name.strip().split(maxsplit=1)
        if len(parts) < 2:
            print("  Error: provide tonic and type, e.g. 'C major'")
            sys.exit(1)
        tonic, stype = parts[0], parts[1]
        s = Scale(tonic, stype)
        octave = args.octave if args.octave is not None else 4
        keys = piano.scale_keys(s, octave)
        _header(f"Scale keys: {s} (octave {octave})")
        print(f"    {'Note':<6s}  {'MIDI':>4s}  {'Oct':>3s}  {'Key':>3s}  {'Color'}")
        print(f"    {'----':<6s}  {'----':>4s}  {'---':>3s}  {'---':>3s}  {'-----'}")
        for k in keys:
            color = "white" if k.white else "black"
            print(f"    {k.note:<6s}  {k.midi:>4d}  {k.octave:>3d}  {k.position:>3d}  {color}")
        _write_svg(PianoSVG.scale(piano, s, octave, compact=args.compact), args)
        _handle_audio(s, args)
        print()
        return

    # Try as chord first, then as note
    name = args.name
    if name is None:
        print("  Error: provide a note (C4), chord (Am7), or use --identify / --scale")
        sys.exit(1)

    # Check if it looks like a note+octave (e.g. C4, Bb3, F#5)
    # Valid note names: A-G optionally followed by #/b/##/bb, then a single digit
    import re
    is_note = False
    m = re.fullmatch(r'([A-Ga-g][#b♯♭]{0,2})(\d)', name)
    if m:
        try:
            note_part = m.group(1)
            oct_part = int(m.group(2))
            Note(note_part)
            is_note = True
        except (ValueError, RuntimeError):
            pass

    if is_note:
        n = Note(note_part)
        octave = oct_part
        k = piano.key(n, octave)
        _header(f"Piano key: {n.name()}{octave}")
        _row("MIDI number", str(k.midi))
        _row("Octave", str(k.octave))
        _row("Note", k.note)
        _row("Color", "white" if k.white else "black")
        _row("Position", f"{k.position} of {piano.num_keys()}")

        # Show all octaves for this note
        all_keys = piano.keys(n)
        _row("All octaves", _join([f"{kk.note}{kk.octave}(#{kk.position})" for kk in all_keys], "  "))
        _write_svg(PianoSVG.note(piano, n, octave, compact=args.compact), args)
        # Set octave from input so _handle_audio picks it up
        if args.octave is None:
            args.octave = octave
        _handle_audio(n, args)
        print()
        return

    # Try as chord
    try:
        c = Chord(name)
    except (ValueError, RuntimeError):
        print(f"  Error: '{name}' is not a valid note+octave or chord name")
        sys.exit(1)

    octave = args.octave if args.octave is not None else 4

    if args.voicings:
        voicings = piano.voicings(c, octave)
        _header(f"Voicings: {c.name()} (octave {octave})")
        print()
        for v in voicings:
            style_name = str(v.style).replace("VoicingStyle.", "")
            print(f"    {style_name}")
            print(f"    {'Note':<6s}  {'MIDI':>4s}  {'Oct':>3s}  {'Color'}")
            print(f"    {'----':<6s}  {'----':>4s}  {'---':>3s}  {'-----'}")
            for k in v.keys:
                color = "white" if k.white else "black"
                print(f"    {k.note:<6s}  {k.midi:>4d}  {k.octave:>3d}  {color}")
            print()
        _write_svg(PianoSVG.chord(piano, c, octave, compact=args.compact), args)
        _handle_audio(c, args)
        return

    # Single voicing
    style_map = {"close": VoicingStyle.Close, "open": VoicingStyle.Open, "shell": VoicingStyle.Shell}
    style = style_map.get(args.style, VoicingStyle.Close)
    v = piano.voicing(c, octave, style)
    style_name = str(v.style).replace("VoicingStyle.", "")

    _header(f"Piano: {c.name()} ({style_name}, octave {octave})")
    _row("Chord", v.chord_name)
    _row("Style", style_name)
    _row("Inversion", str(v.inversion))
    print()
    print(f"    {'Note':<6s}  {'MIDI':>4s}  {'Oct':>3s}  {'Key':>3s}  {'Color'}")
    print(f"    {'----':<6s}  {'----':>4s}  {'---':>3s}  {'---':>3s}  {'-----'}")
    for k in v.keys:
        color = "white" if k.white else "black"
        print(f"    {k.note:<6s}  {k.midi:>4d}  {k.octave:>3d}  {k.position:>3d}  {color}")

    _write_svg(PianoSVG.chord(piano, c, octave, style, compact=args.compact), args)
    _handle_audio(c, args)
    print()


def cmd_musicxml(args):
    """Generate MusicXML output."""
    subcmd = args.musicxml_type

    if subcmd == "note":
        n = Note(args.name)
        octave = args.octave if args.octave is not None else 4
        xml = MusicXML.note(n, octave, args.type)
    elif subcmd == "chord":
        c = Chord(args.name)
        octave = args.octave if args.octave is not None else 4
        xml = MusicXML.chord(c, octave, args.type)
    elif subcmd == "scale":
        parts = args.name.strip().split(maxsplit=1)
        if len(parts) < 2:
            print("  Error: provide tonic and type, e.g. 'C major'")
            sys.exit(1)
        tonic, stype = parts[0], parts[1]
        s = Scale(tonic, stype)
        octave = args.octave if args.octave is not None else 4
        xml = MusicXML.scale(s, octave, args.type)
    elif subcmd == "field":
        parts = args.name.strip().split(maxsplit=1)
        if len(parts) < 2:
            print("  Error: provide tonic and type, e.g. 'C major'")
            sys.exit(1)
        tonic, stype = parts[0], parts[1]
        f = Field(tonic, stype)
        octave = args.octave if args.octave is not None else 4
        xml = MusicXML.field(f, octave, args.type)
    else:
        print(f"  Error: unknown MusicXML type '{subcmd}'")
        sys.exit(1)

    if args.output:
        MusicXML.write(xml, args.output)
        print(f"  Written: {args.output}")
    else:
        print(xml)


def cmd_duration(args):
    """Show duration properties."""
    if args.name_or_num:
        # Try rational construction (e.g. "1 4" → Duration(1, 4))
        if args.denominator is not None:
            d = Duration(int(args.name_or_num), args.denominator)
        else:
            try:
                d = Duration(args.name_or_num, dots=args.dots, tuplet=args.tuplet)
            except (ValueError, RuntimeError):
                print(f"  Error: unknown duration '{args.name_or_num}'")
                print(f"  Valid names: {', '.join(Duration.standard_names())}")
                sys.exit(1)

        _header(f"Duration: {d.name()}")
        _row("Beats", f"{d.beats():.4g}")
        num, den = d.rational()
        _row("Rational", f"{num}/{den}")
        if args.dots:
            _row("Dots", str(args.dots))
        if args.tuplet:
            _row("Tuplet", str(args.tuplet))

        if args.tempo:
            t = Tempo(args.tempo)
            _row(f"Seconds ({int(t.bpm())} BPM)", f"{t.seconds(d):.4g} s")
            _row("Milliseconds", f"{t.seconds(d) * 1000:.1f} ms")
    elif not args.all:
        print("  Error: provide a duration name (e.g. quarter), or use --all")
        sys.exit(1)

    if args.all:
        print()
        _header("Standard durations")
        print(f"    {'Name':<16s}  {'Beats':>6s}  {'Rational':>10s}")
        print(f"    {'----':<16s}  {'-----':>6s}  {'--------':>10s}")
        for name in Duration.standard_names():
            dd = Duration(name)
            num, den = dd.rational()
            print(f"    {name:<16s}  {dd.beats():>6.4g}  {num:>3d}/{den:<3d}")

    print()


def cmd_tempo(args):
    """Show tempo properties."""
    try:
        t = Tempo(args.value)
    except (ValueError, RuntimeError):
        try:
            t = Tempo(float(args.value))
        except (ValueError, RuntimeError):
            print(f"  Error: unknown tempo '{args.value}'")
            print(f"  Use a BPM number (60-240) or marking (Allegro, Adagio, ...)")
            sys.exit(1)

    _header(f"Tempo: {t.bpm():.0f} BPM")
    _row("BPM", f"{t.bpm():.1f}")
    _row("Marking", t.marking())
    _row("ms per beat", f"{t.ms_per_beat():.1f} ms")
    _row("Quarter note", f"{t.seconds(Duration('quarter')):.4g} s")
    _row("Half note", f"{t.seconds(Duration('half')):.4g} s")
    _row("Whole note", f"{t.seconds(Duration('whole')):.4g} s")
    _row("Eighth note", f"{t.seconds(Duration('eighth')):.4g} s")

    if args.all:
        print()
        _header("Standard tempo markings")
        markings = [
            ("Grave", 35), ("Largo", 50), ("Adagio", 60),
            ("Andante", 80), ("Moderato", 108), ("Allegretto", 120),
            ("Allegro", 140), ("Vivace", 168), ("Presto", 188),
            ("Prestissimo", 210),
        ]
        print(f"    {'Marking':<16s}  {'BPM':>6s}  {'ms/beat':>8s}  {'Quarter (s)':>11s}")
        print(f"    {'-------':<16s}  {'---':>6s}  {'-------':>8s}  {'-----------':>11s}")
        for name, bpm in markings:
            tt = Tempo(bpm)
            print(f"    {name:<16s}  {bpm:>6d}  {tt.ms_per_beat():>8.1f}  {tt.seconds(Duration('quarter')):>11.4g}")

    print()


def cmd_timesig(args):
    """Show time signature properties."""
    if args.beats is not None and args.unit is not None:
        ts = TimeSignature(args.beats, args.unit)

        _header(f"Time Signature: {ts}")
        _row("Beats per bar", str(ts.beats_per_bar()))
        _row("Beat unit", str(ts.beat_unit()))
        _row("Classification", ts.classification())
        common = ts.common_name()
        if common:
            _row("Common name", common)
        bar = ts.bar_duration()
        _row("Bar duration", f"{bar.beats():.4g} beats")

        if args.tempo:
            t = Tempo(args.tempo)
            bar_sec = t.seconds(bar)
            _row(f"Bar at {int(t.bpm())} BPM", f"{bar_sec:.4g} s")
    elif not args.all:
        print("  Error: provide beats and unit (e.g. 4 4), or use --all")
        sys.exit(1)

    if args.all:
        print()
        _header("Common time signatures")
        common_sigs = [
            (4, 4), (3, 4), (2, 4), (2, 2),
            (6, 8), (9, 8), (12, 8),
            (5, 4), (7, 8),
        ]
        print(f"    {'Sig':>5s}  {'Class':<10s}  {'Name':<14s}  {'Bar (beats)':>11s}")
        print(f"    {'---':>5s}  {'-----':<10s}  {'----':<14s}  {'-----------':>11s}")
        for b, u in common_sigs:
            tt = TimeSignature(b, u)
            cn = tt.common_name() or ""
            print(f"    {str(tt):>5s}  {tt.classification():<10s}  {cn:<14s}  {tt.bar_duration().beats():>11.4g}")

    print()


# ---------------------------------------------------------------------------
# Argument parser
# ---------------------------------------------------------------------------

def build_parser():
    parser = argparse.ArgumentParser(
        prog="gingo",
        description=textwrap.dedent("""\
            Gingo — music theory from the terminal.

            Explore notes, intervals, chords, scales, harmonic fields,
            harmonic trees, progressions, rhythm, and instruments.

            Pitch concepts build on each other:
              Note → Interval → Chord → Scale → Field → Tree → Progression

            Instruments:
              Piano (forward: theory → keys, reverse: keys → theory)

            Serialization:
              MusicXML (notation interchange for MuseScore, Finale, Sibelius)

            Rhythm concepts:
              Duration → Tempo → Time Signature

            The chromatic universe has 12 pitch classes.
            Everything else is selection, rotation, and relationship.
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=textwrap.dedent(f"""\
            examples:
              %(prog)s note C#
              %(prog)s note Bb --transpose 7
              %(prog)s note C --fifths
              %(prog)s note C --distance G
              %(prog)s interval 5J
              %(prog)s interval 7 --all
              %(prog)s chord Cmaj7
              %(prog)s chord "C,E,G,B" --identify
              %(prog)s scale "C major"
              %(prog)s scale "C major" --modes
              %(prog)s scale "D dorian"
              %(prog)s scale "D dorian" --colors ionian
              %(prog)s scale "C whole tone"
              %(prog)s scale "C major pentatonic"
              %(prog)s scale "C major" --pentatonic
              %(prog)s scale "C major" --degree 5 5
              %(prog)s scale "C major" --walk 1 4
              %(prog)s scale "C major" --relative
              %(prog)s scale "C major" --neighbors
              %(prog)s scale "C,D,E,F,G,A,B" --identify
              %(prog)s field "C major"
              %(prog)s field "C major" --applied V7/II
              %(prog)s field "C major" --relative
              %(prog)s field "C major" --neighbors
              %(prog)s field "CM,FM,G7" --identify
              %(prog)s field "CM,FM" --deduce
              %(prog)s tree "C major" harmonic_tree
              %(prog)s tree "C major" jazz --schemas
              %(prog)s tree "C major" harmonic_tree --paths I
              %(prog)s progression "C major"
              %(prog)s progression --traditions
              %(prog)s progression "C major" --identify "IIm,V7,I"
              %(prog)s progression "C major" --deduce "I,IIm"
              %(prog)s progression "C major" --predict "I,IIm"
              %(prog)s piano C4
              %(prog)s piano Am7
              %(prog)s piano Am7 --voicings
              %(prog)s piano Am7 --style shell
              %(prog)s piano "C major" --scale
              %(prog)s piano --identify 60 64 67
              %(prog)s musicxml note C
              %(prog)s musicxml chord Am7 -o am7.musicxml
              %(prog)s musicxml scale "C major"
              %(prog)s musicxml field "C major"
              %(prog)s compare CM Am
              %(prog)s compare CM GM --field "C major"
              %(prog)s duration quarter
              %(prog)s duration eighth --dots 1
              %(prog)s duration --all
              %(prog)s tempo 120
              %(prog)s tempo Allegro --all
              %(prog)s timesig 4 4
              %(prog)s timesig 6 8 --tempo 120
              %(prog)s timesig --all

            scale types (parent families):
              major, natural minor, harmonic minor,
              melodic minor, diminished, harmonic major,
              whole tone, augmented, blues, chromatic

            mode names (selected):
              ionian, dorian, phrygian, lydian, mixolydian,
              aeolian, locrian, altered, phrygian dominant,
              lydian dominant, lydian augmented, ...

            interval labels (24 total, 2 octaves):
              P1  2m  2M  3m  3M  4J  d5  5J
              #5  M6  7m  7M  8J  b9  9   #9
              b11 11  #11  5  b13 13  #13 bI

            version: {__version__}
        """),
    )

    sub = parser.add_subparsers(
        dest="command",
        title="commands",
        description="choose what to explore",
    )

    # --- note ---
    p_note = sub.add_parser(
        "note", help="explore a note (pitch class)",
        description=textwrap.dedent("""\
            Show properties of a note: semitone index, frequency,
            enharmonic resolution, and transposition.

            The 12 chromatic pitch classes:
              C  C#  D  D#  E  F  F#  G  G#  A  A#  B

            Flat notation is also accepted:
              Db  Eb  Gb  Ab  Bb

            Double accidentals and unicode work too:
              Ebb  F##  B♭  F♯
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo note C#
              gingo note Bb
              gingo note Bb --transpose 7
              gingo note Db --enharmonic C#
              gingo note C --distance F#
              gingo note G --fifths
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_note.add_argument("name", help="note name (C, C#, Db, Bb, ...)")
    p_note.add_argument("--transpose", type=int, metavar="ST",
                        help="transpose by N semitones")
    p_note.add_argument("--enharmonic", metavar="NOTE",
                        help="check if enharmonic to another note")
    p_note.add_argument("--distance", metavar="NOTE",
                        help="distance to another note on the circle of fifths (0-6)")
    p_note.add_argument("--octave", type=int, metavar="N",
                        help="octave for frequency calculation (default: 4)")
    p_note.add_argument("--tuning", type=float, metavar="HZ",
                        help="A4 reference frequency in Hz (default: 440)")
    p_note.add_argument("--fifths", action="store_true",
                        help="show position on the circle of fifths")
    _add_audio_args(p_note)
    p_note.set_defaults(func=cmd_note)

    # --- interval ---
    p_iv = sub.add_parser(
        "interval", help="explore an interval",
        description=textwrap.dedent("""\
            Show properties of an interval by label or semitone count.

            An interval is the distance between two notes.
            Gingo covers 24 intervals (2 octaves, 0-23 semitones).

            Common intervals:
              P1 = unison (0 st)     3m = minor 3rd (3 st)
              3M = major 3rd (4 st)  4J = perfect 4th (5 st)
              5J = perfect 5th (7 st)  7m = minor 7th (10 st)
              7M = major 7th (11 st)   8J = octave (12 st)

            Use --all to see the full table of 24 intervals.
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo interval 5J
              gingo interval 3M
              gingo interval 7         (by semitone count)
              gingo interval 7 --all   (show all 24)
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_iv.add_argument("label", help="interval label (5J, 3M, 7m) or semitone count (0-23)")
    p_iv.add_argument("--all", action="store_true",
                      help="show all 24 intervals (2 octaves)")
    p_iv.set_defaults(func=cmd_interval)

    # --- chord ---
    p_ch = sub.add_parser(
        "chord", help="explore a chord or identify notes",
        description=textwrap.dedent("""\
            Show chord properties or identify a chord from notes.

            A chord = root note + type suffix. 42 chord types are supported.

            Triads:          M  m  dim  aug  sus4  sus2  5
            Sevenths:        7  7M  m7  m7M  dim7  m7(b5)
            Sixths:          6  m6
            Extended:        9  7M(9)  m9  6(9)  m6(9)
            Altered:         7(b5)  aug7  7(#9)  7(b9)  7(#5)
                             7(b5b9)  7(#5#9)  m7(9)  7M(#5)
            Suspended:       7sus4  7sus2
            Add:             add9  madd9  add11  add13
            Other:           dim7M  m7(11)  7(13)  7M(13)  ...

            With --identify, give comma-separated notes (first = root).
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo chord CM
              gingo chord Am7
              gingo chord Bbdim7
              gingo chord "F#7M(#5)"
              gingo chord "C,E,G,B" --identify
              gingo chord "D,F,A" --identify
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_ch.add_argument("name", help="chord name (Cmaj7, Am, G7) or comma-separated notes")
    p_ch.add_argument("--identify", action="store_true",
                      help="identify chord from comma-separated notes")
    _add_audio_args(p_ch)
    p_ch.set_defaults(func=cmd_chord)

    # --- scale ---
    p_sc = sub.add_parser(
        "scale", help="explore a scale, its modes and degrees",
        description=textwrap.dedent("""\
            Show scale notes, binary mask, modes, and degrees.

            A scale is a selection of notes from the 12 chromatic pitch classes,
            organized around a tonic (root note). Format: "TONIC TYPE".

            Parent scale types:
              major ........... Major / Ionian
              natural minor ... Natural Minor / Aeolian
              harmonic minor .. Harmonic Minor
              melodic minor ... Melodic Minor
              harmonic major .. Harmonic Major
              diminished ...... Diminished / Whole-Half
              whole tone ...... Whole Tone (6 notes)
              augmented ....... Augmented (6 notes)
              blues ........... Blues (6 notes)
              chromatic ....... Chromatic (12 notes)

            Mode names (also accepted as type):
              dorian, phrygian, lydian, mixolydian, locrian,
              altered, phrygian dominant, lydian dominant,
              lydian augmented, dorian b2, mixolydian b6, ...

            Pentatonic variants:
              "C major pentatonic", "D dorian pentatonic", ...

            Tonic can be any note: C, C#, Db, D, Eb, E, F, F#, Gb, G, Ab, A, Bb, B
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo scale "C major"
              gingo scale "C major" --modes
              gingo scale "D dorian"
              gingo scale "D dorian" --colors ionian
              gingo scale "A natural minor" --modes
              gingo scale "C harmonic minor" --modes
              gingo scale "C melodic minor" --modes
              gingo scale "C diminished"
              gingo scale "C whole tone"
              gingo scale "A blues"
              gingo scale "C major pentatonic"
              gingo scale "Bb major" --pentatonic
              gingo scale "F# major" --degrees
              gingo scale "C major" --degree 5         (V = G)
              gingo scale "C major" --degree 5 5       (V of V = D)
              gingo scale "C major" --degree 5 5 3     (III of V of V = F)
              gingo scale "C major" --walk 1 4         (from I, walk 4 = F)
              gingo scale "C major" --walk 5 5         (from V, walk 5 = D)
              gingo scale "C major" --relative
              gingo scale "A natural minor" --parallel
              gingo scale "G major" --neighbors
              gingo scale "C,D,E,F,G,A,B" --identify
              gingo scale "A,B,C,D,E,F,G" --identify
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_sc.add_argument("name", help='"tonic type" — e.g. "C major", "A natural minor", "Bb dim"')
    p_sc.add_argument("--modes", action="store_true",
                      help="show all modes of the scale with traditional names")
    p_sc.add_argument("--degrees", action="store_true",
                      help="show each degree note")
    p_sc.add_argument("--pentatonic", action="store_true",
                      help="use pentatonic modality (5 notes)")
    p_sc.add_argument("--colors", metavar="REF",
                      help="show color notes vs a reference mode (e.g. ionian)")
    p_sc.add_argument("--degree", nargs="+", type=int, metavar="N",
                      help="chained degree: --degree 5 5 = V of V")
    p_sc.add_argument("--walk", nargs="+", type=int, metavar="N",
                      help="walk: start + steps, e.g. --walk 1 4 = from I, walk 4")
    p_sc.add_argument("--relative", action="store_true",
                      help="show the relative major/minor key")
    p_sc.add_argument("--parallel", action="store_true",
                      help="show the parallel major/minor key")
    p_sc.add_argument("--neighbors", action="store_true",
                      help="show neighboring keys on the circle of fifths")
    p_sc.add_argument("--identify", action="store_true",
                      help="identify scale from comma-separated notes")
    _add_audio_args(p_sc)
    p_sc.set_defaults(func=cmd_scale)

    # --- field ---
    p_fi = sub.add_parser(
        "field", help="show the harmonic field (triads and sevenths)",
        description=textwrap.dedent("""\
            Build triads and seventh chords on each degree of a scale.

            A harmonic field shows which chords naturally belong to a scale.
            For each degree (I through VII), it builds a triad (3 notes) and
            a seventh chord (4 notes) using only notes from the scale.

            Format: "TONIC TYPE"

            Scale types (accepted names):
              major, natural minor, harmonic minor,
              melodic minor, diminished, harmonic major,
              whole tone, augmented, blues, chromatic

            Tonic can be any note: C, C#, Db, D, Eb, E, F, F#, Gb, G, Ab, A, Bb, B

            Output columns:
              Degree  |  Triad  |  Seventh  |  Notes (triad)  |  Notes (7th)
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo field "C major"
              gingo field "A natural minor"
              gingo field "C harmonic minor"
              gingo field "D melodic minor"
              gingo field "Bb major"
              gingo field "F# diminished"
              gingo field "C major" --applied V7/II
              gingo field "C major" --applied "IIm7(b5)/V"
              gingo field "C major" --applied 5/2
              gingo field "C major" --functions
              gingo field "C major" --relative
              gingo field "A natural minor" --parallel
              gingo field "G major" --neighbors
              gingo field "CM,Dm,Em,FM,GM,Am" --identify
              gingo field "CM,FM,G7" --identify
              gingo field "C,D,E,F,G,A,B" --identify
              gingo field "CM,FM" --deduce
              gingo field "Am,Dm,E7" --deduce
              gingo field "C,D,E" --deduce
              gingo field "CM,FM" --deduce --limit 5
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_fi.add_argument("name", help='"tonic type" — e.g. "C major", "A natural minor", "Eb dim"')
    p_fi.add_argument("--applied", metavar="X/Y",
                      help="applied chord in X/Y notation (e.g. V7/II, 5/2, IIm7(b5)/V)")
    p_fi.add_argument("--functions", action="store_true",
                      help="show harmonic function (T/S/D) and role for each degree")
    p_fi.add_argument("--relative", action="store_true",
                      help="show the relative major/minor harmonic field")
    p_fi.add_argument("--parallel", action="store_true",
                      help="show the parallel major/minor harmonic field")
    p_fi.add_argument("--neighbors", action="store_true",
                      help="show neighboring harmonic fields on the circle of fifths")
    p_fi.add_argument("--identify", action="store_true",
                      help="identify field from comma-separated notes or chords")
    p_fi.add_argument("--deduce", action="store_true",
                      help="deduce likely fields from partial input (ranked)")
    p_fi.add_argument("--limit", type=int, default=10, metavar="N",
                      help="max results for --deduce (default: 10, 0=all)")
    _add_audio_args(p_fi)
    p_fi.set_defaults(func=cmd_field)

    # --- tree ---
    p_tr = sub.add_parser(
        "tree", help="explore harmonic tree (chord transitions in a tradition)",
        description=textwrap.dedent("""\
            Show the harmonic tree — a directed graph of chord-function
            relationships within a key, for a specific tradition.

            A tree maps all valid chord transitions in a tradition.
            Each branch is a chord function (I, IIm, V7, IV, ...) and
            edges represent allowed transitions between them.

            Format: "TONIC TYPE" TRADITION

            Traditions:
              harmonic_tree ... Alencar's harmonic tree (default)
              jazz ............ Classic jazz chord transitions
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo tree "C major" harmonic_tree
              gingo tree "C major" jazz
              gingo tree "A natural minor" harmonic_tree
              gingo tree "C major" harmonic_tree --functions
              gingo tree "C major" harmonic_tree --paths I
              gingo tree "C major" harmonic_tree --shortest I IV
              gingo tree "C major" harmonic_tree --schemas
              gingo tree "C major" harmonic_tree --validate "I,IIm,V7,I"
              gingo tree "C major" jazz --schemas
              gingo tree "C major" harmonic_tree --dot
              gingo tree "C major" harmonic_tree --dot --functions
              gingo tree "C major" harmonic_tree --mermaid
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_tr.add_argument("name", help='"tonic type" — e.g. "C major", "A natural minor"')
    p_tr.add_argument("tradition", nargs="?", default="harmonic_tree",
                      help="tradition name (default: harmonic_tree)")
    p_tr.add_argument("--paths", metavar="BRANCH",
                      help="show paths from a branch origin (e.g. I)")
    p_tr.add_argument("--shortest", nargs=2, metavar=("FROM", "TO"),
                      help="find shortest path between two branches")
    p_tr.add_argument("--schemas", action="store_true",
                      help="show named progression schemas for this tradition")
    p_tr.add_argument("--functions", action="store_true",
                      help="show harmonic function (T/S/D) for each branch")
    p_tr.add_argument("--validate", metavar="BRANCHES",
                      help="validate a comma-separated progression (e.g. I,IIm,V7,I)")
    p_tr.add_argument("--dot", action="store_true",
                      help="export tree as Graphviz DOT")
    p_tr.add_argument("--mermaid", action="store_true",
                      help="export tree as Mermaid diagram")
    _add_audio_args(p_tr)
    p_tr.set_defaults(func=cmd_tree)

    # --- progression ---
    p_pr = sub.add_parser(
        "progression", help="analyze progressions across traditions",
        description=textwrap.dedent("""\
            Analyze harmonic progressions across multiple traditions.

            A Progression coordinates analysis across all registered traditions
            (harmonic_tree, jazz, ...). It can identify which tradition and
            schema best matches a branch sequence, deduce likely matches from
            partial input, and predict next branches.

            Format: "TONIC TYPE"

            Use --traditions to list available traditions.
            Use --identify, --deduce, or --predict with comma-separated branches.
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo progression "C major"
              gingo progression --traditions
              gingo progression "C major" --identify "I,IIm,V7,I"
              gingo progression "C major" --identify "IIm,V7,I"
              gingo progression "C major" --deduce "I,IIm"
              gingo progression "C major" --deduce "I,IIm" --limit 5
              gingo progression "C major" --predict "I,IIm"
              gingo progression "C major" --predict "I,IIm" --tradition jazz
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_pr.add_argument("name", nargs="?", default=None,
                      help='"tonic type" — e.g. "C major", "A natural minor"')
    p_pr.add_argument("--traditions", action="store_true",
                      help="list available traditions")
    p_pr.add_argument("--identify", metavar="BRANCHES",
                      help="identify best tradition/schema for a comma-separated progression")
    p_pr.add_argument("--deduce", metavar="BRANCHES",
                      help="deduce likely matches from partial comma-separated input")
    p_pr.add_argument("--predict", metavar="BRANCHES",
                      help="predict next branches from comma-separated input")
    p_pr.add_argument("--tradition", metavar="NAME",
                      help="filter prediction to a specific tradition")
    p_pr.add_argument("--limit", type=int, default=10, metavar="N",
                      help="max results for --deduce (default: 10)")
    p_pr.set_defaults(func=cmd_progression)

    # --- piano ---
    p_pi = sub.add_parser(
        "piano", help="explore piano keys, voicings, and MIDI mapping",
        description=textwrap.dedent("""\
            Map music theory to piano keys and back.

            Forward: given a note, chord, or scale, show which piano keys to press.
            Reverse: given MIDI numbers, identify the chord.

            Input formats:
              C4 .......... single note with octave
              Am7 ......... chord (default voicing)
              "C major" ... scale (with --scale flag)
              60 64 67 .... MIDI numbers (with --identify flag)

            Voicing styles:
              close ....... all notes in the same octave (default)
              open ........ root drops one octave
              shell ....... root + 3rd + 7th (jazz voicing)
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo piano C4
              gingo piano Bb3
              gingo piano Am7
              gingo piano Am7 --voicings
              gingo piano Am7 --style open
              gingo piano Am7 --style shell
              gingo piano "C major" --scale
              gingo piano --identify 60 64 67
              gingo piano --identify 57 60 64 67
              gingo piano CM --keys 61
              gingo piano Am7 --svg am7.svg
              gingo piano "C major" --scale --svg cmajor.svg
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_pi.add_argument("name", nargs="?", default=None,
                      help="note+octave (C4), chord (Am7), or 'tonic type' with --scale")
    p_pi.add_argument("--identify", nargs="+", type=int, metavar="MIDI",
                      help="identify chord from MIDI numbers")
    p_pi.add_argument("--scale", action="store_true",
                      help="show scale keys on the piano")
    p_pi.add_argument("--voicings", action="store_true",
                      help="show all voicing styles for a chord")
    p_pi.add_argument("--style", choices=["close", "open", "shell"],
                      default="close",
                      help="voicing style (default: close)")
    p_pi.add_argument("--octave", type=int, metavar="N",
                      help="octave (default: 4)")
    p_pi.add_argument("--keys", type=int, default=88, metavar="N",
                      help="number of piano keys (default: 88)")
    p_pi.add_argument("--svg", metavar="FILE",
                      help="export piano visualization as SVG file")
    p_pi.add_argument("--compact", action="store_true",
                      help="compact SVG: only ~2 octaves around highlighted notes")
    _add_audio_args(p_pi)
    p_pi.set_defaults(func=cmd_piano)

    # --- musicxml ---
    p_mx = sub.add_parser(
        "musicxml", help="generate MusicXML notation",
        description=textwrap.dedent("""\
            Generate MusicXML output for notes, chords, scales, and fields.

            MusicXML is the standard interchange format for music notation.
            Output can be opened in MuseScore, Finale, Sibelius, and other
            notation software.

            Subtypes:
              note ........ single note
              chord ....... chord (all notes stacked)
              scale ....... scale notes in sequence
              field ....... harmonic field (1 chord per measure)
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo musicxml note C
              gingo musicxml note "F#" --octave 5
              gingo musicxml chord Am7
              gingo musicxml chord Am7 -o am7.musicxml
              gingo musicxml scale "C major"
              gingo musicxml scale "C major" -o cmajor.musicxml
              gingo musicxml field "C major"
              gingo musicxml field "C major" -o cfield.musicxml
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_mx.add_argument("musicxml_type", choices=["note", "chord", "scale", "field"],
                      help="what to generate (note, chord, scale, field)")
    p_mx.add_argument("name", help="note (C), chord (Am7), or 'tonic type' for scale/field")
    p_mx.add_argument("-o", "--output", metavar="FILE",
                      help="write to file instead of stdout")
    p_mx.add_argument("--octave", type=int, metavar="N",
                      help="octave (default: 4)")
    p_mx.add_argument("--type", default="quarter",
                      choices=["whole", "half", "quarter", "eighth", "sixteenth"],
                      help="note type / duration (default: quarter)")
    p_mx.set_defaults(func=cmd_musicxml)

    # --- compare ---
    p_cmp = sub.add_parser(
        "compare", help="compare two chords (absolute or contextual)",
        description=textwrap.dedent("""\
            Compare two chords and see their relationship.

            Without --field, shows absolute (context-free) comparison:
            common notes, root distance, voice leading, neo-Riemannian
            transformation (P/L/R), subset, enharmonic equivalence.

            With --field, adds contextual comparison within a harmonic field:
            degrees, functions, secondary dominants, tritone substitution,
            modal borrowing, pivot fields, chromatic mediants.

            Neo-Riemannian transformations (triads only):
              P (Parallel)     CM <-> Cm    same root, change quality
              L (Leading-tone) CM <-> Em    move root by semitone
              R (Relative)     CM <-> Am    relative major/minor
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo compare CM Am
              gingo compare CM Cm
              gingo compare CM Em
              gingo compare CM GM --field "C major"
              gingo compare D7 GM --field "C major"
              gingo compare CM Fm --field "C major"
              gingo compare G7 C#7 --field "C major"
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_cmp.add_argument("chord_a", help="first chord name (CM, Am7, Bbdim, ...)")
    p_cmp.add_argument("chord_b", help="second chord name")
    p_cmp.add_argument("--field", metavar='"T type"',
                        help='harmonic field context, e.g. "C major"')
    p_cmp.set_defaults(func=cmd_compare)

    # --- duration ---
    p_dur = sub.add_parser(
        "duration", help="explore rhythmic durations",
        description=textwrap.dedent("""\
            Show properties of a rhythmic duration.

            A duration represents a note length in beats (quarter = 1 beat).
            Standard durations: whole, half, quarter, eighth, sixteenth,
            thirty_second, sixty_fourth.

            Modifiers:
              --dots N    dotted duration (1 dot = 1.5x, 2 dots = 1.75x)
              --tuplet N  tuplet division (3 = triplet = 2/3 of normal)

            Rational form: numerator/denominator (quarter = 1/4).
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo duration quarter
              gingo duration half
              gingo duration eighth --dots 1
              gingo duration quarter --dots 2
              gingo duration eighth --tuplet 3
              gingo duration quarter --tempo 120
              gingo duration --all
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_dur.add_argument("name_or_num", nargs="?", default=None,
                       help="duration name (quarter, half, ...) or numerator")
    p_dur.add_argument("denominator", nargs="?", type=int, default=None,
                       help="denominator for rational form (e.g. 1 4 = quarter)")
    p_dur.add_argument("--dots", type=int, default=0, metavar="N",
                       help="number of dots (1 = dotted, 2 = double-dotted)")
    p_dur.add_argument("--tuplet", type=int, default=0, metavar="N",
                       help="tuplet division (3 = triplet)")
    p_dur.add_argument("--tempo", type=float, metavar="BPM",
                       help="show real-time duration at given tempo")
    p_dur.add_argument("--all", action="store_true",
                       help="show all standard durations")
    p_dur.set_defaults(func=cmd_duration)

    # --- tempo ---
    p_tmp = sub.add_parser(
        "tempo", help="explore tempo markings and conversions",
        description=textwrap.dedent("""\
            Show tempo properties: BPM, marking, and note durations.

            A tempo can be given as a BPM number or an Italian marking.
            Standard markings: Grave, Largo, Adagio, Andante, Moderato,
            Allegretto, Allegro, Vivace, Presto, Prestissimo.
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo tempo 120
              gingo tempo 60
              gingo tempo Allegro
              gingo tempo Adagio
              gingo tempo 140 --all
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_tmp.add_argument("value", help="BPM number or marking name (Allegro, Adagio, ...)")
    p_tmp.add_argument("--all", action="store_true",
                       help="show all standard tempo markings")
    p_tmp.set_defaults(func=cmd_tempo)

    # --- timesig ---
    p_ts = sub.add_parser(
        "timesig", help="explore time signatures",
        description=textwrap.dedent("""\
            Show time signature properties: classification, bar duration.

            A time signature defines the meter: beats per bar and beat unit.
            Common examples: 4/4 (common time), 3/4 (waltz), 6/8 (compound).

            Classification:
              simple ...... beat divides into 2 (4/4, 3/4, 2/4)
              compound .... beat divides into 3 (6/8, 9/8, 12/8)
        """),
        epilog=textwrap.dedent("""\
            examples:
              gingo timesig 4 4
              gingo timesig 3 4
              gingo timesig 6 8
              gingo timesig 2 2
              gingo timesig 7 8
              gingo timesig 4 4 --tempo 120
              gingo timesig --all
        """),
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    p_ts.add_argument("beats", nargs="?", type=int, default=None,
                      help="beats per bar")
    p_ts.add_argument("unit", nargs="?", type=int, default=None,
                      help="beat unit (denominator)")
    p_ts.add_argument("--tempo", type=float, metavar="BPM",
                      help="show bar duration in seconds at given tempo")
    p_ts.add_argument("--all", action="store_true",
                      help="show common time signatures")
    p_ts.set_defaults(func=cmd_timesig)

    return parser


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = build_parser()
    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        sys.exit(0)

    try:
        args.func(args)
    except (ValueError, RuntimeError) as e:
        print(f"\n  Error: {e}\n", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
