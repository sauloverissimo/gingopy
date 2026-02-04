# Bem-vindo ao Gingo

Biblioteca Python de teoria musical para estudo, analise e desenvolvimento.

---

## O que e o Gingo?

O Gingo e uma **biblioteca Python** que modela a harmonia ocidental em camadas.
Voce pode usar em scripts, notebooks, aplicacoes — ou explorar pelo terminal com o CLI.

Pense na musica como uma escada de **12 degraus** (as 12 notas). Tudo que
existe na harmonia — acordes, escalas, campos, progressoes — e uma forma
de **selecionar e organizar** esses 12 degraus.

O Gingo organiza isso em 5 camadas, uma construida sobre a outra:

```
Note         "O que e essa nota?"
  |
Interval     "Qual a distancia entre duas notas?"
  |
Chord        "Que notas tocadas juntas formam isso?"
  |
Scale        "Que notas posso usar nessa tonalidade?"
  |
Field        "Que acordes essa escala gera?"
```

Nao da pra entender campos sem entender acordes. Nao da pra entender
acordes sem entender intervalos. Cada camada depende da anterior.

---

## Duas formas de usar

### Como biblioteca Python (uso principal)

```python
from gingo import Note, Interval, Chord, Scale, Field

note = Note("C#")
chord = Chord("Am7")
scale = Scale("C", "major")
field = Field("C", "major")
```

Use em scripts, notebooks Jupyter, aplicacoes web, ferramentas de estudo,
geradores de exercicios — qualquer coisa que voce consiga construir com Python.

### Como CLI (exploracao e teste)

```
gingo note C#
gingo chord Am7
gingo scale "C major" --modes
gingo field "C major"
```

Para ver a ajuda: `gingo --help`, `gingo chord --help`, etc.

---

## 1. Note — o atomo da musica

Uma nota e um som com uma frequencia. Existem 12 notas na musica ocidental,
chamadas de **pitch classes cromaticos**:

```
C    C#    D    D#    E    F    F#    G    G#    A    A#    B
Do   Do#   Re   Re#   Mi   Fa   Fa#  Sol  Sol#  La   La#   Si
```

Depois do B (Si), volta para o C (Do) — so que uma oitava acima.
Esse ciclo de 12 se repete infinitamente.

### Enarmonia: dois nomes, mesmo som

Algumas notas tem dois nomes. Bb (Si bemol) e A# (La sustenido)
sao a **mesma tecla do piano**, o **mesmo som**. Isso se chama **enarmonia**.

Notacoes aceitas:
- Sustenidos: `C#`, `D#`, `F#`, `G#`, `A#`
- Bemois: `Db`, `Eb`, `Gb`, `Ab`, `Bb`
- Duplos: `Ebb`, `F##`
- Unicode: `B♭`, `F♯`

### Python

```python
from gingo import Note

n = Note("Bb")
n.name()           # "Bb"
n.natural()         # "A#"  (nome canonico, com sustenido)
n.semitone()        # 10
n.frequency()       # 466.16 (Hz, referencia A4=440)
n.sound()           # "A"

# Transposicao
n.transpose(7)      # Note("F")  — subir 7 semitons

# Enarmonia
n.is_enharmonic(Note("A#"))  # True — mesmo som, nomes diferentes

# Ouvir a nota
n.play()                     # toca Bb na oitava 4
n.play(octave=5)             # uma oitava acima
```

### CLI

```
$ gingo note Bb

  Note: Bb
  ————————
    Semitone index...... 10
    Frequency (A4=440).. 466.16 Hz
    Natural name........ A#
    Sound............... A
```

| Opcao               | O que faz                                |
|----------------------|------------------------------------------|
| `--transpose N`      | Sobe ou desce N semitons                 |
| `--enharmonic NOTA`  | Verifica se duas notas sao enarmonicas   |
| `--distance NOTA`    | Distancia no ciclo de quintas (0-6)      |
| `--fifths`           | Mostra a posicao no circulo de quintas   |
| `--play`             | Toca a nota pelo audio do sistema        |
| `--wav FILE`         | Exporta para arquivo WAV                 |

```
gingo note C --transpose 7           # C + 7 semitons = G
gingo note Db --enharmonic C#        # sim, sao enarmonicos
gingo note C --distance G            # distancia no ciclo de quintas
gingo note G --fifths                # posicao no circulo de quintas
gingo note A --play                  # ouvir a nota La
gingo note C --play --waveform square  # onda quadrada
```

**O que da pra estudar:** o sistema cromatico de 12 notas, enarmonia,
frequencias, transposicao.

---

## 2. Interval — a distancia entre duas notas

Um intervalo mede a **distancia** entre duas notas, contada em **semitons**
(a menor distancia possivel entre duas notas). O Gingo cobre
**24 intervalos** (2 oitavas completas, de 0 a 23 semitons).

### Entendendo os nomes

| Semitons | Label | Anglo-saxao | Nome em portugues       |
|:--------:|:-----:|:-----------:|-------------------------|
| 0        | P1    | P1          | Unissono (mesma nota)   |
| 1        | 2m    | mi2         | Segunda menor           |
| 2        | 2M    | ma2         | Segunda maior           |
| 3        | 3m    | mi3         | Terca menor             |
| 4        | 3M    | ma3         | Terca maior             |
| 5        | 4J    | P4          | Quarta justa            |
| 6        | d5    | d5          | Quinta diminuta (tritono)|
| 7        | 5J    | P5          | Quinta justa            |
| 8        | #5    | mi6         | Sexta menor             |
| 9        | M6    | ma6         | Sexta maior             |
| 10       | 7m    | mi7         | Setima menor            |
| 11       | 7M    | ma7         | Setima maior            |
| 12       | 8J    | P8          | Oitava                  |

A partir do semitom 13, os intervalos se repetem na segunda oitava
(b9, 9, #9, 11, #11, 13, etc.) — usados em acordes estendidos no jazz.

### Python

```python
from gingo import Interval

iv = Interval("5J")        # por label
iv = Interval(7)           # por semitons (mesmo resultado)

iv.label()          # "5J"
iv.anglo_saxon()    # "P5"
iv.semitones()      # 7
iv.degree()         # 5
iv.octave()         # 1

# Listar todos os 24 intervalos
for i in range(24):
    iv = Interval(i)
    print(f"{iv.semitones():2d} st  {iv.label():<6s}  {iv.anglo_saxon()}")
```

### CLI

```
$ gingo interval 5J

  Interval: 5J
  —————————————
    Semitones........... 7
    Anglo-saxon......... P5
    Degree.............. 5
    Octave.............. 1
```

| Opcao   | O que faz                                  |
|---------|--------------------------------------------|
| `--all` | Mostra a tabela completa dos 24 intervalos |

```
gingo interval 4             # 4 semitons = 3M (terca maior)
gingo interval 0 --all       # tabela completa dos 24
```

**O que da pra estudar:** como medir distancias entre notas, a diferenca
entre maior/menor/justo/diminuto/aumentado, o sistema de 2 oitavas.

---

## 3. Chord — notas empilhadas por uma formula

Um acorde e uma **receita**: uma nota raiz + um conjunto de intervalos.
O Gingo conhece **42 formulas de acordes**.

### As 42 formulas, por familia

| Familia      | Tipos disponiveis                                            |
|--------------|--------------------------------------------------------------|
| Triades      | `M`  `m`  `dim`  `aug`  `sus4`  `sus2`  `5`                 |
| Setimas      | `7`  `7M`  `m7`  `m7M`  `dim7`  `m7(b5)`                    |
| Sextas       | `6`  `m6`                                                    |
| Estendidos   | `9`  `7M(9)`  `m9`  `6(9)`  `m6(9)`                         |
| Alterados    | `7(b5)` `aug7` `7(#9)` `7(b9)` `7(#5)` `7(b5b9)` `7(#5#9)` |
|              | `m7(9)` `7M(#5)`                                            |
| Suspensos    | `7sus4`  `7sus2`                                             |
| Add          | `add9`  `madd9`  `add11`  `add13`                            |
| Outros       | `dim7M`  `m7(11)`  `7(13)`  `7M(13)` ...                    |

Para formar um acorde, combine: **nota raiz** + **tipo**. Exemplos:
`CM` (Do maior), `Am7` (La menor com setima), `F#dim7` (Fa# diminuto com setima).

### Python

```python
from gingo import Chord

c = Chord("Am7")
c.name()            # "Am7"
c.root()            # Note("A")
c.type()            # "m7"
c.notes()           # [Note("A"), Note("C"), Note("E"), Note("G")]
c.formal_notes()    # [Note("A"), Note("C"), Note("E"), Note("G")]
c.intervals()       # [Interval("P1"), Interval("3m"), Interval("5J"), Interval("7m")]
c.interval_labels() # ["P1", "3m", "5J", "7m"]
c.size()            # 4
c.contains(Note("C"))  # True

# Construcao por lista de notas
Chord(["C", "E", "G"])                 # Chord("CM")
Chord(["A", "C", "E", "G"])            # Chord("Am7")

# Identificacao reversa: dadas as notas, descobre o acorde
Chord.identify(["C", "E", "G", "B"])   # Chord("C7M")
Chord.identify(["D", "F", "A"])        # Chord("Dm")

# Ouvir o acorde
c.play()                               # Am7 com strum suave
c.play(waveform="triangle")            # timbre diferente
c.play(strum=0.08)                     # strum mais lento (arpejado)
c.to_wav("am7.wav")                    # exportar WAV
```

A identificacao reversa e poderosa para estudo: toque notas no instrumento,
passe para o Gingo, e descubra o nome do acorde.

### CLI

```
$ gingo chord Am7

  Chord: Am7
  ——————————
    Root................ A
    Type................ m7
    Notes............... A, C, E, G
    Formal notes........ A, C, E, G
    Intervals........... P1, 3m, 5J, 7m
    Size................ 4
```

| Opcao        | O que faz                                          |
|--------------|----------------------------------------------------|
| `--identify` | Identifica acorde a partir de notas (separar com virgula) |
| `--play`     | Toca o acorde pelo audio do sistema                |
| `--wav FILE` | Exporta para arquivo WAV                           |
| `--strum S`  | Delay entre notas do acorde em segundos (default 0.03) |

```
gingo chord Bbdim7                    # Sib diminuto com setima
gingo chord "F#7M(#5)"               # Fa# maior 7 com quinta aumentada
gingo chord "C,E,G,B" --identify     # descobre = C7M
gingo chord "D,F,A" --identify       # descobre = Dm
gingo chord Am7 --play               # ouvir o acorde
gingo chord Am7 --play --strum 0.08  # strum lento
gingo chord Am7 --wav am7.wav        # exportar WAV
```

**O que da pra estudar:** formacao de acordes (triades, tetrades, estendidos),
como um acorde e definido pela formula de intervalos, identificacao reversa.

---

## 4. Scale — uma selecao de notas

Uma escala e uma **selecao de notas** das 12 disponiveis, seguindo um padrao
de intervalos. E como um filtro: das 12 notas cromaticas, a escala "acende"
algumas e "apaga" outras.

O modelo do Gingo organiza escalas em 3 niveis: **Familia → Modo → Filtro**.

### As 10 familias parentais

| Familia            | Aliases aceitos               | Notas | Descricao                         |
|--------------------|-------------------------------|:-----:|-----------------------------------|
| `major`            | `maj`                         | 7     | A referencia. Tom alegre, estavel |
| `natural minor`    | `m`                           | 7     | Tom melancolico, introspectivo    |
| `harmonic minor`   | `minor`                       | 7     | Dramatica, sonoridade arabe       |
| `melodic minor`    |                               | 7     | Sofisticada, muito usada no jazz  |
| `harmonic major`   |                               | 7     | Maior com 6a menor, dramatica    |
| `diminished`       | `dim`                         | 8     | Tensa, simetrica (octatonica)    |
| `whole tone`       | `wholetone`                   | 6     | Simetrica, impressionista        |
| `augmented`        | `aug`                         | 6     | Simetrica, sonoridade Coltrane   |
| `blues`            |                               | 6     | Pentatonica menor + blue note    |
| `chromatic`        |                               | 12    | Todas as 12 notas                |

### Nomes de modo como tipo

Cada familia de 7 notas gera modos. Voce pode construir uma escala
diretamente pelo nome do modo:

```python
Scale("D", "dorian")             # D Dorian (modo 2 de Major)
Scale("E", "phrygian dominant")  # modo 5 de HarmonicMinor
Scale("C", "altered")            # modo 7 de MelodicMinor
Scale("F", "lydian")             # modo 4 de Major
```

### Python

```python
from gingo import Scale, ScaleType

# Construcao — por enum, string, ou nome de modo
s = Scale("C", ScaleType.Major)
s = Scale("C", "major")
s = Scale("D", "dorian")

# Identidade
s = Scale("D", "dorian")
s.tonic()         # Note("D")
s.parent()        # ScaleType.Major
s.mode_number()   # 2
s.mode_name()     # "Dorian"
s.quality()       # "minor"
s.brightness()    # 3

# Notas
s.notes()         # [Note("D"), Note("E"), Note("F"), ...]
s.size()          # 7
s.degree(3)       # Note("F")  — terceiro grau
s.contains(Note("F#"))  # False

# Grau encadeado (varargs)
s = Scale("C", "major")
s.degree(5)       # Note("G")           — grau V
s.degree(5, 5)    # Note("D")           — V do V
s.degree(5, 5, 3) # Note("F")           — III do V do V

# Walk: navegar pela escala
s.walk(1, 4)      # Note("F")           — de I, quarta = IV
s.walk(5, 5)      # Note("D")           — de V, quinta = II

# Modos: por numero ou nome
s.mode(2)                 # D Dorian
s.mode("lydian")          # F Lydian

# Pentatonica
s.pentatonic()            # C major pentatonic (5 notas)
Scale("A", "minor pentatonic")   # A C D E G

# Notas coloridas (OMT2): o que distingue um modo de outro
dor = Scale("C", "dorian")
dor.colors("ionian")     # [Eb, Bb] — notas que diferem do Ionian

# Identificacao: dadas as notas, descobre a escala
Scale.identify(["C", "D", "E", "F", "G", "A", "B"])  # C major
Scale.identify(["D", "E", "F", "G", "A", "B", "C"])  # D dorian

# Ouvir a escala (notas em sequencia ascendente)
s.play()                              # C D E F G A B C
s.play(duration=0.25, gap=0.03)       # mais rapido
s.play(waveform="triangle")           # timbre suave
```

### Modos — o conceito mais poderoso

Cada escala de 7 notas gera **7 modos**. Sao as mesmas notas, mas
comecando de um grau diferente. Cada modo tem uma **cor sonora** distinta:

| Modo        | Grau | Carater sonoro                                  |
|-------------|:----:|------------------------------------------------|
| Ionian      | 1    | Maior "normal" — alegre, estavel               |
| Dorian      | 2    | Menor com 6a maior — funk, soul, jazz          |
| Phrygian    | 3    | Espanhol, flamenco, metal                      |
| Lydian      | 4    | Sonhador, flutuante, cinema                    |
| Mixolydian  | 5    | Rock, blues, country                           |
| Aeolian     | 6    | Menor natural — melancolico, pop               |
| Locrian     | 7    | Instavel, diminuto — pouco usado como centro   |

**Cada familia de 7 notas tem seus proprios modos** — harmonica menor
(Phrygian Dominant, Ultralocrian...), melodica menor (Lydian Dominant,
Altered...), harmonica maior, etc. Sao **4+ familias x 7 modos = 28+
escalas** com sonoridades unicas, todas acessiveis por nome.

#### Exemplo: listar e ouvir todos os modos

```python
s = Scale("C", "major")
for i in range(1, s.size() + 1):
    m = s.mode(i)
    notes = " ".join(str(n) for n in m.notes())
    print(f"{i}. {m.mode_name():<12s}  {m.tonic()}  {notes}")
    m.play(duration=0.25)   # ouvir cada modo
```

### CLI

```
$ gingo scale "D dorian"

  Scale: Scale("D", "Dorian")
  ———————————————————————————
    Parent.............. Major
    Mode................ Dorian (mode 2)
    Quality............. minor
    Brightness.......... 3
    ...
```

```
$ gingo scale "C major" --modes

  Modes
  —————
    1. Ionian                C    C  D  E  F  G  A  B
    2. Dorian                D    D  E  F  G  A  B  C
    3. Phrygian              E    E  F  G  A  B  C  D
    ...
```

| Opcao          | O que faz                                              |
|----------------|--------------------------------------------------------|
| `--modes`      | Mostra todos os modos da familia com nomes             |
| `--degrees`    | Mostra cada nota grau a grau                           |
| `--pentatonic` | Usa filtro pentatonico (5 notas)                       |
| `--colors REF` | Notas que diferem de um modo de referencia              |
| `--degree N..` | Grau encadeado (ex: `--degree 5 5` = V de V)           |
| `--walk N..`   | Caminho por graus (start + passos)                     |
| `--relative`   | Mostra a relativa maior/menor                          |
| `--parallel`   | Mostra a paralela maior/menor                          |
| `--neighbors`  | Mostra vizinhas no circulo de quintas                  |
| `--identify`   | Identifica escala a partir de notas (separar com virgula) |
| `--play`       | Toca a escala pelo audio do sistema                       |
| `--wav FILE`   | Exporta para arquivo WAV                                  |
| `--gap SEC`    | Silencio entre notas consecutivas (default 0.05)          |

```
gingo scale "C major" --modes             # modos da maior
gingo scale "C harmonic minor" --modes    # modos da harmonica menor
gingo scale "C melodic minor" --modes     # modos da melodica menor
gingo scale "D dorian" --colors ionian    # notas coloridas
gingo scale "C major pentatonic"          # pentatonica maior
gingo scale "A minor pentatonic"          # pentatonica menor
gingo scale "C whole tone"               # tons inteiros
gingo scale "A blues"                    # blues
gingo scale "C major" --degree 5 5       # V de V
gingo scale "C major" --walk 1 4         # caminha a partir do I
gingo scale "C major" --relative         # relativa (A menor)
gingo scale "A natural minor" --parallel # paralela (A maior)
gingo scale "G major" --neighbors        # vizinhas no circulo
gingo scale "C,D,E,F,G,A,B" --identify  # identifica: C major
gingo scale "A,B,C,D,E,F,G" --identify  # identifica: A natural minor
gingo scale "C major" --play             # ouvir a escala
gingo scale "A blues" --play --gap 0.08  # blues com mais separacao
```

**O que da pra estudar:** construcao de escalas, modos de todas as familias,
pentatonicas, notas coloridas, relacoes entre tonalidades.
**Ouca a diferenca:** toque cada modo com `--play` para sentir a cor sonora.

---

## 5. Field — os acordes que pertencem a uma escala

O campo harmonico responde a pergunta fundamental:
**"quais acordes pertencem a esta tonalidade?"**

O processo: pegue uma escala de 7 notas. Em cada grau, empilhe tercas
(pule uma nota, pegue a proxima, pule, pegue). Isso gera automaticamente
um acorde para cada grau.

### Python

```python
from gingo import Field, HarmonicFunction

f = Field("C", "major")

f.tonic()       # Note("C")
f.scale()       # Scale("C", "major")
f.size()        # 7

# Triades (3 notas) e tetrades (4 notas) de cada grau
f.chords()      # [Chord("CM"), Chord("Dm"), Chord("Em"), ...]
f.sevenths()    # [Chord("C7M"), Chord("Dm7"), Chord("Em7"), ...]

# Acorde de um grau especifico
f.chord(1)      # Chord("CM")   — triade do grau I
f.chord(5)      # Chord("GM")   — triade do grau V
f.seventh(5)    # Chord("G7")   — tetrade do grau V

# Funcao harmonica (T/S/D) e papel de cada grau
f.function(1)            # HarmonicFunction.Tonic
f.function(5)            # HarmonicFunction.Dominant
f.function(5).name       # "Dominant"
f.function(5).short      # "D"
f.role(1)                # "primary"
f.role(6)                # "relative of I"

# Funcao por nome de acorde ou objeto Chord
f.function("FM")         # HarmonicFunction.Subdominant
f.function("F#M")        # None (nao pertence ao campo)
f.role("Am")             # "relative of I"

# Acordes aplicados (tonicizacao)
f.applied("V7", 2)       # Chord("A7")  — V7 do grau II
f.applied("V7", "V")     # Chord("D7")  — V7 do grau V
f.applied(5, 2)          # Chord("A7")  — atalho numerico

# Ouvir o campo harmonico (7 acordes em sequencia)
f.play()                          # CM Dm Em FM GM Am Bdim
f.play(duration=0.8, gap=0.1)    # mais lento, com respiracao
f.to_wav("campo_c.wav")          # exportar WAV
```

#### Exemplo: comparar campos de duas tonalidades

```python
major = Field("C", "major")
minor = Field("A", "natural minor")

print("C major:")
for i, (t, s) in enumerate(zip(major.chords(), major.sevenths()), 1):
    print(f"  {i}: {t.name():<8s} {s.name()}")

print("\nA natural minor:")
for i, (t, s) in enumerate(zip(minor.chords(), minor.sevenths()), 1):
    print(f"  {i}: {t.name():<8s} {s.name()}")
```

### CLI

```
$ gingo field "C major"

         Triad         Seventh       Notes (triad)         Notes (7th)
         -----         -------       -------------         -----------
      I  CM            C7M           C E G                 C E G B
     II  Dm            Dm7           D F A                 D F A C
    III  Em            Em7           E G B                 E G B D
     IV  FM            F7M           F A C                 F A C E
      V  GM            G7            G B D                 G B D F
     VI  Am            Am7           A C E                 A C E G
    VII  Bdim          Bm7(b5)       B D F                 B D F A
```

O que essa tabela diz:

- **Grau I** (C): acorde maior com setima maior (C7M) — **tonica**, repouso
- **Grau II** (D): acorde menor com setima (Dm7) — **subdominante**
- **Grau IV** (F): acorde maior com setima maior (F7M) — **subdominante**
- **Grau V** (G): acorde maior com setima menor (G7) — **dominante**, tensao
- **Grau VI** (A): acorde menor com setima (Am7) — tonica relativa

Se uma musica esta em Do maior, esses sao os **7 acordes nativos**.
Qualquer outro acorde e "emprestado" de outro lugar.

Use `--functions` para ver a funcao harmonica (T/S/D) e o papel de cada grau:

```
$ gingo field "C major" --functions

         Triad         Seventh       Function        Role
         -----         -------       --------        ----
      I  CM            C7M           Tonic           primary
     II  Dm            Dm7           Subdominant     relative of IV
    III  Em            Em7           Tonic           transitive
     IV  FM            F7M           Subdominant     primary
      V  GM            G7            Dominant        primary
     VI  Am            Am7           Tonic           relative of I
    VII  Bdim          Bm7(b5)       Dominant        relative of V
```

Use `--applied` para acordes aplicados (tonicizacao):

```
gingo field "C major" --applied V7/II       # A7 (dominante do grau II)
gingo field "C major" --applied "IIm7(b5)/V"  # Am7(b5)
```

| Opcao          | O que faz                                              |
|----------------|--------------------------------------------------------|
| `--functions`  | Mostra funcao harmonica e papel de cada grau            |
| `--applied`    | Acorde aplicado (ex: `--applied V7/II`)                 |
| `--relative`   | Campo relativo maior/menor                              |
| `--parallel`   | Campo paralelo maior/menor                              |
| `--neighbors`  | Campos vizinhos no circulo de quintas                   |
| `--identify`   | Identifica campo a partir de notas ou acordes           |
| `--deduce`     | Deduz campos possiveis (ranqueado)                      |
| `--limit N`    | Limita resultados do --deduce (default 10)              |
| `--play`       | Toca os 7 acordes do campo pelo audio                   |
| `--wav FILE`   | Exporta para arquivo WAV                                |

### Detectar tonalidade (identify vs deduce)

O Gingo tem dois modos de deteccao:

- **`identify`**: exige um conjunto completo e inequivoco e retorna **um** campo.
- **`deduce`**: aceita input parcial e retorna **varias** possibilidades ranqueadas.

```python
# Campo exato (input completo)
Field.identify(["C", "D", "E", "F", "G", "A", "B"])   # Field("C", "major")
Field.identify(["CM", "Dm", "Em", "FM", "GM", "Am"])  # Field("C", "major")

# Inferencia ranqueada (input parcial)
matches = Field.deduce(["CM", "FM"])
matches[0].field   # Field("C", "major") ou Field("F", "major")
matches[0].score   # 1.0
```

```
gingo field "A natural minor"                  # campo de La menor
gingo field "C harmonic minor"                 # campo da harmonica menor
gingo field "D melodic minor"                  # campo da melodica menor
gingo field "Bb major"                         # campo de Sib maior
gingo field "CM,Dm,Em,FM,GM,Am" --identify     # identifica: C major
gingo field "CM,FM,G7" --identify              # identifica: C major
gingo field "CM,FM" --deduce                   # deduz tonalidades possiveis
gingo field "Am,Dm,E7" --deduce                # deduz: A harmonic minor lidera
gingo field "CM,FM" --deduce --limit 5         # limita a 5 resultados
gingo field "C major" --play                   # ouvir os 7 acordes
gingo field "A natural minor" --play --wav campo_am.wav  # ouvir + exportar
```

**O que da pra estudar:** harmonizacao de escalas, triades vs tetrades,
funcoes tonais (tonica, subdominante, dominante), relativa maior/menor.
**Ouca a diferenca:** toque campos de diferentes tonalidades com `--play`.

---

## 6. Compare — o que dois acordes tem em comum?

A comparacao responde: **"qual a relacao entre esses dois acordes?"**

Existem dois niveis de comparacao:

- **Absoluta** (`Chord.compare`): o que se pode dizer olhando apenas os dois acordes, sem contexto tonal
- **Contextual** (`Field.compare`): o que se pode dizer considerando uma tonalidade especifica

### Comparacao absoluta (Chord)

```python
from gingo import Chord

r = Chord("CM").compare(Chord("Am"))

# Notas
r.common_notes       # [Note("C"), Note("E")]  — notas em comum
r.exclusive_a        # [Note("G")]             — so no primeiro
r.exclusive_b        # [Note("A")]             — so no segundo

# Raizes
r.root_distance      # 3   — distancia em semitons (0-6, arco mais curto)
r.root_direction     # -3  — direcao com sinal (-6 a +6)

# Qualidade e tamanho
r.same_quality       # False — M vs m
r.same_size          # True  — ambos triades (3 notas)
r.common_intervals   # ["P1"]  — intervalos identicos

# Relacoes
r.enharmonic         # False — nao sao o mesmo acorde
r.subset             # ""    — nenhum contem o outro ("a_subset_of_b", "equal", etc.)
r.inversion          # False — nao sao inversoes

# Voice leading (Tymoczko 2011) e transformacao neo-Riemanniana (Cohn 2012)
r.voice_leading      # 4  — pareamento otimo de vozes em semitons (Tymoczko 2011)
r.transformation     # "R" — transformacao neo-Riemanniana (Cohn 2012)
```

#### Transformacoes neo-Riemannianas (Cohn 2012)

Tres operacoes fundamentais — P (Parallel), L (Leading-tone), R (Relative) —
que transformam triades movendo apenas uma nota por um semitom (Richard Cohn,
*Audacious Euphony*, 2012). Detectadas automaticamente para pares major/minor:

| Transformacao | Nome | Exemplo | O que muda |
|:---:|---|---|---|
| **P** | Parallel | CM → Cm | Terca muda (E→D#), raiz igual |
| **L** | Leading-tone | CM → Em | Fundamental desce meio tom (C→B) |
| **R** | Relative | CM → Am | Quinta sobe um tom (G→A) |

Quando nao ha transformacao direta, o Gingo tenta composicoes de 2 passos:

| Composicao | Exemplo | Caminho |
|:---:|---|---|
| **RP** | CM → AM | R(CM)=Am, P(Am)=AM |
| **LP** | CM → EM | L(CM)=Em, P(Em)=EM |
| **PL** | CM → G#M | P(CM)=Cm, L(Cm)=G#M |
| **PR** | CM → D#M | P(CM)=Cm, R(Cm)=D#M |

```python
Chord("CM").compare(Chord("Cm")).transformation   # "P"
Chord("CM").compare(Chord("Em")).transformation   # "L"
Chord("CM").compare(Chord("Am")).transformation   # "R"
Chord("CM").compare(Chord("AM")).transformation   # "RP"
Chord("CM").compare(Chord("EM")).transformation   # "LP"
Chord("C7M").compare(Chord("Am7")).transformation # ""  — so triades
```

#### Novas dimensoes de comparacao

```python
r = Chord("CM").compare(Chord("Cm"))

# Interval Vector (Forte 1973) — vetor de classes de intervalo (set theory)
r.interval_vector_a      # [0, 0, 1, 1, 1, 0]  — CM
r.interval_vector_b      # [0, 0, 1, 1, 1, 0]  — Cm (mesmo vetor!)
r.same_interval_vector   # True (Z-relation candidate)

# Transposicao T_n (Lewin 1987) — teoria transformacional generalizada
Chord("CM").compare(Chord("DM")).transposition     # 2 (T_2)
Chord("CM").compare(Chord("Cm")).transposition     # -1 (nao e transposicao)

# Dissonancia psicoacustica (Plomp & Levelt 1965 / Sethares 1998)
r.dissonance_a           # 0.057... (roughness do CM)
r.dissonance_b           # 0.072... (roughness do Cm)
```

#### Serializacao com to_dict()

Todas as structs de comparacao possuem `to_dict()` para facil serializacao:

```python
import json
d = Chord("CM").compare(Chord("Am")).to_dict()
print(json.dumps(d, indent=2))
# {"common_notes": ["C", "E"], "transformation": "R", ...}
```

### Comparacao contextual (Field)

Quando voce sabe a tonalidade, pode extrair muito mais informacao:

```python
from gingo import Field, ScaleType

f = Field("C", ScaleType.Major)

# Acordes diatonicos
r = f.compare(Chord("CM"), Chord("GM"))
r.degree_a           # 1 (grau I)
r.degree_b           # 5 (grau V)
r.function_a         # HarmonicFunction.Tonic
r.function_b         # HarmonicFunction.Dominant
r.same_function      # False
r.diatonic_a         # True
r.diatonic_b         # True
r.degree_distance    # 4 (|1-5|)

# Acordes relativos (mesma funcao harmonica)
r = f.compare(Chord("CM"), Chord("Am"))
r.relative           # True — I e VIm sao tonica
r.same_function      # True

# Root motion — movimento diatonico de raiz (Kostka & Payne)
r = f.compare(Chord("CM"), Chord("GM"))
r.root_motion        # "ascending_fifth"

r = f.compare(Chord("GM"), Chord("CM"))
r.root_motion        # "descending_fifth"

# Dominante secundaria (Kostka & Payne)
r = f.compare(Chord("D7"), Chord("GM"))
r.secondary_dominant # "a_is_V7_of_b" — D7 e o V7 de G

# Diminuta aplicada vii/x (Gauldin 1997)
r = f.compare(Chord("Bdim"), Chord("CM"))
r.applied_diminished # "a_is_viidim_of_b" — Bdim resolve em CM

# Substituicao tritonal (Kostka & Payne)
r = f.compare(Chord("G7"), Chord("C#7"))
r.tritone_sub        # True — raizes a 6 semitons, ambos dom7

# Mediante cromatica (Cohn 2012)
r = f.compare(Chord("CM"), Chord("EM"))
r.chromatic_mediant  # "upper" — terca maior ascendente

# Emprestimo modal
r = f.compare(Chord("CM"), Chord("Fm"))
r.diatonic_b         # False — Fm nao pertence a C maior
r.borrowed_b.scale_type  # "NaturalMinor" — emprestado de C menor
r.borrowed_b.degree      # 4 — grau IV da menor natural
r.foreign_b              # [Note("G#")] — Ab (= G#) nao pertence a C maior

# Acorde pivot (pertence a mais de uma tonalidade)
r = f.compare(Chord("CM"), Chord("Am"))
for p in r.pivot:
    print(f"{p.tonic} {p.scale_type}: {p.degree_a} → {p.degree_b}")
# C Major: 1 → 6
# A NaturalMinor: 3 → 1
# F Major: 5 → 3
# ...
```

### CLI

Se `--field` for usado, a comparacao absoluta vem primeiro e o contexto tonal
aparece em seguida.

```
$ gingo compare CM Am

  Compare: CM vs Am
  ——————————————————
    Common notes......... C, E
    Exclusive A.......... G
    Exclusive B.......... A
    Root distance........ 3
    Root direction....... -3
    Same quality......... no
    Same size............ yes
    Common intervals.... P1, 5J
    Enharmonic.......... no
    Voice leading........ 2 st
    Transformation....... R (Relative)
    Interval vector A... [0, 0, 1, 1, 1, 0]
    Interval vector B... [0, 0, 1, 1, 1, 0]
    Same interval vector yes
    Dissonance A........ 0.1826
    Dissonance B........ 0.1027

```

```
$ gingo compare CM GM --field "C major"

  Compare: CM vs GM
  —————————————————
    Common notes........ G
    Exclusive A......... C, E
    Exclusive B......... B, D
    Root distance....... 5
    Root direction...... -5
    Same quality........ yes
    Same size........... yes
    Common intervals.... P1, 3M, 5J
    Enharmonic.......... no
    Voice leading....... 3 st
    Transformation...... LR (Leading-tone + Relative)
    Interval vector A... [0, 0, 1, 1, 1, 0]
    Interval vector B... [0, 0, 1, 1, 1, 0]
    Same interval vector yes
    Transposition....... T7
    Dissonance A........ 0.1826
    Dissonance B........ 0.0717


  Context: C major
  ————————————————
    Degree A............ I
    Degree B............ V
    Function A.......... Tonic
    Function B.......... Dominant
    Same function....... no
    Diatonic A.......... yes
    Diatonic B.......... yes
    Degree distance..... 4
    Root motion......... ascending_fifth
```

**O que da pra estudar:** relacoes entre acordes, voice leading (Tymoczko 2011),
transformacoes neo-Riemannianas (Cohn 2012), interval vector (Forte 1973),
transposicao T_n (Lewin 1987), dissonancia psicoacustica (Plomp & Levelt 1965 /
Sethares 1998), emprestimo modal, acordes pivot para modulacao, dominantes
secundarias (Kostka & Payne), diminuta aplicada (Gauldin 1997), substituicao
tritonal, mediantes cromaticas.
