// Gingo — Music Theory Library
// MusicXML implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/musicxml.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace gingo {

// ---------------------------------------------------------------------------
// Pitch helpers
// ---------------------------------------------------------------------------

std::string MusicXML::pitch_step(const Note& n) {
    // Extract the base letter from the note's sound() (e.g. "C", "D", "B").
    return n.sound();
}

int MusicXML::pitch_alter(const Note& n) {
    const auto& name = n.name();
    if (name.size() < 2) return 0;

    // Count sharps and flats after the base letter.
    int alter = 0;
    for (std::size_t i = 1; i < name.size(); ++i) {
        if (name[i] == '#')      alter += 1;
        else if (name[i] == 'b') alter -= 1;
    }
    return alter;
}

// ---------------------------------------------------------------------------
// Duration mapping
// ---------------------------------------------------------------------------

// MusicXML divisions per quarter note.  We use 4 as the base so:
//   whole=16, half=8, quarter=4, eighth=2, sixteenth=1
static constexpr int DIVISIONS = 4;

int MusicXML::type_to_divisions(const std::string& type) {
    if (type == "whole")          return 16;
    if (type == "half")           return 8;
    if (type == "quarter")        return 4;
    if (type == "eighth")         return 2;
    if (type == "sixteenth")      return 1;
    if (type == "thirty_second" || type == "32nd") return 1;  // minimum
    return 4;  // default: quarter
}

std::string MusicXML::duration_to_type(const Duration& d) {
    const auto& name = d.name();
    // MusicXML uses hyphenated names for some.
    if (name == "thirty_second") return "32nd";
    if (name == "sixty_fourth")  return "64th";
    return name;
}

// ---------------------------------------------------------------------------
// XML building blocks
// ---------------------------------------------------------------------------

std::string MusicXML::header(const std::string& title) {
    std::ostringstream os;
    os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
       << "<!DOCTYPE score-partwise PUBLIC "
       << "\"-//Recordare//DTD MusicXML 4.0 Partwise//EN\" "
       << "\"http://www.musicxml.org/dtds/partwise.dtd\">\n"
       << "<score-partwise version=\"4.0\">\n"
       << "  <work>\n"
       << "    <work-title>" << title << "</work-title>\n"
       << "  </work>\n"
       << "  <identification>\n"
       << "    <encoding>\n"
       << "      <software>Gingo</software>\n"
       << "    </encoding>\n"
       << "  </identification>\n"
       << "  <part-list>\n"
       << "    <score-part id=\"P1\">\n"
       << "      <part-name>Piano</part-name>\n"
       << "    </score-part>\n"
       << "  </part-list>\n"
       << "  <part id=\"P1\">\n";
    return os.str();
}

std::string MusicXML::footer() {
    return "  </part>\n</score-partwise>\n";
}

std::string MusicXML::note_element(const Note& n, int octave,
                                   const std::string& type, int divisions,
                                   bool is_chord, bool is_rest) {
    std::ostringstream os;
    os << "      <note>\n";

    if (is_chord)
        os << "        <chord/>\n";

    if (is_rest) {
        os << "        <rest/>\n";
    } else {
        os << "        <pitch>\n"
           << "          <step>" << pitch_step(n) << "</step>\n";
        int alter = pitch_alter(n);
        if (alter != 0)
            os << "          <alter>" << alter << "</alter>\n";
        os << "          <octave>" << octave << "</octave>\n"
           << "        </pitch>\n";
    }

    os << "        <duration>" << divisions << "</duration>\n"
       << "        <type>" << type << "</type>\n"
       << "      </note>\n";
    return os.str();
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::string MusicXML::note(const Note& n, int octave,
                           const std::string& type) {
    int dur = type_to_divisions(type);
    std::ostringstream os;
    os << header(n.name() + std::to_string(octave));

    // One measure with attributes + note.
    os << "    <measure number=\"1\">\n"
       << "      <attributes>\n"
       << "        <divisions>" << DIVISIONS << "</divisions>\n"
       << "        <time>\n"
       << "          <beats>4</beats>\n"
       << "          <beat-type>4</beat-type>\n"
       << "        </time>\n"
       << "        <clef>\n"
       << "          <sign>G</sign>\n"
       << "          <line>2</line>\n"
       << "        </clef>\n"
       << "      </attributes>\n"
       << note_element(n, octave, type, dur)
       << "    </measure>\n";

    os << footer();
    return os.str();
}

std::string MusicXML::chord(const Chord& c, int octave,
                            const std::string& type) {
    int dur = type_to_divisions(type);
    auto notes = c.formal_notes();
    if (notes.empty()) notes = c.notes();

    int root_semi = c.root().semitone();

    std::ostringstream os;
    os << header(c.name());

    os << "    <measure number=\"1\">\n"
       << "      <attributes>\n"
       << "        <divisions>" << DIVISIONS << "</divisions>\n"
       << "        <time>\n"
       << "          <beats>4</beats>\n"
       << "          <beat-type>4</beat-type>\n"
       << "        </time>\n"
       << "        <clef>\n"
       << "          <sign>G</sign>\n"
       << "          <line>2</line>\n"
       << "        </clef>\n"
       << "      </attributes>\n";

    for (std::size_t i = 0; i < notes.size(); ++i) {
        int oct = octave;
        if (notes[i].semitone() < root_semi) oct += 1;
        os << note_element(notes[i], oct, type, dur, /*is_chord=*/i > 0);
    }

    os << "    </measure>\n"
       << footer();
    return os.str();
}

std::string MusicXML::scale(const Scale& s, int octave,
                            const std::string& type) {
    int dur = type_to_divisions(type);
    auto notes = s.formal_notes();
    if (notes.empty()) notes = s.notes();

    int tonic_semi = s.tonic().semitone();

    // Calculate notes per measure based on type.
    int notes_per_measure = 16 / dur;  // whole=1, half=2, quarter=4, eighth=8
    if (notes_per_measure < 1) notes_per_measure = 1;

    std::ostringstream os;
    os << header(s.tonic().name() + " " + s.mode_name());

    int measure_num = 1;
    int note_in_measure = 0;

    for (std::size_t i = 0; i < notes.size(); ++i) {
        if (note_in_measure == 0) {
            os << "    <measure number=\"" << measure_num << "\">\n";
            if (measure_num == 1) {
                os << "      <attributes>\n"
                   << "        <divisions>" << DIVISIONS << "</divisions>\n"
                   << "        <time>\n"
                   << "          <beats>4</beats>\n"
                   << "          <beat-type>4</beat-type>\n"
                   << "        </time>\n"
                   << "        <clef>\n"
                   << "          <sign>G</sign>\n"
                   << "          <line>2</line>\n"
                   << "        </clef>\n"
                   << "      </attributes>\n";
            }
        }

        int oct = octave;
        if (notes[i].semitone() < tonic_semi) oct += 1;
        os << note_element(notes[i], oct, type, dur);

        note_in_measure++;
        if (note_in_measure >= notes_per_measure || i == notes.size() - 1) {
            os << "    </measure>\n";
            measure_num++;
            note_in_measure = 0;
        }
    }

    os << footer();
    return os.str();
}

std::string MusicXML::field(const Field& f, int octave,
                            const std::string& type) {
    int dur = type_to_divisions(type);
    auto chords = f.chords();

    std::ostringstream os;
    os << header(f.tonic().name() + " " + "field");

    for (std::size_t m = 0; m < chords.size(); ++m) {
        auto notes = chords[m].formal_notes();
        if (notes.empty()) notes = chords[m].notes();
        int root_semi = chords[m].root().semitone();

        os << "    <measure number=\"" << (m + 1) << "\">\n";
        if (m == 0) {
            os << "      <attributes>\n"
               << "        <divisions>" << DIVISIONS << "</divisions>\n"
               << "        <time>\n"
               << "          <beats>4</beats>\n"
               << "          <beat-type>4</beat-type>\n"
               << "        </time>\n"
               << "        <clef>\n"
               << "          <sign>G</sign>\n"
               << "          <line>2</line>\n"
               << "        </clef>\n"
               << "      </attributes>\n";
        }

        for (std::size_t i = 0; i < notes.size(); ++i) {
            int oct = octave;
            if (notes[i].semitone() < root_semi) oct += 1;
            os << note_element(notes[i], oct, type, dur, /*is_chord=*/i > 0);
        }

        os << "    </measure>\n";
    }

    os << footer();
    return os.str();
}

std::string MusicXML::sequence(const Sequence& seq) {
    auto ts = seq.time_signature();
    int beats = ts.beats_per_bar();
    int beat_type = ts.beat_unit();

    std::ostringstream os;
    os << header("Sequence");

    // Track position in measure (in divisions).
    int capacity = DIVISIONS * beats * 4 / beat_type;  // total divisions per measure
    int pos_in_measure = 0;
    int measure_num = 1;
    bool measure_open = false;

    auto open_measure = [&]() {
        os << "    <measure number=\"" << measure_num << "\">\n";
        if (measure_num == 1) {
            os << "      <attributes>\n"
               << "        <divisions>" << DIVISIONS << "</divisions>\n"
               << "        <time>\n"
               << "          <beats>" << beats << "</beats>\n"
               << "          <beat-type>" << beat_type << "</beat-type>\n"
               << "        </time>\n"
               << "        <clef>\n"
               << "          <sign>G</sign>\n"
               << "          <line>2</line>\n"
               << "        </clef>\n"
               << "      </attributes>\n";

            // Tempo marking.
            double bpm = seq.tempo().bpm();
            os << "      <direction placement=\"above\">\n"
               << "        <direction-type>\n"
               << "          <metronome>\n"
               << "            <beat-unit>quarter</beat-unit>\n"
               << "            <per-minute>" << static_cast<int>(bpm) << "</per-minute>\n"
               << "          </metronome>\n"
               << "        </direction-type>\n"
               << "      </direction>\n";
        }
        measure_open = true;
    };

    auto close_measure = [&]() {
        os << "    </measure>\n";
        measure_open = false;
        measure_num++;
        pos_in_measure = 0;
    };

    for (std::size_t i = 0; i < seq.size(); ++i) {
        if (!measure_open)
            open_measure();

        const auto& event = seq.at(i);

        std::visit([&](auto&& ev) {
            using T = std::decay_t<decltype(ev)>;

            if constexpr (std::is_same_v<T, NoteEvent>) {
                auto type_name = duration_to_type(ev.duration());
                int dur = type_to_divisions(type_name);
                os << note_element(ev.note(), ev.octave(), type_name, dur);
                pos_in_measure += dur;
            }
            else if constexpr (std::is_same_v<T, ChordEvent>) {
                auto type_name = duration_to_type(ev.duration());
                int dur = type_to_divisions(type_name);
                auto notes = ev.chord().formal_notes();
                if (notes.empty()) notes = ev.chord().notes();
                int root_semi = ev.chord().root().semitone();

                for (std::size_t j = 0; j < notes.size(); ++j) {
                    int oct = ev.octave();
                    if (notes[j].semitone() < root_semi) oct += 1;
                    os << note_element(notes[j], oct, type_name, dur,
                                       /*is_chord=*/j > 0);
                }
                pos_in_measure += dur;
            }
            else if constexpr (std::is_same_v<T, Rest>) {
                auto type_name = duration_to_type(ev.duration());
                int dur = type_to_divisions(type_name);
                os << note_element(Note("C"), 4, type_name, dur,
                                   /*is_chord=*/false, /*is_rest=*/true);
                pos_in_measure += dur;
            }
        }, event);

        if (pos_in_measure >= capacity)
            close_measure();
    }

    if (measure_open)
        close_measure();

    os << footer();
    return os.str();
}

// ---------------------------------------------------------------------------
// File output
// ---------------------------------------------------------------------------

void MusicXML::write(const std::string& xml, const std::string& path) {
    std::ofstream out(path);
    if (!out)
        throw std::runtime_error("Cannot open file for writing: " + path);
    out << xml;
}

} // namespace gingo
