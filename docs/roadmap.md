# Roadmap

Estado atual do projeto e visao de futuro.

---

## v1.0.0 — Fundamentos

### Core (C++17)

- [x] Note, Interval, Chord (42 formulas), Scale (10 familias, 36 modos)
- [x] Field: triades, setimas, funcoes T/S/D, acordes aplicados
- [x] Field.identify() e Field.deduce() — inferencia de tonalidade
- [x] Comparacao neo-Riemannian (P/L/R, 2-step, voice leading, 18 dimensoes)
- [x] Comparacao contextual (21 dimensoes: borrowing, pivot, tritone sub, etc.)
- [x] Tree: arvore harmonica, caminhos, validacao de progressoes, DOT/Mermaid

### Ritmo (C++17)

- [x] Duration (fracao racional exata, nomes, pontuacao, tercinas)
- [x] Tempo (BPM, marcacoes italianas, conversao para segundos)
- [x] TimeSignature (formula de compasso)
- [x] NoteEvent, ChordEvent, Rest
- [x] Sequence (composicao com tempo, __getitem__, transpose)

### Audio (Python)

- [x] Sintese: sine, square, sawtooth, triangle
- [x] Envelope ADSR (attack, decay, sustain, release)
- [x] Strum e gap
- [x] `.play()` e `.to_wav()` em todas as classes
- [x] CLI: `--play`, `--wav`, `--waveform`, `--strum`, `--gap`

### Infra

- [x] Zero dependencias runtime (simpleaudio opcional para playback)
- [x] Core C++17 com pybind11
- [x] CLI completa (note, interval, chord, scale, field, compare)
- [x] Type stubs (PEP 561)
- [x] Documentacao MkDocs

---

## v1.0.1 — Ritmo CLI

- [x] CLI de ritmo: duration, tempo, timesig

---

## v1.0.2 — Progressoes, Piano, MusicXML

### Progressoes e Tradicoes

- [x] Progression: coordenador cross-tradition (identify, deduce, predict)
- [x] Tree: construtor 3 argumentos, tradition(), schemas(), is_valid()
- [x] Tradicao Jazz: branches e schemas classicos (ii-V-I, turnaround, backdoor, etc.)
- [x] CLI: tree, progression com identify/deduce/predict

### Piano

- [x] Piano: mapeamento teoria <-> teclas fisicas (MIDI, voicings, reverse identification)
- [x] VoicingStyle: Close, Open, Shell
- [x] PianoSVG: visualizacao interativa do teclado em SVG
  - note, chord, scale, voicing, keys, midi, field, progression
  - HTML5 data attributes (data-midi, data-note, data-octave, data-color)
  - CSS classes para integracao com JS/D3/React

### Notacao

- [x] MusicXML: serializacao para MusicXML 4.0 partwise
  - Note, Chord, Scale, Field, Sequence -> MuseScore/Finale/Sibelius

---

## v1.1.0 — Fretboard

### Fretboard (instrumentos de cordas)

- [x] Fretboard: motor de digitacao para violao, cavaquinho, bandolim
  - Factory methods: `Fretboard.violao()`, `.cavaquinho()`, `.bandolim()`
  - Instrumentos customizados: `Fretboard(tuning, num_frets)`
  - `fingering(chord)` — digitacao otima baseada no sistema CAGED
  - `scale_positions(scale, fret_lo, fret_hi)` — posicoes da escala no braco
  - `positions(note)` — todas as ocorrencias de uma nota
  - Algoritmo multi-criterio: span, posicao, dedos, cordas, barre, conforto
  - 22/24 formas CAGED padrao identicas a jguitar.com

- [x] FretboardSVG: renderizador SVG para diagramas de fretboard
  - chord, fingering, scale, note, positions, field, progression, full
  - Orientacao: `Horizontal` (braco) / `Vertical` (chord box)
  - Lateralidade: `RightHanded` / `LeftHanded`
  - Layouts compostos: Grid, Horizontal, Vertical para campos e progressoes
  - `write()` para salvar SVG em arquivo

- [x] Structs: Tuning, FretPosition, Fingering, StringState
- [x] Enums: Orientation, Handedness, StringAction
- [x] CLI: `gingo fretboard chord/scale/field` com `--svg`, `--left`, `--horizontal`

---

## v1.2.0 — MIDI + Duration Parsing (versao atual)

### Duration Parsing

- [x] Abreviacoes: `Duration("q")`, `Duration("h")`, `Duration("w")`, `Duration("e")`, `Duration("s")`
- [x] Dotted: `Duration("q.")`, `Duration("h..")`
- [x] LilyPond: `Duration("4")`, `Duration("8.")`, `Duration("16")`
- [x] Fracoes: `Duration("1/4")`, `Duration("3/8")`

### MIDI Conversions

- [x] `Duration.midi_ticks(ppqn=480)` — converte para ticks MIDI
- [x] `Duration.from_ticks(ticks, ppqn)` — cria Duration a partir de ticks
- [x] `Tempo.microseconds_per_beat()` — formato MIDI meta-event
- [x] `Tempo.from_microseconds(usec)` — cria Tempo a partir de microsegundos

### MIDI Import/Export

- [x] `Sequence.to_midi(path, ppqn)` — export SMF format 0
- [x] `Sequence.from_midi(path)` — import MIDI (format 0/1)
- [x] Suporte a NoteEvent, ChordEvent, Rest no roundtrip
- [x] Meta-events: tempo, time signature
- [x] Chord detection via `Chord.identify()` no import

---

## Planejado

### Escalas Bebop

- [ ] 6 tipos de escalas bebop (8 notas com passing tone cromatico)
- [ ] Aliases de modos simetricos existentes

---

## Diferenciais do Gingo

- **Teoria completa em camadas**: Note -> Interval -> Chord -> Scale -> Field -> Progression
- **Campo harmonico** com funcoes T/S/D e acordes aplicados
- **Comparacao contextual** com 21 dimensoes (neo-Riemannian, voice leading, borrowing, pivot)
- **Arvore harmonica** com caminhos, validacao e cross-tradition (classico + jazz)
- **Field.deduce()** — inferencia de tonalidade a partir de dados parciais
- **Fretboard** com algoritmo CAGED para digitacoes realisticas de violao/cavaquinho/bandolim
- **FretboardSVG** com orientacao (horizontal/vertical) e lateralidade (destro/canhoto)
- **Piano** com voicings (close, open, shell) e **PianoSVG** interativo
- **MusicXML 4.0** para exportacao de partitura
- **MIDI import/export** com `Sequence.to_midi()` e `Sequence.from_midi()`
- **Duration flexivel** com parsing de abreviacoes, LilyPond e fracoes
- **Sintese de audio** integrada com `.play()` e `.to_wav()` em todas as classes
- **Duration racional** exata (fracao, nao float)
- **Zero dependencias** runtime + backend C++17
