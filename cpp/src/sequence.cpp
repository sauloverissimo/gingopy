// Gingo — Music Theory Library
// Sequence implementation.
//
// SPDX-License-Identifier: MIT

#include <gingo/sequence.hpp>
#include <sstream>
#include <stdexcept>

namespace gingo {

// ---------------------------------------------------------------------------
// Helper to get duration from variant Event

static const Duration& get_duration(const Event& event) {
    return std::visit([](const auto& e) -> const Duration& {
        return e.duration();
    }, event);
}

// ---------------------------------------------------------------------------
// Constructor

Sequence::Sequence(const Tempo& tempo, const TimeSignature& time_signature)
    : tempo_(tempo), time_signature_(time_signature)
{}

Sequence::Sequence(const std::vector<Event>& events, const Tempo& tempo,
                   const TimeSignature& time_signature)
    : events_(events), tempo_(tempo), time_signature_(time_signature)
{}

// ---------------------------------------------------------------------------
// Modifiers

void Sequence::add(const Event& event) {
    events_.push_back(event);
}

void Sequence::remove(size_t index) {
    if (index >= events_.size()) {
        throw std::out_of_range("Event index out of range");
    }
    events_.erase(events_.begin() + index);
}

// ---------------------------------------------------------------------------
// Metrics

double Sequence::total_duration() const {
    double total = 0.0;
    for (const auto& event : events_) {
        total += get_duration(event).beats();
    }
    return total;
}

double Sequence::total_seconds() const {
    double total = 0.0;
    for (const auto& event : events_) {
        total += tempo_.seconds(get_duration(event));
    }
    return total;
}

int Sequence::bar_count() const {
    double bar_duration = time_signature_.bar_duration().beats();
    if (bar_duration == 0.0) return 0;
    return static_cast<int>(std::ceil(total_duration() / bar_duration));
}

// ---------------------------------------------------------------------------
// Transformations

Sequence Sequence::transpose(int semitones) const {
    Sequence result(tempo_, time_signature_);

    for (const auto& event : events_) {
        std::visit([&](const auto& e) {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, NoteEvent>) {
                Note transposed = e.note().transpose(semitones);
                result.add(NoteEvent(transposed, e.duration(), e.octave()));
            } else if constexpr (std::is_same_v<T, ChordEvent>) {
                Chord transposed = e.chord().transpose(semitones);
                result.add(ChordEvent(transposed, e.duration(), e.octave()));
            } else {
                // Rest: no transposition
                result.add(e);
            }
        }, event);
    }

    return result;
}

// ---------------------------------------------------------------------------
// Comparison

bool Sequence::operator==(const Sequence& other) const {
    if (events_.size() != other.events_.size()) return false;
    if (tempo_ != other.tempo_) return false;
    if (time_signature_ != other.time_signature_) return false;

    for (size_t i = 0; i < events_.size(); ++i) {
        // Compare variant events
        bool same = std::visit([&](const auto& a) {
            return std::visit([&](const auto& b) -> bool {
                using TA = std::decay_t<decltype(a)>;
                using TB = std::decay_t<decltype(b)>;
                if constexpr (std::is_same_v<TA, TB>) {
                    return a == b;
                } else {
                    return false;
                }
            }, other.events_[i]);
        }, events_[i]);

        if (!same) return false;
    }

    return true;
}

// ---------------------------------------------------------------------------
// String representation

std::string Sequence::to_string() const {
    std::ostringstream oss;
    oss << "Sequence [" << tempo_.to_string() << ", "
        << time_signature_.to_string() << "] "
        << events_.size() << " events, "
        << total_duration() << " beats";
    return oss.str();
}

}  // namespace gingo
