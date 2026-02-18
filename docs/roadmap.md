# Roadmap

Funcionalidades planejadas e estado atual do projeto.

---

## Implementado (v1.0.0)

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
- [x] Strum (delay entre notas de acordes) e gap (silencio entre eventos)
- [x] `.play()` e `.to_wav()` em todas as classes (Note, Chord, Scale, Field, Tree, Sequence)
- [x] Funcoes standalone: `play()`, `to_wav()`
- [x] Playback: simpleaudio (opcional) com fallback para players do sistema
- [x] CLI: `--play`, `--wav`, `--waveform`, `--strum`, `--gap`

### Infra

- [x] Zero dependencias runtime (simpleaudio opcional para playback)
- [x] Core C++17 com pybind11
- [x] CLI completa com subcomandos (note, interval, chord, scale, field, compare)
- [x] CLI de ritmo: duration, tempo, timesig
- [x] Type stubs (PEP 561)
- [x] Documentacao MkDocs completa

---

## Implementado (v1.0.2)

### Progressoes e Tradicoes

- [x] Progression: coordenador cross-tradition (identify, deduce, predict)
- [x] Tree: construtor 3 argumentos, tradition(), schemas(), is_valid()
- [x] Tradicao Jazz: branches e schemas classicos (ii-V-I, turnaround, backdoor, etc.)
- [x] CLI: tree, progression com identify/deduce/predict

---

## Implementado (v1.0.2)

### Instrumentos — Piano

- [x] Piano: mapeamento teoria ↔ teclas fisicas (MIDI, voicings, reverse identification)
- [x] VoicingStyle: Close, Open, Shell
- [x] PianoSVG: visualizacao interativa do teclado em SVG
  - note, chord, scale, voicing, keys, midi, field, progression
  - HTML5 data attributes (data-midi, data-note, data-octave, data-color)
  - CSS classes para integracao com JS/D3/React

### Notacao

- [x] MusicXML: serializacao para MusicXML 4.0 partwise
  - Note, Chord, Scale, Field, Sequence → MuseScore/Finale/Sibelius

---

## Implementado (v1.1.0)

### Instrumentos — Fretboard (Cordas)

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

## Planejado

### 1. MIDI (`gingo[audio]`)

- [ ] `Duration.midi_ticks(ppqn=480)` / `Duration.from_ticks(ticks, ppqn)`
- [ ] `Tempo.microseconds_per_beat()` / `Tempo.from_microseconds(usec)`
- [ ] `bpm2tempo(bpm)` / `tempo2bpm(usec)` — conversao MIDI
- [ ] `Sequence.to_midi()` / `Sequence.from_midi(path)` — import/export .mid

### 2. Partitura (`gingo[score]`)

- [ ] `Sequence.to_lilypond()` — export LilyPond
- [ ] `Sequence.show()` — renderiza via MuseScore ou LilyPond
- [ ] Suporte a claves, dinamicas, articulacoes

### 3. Escalas futuras

- [ ] Bebop (6 tipos, 8 notas com passing tone cromatico)
- [ ] Aliases de modos simetricos existentes
- [ ] Ver `.old/scales_analysis.md` secao 14.9

### 4. Estrutura hierarquica

- [ ] `Bar(events, time_signature)` — compasso completo
- [ ] `Phrase(bars)` — frase musical
- [ ] `Part(phrases, instrument)` — uma voz/instrumento
- [ ] `Score(parts, tempo)` — partitura com multiplas vozes

### 5. Offset absoluto

- [ ] `Event.offset` — posicao temporal absoluta (em beats)
- [ ] Eventos simultaneos sem ChordEvent
- [ ] Polirritmia: multiplas vozes com offsets independentes

### 6. Parsing alternativo de Duration

- [ ] `Duration("1/4")` — fracao
- [ ] `Duration("q")` — abreviacao
- [ ] `Duration("h.")` — abreviacao + ponto
- [ ] Notacao LilyPond: `"4."`, `"8"`, `"16"`

---

## Diferenciais do Gingo

- Campo harmonico completo com funcoes T/S/D
- Comparacao contextual com 21 dimensoes
- Arvore harmonica com caminhos e validacao
- Field.deduce() — inferencia a partir de dados parciais
- Fretboard com algoritmo CAGED para digitacoes realisticas
- FretboardSVG com orientacao e lateralidade (destro/canhoto)
- Piano com voicings (close, open, shell) e PianoSVG interativo
- MusicXML 4.0 para exportacao de partitura
- Zero deps + C++ backend
- Duration racional exata (fracao, nao float)
- Sintese de audio integrada com zero deps extras
