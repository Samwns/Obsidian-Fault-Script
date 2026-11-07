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

### `raster.ofs`
Rasterizador vetorial com antialiasing (acumulação de área assinalada). Caminhos (`move_to`, `line_to`, `quad_to`, `cubic_to`, `close`), formas (`add_round_rect`, `add_circle`, `add_capsule`, `add_arc_stroke`) e atalhos (`fill_round_rect`, `fill_circle`, `stroke_line`, `stroke_round_rect`). Compõe no Canvas respeitando alfa e clip.

### `ttf.ofs`
Fontes TrueType em OFS puro: `ttf.load(caminho)`, `ttf.draw(cv, fonte, tamanho, x, linha_base, texto, cor)`, `ttf.width`, `ttf.ascent`/`descent`/`line_height`, `ttf.ellipsize`, `ttf.draw_centered`. UTF-8, glifos compostos (acentos), cache e posicionamento subpixel. Texto em várias linhas: `ttf.wrap(fonte, tamanho, texto, largura_max)` quebra por palavras e devolve `Array<obsidian>` (respeita `\n`); `ttf.draw_wrapped(cv, fonte, tamanho, x, y, largura_max, altura_linha, texto, cor, max_chars)` desenha o texto quebrado a partir do topo `y` e devolve o número de linhas (`max_chars < 0` desenha tudo; útil para efeito de máquina de escrever). Outlines **CFF** (OpenType/PostScript, Type 2 charstrings com subrs locais/globais e fontes CID com FDArray/FDSelect, hints ignorados) além de `glyf`. **Coleções TTC**: `ttf.load_face(caminho, i)`, `ttf.load_face_named(caminho, "CJK JP")` (procura no nome da face), `ttf.face_count`. **Fallback por glifo**: `ttf.set_fallback(fonte)` ou `ttf.set_fallback_file(caminho, nome_da_face)` (carrega só quando faltar o primeiro glifo); `draw`/`width`/`wrap`/`ellipsize` usam o fallback quando a fonte não tem o caractere; `ttf.has_glyph(fonte, cp)`. O cache de glifos rasterizados recomeça ao passar de `ttf.pool_limit` pixels (`ttf.clear_cache()`). Ex.: Roboto + `/system/fonts/NotoSansCJK-Regular.ttc` no Android para japonês e setas.

### `x11win.ofs`
Janela X11 nativa: `open`, `present(canvas)`, `poll()` → `EV_KEY`/`EV_MOUSE_DOWN`/`EV_MOUSE_UP`/`EV_MOUSE_MOVE`/`EV_WHEEL`/`EV_RESIZE`/`EV_CLOSE`, campos `ev_x`, `ev_y`, `ev_button`, `ev_key` (keysym), `ev_text` (UTF-8, com teclas mortas pt-BR), `ev_mods`; `set_cursor`, `set_icon`, `set_title`, `now_ms`, `sleep_ms`.

### `icons.ofs`
Ícones vetoriais 24×24: `icons.draw(cv, icons.BACK, x, y, tamanho, cor)` e `icons.draw_centered`. Constantes `BACK`, `FORWARD`, `RELOAD`, `HOME`, `PLUS`, `CLOSE`, `LOCK`, `INFO`, `STAR`, `MENU`, `SEARCH`, `GLOBE`, `DOWNLOAD`, `HISTORY`, `SETTINGS`, `PROFILE`, `STOP`, `FILE`, `BOOKMARK`, `SHIELD`, `PLUGIN`, `SAVE`, `FOLDER`, `PLAY`, `HAMBURGER`, `GAMEPAD`; `icons.by_name("plugins")` aceita nomes e apelidos (`import`, `load`, `config`...). Ícones de interface adicionados para o OAL e o Moon: `AUTO`, `SKIP`, `BACKLOG`, `EYE`, `EYE_OFF`, `ROTATE`, `PORTRAIT`, `LANDSCAPE`, `TRASH`, `EDIT`, `IMAGE`, `VOLUME`, `TEXT`, `PALETTE`, `CHECK`, `STORAGE`, `KEYBOARD`, `BACKSPACE`, `SHIFT`, `PAUSE`, `JOYSTICK`, `SPACE`, `CHEVRON`, `RESIZE`, `TABS`. Ícones de **motor/engine** de jogo: `WINDOWS` (janela de programa de PC), `RENPY` (balão de fala: visual novel), `UNITY` (cubo isométrico: motor 3D), `MOON`, `UNKNOWN`. `icons.by_name` aceita também nomes em português (`lixeira`, `retrato`, `paisagem`, `teclado`, `joystick`...).

### `android.ofs`
Backend Android em OFS puro (NativeActivity, aarch64, API 26+): exporta `ANativeActivity_onCreate`, liga os callbacks com `fault_addr`, entrada por `AInputQueue_attachLooper`, frames por `AChoreographer`. O programa define `android_event(ev)`. Funções: `lock`/`unlock_post`/`fill_rect`/`blit` (ANativeWindow RGBA), `request_frame`, `finish`, `set_orientation` (JNI), `internal_data_path`, `external_data_path`, `log`/`log_tag`. Ver [ANDROID.md](ANDROID.md).

### `oal_runtime.ofs`, `oal_android.ofs`, `oal_x11.ofs`
Runtime da OAL: widgets de toque (barra, botão, ícone, lista, grade, cartão, `fab`, D-pad), estilos com estado pressionado e opacidade, rolagem, navegação, retrato/paisagem, desenhados em `canvas`/`raster`/`ttf`/`icons` sobre o núcleo de layout do `oll.ofs`. `oal_android.ofs` é o backend Android; `oal_x11.ofs` (janela de celular no desktop) é experimental. Ver [OAL_REFERENCE.md](OAL_REFERENCE.md).

### `wav.ofs`
Decodificador WAV (RIFF) em OFS puro, sem dependência de plataforma: `wav.load(caminho)` (`-1` se falhar), `wav.decode(ptr, tamanho)`, `wav.pcm(s)` (ponteiro para amostras i16 intercaladas), `wav.frames`, `wav.channels`, `wav.rate`, `wav.free`. Suporta PCM 8/16 bits e Microsoft ADPCM. `wav.from_pcm(ptr, quadros, canais, hz)` registra um PCM i16 já na memória.

### `audio_android.ofs`
Saída de som com AAudio (Android 8+): `audio.open()` (44,1 kHz, estéreo, i16), `audio.play(canal, som, loop)`, `stop`, `stop_all`, `set_volume(canal, 0-100)`, `set_master`, `playing`, `pause`/`resume`/`close` e `audio.pump()` uma vez por quadro. O mixer roda na thread de áudio e só usa memória crua; a CLI linka `-laaudio` com `ofs build --target android`. Ver [ANDROID.md](ANDROID.md). `audio.position(canal)`: quadros já tocados do som do canal (relógio para sincronizar vídeo).

### `avi.ofs`
Leitor AVI (RIFF/OpenDML) e decodificador **Microsoft Video 1** (CRAM/MSVC, 16 bits) em OFS puro: `avi.open(caminho)`, `codec` (FourCC), `supported`, `width`/`height`/`frames`/`us_per_frame`, `avi.decode(v, quadro, dst)` (0xRRGGBB por pixel; sequencial, quadros delta), `audio_format`/`audio_channels`/`audio_rate`/`audio_bits`, `read_audio` (PCM de todos os blocos `##wb`) + `audio_bytes`, `close`. Cada quadro é lido do disco na hora. Saída igual bit a bit à do ffmpeg. Outros codecs (Xvid/MPEG-4, Indeo) não são decodificados.

### `hashmap.ofs`
Tabela hash inteiro→inteiro: `hashmap.new(cap)`, `set`, `get(m, chave, padrao)`, `has`, `count`, `clear`.

### `tls.ofs`
Conexões HTTPS usando o OpenSSL do sistema: `tls.connect(fd, host)` (SNI + verificação de certificado e nome), `tls.send`, `tls.recv_all` (`tls.last_len`), `tls.close`, `tls.last_error`. Linkar com `OFS_LINK_LIBS="-lssl -lcrypto"`.

### `png.ofs`
Decodificador PNG em OFS puro (inflate/zlib próprio): `png.load(caminho)`, `png.width`, `png.height`, `png.draw(cv, img, x, y, w, h)`, `png.draw_cover`, `png.draw_region`. Pixels em `0xAARRGGBB`.

### `zip.ofs`
Ler e criar `.zip` em OFS puro (inflate do `png.ofs`): `zip.open(caminho)`, `count`, `name(i)`, `size(i)`, `is_dir(i)`, `find(nome)`, `read(i)` (`zip.data_ptr`/`data_len`, `free_data`), `read_text(nome)`, `extract(i, destino)`, `extract_all(pasta)` (recusa `..` e caminhos absolutos; confere o CRC), `close`; escrita sem compressão: `create`, `add_file`, `add_text`, `add_bytes`, `finish`. Sem zip64/criptografia. `png.inflate_raw` expõe o deflate cru.
**Extração em blocos** (não trava a interface): `zip.xbegin(i, destino, hash)` abre a entrada; chame `zip.xstep(bytes)` ou `zip.xstep_ms(ms)` (um por quadro) até devolver `1` (gravado, CRC conferido, `zip.x_sha` com o sha256 se `hash`) ou `-1` (`zip.last_error`; o arquivo parcial é apagado); `zip.xpct()` dá 0..100 e `zip.xabort()` cancela. O inflate retoma de onde parou (`png.inflate_begin/inflate_step/inflate_take/inflate_abort`) e tem estado próprio, então `png.load` pode rodar entre os passos.

### `sha256.ofs`
`sha256.file(caminho)` e `sha256.bytes(ptr, n)` (64 hex minúsculos; blocos de 64 KB), `sha256.crc32_update(crc, ptr, n)`.

### `android_http.ofs`, `android_webview.ofs`
HTTP/HTTPS por `HttpURLConnection` via JNI e WebView em tela cheia num `Dialog`, sem Java nosso. Ver [ANDROID.md](ANDROID.md). `icons.ofs` ganhou `RPGM` e `TYRANO`; `oal_runtime.ofs` ganhou `colorpicker`, `set_palette_full`, `hsv_to_rgb`.

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
