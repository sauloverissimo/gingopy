// Gingo — Music Theory Library
// pybind11 bindings exposing the C++ core to Python.
//
// SPDX-License-Identifier: MIT

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <gingo/gingo.hpp>

#include <algorithm>
#include <set>

namespace py = pybind11;

// Helper: slice a std::vector and return a py::list.
template<typename T>
static py::list slice_vector(const std::vector<T>& vec, py::slice sl) {
    py::ssize_t start, stop, step, slicelength;
    if (!sl.compute(static_cast<py::ssize_t>(vec.size()),
                    &start, &stop, &step, &slicelength))
        throw py::error_already_set();
    py::list result;
    for (py::ssize_t i = 0; i < slicelength; ++i) {
        result.append(py::cast(vec[static_cast<std::size_t>(start)]));
        start += step;
    }
    return result;
}
using namespace gingo;

// Helper: lowercase an ASCII string (for mode/scale type display).
static std::string to_lower(std::string s) {
    for (auto& c : s) if (c >= 'A' && c <= 'Z') c += 32;
    return s;
}

// Helper: parse a Roman numeral key into degree, accidental, and quality.
// Grammar: [accidental] numeral quality
//   "V7" → {5, 0, "7"},  "bVII" → {7, -1, ""},  "#IVm7" → {4, +1, "m7"}
struct RomanKey { int degree; int accidental; std::string quality; };

static RomanKey parse_roman_key(const std::string& s) {
    if (s.empty())
        throw py::key_error("empty Roman numeral");

    RomanKey r{0, 0, ""};
    std::size_t pos = 0;

    if (s[0] == 'b')      { r.accidental = -1; pos = 1; }
    else if (s[0] == '#') { r.accidental =  1; pos = 1; }

    auto rest = s.substr(pos);
    if (rest.size() >= 3 && rest.substr(0, 3) == "VII") { r.degree = 7; pos += 3; }
    else if (rest.size() >= 3 && rest.substr(0, 3) == "III") { r.degree = 3; pos += 3; }
    else if (rest.size() >= 2 && rest.substr(0, 2) == "VI")  { r.degree = 6; pos += 2; }
    else if (rest.size() >= 2 && rest.substr(0, 2) == "IV")  { r.degree = 4; pos += 2; }
    else if (rest.size() >= 2 && rest.substr(0, 2) == "II")  { r.degree = 2; pos += 2; }
    else if (!rest.empty() && rest[0] == 'V') { r.degree = 5; pos += 1; }
    else if (!rest.empty() && rest[0] == 'I') { r.degree = 1; pos += 1; }
    else throw py::key_error("invalid Roman numeral: \"" + s + "\"");

    r.quality = s.substr(pos);

    // Reject quality that looks like a continued Roman numeral (e.g. "VIII")
    if (!r.quality.empty() && (r.quality[0] == 'I' || r.quality[0] == 'V'))
        throw py::key_error("invalid Roman numeral: \"" + s + "\"");

    return r;
}

// Helper: resolve a note name to flat spelling when the accidental is flat.
// transpose() returns sharp-based names; "bVII" in C major should give "Bb" not "A#".
static std::string to_flat_name(const std::string& name) {
    static const std::pair<const char*, const char*> table[] = {
        {"C#", "Db"}, {"D#", "Eb"}, {"F#", "Gb"}, {"G#", "Ab"}, {"A#", "Bb"}
    };
    for (auto& [sharp, flat] : table)
        if (name == sharp) return flat;
    return name;
}

// Helper: identify a scale from a list of note names (pitch-class set matching).
// Shared by Scale.identify and Field.identify.
static Scale identify_scale(const std::vector<std::string>& note_names) {
    if (note_names.empty())
        throw py::value_error("identify() requires at least one note");

    Note tonic(note_names[0]);
    int tonic_pc = tonic.semitone();

    std::set<int> target;
    for (auto& n : note_names) target.insert(Note(n).semitone());

    static const char* chromatic[] = {
        "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
    };

    // Try diatonic, then pentatonic
    for (bool penta : {false, true}) {
        for (int st = 0; st <= 9; ++st) {
            for (int t = 0; t < 12; ++t) {
                try {
                    Scale parent(chromatic[t],
                                 static_cast<ScaleType>(st));
                    Scale base = penta
                        ? parent.pentatonic() : parent;
                    auto bn = base.notes();

                    std::set<int> base_pcs;
                    for (auto& n : bn)
                        base_pcs.insert(n.semitone());
                    if (base_pcs != target) continue;

                    // PC set matches — find the mode on our tonic
                    int num_modes = static_cast<int>(
                        parent.notes().size());
                    for (int m = 1; m <= num_modes; ++m) {
                        Scale mode_s = (m == 1)
                            ? parent : parent.mode(m);
                        if (penta) mode_s = mode_s.pentatonic();
                        if (mode_s.tonic().semitone() != tonic_pc)
                            continue;
                        auto mn = mode_s.notes();
                        std::set<int> mode_pcs;
                        for (auto& n : mn)
                            mode_pcs.insert(n.semitone());
                        if (mode_pcs == target)
                            return mode_s;
                    }
                } catch (...) { continue; }
            }
        }
    }
    throw py::value_error(
        "no matching scale found for the given notes");
}

// Helper: convert 1-indexed degree to Roman numeral.
static const char* degree_to_roman(int degree) {
    static const char* romans[] = {
        "I","II","III","IV","V","VI","VII"
    };
    if (degree >= 1 && degree <= 7) return romans[degree - 1];
    return "?";
}

// Binding-only struct: result item for Field.deduce().
struct FieldMatch {
    Field field;
    double score;
    int matched;
    int total;
    std::vector<std::string> roles;
};

// Binding-only struct: result item for Progression.identify() / deduce().
struct ProgressionMatch {
    std::string tradition;
    std::string schema;
    double score;
    int matched;
    int total;
    std::vector<std::string> branches;
};

// Binding-only struct: result item for Progression.predict().
struct ProgressionRoute {
    std::string next;
    std::string tradition;
    std::string schema;
    std::vector<std::string> path;
    double confidence;
};

// Helper: find the role of a chord in a field as Roman numeral.
// Returns "I".."VII" for triads, "I7".."VII7" for sevenths, "" if not found.
static std::string chord_role_in_field(const Field& f, const Chord& ic) {
    auto triads = f.chords();
    for (int d = 0; d < static_cast<int>(triads.size()); ++d) {
        if (triads[d].root().semitone() == ic.root().semitone()
            && triads[d].type() == ic.type())
            return degree_to_roman(d + 1);
    }
    auto sevs = f.sevenths();
    for (int d = 0; d < static_cast<int>(sevs.size()); ++d) {
        if (sevs[d].root().semitone() == ic.root().semitone()
            && sevs[d].type() == ic.type())
            return std::string(degree_to_roman(d + 1)) + "7";
    }
    return "";
}

// Helper: human-readable scale type string from mode_name().
static std::string scale_display_name(const Scale& s) {
    std::string t = s.mode_name();
    if (t == "Ionian" || t == "Major")         t = "major";
    else if (t == "Aeolian" || t == "NaturalMinor") t = "natural minor";
    else if (t == "HarmonicMinor")  t = "harmonic minor";
    else if (t == "MelodicMinor")   t = "melodic minor";
    else if (t == "HarmonicMajor")  t = "harmonic major";
    else if (t == "WholeTone")      t = "whole tone";
    else t = to_lower(t);
    if (s.is_pentatonic()) t += " pentatonic";
    return t;
}

// Helper: field display string for FieldMatch repr.
static std::string field_display(const Field& f) {
    return f.tonic().name() + " " + scale_display_name(f.scale());
}

// Helper: count valid consecutive transitions in a branch sequence.
static int count_valid_transitions(const Tree& tree,
                                    const std::vector<std::string>& branches) {
    int count = 0;
    for (std::size_t i = 0; i + 1 < branches.size(); ++i)
        if (tree.is_valid({branches[i], branches[i + 1]}))
            ++count;
    return count;
}

// Helper: check if needle appears as a contiguous sub-sequence of haystack.
static bool is_contiguous_subseq(const std::vector<std::string>& needle,
                                  const std::vector<std::string>& haystack) {
    if (needle.size() > haystack.size()) return false;
    for (std::size_t i = 0; i + needle.size() <= haystack.size(); ++i) {
        bool match = true;
        for (std::size_t j = 0; j < needle.size(); ++j) {
            if (needle[j] != haystack[i + j]) { match = false; break; }
        }
        if (match) return true;
    }
    return false;
}

// Helper: check if prefix is a prefix of seq.
static bool is_prefix_of(const std::vector<std::string>& prefix,
                           const std::vector<std::string>& seq) {
    if (prefix.size() > seq.size()) return false;
    for (std::size_t i = 0; i < prefix.size(); ++i)
        if (prefix[i] != seq[i]) return false;
    return true;
}

PYBIND11_MODULE(_gingo, m) {
    m.doc() = "🪇 Gingo — Music theory library for Python";

    // ---- ScaleType enum ----------------------------------------------------
    py::enum_<ScaleType>(m, "ScaleType")
        .value("Major",         ScaleType::Major)
        .value("NaturalMinor",  ScaleType::NaturalMinor)
        .value("HarmonicMinor", ScaleType::HarmonicMinor)
        .value("MelodicMinor",  ScaleType::MelodicMinor)
        .value("Diminished",    ScaleType::Diminished)
        .value("HarmonicMajor", ScaleType::HarmonicMajor)
        .value("WholeTone",     ScaleType::WholeTone)
        .value("Augmented",     ScaleType::Augmented)
        .value("Blues",         ScaleType::Blues)
        .value("Chromatic",     ScaleType::Chromatic);

    // ---- Modality enum -----------------------------------------------------
    py::enum_<Modality>(m, "Modality")
        .value("Diatonic",   Modality::Diatonic)
        .value("Pentatonic", Modality::Pentatonic);

    // ---- Note --------------------------------------------------------------
    py::class_<Note>(m, "Note")
        .def(py::init<const std::string&>(), py::arg("name"))
        .def("name",          &Note::name)
        .def("natural",       &Note::natural)
        .def("sound",         &Note::sound)
        .def("semitone",      &Note::semitone)
        .def("frequency",     &Note::frequency, py::arg("octave") = 4, py::arg("tuning") = 440.0)
        .def("is_enharmonic", &Note::is_enharmonic, py::arg("other"))
        .def("transpose",     &Note::transpose, py::arg("semitones"))
        .def("distance",      &Note::distance, py::arg("other"))
        .def_static("fifths", &Note::fifths)
        .def("__eq__",        &Note::operator==)
        .def("__ne__",        &Note::operator!=)
        .def("__repr__",      &Note::to_string)
        .def("__str__",       &Note::name)
        .def("__hash__",      [](const Note& n) { return n.semitone(); })
        .def("__lt__",        [](const Note& a, const Note& b) { return a.semitone() < b.semitone(); })
        .def("__le__",        [](const Note& a, const Note& b) { return a.semitone() <= b.semitone(); })
        .def("__gt__",        [](const Note& a, const Note& b) { return a.semitone() > b.semitone(); })
        .def("__ge__",        [](const Note& a, const Note& b) { return a.semitone() >= b.semitone(); })
        .def("__int__",       [](const Note& n) { return n.semitone(); })
        .def("__add__",       [](const Note& n, py::object other) {
            if (py::isinstance<Interval>(other))
                return n.transpose(other.cast<Interval>().semitones());
            if (py::isinstance<py::int_>(other))
                return n.transpose(other.cast<int>());
            throw py::type_error("unsupported operand type for +");
        })
        .def("__radd__",      [](const Note& n, int st) { return n.transpose(st); })
        .def("__float__",     [](const Note& n) { return n.frequency(4); })
        .def("__sub__",       [](const Note& n, py::object other) -> py::object {
            if (py::isinstance<Interval>(other))
                return py::cast(n.transpose(-other.cast<Interval>().semitones()));
            if (py::isinstance<py::int_>(other))
                return py::cast(n.transpose(-other.cast<int>()));
            if (py::isinstance<Note>(other))
                return py::cast(Interval(other.cast<Note>(), n));
            throw py::type_error("unsupported operand type for -");
        });

    // ---- Interval ----------------------------------------------------------
    py::class_<Interval>(m, "Interval")
        .def(py::init<const std::string&>(), py::arg("label"))
        .def(py::init<int>(), py::arg("semitones"))
        .def(py::init<const Note&, const Note&>(), py::arg("from_note"), py::arg("to_note"))
        .def("label",        &Interval::label)
        .def("anglo_saxon",  &Interval::anglo_saxon)
        .def("semitones",    &Interval::semitones)
        .def("degree",       &Interval::degree)
        .def("octave",       &Interval::octave)
        .def("simple",       &Interval::simple)
        .def("is_compound",  &Interval::is_compound)
        .def("invert",       &Interval::invert)
        .def("consonance",   &Interval::consonance, py::arg("include_fourth") = false)
        .def("is_consonant", &Interval::is_consonant, py::arg("include_fourth") = false)
        .def("full_name",    &Interval::full_name)
        .def("full_name_pt", &Interval::full_name_pt)
        .def("__eq__",       [](const Interval& iv, py::object other) {
            if (py::isinstance<Interval>(other))
                return iv.semitones() == other.cast<Interval>().semitones();
            if (py::isinstance<py::int_>(other))
                return iv.semitones() == other.cast<int>();
            return false;
        })
        .def("__ne__",       [](const Interval& iv, py::object other) {
            if (py::isinstance<Interval>(other))
                return iv.semitones() != other.cast<Interval>().semitones();
            if (py::isinstance<py::int_>(other))
                return iv.semitones() != other.cast<int>();
            return true;
        })
        .def("__repr__",    &Interval::to_string)
        .def("__str__",     &Interval::label)
        .def("__hash__",    [](const Interval& iv) { return iv.semitones(); })
        .def("__lt__",      [](const Interval& a, const Interval& b) { return a.semitones() < b.semitones(); })
        .def("__le__",      [](const Interval& a, const Interval& b) { return a.semitones() <= b.semitones(); })
        .def("__gt__",      [](const Interval& a, const Interval& b) { return a.semitones() > b.semitones(); })
        .def("__ge__",      [](const Interval& a, const Interval& b) { return a.semitones() >= b.semitones(); })
        .def("__int__",     [](const Interval& iv) { return iv.semitones(); })
        .def("__add__",     &Interval::operator+)
        .def("__sub__",     &Interval::operator-);

    // ---- Chord -------------------------------------------------------------
    py::class_<Chord>(m, "Chord")
        .def(py::init([](py::object arg) -> Chord {
            if (py::isinstance<py::str>(arg))
                return Chord(arg.cast<std::string>());
            if (py::isinstance<py::list>(arg) || py::isinstance<py::tuple>(arg)) {
                std::vector<std::string> names;
                for (auto item : arg)
                    names.push_back(py::isinstance<Note>(item)
                        ? item.cast<Note>().name()
                        : item.cast<std::string>());
                return Chord::identify(names);
            }
            throw py::type_error(
                "Chord() requires a name string or list of notes");
        }), py::arg("name"))
        .def("name",            &Chord::name)
        .def("root",            &Chord::root)
        .def("type",            &Chord::type)
        .def("notes",           &Chord::notes)
        .def("formal_notes",    &Chord::formal_notes)
        .def("intervals",       &Chord::intervals)
        .def("interval_labels", &Chord::interval_labels)
        .def("size",            &Chord::size)
        .def("contains",       &Chord::contains, py::arg("note"))
        .def("compare",        &Chord::compare, py::arg("other"))
        .def("transpose",     &Chord::transpose, py::arg("semitones"))
        .def_static("identify",
            py::overload_cast<const std::vector<std::string>&>(&Chord::identify),
            py::arg("note_names"))
        .def("__eq__",   &Chord::operator==)
        .def("__ne__",   &Chord::operator!=)
        .def("__repr__", &Chord::to_string)
        .def("__str__",  &Chord::name)
        .def("__add__",  [](const Chord& c, int semitones) {
            return c.transpose(semitones);
        })
        .def("__sub__",  [](const Chord& c, int semitones) {
            return c.transpose(-semitones);
        })
        .def("__hash__", [](const Chord& c) {
            // Consistent with operator== which compares name_
            std::size_t h = 0;
            for (char ch : c.name()) h = h * 131 + static_cast<unsigned char>(ch);
            return h;
        })
        .def("__len__",      [](const Chord& c) { return c.size(); })
        .def("__contains__", [](const Chord& c, py::object arg) {
            Note n = py::isinstance<Note>(arg)
                ? arg.cast<Note>()
                : Note(arg.cast<std::string>());
            return c.contains(n);
        })
        .def("__iter__",     [](const Chord& c) {
            py::list l = py::cast(c.notes());
            return l.attr("__iter__")();
        })
        .def("__reversed__", [](const Chord& c) {
            auto notes = c.notes();
            std::reverse(notes.begin(), notes.end());
            py::list l = py::cast(notes);
            return l.attr("__iter__")();
        })
        .def("__getitem__",  [](const Chord& c, py::object key) -> py::object {
            auto notes = c.notes();
            int sz = static_cast<int>(notes.size());
            if (py::isinstance<py::slice>(key))
                return slice_vector(notes, key.cast<py::slice>());
            int i = key.cast<int>();
            if (i < 0) i += sz;
            if (i < 0 || i >= sz)
                throw py::index_error("chord index out of range");
            return py::cast(notes[static_cast<std::size_t>(i)]);
        });

    // ---- ChordComparison (struct) ------------------------------------------
    py::class_<ChordComparison>(m, "ChordComparison")
        .def_readonly("common_notes",      &ChordComparison::common_notes)
        .def_readonly("exclusive_a",        &ChordComparison::exclusive_a)
        .def_readonly("exclusive_b",        &ChordComparison::exclusive_b)
        .def_readonly("root_distance",      &ChordComparison::root_distance)
        .def_readonly("root_direction",     &ChordComparison::root_direction)
        .def_readonly("same_quality",       &ChordComparison::same_quality)
        .def_readonly("same_size",          &ChordComparison::same_size)
        .def_readonly("common_intervals",   &ChordComparison::common_intervals)
        .def_readonly("enharmonic",         &ChordComparison::enharmonic)
        .def_readonly("subset",             &ChordComparison::subset)
        .def_readonly("voice_leading",      &ChordComparison::voice_leading)
        .def_readonly("transformation",     &ChordComparison::transformation)
        .def_readonly("inversion",              &ChordComparison::inversion)
        .def_readonly("interval_vector_a",     &ChordComparison::interval_vector_a)
        .def_readonly("interval_vector_b",     &ChordComparison::interval_vector_b)
        .def_readonly("same_interval_vector",  &ChordComparison::same_interval_vector)
        .def_readonly("transposition",         &ChordComparison::transposition)
        .def_readonly("dissonance_a",          &ChordComparison::dissonance_a)
        .def_readonly("dissonance_b",          &ChordComparison::dissonance_b)
        .def("to_dict", [](const ChordComparison& c) {
            auto note_names = [](const std::vector<Note>& notes) {
                py::list out;
                for (const auto& n : notes) out.append(n.name());
                return out;
            };
            py::dict d;
            d["common_notes"]        = note_names(c.common_notes);
            d["exclusive_a"]         = note_names(c.exclusive_a);
            d["exclusive_b"]         = note_names(c.exclusive_b);
            d["root_distance"]       = c.root_distance;
            d["root_direction"]      = c.root_direction;
            d["same_quality"]        = c.same_quality;
            d["same_size"]           = c.same_size;
            d["common_intervals"]    = c.common_intervals;
            d["enharmonic"]          = c.enharmonic;
            d["subset"]              = c.subset;
            d["voice_leading"]       = c.voice_leading;
            d["transformation"]      = c.transformation;
            d["inversion"]           = c.inversion;
            d["interval_vector_a"]   = c.interval_vector_a;
            d["interval_vector_b"]   = c.interval_vector_b;
            d["same_interval_vector"]= c.same_interval_vector;
            d["transposition"]       = c.transposition;
            d["dissonance_a"]        = c.dissonance_a;
            d["dissonance_b"]        = c.dissonance_b;
            return d;
        })
        .def("__repr__", [](const ChordComparison& c) {
            return "ChordComparison(root_dist=" + std::to_string(c.root_distance)
                 + ", common=" + std::to_string(c.common_notes.size())
                 + ", vl=" + std::to_string(c.voice_leading) + ")";
        });

    // ---- Scale -------------------------------------------------------------
    py::class_<Scale>(m, "Scale")
        .def(py::init<const std::string&, ScaleType, Modality>(),
             py::arg("tonic"),
             py::arg("type"),
             py::arg("modality") = Modality::Diatonic)
        .def(py::init<const std::string&, const std::string&, const std::string&>(),
             py::arg("tonic"),
             py::arg("type"),
             py::arg("modality") = "diatonic")
        .def("tonic",        &Scale::tonic)
        .def("type",         &Scale::type)
        .def("modality",     &Scale::modality)
        .def("parent",       &Scale::parent)
        .def("mode_number",  &Scale::mode_number)
        .def("is_pentatonic",&Scale::is_pentatonic)
        .def("mode_name",    &Scale::mode_name)
        .def("quality",      &Scale::quality)
        .def("brightness",   &Scale::brightness)
        .def("colors", [](const Scale& self, py::object ref) -> std::vector<Note> {
            if (py::isinstance<Scale>(ref))
                return self.colors(ref.cast<Scale>());
            if (py::isinstance<py::str>(ref))
                return self.colors(ref.cast<std::string>());
            throw py::type_error("colors() requires a Scale or str argument");
        }, py::arg("reference"))
        .def("notes",        &Scale::notes)
        .def("formal_notes", &Scale::formal_notes)
        .def("degree", [](const Scale& self, py::args args) {
            if (args.empty())
                throw py::value_error("degree() requires at least one argument");
            if (args.size() == 1)
                return self.degree(args[0].cast<int>());
            std::vector<int> degrees;
            degrees.reserve(args.size());
            for (auto& a : args) degrees.push_back(a.cast<int>());
            return self.degree(degrees);
        })
        .def("walk", [](const Scale& self, py::args args) {
            if (args.size() < 2)
                throw py::value_error(
                    "walk() requires start + at least one step");
            int start = args[0].cast<int>();
            std::vector<int> steps;
            steps.reserve(args.size() - 1);
            for (std::size_t i = 1; i < args.size(); ++i)
                steps.push_back(args[i].cast<int>());
            return self.walk(start, steps);
        })
        .def("size",         &Scale::size)
        .def("contains",     &Scale::contains, py::arg("note"))
        .def("degree_of", [](const Scale& self, const Note& note) -> py::object {
            auto result = self.degree_of(note);
            return result ? py::cast(*result) : py::none();
        }, py::arg("note"))
        .def("mode", [](const Scale& self, py::object arg) -> Scale {
            if (py::isinstance<py::int_>(arg))
                return self.mode(arg.cast<int>());
            if (py::isinstance<py::str>(arg))
                return self.mode(arg.cast<std::string>());
            throw py::type_error("mode() requires int or str");
        }, py::arg("degree_or_name"))
        .def("pentatonic",   &Scale::pentatonic)
        .def("signature",    &Scale::signature)
        .def("relative",     &Scale::relative)
        .def("parallel",     &Scale::parallel)
        .def("neighbors",    &Scale::neighbors)
        .def("mask",         &Scale::mask)
        .def_static("identify", [](py::object notes_arg) -> Scale {
            std::vector<std::string> note_names;
            for (auto item : notes_arg)
                note_names.push_back(py::isinstance<Note>(item)
                    ? item.cast<Note>().name()
                    : item.cast<std::string>());
            return identify_scale(note_names);
        }, py::arg("notes"))
        .def_static("parse_type",     &Scale::parse_type, py::arg("name"))
        .def_static("parse_modality", &Scale::parse_modality, py::arg("name"))
        .def("__repr__",     &Scale::to_string)
        .def("__len__",      &Scale::size)
        .def("__eq__",       [](const Scale& a, const Scale& b) {
            return a.tonic().semitone() == b.tonic().semitone()
                && a.parent() == b.parent()
                && a.mode_number() == b.mode_number()
                && a.is_pentatonic() == b.is_pentatonic();
        })
        .def("__ne__",       [](const Scale& a, const Scale& b) {
            return !(a.tonic().semitone() == b.tonic().semitone()
                  && a.parent() == b.parent()
                  && a.mode_number() == b.mode_number()
                  && a.is_pentatonic() == b.is_pentatonic());
        })
        .def("__hash__",     [](const Scale& s) {
            auto h = static_cast<std::size_t>(s.tonic().semitone());
            h = h * 131 + static_cast<std::size_t>(s.parent());
            h = h * 131 + static_cast<std::size_t>(s.mode_number());
            h = h * 131 + static_cast<std::size_t>(s.is_pentatonic());
            return h;
        })
        .def("__str__",      [](const Scale& s) {
            return s.tonic().name() + " " + scale_display_name(s);
        })
        .def("__contains__", [](const Scale& s, py::object arg) {
            Note n = py::isinstance<Note>(arg)
                ? arg.cast<Note>()
                : Note(arg.cast<std::string>());
            return s.contains(n);
        })
        .def("__iter__",     [](const Scale& s) {
            py::list l = py::cast(s.notes());
            return l.attr("__iter__")();
        })
        .def("__getitem__",  [](const Scale& s, py::object key) -> py::object {
            auto notes = s.notes();
            int sz = static_cast<int>(notes.size());
            if (py::isinstance<py::slice>(key))
                return slice_vector(notes, key.cast<py::slice>());
            int i = key.cast<int>();
            if (i < 0) i += sz;
            if (i < 0 || i >= sz)
                throw py::index_error("scale index out of range");
            return py::cast(notes[static_cast<std::size_t>(i)]);
        })
        .def("__reversed__", [](const Scale& s) {
            auto notes = s.notes();
            std::reverse(notes.begin(), notes.end());
            py::list l = py::cast(notes);
            return l.attr("__iter__")();
        });

    // ---- HarmonicFunction enum ---------------------------------------------
    py::enum_<HarmonicFunction>(m, "HarmonicFunction")
        .value("Tonic",       HarmonicFunction::Tonic)
        .value("Subdominant", HarmonicFunction::Subdominant)
        .value("Dominant",    HarmonicFunction::Dominant)
        .def_property_readonly("short", [](HarmonicFunction f) {
            return harmonic_function_short(f);
        });

    // ---- Field -------------------------------------------------------------
    py::class_<Field>(m, "Field")
        .def(py::init<const std::string&, ScaleType>(),
             py::arg("tonic"), py::arg("type"))
        .def(py::init<const std::string&, const std::string&>(),
             py::arg("tonic"), py::arg("type"))
        .def("tonic",              &Field::tonic)
        .def("scale",              &Field::scale)
        .def("chords",             &Field::chords)
        .def("sevenths",           &Field::sevenths)
        .def("chord",              &Field::chord, py::arg("degree"))
        .def("seventh",            &Field::seventh, py::arg("degree"))
        .def("applied", [](const Field& self, py::object func, py::object target) -> Chord {
            if (py::isinstance<py::int_>(func) && py::isinstance<py::int_>(target))
                return self.applied(func.cast<int>(), target.cast<int>());
            if (py::isinstance<py::str>(func) && py::isinstance<py::int_>(target))
                return self.applied(func.cast<std::string>(), target.cast<int>());
            if (py::isinstance<py::str>(func) && py::isinstance<py::str>(target))
                return self.applied(func.cast<std::string>(), target.cast<std::string>());
            throw py::type_error(
                "applied() requires (str, int), (str, str), or (int, int)");
        }, py::arg("function"), py::arg("target"))
        .def("function", [](const Field& self, py::object arg) -> py::object {
            if (py::isinstance<py::int_>(arg))
                return py::cast(self.function(arg.cast<int>()));
            if (py::isinstance<py::str>(arg)) {
                auto r = self.function(arg.cast<std::string>());
                return r ? py::cast(*r) : py::none();
            }
            if (py::isinstance<Chord>(arg)) {
                auto r = self.function(arg.cast<Chord>());
                return r ? py::cast(*r) : py::none();
            }
            throw py::type_error(
                "function() requires int, str, or Chord");
        }, py::arg("degree_or_chord"))
        .def("role", [](const Field& self, py::object arg) -> py::object {
            if (py::isinstance<py::int_>(arg))
                return py::cast(self.role(arg.cast<int>()));
            if (py::isinstance<py::str>(arg)) {
                auto r = self.role(arg.cast<std::string>());
                return r ? py::cast(*r) : py::none();
            }
            if (py::isinstance<Chord>(arg)) {
                auto r = self.role(arg.cast<Chord>());
                return r ? py::cast(*r) : py::none();
            }
            throw py::type_error(
                "role() requires int, str, or Chord");
        }, py::arg("degree_or_chord"))
        .def("compare",            &Field::compare, py::arg("a"), py::arg("b"))
        .def("signature",          &Field::signature)
        .def("relative",           &Field::relative)
        .def("parallel",           &Field::parallel)
        .def("neighbors",          &Field::neighbors)
        .def("size",               &Field::size)
        .def_static("identify", [](py::object items_arg) -> Field {
            py::list items = items_arg.cast<py::list>();
            if (items.empty())
                throw py::value_error("identify() requires at least one item");

            auto first = items[0];
            bool use_notes = false;

            // Dispatch: Note objects → note-based
            if (py::isinstance<Note>(first)) {
                use_notes = true;
            }
            // Chord objects → chord-based
            else if (py::isinstance<Chord>(first)) {
                use_notes = false;
            }
            // Strings: strict note pattern [A-G][#b]? → notes, else → chords
            else if (py::isinstance<py::str>(first)) {
                use_notes = true;
                for (auto item : items) {
                    auto s = item.cast<std::string>();
                    if (s.empty() || s[0] < 'A' || s[0] > 'G') {
                        use_notes = false; break;
                    }
                    if (s.size() == 1) continue;  // "C", "D", etc.
                    if (s.size() == 2 && (s[1] == '#' || s[1] == 'b'))
                        continue;  // "C#", "Db", etc.
                    use_notes = false; break;
                }
            }

            if (use_notes) {
                // Note-based: Scale.identify → Field
                std::vector<std::string> note_names;
                for (auto item : items)
                    note_names.push_back(py::isinstance<Note>(item)
                        ? item.cast<Note>().name()
                        : item.cast<std::string>());
                Scale s = identify_scale(note_names);
                int mn = s.mode_number();
                // Mode 1: tonic + parent type directly
                if (mn == 1)
                    return Field(s.tonic().name(), s.parent());
                // Aeolian (mode 6 of Major) → NaturalMinor field
                if (s.parent() == ScaleType::Major && mn == 6)
                    return Field(s.tonic().name(),
                                 ScaleType::NaturalMinor);
                // Other modes: use parent tonic (mode-1 root)
                auto notes = s.notes();
                int sz = static_cast<int>(notes.size());
                int pi = (sz - mn + 1) % sz;
                return Field(notes[pi].name(), s.parent());
            }

            // Chord-based: brute-force match against candidate fields
            std::vector<Chord> input_chords;
            for (auto item : items)
                input_chords.push_back(py::isinstance<Chord>(item)
                    ? item.cast<Chord>()
                    : Chord(item.cast<std::string>()));

            static const char* chromatic[] = {
                "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
            };
            static const ScaleType field_types[] = {
                ScaleType::Major, ScaleType::NaturalMinor,
                ScaleType::HarmonicMinor, ScaleType::MelodicMinor,
                ScaleType::HarmonicMajor
            };

            int best_score = 0;
            Field best("C", ScaleType::Major);  // placeholder

            for (auto st : field_types) {
                for (int t = 0; t < 12; ++t) {
                    try {
                        Field candidate(chromatic[t], st);
                        auto triads = candidate.chords();
                        auto sevs   = candidate.sevenths();

                        int score = 0;
                        for (auto& ic : input_chords) {
                            int ic_root = ic.root().semitone();
                            auto ic_type = ic.type();
                            bool found = false;
                            for (auto& d : triads) {
                                if (d.root().semitone() == ic_root
                                    && d.type() == ic_type)
                                { found = true; break; }
                            }
                            if (!found) {
                                for (auto& d : sevs) {
                                    if (d.root().semitone() == ic_root
                                        && d.type() == ic_type)
                                    { found = true; break; }
                                }
                            }
                            if (found) ++score;
                        }
                        if (score > best_score) {
                            best_score = score;
                            best = candidate;
                            if (score == static_cast<int>(
                                    input_chords.size()))
                                return best;  // perfect match
                        }
                    } catch (...) { continue; }
                }
            }

            if (best_score == 0)
                throw py::value_error(
                    "no matching field found for the given chords");
            return best;
        }, py::arg("items"))
        .def_static("deduce", [](py::object items_arg, int limit) -> py::list {
            py::list items = items_arg.cast<py::list>();
            if (items.empty())
                throw py::value_error("deduce() requires at least one item");

            auto first = items[0];
            bool use_notes = false;

            if (py::isinstance<Note>(first)) {
                use_notes = true;
            } else if (py::isinstance<Chord>(first)) {
                use_notes = false;
            } else if (py::isinstance<py::str>(first)) {
                use_notes = true;
                for (auto item : items) {
                    auto s = item.cast<std::string>();
                    if (s.empty() || s[0] < 'A' || s[0] > 'G') {
                        use_notes = false; break;
                    }
                    if (s.size() == 1) continue;
                    if (s.size() == 2 && (s[1] == '#' || s[1] == 'b'))
                        continue;
                    use_notes = false; break;
                }
            }

            static const char* chromatic[] = {
                "C","C#","D","D#","E","F","F#","G","G#","A","A#","B"
            };
            static const ScaleType field_types[] = {
                ScaleType::Major, ScaleType::NaturalMinor,
                ScaleType::HarmonicMinor, ScaleType::MelodicMinor,
                ScaleType::HarmonicMajor
            };

            std::vector<FieldMatch> results;

            if (use_notes) {
                // Note-based: check membership in each candidate field's scale
                std::vector<int> input_pcs;
                for (auto item : items) {
                    std::string n = py::isinstance<Note>(item)
                        ? item.cast<Note>().name()
                        : item.cast<std::string>();
                    input_pcs.push_back(Note(n).semitone());
                }
                int total = static_cast<int>(input_pcs.size());

                for (auto st : field_types) {
                    for (int t = 0; t < 12; ++t) {
                        try {
                            Field candidate(chromatic[t], st);
                            auto sc = candidate.scale();
                            auto sc_notes = sc.notes();

                            int matched = 0;
                            std::vector<std::string> roles;
                            roles.reserve(total);

                            for (int pc : input_pcs) {
                                bool found = false;
                                for (int d = 0; d < static_cast<int>(sc_notes.size()); ++d) {
                                    if (sc_notes[d].semitone() == pc) {
                                        roles.push_back(degree_to_roman(d + 1));
                                        ++matched;
                                        found = true;
                                        break;
                                    }
                                }
                                if (!found) roles.push_back("");
                            }

                            if (matched > 0) {
                                double score = static_cast<double>(matched) / total;
                                results.push_back(FieldMatch{
                                    candidate, score, matched, total,
                                    std::move(roles)});
                            }
                        } catch (...) { continue; }
                    }
                }
            } else {
                // Chord-based: match against candidate fields
                std::vector<Chord> input_chords;
                for (auto item : items)
                    input_chords.push_back(py::isinstance<Chord>(item)
                        ? item.cast<Chord>()
                        : Chord(item.cast<std::string>()));
                int total = static_cast<int>(input_chords.size());

                for (auto st : field_types) {
                    for (int t = 0; t < 12; ++t) {
                        try {
                            Field candidate(chromatic[t], st);
                            int matched = 0;
                            std::vector<std::string> roles;
                            roles.reserve(total);

                            for (auto& ic : input_chords) {
                                auto role = chord_role_in_field(candidate, ic);
                                if (!role.empty()) ++matched;
                                roles.push_back(std::move(role));
                            }

                            if (matched > 0) {
                                double score = static_cast<double>(matched) / total;
                                results.push_back(FieldMatch{
                                    candidate, score, matched, total,
                                    std::move(roles)});
                            }
                        } catch (...) { continue; }
                    }
                }
            }

            // Sort: score desc, then field_type priority (Major first)
            std::stable_sort(results.begin(), results.end(),
                [](const FieldMatch& a, const FieldMatch& b) {
                    if (a.score != b.score) return a.score > b.score;
                    return static_cast<int>(a.field.scale().type())
                         < static_cast<int>(b.field.scale().type());
                });

            // Apply limit
            if (limit > 0 && static_cast<int>(results.size()) > limit)
                results.erase(results.begin() + limit, results.end());

            py::list out;
            for (auto& r : results)
                out.append(py::cast(std::move(r)));
            return out;
        }, py::arg("items"), py::arg("limit") = 10)
        .def("__repr__",           &Field::to_string)
        .def("__len__",            &Field::size)
        .def("__eq__",             [](const Field& a, const Field& b) {
            return a.tonic().semitone() == b.tonic().semitone()
                && a.scale().type() == b.scale().type();
        })
        .def("__ne__",             [](const Field& a, const Field& b) {
            return !(a.tonic().semitone() == b.tonic().semitone()
                  && a.scale().type() == b.scale().type());
        })
        .def("__hash__",           [](const Field& f) {
            auto h = static_cast<std::size_t>(f.tonic().semitone());
            h = h * 131 + static_cast<std::size_t>(f.scale().type());
            return h;
        })
        .def("__str__",            [](const Field& f) {
            return f.tonic().name() + " " + scale_display_name(f.scale());
        })
        .def("__contains__",       [](const Field& f, py::object arg) {
            Chord c = py::isinstance<Chord>(arg)
                ? arg.cast<Chord>()
                : Chord(arg.cast<std::string>());
            return f.function(c).has_value();
        })
        .def("__iter__",           [](const Field& f) {
            py::list l = py::cast(f.chords());
            return l.attr("__iter__")();
        })
        .def("__reversed__",      [](const Field& f) {
            auto chords = f.chords();
            std::reverse(chords.begin(), chords.end());
            py::list l = py::cast(chords);
            return l.attr("__iter__")();
        })
        .def("__getitem__",        [](const Field& f, py::object key) -> py::object {
            int sz = static_cast<int>(f.size());
            if (py::isinstance<py::slice>(key)) {
                auto chords = f.chords();
                return slice_vector(chords, key.cast<py::slice>());
            }
            if (py::isinstance<py::tuple>(key)) {
                auto tup = key.cast<py::tuple>();
                if (tup.size() != 2)
                    throw py::key_error(
                        "applied chord tuple requires exactly 2 elements: "
                        "f[\"V7\", \"IV\"]");
                auto a = tup[0], b = tup[1];
                if (py::isinstance<py::str>(a) && py::isinstance<py::str>(b))
                    return py::cast(f.applied(
                        a.cast<std::string>(), b.cast<std::string>()));
                if (py::isinstance<py::str>(a) && py::isinstance<py::int_>(b))
                    return py::cast(f.applied(
                        a.cast<std::string>(), b.cast<int>()));
                if (py::isinstance<py::int_>(a) && py::isinstance<py::int_>(b))
                    return py::cast(f.applied(
                        a.cast<int>(), b.cast<int>()));
                throw py::type_error(
                    "applied chord tuple requires (str, str), (str, int), "
                    "or (int, int)");
            }
            if (py::isinstance<py::str>(key)) {
                auto rk = parse_roman_key(key.cast<std::string>());
                if (rk.accidental == 0 && rk.quality.empty())
                    return py::cast(f.chord(rk.degree));
                if (rk.accidental == 0 && rk.quality == "7")
                    return py::cast(f.seventh(rk.degree));
                // Explicit quality or accidental: build chord from root
                Note root = f.scale().degree(rk.degree);
                if (rk.accidental != 0)
                    root = root.transpose(rk.accidental);
                std::string rn = (rk.accidental == -1)
                    ? to_flat_name(root.natural())
                    : root.natural();
                std::string q = rk.quality.empty() ? "M" : rk.quality;
                return py::cast(Chord(rn + q));
            }
            int i = key.cast<int>();
            if (i < 0) i += sz;
            if (i < 0 || i >= sz)
                throw py::index_error("field index out of range");
            return py::cast(f.chord(i + 1));
        });

    // ---- FieldMatch (struct, binding-only) ---------------------------------
    py::class_<FieldMatch>(m, "FieldMatch")
        .def_readonly("field",   &FieldMatch::field)
        .def_readonly("score",   &FieldMatch::score)
        .def_readonly("matched", &FieldMatch::matched)
        .def_readonly("total",   &FieldMatch::total)
        .def_readonly("roles",   &FieldMatch::roles)
        .def("to_dict", [](const FieldMatch& fm) {
            py::dict d;
            d["field"]   = field_display(fm.field);
            d["score"]   = fm.score;
            d["matched"] = fm.matched;
            d["total"]   = fm.total;
            d["roles"]   = fm.roles;
            return d;
        })
        .def("__repr__", [](const FieldMatch& fm) {
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "FieldMatch(%s, score=%.2f, %d/%d)",
                field_display(fm.field).c_str(),
                fm.score, fm.matched, fm.total);
            return std::string(buf);
        });

    // ---- BorrowedInfo (struct) ---------------------------------------------
    py::class_<BorrowedInfo>(m, "BorrowedInfo")
        .def_readonly("scale_type",  &BorrowedInfo::scale_type)
        .def_readonly("degree",      &BorrowedInfo::degree)
        .def_readonly("function",    &BorrowedInfo::function)
        .def_readonly("role",        &BorrowedInfo::role)
        .def("to_dict", [](const BorrowedInfo& b) {
            py::dict d;
            d["scale_type"] = b.scale_type;
            d["degree"]     = b.degree;
            d["function"]   = harmonic_function_name(b.function);
            d["role"]       = b.role;
            return d;
        })
        .def("__repr__", [](const BorrowedInfo& b) {
            return "BorrowedInfo(\"" + b.scale_type
                 + "\", deg=" + std::to_string(b.degree) + ")";
        });

    // ---- PivotInfo (struct) ------------------------------------------------
    py::class_<PivotInfo>(m, "PivotInfo")
        .def_readonly("tonic",       &PivotInfo::tonic)
        .def_readonly("scale_type",  &PivotInfo::scale_type)
        .def_readonly("degree_a",    &PivotInfo::degree_a)
        .def_readonly("degree_b",    &PivotInfo::degree_b)
        .def("to_dict", [](const PivotInfo& p) {
            py::dict d;
            d["tonic"]      = p.tonic;
            d["scale_type"] = p.scale_type;
            d["degree_a"]   = p.degree_a;
            d["degree_b"]   = p.degree_b;
            return d;
        })
        .def("__repr__", [](const PivotInfo& p) {
            return "PivotInfo(\"" + p.tonic + "\", \"" + p.scale_type + "\")";
        });

    // ---- FieldComparison (struct) ------------------------------------------
    py::class_<FieldComparison>(m, "FieldComparison")
        .def_property_readonly("degree_a", [](const FieldComparison& f) -> py::object {
            return f.degree_a ? py::cast(*f.degree_a) : py::none();
        })
        .def_property_readonly("degree_b", [](const FieldComparison& f) -> py::object {
            return f.degree_b ? py::cast(*f.degree_b) : py::none();
        })
        .def_property_readonly("function_a", [](const FieldComparison& f) -> py::object {
            return f.function_a ? py::cast(*f.function_a) : py::none();
        })
        .def_property_readonly("function_b", [](const FieldComparison& f) -> py::object {
            return f.function_b ? py::cast(*f.function_b) : py::none();
        })
        .def_property_readonly("role_a", [](const FieldComparison& f) -> py::object {
            return f.role_a ? py::cast(*f.role_a) : py::none();
        })
        .def_property_readonly("role_b", [](const FieldComparison& f) -> py::object {
            return f.role_b ? py::cast(*f.role_b) : py::none();
        })
        .def_property_readonly("degree_distance", [](const FieldComparison& f) -> py::object {
            return f.degree_distance ? py::cast(*f.degree_distance) : py::none();
        })
        .def_property_readonly("same_function", [](const FieldComparison& f) -> py::object {
            return f.same_function ? py::cast(*f.same_function) : py::none();
        })
        .def_readonly("relative",            &FieldComparison::relative)
        .def_readonly("progression",          &FieldComparison::progression)
        .def_readonly("root_motion",          &FieldComparison::root_motion)
        .def_readonly("secondary_dominant",   &FieldComparison::secondary_dominant)
        .def_readonly("applied_diminished",   &FieldComparison::applied_diminished)
        .def_readonly("diatonic_a",           &FieldComparison::diatonic_a)
        .def_readonly("diatonic_b",           &FieldComparison::diatonic_b)
        .def_property_readonly("borrowed_a", [](const FieldComparison& f) -> py::object {
            return f.borrowed_a ? py::cast(*f.borrowed_a) : py::none();
        })
        .def_property_readonly("borrowed_b", [](const FieldComparison& f) -> py::object {
            return f.borrowed_b ? py::cast(*f.borrowed_b) : py::none();
        })
        .def_readonly("pivot",               &FieldComparison::pivot)
        .def_readonly("tritone_sub",         &FieldComparison::tritone_sub)
        .def_readonly("chromatic_mediant",   &FieldComparison::chromatic_mediant)
        .def_readonly("foreign_a",           &FieldComparison::foreign_a)
        .def_readonly("foreign_b",           &FieldComparison::foreign_b)
        .def("to_dict", [](const FieldComparison& f) {
            auto opt_int = [](const std::optional<int>& v) -> py::object {
                return v ? py::cast(*v) : py::none();
            };
            auto opt_str = [](const std::optional<std::string>& v) -> py::object {
                return v ? py::cast(*v) : py::none();
            };
            auto opt_bool = [](const std::optional<bool>& v) -> py::object {
                return v ? py::cast(*v) : py::none();
            };
            auto opt_func = [](const std::optional<HarmonicFunction>& v) -> py::object {
                return v ? py::cast(harmonic_function_name(*v)) : py::none();
            };
            auto opt_borrowed = [](const std::optional<BorrowedInfo>& v) -> py::object {
                if (!v) return py::none();
                py::dict d;
                d["scale_type"] = v->scale_type;
                d["degree"]     = v->degree;
                d["function"]   = harmonic_function_name(v->function);
                d["role"]       = v->role;
                return py::object(d);
            };
            auto note_names = [](const std::vector<Note>& notes) {
                py::list out;
                for (const auto& n : notes) out.append(n.name());
                return out;
            };

            py::list pivots;
            for (const auto& p : f.pivot) {
                py::dict pd;
                pd["tonic"]      = p.tonic;
                pd["scale_type"] = p.scale_type;
                pd["degree_a"]   = p.degree_a;
                pd["degree_b"]   = p.degree_b;
                pivots.append(pd);
            }

            py::dict d;
            d["degree_a"]           = opt_int(f.degree_a);
            d["degree_b"]           = opt_int(f.degree_b);
            d["function_a"]         = opt_func(f.function_a);
            d["function_b"]         = opt_func(f.function_b);
            d["role_a"]             = opt_str(f.role_a);
            d["role_b"]             = opt_str(f.role_b);
            d["degree_distance"]    = opt_int(f.degree_distance);
            d["same_function"]      = opt_bool(f.same_function);
            d["relative"]           = f.relative;
            d["progression"]        = f.progression;
            d["root_motion"]        = f.root_motion;
            d["secondary_dominant"] = f.secondary_dominant;
            d["applied_diminished"] = f.applied_diminished;
            d["diatonic_a"]         = f.diatonic_a;
            d["diatonic_b"]         = f.diatonic_b;
            d["borrowed_a"]         = opt_borrowed(f.borrowed_a);
            d["borrowed_b"]         = opt_borrowed(f.borrowed_b);
            d["pivot"]              = pivots;
            d["tritone_sub"]        = f.tritone_sub;
            d["chromatic_mediant"]  = f.chromatic_mediant;
            d["foreign_a"]          = note_names(f.foreign_a);
            d["foreign_b"]          = note_names(f.foreign_b);
            return d;
        })
        .def("__repr__", [](const FieldComparison& f) {
            std::string da = f.degree_a ? std::to_string(*f.degree_a) : "None";
            std::string db = f.degree_b ? std::to_string(*f.degree_b) : "None";
            return "FieldComparison(deg_a=" + da + ", deg_b=" + db + ")";
        });

    // ---- Tradition (struct) ------------------------------------------------
    py::class_<Tradition>(m, "Tradition")
        .def_readonly("name",        &Tradition::name)
        .def_readonly("description", &Tradition::description)
        .def("to_dict", [](const Tradition& t) {
            py::dict d;
            d["name"]        = t.name;
            d["description"] = t.description;
            return d;
        })
        .def("__repr__", [](const Tradition& t) {
            return "Tradition(\"" + t.name + "\")";
        })
        .def("__str__", [](const Tradition& t) { return t.name; });

    // ---- Schema (struct) ---------------------------------------------------
    py::class_<Schema>(m, "Schema")
        .def_readonly("name",        &Schema::name)
        .def_readonly("description", &Schema::description)
        .def_readonly("branches",    &Schema::branches)
        .def("to_dict", [](const Schema& s) {
            py::dict d;
            d["name"]        = s.name;
            d["description"] = s.description;
            d["branches"]    = s.branches;
            return d;
        })
        .def("__repr__", [](const Schema& s) {
            return "Schema(\"" + s.name + "\")";
        })
        .def("__str__", [](const Schema& s) { return s.name; });

    // ---- HarmonicPath (struct) ---------------------------------------------
    py::class_<HarmonicPath>(m, "HarmonicPath")
        .def_readonly("id",              &HarmonicPath::id)
        .def_readonly("branch",          &HarmonicPath::branch)
        .def_readonly("chord",           &HarmonicPath::chord)
        .def_readonly("interval_labels", &HarmonicPath::interval_labels)
        .def_readonly("note_names",      &HarmonicPath::note_names)
        .def("__repr__", [](const HarmonicPath& hp) {
            return "HarmonicPath(id=" + std::to_string(hp.id)
                 + ", branch=\"" + hp.branch + "\""
                 + ", chord=" + hp.chord.name() + ")";
        });

    // ---- Tree --------------------------------------------------------------
    py::class_<Tree>(m, "Tree")
        .def(py::init<const std::string&, ScaleType, const std::string&>(),
             py::arg("tonic"), py::arg("type"), py::arg("tradition"))
        .def(py::init<const std::string&, const std::string&, const std::string&>(),
             py::arg("tonic"), py::arg("type"), py::arg("tradition"))
        .def("tonic",       &Tree::tonic)
        .def("type",        &Tree::type)
        .def("tradition",   &Tree::tradition)
        .def("branches",    &Tree::branches)
        .def("paths",       &Tree::paths, py::arg("branch_origin"))
        .def("shortest_path", &Tree::shortest_path,
             py::arg("from"), py::arg("to"))
        .def("is_valid",    &Tree::is_valid, py::arg("branches"))
        .def("function",    &Tree::function, py::arg("branch"))
        .def("branches_with_function", &Tree::branches_with_function,
             py::arg("func"))
        .def("schemas",     &Tree::schemas)
        .def("to_dot",      &Tree::to_dot,
             py::arg("show_functions") = false)
        .def("to_mermaid",  &Tree::to_mermaid)
        .def("__repr__",    &Tree::to_string)
        .def("__str__",     [](const Tree& t) {
            return t.tonic().name() + " " + t.tradition().name;
        });

    // ---- ProgressionMatch (struct, binding-only) ---------------------------
    py::class_<ProgressionMatch>(m, "ProgressionMatch")
        .def_readonly("tradition", &ProgressionMatch::tradition)
        .def_readonly("schema",    &ProgressionMatch::schema)
        .def_readonly("score",     &ProgressionMatch::score)
        .def_readonly("matched",   &ProgressionMatch::matched)
        .def_readonly("total",     &ProgressionMatch::total)
        .def_readonly("branches",  &ProgressionMatch::branches)
        .def("to_dict", [](const ProgressionMatch& pm) {
            py::dict d;
            d["tradition"] = pm.tradition;
            d["schema"]    = pm.schema;
            d["score"]     = pm.score;
            d["matched"]   = pm.matched;
            d["total"]     = pm.total;
            d["branches"]  = pm.branches;
            return d;
        })
        .def("__repr__", [](const ProgressionMatch& pm) {
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "ProgressionMatch(\"%s\", schema=\"%s\", score=%.2f, %d/%d)",
                pm.tradition.c_str(), pm.schema.c_str(),
                pm.score, pm.matched, pm.total);
            return std::string(buf);
        });

    // ---- ProgressionRoute (struct, binding-only) ---------------------------
    py::class_<ProgressionRoute>(m, "ProgressionRoute")
        .def_readonly("next",       &ProgressionRoute::next)
        .def_readonly("tradition",  &ProgressionRoute::tradition)
        .def_readonly("schema",     &ProgressionRoute::schema)
        .def_readonly("path",       &ProgressionRoute::path)
        .def_readonly("confidence", &ProgressionRoute::confidence)
        .def("to_dict", [](const ProgressionRoute& pr) {
            py::dict d;
            d["next"]       = pr.next;
            d["tradition"]  = pr.tradition;
            d["schema"]     = pr.schema;
            d["path"]       = pr.path;
            d["confidence"] = pr.confidence;
            return d;
        })
        .def("__repr__", [](const ProgressionRoute& pr) {
            char buf[128];
            std::snprintf(buf, sizeof(buf),
                "ProgressionRoute(\"%s\", tradition=\"%s\", confidence=%.2f)",
                pr.next.c_str(), pr.tradition.c_str(), pr.confidence);
            return std::string(buf);
        });

    // ---- Progression -------------------------------------------------------
    py::class_<Progression>(m, "Progression")
        .def(py::init<const std::string&, ScaleType>(),
             py::arg("tonic"), py::arg("type"))
        .def(py::init<const std::string&, const std::string&>(),
             py::arg("tonic"), py::arg("type"))
        .def("tonic",      &Progression::tonic)
        .def("type",       &Progression::type)
        .def_static("traditions", &Progression::traditions)
        .def("tree",       &Progression::tree, py::arg("tradition"))
        .def("identify", [](const Progression& self,
                            const std::vector<std::string>& branches) {
            if (branches.empty())
                throw py::value_error("identify() requires at least one branch");

            auto traditions = Progression::traditions();
            ProgressionMatch best{"", "", 0.0, 0, 0, branches};
            double best_schema_score = 0.0;

            for (const auto& trad : traditions) {
                Tree tree = self.tree(trad.name);

                int total_trans = std::max(1,
                    static_cast<int>(branches.size()) - 1);
                int valid_trans = count_valid_transitions(tree, branches);
                double trans_score =
                    static_cast<double>(valid_trans) / total_trans;

                std::string matched_schema;
                double schema_score = 0.0;

                for (const auto& s : tree.schemas()) {
                    if (s.branches == branches) {
                        schema_score = 1.0;
                        matched_schema = s.name;
                        break;
                    }
                    if (is_contiguous_subseq(branches, s.branches)) {
                        double ss = static_cast<double>(branches.size())
                                  / s.branches.size();
                        if (ss > schema_score) {
                            schema_score = ss;
                            matched_schema = s.name;
                        }
                    }
                }

                double final_score = std::max(trans_score, schema_score);

                bool better = final_score > best.score
                    || (final_score == best.score
                        && schema_score > best_schema_score);

                if (better) {
                    best.tradition = trad.name;
                    best.schema    = matched_schema;
                    best.score     = final_score;
                    best.matched   = valid_trans;
                    best.total     = total_trans;
                    best_schema_score = schema_score;
                }
            }

            if (best.score <= 0.0)
                throw py::value_error(
                    "no matching progression found in any tradition");

            return best;
        }, py::arg("branches"))
        .def("deduce", [](const Progression& self,
                          const std::vector<std::string>& branches,
                          int limit) {
            if (branches.empty())
                throw py::value_error(
                    "deduce() requires at least one branch");

            auto traditions = Progression::traditions();
            std::vector<ProgressionMatch> results;

            for (const auto& trad : traditions) {
                Tree tree = self.tree(trad.name);
                int total_trans = std::max(1,
                    static_cast<int>(branches.size()) - 1);
                int valid_trans =
                    count_valid_transitions(tree, branches);
                double trans_score =
                    static_cast<double>(valid_trans) / total_trans;

                bool has_schema_match = false;

                for (const auto& s : tree.schemas()) {
                    double schema_score = 0.0;

                    if (s.branches == branches) {
                        schema_score = 1.0;
                    } else if (is_prefix_of(branches, s.branches)) {
                        schema_score =
                            static_cast<double>(branches.size())
                            / s.branches.size();
                    } else if (is_contiguous_subseq(
                                   branches, s.branches)) {
                        schema_score =
                            static_cast<double>(branches.size())
                            / s.branches.size() * 0.9;
                    }

                    if (schema_score > 0.0) {
                        has_schema_match = true;
                        results.push_back({
                            trad.name, s.name,
                            std::max(trans_score, schema_score),
                            valid_trans, total_trans, branches
                        });
                    }
                }

                if (!has_schema_match && trans_score > 0.0) {
                    results.push_back({
                        trad.name, "", trans_score,
                        valid_trans, total_trans, branches
                    });
                }
            }

            std::stable_sort(results.begin(), results.end(),
                [](const ProgressionMatch& a,
                   const ProgressionMatch& b) {
                    return a.score > b.score;
                });

            if (limit > 0
                && static_cast<int>(results.size()) > limit)
                results.erase(results.begin() + limit,
                              results.end());

            py::list out;
            for (auto& r : results)
                out.append(py::cast(std::move(r)));
            return out;
        }, py::arg("branches"), py::arg("limit") = 10)
        .def("predict", [](const Progression& self,
                           const std::vector<std::string>& branches,
                           const std::string& tradition_filter) {
            if (branches.empty())
                throw py::value_error(
                    "predict() requires at least one branch");

            auto traditions = Progression::traditions();
            std::vector<ProgressionRoute> results;
            std::string last = branches.back();

            for (const auto& trad : traditions) {
                if (!tradition_filter.empty()
                    && trad.name != tradition_filter)
                    continue;

                Tree tree = self.tree(trad.name);
                auto reachable = tree.paths(last);
                auto schemas = tree.schemas();

                for (std::size_t i = 1; i < reachable.size(); ++i) {
                    std::string next = reachable[i].branch;

                    auto candidate = branches;
                    candidate.push_back(next);

                    std::string matched_schema;
                    double confidence = 0.3;

                    for (const auto& s : schemas) {
                        if (is_prefix_of(candidate, s.branches)) {
                            double c =
                                static_cast<double>(candidate.size())
                                / s.branches.size();
                            if (c > confidence) {
                                confidence = c;
                                matched_schema = s.name;
                            }
                        }
                        if (is_contiguous_subseq(
                                candidate, s.branches)) {
                            double c =
                                static_cast<double>(candidate.size())
                                / s.branches.size() * 0.8;
                            if (c > confidence) {
                                confidence = c;
                                matched_schema = s.name;
                            }
                        }
                    }

                    results.push_back({
                        next, trad.name, matched_schema,
                        candidate, confidence
                    });
                }
            }

            std::stable_sort(results.begin(), results.end(),
                [](const ProgressionRoute& a,
                   const ProgressionRoute& b) {
                    return a.confidence > b.confidence;
                });

            py::list out;
            for (auto& r : results)
                out.append(py::cast(std::move(r)));
            return out;
        }, py::arg("branches"), py::arg("tradition") = "")
        .def("__repr__", &Progression::to_string)
        .def("__str__",  [](const Progression& p) {
            return p.tonic().name() + " progression";
        });

    // ---- Duration ----------------------------------------------------------
    py::class_<Duration>(m, "Duration")
        .def(py::init<const std::string&, int, int>(),
             py::arg("name"), py::arg("dots") = 0, py::arg("tuplet") = 0)
        .def(py::init<int, int>(),
             py::arg("numerator"), py::arg("denominator"))
        .def("name",        &Duration::name)
        .def("dots",        &Duration::dots)
        .def("tuplet",      &Duration::tuplet)
        .def("beats",       &Duration::beats, py::arg("reference_value") = 1.0)
        .def("rational",    &Duration::rational)
        .def("numerator",   &Duration::numerator)
        .def("denominator", &Duration::denominator)
        .def("midi_ticks",  &Duration::midi_ticks, py::arg("ppqn") = 480)
        .def_static("from_ticks", &Duration::from_ticks,
                    py::arg("ticks"), py::arg("ppqn") = 480)
        .def_static("standard_names", &Duration::standard_names)
        .def("__add__",     &Duration::operator+)
        .def("__mul__",     &Duration::operator*, py::arg("factor"))
        .def("__eq__",      &Duration::operator==)
        .def("__ne__",      &Duration::operator!=)
        .def("__lt__",      &Duration::operator<)
        .def("__repr__",    &Duration::to_string)
        .def("__str__",     &Duration::name);

    // ---- Tempo -------------------------------------------------------------
    py::class_<Tempo>(m, "Tempo")
        .def(py::init<double>(), py::arg("bpm"))
        .def(py::init<const std::string&>(), py::arg("marking"))
        .def("bpm",          &Tempo::bpm)
        .def("marking",      &Tempo::marking)
        .def("seconds",      &Tempo::seconds, py::arg("duration"))
        .def("ms_per_beat",  &Tempo::ms_per_beat)
        .def("microseconds_per_beat", &Tempo::microseconds_per_beat)
        .def_static("bpm_to_marking",  &Tempo::bpm_to_marking, py::arg("bpm"))
        .def_static("marking_to_bpm",  &Tempo::marking_to_bpm, py::arg("marking"))
        .def_static("from_microseconds", &Tempo::from_microseconds, py::arg("usec"))
        .def("__eq__",       &Tempo::operator==)
        .def("__ne__",       &Tempo::operator!=)
        .def("__repr__",     &Tempo::to_string)
        .def("__str__",      [](const Tempo& t) {
            return std::to_string(static_cast<int>(t.bpm())) + " BPM";
        })
        .def("__float__",    &Tempo::bpm)
        .def("__int__",      [](const Tempo& t) { return static_cast<int>(t.bpm()); });

    // ---- TimeSignature -----------------------------------------------------
    py::class_<TimeSignature>(m, "TimeSignature")
        .def(py::init<int, int>(),
             py::arg("beats_per_bar"), py::arg("beat_unit"))
        .def("beats_per_bar",  &TimeSignature::beats_per_bar)
        .def("beat_unit",      &TimeSignature::beat_unit)
        .def("signature",      &TimeSignature::signature)
        .def("bar_duration",   &TimeSignature::bar_duration)
        .def("classification", &TimeSignature::classification)
        .def("common_name",    &TimeSignature::common_name)
        .def("__eq__",         &TimeSignature::operator==)
        .def("__ne__",         &TimeSignature::operator!=)
        .def("__repr__",       &TimeSignature::to_string)
        .def("__str__",        &TimeSignature::to_string);

    // ---- NoteEvent ---------------------------------------------------------
    py::class_<NoteEvent>(m, "NoteEvent")
        .def(py::init<const Note&, const Duration&, int>(),
             py::arg("note"), py::arg("duration"), py::arg("octave") = 4)
        .def("note",         &NoteEvent::note)
        .def("octave",       &NoteEvent::octave)
        .def("midi_number",  &NoteEvent::midi_number)
        .def("duration",     &NoteEvent::duration)
        .def("frequency",    &NoteEvent::frequency, py::arg("tuning") = 440.0)
        .def("__eq__",       &NoteEvent::operator==)
        .def("__ne__",       &NoteEvent::operator!=)
        .def("__repr__",     &NoteEvent::to_string)
        .def("__str__",      [](const NoteEvent& e) {
            return e.note().name() + std::to_string(e.octave());
        });

    // ---- ChordEvent --------------------------------------------------------
    py::class_<ChordEvent>(m, "ChordEvent")
        .def(py::init<const Chord&, const Duration&, int>(),
             py::arg("chord"), py::arg("duration"), py::arg("octave") = 4)
        .def("chord",        &ChordEvent::chord)
        .def("octave",       &ChordEvent::octave)
        .def("duration",     &ChordEvent::duration)
        .def("note_events",  &ChordEvent::note_events)
        .def("__eq__",       &ChordEvent::operator==)
        .def("__ne__",       &ChordEvent::operator!=)
        .def("__repr__",     &ChordEvent::to_string)
        .def("__str__",      [](const ChordEvent& e) {
            return e.chord().name();
        });

    // ---- Rest --------------------------------------------------------------
    py::class_<Rest>(m, "Rest")
        .def(py::init<const Duration&>(), py::arg("duration"))
        .def("duration",     &Rest::duration)
        .def("__eq__",       &Rest::operator==)
        .def("__ne__",       &Rest::operator!=)
        .def("__repr__",     &Rest::to_string)
        .def("__str__",      [](const Rest&) -> std::string { return "Rest"; });

    // ---- Sequence ----------------------------------------------------------
    py::class_<Sequence>(m, "Sequence")
        .def(py::init<const Tempo&, const TimeSignature&>(),
             py::arg("tempo") = Tempo(120),
             py::arg("time_signature") = TimeSignature(4, 4))
        .def("tempo",          &Sequence::tempo)
        .def("set_tempo",      &Sequence::set_tempo, py::arg("tempo"))
        .def("time_signature", &Sequence::time_signature)
        .def("set_time_signature", &Sequence::set_time_signature, py::arg("ts"))
        .def("add",            [](Sequence& self, py::object obj) {
            if (py::isinstance<NoteEvent>(obj))
                self.add(obj.cast<NoteEvent>());
            else if (py::isinstance<ChordEvent>(obj))
                self.add(obj.cast<ChordEvent>());
            else if (py::isinstance<Rest>(obj))
                self.add(obj.cast<Rest>());
            else
                throw py::type_error("add() requires NoteEvent, ChordEvent, or Rest");
        }, py::arg("event"))
        .def("remove",         &Sequence::remove, py::arg("index"))
        .def("clear",          &Sequence::clear)
        .def("total_duration", &Sequence::total_duration)
        .def("total_seconds",  &Sequence::total_seconds)
        .def("bar_count",      &Sequence::bar_count)
        .def("transpose",      &Sequence::transpose, py::arg("semitones"))
        .def("to_midi",        &Sequence::to_midi,
             py::arg("path"), py::arg("ppqn") = 480)
        .def_static("from_midi", &Sequence::from_midi, py::arg("path"))
        .def("__len__",        &Sequence::size)
        .def("__getitem__",    [](const Sequence& self, int index) -> py::object {
            int n = static_cast<int>(self.size());
            if (index < 0) index += n;
            if (index < 0 || index >= n)
                throw py::index_error("Sequence index out of range");
            const auto& ev = self.at(static_cast<size_t>(index));
            return std::visit([](const auto& e) -> py::object {
                return py::cast(e);
            }, ev);
        }, py::arg("index"))
        .def("__eq__",         &Sequence::operator==)
        .def("__ne__",         &Sequence::operator!=)
        .def("__repr__",       &Sequence::to_string)
        .def("__str__",        [](const Sequence& s) {
            return std::to_string(s.size()) + " events, "
                 + std::to_string(s.total_duration()) + " beats";
        });

    // ---- PianoKey (struct) -------------------------------------------------
    py::class_<PianoKey>(m, "PianoKey")
        .def_readonly("midi",     &PianoKey::midi)
        .def_readonly("octave",   &PianoKey::octave)
        .def_readonly("note",     &PianoKey::note)
        .def_readonly("white",    &PianoKey::white)
        .def_readonly("position", &PianoKey::position)
        .def("to_dict", [](const PianoKey& k) {
            py::dict d;
            d["midi"]     = k.midi;
            d["octave"]   = k.octave;
            d["note"]     = k.note;
            d["white"]    = k.white;
            d["position"] = k.position;
            return d;
        })
        .def("__repr__", &PianoKey::to_string)
        .def("__str__",  [](const PianoKey& k) {
            return k.note + std::to_string(k.octave);
        });

    // ---- VoicingStyle (enum) -----------------------------------------------
    py::enum_<VoicingStyle>(m, "VoicingStyle")
        .value("Close", VoicingStyle::Close)
        .value("Open",  VoicingStyle::Open)
        .value("Shell", VoicingStyle::Shell);

    py::enum_<Layout>(m, "Layout")
        .value("Vertical",   Layout::Vertical)
        .value("Horizontal", Layout::Horizontal)
        .value("Grid",       Layout::Grid);

    py::enum_<Orientation>(m, "Orientation")
        .value("Horizontal", Orientation::Horizontal)
        .value("Vertical",   Orientation::Vertical);

    py::enum_<Handedness>(m, "Handedness")
        .value("RightHanded", Handedness::RightHanded)
        .value("LeftHanded",  Handedness::LeftHanded);

    // ---- PianoVoicing (struct) ---------------------------------------------
    py::class_<PianoVoicing>(m, "PianoVoicing")
        .def_readonly("keys",       &PianoVoicing::keys)
        .def_readonly("style",      &PianoVoicing::style)
        .def_readonly("chord_name", &PianoVoicing::chord_name)
        .def_readonly("inversion",  &PianoVoicing::inversion)
        .def("to_dict", [](const PianoVoicing& v) {
            py::dict d;
            py::list keys;
            for (const auto& k : v.keys) {
                py::dict kd;
                kd["midi"]     = k.midi;
                kd["octave"]   = k.octave;
                kd["note"]     = k.note;
                kd["white"]    = k.white;
                kd["position"] = k.position;
                keys.append(kd);
            }
            d["keys"]       = keys;
            d["style"]      = py::cast(v.style);
            d["chord_name"] = v.chord_name;
            d["inversion"]  = v.inversion;
            return d;
        })
        .def("__repr__", &PianoVoicing::to_string)
        .def("__str__",  [](const PianoVoicing& v) {
            return v.chord_name + " voicing";
        });

    // ---- Piano -------------------------------------------------------------
    py::class_<Piano>(m, "Piano")
        .def(py::init<int>(), py::arg("num_keys") = 88)
        .def("num_keys", &Piano::num_keys)
        .def("lowest",   &Piano::lowest)
        .def("highest",  &Piano::highest)
        .def("in_range", &Piano::in_range, py::arg("midi"))
        .def("key",       &Piano::key,
             py::arg("note"), py::arg("octave") = 4)
        .def("keys",      &Piano::keys, py::arg("note"))
        .def("voicing",   &Piano::voicing,
             py::arg("chord"), py::arg("octave") = 4,
             py::arg("style") = VoicingStyle::Close)
        .def("voicings",  &Piano::voicings,
             py::arg("chord"), py::arg("octave") = 4)
        .def("scale_keys", &Piano::scale_keys,
             py::arg("scale"), py::arg("octave") = 4)
        .def("note_at",   &Piano::note_at, py::arg("midi"))
        .def("identify",  &Piano::identify, py::arg("midi_numbers"))
        .def("__repr__",  &Piano::to_string)
        .def("__str__",   [](const Piano& p) {
            return "Piano(" + std::to_string(p.num_keys()) + " keys)";
        });

    // ---- MusicXML ----------------------------------------------------------
    py::class_<MusicXML>(m, "MusicXML")
        .def_static("note", &MusicXML::note,
             py::arg("note"), py::arg("octave") = 4,
             py::arg("type") = "quarter")
        .def_static("chord", &MusicXML::chord,
             py::arg("chord"), py::arg("octave") = 4,
             py::arg("type") = "whole")
        .def_static("scale", &MusicXML::scale,
             py::arg("scale"), py::arg("octave") = 4,
             py::arg("type") = "quarter")
        .def_static("field", &MusicXML::field,
             py::arg("field"), py::arg("octave") = 4,
             py::arg("type") = "whole")
        .def_static("sequence", &MusicXML::sequence, py::arg("sequence"))
        .def_static("write", &MusicXML::write,
             py::arg("xml"), py::arg("path"));

    // ---- FretPosition (struct) ----------------------------------------------
    py::class_<FretPosition>(m, "FretPosition")
        .def_readonly("string", &FretPosition::string)
        .def_readonly("fret",   &FretPosition::fret)
        .def_readonly("midi",   &FretPosition::midi)
        .def_readonly("note",   &FretPosition::note)
        .def_readonly("octave", &FretPosition::octave)
        .def("to_dict", [](const FretPosition& p) {
            py::dict d;
            d["string"] = p.string;
            d["fret"]   = p.fret;
            d["midi"]   = p.midi;
            d["note"]   = p.note;
            d["octave"] = p.octave;
            return d;
        })
        .def("__repr__", &FretPosition::to_string)
        .def("__str__",  [](const FretPosition& p) {
            return p.note + std::to_string(p.octave);
        });

    // ---- StringAction (enum) ------------------------------------------------
    py::enum_<StringAction>(m, "StringAction")
        .value("Open",    StringAction::Open)
        .value("Fretted", StringAction::Fretted)
        .value("Muted",   StringAction::Muted);

    // ---- StringState (struct) -----------------------------------------------
    py::class_<StringState>(m, "StringState")
        .def_readonly("string", &StringState::string)
        .def_readonly("action", &StringState::action)
        .def_readonly("fret",   &StringState::fret)
        .def_readonly("finger", &StringState::finger)
        .def("__repr__", [](const StringState& ss) {
            std::string act;
            switch (ss.action) {
                case StringAction::Open:    act = "open"; break;
                case StringAction::Fretted: act = "fret " + std::to_string(ss.fret); break;
                case StringAction::Muted:   act = "muted"; break;
            }
            return "StringState(string=" + std::to_string(ss.string) + ", " + act + ")";
        });

    // ---- Tuning (struct) ----------------------------------------------------
    py::class_<Tuning>(m, "Tuning")
        .def(py::init<>())
        .def_readwrite("name",      &Tuning::name)
        .def_readwrite("open_midi", &Tuning::open_midi)
        .def_readwrite("num_frets", &Tuning::num_frets)
        .def("__repr__", [](const Tuning& t) {
            return "Tuning(\"" + t.name + "\", " +
                   std::to_string(t.open_midi.size()) + " strings, " +
                   std::to_string(t.num_frets) + " frets)";
        });

    // ---- Fingering (struct) -------------------------------------------------
    py::class_<Fingering>(m, "Fingering")
        .def_readonly("strings",    &Fingering::strings)
        .def_readonly("chord_name", &Fingering::chord_name)
        .def_readonly("barre",      &Fingering::barre)
        .def_readonly("base_fret",  &Fingering::base_fret)
        .def_readonly("capo",       &Fingering::capo)
        .def_readonly("midi_notes", &Fingering::midi_notes)
        .def("to_dict", [](const Fingering& f) {
            py::dict d;
            py::list strings;
            for (const auto& ss : f.strings) {
                py::dict sd;
                sd["string"] = ss.string;
                sd["action"] = py::cast(ss.action);
                sd["fret"]   = ss.fret;
                sd["finger"] = ss.finger;
                strings.append(sd);
            }
            d["strings"]    = strings;
            d["chord_name"] = f.chord_name;
            d["barre"]      = f.barre;
            d["base_fret"]  = f.base_fret;
            d["capo"]       = f.capo;
            d["midi_notes"] = f.midi_notes;
            return d;
        })
        .def("__repr__", &Fingering::to_string)
        .def("__str__",  [](const Fingering& f) {
            return f.chord_name + " fingering";
        });

    // ---- Fretboard ----------------------------------------------------------
    py::class_<Fretboard>(m, "Fretboard")
        .def(py::init<const Tuning&>(), py::arg("tuning"))
        .def(py::init<const std::string&, const std::vector<int>&, int>(),
             py::arg("name"), py::arg("open_midi"), py::arg("num_frets") = 19)
        .def_static("cavaquinho", &Fretboard::cavaquinho,
             py::arg("num_frets") = 17)
        .def_static("violao", &Fretboard::violao,
             py::arg("num_frets") = 19)
        .def_static("bandolim", &Fretboard::bandolim,
             py::arg("num_frets") = 17)
        .def("name",        &Fretboard::name)
        .def("num_strings", &Fretboard::num_strings)
        .def("num_frets",   &Fretboard::num_frets)
        .def("tuning",      &Fretboard::tuning)
        .def("position",    &Fretboard::position,
             py::arg("string"), py::arg("fret"))
        .def("positions",   &Fretboard::positions, py::arg("note"))
        .def("note_at",     &Fretboard::note_at,
             py::arg("string"), py::arg("fret"))
        .def("midi_at",     &Fretboard::midi_at,
             py::arg("string"), py::arg("fret"))
        .def("scale_positions",
             py::overload_cast<const Scale&>(&Fretboard::scale_positions, py::const_),
             py::arg("scale"))
        .def("scale_positions",
             py::overload_cast<const Scale&, int, int>(&Fretboard::scale_positions, py::const_),
             py::arg("scale"), py::arg("fret_lo"), py::arg("fret_hi"))
        .def("fingering",   &Fretboard::fingering,
             py::arg("chord"), py::arg("position") = 0)
        .def("fingerings",  &Fretboard::fingerings,
             py::arg("chord"), py::arg("max_results") = 5)
        .def("identify",    &Fretboard::identify, py::arg("string_frets"))
        .def("capo",        &Fretboard::capo, py::arg("fret"))
        .def("__repr__",    &Fretboard::to_string)
        .def("__str__",     [](const Fretboard& fb) {
            return "Fretboard(\"" + fb.name() + "\", " +
                   std::to_string(fb.num_strings()) + " strings)";
        });

    // ---- FretboardSVG -------------------------------------------------------
    py::class_<FretboardSVG>(m, "FretboardSVG")
        .def_static("chord", &FretboardSVG::chord,
             py::arg("fretboard"), py::arg("chord"), py::arg("position") = 0,
             py::arg("orientation") = Orientation::Vertical,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("fingering", &FretboardSVG::fingering,
             py::arg("fretboard"), py::arg("fingering"),
             py::arg("orientation") = Orientation::Vertical,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("scale", &FretboardSVG::scale,
             py::arg("fretboard"), py::arg("scale"),
             py::arg("fret_lo") = 0, py::arg("fret_hi") = 12,
             py::arg("orientation") = Orientation::Horizontal,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("positions", &FretboardSVG::positions,
             py::arg("fretboard"), py::arg("highlighted"),
             py::arg("title") = "",
             py::arg("orientation") = Orientation::Horizontal,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("note", &FretboardSVG::note,
             py::arg("fretboard"), py::arg("note"),
             py::arg("orientation") = Orientation::Horizontal,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("field", &FretboardSVG::field,
             py::arg("fretboard"), py::arg("field"),
             py::arg("layout") = Layout::Vertical,
             py::arg("orientation") = Orientation::Vertical,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("progression", &FretboardSVG::progression,
             py::arg("fretboard"), py::arg("field"),
             py::arg("branches"),
             py::arg("layout") = Layout::Vertical,
             py::arg("orientation") = Orientation::Vertical,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("full", &FretboardSVG::full,
             py::arg("fb"),
             py::arg("orientation") = Orientation::Horizontal,
             py::arg("handedness") = Handedness::RightHanded)
        .def_static("write", &FretboardSVG::write,
             py::arg("svg"), py::arg("path"));

    // ---- PianoSVG -----------------------------------------------------------
    py::class_<PianoSVG>(m, "PianoSVG")
        .def_static("note", &PianoSVG::note,
             py::arg("piano"), py::arg("note"), py::arg("octave") = 4,
             py::arg("compact") = false)
        .def_static("chord", &PianoSVG::chord,
             py::arg("piano"), py::arg("chord"), py::arg("octave") = 4,
             py::arg("style") = VoicingStyle::Close,
             py::arg("compact") = false)
        .def_static("scale", &PianoSVG::scale,
             py::arg("piano"), py::arg("scale"), py::arg("octave") = 4,
             py::arg("compact") = false)
        .def_static("keys", &PianoSVG::keys,
             py::arg("piano"), py::arg("highlighted"),
             py::arg("title") = "",
             py::arg("compact") = false)
        .def_static("voicing", &PianoSVG::voicing,
             py::arg("piano"), py::arg("voicing"),
             py::arg("compact") = false)
        .def_static("midi", &PianoSVG::midi,
             py::arg("piano"), py::arg("midi_numbers"),
             py::arg("compact") = false)
        .def_static("field", &PianoSVG::field,
             py::arg("piano"), py::arg("field"),
             py::arg("octave") = 4,
             py::arg("layout") = Layout::Vertical,
             py::arg("sevenths") = false)
        .def_static("progression", &PianoSVG::progression,
             py::arg("piano"), py::arg("field"),
             py::arg("branches"),
             py::arg("octave") = 4,
             py::arg("layout") = Layout::Vertical)
        .def_static("write", &PianoSVG::write,
             py::arg("svg"), py::arg("path"));

}
