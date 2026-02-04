# Ritmo — Duration, Tempo, Sequence

O modulo de ritmo permite construir composicoes com tempo, compasso e
duracoes musicais reais. Cada evento (nota, acorde, pausa) tem sua
propria duracao, e a Sequence toca tudo na ordem correta.

---

## Duration — duracoes musicais

Uma `Duration` representa o valor ritmico de uma nota ou pausa.
Pode ser construida pelo nome ou pela fracao:

```python
from gingo import Duration

# Por nome
q = Duration("quarter")       # seminima (1/4)
h = Duration("half")          # minima (1/2)
w = Duration("whole")         # semibreve (1/1)
e = Duration("eighth")        # colcheia (1/8)
s = Duration("sixteenth")     # semicolcheia (1/16)

# Por fracao
d = Duration(3, 8)            # 3/8
```

### Nomes disponiveis

```python
print(Duration.standard_names())
# ['whole', 'half', 'quarter', 'eighth', 'sixteenth', 'thirty_second']
```

| Nome             | Fracao  | Nome em portugues |
|------------------|---------|-------------------|
| `whole`          | 1/1     | Semibreve         |
| `half`           | 1/2     | Minima            |
| `quarter`        | 1/4     | Seminima          |
| `eighth`         | 1/8     | Colcheia          |
| `sixteenth`      | 1/16    | Semicolcheia      |
| `thirty_second`  | 1/32    | Fusa              |

### Pontuacao e quialteras

```python
# Ponto de aumento (1.5x a duracao)
dq = Duration("quarter", dots=1)   # 3/8
dq.beats()                          # 1.5

# Tercina (3 notas no espaco de 2)
t = Duration("quarter", tuplet=3)
t.beats()                            # ~0.667
```

### Operacoes

```python
a = Duration("quarter")
b = Duration("eighth")

c = a + b           # 3/8
d = a * 2           # 1/2 (= half)

a.rational()         # (1, 4)
a.numerator()        # 1
a.denominator()      # 4
a.beats()            # 1.0 (em seminimas)
```

---

## Tempo — velocidade da musica

O `Tempo` define a velocidade em BPM (batidas por minuto) e converte
duracoes em segundos reais:

```python
from gingo import Tempo, Duration

# Por BPM
t = Tempo(120)
t.bpm()              # 120.0
t.marking()          # "Allegro"
t.ms_per_beat()      # 500.0 (ms por seminima)

# Por nome (marking)
t = Tempo("Adagio")
t.bpm()              # 70.0

# Converter duracao em segundos
t = Tempo(120)
t.seconds(Duration("quarter"))   # 0.5
t.seconds(Duration("half"))      # 1.0
t.seconds(Duration("eighth"))    # 0.25
```

### Markings padrao

| Marking       | BPM         | Carater                       |
|---------------|-------------|-------------------------------|
| `Largo`       | ~45         | Muito lento, solene           |
| `Adagio`      | ~70         | Lento, expressivo             |
| `Andante`     | ~90         | Caminhando                    |
| `Moderato`    | ~110        | Moderado                      |
| `Allegro`     | ~130        | Rapido, alegre                |
| `Vivace`      | ~155        | Vivo                          |
| `Presto`      | ~185        | Muito rapido                  |

```python
# Converter entre BPM e marking
Tempo.bpm_to_marking(120.0)   # "Allegro"
Tempo.marking_to_bpm("Adagio") # 70.0
```

---

## TimeSignature — compasso

O `TimeSignature` define quantas batidas por compasso e qual nota vale uma batida:

```python
from gingo import TimeSignature

ts = TimeSignature(4, 4)     # 4/4 (compasso quaternario)
ts.beats_per_bar()           # 4
ts.beat_unit()               # 4 (seminima)
ts.signature()               # (4, 4)
ts.classification()          # "simple quadruple"
ts.common_name()             # "Common time"

# Outros compassos
ts34 = TimeSignature(3, 4)   # 3/4 (valsa)
ts68 = TimeSignature(6, 8)   # 6/8 (composto binario)
```

---

## NoteEvent, ChordEvent, Rest — eventos

Eventos sao as unidades de uma `Sequence`. Cada evento tem uma duracao
e (para notas e acordes) uma oitava:

```python
from gingo import (
    Note, Chord, Duration,
    NoteEvent, ChordEvent, Rest,
)

# NoteEvent: nota + duracao + oitava
ne = NoteEvent(Note("C"), Duration("quarter"), 4)
ne.note()         # Note("C")
ne.octave()       # 4
ne.duration()     # Duration("quarter")
ne.frequency()    # 261.63 Hz
ne.midi_number()  # 60

# ChordEvent: acorde + duracao + oitava
ce = ChordEvent(Chord("Am"), Duration("half"), 4)
ce.chord()        # Chord("Am")
ce.octave()       # 4
ce.duration()     # Duration("half")
ce.note_events()  # lista de NoteEvents

# Rest: pausa
r = Rest(Duration("quarter"))
r.duration()      # Duration("quarter")
```

### Ouvir eventos individuais

```python
ne = NoteEvent(Note("E"), Duration("quarter"), 5)
ne.play()                    # toca na oitava e duracao do evento

ce = ChordEvent(Chord("Dm7"), Duration("half"), 4)
ce.play(waveform="triangle")
```

---

## Sequence — composicoes

Uma `Sequence` organiza eventos em ordem, com tempo e compasso:

```python
from gingo import (
    Note, Chord, Sequence, Tempo, TimeSignature,
    NoteEvent, ChordEvent, Rest, Duration,
)

# Criar sequencia
seq = Sequence(Tempo(120), TimeSignature(4, 4))

# Adicionar eventos
seq.add(NoteEvent(Note("C"), Duration("quarter"), 4))
seq.add(NoteEvent(Note("E"), Duration("quarter"), 4))
seq.add(NoteEvent(Note("G"), Duration("quarter"), 4))
seq.add(Rest(Duration("quarter")))
seq.add(ChordEvent(Chord("CM"), Duration("half"), 4))

# Informacoes
len(seq)              # 5 eventos
seq.total_seconds()   # duracao total em segundos
seq.bar_count()       # quantos compassos

# Acessar eventos
seq[0]                # NoteEvent (C)
seq[-1]               # ChordEvent (CM)

# Tocar e exportar
seq.play()
seq.to_wav("melodia.wav")
```

### Exemplo: melodia com acordes

```python
seq = Sequence(Tempo(100), TimeSignature(4, 4))

# Compasso 1: melodia
for note_name in ["C", "D", "E", "F"]:
    seq.add(NoteEvent(Note(note_name), Duration("quarter"), 4))

# Compasso 2: acorde + pausa
seq.add(ChordEvent(Chord("CM"), Duration("half"), 4))
seq.add(Rest(Duration("half")))

seq.play(waveform="triangle")
```

### Exemplo: II-V-I em semininimas

```python
seq = Sequence(Tempo(120), TimeSignature(4, 4))

seq.add(ChordEvent(Chord("Dm7"), Duration("quarter"), 4))
seq.add(ChordEvent(Chord("G7"), Duration("quarter"), 4))
seq.add(ChordEvent(Chord("C7M"), Duration("half"), 4))

seq.play(strum=0.04)
```

### Transposicao

```python
# Transpor toda a sequencia (notas e acordes)
seq_up = seq.transpose(5)    # sobe 5 semitons
seq_up.play()
```

### Manipulacao

```python
seq.remove(2)     # remove evento no indice 2
seq.clear()       # remove todos
seq.set_tempo(Tempo(140))
seq.set_time_signature(TimeSignature(3, 4))
```

---

## CLI

O modulo de ritmo nao tem subcomando CLI proprio — use a API Python.
Para **ouvir** objetos musicais pelo terminal, use `--play`:

```bash
gingo note C --play
gingo chord Am7 --play
gingo scale "C major" --play
gingo field "C major" --play
```
