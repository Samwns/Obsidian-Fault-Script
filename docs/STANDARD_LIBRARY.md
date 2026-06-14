# Biblioteca Padrao OFS

A biblioteca padrao fica em `ofs/stdlib/`. Bibliotecas instaladas sao importadas por nome:

```ofs
attach {math}
attach {string}
```

Durante desenvolvimento, um arquivo local pode ser importado por caminho:

```ofs
attach {F:../libs/minha_lib.ofs}
```

## `math.ofs`

Funcoes numericas escritas em OFS:

- `square(x)`: retorna `x * x`;
- `cube(x)`: retorna `x * x * x`;
- `sum_range(start, end)`: soma o intervalo inclusivo;
- `is_prime(n)`: testa primalidade por divisores impares;
- `count_digits(n)`: conta digitos decimais;
- `sum_digits(n)`: soma digitos decimais;
- `reverse_number(n)`: inverte os digitos;
- `is_palindrome_number(n)`: compara o numero com seu reverso.

```ofs
attach {math}

core main() {
    echo(square(12))
    echo(is_prime(97))
}
```

## `string.ofs`

Helpers de texto sobre os builtins do runtime:

- `repeat_str(text, count)`: repete uma string;
- `starts_with_char(text, code)`: compara o primeiro byte;
- `is_empty(text)`: verifica comprimento zero.

O runtime tambem fornece `ofs_str_len`, `ofs_str_char_at`, `ofs_str_substr`, `ofs_str_concat`, `ofs_str_eq`, `ofs_str_contains`, conversao para maiusculas/minusculas e conversoes numericas.

## `io.ofs`

- `prompt(message)`: imprime a mensagem e le uma linha;
- `print_separator(character, count)`: imprime uma linha repetida;
- `print_header(title)`: imprime um cabecalho de terminal.

## `terminal_colors.ofs`

Define sequencias ANSI e helpers de impressao colorida. Deve ser usado em terminais compativeis; a cor nao altera o conteudo retornado pelo programa.

## `odl.ofs`

API tipada para gerar documentos:

- baixo nivel: `attr`, `tag`, `void_tag`, `text`;
- documento: `document`, `meta`, `css`, `icon`, `script`, `script_module`;
- estrutura: `div`, `section`, `article`, `nav`, `header`, `main`, `footer`;
- conteudo: `a`, `img`, `p`, `h1`, `h2`, `h3`, `strong`, `span`;
- listas/formularios: `ul`, `ol`, `li`, `form`, `label`, `input`, `button`;
- codigo/tabelas: `code_block`, `table`, `table_id`.

## `oes.ofs`

API para gerar a camada visual:

- `prop(name, value)`: declaracao;
- `rule(selector, body)`: regra;
- `media`, `supports`, `layer`: grupos condicionais;
- `font_face`: fonte;
- `keyframes`, `frame`: animacao;
- `var`, `use`: tokens;
- `transition`, `animation`: helpers de movimento.

## `webserver.ofs`

Define:

- constantes de metodo HTTP;
- status 200, 201, 400, 404 e 500;
- MIME JSON, HTML, texto, CSS, JavaScript, ODL e OES;
- `status_text`;
- `http_response`;
- `json_string` e `json_number`;
- `get_mime_type`;
- `log_request`;
- `html_page`, `not_found_page`, `server_error_page`;
- `serve_once`, `serve_forever`, `serve_html_once`, `serve_html_forever`.

O servidor atual entrega uma resposta preparada. Roteamento dinamico, TLS e limites de conexao ainda nao formam um framework de producao completo.

## `webui.ofs`

Define `WebTheme` e:

- `theme_dark`, `theme_light`;
- `stylesheet`;
- `page`;
- `nav`, `hero`, `button`;
- `panel`, `grid`, `card`, `stack`;
- `serve`.

Componentes retornam `obsidian`, entao podem ser compostos por concatenacao ou por funcoes OFS.

## `canvas.ofs`, `window.ofs`, `ui.ofs`

Formam a UI nativa experimental:

- canvas: desenho 2D;
- window: ciclo de vida de janela;
- ui: texto, medidas, caixas e controles imediatos.

O suporte depende do runtime e da plataforma.

## `bedrock.ofs`

Primitivos de regioes, ponteiros e memoria explicita. Nao e necessario para programas OFS comuns.

## `bedrock_packet.ofs`

Estruturas experimentais para organizar dados de pacote e protocolos binarios.

## `memory_modes.ofs`

Experimentos dos modos de memoria e das fronteiras de codigo baixo nivel.

## `rift.ofs`

Superficie experimental de FFI/ABI C. Use quando uma funcao externa precisa ser declarada e chamada pela OFS.

## `ofshtml.ofs`

Helpers HTML antigos mantidos para compatibilidade. Projetos web novos devem preferir ODL e OES.

## `test_lib.ofs`

Funcoes auxiliares usadas em exemplos e testes da linguagem.
