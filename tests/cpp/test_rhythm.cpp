// Gingo — Rhythm module tests (Duration, Tempo, TimeSignature, Event, Sequence)
// SPDX-License-Identifier: MIT

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <gingo/duration.hpp>
#include <gingo/tempo.hpp>
#include <gingo/time_signature.hpp>
#include <gingo/event.hpp>
#include <gingo/sequence.hpp>

using namespace gingo;
using Catch::Matchers::WithinAbs;

// ===========================================================================
// Duration
// ===========================================================================

TEST_CASE("Duration construction from name", "[duration]") {
    Duration q("quarter");
    REQUIRE(q.name() == "quarter");
    REQUIRE(q.numerator() == 1);
    REQUIRE(q.denominator() == 4);
    REQUIRE_THAT(q.beats(), WithinAbs(1.0, 1e-9));
}

TEST_CASE("Duration standard names", "[duration]") {
    Duration w("whole");
    REQUIRE_THAT(w.beats(), WithinAbs(4.0, 1e-9));

    Duration h("half");
    REQUIRE_THAT(h.beats(), WithinAbs(2.0, 1e-9));

    Duration e("eighth");
    REQUIRE_THAT(e.beats(), WithinAbs(0.5, 1e-9));

    Duration s("sixteenth");
    REQUIRE_THAT(s.beats(), WithinAbs(0.25, 1e-9));
}

TEST_CASE("Duration from rational", "[duration]") {
    Duration d(1, 4);
    REQUIRE(d.numerator() == 1);
    REQUIRE(d.denominator() == 4);
    REQUIRE(d.name() == "quarter");
}

TEST_CASE("Dotted duration", "[duration]") {
    Duration dq("quarter", 1);  // dotted quarter = 3/8
    REQUIRE(dq.numerator() == 3);
    REQUIRE(dq.denominator() == 8);
    REQUIRE_THAT(dq.beats(), WithinAbs(1.5, 1e-9));
}

TEST_CASE("Double-dotted duration", "[duration]") {
    Duration ddq("quarter", 2);  // double-dotted quarter = 7/16
    REQUIRE(ddq.numerator() == 7);
    REQUIRE(ddq.denominator() == 16);
    REQUIRE_THAT(ddq.beats(), WithinAbs(1.75, 1e-9));
}

TEST_CASE("Triplet duration", "[duration]") {
    Duration t("eighth", 0, 3);  // eighth triplet = 1/12
    REQUIRE(t.numerator() == 1);
    REQUIRE(t.denominator() == 12);
    REQUIRE_THAT(t.beats(), WithinAbs(1.0/3.0, 1e-9));
}

TEST_CASE("Duration addition", "[duration]") {
    Duration q("quarter");
    Duration e("eighth");
    Duration sum = q + e;
    REQUIRE_THAT(sum.beats(), WithinAbs(1.5, 1e-9));
}

TEST_CASE("Duration comparison", "[duration]") {
    Duration q("quarter");
    Duration h("half");
    Duration q2("quarter");

    REQUIRE(q == q2);
    REQUIRE(q != h);
    REQUIRE(q < h);
}

TEST_CASE("Duration midi_ticks", "[duration]") {
    REQUIRE(Duration("quarter").midi_ticks() == 480);
    REQUIRE(Duration("half").midi_ticks() == 960);
    REQUIRE(Duration("whole").midi_ticks() == 1920);
    REQUIRE(Duration("eighth").midi_ticks() == 240);
    REQUIRE(Duration("sixteenth").midi_ticks() == 120);
    REQUIRE(Duration("quarter", 1).midi_ticks() == 720);  // dotted quarter
    // Custom ppqn
    REQUIRE(Duration("quarter").midi_ticks(96) == 96);
    REQUIRE(Duration("half").midi_ticks(96) == 192);
}

TEST_CASE("Duration from_ticks", "[duration]") {
    Duration d = Duration::from_ticks(480, 480);
    REQUIRE(d.name() == "quarter");
    REQUIRE(d.numerator() == 1);
    REQUIRE(d.denominator() == 4);

    Duration d2 = Duration::from_ticks(960, 480);
    REQUIRE(d2.name() == "half");

    Duration d3 = Duration::from_ticks(720, 480);
    REQUIRE_THAT(d3.beats(), WithinAbs(1.5, 1e-9));  // dotted quarter
}

TEST_CASE("Duration invalid name throws", "[duration]") {
    REQUIRE_THROWS_AS(Duration("invalid"), std::invalid_argument);
}

TEST_CASE("Duration standard_names returns all names", "[duration]") {
    auto names = Duration::standard_names();
    REQUIRE(names.size() == 7);
    REQUIRE(names[0] == "whole");
    REQUIRE(names[2] == "quarter");
}

TEST_CASE("Duration from abbreviation", "[duration]") {
    REQUIRE(Duration("q").name() == "quarter");
    REQUIRE(Duration("h").name() == "half");
    REQUIRE(Duration("w").name() == "whole");
    REQUIRE(Duration("e").name() == "eighth");
    REQUIRE(Duration("s").name() == "sixteenth");
    REQUIRE_THAT(Duration("q").beats(), WithinAbs(1.0, 1e-9));
    REQUIRE_THAT(Duration("w").beats(), WithinAbs(4.0, 1e-9));
}

TEST_CASE("Duration dotted abbreviation", "[duration]") {
    Duration dq("q.");
    REQUIRE(dq.name() == "quarter");
    REQUIRE(dq.dots() == 1);
    REQUIRE(dq.numerator() == 3);
    REQUIRE(dq.denominator() == 8);
    REQUIRE_THAT(dq.beats(), WithinAbs(1.5, 1e-9));

    Duration ddh("h..");
    REQUIRE(ddh.name() == "half");
    REQUIRE(ddh.dots() == 2);
    REQUIRE_THAT(ddh.beats(), WithinAbs(3.5, 1e-9));
}

TEST_CASE("Duration LilyPond notation", "[duration]") {
    REQUIRE(Duration("4").name() == "quarter");
    REQUIRE(Duration("8").name() == "eighth");
    REQUIRE(Duration("2").name() == "half");
    REQUIRE(Duration("1").name() == "whole");
    REQUIRE(Duration("16").name() == "sixteenth");
    REQUIRE(Duration("32").name() == "thirty_second");
    REQUIRE(Duration("64").name() == "sixty_fourth");
    REQUIRE_THAT(Duration("4").beats(), WithinAbs(1.0, 1e-9));
}

TEST_CASE("Duration dotted LilyPond notation", "[duration]") {
    Duration d("4.");
    REQUIRE(d.name() == "quarter");
    REQUIRE(d.dots() == 1);
    REQUIRE_THAT(d.beats(), WithinAbs(1.5, 1e-9));

    Duration d2("2..");
    REQUIRE(d2.name() == "half");
    REQUIRE(d2.dots() == 2);
    REQUIRE_THAT(d2.beats(), WithinAbs(3.5, 1e-9));
}

TEST_CASE("Duration from fraction string", "[duration]") {
    Duration d("1/4");
    REQUIRE(d.numerator() == 1);
    REQUIRE(d.denominator() == 4);
    REQUIRE(d.name() == "quarter");

    Duration d2("3/8");
    REQUIRE(d2.numerator() == 3);
    REQUIRE(d2.denominator() == 8);
    REQUIRE_THAT(d2.beats(), WithinAbs(1.5, 1e-9));

    Duration d3("1/1");
    REQUIRE(d3.name() == "whole");
    REQUIRE_THAT(d3.beats(), WithinAbs(4.0, 1e-9));
}

TEST_CASE("Duration explicit dots override parsed dots", "[duration]") {
    // "q" with explicit dots=1 should give dotted quarter
    Duration d("q", 1);
    REQUIRE(d.dots() == 1);
    REQUIRE_THAT(d.beats(), WithinAbs(1.5, 1e-9));

    // "q." with explicit dots=2 — explicit wins
    Duration d2("q.", 2);
    REQUIRE(d2.dots() == 2);
    REQUIRE_THAT(d2.beats(), WithinAbs(1.75, 1e-9));
}

// ===========================================================================
// Tempo
// ===========================================================================

TEST_CASE("Tempo construction from BPM", "[tempo]") {
    Tempo t(120);
    REQUIRE_THAT(t.bpm(), WithinAbs(120.0, 1e-9));
}

TEST_CASE("Tempo construction from marking", "[tempo]") {
    Tempo t("Allegro");
    REQUIRE(t.bpm() > 100);
}

TEST_CASE("Tempo marking resolution", "[tempo]") {
    REQUIRE(Tempo(60).marking() == "Adagio");
    REQUIRE(Tempo(140).marking() == "Allegro");
    REQUIRE(Tempo(220).marking() == "Prestissimo");
}

TEST_CASE("Tempo seconds calculation", "[tempo]") {
    Tempo t(120);
    Duration q("quarter");
    REQUIRE_THAT(t.seconds(q), WithinAbs(0.5, 1e-9));

    Duration h("half");
    REQUIRE_THAT(t.seconds(h), WithinAbs(1.0, 1e-9));
}

TEST_CASE("Tempo ms_per_beat", "[tempo]") {
    Tempo t(120);
    REQUIRE_THAT(t.ms_per_beat(), WithinAbs(500.0, 1e-9));
}

TEST_CASE("Tempo microseconds_per_beat", "[tempo]") {
    REQUIRE(Tempo(120).microseconds_per_beat() == 500000);
    REQUIRE(Tempo(60).microseconds_per_beat() == 1000000);
    REQUIRE(Tempo(240).microseconds_per_beat() == 250000);
}

TEST_CASE("Tempo from_microseconds", "[tempo]") {
    Tempo t = Tempo::from_microseconds(500000);
    REQUIRE_THAT(t.bpm(), WithinAbs(120.0, 1e-9));

    Tempo t2 = Tempo::from_microseconds(1000000);
    REQUIRE_THAT(t2.bpm(), WithinAbs(60.0, 1e-9));

    REQUIRE_THROWS_AS(Tempo::from_microseconds(0), std::invalid_argument);
    REQUIRE_THROWS_AS(Tempo::from_microseconds(-1), std::invalid_argument);
}

TEST_CASE("Tempo invalid BPM throws", "[tempo]") {
    REQUIRE_THROWS_AS(Tempo(0), std::invalid_argument);
    REQUIRE_THROWS_AS(Tempo(-10), std::invalid_argument);
}

// ===========================================================================
// TimeSignature
// ===========================================================================

TEST_CASE("TimeSignature 4/4", "[time_signature]") {
    TimeSignature ts(4, 4);
    REQUIRE(ts.beats_per_bar() == 4);
    REQUIRE(ts.beat_unit() == 4);
    REQUIRE(ts.classification() == "simple");
    REQUIRE(ts.common_name() == "common time");
}

TEST_CASE("TimeSignature 6/8 compound", "[time_signature]") {
    TimeSignature ts(6, 8);
    REQUIRE(ts.classification() == "compound");
}

TEST_CASE("TimeSignature 3/4", "[time_signature]") {
    TimeSignature ts(3, 4);
    REQUIRE(ts.classification() == "simple");
    REQUIRE_THAT(ts.bar_duration().beats(), WithinAbs(3.0, 1e-9));
}

TEST_CASE("TimeSignature 2/2 cut time", "[time_signature]") {
    TimeSignature ts(2, 2);
    REQUIRE(ts.common_name() == "cut time");
}

TEST_CASE("TimeSignature invalid beat unit throws", "[time_signature]") {
    REQUIRE_THROWS_AS(TimeSignature(4, 3), std::invalid_argument);
}

TEST_CASE("TimeSignature equality", "[time_signature]") {
    REQUIRE(TimeSignature(4, 4) == TimeSignature(4, 4));
    REQUIRE(TimeSignature(3, 4) != TimeSignature(4, 4));
}

// ===========================================================================
// NoteEvent
// ===========================================================================

TEST_CASE("NoteEvent construction", "[event]") {
    Note n("C");
    Duration d("quarter");
    NoteEvent e(n, d, 4);

    REQUIRE(e.note().name() == "C");
    REQUIRE(e.octave() == 4);
    REQUIRE(e.duration().name() == "quarter");
}

TEST_CASE("NoteEvent MIDI number", "[event]") {
    NoteEvent c4(Note("C"), Duration("quarter"), 4);
    REQUIRE(c4.midi_number() == 60);

    NoteEvent a4(Note("A"), Duration("quarter"), 4);
    REQUIRE(a4.midi_number() == 69);

    NoteEvent c5(Note("C"), Duration("quarter"), 5);
    REQUIRE(c5.midi_number() == 72);
}

TEST_CASE("NoteEvent frequency", "[event]") {
    NoteEvent a4(Note("A"), Duration("quarter"), 4);
    REQUIRE_THAT(a4.frequency(), WithinAbs(440.0, 0.01));
}

TEST_CASE("NoteEvent equality", "[event]") {
    NoteEvent a(Note("C"), Duration("quarter"), 4);
    NoteEvent b(Note("C"), Duration("quarter"), 4);
    NoteEvent c(Note("D"), Duration("quarter"), 4);

    REQUIRE(a == b);
    REQUIRE(a != c);
}

// ===========================================================================
// ChordEvent
// ===========================================================================

TEST_CASE("ChordEvent construction", "[event]") {
    Chord ch("C");
    Duration d("whole");
    ChordEvent e(ch, d);

    REQUIRE(e.chord().name() == "C");
    REQUIRE(e.duration().name() == "whole");
}

TEST_CASE("ChordEvent note_events", "[event]") {
    Chord ch("CM");  // C E G
    Duration d("quarter");
    ChordEvent e(ch, d, 4);

    auto notes = e.note_events();
    REQUIRE(notes.size() == 3);
}

// ===========================================================================
// Rest
// ===========================================================================

TEST_CASE("Rest construction", "[event]") {
    Duration d("quarter");
    Rest r(d);
    REQUIRE(r.duration().name() == "quarter");
}

TEST_CASE("Rest equality", "[event]") {
    REQUIRE(Rest(Duration("quarter")) == Rest(Duration("quarter")));
    REQUIRE(Rest(Duration("quarter")) != Rest(Duration("half")));
}

// ===========================================================================
// Sequence
// ===========================================================================

TEST_CASE("Sequence empty construction", "[sequence]") {
    Sequence seq;
    REQUIRE(seq.size() == 0);
    REQUIRE(seq.empty());
}

TEST_CASE("Sequence add events", "[sequence]") {
    Sequence seq;
    seq.add(NoteEvent(Note("C"), Duration("quarter")));
    seq.add(NoteEvent(Note("E"), Duration("quarter")));
    seq.add(Rest(Duration("half")));

    REQUIRE(seq.size() == 3);
    REQUIRE_THAT(seq.total_duration(), WithinAbs(4.0, 1e-9));
}

TEST_CASE("Sequence total_seconds", "[sequence]") {
    Sequence seq(Tempo(120), TimeSignature(4, 4));
    seq.add(NoteEvent(Note("C"), Duration("quarter")));
    seq.add(NoteEvent(Note("E"), Duration("quarter")));

    // 2 quarter notes at 120 BPM = 2 × 0.5s = 1.0s
    REQUIRE_THAT(seq.total_seconds(), WithinAbs(1.0, 1e-9));
}

TEST_CASE("Sequence bar_count", "[sequence]") {
    Sequence seq(Tempo(120), TimeSignature(4, 4));
    // 4 quarter notes = 1 bar in 4/4
    seq.add(NoteEvent(Note("C"), Duration("quarter")));
    seq.add(NoteEvent(Note("D"), Duration("quarter")));
    seq.add(NoteEvent(Note("E"), Duration("quarter")));
    seq.add(NoteEvent(Note("F"), Duration("quarter")));

    REQUIRE(seq.bar_count() == 1);

    // Add one more = 2 bars
    seq.add(NoteEvent(Note("G"), Duration("quarter")));
    REQUIRE(seq.bar_count() == 2);
}

TEST_CASE("Sequence transpose", "[sequence]") {
    Sequence seq;
    seq.add(NoteEvent(Note("C"), Duration("quarter")));
    seq.add(Rest(Duration("quarter")));

    Sequence transposed = seq.transpose(7);  // up a fifth
    REQUIRE(transposed.size() == 2);
}

TEST_CASE("Sequence remove", "[sequence]") {
    Sequence seq;
    seq.add(NoteEvent(Note("C"), Duration("quarter")));
    seq.add(NoteEvent(Note("E"), Duration("quarter")));
    REQUIRE(seq.size() == 2);

    seq.remove(0);
    REQUIRE(seq.size() == 1);
}

TEST_CASE("Sequence clear", "[sequence]") {
    Sequence seq;
    seq.add(NoteEvent(Note("C"), Duration("quarter")));
    seq.add(NoteEvent(Note("E"), Duration("quarter")));
    seq.clear();
    REQUIRE(seq.empty());
}

TEST_CASE("Sequence with chord events", "[sequence]") {
    Sequence seq(Tempo(120), TimeSignature(4, 4));
    seq.add(ChordEvent(Chord("CM"), Duration("half")));
    seq.add(ChordEvent(Chord("GM"), Duration("half")));

    REQUIRE(seq.size() == 2);
    REQUIRE_THAT(seq.total_duration(), WithinAbs(4.0, 1e-9));
    REQUIRE(seq.bar_count() == 1);
}
