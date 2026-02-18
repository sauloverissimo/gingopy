# Audio — ouvir o que voce codou

O Gingo sintetiza audio em tempo real a partir de qualquer objeto musical.
Zero dependencias externas para sintese e exportacao WAV.

!!! tip "Audio interativo"
    Esta pagina inclui players de audio — clique :material-play: para ouvir
    cada exemplo diretamente no navegador.

---

## Instalacao

A sintese funciona sem dependencias extras. Para **playback direto** (ouvir pelo
alto-falante), instale o pacote com extras de audio:

```bash
pip install gingo[audio]
```

Sem `simpleaudio`, o Gingo tenta usar players do sistema (`aplay`, `paplay`,
`ffplay` no Linux; `afplay` no macOS; player padrao no Windows).

---

## Play rapido

Qualquer objeto musical tem o metodo `.play()`:

```python
from gingo import Note, Chord, Scale, Field, Tree

Note("C").play()                   # nota unica
Chord("Am7").play()                # acorde (com strum)
Scale("C", "major").play()         # escala ascendente
Field("C", "major").play()         # 7 acordes do campo
Tree("C", "major").play()          # acordes diatonicos
```

Funcao standalone equivalente:

```python
from gingo.audio import play

play(Note("A"))
play(Chord("G7"), waveform="triangle")
play(["CM", "Am", "FM", "GM"])     # lista de nomes de acordes
```

| Objeto | Audio |
|:---|:---:|
| `Note("C")` | <audio controls preload="none"><source src="../../assets/audio/play_note_c.mp3" type="audio/mpeg"></audio> |
| `Chord("Am7")` | <audio controls preload="none"><source src="../../assets/audio/play_chord_am7.mp3" type="audio/mpeg"></audio> |
| `Scale("C", "major")` | <audio controls preload="none"><source src="../../assets/audio/play_scale_cmaj.mp3" type="audio/mpeg"></audio> |
| `Field("C", "major")` | <audio controls preload="none"><source src="../../assets/audio/play_field_cmaj.mp3" type="audio/mpeg"></audio> |
| `["CM", "Am", "FM", "GM"]` | <audio controls preload="none"><source src="../../assets/audio/play_list_pop.mp3" type="audio/mpeg"></audio> |

---

## Parametros

Todos os parametros sao keyword-only, com defaults musicalmente uteis:

| Parametro     | Tipo                 | Default      | Descricao                                      |
|---------------|----------------------|--------------|-------------------------------------------------|
| `octave`      | `int`                | `4`          | Oitava base (4 = oitava do Do central)          |
| `duration`    | `float`              | `0.5`        | Segundos por nota/acorde                        |
| `waveform`    | `str` ou `Waveform`  | `"sine"`     | Forma de onda: sine, square, sawtooth, triangle |
| `amplitude`   | `float`              | `0.8`        | Volume (0.0 a 1.0)                              |
| `envelope`    | `Envelope` ou `None` | `None`       | Envelope ADSR (default: attack=0.01, decay=0.08, sustain=0.6, release=0.2) |
| `strum`       | `float`              | `0.03`       | Delay entre notas de um acorde (segundos)       |
| `gap`         | `float`              | `0.05`       | Silencio entre notas/acordes consecutivos       |
| `tuning`      | `float`              | `440.0`      | Referencia A4 em Hz                              |
| `sample_rate` | `int`                | `44100`      | Amostras por segundo                             |

### Exemplos com parametros

```python
# Acorde com strum lento (efeito arpejado)
Chord("Am7").play(strum=0.08, waveform="triangle")

# Escala rapida, sem gap entre notas (legato)
Scale("C", "major").play(duration=0.2, gap=0.0)

# Campo harmonico com som de onda quadrada
Field("C", "major").play(waveform="square", duration=0.8)

# Oitava mais aguda, afinacao barroca
Note("A").play(octave=5, tuning=415.0)
```

---

## Formas de onda

Quatro formas de onda disponiveis, cada uma com timbre distinto:

| Waveform    | Timbre                        | Uso tipico                          |
|-------------|-------------------------------|-------------------------------------|
| `sine`      | Puro, suave, sem harmonicos   | Tom de referencia, melodias simples |
| `square`    | Rico, cheio, tipo 8-bit       | Jogos retro, tons marcantes         |
| `sawtooth`  | Brilhante, agressivo          | Sintetizadores, leads               |
| `triangle`  | Suave, levemente oco          | Tons gentis, acompanhamentos        |

```python
from gingo import Note

# Compare as quatro formas de onda
for wf in ["sine", "square", "sawtooth", "triangle"]:
    print(f"Tocando {wf}...")
    Note("A").play(waveform=wf, duration=0.8)
```

Compare os timbres da nota La (A4):

| Waveform | Timbre | Audio |
|:---|:---|:---:|
| `sine` | Puro, suave | <audio controls preload="none"><source src="../../assets/audio/waveform_sine.mp3" type="audio/mpeg"></audio> |
| `square` | Rico, cheio | <audio controls preload="none"><source src="../../assets/audio/waveform_square.mp3" type="audio/mpeg"></audio> |
| `sawtooth` | Brilhante | <audio controls preload="none"><source src="../../assets/audio/waveform_sawtooth.mp3" type="audio/mpeg"></audio> |
| `triangle` | Suave, oco | <audio controls preload="none"><source src="../../assets/audio/waveform_triangle.mp3" type="audio/mpeg"></audio> |

---

## Envelope ADSR

O envelope controla como o volume de cada nota evolui no tempo:

```
Amplitude
  1.0 |    /\
      |   /  \___________
  0.6 |  /    decay      \  sustain
      | /                 \
  0.0 |/___________________\________
      A    D       S          R
      attack  decay  sustain  release
```

| Fase       | Default  | O que controla                                |
|------------|----------|-----------------------------------------------|
| `attack`   | `0.01 s` | Tempo de subida (0 = ataque imediato)         |
| `decay`    | `0.08 s` | Tempo de descida do pico ate o sustain        |
| `sustain`  | `0.6`    | Nivel de amplitude enquanto a nota soa (0-1)  |
| `release`  | `0.2 s`  | Tempo de descida quando a nota termina        |

```python
from gingo.audio import Envelope, play
from gingo import Chord

# Envelope com ataque lento (efeito pad/strings)
pad = Envelope(attack=0.3, decay=0.1, sustain=0.8, release=0.5)
Chord("Am7").play(envelope=pad, duration=2.0)

# Envelope percussivo (piano/marimba)
perc = Envelope(attack=0.005, decay=0.2, sustain=0.2, release=0.1)
Note("C").play(envelope=perc, duration=0.8)
```

| Envelope | Audio |
|:---|:---:|
| Pad (attack lento) | <audio controls preload="none"><source src="../../assets/audio/adsr_pad.mp3" type="audio/mpeg"></audio> |
| Percussivo (ataque rapido) | <audio controls preload="none"><source src="../../assets/audio/adsr_perc.mp3" type="audio/mpeg"></audio> |

---

## Strum — o dedilhar do acorde

O parametro `strum` adiciona um pequeno delay entre cada nota de um acorde,
simulando o efeito de dedilhar (como no violao). Sem strum, todas as notas
soam simultaneamente — o que pode parecer artificial.

```python
from gingo import Chord

# Sem strum — todas as notas ao mesmo tempo
Chord("Am7").play(strum=0.0)

# Strum default (0.03s) — leve separacao
Chord("Am7").play()

# Strum mais lento — efeito arpejado
Chord("Am7").play(strum=0.08)

# Strum bem lento — quase um arpejo
Chord("Am7").play(strum=0.15)
```

Compare o efeito de strum em Am7:

| Strum | Efeito | Audio |
|:---|:---|:---:|
| `0.0` | Simultaneo | <audio controls preload="none"><source src="../../assets/audio/strum_0.mp3" type="audio/mpeg"></audio> |
| `0.03` | Default | <audio controls preload="none"><source src="../../assets/audio/strum_003.mp3" type="audio/mpeg"></audio> |
| `0.08` | Arpejado | <audio controls preload="none"><source src="../../assets/audio/strum_008.mp3" type="audio/mpeg"></audio> |
| `0.15` | Lento | <audio controls preload="none"><source src="../../assets/audio/strum_015.mp3" type="audio/mpeg"></audio> |

O strum afeta apenas acordes (Chord, ChordEvent, e acordes em Field/Tree).
Notas individuais e escalas nao sao afetados.

---

## Gap — respiracao entre notas

O parametro `gap` insere um curto silencio entre notas/acordes consecutivos.
Sem gap, os sons se encostam (legato completo). Com gap, cada som tem espaco
para respirar — como levantar o dedo entre as teclas.

```python
from gingo import Scale, Field

# Escala com gap default (0.05s) — articulacao natural
Scale("C", "major").play()

# Sem gap — legato (notas conectadas)
Scale("C", "major").play(gap=0.0)

# Gap grande — staccato (notas bem separadas)
Scale("C", "major").play(gap=0.15)

# Campo harmonico com gap entre acordes
Field("C", "major").play(gap=0.1, duration=0.8)
```

Compare o efeito de gap na escala de C major:

| Gap | Articulacao | Audio |
|:---|:---|:---:|
| `0.0` | Legato | <audio controls preload="none"><source src="../../assets/audio/gap_0.mp3" type="audio/mpeg"></audio> |
| `0.05` | Natural | <audio controls preload="none"><source src="../../assets/audio/gap_005.mp3" type="audio/mpeg"></audio> |
| `0.15` | Staccato | <audio controls preload="none"><source src="../../assets/audio/gap_015.mp3" type="audio/mpeg"></audio> |

---

## Exportar WAV

Exporte qualquer objeto para arquivo WAV (16-bit mono):

```python
from gingo import Note, Chord, Scale, Field
from gingo.audio import to_wav

# Nota
Note("A").to_wav("la.wav")

# Acorde
Chord("Am7").to_wav("am7.wav", waveform="triangle")

# Escala completa
Scale("C", "major").to_wav("c_major.wav", duration=0.3)

# Campo harmonico
Field("C", "major").to_wav("campo_c.wav", duration=0.8)

# Funcao standalone
to_wav(Chord("G7"), "g7.wav")
```

Os mesmos parametros de `play()` funcionam em `to_wav()`.

---

## CLI — --play e --wav

No terminal, use as flags `--play` e `--wav` em qualquer subcomando:

```bash
# Ouvir
gingo note C --play
gingo chord Am7 --play
gingo scale "C major" --play
gingo field "C major" --play

# Exportar WAV
gingo note A --wav la.wav
gingo chord Am7 --wav am7.wav

# Combinar opcoes
gingo chord G7 --play --waveform triangle --strum 0.06
gingo scale "A minor" --play --gap 0.1
gingo field "C major" --play --wav campo.wav
```

### Opcoes de audio no CLI

| Flag          | Descricao                                       | Default      |
|---------------|--------------------------------------------------|--------------|
| `--play`      | Toca pelo audio do sistema                       | —            |
| `--wav FILE`  | Exporta para arquivo WAV                         | —            |
| `--waveform`  | sine, square, sawtooth, triangle                 | `sine`       |
| `--strum SEC` | Delay entre notas do acorde (segundos)           | `0.03`       |
| `--gap SEC`   | Silencio entre notas/acordes consecutivos        | `0.05`       |

---

## Comportamento por tipo de objeto

Cada classe tem um comportamento especifico ao ser tocada:

| Objeto         | Comportamento                                                |
|----------------|--------------------------------------------------------------|
| `Note`         | Nota unica na oitava e duracao especificadas                 |
| `Chord`        | Notas simultaneas com strum entre elas                       |
| `Scale`        | Notas em sequencia ascendente + tonica uma oitava acima      |
| `Field`        | 7 triades do campo em sequencia                              |
| `Tree`         | Acordes diatonicos (via Field correspondente) **(beta)**     |
| `NoteEvent`    | Nota com oitava e duracao proprias                           |
| `ChordEvent`   | Acorde com oitava e duracao proprias                         |
| `Rest`         | Silencio pela duracao especificada                           |
| `Sequence`     | Eventos na ordem, usando tempo e duracoes proprios           |
| `list[str]`    | Lista de nomes de acordes/notas em sequencia                 |

### Exemplo: cada tipo

```python
from gingo import (
    Note, Chord, Scale, Field, Tree,
    NoteEvent, ChordEvent, Rest,
    Duration, Tempo, TimeSignature, Sequence,
)

# Nota: Do central por 0.5s
Note("C").play()

# Acorde: Am7 com todas as notas + strum
Chord("Am7").play()

# Escala: C major ascendente (C D E F G A B C)
Scale("C", "major").play(duration=0.3)

# Campo harmonico: CM Dm Em FM GM Am Bdim
Field("C", "major").play(duration=0.6)

# Arvore: mesmos acordes que o campo
Tree("C", "major").play()

# NoteEvent: nota com oitava e duracao especificas
ne = NoteEvent(Note("E"), Duration("quarter"), 5)
ne.play()

# ChordEvent: acorde com oitava e duracao especificas
ce = ChordEvent(Chord("Dm"), Duration("half"), 4)
ce.play()

# Sequence: composicao com tempo
seq = Sequence(Tempo(120), TimeSignature(4, 4))
seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))
seq.add(NoteEvent(Note("E"), Duration("quarter"), 4))
seq.add(NoteEvent(Note("G"), Duration("quarter"), 4))
seq.add(ChordEvent(Chord("CM"), Duration("half"), 4))
seq.play()

# Lista de acordes
from gingo.audio import play
play(["CM", "Am", "FM", "GM"], duration=0.6)
```

---

## Sequence — composicoes com tempo

Uma `Sequence` respeita o tempo (BPM) e as duracoes de cada evento. O parametro
`duration` de `play()` e ignorado — a Sequence usa seus proprios valores.

```python
from gingo import (
    Note, Chord, Sequence, Tempo, TimeSignature,
    NoteEvent, ChordEvent, Rest, Duration,
)

# Melodia simples a 100 BPM
seq = Sequence(Tempo(100), TimeSignature(4, 4))

# Frase: C - E - G - (pausa) - CM
seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))
seq.add(NoteEvent(Note("E"), Duration("quarter"), 4))
seq.add(NoteEvent(Note("G"), Duration("quarter"), 4))
seq.add(Rest(Duration("quarter")))
seq.add(ChordEvent(Chord("CM"), Duration("half"), 4))

seq.play(waveform="triangle")
seq.to_wav("melodia.wav")
```

<audio controls preload="none"><source src="../../assets/audio/seq_melody.mp3" type="audio/mpeg"></audio>

---

## Exemplos praticos

### Progressao I-V-vi-IV (pop)

```python
from gingo.audio import play
play(["CM", "GM", "Am", "FM"], duration=0.8, waveform="triangle")
```

<audio controls preload="none"><source src="../../assets/audio/jazz_pop.mp3" type="audio/mpeg"></audio>

### Progressao ii-V-I (jazz)

```python
from gingo.audio import play
play(["Dm7", "G7", "C7M"], duration=1.0, strum=0.05)
```

<audio controls preload="none"><source src="../../assets/audio/jazz_251.mp3" type="audio/mpeg"></audio>

### Modos gregos — compare o som

```python
from gingo import Scale

modos = ["ionian", "dorian", "phrygian", "lydian",
         "mixolydian", "aeolian", "locrian"]

for modo in modos:
    print(f"Tocando {modo}...")
    Scale("C", modo).play(duration=0.25, gap=0.03)
```

| Modo | Carater | Audio |
|:---|:---|:---:|
| Ionian | Maior, brilhante | <audio controls preload="none"><source src="../../assets/audio/mode_ionian.mp3" type="audio/mpeg"></audio> |
| Dorian | Menor, jazzy | <audio controls preload="none"><source src="../../assets/audio/mode_dorian.mp3" type="audio/mpeg"></audio> |
| Phrygian | Menor, flamenca | <audio controls preload="none"><source src="../../assets/audio/mode_phrygian.mp3" type="audio/mpeg"></audio> |
| Lydian | Maior, eterea | <audio controls preload="none"><source src="../../assets/audio/mode_lydian.mp3" type="audio/mpeg"></audio> |
| Mixolydian | Maior, blues | <audio controls preload="none"><source src="../../assets/audio/mode_mixolydian.mp3" type="audio/mpeg"></audio> |
| Aeolian | Menor natural | <audio controls preload="none"><source src="../../assets/audio/mode_aeolian.mp3" type="audio/mpeg"></audio> |
| Locrian | Diminuta, tensa | <audio controls preload="none"><source src="../../assets/audio/mode_locrian.mp3" type="audio/mpeg"></audio> |

### Comparar campos maior e menor

```python
from gingo import Field

print("Campo maior:")
Field("C", "major").play(duration=0.6)

print("Campo menor:")
Field("A", "natural minor").play(duration=0.6)
```

| Campo | Audio |
|:---|:---:|
| C major | <audio controls preload="none"><source src="../../assets/audio/field_c_major.mp3" type="audio/mpeg"></audio> |
| A minor | <audio controls preload="none"><source src="../../assets/audio/field_a_minor.mp3" type="audio/mpeg"></audio> |

### Familias de escalas

```python
from gingo import Scale

familias = [
    "major", "natural minor", "harmonic minor",
    "melodic minor", "blues", "whole tone",
]

for f in familias:
    print(f"Tocando {f}...")
    Scale("C", f).play(duration=0.25)
```

| Familia | Audio |
|:---|:---:|
| Major | <audio controls preload="none"><source src="../../assets/audio/family_major.mp3" type="audio/mpeg"></audio> |
| Natural minor | <audio controls preload="none"><source src="../../assets/audio/family_natural_minor.mp3" type="audio/mpeg"></audio> |
| Harmonic minor | <audio controls preload="none"><source src="../../assets/audio/family_harmonic_minor.mp3" type="audio/mpeg"></audio> |
| Melodic minor | <audio controls preload="none"><source src="../../assets/audio/family_melodic_minor.mp3" type="audio/mpeg"></audio> |
| Blues | <audio controls preload="none"><source src="../../assets/audio/family_blues.mp3" type="audio/mpeg"></audio> |
| Whole tone | <audio controls preload="none"><source src="../../assets/audio/family_whole_tone.mp3" type="audio/mpeg"></audio> |

### Exportar todos os acordes do campo

```python
from gingo import Field

campo = Field("C", "major")
for i, ch in enumerate(campo.chords()):
    ch.to_wav(f"grau_{i+1}_{ch.name()}.wav", duration=1.0)
    print(f"Exportado: grau_{i+1}_{ch.name()}.wav")
```

---

## Referencia tecnica

### Waveform (enum)

```python
from gingo.audio import Waveform

Waveform.SINE       # "sine"
Waveform.SQUARE     # "square"
Waveform.SAWTOOTH   # "sawtooth"
Waveform.TRIANGLE   # "triangle"
```

### Envelope (class)

```python
from gingo.audio import Envelope

env = Envelope(
    attack=0.01,   # segundos
    decay=0.08,    # segundos
    sustain=0.6,   # amplitude (0-1)
    release=0.2,   # segundos
)

# Calcular amplitude em t=0.5 para nota de 1 segundo
amp = env.amplitude(0.5, 1.0)  # float
```

### play() e to_wav()

```python
from gingo.audio import play, to_wav

# play(obj, *, octave=4, duration=0.5, waveform="sine",
#      amplitude=0.8, envelope=None, strum=0.03, gap=0.05,
#      tuning=440.0, sample_rate=44100) -> None

# to_wav(obj, path, *, ...) -> None
# Mesmos parametros de play()
```
