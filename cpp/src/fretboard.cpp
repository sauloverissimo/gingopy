// Gingo — Music Theory Library
// Fretboard implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/fretboard.hpp>

#include <algorithm>
#include <cmath>
#include <functional>
#include <set>
#include <sstream>
#include <stdexcept>

namespace gingo {

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

static const char* CHROMATIC[] = {
    "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
};

std::string Fretboard::midi_to_name(int midi) {
    return CHROMATIC[((midi % 12) + 12) % 12];
}

int Fretboard::midi_to_octave(int midi) {
    return midi / 12 - 1;
}

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

Fretboard::Fretboard(const Tuning& tuning)
    : tuning_(tuning)
{
    if (tuning_.open_midi.empty())
        throw std::invalid_argument("Tuning must have at least one string");
    if (tuning_.num_frets < 1)
        throw std::invalid_argument("num_frets must be at least 1");
}

Fretboard::Fretboard(const std::string& name, const std::vector<int>& open_midi,
                       int num_frets)
    : tuning_{name, open_midi, num_frets}
{
    if (open_midi.empty())
        throw std::invalid_argument("Tuning must have at least one string");
    if (num_frets < 1)
        throw std::invalid_argument("num_frets must be at least 1");
}

// ---------------------------------------------------------------------------
// Factories
// ---------------------------------------------------------------------------

Fretboard Fretboard::cavaquinho(int num_frets) {
    // Standard cavaquinho: D5(74)-B4(71)-G4(67)-D4(62)
    return Fretboard(Tuning{"cavaquinho", {74, 71, 67, 62}, num_frets});
}

Fretboard Fretboard::violao(int num_frets) {
    // Standard guitar: E4(64)-B3(59)-G3(55)-D3(50)-A2(45)-E2(40)
    return Fretboard(Tuning{"violao", {64, 59, 55, 50, 45, 40}, num_frets});
}

Fretboard Fretboard::bandolim(int num_frets) {
    // Standard mandolin: E5(76)-A4(69)-D4(62)-G3(55)
    return Fretboard(Tuning{"bandolim", {76, 69, 62, 55}, num_frets});
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

void Fretboard::validate(int string, int fret) const {
    int n = num_strings();
    if (string < 1 || string > n)
        throw std::out_of_range(
            "String " + std::to_string(string) + " is out of range [1, " +
            std::to_string(n) + "]");
    if (fret < 0 || fret > tuning_.num_frets)
        throw std::out_of_range(
            "Fret " + std::to_string(fret) + " is out of range [0, " +
            std::to_string(tuning_.num_frets) + "]");
}

int Fretboard::compute_midi(int string_idx, int fret) const {
    return tuning_.open_midi[string_idx] + fret;
}

// ---------------------------------------------------------------------------
// Forward: theory → positions
// ---------------------------------------------------------------------------

FretPosition Fretboard::position(int string, int fret) const {
    validate(string, fret);
    int idx = string - 1;
    int midi = compute_midi(idx, fret);
    return FretPosition{
        string,
        fret,
        midi,
        midi_to_name(midi),
        midi_to_octave(midi)
    };
}

std::vector<FretPosition> Fretboard::positions(const Note& note) const {
    std::vector<FretPosition> result;
    int target_pc = note.semitone();

    for (int s = 0; s < num_strings(); ++s) {
        int open = tuning_.open_midi[s];
        // Find first fret with matching pitch class.
        int offset = ((target_pc - (open % 12)) % 12 + 12) % 12;
        for (int f = offset; f <= tuning_.num_frets; f += 12) {
            int midi = open + f;
            result.push_back(FretPosition{
                s + 1, f, midi, midi_to_name(midi), midi_to_octave(midi)
            });
        }
    }

    // Sort by string then fret.
    std::sort(result.begin(), result.end(),
              [](const FretPosition& a, const FretPosition& b) {
                  if (a.string != b.string) return a.string < b.string;
                  return a.fret < b.fret;
              });
    return result;
}

Note Fretboard::note_at(int string, int fret) const {
    validate(string, fret);
    int midi = compute_midi(string - 1, fret);
    return Note(midi_to_name(midi));
}

int Fretboard::midi_at(int string, int fret) const {
    validate(string, fret);
    return compute_midi(string - 1, fret);
}

std::vector<FretPosition> Fretboard::scale_positions(const Scale& scale) const {
    return scale_positions(scale, 0, tuning_.num_frets);
}

std::vector<FretPosition> Fretboard::scale_positions(const Scale& scale,
                                                       int fret_lo, int fret_hi) const {
    auto notes = scale.notes();
    std::set<int> pcs;
    for (const auto& n : notes) pcs.insert(n.semitone());

    std::vector<FretPosition> result;

    for (int s = 0; s < num_strings(); ++s) {
        for (int f = fret_lo; f <= std::min(fret_hi, tuning_.num_frets); ++f) {
            int midi = compute_midi(s, f);
            int pc = ((midi % 12) + 12) % 12;
            if (pcs.count(pc)) {
                result.push_back(FretPosition{
                    s + 1, f, midi, midi_to_name(midi), midi_to_octave(midi)
                });
            }
        }
    }

    std::sort(result.begin(), result.end(),
              [](const FretPosition& a, const FretPosition& b) {
                  if (a.string != b.string) return a.string < b.string;
                  return a.fret < b.fret;
              });
    return result;
}

// ---------------------------------------------------------------------------
// Fingering generation
// ---------------------------------------------------------------------------

double Fretboard::score_fingering(const Fingering& f, int root_pc) {
    double score = 0.0;
    int n = static_cast<int>(f.strings.size());

    int muted = 0;
    int fretted_count = 0;
    int min_fret = 999, max_fret = 0;
    int open_count = 0;

    // Find the first and last sounding string indices (0-based, strings[0]=str1=highest).
    int first_sounding = -1;
    int last_sounding = -1;
    for (int i = 0; i < n; ++i) {
        if (f.strings[i].action != StringAction::Muted) {
            if (first_sounding < 0) first_sounding = i;
            last_sounding = i;
        }
    }

    for (int i = 0; i < n; ++i) {
        const auto& ss = f.strings[i];
        if (ss.action == StringAction::Muted) {
            muted++;
            // Inner muted strings (between sounding strings) are very bad.
            if (first_sounding >= 0 && i > first_sounding && i < last_sounding) {
                score += 50.0;
            }
        } else if (ss.action == StringAction::Open) {
            open_count++;
        } else {
            fretted_count++;
            if (ss.fret < min_fret) min_fret = ss.fret;
            if (ss.fret > max_fret) max_fret = ss.fret;
        }
    }

    // --- Penalties ---

    // Muted strings at the edges (bass or treble) are normal;
    // still a small cost because fewer notes = thinner sound.
    score += muted * 3.0;

    // Fret span: how far the fingers need to stretch.
    // Span 0-2 is comfortable, 3 is normal for barre shapes,
    // 4+ starts getting difficult.
    int span = (fretted_count > 0) ? (max_fret - min_fret) : 0;
    score += span * 4.0;

    // Hard stretch: span > 3 frets is very difficult.
    if (fretted_count >= 2 && span > 3) {
        score += (span - 3) * 20.0;
    }

    // Higher positions: moderate linear penalty. Each fret up costs
    // a fixed amount. This ensures lower-position barre chords beat
    // higher-position ones with the same number of strings.
    int actual_pos = (min_fret < 999) ? min_fret : 0;
    score += actual_pos * 2.0;

    // Count actual fingers needed.
    // Pestana (barré) = 1 finger laid flat at min_fret across all
    // consecutive fretted strings. Higher frets on top = 1 extra
    // finger per distinct fret value. Strings outside the run = 1 each.
    int fingers_used = 0;
    {
        // Find longest consecutive fretted run.
        int b_start = -1, b_len = 0, r_start = -1, r_len = 0;
        for (int i = 0; i < n; ++i) {
            if (f.strings[i].action == StringAction::Fretted) {
                if (r_len == 0) r_start = i;
                ++r_len;
                if (r_len > b_len) { b_start = r_start; b_len = r_len; }
            } else {
                r_len = 0;
            }
        }
        // Check if barré is possible (2+ strings at min_fret in run).
        int at_min = 0;
        std::set<int> higher_in_run;
        if (b_start >= 0 && min_fret < 999) {
            for (int i = b_start; i < b_start + b_len; ++i) {
                if (f.strings[i].fret == min_fret) at_min++;
                else higher_in_run.insert(f.strings[i].fret);
            }
        }
        bool has_barre = (b_len >= 2 && at_min >= 2);
        if (has_barre) {
            fingers_used = 1 + static_cast<int>(higher_in_run.size());
            for (int i = 0; i < n; ++i) {
                if (f.strings[i].action == StringAction::Fretted &&
                    (i < b_start || i >= b_start + b_len))
                    fingers_used++;
            }
        } else {
            for (int i = 0; i < n; ++i) {
                if (f.strings[i].action == StringAction::Fretted)
                    fingers_used++;
            }
        }
    }
    // Penalize many fingers: 3 fingers is comfortable, 4 is harder.
    if (fingers_used > 3) {
        score += (fingers_used - 3) * 15.0;
    }

    // Fret gap between adjacent fretted strings: penalize when fingers
    // are far apart on neighboring strings (hard to reach).
    {
        int prev_fret = -1;
        int prev_idx = -1;
        for (int i = 0; i < n; ++i) {
            if (f.strings[i].action == StringAction::Fretted) {
                if (prev_fret >= 0) {
                    int gap = std::abs(f.strings[i].fret - prev_fret);
                    if (gap >= 3) {
                        score += (gap - 2) * 15.0;
                    }
                    // Extra penalty when fretted strings are separated by
                    // open/muted strings AND at different frets. This
                    // "leapfrog" pattern (e.g., Bm = 2-0-4-0-2) is
                    // uncomfortable because fingers must skip over strings.
                    int string_gap = i - prev_idx;
                    if (string_gap >= 2 && gap >= 2) {
                        score += gap * 5.0;
                    }
                }
                prev_fret = f.strings[i].fret;
                prev_idx = i;
            }
        }
    }

    // Open strings mixed with high frets are impractical. E.g., playing
    // frets 4-5 with an open string in between is uncomfortable because
    // the hand is positioned far from the nut.
    if (open_count > 0 && max_fret >= 4) {
        score += 30.0;
    }

    // --- Rewards ---

    // Open strings are easy to play (no finger needed).
    score -= open_count * 2.0;

    // More sounding strings = fuller chord. Strong reward so that
    // barre chords with 5–6 strings beat incomplete voicings.
    int sounding = n - muted;
    score -= sounding * 5.0;

    // --- Bass note analysis ---

    // The lowest sounding note should be the root for a strong sound.
    if (root_pc >= 0 && !f.midi_notes.empty()) {
        int bass_midi = f.midi_notes[0];  // midi_notes sorted ascending
        int bass_pc = ((bass_midi % 12) + 12) % 12;
        if (bass_pc != root_pc) {
            score += 20.0;  // Non-root bass sounds weak/inverted.
        }
    }

    return score;
}

std::vector<Fingering> Fretboard::generate_fingerings(
        const Chord& chord, int base_pos, int max_span) const {
    auto chord_notes = chord.notes();
    if (chord_notes.empty()) return {};

    int root_pc = chord_notes[0].semitone();
    int n_strings = num_strings();

    // Collect target pitch classes.
    std::set<int> target_pcs;
    for (const auto& n : chord_notes) target_pcs.insert(n.semitone());

    // For each string, find valid frets within the window [base_pos, base_pos+max_span].
    // Also consider open strings (fret 0) and muted.
    struct StringOption {
        int fret;           // -1 = muted
        int pc;             // pitch class (-1 for muted)
        StringAction action;
    };

    std::vector<std::vector<StringOption>> options(n_strings);

    for (int s = 0; s < n_strings; ++s) {
        // Option: muted
        options[s].push_back({-1, -1, StringAction::Muted});

        // Option: open string (fret 0)
        int open_midi = tuning_.open_midi[s];
        int open_pc = ((open_midi % 12) + 12) % 12;
        if (target_pcs.count(open_pc)) {
            options[s].push_back({0, open_pc, StringAction::Open});
        }

        // Options within fret window.
        int lo = std::max(1, base_pos);
        int hi = std::min(base_pos + max_span, tuning_.num_frets);
        for (int f = lo; f <= hi; ++f) {
            int midi = compute_midi(s, f);
            int pc = ((midi % 12) + 12) % 12;
            if (target_pcs.count(pc)) {
                options[s].push_back({f, pc, StringAction::Fretted});
            }
        }
    }

    // Enumerate combinations (DFS with pruning).
    std::vector<Fingering> results;
    std::vector<StringOption> current(n_strings);

    // Recursive helper — lambda as a std::function for self-reference.
    // We enumerate from the last string (lowest pitch) to the first.
    std::function<void(int)> enumerate = [&](int s) {
        if (s < 0) {
            // Check that we cover all pitch classes.
            std::set<int> covered;
            int sounding = 0;
            for (int i = 0; i < n_strings; ++i) {
                if (current[i].pc >= 0) {
                    covered.insert(current[i].pc);
                    sounding++;
                }
            }
            if (covered.size() < target_pcs.size()) return;
            if (sounding < 2) return;

            // Reject fingerings with inner muted strings (muted between
            // sounding strings). This is physically impossible or at least
            // highly impractical.
            int first_sounding = -1, last_sounding = -1;
            for (int i = 0; i < n_strings; ++i) {
                if (current[i].action != StringAction::Muted) {
                    if (first_sounding < 0) first_sounding = i;
                    last_sounding = i;
                }
            }
            for (int i = first_sounding + 1; i < last_sounding; ++i) {
                if (current[i].action == StringAction::Muted) return;
            }

            // Count fingers required (max 4 available).
            //
            // A barré (pestana) = one finger laid flat across ALL
            // consecutive fretted strings at the lowest fret. Strings
            // with higher frets are pressed ON TOP of the barré by
            // other fingers, so they do NOT break the barré span.
            //
            // Example: FM = [1,1,2,3,3,1] — barré spans all 6 strings
            //   at fret 1 (1 finger), fret 2 (1 finger), fret 3 (1 finger)
            //   = 3 fingers total (not 6).
            //
            // Algorithm:
            //   1. Find lo_fret = lowest fretted position.
            //   2. Find longest consecutive run of fretted strings
            //      (any fret >= lo_fret, including higher). A fretted
            //      string at a higher fret doesn't break the run because
            //      the barré finger is underneath.
            //   3. If run >= 2 and at least 2 strings are at lo_fret:
            //      barré = 1 finger. Each distinct higher fret within
            //      the run = 1 extra finger. Fretted strings outside
            //      the run = 1 finger each.
            //   4. Total > 4 → reject.
            {
                int lo_fret = 999;
                for (int i2 = 0; i2 < n_strings; ++i2) {
                    if (current[i2].action == StringAction::Fretted &&
                        current[i2].fret < lo_fret)
                        lo_fret = current[i2].fret;
                }

                // Find longest consecutive run of fretted strings.
                // Higher frets do NOT break the run (barré is underneath).
                int best_start = -1, best_len = 0;
                int run_start = -1, run_len = 0;
                for (int i2 = 0; i2 < n_strings; ++i2) {
                    if (current[i2].action == StringAction::Fretted) {
                        if (run_len == 0) run_start = i2;
                        ++run_len;
                        if (run_len > best_len) {
                            best_start = run_start;
                            best_len = run_len;
                        }
                    } else {
                        run_len = 0;
                    }
                }

                // Count strings at lo_fret within the best run.
                int at_lo_in_run = 0;
                std::set<int> higher_frets_in_run;
                if (best_start >= 0) {
                    for (int i2 = best_start; i2 < best_start + best_len; ++i2) {
                        if (current[i2].fret == lo_fret)
                            at_lo_in_run++;
                        else
                            higher_frets_in_run.insert(current[i2].fret);
                    }
                }

                int fingers_needed;
                bool has_barre = (best_len >= 2 && at_lo_in_run >= 2);

                if (has_barre) {
                    // Barré = 1 finger (covers all strings in the run at lo_fret).
                    // Each distinct higher fret in the run = 1 extra finger.
                    fingers_needed = 1 + static_cast<int>(higher_frets_in_run.size());
                    // Fretted strings OUTSIDE the run = 1 finger each.
                    for (int i2 = 0; i2 < n_strings; ++i2) {
                        if (current[i2].action == StringAction::Fretted &&
                            (i2 < best_start || i2 >= best_start + best_len))
                            fingers_needed++;
                    }
                } else {
                    // No barré: every fretted string = 1 finger.
                    fingers_needed = 0;
                    for (int i2 = 0; i2 < n_strings; ++i2) {
                        if (current[i2].action == StringAction::Fretted)
                            fingers_needed++;
                    }
                }

                if (fingers_needed > 4) return;
            }

            // Build Fingering.
            Fingering f;
            f.chord_name = chord.name();
            f.capo = 0;

            int min_fret = 999, max_fret = 0;
            for (int i = 0; i < n_strings; ++i) {
                StringState ss;
                ss.string = i + 1;
                ss.action = current[i].action;
                ss.fret = current[i].fret;
                ss.finger = 0;
                f.strings.push_back(ss);

                if (ss.action != StringAction::Muted) {
                    int midi = compute_midi(i, std::max(0, ss.fret));
                    f.midi_notes.push_back(midi);
                }
                if (ss.action == StringAction::Fretted) {
                    if (ss.fret < min_fret) min_fret = ss.fret;
                    if (ss.fret > max_fret) max_fret = ss.fret;
                }
            }

            // base_fret: if any open strings are used, we're at position 1
            // (show the nut). Otherwise use the lowest fretted position.
            bool has_open = false;
            for (const auto& ss : f.strings) {
                if (ss.action == StringAction::Open) { has_open = true; break; }
            }
            if (has_open) {
                f.base_fret = 1;
            } else {
                f.base_fret = (min_fret < 999) ? min_fret : 1;
            }

            // Detect barré: the barré finger lays flat at min_fret across
            // consecutive fretted strings. Higher frets on top don't break
            // the span. We need 2+ strings at min_fret in the span.
            f.barre = 0;
            if (min_fret < 999 && min_fret > 0) {
                // Find longest consecutive fretted run.
                int b_start = -1, b_len = 0, r_start = -1, r_len = 0;
                for (int i = 0; i < n_strings; ++i) {
                    if (f.strings[i].action == StringAction::Fretted) {
                        if (r_len == 0) r_start = i;
                        r_len++;
                        if (r_len > b_len) { b_start = r_start; b_len = r_len; }
                    } else {
                        r_len = 0;
                    }
                }
                // Count strings at min_fret within the run.
                int at_min = 0;
                if (b_start >= 0) {
                    for (int i = b_start; i < b_start + b_len; ++i) {
                        if (f.strings[i].fret == min_fret) at_min++;
                    }
                }
                if (b_len >= 2 && at_min >= 2) f.barre = min_fret;
            }

            // Sort midi_notes.
            std::sort(f.midi_notes.begin(), f.midi_notes.end());

            results.push_back(std::move(f));
            return;
        }

        for (const auto& opt : options[s]) {
            current[s] = opt;
            enumerate(s - 1);
        }
    };

    enumerate(n_strings - 1);
    return results;
}

Fingering Fretboard::fingering(const Chord& chord, int position) const {
    auto chord_notes = chord.notes();
    int root_pc = chord_notes.empty() ? -1 : chord_notes[0].semitone();

    // Scan the entire fretboard to find the globally best voicing.
    // The position hint biases the search but doesn't restrict it.
    std::vector<Fingering> all;

    for (int p = 0; p <= tuning_.num_frets - 3; p++) {
        auto batch = generate_fingerings(chord, p);
        for (auto& f : batch) all.push_back(std::move(f));
    }

    if (all.empty())
        throw std::invalid_argument(
            "Cannot find fingering for " + chord.name() + " on " + name());

    // Score, deduplicate, and pick the best.
    std::sort(all.begin(), all.end(),
              [root_pc](const Fingering& a, const Fingering& b) {
                  return score_fingering(a, root_pc) < score_fingering(b, root_pc);
              });

    // Deduplicate by fret signature (same voicing from different windows).
    std::set<std::vector<int>> seen;
    for (const auto& f : all) {
        std::vector<int> sig;
        for (const auto& ss : f.strings) sig.push_back(ss.fret);
        if (seen.insert(sig).second) return f;
    }
    return all[0];
}

std::vector<Fingering> Fretboard::fingerings(const Chord& chord,
                                               int max_results) const {
    auto chord_notes = chord.notes();
    int root_pc = chord_notes.empty() ? -1 : chord_notes[0].semitone();

    std::vector<Fingering> all;

    // Scan positions along the fretboard.
    for (int p = 0; p <= tuning_.num_frets - 3; p++) {
        auto batch = generate_fingerings(chord, p);
        for (auto& f : batch) all.push_back(std::move(f));
    }

    if (all.empty())
        throw std::invalid_argument(
            "Cannot find fingering for " + chord.name() + " on " + name());

    // Score, deduplicate by midi_notes, and pick top N.
    std::sort(all.begin(), all.end(),
              [root_pc](const Fingering& a, const Fingering& b) {
                  return score_fingering(a, root_pc) < score_fingering(b, root_pc);
              });

    // Deduplicate: same midi_notes set → keep only the best scored.
    std::vector<Fingering> unique;
    std::set<std::vector<int>> seen;
    for (auto& f : all) {
        // Build a signature: sorted fret per string.
        std::vector<int> sig;
        for (const auto& ss : f.strings) sig.push_back(ss.fret);
        if (seen.count(sig)) continue;
        seen.insert(sig);
        unique.push_back(std::move(f));
        if (static_cast<int>(unique.size()) >= max_results) break;
    }

    return unique;
}

// ---------------------------------------------------------------------------
// Reverse: positions → theory
// ---------------------------------------------------------------------------

Chord Fretboard::identify(const std::vector<std::pair<int, int>>& string_frets) const {
    if (string_frets.empty())
        throw std::invalid_argument("Cannot identify chord from empty position list");

    std::vector<std::string> names;
    std::vector<int> seen_pcs;

    // Sort by pitch (lowest first) for root detection.
    std::vector<std::pair<int, int>> sorted = string_frets;
    std::sort(sorted.begin(), sorted.end(),
              [this](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                  return compute_midi(a.first - 1, a.second) <
                         compute_midi(b.first - 1, b.second);
              });

    for (const auto& sf : sorted) {
        int midi = compute_midi(sf.first - 1, sf.second);
        int pc = ((midi % 12) + 12) % 12;
        bool dup = false;
        for (int s : seen_pcs) {
            if (s == pc) { dup = true; break; }
        }
        if (!dup) {
            seen_pcs.push_back(pc);
            names.push_back(midi_to_name(midi));
        }
    }

    return Chord::identify(names);
}

// ---------------------------------------------------------------------------
// Capo
// ---------------------------------------------------------------------------

Fretboard Fretboard::capo(int fret) const {
    if (fret < 0 || fret >= tuning_.num_frets)
        throw std::out_of_range(
            "Capo fret " + std::to_string(fret) + " is out of range [0, " +
            std::to_string(tuning_.num_frets - 1) + "]");

    std::vector<int> new_open;
    new_open.reserve(tuning_.open_midi.size());
    for (int m : tuning_.open_midi) new_open.push_back(m + fret);

    return Fretboard(Tuning{
        tuning_.name + " (capo " + std::to_string(fret) + ")",
        new_open,
        tuning_.num_frets - fret
    });
}

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

std::string FretPosition::to_string() const {
    return "FretPosition(string=" + std::to_string(string) +
           ", fret=" + std::to_string(fret) +
           ", " + note + std::to_string(octave) +
           ", midi=" + std::to_string(midi) + ")";
}

std::string Fingering::to_string() const {
    std::ostringstream os;
    os << "Fingering(" << chord_name << ", [";
    for (std::size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) os << " ";
        if (strings[i].action == StringAction::Muted) os << "x";
        else os << strings[i].fret;
    }
    os << "]";
    if (barre > 0) os << ", barre=" << barre;
    os << ")";
    return os.str();
}

std::string Fretboard::to_string() const {
    std::ostringstream os;
    os << "Fretboard(\"" << name() << "\", "
       << num_strings() << " strings, " << num_frets() << " frets, [";
    for (std::size_t i = 0; i < tuning_.open_midi.size(); ++i) {
        if (i > 0) os << " ";
        int m = tuning_.open_midi[i];
        os << midi_to_name(m) << midi_to_octave(m);
    }
    os << "])";
    return os.str();
}

} // namespace gingo
