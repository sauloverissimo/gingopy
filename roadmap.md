# Roadmap

Funcionalidades planejadas e estado atual do projeto.

---

## Implementado (v1.0.0)

### Core (C++17)

- Note, Interval, Chord (42 formulas), Scale (10 familias, 36 modos)
- Field: triades, setimas, funcoes T/S/D, acordes aplicados
- Field.identify() e Field.deduce() — inferencia de tonalidade
- Comparacao neo-Riemannian (P/L/R, 2-step, voice leading, 18 dimensoes)
- Comparacao contextual (21 dimensoes: borrowing, pivot, tritone sub, etc.)
- Tree: arvore harmonica, caminhos, validacao de progressoes, DOT/Mermaid

### Ritmo (C++17)

- Duration (fracao racional exata, nomes, pontuacao, tercinas)
- Tempo (BPM, marcacoes italianas, conversao para segundos)
- TimeSignature (formula de compasso)
- NoteEvent, ChordEvent, Rest
- Sequence (composicao com tempo, __getitem__, transpose)

### Audio (Python)

- Sintese: sine, square, sawtooth, triangle
- Envelope ADSR (attack, decay, sustain, release)
- Strum (delay entre notas de acordes) e gap (silencio entre eventos)
- `.play()` e `.to_wav()` em todas as classes (Note, Chord, Scale, Field, Tree, Sequence)
- Funcoes standalone: `play()`, `to_wav()`
- Playback: simpleaudio (opcional) com fallback para players do sistema
- CLI: `--play`, `--wav`, `--waveform`, `--strum`, `--gap`

### Infra

- Zero dependencias runtime (simpleaudio opcional para playback)
- Core C++17 com pybind11
- CLI completa com subcomandos (note, interval, chord, scale, field, compare)
- Type stubs (PEP 561)
- Documentacao MkDocs completa

---

## Planejado

### 1. Visualizacao de instrumentos (`gingo[visual]`)

**Fase 1 — Piano**

- Piano ASCII para CLI (1-2 oitavas, notas destacadas)
- `.piano()` em Note, Chord, Scale — retorna string ASCII art
- CLI: `gingo chord Am7 --piano`, `gingo scale "C major" --piano`
- Suporte a destacar notas ativas, root, intervalos

**Fase 2 — Cordas**

- Guitar (6 cordas, diagrama de braco com trastes)
- Bass (4 cordas)
- Ukulele (4 cordas)
- Afinacao padrao configuravel, multiplas posicoes
- Interface unificada: `.guitar()`, `.bass()`, `.ukulele()`

**Fase 3 — Extensibilidade**

- Instrument base class (tuning, strings/keys, fret/key range)
- Instrumentos customizados: `Instrument("banjo", tuning=["G","D","G","B","D"])`
- Renderizacao: ASCII (CLI), SVG (notebooks), Unicode (terminais modernos)

### 2. MIDI (`gingo[audio]`)

- `Duration.midi_ticks(ppqn=480)` / `Duration.from_ticks(ticks, ppqn)`
- `Tempo.microseconds_per_beat()` / `Tempo.from_microseconds(usec)`
- `bpm2tempo(bpm)` / `tempo2bpm(usec)` — conversao MIDI
- `Sequence.to_midi()` / `Sequence.from_midi(path)` — import/export .mid

### 3. Partitura (`gingo[score]`)

- `Sequence.to_musicxml()` — export MusicXML
- `Sequence.to_lilypond()` — export LilyPond
- `Sequence.show()` — renderiza via MuseScore ou LilyPond
- Suporte a claves, dinamicas, articulacoes

### 4. Escalas futuras

- Bebop (6 tipos, 8 notas com passing tone cromatico)
- Aliases de modos simetricos existentes
- Ver `.old/scales_analysis.md` secao 14.9

### 5. Estrutura hierarquica

- `Bar(events, time_signature)` — compasso completo
- `Phrase(bars)` — frase musical
- `Part(phrases, instrument)` — uma voz/instrumento
- `Score(parts, tempo)` — partitura com multiplas vozes

### 6. Offset absoluto

- `Event.offset` — posicao temporal absoluta (em beats)
- Eventos simultaneos sem ChordEvent
- Polirritmia: multiplas vozes com offsets independentes

### 7. Parsing alternativo de Duration

- `Duration("1/4")` — fracao
- `Duration("q")` — abreviacao
- `Duration("h.")` — abreviacao + ponto
- Notacao LilyPond: `"4."`, `"8"`, `"16"`

---

## Comparativo com bibliotecas de referencia

### Arquitetura

| Aspecto            | gingo            | music21          | abjad            | mingus           |
|--------------------|------------------|------------------|------------------|------------------|
| Deps runtime       | 0                | 8 (numpy, etc.)  | 3 + LilyPond     | 1 (six)          |
| Backend            | C++17 + pybind11 | Python puro      | Python puro      | Python puro      |
| Manutencao         | Ativa            | Ativa            | Ativa            | Abandonada (2020)|

### Funcionalidades

| Funcionalidade                         | gingo | music21 | abjad | mingus |
|----------------------------------------|-------|---------|-------|--------|
| Notas, intervalos                      | sim   | sim     | sim   | sim    |
| Acordes (formulas, identify)           | 42    | sim     | nao   | parcial|
| Escalas (familias, modos)              | 36    | sim     | nao   | parcial|
| Campo harmonico com funcoes T/S/D      | sim   | parcial | nao   | nao    |
| Acordes aplicados (V7/II)              | sim   | nao     | nao   | nao    |
| Comparacao neo-Riemannian              | sim   | parcial | nao   | nao    |
| Comparacao contextual (21 dim)         | sim   | nao     | nao   | nao    |
| Arvore harmonica (progressoes)         | sim   | nao     | nao   | nao    |
| Inferencia de tonalidade (deduce)      | sim   | nao     | nao   | nao    |
| Duration racional exata                | sim   | nao (float) | sim | nao    |
| Sintese de audio / playback            | sim   | sim (MIDI) | nao | sim (MIDI) |
| WAV export                             | sim   | nao     | nao   | nao    |
| Partitura (MusicXML, LilyPond)        | nao   | sim     | sim   | nao    |
| MIDI read/write                        | nao   | sim     | nao   | sim    |
| Corpus de obras                        | nao   | sim     | nao   | nao    |
| Frequency com tuning parametrizavel    | sim   | nao     | nao   | sim    |

### Diferenciais exclusivos do gingo

- Campo harmonico completo com funcoes T/S/D
- Comparacao contextual com 21 dimensoes
- Arvore harmonica com caminhos e validacao
- Field.deduce() — inferencia a partir de dados parciais
- Zero deps + C++ backend
- Duration racional exata (fracao, nao float)
- Sintese de audio integrada com zero deps extras
