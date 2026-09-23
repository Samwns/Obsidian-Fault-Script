# Referência da Biblioteca Padrão do OFS

Os módulos da biblioteca padrão do OFS estão localizados no diretório `ofs/stdlib/`. Módulos instalados no sistema são importados pelo nome com a diretiva `attach`:

```ofs
attach {math}
attach {string}
attach {io}
```

Durante o desenvolvimento ou em projetos locais, um arquivo específico pode ser importado via caminho relativo ou absoluto utilizando o prefixo `F:`:

```ofs
attach {F:../modulos/meu_modulo.ofs}
```

---

## 1. Módulos Fundamentais

### `math.ofs`

Operações e funções matemáticas implementadas em OFS puro:

- `square(x)`: Retorna o quadrado do número (`x * x`).
- `cube(x)`: Retorna o cubo do número (`x * x * x`).
- `sum_range(inicio, fim)`: Calcula o somatório do intervalo inclusivo de inteiros.
- `is_prime(n)`: Testa primalidade através da verificação de divisores ímpares.
- `count_digits(n)`: Retorna a quantidade de dígitos do número inteiro.
- `sum_digits(n)`: Retorna a soma dos dígitos decimais do número.
- `reverse_number(n)`: Inverte a ordem dos dígitos decimais.
- `is_palindrome_number(n)`: Verifica se o número é idêntico ao seu reverso.

```ofs
attach {math}

core main() {
    echo(square(12))
    echo(is_prime(97))
}
```

### `string.ofs`

Rotinas auxiliares para manipulação de strings sobre as primitivas do runtime:

- `repeat_str(texto, vezes)`: Retorna a repetição da string pelo número de vezes indicado.
- `starts_with_char(texto, codigo_char)`: Verifica se o primeiro caractere da string corresponde ao código ASCII fornecido.
- `is_empty(texto)`: Retorna verdadeiro caso a string possua comprimento zero.

O runtime nativo disponibiliza adicionalmente as rotinas `ofs_str_len`, `ofs_str_char_at`, `ofs_str_substr`, `ofs_str_concat`, `ofs_str_eq`, `ofs_str_contains` e conversões numéricas.

### `io.ofs`

Entrada e saída no terminal:

- `prompt(mensagem)`: Imprime a mensagem informada e realiza a leitura de uma linha da entrada padrão (stdin).
- `print_separator(caractere, tamanho)`: Imprime uma linha repetida com o caractere delimitador.
- `print_header(titulo)`: Imprime um cabeçalho formatado no terminal.

### `terminal_colors.ofs`

Declaração de constantes de controle ANSI e utilitários para formatação de texto colorido em emuladores de terminal compatíveis.

---

## 2. Módulos da Stack Web

### `odl.ofs`

API programática em OFS para construção e geração de árvores de documentos ODL/HTML:

- **Nível estrutural**: `document`, `meta`, `css`, `icon`, `script`, `script_module`.
- **Contêineres semânticos**: `div`, `section`, `article`, `nav`, `header`, `main`, `footer`.
- **Tipografia e conteúdo**: `a`, `img`, `p`, `h1`, `h2`, `h3`, `strong`, `span`.
- **Formulários e listas**: `ul`, `ol`, `li`, `form`, `label`, `input`, `button`.
- **Tabelas e blocos de código**: `table`, `table_id`, `code_block`.
- **Primitivas de baixo nível**: `tag`, `void_tag`, `attr`, `text`.

### `oes.ofs`

API programática para criação de folhas de estilo e regras OES/CSS:

- `prop(nome, valor)`: Emissão de propriedades individuais de estilo.
- `rule(seletor, corpo)`: Declaração de blocos seletores com propriedades.
- `media(consulta, corpo)` / `supports(consulta, corpo)` / `layer(nome, corpo)`: Agrupamentos condicionais e camadas.
- `keyframes(nome, frames)` / `frame(etapa, corpo)`: Definição de linhas do tempo de animação.
- `var(nome, valor)` / `use(nome)`: Declaração e uso de tokens visuais.
- `transition(...)` / `animation(...)`: Auxiliares de movimento.

### `webserver.ofs`

Servidor HTTP de alto desempenho e utilitários de protocolo:

- Constantes de métodos HTTP (`GET`, `POST`, `PUT`, `DELETE`).
- Definições de códigos de status HTTP (200, 201, 400, 404, 500) e textos de status (`status_text`).
- Detecção e mapeamento de tipos MIME (`get_mime_type`) para HTML, CSS, JS, JSON, texto, ODL e OES.
- Construção de respostas HTTP (`http_response`) com cabeçalhos apropriados.
- Emissão de fragmentos JSON estruturados (`json_string`, `json_number`).
- Registro estruturado de requisições (`log_request`).
- Laços de execução: `serve_once`, `serve_forever`, `serve_html_once`, `serve_html_forever`.

### `webui.ofs`

Biblioteca de componentes para interfaces web construída sobre ODL e OES:

- Temas padrão: `theme_dark()`, `theme_light()`.
- Componentes de layout: `page`, `nav`, `hero`, `panel`, `grid`, `card`, `stack`.
- Controles: `button`.
- Servidor rápido: `serve`.

---

## 3. Módulos Gráficos Nativos

### `canvas.ofs`

API para renderização gráfica 2D direta em buffer de pixels raster:

- Criação e liberação de superfícies raster (`canvas.create`, `canvas.destroy`).
- Limpeza de superfície (`canvas.clear`).
- Desenho de primitivas: pixels (`set_pixel`), retângulos sólidos e vazados, linhas e texto bitmap.
- Exportação de imagens para formato PPM binário sem dependências externas.

### `window.ofs`

Gerenciamento de janelas nativas do sistema operacional:

- Criação e destruição de janelas (`window.create`, `window.destroy`).
- Tratamento do loop de eventos e mensagens (`window.poll`).
- Integração nativa com X11 no Linux e Win32 no Windows.

### `ui.ofs`

Controles de interface de usuário em modo imediato:

- Estruturas de caixas delimitadoras (`UiRect`).
- Estilização temânica (`UiStyle`, `ui.dark()`, `ui.light()`).
- Controles básicos: botões, rótulos e medição tipográfica.

---

## 4. Módulos de Baixo Nível e Sistema

### `bedrock.ofs`

Primitivas de gerenciamento explícito de memória:

- Alocação e desalocação manual de blocos de memória e células (`bedrock_cell_new`, `bedrock_cell_drop`).
- Operações de leitura e escrita direta em endereços de memória.
- Criação e reciclagem de regiões e arenas de alocação.

### `bedrock_packet.ofs`

Estruturas utilitárias para montagem e parsing de pacotes de dados binários em protocolos de rede.

### `rift.ofs`

Camada para declaração e chamada de funções externas via Foreign Function Interface (FFI) com convenção C ABI.

### `memory_modes.ofs`

Declaração de interfaces experimentais para controle de padrões de acesso a buffers e alocações de sistema.

---

## 5. Módulos de Apoio e Compatibilidade

### `test_lib.ofs`

Módulo auxiliar para validação de testes unitários e exemplos da linguagem.

### `ofshtml.ofs`

Módulo legado mantido para compatibilidade reversa com códigos que utilizavam geradores HTML anteriores ao ODL. Projetos novos devem utilizar `odl.ofs`.
