# 📥 Instalação

Este guia orienta a instalação do Gingo em diferentes plataformas e ambientes.

## Requisitos do Sistema

```mermaid
graph TD
    A[Requisitos] --> B[Python 3.10+]
    A --> C[pip]
    A --> D[Terminal/Shell]

    B --> B1[Incluído na distribuição<br/>Python oficial]
    C --> C1[Gerenciador de pacotes<br/>Python padrão]
    D --> D1[Interface de linha<br/>de comando]

    style A fill:#FFD700,stroke:#FF8C00,stroke-width:4px,color:#000
    style B fill:#87CEEB,stroke:#4169E1,stroke-width:3px,color:#000
    style C fill:#98FB98,stroke:#32CD32,stroke-width:3px,color:#000
    style D fill:#FFB6C1,stroke:#FF1493,stroke-width:3px,color:#000
    style B1 fill:#E0FFE0,stroke:#228B22,stroke-width:2px,color:#000
    style C1 fill:#E0FFE0,stroke:#228B22,stroke-width:2px,color:#000
    style D1 fill:#E0FFE0,stroke:#228B22,stroke-width:2px,color:#000
```

**Sobre o terminal**

O terminal é uma interface textual para comunicação com o sistema operacional:

- **Windows**: Prompt de Comando ou PowerShell
- **macOS**: Terminal
- **Linux**: Terminal (shell padrão)

## Passo 1: Verificar Instalação do Python

### Windows

1. Pressione `Win + R`
2. Digite `cmd` e pressione `Enter`
3. Execute:

```bash
python --version
```

### macOS / Linux

1. Abra o Terminal
2. Execute:

```bash
python3 --version
```

### Interpretação dos Resultados

```mermaid
flowchart TD
    A[Comando executado] --> B{Saída?}

    B -->|Python 3.10 ou superior| C[Instalação válida]
    B -->|Python 3.9 ou inferior| D[Atualização necessária]
    B -->|Comando não reconhecido| E[Instalação necessária]

    C --> F[Prosseguir para Passo 2]
    D --> G[Baixar em<br/>python.org]
    E --> G

    style A fill:#87CEEB,stroke:#4169E1,stroke-width:3px,color:#000
    style B fill:#FFD700,stroke:#FF8C00,stroke-width:3px,color:#000
    style C fill:#98FB98,stroke:#32CD32,stroke-width:3px,color:#000
    style D fill:#FFE5B4,stroke:#FF8C00,stroke-width:3px,color:#000
    style E fill:#FFB6C1,stroke:#FF1493,stroke-width:3px,color:#000
    style F fill:#E0FFE0,stroke:#228B22,stroke-width:3px,color:#000
    style G fill:#87CEEB,stroke:#4169E1,stroke-width:2px,color:#000
```

**Saída esperada**

```
Python 3.12.0
```
Qualquer versão ≥ 3.10 é compatível.

**Instalação do Python**

Se o Python não estiver instalado:

1. Acesse [python.org/downloads](https://www.python.org/downloads/)
2. Baixe o instalador apropriado
3. Execute a instalação
4. **IMPORTANTE (Windows)**: Marque a opção "Add Python to PATH"

## Passo 2: Instalar o Gingo

```mermaid
sequenceDiagram
    participant U as Usuário
    participant T as Terminal
    participant P as pip
    participant G as Gingo

    U->>T: Abre terminal
    U->>T: pip install gingo
    T->>P: Solicita instalação
    P->>G: Baixa e instala
    G->>P: Instalação concluída
    P->>T: Confirmação
    T->>U: Sucesso
```

### Comando de Instalação

=== "Windows"

    ```bash
    pip install gingo
    ```

=== "macOS / Linux"

    ```bash
    pip3 install gingo
    ```

### Saída Esperada

Durante a instalação, você verá:

```
Collecting gingo
  Downloading gingo-1.0.0-cp312-cp312-win_amd64.whl (245 kB)
     ━━━━━━━━━━━━━━━━━━━━━━━━━━━━ 245.0/245.0 kB 5.2 MB/s
Installing collected packages: gingo
Successfully installed gingo-1.0.0
```

**Instalação bem-sucedida**

A mensagem `Successfully installed gingo-X.X.X` confirma a instalação.

**Erros de instalação**

Consulte a seção "Solução de Problemas" abaixo.

## Passo 3: Verificação da Instalação

### Verificação via CLI

Execute:

```bash
gingo --version
```

Saída esperada:

```
gingo 1.0.0
```

```mermaid
flowchart LR
    A[gingo --version] --> B{Resultado?}

    B -->|Versão exibida| C[Instalação válida]
    B -->|Comando não encontrado| D[Erro na instalação]

    C --> E[Prosseguir para teste Python]
    D --> F[Consultar troubleshooting]

    style A fill:#87CEEB,stroke:#4169E1,stroke-width:3px,color:#000
    style B fill:#FFD700,stroke:#FF8C00,stroke-width:3px,color:#000
    style C fill:#98FB98,stroke:#32CD32,stroke-width:3px,color:#000
    style D fill:#FFB6C1,stroke:#FF1493,stroke-width:3px,color:#000
    style E fill:#E0FFE0,stroke:#228B22,stroke-width:2px,color:#000
    style F fill:#FFE5B4,stroke:#FF8C00,stroke-width:2px,color:#000
```

### Verificação via Python

Entre no interpretador interativo:

```bash
python
```

Ou no macOS/Linux:

```bash
python3
```

Execute:

```python
from gingo import Note, Chord, Scale
print(Note("C"))
print(Chord("C"))
print(Scale("C", "major"))
```

Saída esperada:

```
C
C
C Major
```

Para sair do interpretador:

```python
exit()
```

```mermaid
graph TD
    A[Abrir interpretador] --> B[Importar gingo]
    B --> C{Sucesso?}

    C -->|Sim| D[Instalação verificada]
    C -->|ModuleNotFoundError| E[Erro de importação]

    D --> F[Pronto para uso]
    E --> G[Consultar troubleshooting]

    style A fill:#87CEEB,stroke:#4169E1,stroke-width:3px,color:#000
    style B fill:#DDA0DD,stroke:#9370DB,stroke-width:3px,color:#000
    style C fill:#FFD700,stroke:#FF8C00,stroke-width:3px,color:#000
    style D fill:#98FB98,stroke:#32CD32,stroke-width:3px,color:#000
    style E fill:#FFB6C1,stroke:#FF1493,stroke-width:3px,color:#000
    style F fill:#E0FFE0,stroke:#228B22,stroke-width:4px,color:#000
    style G fill:#FFE5B4,stroke:#FF8C00,stroke-width:2px,color:#000
```

**Instalação confirmada**

Se ambos os testes foram bem-sucedidos, o Gingo está operacional.

Próximo passo: **[Conceitos Básicos](conceitos-basicos.md)**

## 🔧 Solução de Problemas

### Problema 1: "pip não é reconhecido"

```mermaid
graph TD
    A[Erro: pip não reconhecido] --> B[Causa: Python não no PATH]
    B --> C[Solução 1:<br/>Reinstalar Python]
    B --> D[Solução 2:<br/>Usar python -m pip]

    C --> C1[Marcar 'Add to PATH'<br/>na instalação]
    D --> D2[python -m pip install gingo]

    style A fill:#FFB6C1,stroke:#FF1493,stroke-width:3px,color:#000
    style B fill:#FFE5B4,stroke:#FF8C00,stroke-width:3px,color:#000
    style C fill:#87CEEB,stroke:#4169E1,stroke-width:2px,color:#000
    style D fill:#98FB98,stroke:#32CD32,stroke-width:2px,color:#000
    style C1 fill:#E0FFE0,stroke:#228B22,stroke-width:2px,color:#000
    style D2 fill:#E0FFE0,stroke:#228B22,stroke-width:2px,color:#000
```

**Solução rápida**:

```bash
python -m pip install gingo
```

### Problema 2: "Permission denied" ou "Access denied"

Erro relacionado a permissões insuficientes.

**Solução**:

=== "Windows"

    Execute o terminal como Administrador:

    1. Localize "cmd" ou "PowerShell" no menu Iniciar
    2. Clique com botão direito
    3. Selecione "Executar como administrador"
    4. Execute novamente o comando de instalação

=== "macOS / Linux"

    Use `sudo` para elevar privilégios:

    ```bash
    sudo pip3 install gingo
    ```

    Digite sua senha quando solicitado (a entrada não é exibida visualmente).

### Problema 3: "No module named 'gingo'"

```mermaid
flowchart TD
    A[ModuleNotFoundError] --> B{Versão de Python?}

    B -->|python| C[Instalar com<br/>python -m pip]
    B -->|python3| D[Instalar com<br/>python3 -m pip]

    C --> E[Executar com<br/>python]
    D --> F[Executar com<br/>python3]

    E --> G[Problema resolvido]
    F --> G

    style A fill:#FFB6C1,stroke:#FF1493,stroke-width:3px,color:#000
    style B fill:#FFD700,stroke:#FF8C00,stroke-width:3px,color:#000
    style C fill:#87CEEB,stroke:#4169E1,stroke-width:2px,color:#000
    style D fill:#87CEEB,stroke:#4169E1,stroke-width:2px,color:#000
    style E fill:#DDA0DD,stroke:#9370DB,stroke-width:2px,color:#000
    style F fill:#DDA0DD,stroke:#9370DB,stroke-width:2px,color:#000
    style G fill:#98FB98,stroke:#32CD32,stroke-width:3px,color:#000
```

**Solução**: Consistência entre instalação e execução:

```bash
# Instalar
python -m pip install gingo

# Executar
python
>>> from gingo import Note
```

### Problema 4: Versão desatualizada do pip

Se aparecer aviso:

```bash
WARNING: You are using pip version 20.0.0; however, version 24.0 is available.
```

**Solução**: Atualizar o pip:

```bash
python -m pip install --upgrade pip
```

Após a atualização, instale o Gingo normalmente.

## 🌟 Instalação Avançada

Para desenvolvedores e casos de uso específicos.

### Instalação com Audio (playback)

```bash
pip install gingo[audio]
```

Instala `simpleaudio` para playback direto pelo alto-falante. A sintese de audio
e exportacao WAV funcionam sem dependencias extras — `gingo[audio]` e necessario
apenas para o `.play()` tocar som pelo sistema.

Sem `simpleaudio`, o Gingo tenta players do sistema (`aplay`/`paplay`/`ffplay`
no Linux, `afplay` no macOS).

### Instalação com Dependências de Teste

```bash
pip install gingo[test]
```

### Instalação Completa

```bash
pip install gingo[audio,test]
```

```mermaid
graph TD
    A[gingo] --> B[Instalação<br/>Padrão]
    A --> C[gingo dev]
    A --> D[gingo test]
    A --> E[gingo dev,test]

    B --> B1[Uso normal]
    C --> C1[Desenvolvimento]
    D --> D1[Testes]
    E --> E1[Completo]

    style A fill:#FFD700,stroke:#FF8C00,stroke-width:4px,color:#000
    style B fill:#98FB98,stroke:#32CD32,stroke-width:3px,color:#000
    style C fill:#87CEEB,stroke:#4169E1,stroke-width:2px,color:#000
    style D fill:#DDA0DD,stroke:#9370DB,stroke-width:2px,color:#000
    style E fill:#FFB6C1,stroke:#FF1493,stroke-width:2px,color:#000
```

## 🆘 Suporte Adicional

Se os problemas persistirem:

```mermaid
mindmap
  root((Suporte))
    GitHub Issues
      Descrever problema
      Incluir logs de erro
      Informar versões
    Documentação
      FAQ
      Exemplos
      API Reference
    Comunidade
      Fóruns Python
      Stack Overflow
      Reddit r/learnpython
```

**Informações para suporte**

Ao reportar problemas, inclua:

1. Sistema operacional e versão
2. Versão do Python (`python --version`)
3. Mensagem de erro completa
4. Comandos executados

## 🎉 Instalação Concluída

```mermaid
journey
    title Processo de Instalação
    section Preparação
      Verificar Python: 5: Usuário
      Abrir Terminal: 5: Usuário
    section Instalação
      pip install gingo: 5: pip
      Download: 3: pip
    section Verificação
      Testar CLI: 5: Usuário
      Testar importação: 5: Usuário
    section Resultado
      Instalação confirmada: 5: Sucesso
```

## Próximos Passos

Agora que o Gingo está instalado:

1. **[Conceitos Básicos](conceitos-basicos.md)** - Fundamentos de teoria musical
2. **[Primeiros Passos](primeiros-passos.md)** - Exemplos práticos de código
3. **[Começando](comecando.md)** - Casos de uso avançados

```mermaid
graph LR
    A[Instalação<br/>Completa] --> B[Conceitos<br/>Básicos]
    B --> C[Primeiros<br/>Passos]
    C --> D[Começando]
    D --> E[Domínio]

    style A fill:#98FB98,stroke:#32CD32,stroke-width:4px,color:#000
    style B fill:#87CEEB,stroke:#4169E1,stroke-width:3px,color:#000
    style C fill:#FFB6C1,stroke:#FF1493,stroke-width:3px,color:#000
    style D fill:#DDA0DD,stroke:#9370DB,stroke-width:3px,color:#000
    style E fill:#FFD700,stroke:#FF8C00,stroke-width:4px,color:#000
```

**Próxima etapa: Conceitos Básicos**

Continue para **[Conceitos Básicos](conceitos-basicos.md)** para compreender os fundamentos teóricos.

---

💡 **Dica**: Salve esta página para referência futura em caso de reinstalação ou troubleshooting.
