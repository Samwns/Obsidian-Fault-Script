# Guia de Introdução ao OFS

O OFS (Obsidian Fault Script) é uma linguagem de programação compilada, self-hosted, que gera LLVM IR e produz executáveis binários nativos vinculados a um runtime nativo.

O ecossistema inclui compilador, biblioteca padrão, ferramentas de linha de comando e as linguagens de domínio web ODL e OES.

---

## 1. Instalação

### Pacotes pré-compilados

Instaladores oficiais estão disponíveis para diversas distribuições e sistemas operacionais:

- **Debian / Ubuntu**:
  ```bash
  sudo dpkg -i ofs-debian-ubuntu-x64-installer-v*.deb
  ```
- **Fedora / RHEL / openSUSE**:
  ```bash
  sudo rpm -Uvh ofs-fedora-x64-installer-v*.rpm
  ```
- **Arch / Manjaro**:
  ```bash
  sudo pacman -U ofs-arch-x64-installer-v*.pkg.tar.zst
  ```
- **macOS (Apple Silicon / ARM64)**:
  Execute o script instalador fornecido no release do GitHub.
- **Windows (x64)**:
  Execute o instalador `ofs-windows-x64-installer-v*.exe`.
- **VS Code**:
  Instale a extensão oficial `ofs-vscode-extension.vsix` para suporte a sintaxe, realce e recarregamento dinâmico (*Go Live*).

### Compilação a partir do código-fonte (Bootstrap)

Para compilar o compilador diretamente a partir do repositório:

```bash
git clone https://github.com/Samwns/Obsidian-Fault-Script.git
cd Obsidian-Fault-Script
bash ofs/bootstrap/scripts/bootstrap-minimal.sh
```

O binário do compilador será gerado em `ofs/dist/ofs` (ou `ofs/dist/ofscc`).

---

## 2. Comandos da Linha de Comando (CLI)

O executável `ofs` expõe os seguintes subcomandos:

| Comando | Descrição |
|---|---|
| `ofs build <arquivo.ofs> -o <binário>` | Compila o arquivo para um executável binário nativo. |
| `ofs check <arquivo.ofs>` | Valida a sintaxe e a tipagem sem emitir código. |
| `ofs run <arquivo.ofs>` | Compila em diretório temporário e executa imediatamente. |
| `ofs tokens <arquivo.ofs>` | Imprime o fluxo de tokens gerado pela análise léxica. |
| `ofs ast <arquivo.ofs>` | Exibe a árvore sintática abstrata (AST) do programa. |
| `ofs ir <arquivo.ofs>` | Emite o código intermediário LLVM IR (`.ll`). |
| `ofs asm <arquivo.ofs>` | Emite o assembly nativo da arquitetura alvo. |
| `ofs odl <arquivo.odl> -o <saída.html>` | Compila um documento ODL para HTML5. |
| `ofs oes <arquivo.oes> -o <saída.css>` | Compila regras de estilo OES para CSS. |

---

## 3. Primeiro Programa

Crie um arquivo chamado `hello.ofs`:

```ofs
core main() {
    echo("Olá, Mundo via OFS!")
}
```

Compilação e execução:

```bash
ofs build hello.ofs -o hello
./hello
```

Saída:
```text
Olá, Mundo via OFS!
```

---

## 4. Tipos de Dados

OFS é estaticamente tipado. O compilador oferece inferência de tipos em declarações de variáveis locais (`forge`).

### Tipos fundamentais

- `stone`: Inteiro de 64 bits com sinal.
- `crystal`: Ponto flutuante de 64 bits (IEEE 754).
- `obsidian`: Sequência de caracteres UTF-8 (*string*).
- `bool`: Booleano (`true` ou `false`).
- `void`: Ausência de valor de retorno.

### Inteiros de largura fixa

- Sem sinal: `u8`, `u16`, `u32`, `u64`
- Com sinal: `i8`, `i32`

Exemplo de declaração e conversão explícita:

```ofs
core main() {
    forge contador: stone = 10
    forge taxa: crystal = 3.14159
    forge ativo: bool = true
    forge nome: obsidian = "Obsidian"

    forge canal_r: u8 = 255
    forge canal_g: u8 = 128
    forge pixel: u32 = (canal_r as u32 << 16) | (canal_g as u32 << 8)

    echo(nome)
    echo(pixel)
}
```

---

## 5. Funções

Funções são definidas com a palavra-chave `vein`. O ponto de entrada nativo do programa é a função `core main()`.

```ofs
vein somar(a: stone, b: stone) -> stone {
    return a + b
}

vein fatorial(n: stone) -> stone {
    if (n <= 1) {
        return 1
    }
    return n * fatorial(n - 1)
}

core main() {
    forge s = somar(15, 27)
    forge f = fatorial(5)
    echo(s)
    echo(f)
}
```

Funções de ordem superior e expressões anônimas (*lambdas*):

```ofs
vein aplicar(x: stone, operacao: vein(stone) -> stone) -> stone {
    return operacao(x)
}

core main() {
    forge dobro = vein(n: stone) -> stone {
        return n * 2
    }
    echo(aplicar(21, dobro))
}
```

---

## 6. Estruturas de Dados (`monolith`)

OFS utiliza a palavra-chave `monolith` para definir tipos compostos e blocos `impl` para associar métodos:

```ofs
monolith Retangulo {
    largura: stone
    altura: stone
}

impl Retangulo {
    vein area(self) -> stone {
        return self.largura * self.altura
    }

    vein perimetro(self) -> stone {
        return (self.largura + self.altura) * 2
    }
}

core main() {
    forge r: Retangulo
    r.largura = 10
    r.altura = 5

    echo(r.area())
    echo(r.perimetro())
}
```

---

## 7. Módulos e Importações (`attach`)

A declaração `attach` importa módulos da biblioteca padrão instalada ou arquivos locais:

```ofs
attach {math}
attach {string}
attach {io}
```

Para importar arquivos locais relativos ou absolutos, utilize o prefixo `F:`:

```ofs
attach {F:./modulos/auxiliar.ofs}
```

---

## 8. Controle de Fluxo

### Condicionais (`if` / `else`)

```ofs
if (x > 0) {
    echo("positivo")
} else if (x < 0) {
    echo("negativo")
} else {
    echo("zero")
}
```

### Laços de repetição (`while`)

```ofs
forge i: stone = 0
while (i < 5) {
    echo(i)
    i = i + 1
}
```

### Seleção múltipla (`match`)

```ofs
match status_code {
    case 200: { echo("Sucesso") }
    case 404: { echo("Não encontrado") }
    case 500: { echo("Erro interno") }
    default:  { echo("Desconhecido") }
}
```

### Tratamento de exceções (`tremor` / `catch` / `throw`)

```ofs
tremor {
    if (divisor == 0) {
        throw "Divisão por zero inválida"
    }
    echo(dividendo / divisor)
} catch (erro: obsidian) {
    echo("Exceção capturada: " + erro)
}
```

---

## 9. Interoperabilidade Nativa com C (`rift` / `extern vein`)

É possível declarar e chamar símbolos C externos diretamente:

```ofs
extern vein puts(s: obsidian) -> stone
rift vein strlen(s: obsidian) -> stone bind "strlen" abi c

core main() {
    puts("Mensagem enviada via puts do C")
}
```

---

## 10. Stack Web: ODL e OES

O ecossistema OFS possui duas linguagens de domínio específico (DSLs) dedicadas à construção de interfaces web estáticas e dinâmicas:

### ODL (Obsidian Document Language)

Estrutura semântica para documentos web compilada para HTML5:

```odl
page "Aplicação OFS" "pt"
skin "estilo.css"

flow
  deck.navegacao
    title1 "Título Principal"
    link.botao "Documentação" "#docs"
  stage#conteudo
    band.artigo
      mark "## Conteúdo nativo\nTexto formatado em **Markdown**."
      raw "<div class=\"custom\">HTML bruto quando necessário</div>"
  spark "app.js"
```

Compilação do ODL:
```bash
ofs odl documento.odl -o public/index.html
```

### OES (Obsidian Effect Scripts)

Definição de estilos, variáveis, regras responsivas e animações compilada para CSS:

```oes
gem primario "#5b34ea"
gem fundo "#120e24"

paint .botao
  fill "$primario"
  curve "6px"
  space "8px 16px"
  edge "none"

veil "(max-width: 768px)"
  paint .grade
    grid-cols "1fr"

pulse "surgir"
  rise
    opacity "0"
  rest
    opacity "1"
```

Compilação do OES:
```bash
ofs oes estilo.oes -o public/estilo.css
```

### Servidor HTTP nativo (`webserver`)

A biblioteca padrão fornece um servidor HTTP nativo integrado ao runtime:

```ofs
attach {webserver}

core main() {
    forge html = "<h1>Servidor Nativo OFS</h1><p>Executando sem dependências externas.</p>"
    webserver.serve_html_forever(8080, html)
}
```

---

## 11. Recursos de Baixo Nível

OFS suporta manipulação direta de memória e contextos específicos quando necessário:

- `fracture { ... }`: Bloco com contexto de ponteiros tipados.
- `abyss { ... }`: Fronteira explícita para operações de baixo nível de máquina.
- `bedrock`: Biblioteca padrão para alocações explícitas de células e regiões de memória.

```ofs
attach {bedrock}

core main() {
    forge celula = bedrock_cell_new(42)
    echo(bedrock_cell_read(celula))
    bedrock_cell_drop(celula)
}
```

---

## 12. Próximos Passos

- Consulte a [Referência da Linguagem](LANGUAGE_REFERENCE.md) para detalhes exaustivos da gramática e dos tipos.
- Consulte a [Referência ODL](ODL_REFERENCE.md) e a [Referência OES](OES_REFERENCE.md) para desenvolvimento web.
- Consulte a documentação da [Biblioteca Padrão](STANDARD_LIBRARY.md).
