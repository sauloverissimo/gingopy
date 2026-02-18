# Fretboard — Digitacoes e Visualizacao

O modulo Fretboard permite gerar digitacoes realisticas de acordes e visualizar escalas, notas e campos harmonicos para instrumentos de cordas com trastes.

---

## Instrumentos disponiveis

```python
from gingo import Fretboard

# Instrumentos padrao
violao = Fretboard.violao()       # 6 cordas, EADGBE, 19 trastes
cav    = Fretboard.cavaquinho()   # 4 cordas, DGBD, 17 trastes
band   = Fretboard.bandolim()     # 4 cordas, GDAE, 17 trastes

# Informacoes do instrumento
print(violao.num_strings())   # 6
print(violao.num_frets())     # 19
print(violao.tuning().name)   # "standard"
```

---

## Digitacoes de acordes

O metodo `fingering()` gera a digitacao otima para um acorde usando um algoritmo baseado no sistema CAGED. O algoritmo considera span, posicao, numero de dedos, cordas soando, deteccao de pestana e conforto fisico.

```python
from gingo import Fretboard, Chord

violao = Fretboard.violao()

# Digitacao padrao (melhor posicao)
f = violao.fingering(Chord("CM"))
print(f.chord_name)   # "CM"
print(f.base_fret)    # 1
print(f.barre)        # 0 (sem pestana)

# Detalhe de cada corda
for s in f.strings:
    print(f"  Corda {s.string}: traste={s.fret}, acao={s.action}")

# Acorde com pestana
f = violao.fingering(Chord("FM"))
print(f.barre)        # 1 (pestana no traste 1)

# Digitacao em posicao especifica
f = violao.fingering(Chord("CM"), position=5)
```

### Acoes de corda (StringAction)

| Valor     | Descricao                       |
|-----------|---------------------------------|
| `Open`    | Corda solta (sem dedo)          |
| `Fretted` | Corda pressionada em um traste  |
| `Muted`   | Corda abafada (sem som)         |

---

## Posicoes de escalas

```python
from gingo import Fretboard, Scale

violao = Fretboard.violao()

# Todas as posicoes da escala no braco
posicoes = violao.scale_positions(Scale("C", "major"), 0, 12)

for p in posicoes[:5]:
    print(f"  Corda {p.string}, traste {p.fret}: {p.note} (MIDI {p.midi})")
```

---

## Posicoes de notas

```python
from gingo import Fretboard, Note

violao = Fretboard.violao()

# Todas as posicoes de C no braco
posicoes = violao.positions(Note("C"))
for p in posicoes:
    print(f"  Corda {p.string}, traste {p.fret}: {p.note}")

# Posicao especifica
pos = violao.position(1, 5)  # Corda 1, traste 5
print(f"{pos.note} (MIDI {pos.midi})")  # A (MIDI 69)
```

---

## Visualizacao SVG

O `FretboardSVG` renderiza diagramas SVG de alta qualidade.

### Diagrama de acorde

```python
from gingo import Fretboard, FretboardSVG, Chord

violao = Fretboard.violao()

# Chord box vertical (padrao)
svg = FretboardSVG.chord(violao, Chord("Am"))
FretboardSVG.write(svg, "am_chord.svg")

# Digitacao especifica
f = violao.fingering(Chord("FM"))
svg = FretboardSVG.fingering(violao, f)
```

### Escala no braco

```python
from gingo import Fretboard, FretboardSVG, Scale

violao = Fretboard.violao()

# Escala horizontal (padrao)
svg = FretboardSVG.scale(violao, Scale("C", "major"), 0, 12)

# Trecho especifico
svg = FretboardSVG.scale(violao, Scale("A", "minor pentatonic"), 5, 12)
```

### Campo harmonico

```python
from gingo import Fretboard, FretboardSVG, Field, Layout

violao = Fretboard.violao()

# Campo completo (layout vertical)
svg = FretboardSVG.field(violao, Field("C", "major"))

# Layout em grid
svg = FretboardSVG.field(violao, Field("G", "major"), Layout.Grid)

# Layout horizontal
svg = FretboardSVG.field(violao, Field("D", "major"), Layout.Horizontal)
```

### Progressao

```python
from gingo import Fretboard, FretboardSVG, Field, Layout

violao = Fretboard.violao()

svg = FretboardSVG.progression(
    violao, Field("C", "major"),
    ["I", "V", "vi", "IV"],
    Layout.Horizontal
)
FretboardSVG.write(svg, "i_v_vi_iv.svg")
```

### Notas e posicoes customizadas

```python
from gingo import Fretboard, FretboardSVG, Note

violao = Fretboard.violao()

# Todas as posicoes de uma nota
svg = FretboardSVG.note(violao, Note("C"))

# Posicoes customizadas com titulo
posicoes = violao.scale_positions(Scale("A", "minor pentatonic"), 5, 12)
svg = FretboardSVG.positions(violao, posicoes, "Am Pentatonic (pos. 5)")

# Braco completo
svg = FretboardSVG.full(violao)
```

---

## Orientacao e lateralidade

Todos os metodos de FretboardSVG suportam dois parametros opcionais:

- **Orientation**: `Horizontal` (vista do braco) ou `Vertical` (chord box)
- **Handedness**: `RightHanded` (destro) ou `LeftHanded` (canhoto)

```python
from gingo import (
    Fretboard, FretboardSVG, Chord, Scale,
    Orientation, Handedness
)

violao = Fretboard.violao()

# Acorde horizontal, canhoto
svg = FretboardSVG.chord(violao, Chord("Am"), 0,
    Orientation.Horizontal, Handedness.LeftHanded)

# Escala vertical, destro
svg = FretboardSVG.scale(violao, Scale("C", "major"), 0, 12,
    Orientation.Vertical, Handedness.RightHanded)
```

### Defaults por metodo

| Metodo                      | Orientacao | Lateralidade |
|-----------------------------|------------|--------------|
| `chord()`, `fingering()`   | Vertical   | RightHanded  |
| `scale()`, `note()`, `positions()` | Horizontal | RightHanded |
| `field()`, `progression()`, `full()` | Vertical | RightHanded |

### Como funciona a lateralidade

A lateralidade **so espelha o eixo perpendicular as cordas**:

- **Horizontal**: eixo X (trastes) muda, eixo Y (cordas) mantem
- **Vertical**: eixo X (cordas) muda, eixo Y (trastes) mantem

---

## CLI

```bash
# Digitacao de acorde
gingo fretboard chord CM
gingo fretboard chord Am7 --svg am7.svg

# Escala
gingo fretboard scale "C major"
gingo fretboard scale "A minor pentatonic" --svg scale.svg

# Campo harmonico
gingo fretboard field "C major" --svg field.svg

# Canhoto
gingo fretboard chord Am --left

# Orientacao horizontal
gingo fretboard chord Am --horizontal
```

---

## Como visualizar SVGs

```python
# Opcao 1: Salvar e abrir no navegador
import subprocess
FretboardSVG.write(svg, "diagrama.svg")
subprocess.Popen(["xdg-open", "diagrama.svg"])  # Linux
# subprocess.Popen(["open", "diagrama.svg"])    # macOS

# Opcao 2: Jupyter notebook
from IPython.display import SVG, display
display(SVG(data=svg))
```

---

## Instrumentos customizados

```python
from gingo import Fretboard

# Afinacao Drop D
violao = Fretboard.violao()
midi = list(violao.tuning().open_midi)
midi[5] = 38  # E2 (40) -> D2 (38)
drop_d = Fretboard(midi, 22)
```
