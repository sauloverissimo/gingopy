# Gingo

**Biblioteca Python para Teoria Musical e Análise Harmônica**

Gingo é uma biblioteca Python que implementa conceitos de teoria musical ocidental, fornecendo ferramentas para análise harmônica, manipulação de escalas e acordes, e validação de progressões. Construída em C++17 com bindings Python (pybind11), combina performance nativa com API Python intuitiva.

---

## Arquitetura

Gingo modela conceitos musicais em camadas hierárquicas, onde cada componente fundamental serve de base para estruturas mais complexas:

```mermaid
graph LR
    A[Nota] --> B[Intervalo]
    B --> C[Acorde]
    A --> D[Escala]
    C --> E[Campo Harmônico]
    D --> E
    E --> F[Árvore Harmônica]

    style A fill:#e1f5e1,color:#000
    style B fill:#e3f2fd,color:#000
    style C fill:#fff3e0,color:#000
    style D fill:#fce4ec,color:#000
    style E fill:#f3e5f5,color:#000
    style F fill:#e0f2f1,color:#000
```

Esta arquitetura em camadas permite a construção progressiva de conceitos harmônicos complexos a partir de elementos fundamentais.

---

## Instalação

```bash
pip install gingo            # biblioteca completa
pip install gingo[audio]     # + playback com simpleaudio
```

Requer Python 3.10+. Wheels pré-compiladas disponíveis para Linux, macOS e Windows.
A sintese de audio e exportacao WAV funcionam sem dependencias extras.

---

## Exemplos

### 1. Notas Musicais

```python
from gingo import Note

# Criar uma nota
nota = Note("C")  # Dó
print(nota.name())  # "C"

# Calcular frequência
freq = nota.frequency(4)  # Dó da 4ª oitava (C4)
print(f"{freq:.2f} Hz")  # 261.63 Hz
```

**Ciclo cromático:**
```mermaid
graph LR
    C[C - Dó] --> D[D - Ré] --> E[E - Mi] --> F[F - Fá]
    F --> G[G - Sol] --> A[A - Lá] --> B[B - Si] --> C2[C - Dó]

    style C fill:#ff6b6b,color:#000
    style D fill:#feca57,color:#000
    style E fill:#48dbfb,color:#000
    style F fill:#ff9ff3,color:#000
    style G fill:#1dd1a1,color:#000
    style A fill:#f368e0,color:#000
    style B fill:#5f27cd,color:#fff
    style C2 fill:#ff6b6b,color:#000
```

### 2. Acordes

```python
from gingo import Chord

# Criar um acorde
acorde = Chord("Am7")  # Lá menor com sétima (tétrade)

# Obter notas do acorde
notas = [n.name() for n in acorde.notes()]
print(notas)  # ['A', 'C', 'E', 'G']
```

**Estrutura intervalar de Am7:**
```mermaid
graph TD
    A[Am7<br/>Tétrade menor] --> B[A - Fundamental]
    A --> C[C - Terça menor<br/>3 semitons]
    A --> D[E - Quinta justa<br/>7 semitons]
    A --> E[G - Sétima menor<br/>10 semitons]

    style A fill:#667eea,color:#fff
    style B fill:#f093fb,color:#000
    style C fill:#4facfe,color:#000
    style D fill:#43e97b,color:#000
    style E fill:#fa709a,color:#000
```

### 3. Escalas

```python
from gingo import Scale

# Criar uma escala
escala = Scale("C", "major")  # Dó maior (escala diatônica)

# Obter notas da escala
notas = [n.name() for n in escala.notes()]
print(notas)
# ['C', 'D', 'E', 'F', 'G', 'A', 'B']
```

**Escala de Dó maior com funções dos graus:**
```mermaid
graph LR
    I[I - C<br/>Tônica] --> II[II - D<br/>Supertônica]
    II --> III[III - E<br/>Mediante]
    III --> IV[IV - F<br/>Subdominante]
    IV --> V[V - G<br/>Dominante]
    V --> VI[VI - A<br/>Superdominante]
    VI --> VII[VII - B<br/>Sensível]
    VII --> I2[I - C<br/>Tônica]

    style I fill:#1dd1a1,color:#000
    style IV fill:#feca57,color:#000
    style V fill:#ee5a6f,color:#000
    style I2 fill:#1dd1a1,color:#000
```

### 4. Campo Harmônico

```python
from gingo import Field, ScaleType

# Criar campo harmônico
campo = Field("C", ScaleType.Major)

# Obter acordes diatônicos
acordes = [c.name() for c in campo.chords()]
print(acordes)
# ['CM', 'Dm', 'Em', 'FM', 'GM', 'Am', 'Bdim']
```

**Campo harmônico de Dó maior (tríades):**
```mermaid
graph LR
    I[I - CM<br/>Tônica] --> II[IIm - Dm<br/>Subdominante]
    II --> III[IIIm - Em<br/>Tônica relativa]
    III --> IV[IV - FM<br/>Subdominante]
    IV --> V[V - GM<br/>Dominante]
    V --> VI[VIm - Am<br/>Tônica relativa]
    VI --> VII[VIIdim - Bdim<br/>Dominante]

    style I fill:#1dd1a1,color:#000
    style II fill:#feca57,color:#000
    style III fill:#1dd1a1,color:#000
    style IV fill:#feca57,color:#000
    style V fill:#ee5a6f,color:#000
    style VI fill:#1dd1a1,color:#000
    style VII fill:#ee5a6f,color:#000
```

### 5. Audio — ouvir o que voce codou

```python
from gingo import Note, Chord, Scale, Field

Note("C").play()                        # nota unica
Chord("Am7").play()                     # acorde com strum
Scale("C", "major").play(duration=0.3)  # escala ascendente
Field("C", "major").play(duration=0.6)  # 7 acordes do campo

# Exportar WAV
Chord("Am7").to_wav("am7.wav", waveform="triangle")
```

No terminal:

```bash
gingo chord Am7 --play --waveform triangle
gingo scale "C major" --play
```

### 6. Progressões Harmônicas (beta)

!!! info "Beta"
    A classe Tree esta em fase beta. A API pode mudar em versoes futuras.

```python
from gingo import Tree, ScaleType, Progression

# Criar árvore harmônica (baseada na teoria de José de Alencar)
tree = Tree("C", ScaleType.Major, "harmonic_tree")

# Validar progressão
valido = tree.is_valid(["IIm", "V7", "I"])
print(valido)  # True - progressão II-V-I

# Usar Progression para análise cross-tradition
prog = Progression("C", "major")
match = prog.identify(["IIm", "V7", "I"])
print(match.schema)  # "descending"
```

**Progressão II-V-I (cadência harmônica fundamental do jazz):**
```mermaid
graph LR
    II[IIm<br/>Dm7<br/>Preparação] -->|Tensão| V[V7<br/>G7<br/>Dominante]
    V -->|Resolução| I[I<br/>CM<br/>Tônica]

    style II fill:#feca57,color:#000
    style V fill:#ee5a6f,color:#000
    style I fill:#1dd1a1,color:#000
```

---

## Características

**API Intuitiva**

Interface Python clara e consistente, seguindo convenções modernas de design de bibliotecas.

**Performance**

Core implementado em C++17 com lookup tables e cached computations para operações eficientes.

**Audio integrado**

Ouca qualquer objeto com `.play()` — notas, acordes, escalas, campos harmonicos.
Exporte para WAV com `.to_wav()`. Quatro formas de onda, envelope ADSR, strum e gap.
Zero dependencias para sintese; `simpleaudio` opcional para playback.

**Fundamentação Teórica**

Implementa conceitos de teoria musical acadêmica:

- **Teoria das Árvores Harmônicas** (José de Alencar)
- **Neo-Riemannian transformations** (Richard Cohn)
- **Voice leading** (Dmitri Tymoczko)
- **Interval vectors** (Allen Forte)
- **Dissonância psicoacústica** (Plomp & Levelt)

**Documentação Completa**

Referência detalhada com exemplos práticos, conceitos de teoria musical e casos de uso avançados.

---

## Para Quem É o Gingo

<div class="grid" markdown>

:material-guitar:{ .lg } **Músicos**
:   Análise de harmonias, transposição de escalas, identificação de acordes e validação de progressões.

:material-school:{ .lg } **Estudantes**
:   Estudo de teoria musical através de experimentação prática com código Python.

:material-code-tags:{ .lg } **Desenvolvedores**
:   Criação de ferramentas de análise musical, aplicações de ensino e software de composição assistida.

:material-piano:{ .lg } **Professores**
:   Demonstrações interativas de conceitos harmônicos e exercícios programáticos para alunos.

</div>

---

## Próximos Passos

<div class="grid cards" markdown>

-   :material-book-open-page-variant:{ .lg .middle } __Conceitos Básicos__

    ---

    Fundamentos de teoria musical e sistema de notação

    [:octicons-arrow-right-24: Ver Conceitos](guia/conceitos-basicos.md)

-   :material-rocket-launch:{ .lg .middle } __Primeiros Passos__

    ---

    Exemplos práticos de uso da biblioteca

    [:octicons-arrow-right-24: Começar](guia/primeiros-passos.md)

-   :material-school:{ .lg .middle } __Tutoriais__

    ---

    Guias detalhados de cada classe e conceito

    [:octicons-arrow-right-24: Ver Tutoriais](tutoriais/notas.md)

-   :material-api:{ .lg .middle } __Referência da API__

    ---

    Documentação completa de métodos e classes

    [:octicons-arrow-right-24: Ver API](api/referencia.md)

</div>

---

## Exemplo Avançado: Análise Contextual

```python
from gingo import Field, Chord, ScaleType

# Criar campo harmônico de Dó maior
campo = Field("C", ScaleType.Major)

# Comparar dois acordes dentro do contexto harmônico
resultado = campo.compare(Chord("CM"), Chord("GM"))

# Análise contextual (21 dimensões)
print(f"Grau de CM: {resultado.degree_a}")      # 1 (I)
print(f"Grau de GM: {resultado.degree_b}")      # 5 (V)
print(f"Função de CM: {resultado.function_a}")  # Tonic
print(f"Função de GM: {resultado.function_b}")  # Dominant
print(f"Movimento: {resultado.root_motion}")    # "ascending_fifth"
```

**Relação funcional I-V:**
```mermaid
flowchart LR
    A[CM<br/>Dó Maior<br/>I - Tônica] -->|Movimento de<br/>Quinta Ascendente| B[GM<br/>Sol Maior<br/>V - Dominante]

    style A fill:#1dd1a1,color:#000,stroke:#000,stroke-width:3px
    style B fill:#ee5a6f,color:#000,stroke:#000,stroke-width:3px
```

**Funções Harmônicas**

O Gingo classifica acordes em três funções principais:

- **Tônica (T)**: estabilidade e repouso (I, IIIm, VIm)
- **Subdominante (S)**: preparação e afastamento (IIm, IV)
- **Dominante (D)**: tensão e resolução (V, V7, VIIdim)

---

## Recursos Adicionais

- **GitHub**: [github.com/sauloverissimo/gingo](https://github.com/sauloverissimo/gingo)
- **Referências Bibliográficas**: [Teoria e fundamentos acadêmicos](referencias.md)
- **Exemplos Práticos**: Disponíveis em cada seção de tutorial
