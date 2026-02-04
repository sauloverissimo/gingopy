# API completa — referencia rapida

### Classes

| Classe       | Construtor                                                    | Descricao            |
|--------------|---------------------------------------------------------------|----------------------|
| `Note`       | `Note("C#")`                                                  | Uma nota             |
| `Interval`   | `Interval("5J")` ou `Interval(7)`                             | Distancia em semitons|
| `Chord`      | `Chord("Am7")`                                                | Acorde por nome      |
| `Scale`      | `Scale("C", "major")` ou `Scale("D", "dorian")`              | Escala               |
| `Field`      | `Field("C", "major")`                                         | Campo harmonico      |
| `Tree`       | `Tree("C", "major")`                                          | Arvore harmonica (progressoes) **(beta)** |
| `Duration`   | `Duration("quarter")` ou `Duration(1, 4)`                     | Duracao ritmica                |
| `Tempo`      | `Tempo(120)`                                                   | Andamento em BPM               |
| `TimeSignature` | `TimeSignature(4, 4)`                                       | Formula de compasso            |
| `NoteEvent`  | `NoteEvent(Note("C"), Duration("quarter"), 4)`                 | Nota com duracao e oitava      |
| `ChordEvent` | `ChordEvent(Chord("CM"), Duration("half"), 4)`                 | Acorde com duracao e oitava    |
| `Rest`       | `Rest(Duration("quarter"))`                                    | Pausa (silencio)               |
| `Sequence`   | `Sequence(Tempo(120), TimeSignature(4, 4))`                    | Sequencia de eventos musicais  |

### Metodos de Note

| Metodo                    | Retorno  | Descricao                         |
|---------------------------|----------|-----------------------------------|
| `name()`                  | `str`    | Nome como informado               |
| `natural()`               | `str`    | Nome canonico (com sustenido)     |
| `sound()`                 | `str`    | Nota natural (sem acidente)       |
| `semitone()`              | `int`    | Indice cromatico (0-11)           |
| `frequency(octave=4)`     | `float`  | Frequencia em Hz (A4=440)         |
| `is_enharmonic(other)`    | `bool`   | Mesmo som, nomes diferentes?      |
| `transpose(semitones)`    | `Note`   | Nova nota transposta              |

### Metodos de Interval

| Metodo            | Retorno  | Descricao                    |
|-------------------|----------|------------------------------|
| `label()`         | `str`    | Abreviacao (P1, 3m, 5J...)   |
| `anglo_saxon()`   | `str`    | Nome anglo-saxao (P5, ma3...) |
| `semitones()`     | `int`    | Distancia em semitons        |
| `degree()`        | `int`    | Numero do grau               |
| `octave()`        | `int`    | Oitava (1 ou 2)              |

### Metodos de Chord

| Metodo                     | Retorno          | Descricao                       |
|----------------------------|------------------|---------------------------------|
| `name()`                   | `str`            | Nome completo (Am7, C7M...)     |
| `root()`                   | `Note`           | Nota raiz                       |
| `type()`                   | `str`            | Sufixo do tipo (m7, 7M, dim...)  |
| `notes()`                  | `List[Note]`     | Notas do acorde                 |
| `formal_notes()`           | `List[Note]`     | Notas com grafia formal         |
| `intervals()`              | `List[Interval]` | Intervalos do acorde            |
| `interval_labels()`        | `List[str]`      | Labels dos intervalos           |
| `size()`                   | `int`            | Quantidade de notas             |
| `contains(note)`           | `bool`           | Nota pertence ao acorde?        |
| `compare(other)`           | `ChordComparison`| Comparacao detalhada (18 dimensoes) |
| `Chord.identify(notes)`    | `Chord`          | Identifica acorde pelas notas   |

### Metodos de Scale

| Metodo                    | Retorno          | Descricao                       |
|---------------------------|------------------|---------------------------------|
| `tonic()`                 | `Note`           | Nota tonica                     |
| `parent()`                | `ScaleType`      | Familia parental (Major, HarmonicMinor, ...) |
| `mode_number()`           | `int`            | Numero do modo (1-7)            |
| `mode_name()`             | `str`            | Nome do modo (Ionian, Dorian, ...) |
| `quality()`               | `str`            | Qualidade tonal ("major" / "minor") |
| `brightness()`            | `int`            | Brilho do modo (1=Locrian, 7=Lydian) |
| `is_pentatonic()`         | `bool`           | Se esta no filtro pentatonico   |
| `type()`                  | `ScaleType`      | Tipo da escala (backward compat, = parent) |
| `modality()`              | `Modality`       | Diatonica ou pentatonica (backward compat) |
| `notes()`                 | `List[Note]`     | Notas da escala                 |
| `formal_notes()`          | `List[Note]`     | Notas com grafia formal         |
| `degree(*degrees)`        | `Note`           | Grau encadeado: `degree(5, 5)` = V do V |
| `walk(start, *steps)`     | `Note`           | Navegar pela escala: `walk(1, 4)` = IV |
| `size()`                  | `int`            | Quantidade de notas             |
| `contains(note)`          | `bool`           | Nota pertence a escala?         |
| `mode(n_or_name)`         | `Scale`          | Modo por numero (int) ou nome (str) |
| `pentatonic()`            | `Scale`          | Versao pentatonica da escala    |
| `colors(reference)`       | `List[Note]`     | Notas que diferem de um modo de referencia |
| `mask()`                  | `List[int]`      | Mascara binaria (24 bits)       |
| `Scale.identify(notes)`   | `Scale`          | Identifica escala a partir de um conjunto completo de notas |

### Metodos de Field

| Metodo              | Retorno                    | Descricao                             |
|---------------------|----------------------------|---------------------------------------|
| `tonic()`           | `Note`                     | Nota tonica                           |
| `scale()`           | `Scale`                    | Escala base                           |
| `chords()`          | `List[Chord]`              | Triades de todos os graus             |
| `sevenths()`        | `List[Chord]`              | Tetrades de todos os graus            |
| `chord(degree)`     | `Chord`                    | Triade de um grau especifico          |
| `seventh(degree)`   | `Chord`                    | Tetrade de um grau especifico         |
| `applied(func, target)` | `Chord`                | Acorde aplicado (tonicizacao)         |
| `function(degree)`  | `HarmonicFunction`         | Funcao harmonica do grau (T/S/D)      |
| `function(chord)`   | `Optional[HarmonicFunction]` | Funcao do acorde (None se nao pertence) |
| `role(degree)`      | `str`                      | Papel do grau ("primary", "relative of I", etc.) |
| `role(chord)`       | `Optional[str]`            | Papel do acorde (None se nao pertence) |
| `compare(a, b)`     | `FieldComparison`          | Comparacao contextual (21 dimensoes)  |
| `size()`            | `int`                      | Quantidade de graus                   |
| `Field.identify(items)` | `Field`                 | Identifica campo a partir de notas ou acordes completos |
| `Field.deduce(items, limit=10)` | `List[FieldMatch]` | Deduz campos provaveis a partir de input parcial |

### Metodos de Tree

| Metodo                      | Retorno             | Descricao                             |
|-----------------------------|---------------------|---------------------------------------|
| `tonic()`                   | `Note`              | Nota tonica                           |
| `type()`                    | `ScaleType`         | Tipo de escala                        |
| `branches()`                | `List[str]`         | Todos os branches harmonicos          |
| `paths(branch)`             | `List[HarmonicPath]`| Todos os caminhos de um branch        |
| `shortest_path(from_, to)`  | `List[str]`         | Caminho mais curto entre branches     |
| `is_valid_progression(branches)` | `bool`         | Valida progressao harmonica           |
| `function(branch)`          | `HarmonicFunction`  | Funcao harmonica (T/S/D)              |
| `branches_with_function(func)` | `List[str]`      | Branches com funcao especifica        |
| `to_dot(show_functions=False)` | `str`            | Exporta para Graphviz DOT             |
| `to_mermaid()`              | `str`               | Exporta para diagrama Mermaid         |

### Atributos de HarmonicPath

| Atributo           | Tipo        | Descricao                       |
|--------------------|-------------|---------------------------------|
| `id`               | `int`       | Identificador do caminho        |
| `branch`           | `str`       | Nome do branch destino          |
| `chord`            | `Chord`     | Acorde resolvido                |
| `interval_labels`  | `List[str]` | Labels dos intervalos do acorde |
| `note_names`       | `List[str]` | Nomes das notas do acorde       |

### Atributos de ChordComparison

| Atributo           | Tipo             | Descricao                                |
|--------------------|------------------|------------------------------------------|
| `common_notes`     | `List[Note]`     | Notas em comum                           |
| `exclusive_a`      | `List[Note]`     | Notas exclusivas do acorde A             |
| `exclusive_b`      | `List[Note]`     | Notas exclusivas do acorde B             |
| `root_distance`    | `int`            | Distancia entre raizes (0-6 semitons)    |
| `root_direction`   | `int`            | Direcao com sinal (-6 a +6)             |
| `same_quality`     | `bool`           | Mesmo tipo (M, m, dim, etc.)            |
| `same_size`        | `bool`           | Mesmo numero de notas                    |
| `common_intervals` | `List[str]`      | Intervalos identicos em ambos            |
| `enharmonic`       | `bool`           | Mesmo conjunto de notas                  |
| `subset`           | `str`            | `""`, `"a_subset_of_b"`, `"b_subset_of_a"`, `"equal"` |
| `voice_leading`    | `int`            | Pareamento otimo de vozes em semitons (Tymoczko 2011). -1 se tamanhos diferentes |
| `transformation`   | `str`            | Transformacao neo-Riemanniana (Cohn 2012): `""`, `"P"`, `"L"`, `"R"`, `"RP"`, `"LP"`, `"PL"`, `"PR"`, `"LR"`, `"RL"` (so triades) |
| `inversion`        | `bool`           | Mesmas notas, raiz diferente             |
| `interval_vector_a`| `List[int]`      | Vetor de classes de intervalo (Forte 1973): 6 elementos ic1-6 do acorde A |
| `interval_vector_b`| `List[int]`      | Vetor de classes de intervalo (Forte 1973) do acorde B |
| `same_interval_vector` | `bool`       | Mesmo vetor = candidato a Z-relation (Forte 1973) |
| `transposition`    | `int`            | Indice de transposicao T_n (Lewin 1987): 0-11, ou -1 se nao sao transposicao |
| `dissonance_a`     | `float`          | Roughness psicoacustico (Plomp & Levelt 1965 / Sethares 1998) do acorde A |
| `dissonance_b`     | `float`          | Roughness psicoacustico (Plomp & Levelt 1965 / Sethares 1998) do acorde B |

| Metodo         | Retorno | Descricao                                         |
|----------------|---------|---------------------------------------------------|
| `to_dict()`    | `dict`  | Serializa todos os campos em dict Python puro (Notes como strings) |

### Atributos de FieldComparison

| Atributo             | Tipo                         | Descricao                               |
|----------------------|------------------------------|-----------------------------------------|
| `degree_a`           | `Optional[int]`              | Grau do acorde A (None se nao-diatonico)|
| `degree_b`           | `Optional[int]`              | Grau do acorde B                        |
| `function_a`         | `Optional[HarmonicFunction]` | Funcao harmonica de A                   |
| `function_b`         | `Optional[HarmonicFunction]` | Funcao harmonica de B                   |
| `role_a`             | `Optional[str]`              | Papel de A ("primary", "relative of I") |
| `role_b`             | `Optional[str]`              | Papel de B                              |
| `degree_distance`    | `Optional[int]`              | Distancia entre graus                   |
| `same_function`      | `Optional[bool]`             | Mesma funcao harmonica                  |
| `relative`           | `bool`                       | Par de acordes relativos                |
| `progression`        | `bool`                       | Reservado para uso futuro               |
| `root_motion`        | `str`                        | Movimento diatonico de raiz (Kostka & Payne): `""`, `"ascending_fifth"`, `"descending_fifth"`, `"ascending_third"`, `"descending_third"`, `"ascending_step"`, `"descending_step"`, `"tritone"`, `"unison"` |
| `secondary_dominant` | `str`                        | Dominante secundaria (Kostka & Payne): `""`, `"a_is_V7_of_b"`, `"b_is_V7_of_a"` |
| `applied_diminished` | `str`                        | Diminuta aplicada vii/x (Gauldin 1997): `""`, `"a_is_viidim_of_b"`, `"b_is_viidim_of_a"` |
| `diatonic_a`         | `bool`                       | A pertence ao campo                     |
| `diatonic_b`         | `bool`                       | B pertence ao campo                     |
| `borrowed_a`         | `Optional[BorrowedInfo]`     | De qual escala A e emprestado           |
| `borrowed_b`         | `Optional[BorrowedInfo]`     | De qual escala B e emprestado           |
| `pivot`              | `List[PivotInfo]`            | Tonalidades onde ambos tem grau         |
| `tritone_sub`        | `bool`                       | Substituicao tritonal (ambos dom7, dist=6) |
| `chromatic_mediant`  | `str`                        | `""`, `"upper"`, `"lower"`              |
| `foreign_a`          | `List[Note]`                 | Notas de A fora da escala               |
| `foreign_b`          | `List[Note]`                 | Notas de B fora da escala               |

| Metodo         | Retorno | Descricao                                         |
|----------------|---------|---------------------------------------------------|
| `to_dict()`    | `dict`  | Serializa todos os campos em dict Python puro     |

### Atributos de FieldMatch

| Atributo     | Tipo        | Descricao                                   |
|--------------|-------------|---------------------------------------------|
| `field`      | `Field`     | Campo harmonico candidato                    |
| `score`      | `float`     | Proporcao de matches (0.0 a 1.0)            |
| `matched`    | `int`       | Quantidade de itens reconhecidos             |
| `total`      | `int`       | Total de itens no input                      |
| `roles`      | `List[str]` | Papel/romanos para cada item                 |

| Metodo         | Retorno | Descricao                                         |
|----------------|---------|---------------------------------------------------|
| `to_dict()`    | `dict`  | Serializa em dict                                 |

### Atributos de BorrowedInfo

| Atributo     | Tipo               | Descricao                      |
|--------------|--------------------|---------------------------------|
| `scale_type` | `str`              | Tipo da escala de origem        |
| `degree`     | `int`              | Grau nessa escala               |
| `function`   | `HarmonicFunction` | Funcao nessa escala             |
| `role`       | `str`              | Papel nessa escala              |

| Metodo         | Retorno | Descricao                                         |
|----------------|---------|---------------------------------------------------|
| `to_dict()`    | `dict`  | Serializa em dict (function como string do nome)  |

### Atributos de PivotInfo

| Atributo     | Tipo   | Descricao                          |
|--------------|--------|------------------------------------|
| `tonic`      | `str`  | Tonica da tonalidade pivot         |
| `scale_type` | `str`  | Tipo de escala                     |
| `degree_a`   | `int`  | Grau do acorde A nessa tonalidade  |
| `degree_b`   | `int`  | Grau do acorde B nessa tonalidade  |

| Metodo         | Retorno | Descricao                                         |
|----------------|---------|---------------------------------------------------|
| `to_dict()`    | `dict`  | Serializa em dict                                 |

### Metodos de Duration

| Metodo                     | Retorno    | Descricao                             |
|----------------------------|------------|---------------------------------------|
| `numerator()`              | `int`      | Numerador da fracao                   |
| `denominator()`            | `int`      | Denominador da fracao                 |
| `beats()`                  | `float`    | Valor em tempos (1.0 = semibreve)     |
| `dotted()`                 | `Duration` | Versao pontuada (1.5x)               |
| `double_dotted()`          | `Duration` | Versao duplamente pontuada (1.75x)   |
| `triplet()`                | `Duration` | Versao em tercina (2/3x)             |

Nomes aceitos: `"whole"`, `"half"`, `"quarter"`, `"eighth"`, `"sixteenth"`, `"thirty_second"`

### Metodos de Tempo

| Metodo                     | Retorno  | Descricao                             |
|----------------------------|----------|---------------------------------------|
| `bpm()`                    | `int`    | Batidas por minuto                    |
| `seconds(duration)`        | `float`  | Duracao em segundos para um Duration  |
| `marking()`                | `str`    | Marcacao italiana (Allegro, Adagio...)  |

### Metodos de TimeSignature

| Metodo                     | Retorno  | Descricao                             |
|----------------------------|----------|---------------------------------------|
| `beats_per_measure()`      | `int`    | Tempos por compasso (numerador)       |
| `beat_value()`             | `int`    | Figura que vale um tempo (denominador)|

### Metodos de NoteEvent

| Metodo                     | Retorno    | Descricao                             |
|----------------------------|------------|---------------------------------------|
| `note()`                   | `Note`     | Nota associada                        |
| `duration()`               | `Duration` | Duracao do evento                     |
| `octave()`                 | `int`      | Oitava do evento                      |
| `frequency(tuning=440.0)`  | `float`    | Frequencia em Hz                      |

### Metodos de ChordEvent

| Metodo                     | Retorno            | Descricao                        |
|----------------------------|--------------------|----------------------------------|
| `chord()`                  | `Chord`            | Acorde associado                 |
| `duration()`               | `Duration`         | Duracao do evento                |
| `octave()`                 | `int`              | Oitava do evento                 |
| `note_events()`            | `List[NoteEvent]`  | NoteEvents das notas do acorde   |

### Metodos de Rest

| Metodo                     | Retorno    | Descricao                             |
|----------------------------|------------|---------------------------------------|
| `duration()`               | `Duration` | Duracao do silencio                   |

### Metodos de Sequence

| Metodo                     | Retorno              | Descricao                            |
|----------------------------|----------------------|--------------------------------------|
| `tempo()`                  | `Tempo`              | Andamento da sequencia               |
| `time_signature()`         | `TimeSignature`      | Formula de compasso                  |
| `add(event)`               | `None`               | Adiciona NoteEvent, ChordEvent ou Rest |
| `__len__()`                | `int`                | Quantidade de eventos                |
| `__getitem__(i)`           | `NoteEvent\|ChordEvent\|Rest` | Acesso por indice (suporta negativo) |

---

## Audio (`gingo.audio`)

### Waveform (enum)

| Valor               | String    | Timbre                           |
|---------------------|-----------|----------------------------------|
| `Waveform.SINE`     | `"sine"`     | Puro, suave, sem harmonicos   |
| `Waveform.SQUARE`   | `"square"`   | Rico, cheio, tipo 8-bit       |
| `Waveform.SAWTOOTH` | `"sawtooth"` | Brilhante, agressivo          |
| `Waveform.TRIANGLE` | `"triangle"` | Suave, levemente oco          |

### Envelope (ADSR)

```python
from gingo.audio import Envelope

env = Envelope(attack=0.01, decay=0.08, sustain=0.6, release=0.2)
amp = env.amplitude(t, note_duration)  # float
```

| Atributo   | Tipo    | Default  | Descricao                             |
|------------|---------|----------|---------------------------------------|
| `attack`   | `float` | `0.01`   | Tempo de subida em segundos           |
| `decay`    | `float` | `0.08`   | Tempo de descida do pico ao sustain   |
| `sustain`  | `float` | `0.6`    | Nivel de amplitude (0.0-1.0)          |
| `release`  | `float` | `0.2`    | Tempo de descida ao soltar a nota     |

### play()

```python
from gingo.audio import play

play(obj, *, octave=4, duration=0.5, waveform="sine",
     amplitude=0.8, envelope=None, strum=0.03, gap=0.05,
     tuning=440.0, sample_rate=44100)
```

Aceita: `Note`, `Chord`, `Scale`, `Field`, `Tree`, `Sequence`, `NoteEvent`, `ChordEvent`, `Rest`, `list[str]`

### to_wav()

```python
from gingo.audio import to_wav

to_wav(obj, path, *, octave=4, duration=0.5, waveform="sine",
       amplitude=0.8, envelope=None, strum=0.03, gap=0.05,
       tuning=440.0, sample_rate=44100)
```

Mesmos parametros de `play()`. Exporta WAV 16-bit mono.

### Parametros de audio

| Parametro     | Tipo                 | Default      | Descricao                                      |
|---------------|----------------------|--------------|-------------------------------------------------|
| `octave`      | `int`                | `4`          | Oitava base (4 = Do central)                    |
| `duration`    | `float`              | `0.5`        | Segundos por nota/acorde                        |
| `waveform`    | `str` ou `Waveform`  | `"sine"`     | Forma de onda                                   |
| `amplitude`   | `float`              | `0.8`        | Volume (0.0 a 1.0)                              |
| `envelope`    | `Envelope` ou `None` | `None`       | Envelope ADSR                                   |
| `strum`       | `float`              | `0.03`       | Delay entre notas de um acorde (segundos)       |
| `gap`         | `float`              | `0.05`       | Silencio entre notas/acordes consecutivos       |
| `tuning`      | `float`              | `440.0`      | Referencia A4 em Hz                              |
| `sample_rate` | `int`                | `44100`      | Amostras por segundo                             |

### Metodos .play() e .to_wav() nas classes

Todas as classes musicais possuem `.play(**kwargs)` e `.to_wav(path, **kwargs)`:

```python
Note("C").play()
Chord("Am7").play(waveform="triangle")
Scale("C", "major").play(duration=0.3)
Field("C", "major").play(duration=0.6)
Tree("C", "major").play()

Note("C").to_wav("do.wav")
Chord("Am7").to_wav("am7.wav", waveform="triangle")
```

### Enums

| Enum               | Valores                                                        |
|--------------------|----------------------------------------------------------------|
| `ScaleType`        | `Major`, `NaturalMinor`, `HarmonicMinor`, `MelodicMinor`, `Diminished`, `HarmonicMajor`, `WholeTone`, `Augmented`, `Blues`, `Chromatic` |
| `Modality`         | `Diatonic`, `Pentatonic`                                       |
| `HarmonicFunction` | `Tonic`, `Subdominant`, `Dominant` — props: `.name`, `.short`  |

---

## Resumo dos comandos CLI

| Comando     | O que faz                                       | Exemplo                              |
|-------------|-------------------------------------------------|--------------------------------------|
| `note`      | Propriedades de uma nota                        | `gingo note C#`                        |
| `interval`  | Distancia entre notas (0-23 semitons)           | `gingo interval 5J`                    |
| `chord`     | Acorde por nome ou identificacao reversa        | `gingo chord Am7`                      |
| `scale`     | Escala: notas, modos, mascara                   | `gingo scale "C major" --modes`        |
| `field`     | Campo harmonico: acordes, funcoes, tonicizacao  | `gingo field "C major" --functions`    |
| `compare`   | Comparacao entre dois acordes (absoluta ou contextual) | `gingo compare CM Am --field "C major"` |

## Todas as opcoes CLI

| Opcao              | Disponivel em | O que faz                                  |
|--------------------|---------------|--------------------------------------------|
| `--transpose N`    | `note`        | Transpoe N semitons (positivo=sobe)        |
| `--enharmonic NOTA`| `note`        | Verifica se duas notas sao enarmonicas     |
| `--distance NOTA`  | `note`        | Distancia no ciclo de quintas (0-6)        |
| `--fifths`         | `note`        | Mostra a posicao no circulo de quintas     |
| `--all`            | `interval`    | Mostra todos os 24 intervalos              |
| `--identify`       | `chord`       | Identifica acorde a partir de notas        |
| `--modes`          | `scale`       | Mostra todos os modos da familia com nomes |
| `--degrees`        | `scale`       | Mostra cada nota grau a grau               |
| `--pentatonic`     | `scale`       | Usa filtro pentatonico (5 notas)           |
| `--colors REF`     | `scale`       | Notas que diferem de um modo de referencia |
| `--degree N..`     | `scale`       | Grau encadeado (ex: `--degree 5 5` = V de V) |
| `--walk N..`       | `scale`       | Caminho por graus (start + passos)         |
| `--relative`       | `scale`       | Mostra a relativa maior/menor              |
| `--parallel`       | `scale`       | Mostra a paralela maior/menor              |
| `--neighbors`      | `scale`       | Mostra vizinhas no circulo de quintas      |
| `--identify`       | `scale`       | Identifica escala a partir de notas (virgula) |
| `--applied X/Y`    | `field`       | Acorde aplicado (tonicizacao) em notacao X/Y |
| `--functions`      | `field`       | Mostra funcao harmonica (T/S/D) e papel    |
| `--relative`       | `field`       | Mostra o campo relativo maior/menor        |
| `--parallel`       | `field`       | Mostra o campo paralelo maior/menor        |
| `--neighbors`      | `field`       | Mostra campos vizinhos no circulo de quintas |
| `--identify`       | `field`       | Identifica campo a partir de notas ou acordes |
| `--deduce`         | `field`       | Deduz campos possiveis (ranqueado)         |
| `--limit N`        | `field`       | Limita resultados do --deduce (default 10) |
| `--field "T tipo"` | `compare`     | Adiciona contexto tonal a comparacao       |
| `--play`           | `note`, `chord`, `scale`, `field` | Toca audio pelo sistema                |
| `--wav FILE`       | `note`, `chord`, `scale`, `field` | Exporta para arquivo WAV               |
| `--waveform WF`    | `note`, `chord`, `scale`, `field` | sine, square, sawtooth, triangle       |
| `--strum SEC`      | `note`, `chord`, `scale`, `field` | Delay entre notas do acorde (default 0.03) |
| `--gap SEC`        | `note`, `chord`, `scale`, `field` | Silencio entre notas/acordes (default 0.05) |

---

## Roteiro de estudo sugerido

Se voce esta comecando do zero, siga esta ordem:

### Nivel 1 — Fundamentos

1. **Conheca as 12 notas:**
   ```python
   from gingo import Note
   for name in ["C","C#","D","D#","E","F","F#","G","G#","A","A#","B"]:
       n = Note(name)
       print(f"{n.name():<3s}  semitone={n.semitone():2d}  freq={n.frequency():.1f} Hz")
       n.play(duration=0.2)  # ouca cada nota
   ```

2. **Entenda enarmonia:**
   ```python
   print(Note("Db").is_enharmonic(Note("C#")))  # True
   print(Note("Gb").is_enharmonic(Note("F#")))  # True
   ```

3. **Explore intervalos:**
   ```python
   from gingo import Interval
   for i in range(13):
       iv = Interval(i)
       print(f"{iv.semitones():2d} st  {iv.label():<6s}  {iv.anglo_saxon()}")
   ```

### Nivel 2 — Acordes

4. **Compare acordes basicos:**
   ```python
   from gingo import Chord
   for name in ["CM", "Cm", "C7", "Cm7", "C7M", "Cdim"]:
       c = Chord(name)
       notes = ", ".join(str(n) for n in c.notes())
       print(f"{c.name():<8s}  {notes}")
       c.play(duration=0.6)  # ouca cada acorde
   ```

5. **Pratique identificacao reversa:**
   ```python
   print(Chord.identify(["C", "E", "G"]).name())       # CM
   print(Chord.identify(["C", "E", "G", "B"]).name())   # C7M
   print(Chord.identify(["A", "C", "E"]).name())         # Am
   ```

### Nivel 3 — Escalas e modos

6. **Veja e ouca todos os modos da escala maior:**
   ```python
   from gingo import Scale
   s = Scale("C", "major")
   for i in range(1, s.size() + 1):
       m = s.mode(i)
       notes = " ".join(str(n) for n in m.notes())
       print(f"{i}. {m.mode_name():<12s} {m.tonic()} -> {notes}")
       m.play(duration=0.25)  # ouca cada modo
   ```

7. **Compare familias:**
   ```python
   for family in ["major", "harmonic minor", "melodic minor",
                   "harmonic major", "whole tone", "blues"]:
       s = Scale("C", family)
       notes = " ".join(str(n) for n in s.notes())
       print(f"{family:<16s}  {notes}")
   ```

### Nivel 4 — Harmonia

8. **Construa e ouca campos harmonicos:**
   ```python
   from gingo import Field
   f = Field("C", "major")
   for i, (t, s) in enumerate(zip(f.chords(), f.sevenths()), 1):
       print(f"  {i}: {t.name():<8s}  {s.name()}")
   f.play(duration=0.6)  # ouca o campo completo
   ```

---

## Referencia rapida de tipos de escala

Para qualquer lugar que pede tipo de escala (Python ou CLI):

### Familias parentais (10)

| Voce pode escrever                   | O Gingo entende como     |
|--------------------------------------|--------------------------|
| `"major"` ou `"maj"`                | Escala maior (Ionian)    |
| `"natural minor"` ou `"m"`          | Menor natural (Aeolian)  |
| `"harmonic minor"` ou `"minor"`     | Harmonica menor          |
| `"melodic minor"`                    | Melodica menor           |
| `"harmonic major"`                   | Harmonica maior          |
| `"diminished"` ou `"dim"`           | Diminuta (8 notas)       |
| `"whole tone"` ou `"wholetone"`     | Tons inteiros (6 notas)  |
| `"augmented"` ou `"aug"`            | Aumentada (6 notas)      |
| `"blues"`                            | Blues (6 notas)          |
| `"chromatic"`                        | Cromatica (12 notas)     |

### Nomes de modo (aceitos como tipo)

| Voce pode escrever        | Familia parental   | Modo |
|---------------------------|--------------------|------|
| `"ionian"`                | Major              | 1    |
| `"dorian"`                | Major              | 2    |
| `"phrygian"`              | Major              | 3    |
| `"lydian"`                | Major              | 4    |
| `"mixolydian"`            | Major              | 5    |
| `"aeolian"`               | Major              | 6    |
| `"locrian"`               | Major              | 7    |
| `"altered"`               | MelodicMinor       | 7    |
| `"lydian dominant"`       | MelodicMinor       | 4    |
| `"lydian augmented"`      | MelodicMinor       | 3    |
| `"phrygian dominant"`     | HarmonicMinor      | 5    |

### Variantes pentatonicas

| Voce pode escrever            | Resultado                    |
|-------------------------------|------------------------------|
| `"major pentatonic"`          | Maior pentatonica (5 notas)  |
| `"minor pentatonic"`          | Menor pentatonica (5 notas)  |
| `"dorian pentatonic"`         | Dorian pentatonica (5 notas) |
