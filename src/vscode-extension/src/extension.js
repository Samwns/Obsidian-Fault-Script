const vscode = require('vscode');
const cp = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');
const https = require('https');
const http = require('http');

const KEYWORDS = [
  'core', 'vein', 'forge', 'const', 'monolith', 'impl', 'namespace', 'strata', 'extern', 'rift',
  'if', 'else', 'while', 'cycle', 'mine', 'match', 'assay', 'case', 'default',
  'return', 'break', 'shatter', 'continue', 'erode', 'throw', 'tremor', 'catch',
  'anchor', 'seal', 'fracture', 'abyss', 'bedrock', 'tectonic', 'attach',
  'bind', 'abi', 'echo', 'in', 'shard',
  'u8', 'u16', 'u32', 'u64', 'i8', 'i16', 'i32',
  'true', 'false', 'null', 'as'
];

const TYPES = ['stone', 'crystal', 'obsidian', 'bool', 'void', 'Array', 'u8', 'u16', 'u32', 'u64', 'i8', 'i16', 'i32'];

const MODULES = [
  'core', 'math', 'string', 'io', 'oes', 'ofshtml', 'odl', 'webserver', 'webui', 'bedrock', 'bedrock-packet',
  'terminal-colors', 'memory-modes', 'rift', 'canvas', 'window', 'test-lib',
  'fs', 'net', 'net-windows', 'http', 'oll', 'oll-x11', 'ui', 'raster', 'ttf', 'x11win', 'icons', 'hashmap', 'strbuf',
  'android', 'android-http', 'android-input', 'android-webview', 'audio-android', 'avi',
  'oal-android', 'oal-runtime', 'oal-x11', 'png', 'sha256', 'tls', 'wav', 'zip'
];

const BUILTINS = [
  'echo', 'ofs_stone_to_obsidian', 'ofs_crystal_to_obsidian', 'ofs_read_line',
  'ofs_str_len', 'ofs_str_char_at', 'ofs_str_substr', 'ofs_str_eq', 'ofs_str_contains',
  'ofs_str_upper', 'ofs_str_lower', 'ofs_pow', 'ofs_sqrt'
];

const LOW_LEVEL_INTRINSICS = [
  'fault_count', 'fault_lead', 'fault_trail', 'fault_swap',
  'fault_spin_left', 'fault_spin_right', 'fault_step',
  'fault_cut', 'fault_patch', 'fault_weave',
  'fault_fence', 'fault_prefetch', 'fault_trap',
  'fault_unreachable', 'fault_memcpy', 'fault_memset', 'fault_addr', 'fault_call'
];

const STDLIB_COMPLETIONS = [
  'abs', 'max', 'min', 'clamp', 'is_even', 'is_odd', 'factorial', 'fibonacci',
  'square', 'cube', 'sum_range', 'is_prime',
  'repeat_str', 'starts_with_char', 'is_empty',
  'prompt', 'print_separator', 'print_header',
  'status_text', 'http_response', 'not_found_response', 'error_response',
  'serve_once', 'serve_forever', 'serve_html_once', 'serve_html_forever',
  'webui.page', 'webui.nav', 'webui.hero', 'webui.button', 'webui.panel',
  'webui.grid', 'webui.card', 'webui.stack', 'webui.serve',
  'echo_red', 'echo_green', 'echo_yellow', 'echo_blue', 'echo_purple', 'echo_cyan',
  'explain_memory_modes', 'memory_mode_mid_name',
  'rift_text_size', 'rift_line', 'rift_banner', 'rift_report_line',
  'bedrock_cell_new', 'bedrock_cell_read', 'bedrock_cell_write', 'bedrock_cell_add',
  'bedrock_cell_sub', 'bedrock_cell_drop', 'bedrock_region_new', 'bedrock_region_at',
  'bedrock_region_read', 'bedrock_region_write', 'bedrock_region_drop',
  'bedrock_prefetch', 'bedrock_field_cut', 'bedrock_field_patch',
  'bedrock_lane8_get', 'bedrock_lane8_set', 'bedrock_lane16_le_get', 'bedrock_lane16_be_get',
  'bedrock_packet_store', 'bedrock_packet_header', 'bedrock_packet_payload',
  'bedrock_packet_opcode', 'bedrock_packet_lane', 'bedrock_packet_opcode_be16',
  'canvas.create', 'canvas.destroy', 'canvas.clear', 'canvas.set_pixel',
  'canvas.get_pixel', 'canvas.fill_rect', 'canvas.present',
  'window.create', 'window.destroy', 'window.poll', 'window.is_open',
  'window.width', 'window.height', 'window.set_title',
  'input.mouse_x', 'input.mouse_y', 'input.is_down', 'input.key',
  'ofs_str_char_at_known',
  'raster.begin', 'raster.move_to', 'raster.line_to', 'raster.quad_to', 'raster.cubic_to',
  'raster.close', 'raster.fill', 'raster.fill_round_rect', 'raster.fill_circle',
  'raster.stroke_line', 'raster.stroke_round_rect', 'raster.add_round_rect', 'raster.add_circle',
  'raster.add_capsule', 'raster.add_arc_stroke',
  'ttf.load', 'ttf.draw', 'ttf.width', 'ttf.ascent', 'ttf.descent', 'ttf.line_height',
  'ttf.ellipsize', 'ttf.draw_centered', 'ttf.glyph_index',
  'x11win.open', 'x11win.close', 'x11win.is_open', 'x11win.present', 'x11win.poll',
  'x11win.width', 'x11win.height', 'x11win.set_title', 'x11win.set_cursor', 'x11win.set_icon',
  'x11win.now_ms', 'x11win.sleep_ms', 'x11win.mouse_x', 'x11win.mouse_y',
  'icons.draw', 'icons.draw_centered',
  'strbuf.create', 'strbuf.append', 'strbuf.append_char', 'strbuf.append_int',
  'strbuf.to_obsidian', 'strbuf.length', 'strbuf.clear', 'strbuf.release',
  'hashmap.new', 'hashmap.set', 'hashmap.get', 'hashmap.has', 'hashmap.count', 'hashmap.clear'
];

const HOVER_DOCS = {
  core: {
    description: 'Ponto de entrada do programa.',
    context: 'Todo programa executável tem um `core main()`; a execução começa nele.',
    example: 'core main() {\n    echo("hello")\n}'
  },
  vein: {
    description: 'Declara uma função.',
    context: 'Parâmetros levam tipo (`nome: tipo`) e o retorno vem após `->`. Também aparece em `extern vein` e `rift vein`, sem corpo.',
    example: 'vein add(a: stone, b: stone) -> stone {\n    return a + b\n}'
  },
  forge: {
    description: 'Declara uma variável (tipada ou com tipo inferido).',
    context: 'Use para variáveis locais ou globais mutáveis.',
    example: 'forge x: stone = 10\nforge y = 20'
  },
  const: {
    description: 'Declara uma variável imutável.',
    context: 'Use para valores que não devem mudar durante a execução.',
    example: 'const max: stone = 100'
  },
  monolith: {
    description: 'Declara uma estrutura (tipo composto) com campos nomeados.',
    context: 'Os métodos entram em um bloco `impl`. Campos são acessados com `.`.',
    example: 'monolith Rect {\n    w: stone\n    h: stone\n}\n\ncore main() {\n    forge r: Rect\n    r.w = 3\n    echo(r.w)\n}'
  },
  strata: {
    description: 'Declara uma enumeração de variantes.',
    context: 'Use junto com `?` para propagar falhas: as variantes `Crack` e `Barren` contam como falha.',
    example: 'strata Status { Idle, Running, Failed }'
  },
  impl: {
    description: 'Declara os métodos de um monolith.',
    context: 'O primeiro parâmetro `self: Tipo` dá acesso aos campos; chame com `obj.metodo()`.',
    example: 'monolith Rect {\n    w: stone\n    h: stone\n}\n\nimpl Rect {\n    vein area(self: Rect) -> stone {\n        return self.w * self.h\n    }\n}\n\ncore main() {\n    forge r: Rect\n    r.w = 3\n    r.h = 4\n    echo(r.area())\n}'
  },
  namespace: {
    description: 'Agrupa símbolos em um escopo nomeado.',
    context: 'Use para organizar APIs e evitar colisão de nomes; acesse com `nome.simbolo`.',
    example: 'namespace mathx {\n    vein square(x: stone) -> stone {\n        return x * x\n    }\n}\n\ncore main() {\n    echo(mathx.square(5))\n}'
  },
  if: {
    description: 'Execução condicional.',
    context: 'A condição vai entre parênteses; `else if` encadeia ramos.',
    example: 'if (x > 0) {\n    echo("positivo")\n} else if (x == 0) {\n    echo("zero")\n} else {\n    echo("negativo")\n}'
  },
  else: {
    description: 'Ramo alternativo de um `if`.',
    context: 'Pode ser seguido de outro `if` (`else if`) ou de um bloco.',
    example: 'if (x > 0) {\n    echo("sim")\n} else {\n    echo("nao")\n}'
  },
  cycle: {
    description: 'Laço: forma de 3 partes ou por coleção (`item in lista`).',
    context: '`mine` é o sinônimo temático de `cycle`. Use `shatter`/`break` para sair e `erode`/`continue` para pular a iteração.',
    example: 'forge nums: Array<stone> = [1, 2, 3]\ncycle (n in nums) {\n    echo(n)\n}\ncycle (forge i = 0; i < 3; i++) {\n    echo(i)\n}'
  },
  mine: {
    description: 'Laço por coleção: `mine (item in lista)` (nome temático de `cycle`).',
    context: 'Só aceita a forma `item in iteravel`; para o laço de 3 partes use `cycle`.',
    example: 'forge nums: Array<stone> = [1, 2, 3]\nmine (n in nums) {\n    echo(n)\n}'
  },
  while: {
    description: 'Laço controlado por condição.',
    context: 'Use quando o número de repetições depende de estado dinâmico.',
    example: 'forge x = 3\nwhile (x > 0) {\n    x -= 1\n}'
  },
  break: {
    description: 'Sai do laço atual (`cycle`, `mine` ou `while`).',
    context: '`shatter` é o sinônimo temático.',
    example: 'cycle (forge i = 0; i < 10; i++) {\n    if (i == 3) { break }\n    echo(i)\n}'
  },
  shatter: {
    description: 'Sai do laço atual; sinônimo de `break`.',
    context: 'Roda os blocos `seal`/`anchor` pendentes antes de sair.',
    example: 'cycle (forge i = 0; i < 10; i++) {\n    if (i == 3) { shatter }\n    echo(i)\n}'
  },
  continue: {
    description: 'Pula para a próxima iteração do laço.',
    context: '`erode` é o sinônimo temático.',
    example: 'cycle (forge i = 0; i < 5; i++) {\n    if (i == 2) { continue }\n    echo(i)\n}'
  },
  erode: {
    description: 'Pula para a próxima iteração do laço; sinônimo de `continue`.',
    context: 'Roda os blocos `seal`/`anchor` pendentes do corpo antes de continuar.',
    example: 'cycle (forge i = 0; i < 5; i++) {\n    if (i == 2) { erode }\n    echo(i)\n}'
  },
  return: {
    description: 'Encerra a função, opcionalmente devolvendo um valor.',
    context: 'Os blocos `seal`/`anchor` pendentes rodam antes de sair.',
    example: 'vein double(n: stone) -> stone {\n    return n * 2\n}'
  },
  match: {
    description: 'Casamento de padrão: compara o valor com cada caso usando `==`.',
    context: 'Cada braço é `valor => { ... }`; `_` é o caso padrão. `assay` é o sinônimo temático.',
    example: 'forge code = 200\nmatch (code) {\n    200 => { echo("ok") }\n    404 => { echo("nao encontrado") }\n    _ => { echo("outro") }\n}'
  },
  assay: {
    description: 'Casamento de padrão; sinônimo de `match`.',
    context: 'Mesma sintaxe: braços `valor => { ... }` e `_` como caso padrão.',
    example: 'forge code = 404\nassay (code) {\n    200 => { echo("ok") }\n    _ => { echo("outro") }\n}'
  },
  tremor: {
    description: 'Bloco de tratamento de erro, usado com `catch`.',
    context: 'O que for lançado por `throw` dentro do bloco chega ao `catch (e: obsidian)`.',
    example: 'tremor {\n    throw "boom"\n} catch (e: obsidian) {\n    echo(e)\n}'
  },
  catch: {
    description: 'Tratador de um bloco `tremor`.',
    context: 'Recebe o valor lançado em uma variável, opcionalmente tipada.',
    example: 'tremor {\n    throw "falhou"\n} catch (e: obsidian) {\n    echo(e)\n}'
  },
  throw: {
    description: 'Lança um erro com uma mensagem.',
    context: 'Dentro de `tremor` é capturado pelo `catch`; fora dele exibe a mensagem e encerra com código 1. Não é permitido dentro de `seal`/`anchor` (E0042).',
    example: 'throw "valor invalido"'
  },
  anchor: {
    description: 'Registra uma ação que roda quando o escopo atual termina (defer).',
    context: 'Roda em qualquer saída (fim do bloco, `return`, `break`/`continue`, `?`), em ordem inversa de registro. `return`, `break`, `continue` e `throw` dentro dele dão erro E0042. `seal` é o mesmo recurso.',
    example: 'vein run() -> stone {\n    anchor { echo("fim") }\n    echo("trabalhando")\n    return 0\n}'
  },
  seal: {
    description: 'Sinônimo de `anchor`: sela o recurso ao sair do escopo (defer).',
    context: 'Aceita um bloco ou um comando só. Várias ações rodam em ordem inversa. `return`/`break`/`continue`/`throw` dentro dele dão erro E0042.',
    example: 'vein run() -> stone {\n    seal { echo("fechado") }\n    echo("usando o recurso")\n    return 0\n}'
  },
  attach: {
    description: 'Anexa um módulo OFS (stdlib, pacote instalado ou arquivo).',
    context: '`attach {nome}` procura na stdlib e em `OFS_LIB_PATH`; `attach {F:caminho.ofs}` anexa um arquivo.',
    example: 'attach {strbuf}\n// ou um arquivo específico:\n// attach {F:helpers.ofs}'
  },
  extern: {
    description: 'Declara, sem corpo, um símbolo nativo externo (libc, libm, X11, JNI do Android).',
    context: 'É uma fronteira de chamada: o símbolo é resolvido no link e chamado pela convenção de chamada do sistema (ABI C). Não existe corpo OFS para ele. `rift vein` é a forma temática e aceita `bind` e `abi`.',
    example: 'extern vein puts(s: obsidian) -> stone\n\ncore main() {\n    puts("ola")\n}'
  },
  rift: {
    description: 'Declara, sem corpo, um símbolo nativo externo (fronteira de chamada), como `extern`.',
    context: 'Aceita `bind "simbolo"` para ligar outro nome ao símbolo nativo e `abi c` para indicar a convenção de chamada do sistema. Use para libc, libm, X11 e JNI.',
    example: 'rift vein text_size(s: obsidian) -> stone bind "strlen" abi c\n\ncore main() {\n    echo(text_size("fault"))\n}'
  },
  abi: {
    description: 'Metadado de `rift vein`/`extern vein`: convenção de chamada do símbolo nativo.',
    context: 'Aparece depois do tipo de retorno (e de `bind`, se houver). `c` indica a convenção de chamada do sistema (ABI C).',
    example: 'rift vein text_size(s: obsidian) -> stone bind "strlen" abi c'
  },
  bind: {
    description: 'Metadado de `rift vein`/`extern vein`: nome do símbolo nativo.',
    context: 'Liga um nome OFS a um símbolo nativo com outro nome.',
    example: 'rift vein text_size(s: obsidian) -> stone bind "strlen" abi c'
  },
  as: {
    description: 'Conversão explícita de tipo.',
    context: 'Entre inteiros (trunca/estende), `stone`↔`crystal`, ponteiros e `stone`.',
    example: 'forge x = 7\nforge y: crystal = x as crystal'
  },
  in: {
    description: 'Separa o item da coleção em `cycle`/`mine`.',
    context: 'Use em `cycle (item in lista)`.',
    example: 'forge nums: Array<stone> = [1, 2]\ncycle (n in nums) {\n    echo(n)\n}'
  },
  echo: {
    description: 'Imprime um valor seguido de quebra de linha.',
    context: 'Aceita `stone`, `crystal`, `obsidian` e `bool`.',
    example: 'echo("Hello, World!")\necho(42)\necho(3.14)'
  },
  shard: {
    description: 'Declara uma variável ponteiro tipada.',
    context: 'Ponteiros só existem dentro de `fracture`, `bedrock` ou `abyss`.',
    example: 'forge x = 1\nfracture {\n    shard p: *stone = &x\n    *p = *p + 1\n}\necho(x)'
  },
  fracture: {
    description: 'Bloco para código de baixo nível com ponteiros (`&x`, `*p`, `shard`).',
    context: 'Hoje é um bloco comum que marca a região de baixo nível; os intrínsecos `fault_*` e ponteiros só valem dentro de `fracture`, `bedrock` ou `abyss`.',
    example: 'forge x = 1\nfracture {\n    shard p: *stone = &x\n    *p = 42\n}\necho(x)'
  },
  abyss: {
    description: 'Bloco de baixo nível sem rede de proteção (ponteiros e `fault_*`).',
    context: 'Libera tudo de baixo nível (inclusive `fault_call`, `fault_addr`, `quartz`); dentro de um `bedrock`, o que escapa da arena deixa de ser erro (risco do programador). Prefira `fracture` ou `bedrock` quando possível.',
    example: 'forge x = 1\nabyss {\n    shard p: *stone = &x\n    *p = 7\n}'
  },
  bedrock: {
    description: 'Bloco de baixo nível tipado (ponteiros e intrínsecos `fault_*`) e, quando aloca, arena (A5.3).',
    context: 'O que o próprio bloco aloca (texto, `[...]`, `monolith`, `ofs_alloc`) sai de uma vez na saída do bloco, em qualquer saída. Texto que sai do bloco é copiado; `Array`/`monolith`/ponteiro que escapa é `[ERRO DE ARENA]` (dentro de `abyss` vale). Veins chamadas de dentro alocam no heap normal. Também é o nome da biblioteca `attach {bedrock}` (células e regiões).',
    example: 'while (i < n) {\n    bedrock {\n        forge s = "k" + ofs_stone_to_obsidian(i)\n        forge buf: *stone = ofs_alloc(1024)\n        ultimo = s   // texto sai copiado\n    }               // a arena sai aqui\n    i++\n}'
  },
  geode: {
    description: 'Vein sem nome que captura por valor as variáveis de fora (closure): `geode (x: T) -> R { ... }`.',
    context: 'O valor tem o tipo `vein(T) -> R`: guarda, passa, devolve e chama como qualquer valor de vein. A captura é copiada quando o geode nasce (texto e números; `Array`/`monolith` são o mesmo objeto, como argumento). Atribuir a uma captura dentro do geode é `[ERRO DE GEODE]`. Sem captura vai para o C como callback; com captura não.',
    example: 'vein somador(base: stone) -> vein(stone) -> stone {\n    return geode (x: stone) -> stone { return x + base }\n}\n\ncore main() {\n    forge s7 = somador(7)\n    echo(s7(1))   // 8\n}'
  },
  tectonic: {
    description: 'Prefixo opcional de um bloco de baixo nível: `tectonic fracture|abyss|bedrock { ... }`.',
    context: 'O compilador aceita as três formas como blocos equivalentes.',
    example: 'tectonic bedrock {\n    fault_fence()\n}'
  },
  layout: {
    description: 'Palavra reservada; o suporte a `monolith ... layout` ainda não existe.',
    context: 'Todo `monolith` usa o layout nativo. Evite usar `layout` como nome.',
    example: 'monolith Header {\n    tag: stone\n    flags: stone\n}'
  },
  intent: {
    description: 'Palavra reservada; a anotação `intent` ainda não é aceita pelo parser.',
    context: 'Evite usar `intent` como nome.',
    example: 'vein square(x: stone) -> stone {\n    return x * x\n}'
  },
  strbuf: {
    description: 'Biblioteca de buffer de texto que cresce (`attach {strbuf}`).',
    context: 'Anexar é O(1) amortizado. API: `create`, `append`, `append_char`, `append_int`, `to_obsidian` (cópia), `length`, `clear`, `release`.',
    example: 'attach {strbuf}\n\ncore main() {\n    forge b: StrBuf = strbuf.create(64)\n    strbuf.append(b, "total: ")\n    strbuf.append_int(b, 42)\n    echo(strbuf.to_obsidian(b))\n    strbuf.release(b)\n}'
  },
  fault_count: {
    description: 'Intrínseco: contagem de bits 1 (popcount), em 64 bits.',
    context: 'Dentro de `bedrock`, `fracture` ou `abyss`. `fault_count(-1)` vale 64.',
    example: 'bedrock {\n    echo(fault_count(0xF0F0))\n}'
  },
  fault_lead: {
    description: 'Intrínseco: conta zeros à esquerda (64 bits).',
    context: 'Dentro de `bedrock`, `fracture` ou `abyss`. `fault_lead(0)` vale 64.',
    example: 'bedrock {\n    echo(fault_lead(1))\n}'
  },
  fault_trail: {
    description: 'Intrínseco: conta zeros à direita (64 bits).',
    context: 'Dentro de `bedrock`, `fracture` ou `abyss`. `fault_trail(0)` vale 64.',
    example: 'bedrock {\n    echo(fault_trail(8))\n}'
  },
  fault_spin_left: {
    description: 'Intrínseco: rotação de bits para a esquerda (64 bits).',
    context: 'A distância é tomada módulo 64.',
    example: 'bedrock {\n    echo(fault_spin_left(1, 3))\n}'
  },
  fault_spin_right: {
    description: 'Intrínseco: rotação de bits para a direita (64 bits).',
    context: 'A distância é tomada módulo 64.',
    example: 'bedrock {\n    echo(fault_spin_right(8, 3))\n}'
  },
  fault_weave: {
    description: 'Intrínseco: mistura bits de dois valores conforme uma máscara.',
    context: '`fault_weave(mask, left, right)` = `(left & mask) | (right & ~mask)`.',
    example: 'bedrock {\n    echo(fault_weave(0xFF00, 0xAAAA, 0x5555))\n}'
  },
  fault_swap: {
    description: 'Intrínseco: inverte a ordem dos bytes (byte swap, 64 bits).',
    context: 'Dentro de `bedrock`, `fracture` ou `abyss`.',
    example: 'bedrock {\n    echo(fault_swap(0x0102))\n}'
  },
  fault_cut: {
    description: 'Intrínseco: extrai um campo de bits.',
    context: '`fault_cut(valor, deslocamento, largura)`. Útil em cabeçalhos e layouts compactados.',
    example: 'bedrock {\n    forge opcode: stone = fault_cut(0xAB00, 8, 8)\n    echo(opcode)\n}'
  },
  fault_patch: {
    description: 'Intrínseco: substitui um campo de bits.',
    context: '`fault_patch(base, deslocamento, largura, novo)` devolve a base com o campo trocado.',
    example: 'bedrock {\n    forge patched: stone = fault_patch(0, 8, 8, 0x2A)\n    echo(patched)\n}'
  },
  fault_memcpy: {
    description: 'Intrínseco: copia `len` bytes de `src` para `dst` (`llvm.memcpy`).',
    context: '`dst` e `src` são ponteiros.',
    example: 'forge a = 5\nforge b = 0\nbedrock {\n    shard pa: *stone = &a\n    shard pb: *stone = &b\n    fault_memcpy(pb, pa, 8)\n}\necho(b)'
  },
  fault_memset: {
    description: 'Intrínseco: preenche `len` bytes de `dst` com o byte `val` (`llvm.memset`).',
    context: '`dst` é um ponteiro.',
    example: 'forge a = 5\nbedrock {\n    shard pa: *stone = &a\n    fault_memset(pa, 0, 8)\n}\necho(a)'
  },
  fault_unreachable: {
    description: 'Intrínseco: marca o caminho como inalcançável.',
    context: 'Gera `trap` + `unreachable`; se executado, encerra o processo com SIGILL.',
    example: 'bedrock {\n    if (1 > 2) { fault_unreachable() }\n}'
  },
  fault_trap: {
    description: 'Intrínseco: encerra o processo com uma trap de máquina (SIGILL).',
    context: 'Use para abortar em estado impossível.',
    example: 'bedrock {\n    if (1 > 2) { fault_trap() }\n}'
  },
  fault_fence: {
    description: 'Intrínseco: barreira de memória.',
    context: 'No interpretador (`ofs run`) não faz nada.',
    example: 'bedrock {\n    fault_fence()\n}'
  },
  fault_prefetch: {
    description: 'Intrínseco: dica de pré-busca (prefetch) do alvo de um ponteiro.',
    context: 'No interpretador (`ofs run`) não faz nada.',
    example: 'forge x = 1\nbedrock {\n    shard p: *stone = &x\n    fault_prefetch(p)\n}'
  },
  fault_addr: {
    description: 'Intrínseco: endereço de uma `vein` como `stone`.',
    context: 'Aceita veins do módulo e `extern`/`rift vein`. Só dentro de uma vein. Combine com `fault_call` ou passe a símbolos nativos que esperam callbacks.',
    example: 'vein inc(n: stone) -> stone {\n    return n + 1\n}\n\ncore main() {\n    bedrock {\n        forge f: stone = fault_addr(inc)\n        echo(fault_call(f, 41))\n    }\n}'
  },
  fault_call: {
    description: 'Intrínseco: chamada indireta por endereço (`fault_call(endereco, args...)`).',
    context: 'Segue a convenção de chamada do sistema (ABI C): argumentos inteiros, ponteiros e `obsidian`; o retorno é lido como `stone`.',
    example: 'vein inc(n: stone) -> stone {\n    return n + 1\n}\n\ncore main() {\n    bedrock {\n        echo(fault_call(fault_addr(inc), 41))\n    }\n}'
  },
  fault_step: {
    description: 'Intrínseco: avança um ponteiro tipado por N elementos.',
    context: 'Soma `N * tamanho do tipo apontado`, como aritmética de ponteiros.',
    example: 'forge x = 1\nbedrock {\n    shard p: *stone = &x\n    shard q: *stone = fault_step(p, 0)\n    echo(*q)\n}'
  },
  mineral: {
    description: 'Tipo com regra (A5.1): `mineral Nome = Base grain regra`.',
    context: 'É compatível com o tipo base: base → mineral confere a `grain` (forge, atribuição, parâmetro, retorno); mineral → base é livre. Literal que viola a regra é `[ERRO DE CONTRATO]` na compilação.',
    example: 'mineral UserId = stone grain value > 0\nmineral Email = obsidian grain contains("@")'
  },
  grain: {
    description: 'Regra de um `mineral`; `value` é o valor conferido.',
    context: 'Várias `grain` podem ser somadas. Forma curta: uma chamada sem `value` recebe `value` como 1º argumento (`contains("@")` = `ofs_str_contains(value, "@")`).',
    example: 'mineral Nota = stone grain value >= 0 grain value <= 10'
  },
  bearing: {
    description: 'Pré-condição da vein (quem chama garante), conferida na entrada.',
    context: 'Violação: `ofs: [CONTRATO VIOLADO arquivo:l:c]` no stderr e código 1 (`ofs run` e `ofs build`); `ofs build --release` (OFS_CONTRACTS=0) tira a checagem. Com argumentos literais a violação é erro de compilação. `ofs assay` testa os limites.',
    example: 'vein dividir(a: stone, b: stone) -> stone\n    bearing b != 0\n{\n    return a / b\n}'
  },
  settle: {
    description: 'Pós-condição da vein, conferida em cada `return`; `result` é o valor devolvido.',
    context: 'Os parâmetros valem como na entrada. Mesma regra de execução do `bearing` (stderr + código 1; `--release` desliga).',
    example: 'vein metade(n: stone) -> stone\n    bearing n >= 0\n    settle result * 2 <= n\n{\n    return n / 2\n}'
  },
  result: {
    description: 'No `settle`: o valor que a vein devolve.',
    context: 'Só vale no `settle` (no `bearing` é `[ERRO DE CONTRATO]`); fora dos contratos é um nome comum.',
    example: 'vein dobro(x: stone) -> stone settle result == x * 2 {\n    return x + x\n}'
  },
  stratum: {
    description: 'Estrato do arquivo (Ideia 4, segurança gradual): `stratum crust`, `stratum fault` (padrão) ou `stratum abyss`, no topo do arquivo.',
    context: '`crust` = nenhum baixo nível (nem bloco `fracture`/`abyss`, nem `extern`, ponteiro ou `quartz`; `bedrock` vale só como arena): `[ERRO DE ESTRATO]`. `fault` = regra da F4 (baixo nível só em bloco) + `claim`. `abyss` = baixo nível em qualquer lugar e vale como `claim` de tudo. O `ofs check` e o `ofs doc` mostram o estrato de cada módulo.',
    example: 'stratum crust\n\nvein media(xs: Array<stone>) -> stone {\n    ...\n}'
  },
  claim: {
    description: 'Permissões que o módulo pede (A5.4), no topo do arquivo: `claim rift` (extern/rift vein), `claim abyss` (bloco abyss), `claim quartz` (assembly), `claim magma` (`magma.cast`: compilar e carregar código em execução).',
    context: 'Uso sem claim: `[AVISO DE PERMISSAO]` até 2027-03-16, depois erro; `OFS_STRICT_FAULT=1` já é `[ERRO DE PERMISSAO]`. `extern` de símbolo do runtime (`ofs_*`) não pede claim. Quem anexa o módulo vê o claim no `ofs check` e no `ofs doc`. Forma de comentário `//! claim rift` para código que compila com uma semente anterior.',
    example: 'claim rift, abyss\n\nextern vein getenv(name: obsidian) -> obsidian'
  },
  magma: {
    description: '`magma.cast` (Ideia 5, `attach {magma}`): compila e carrega um mod OFS com o programa rodando, no `ofs run` e no nativo. `magma.cast(fonte_ou_caminho)` → handle; `magma.cooled(m)`/`magma.crack(m)` (erro do compilador como valor); `magma.vent(m, "nome")` com o tipo de destino → valor de vein (assinatura conferida, `null` se não bate); `magma.veins`, `magma.release`, `magma.caprock`.',
    context: 'Quem chama `magma.cast` declara `claim magma` (sempre erro sem ele; `stratum crust` nunca). O mod é compilado pelo `ofscc` num processo à parte e vira `.so` com o clang (cache por hash do IR em `~/.cache/ofs/magma`); usa o runtime do programa (o `ofs build` liga com `--export-dynamic`). O mod não vê os globais do programa: conversa por argumentos, retornos e valores de vein.',
    example: 'attach {magma}\nclaim magma\n\ncore main() {\n    forge m = magma.cast("mods/forja.ofs")\n    if (!magma.cooled(m)) { echo(magma.crack(m)) }\n    forge forjar: vein(stone) -> stone = magma.vent(m, "forjar")\n    if (forjar != null) { echo(forjar(10)) }\n}'
  },
  caprock: {
    description: 'Teto das permissões dos módulos anexados (A5.4), só no programa principal: `caprock rift, abyss` ou `caprock crust` (nada).',
    context: 'Um módulo anexado que pede (ou usa, ou é `stratum abyss`) algo fora do caprock é `[ERRO DE PERMISSAO]`. O runtime (`stdlib/runtime`) fica fora da regra.',
    example: 'caprock rift\nattach {io}\nattach {F:plugin.ofs}'
  },
  value: {
    description: 'Na `grain` de um `mineral`: o valor conferido.',
    context: 'Fora da `grain` é um nome comum.',
    example: 'mineral Porta = stone grain value > 0 && value < 65536'
  },
  quartz: {
    description: 'Assembly inline com efeito colateral; exige `abyss`.',
    context: 'AT&T por padrão; entradas `in "r" valor`, saída `-> destino "=r"` e clobbers `melt "memory"`. `ofs run` carrega a vein como código nativo.',
    example: 'abyss {\n    quartz { "nop" }\n}'
  },
  u8: {
    description: 'Inteiro sem sinal de 8 bits.',
    context: 'Use para canais de cor, bytes e buffers compactos.',
    example: 'forge r: u8 = 255'
  },
  u16: {
    description: 'Inteiro sem sinal de 16 bits.',
    context: 'Use para campos compactos maiores que um byte.',
    example: 'forge size: u16 = 1024'
  },
  u32: {
    description: 'Inteiro sem sinal de 32 bits.',
    context: 'Use para pixels RGBA e mascaras.',
    example: 'forge pixel: u32 = 0xFF0000'
  },
  u64: {
    description: 'Inteiro sem sinal de 64 bits.',
    context: 'Use para largura de 64 bits sem sinal.',
    example: 'forge flags: u64 = 1'
  },
  i8: {
    description: 'Inteiro com sinal de 8 bits.',
    context: 'Use para dados pequenos com sinal.',
    example: 'forge delta: i8 = -1'
  },
  i32: {
    description: 'Inteiro com sinal de 32 bits.',
    context: 'Use para inteiros compactos com sinal.',
    example: 'forge acc: i32 = -42'
  },
  run: {
    description: 'Comando CLI para compilar e executar imediatamente.',
    context: 'Melhor para ciclo rapido de desenvolvimento e testes locais.',
    example: 'ofs run main.ofs'
  },
  build: {
    description: 'Comando CLI para gerar executavel nativo.',
    context: 'Use para distribuicao, release e execucao sem compilador no alvo.',
    example: 'ofs build main.ofs -o app'
  },
  check: {
    description: 'Comando CLI para validar tipos/semantica sem gerar binario.',
    context: 'Use em validacao rapida, CI e antes de executar build/run.',
    example: 'ofs check main.ofs'
  },
  tokens: {
    description: 'Comando CLI de debug lexico (stream de tokens).',
    context: 'Use quando precisar diagnosticar tokenizacao e lexer.',
    example: 'ofs tokens main.ofs'
  },
  ast: {
    description: 'Comando CLI de debug sintatico (arvore AST).',
    context: 'Use para investigar parsing e estrutura do codigo.',
    example: 'ofs ast main.ofs'
  },
  ir: {
    description: 'Comando CLI para emitir LLVM IR.',
    context: 'Use para debug de codegen e analise de otimizacao.',
    example: 'ofs ir main.ofs'
  },
  rift_text_size: {
    description: 'Wrapper de stdlib para tamanho de texto via rift.',
    context: 'Use `attach {rift}` para consumir fronteiras externas com API OFS.',
    example: 'attach {rift}\n\ncore main() {\n    echo(rift_text_size("fault"))\n}'
  },
  window: {
    description: 'Modulo nativo de janela e input da OFS.',
    context: 'Use `attach {window}` para criar janela, ler input e apresentar um buffer.',
    example: 'attach {window}\nwindow.create("demo", 640, 480)'
  },
  raster: {
    description: 'Rasterizador vetorial com antialiasing (100% OFS).',
    context: 'Use `attach {raster}` para caminhos, retangulos arredondados, circulos e tracos suaves sobre um Canvas.',
    example: 'attach {canvas}\nattach {raster}\n\ncore main() {\n    forge cv = canvas.create(160, 60)\n    raster.fill_round_rect(cv, 10.0, 10.0, 120.0, 40.0, 12.0, 3355494)\n}'
  },
  ttf: {
    description: 'Fontes TrueType com UTF-8, acentos e cache de glifos (100% OFS).',
    context: 'Use `attach {ttf}` para desenhar texto real (Inter, Noto, DejaVu) em um Canvas.',
    example: 'attach {canvas}\nattach {ttf}\n\ncore main() {\n    forge cv = canvas.create(320, 48)\n    forge f = ttf.load("/usr/share/fonts/rsms-inter-fonts/Inter-Regular.ttf")\n    ttf.draw(cv, f, 14, 10, 30, "Ola, acao!", 15132390)\n}'
  },
  x11win: {
    description: 'Janela X11 nativa com teclado (UTF-8, teclas mortas), mouse, roda e resize.',
    context: 'Use `attach {x11win}` para abrir uma janela real e apresentar um Canvas.',
    example: 'attach {canvas}\nattach {x11win}\n\ncore main() {\n    forge cv = canvas.create(800, 600)\n    x11win.open("App", 800, 600)\n    while (x11win.is_open()) {\n        forge ev = x11win.poll()\n        x11win.present(cv)\n        x11win.sleep_ms(16)\n    }\n}'
  },
  icons: {
    description: 'Icones vetoriais de interface (grade 24x24).',
    context: 'Use `attach {icons}` com constantes como icons.BACK, icons.RELOAD, icons.HOME.',
    example: 'attach {canvas}\nattach {icons}\n\ncore main() {\n    forge cv = canvas.create(24, 24)\n    icons.draw(cv, icons.RELOAD, 2, 2, 20, 13421772)\n}'
  },
  hashmap: {
    description: 'Tabela hash inteiro -> inteiro com crescimento automatico.',
    context: 'Use `attach {hashmap}` para caches e indices rapidos.',
    example: 'attach {hashmap}\n\ncore main() {\n    forge m = hashmap.new(64)\n    hashmap.set(m, 42, 7)\n    echo(hashmap.get(m, 42, -1))\n}'
  },
  canvas: {
    description: 'Modulo nativo/instavel de canvas da OFS.',
    context: 'Use `attach {canvas}` para trabalhar com buffer de pixels junto com `window`.',
    example: 'attach {canvas}\nforge cv = canvas.create(64, 48)\ncanvas.set_pixel(cv, 1, 1, 0xFF0000)'
  },
  bedrock_region_new: {
    description: 'Cria uma regiao low-level de stones zerada.',
    context: 'Use para buffers/tabelas de tamanho fixo geridos em OFS low-level.',
    example: 'attach {bedrock}\n\ncore main() {\n    forge region = bedrock_region_new(4)\n    bedrock_region_write(region, 0, 10)\n    echo(bedrock_region_read(region, 0))\n    bedrock_region_drop(region)\n}'
  },
  bedrock_prefetch: {
    description: 'Helper OFS para solicitar prefetch de uma regiao bedrock.',
    context: 'Use antes de ler slots quentes em regioes low-level sem expor chamadas de intrinsic em toda a base.',
    example: 'attach {bedrock}\n\ncore main() {\n    forge region = bedrock_region_new(4)\n    bedrock_prefetch(region, 0)\n    bedrock_region_drop(region)\n}'
  },
  bedrock_view_read: {
    description: 'Le um valor a partir de uma view bedrock sobre uma regiao.',
    context: 'Use para janelas/slices de tabelas low-level sem sair da API OFS.',
    example: 'attach {bedrock}\n\ncore main() {\n    forge region = bedrock_region_new(4)\n    bedrock_region_write(region, 1, 42)\n    echo(bedrock_view_read(region, 1, 0))\n    bedrock_region_drop(region)\n}'
  },
  bedrock_lane8_get: {
    description: 'Helper OFS para extrair um lane de 8 bits.',
    context: 'Use em headers e layouts compactados sem escrever shifts diretamente.',
    example: 'attach {bedrock}\n\ncore main() {\n    forge header: stone = 0x2A01\n    echo(bedrock_lane8_get(header, 0))\n}'
  },
  bedrock_lane16_le_get: {
    description: 'Helper OFS para extrair uma janela little-endian de 16 bits.',
    context: 'Use em tipos ABI-facing e headers quando o layout local segue ordem little-endian.',
    example: 'attach {bedrock}\n\ncore main() {\n    forge header: stone = 0x2A01\n    echo(bedrock_lane16_le_get(header, 0))\n}'
  },
  bedrock_lane16_be_get: {
    description: 'Helper OFS para extrair uma janela big-endian de 16 bits.',
    context: 'Use em headers de rede e protocolos sem espalhar byte swap manual pelo codigo.',
    example: 'attach {bedrock}\n\ncore main() {\n    forge header: stone = 0x2A01\n    echo(bedrock_lane16_be_get(header, 0))\n}'
  },
  bedrock_packet_opcode_be16: {
    description: 'Helper de pacote para ler a janela big-endian de 16 bits do cabecalho.',
    context: 'Use quando um header OFS precisa expor um campo protocolar em ordem de rede.',
    example: 'attach {bedrock-packet}\n\ncore main() {\n    forge header: stone = 0x0102030405060708\n    echo(bedrock_packet_opcode_be16(header))\n}'
  },
  version: {
    description: 'Comando CLI para exibir versao do compilador.',
    context: 'Use para diagnostico de ambiente e compatibilidade.',
    example: 'ofs version'
  },
  update: {
    description: 'Comando CLI de autoatualizacao via GitHub Releases.',
    context: 'Use para atualizar o compilador para a release mais recente.',
    example: 'ofs update'
  },
  help: {
    description: 'Comando CLI de ajuda com lista e contexto dos comandos.',
    context: 'Use para consultar sintaxe e fluxo recomendado de uso.',
    example: 'ofs help'
  },
  i16: {
    description: 'Inteiro com sinal de 16 bits.',
    context: 'Use para dados compactos com sinal.',
    example: 'forge delta: i16 = -300'
  }
};

const DIAGNOSTIC_REGEX = /^(.*):(\d+):(\d+):\s*(.*)$/;
// Matches new brace syntax: attach {name}  or  attach {F:path}
const ATTACH_LIB_REGEX  = /^\s*(?:attach|import)\s*\{\s*([A-Za-z_][A-Za-z0-9_.\-]*)\s*\}\s*$/;
const ATTACH_FILE_REGEX = /^\s*attach\s*\{\s*[Ff](?:ile)?:\s*([^}]+?)\s*\}\s*$/;
const ATTACH_REGEX = ATTACH_LIB_REGEX; // kept for back-compat references
const REGISTRY_URL = 'https://raw.githubusercontent.com/Samwns/Obsidian-Fault-Script/main/src/packages/registry.json';
const REGISTRY_CACHE_TTL_MS = 60 * 60 * 1000; // 1 hour
const VEIN_REGEX = /^\s*(?:(?:extern|rift)\s+)?vein\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/;
const MONOLITH_REGEX = /^\s*monolith\s+([A-Za-z_][A-Za-z0-9_]*)(?:\s+layout\s+(?:native|packed|c))?\s*\{/;
const RELEASES_API = 'https://api.github.com/repos/Samwns/Obsidian-Fault-Script/releases/latest';

let extensionContextRef = null;
let managedCompilerPathCache = null;
let compilerInstallPromise = null;
let activeExecution = null;
let installedLibDecoration = null;
let outdatedLibDecoration = null;
let registryCache = null;
let registryCacheTime = 0;

function platformTriple() {
  if (process.platform === 'win32' && os.arch() === 'x64') return 'win32-x64';
  if (process.platform === 'linux' && os.arch() === 'x64') return 'linux-x64';
  if (process.platform === 'darwin' && os.arch() === 'arm64') return 'darwin-arm64';
  return null;
}

function getEmbeddedCompilerPath() {
  const triple = platformTriple();
  if (!triple || !extensionContextRef?.extensionPath) {
    return null;
  }

  const fileName = process.platform === 'win32' ? 'ofs.exe' : 'ofs';
  const candidate = path.join(extensionContextRef.extensionPath, 'bin', triple, fileName);
  return fs.existsSync(candidate) ? candidate : null;
}

function compilerHasNativeRuntime(ofsPath) {
  if (!ofsPath || ofsPath === 'ofs') {
    return true;
  }

  const compilerDir = path.dirname(ofsPath);
  return fs.existsSync(path.join(compilerDir, 'magma.o'));
}

function ensureDir(dirPath) {
  fs.mkdirSync(dirPath, { recursive: true });
}

function getManagedCompilerPath() {
  if (!extensionContextRef?.globalStorageUri?.fsPath) {
    return null;
  }

  const installDir = path.join(extensionContextRef.globalStorageUri.fsPath, 'compiler');
  ensureDir(installDir);
  return process.platform === 'win32'
    ? path.join(installDir, 'ofs.exe')
    : path.join(installDir, 'ofs');
}

function httpGetJson(url) {
  return new Promise((resolve, reject) => {
    const req = https.get(url, { headers: { 'User-Agent': 'obsidian-fault-src/vscode-extension' } }, (res) => {
      if (res.statusCode >= 300 && res.statusCode < 400 && res.headers.location) {
        resolve(httpGetJson(res.headers.location));
        return;
      }

      if (res.statusCode !== 200) {
        reject(new Error(`HTTP ${res.statusCode} while fetching ${url}`));
        return;
      }

      let raw = '';
      res.setEncoding('utf8');
      res.on('data', (chunk) => { raw += chunk; });
      res.on('end', () => {
        try {
          resolve(JSON.parse(raw));
        } catch (err) {
          reject(err);
        }
      });
    });
    req.on('error', reject);
  });
}

function downloadToFile(url, destPath) {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(destPath);
    const request = https.get(url, { headers: { 'User-Agent': 'obsidian-fault-src/vscode-extension' } }, (res) => {
      if (res.statusCode >= 300 && res.statusCode < 400 && res.headers.location) {
        file.close(() => {
          fs.rm(destPath, { force: true }, () => {
            resolve(downloadToFile(res.headers.location, destPath));
          });
        });
        return;
      }

      if (res.statusCode !== 200) {
        file.close(() => fs.rm(destPath, { force: true }, () => reject(new Error(`HTTP ${res.statusCode} while downloading asset`))));
        return;
      }

      res.pipe(file);
      file.on('finish', () => file.close(resolve));
    });

    request.on('error', (err) => {
      file.close(() => fs.rm(destPath, { force: true }, () => reject(err)));
    });
  });
}

function pickReleaseAsset(releaseData) {
  const assets = Array.isArray(releaseData?.assets) ? releaseData.assets : [];
  if (process.platform === 'linux' && os.arch() === 'x64') {
    return assets.find((a) => /^ofs-linux-x64-installer-.*\.tar\.gz$/.test(a.name))
      || assets.find((a) => a.name === 'ofs-linux-x64-installer.tar.gz');
  }

  if (process.platform === 'win32' && os.arch() === 'x64') {
    return assets.find((a) => /^ofs-windows-x64-installer-.*\.exe$/.test(a.name))
      || assets.find((a) => a.name === 'ofs-windows-x64-installer.exe');
  }

  if (process.platform === 'darwin' && os.arch() === 'arm64') {
    return assets.find((a) => /^ofs-macos-arm64-installer-.*\.pkg$/.test(a.name))
      || assets.find((a) => a.name === 'ofs-macos-arm64-installer.pkg');
  }

  return null;
}

async function installCompilerFromRelease(progress) {
  progress.report({ message: 'Checking latest OFS release...', increment: 10 });
  const release = await httpGetJson(RELEASES_API);
  const asset = pickReleaseAsset(release);
  if (!asset?.browser_download_url || !asset?.name) {
    throw new Error(`No compatible installer found for ${process.platform}-${os.arch()}`);
  }

  const managedPath = getManagedCompilerPath();
  if (!managedPath) {
    throw new Error('Could not prepare extension storage for compiler install');
  }

  const tmpDir = fs.mkdtempSync(path.join(os.tmpdir(), 'ofs-ext-install-'));
  const assetPath = path.join(tmpDir, asset.name);

  progress.report({ message: `Downloading ${asset.name}...`, increment: 35 });
  await downloadToFile(asset.browser_download_url, assetPath);

  if (process.platform === 'linux') {
    const unpackDir = path.join(tmpDir, 'unpack');
    ensureDir(unpackDir);
    const extracted = await runExecFile('tar', ['-xzf', assetPath, '-C', unpackDir]);
    if (extracted.error) {
      throw new Error((extracted.stderr || extracted.stdout || 'Failed to extract Linux installer').trim());
    }

    const binaryCandidate = path.join(unpackDir, 'ofs');
    if (!fs.existsSync(binaryCandidate)) {
      throw new Error('Downloaded package does not contain compiler binary');
    }

    fs.copyFileSync(binaryCandidate, managedPath);
    fs.chmodSync(managedPath, 0o755);

    const installDir = path.dirname(managedPath);
    for (const runtimeFile of ['magma.o']) {
      const sourcePath = path.join(unpackDir, runtimeFile);
      if (fs.existsSync(sourcePath)) {
        fs.copyFileSync(sourcePath, path.join(installDir, runtimeFile));
      }
    }

    progress.report({ message: 'Compiler installed in extension storage.', increment: 45 });
    fs.rmSync(tmpDir, { recursive: true, force: true });
    return managedPath;
  }

  if (process.platform === 'win32') {
    progress.report({ message: 'Running Windows installer...', increment: 20 });
    const winInstall = await runExecFile('powershell', [
      '-NoProfile',
      '-ExecutionPolicy',
      'Bypass',
      '-Command',
      `Start-Process -FilePath '${assetPath.replace(/'/g, "''")}' -Verb RunAs -Wait`
    ]);
    if (winInstall.error) {
      throw new Error((winInstall.stderr || winInstall.stdout || 'Windows installer execution failed').trim());
    }
    fs.rmSync(tmpDir, { recursive: true, force: true });
    return 'ofs';
  }

  if (process.platform === 'darwin') {
    fs.rmSync(tmpDir, { recursive: true, force: true });
    throw new Error('Automatic install on macOS requires admin privileges. Install the .pkg from Releases.');
  }

  fs.rmSync(tmpDir, { recursive: true, force: true });
  throw new Error(`Unsupported platform: ${process.platform}-${os.arch()}`);
}

function getWorkspaceRoot() {
  return vscode.workspace.workspaceFolders?.[0]?.uri.fsPath || vscode.workspace.rootPath || undefined;
}

function getBundledCompilerPath() {
  const root = getWorkspaceRoot();
  if (!root) {
    return null;
  }

  const candidates = process.platform === 'win32'
    ? [
        path.join(root, 'ofs', 'ofscc', 'ofscc_v2.exe'),
        path.join(root, 'ofs', 'ofscc', 'ofscc.exe'),
        path.join(root, 'ofs', 'build', 'ofs.exe'),
        path.join(root, 'ofs', 'build', 'Release', 'ofs.exe')
      ]
    : [
        path.join(root, 'ofs', 'ofscc', 'ofscc_v2'),
        path.join(root, 'ofs', 'ofscc', 'ofscc'),
        path.join(root, 'ofs', 'build', 'ofs')
      ];

  for (const candidate of candidates) {
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  return null;
}

function getOfsPath() {
  const configured = vscode.workspace.getConfiguration().get('ofs.path', 'ofs');
  if (configured && configured !== 'ofs') {
    return configured;
  }

  const preferEmbedded = vscode.workspace.getConfiguration().get('ofs.preferEmbeddedCompiler', true);
  if (preferEmbedded) {
    const embedded = getEmbeddedCompilerPath();
    if (embedded) {
      return embedded;
    }
  }

  const bundled = getBundledCompilerPath();
  if (bundled) {
    return bundled;
  }

  const managed = managedCompilerPathCache || getManagedCompilerPath();
  if (managed && fs.existsSync(managed)) {
    managedCompilerPathCache = managed;
    return managed;
  }

  return configured;
}

function getOfsPathForNativeBuild() {
  const configured = vscode.workspace.getConfiguration().get('ofs.path', 'ofs');
  if (configured && configured !== 'ofs') {
    return configured;
  }

  const bundled = getBundledCompilerPath();
  if (bundled && compilerHasNativeRuntime(bundled)) {
    return bundled;
  }

  const preferEmbedded = vscode.workspace.getConfiguration().get('ofs.preferEmbeddedCompiler', true);
  if (preferEmbedded) {
    const embedded = getEmbeddedCompilerPath();
    if (embedded && compilerHasNativeRuntime(embedded)) {
      return embedded;
    }
  }

  const managed = managedCompilerPathCache || getManagedCompilerPath();
  if (managed && fs.existsSync(managed) && compilerHasNativeRuntime(managed)) {
    managedCompilerPathCache = managed;
    return managed;
  }

  return configured;
}

function ensureCompilerExecutable(ofsPath) {
  if (!ofsPath || ofsPath === 'ofs' || process.platform === 'win32') {
    return ofsPath;
  }

  try {
    fs.chmodSync(ofsPath, 0o755);
  } catch {
    // Ignore chmod failures here and let execution report the real error later.
  }

  return ofsPath;
}

async function resolveReadyOfsPath() {
  if (compilerInstallPromise) {
    await compilerInstallPromise;
  }

  let ofsPath = getOfsPath();
  if (ofsPath !== 'ofs') {
    return ensureCompilerExecutable(ofsPath);
  }

  if (await commandExists('ofs')) {
    return 'ofs';
  }

  const autoInstall = vscode.workspace.getConfiguration().get('ofs.autoInstallCompiler', true);
  if (!autoInstall) {
    return null;
  }

  if (!compilerInstallPromise) {
    compilerInstallPromise = vscode.window.withProgress(
      {
        location: vscode.ProgressLocation.Notification,
        title: 'Installing OFS compiler',
        cancellable: false
      },
      async (progress) => installCompilerFromRelease(progress)
    ).then((installedPath) => {
      if (installedPath && installedPath !== 'ofs') {
        managedCompilerPathCache = installedPath;
      }
      vscode.window.showInformationMessage('OFS compiler installed and ready to use.');
      return installedPath;
    }).catch((err) => {
      vscode.window.showWarningMessage(`OFS compiler auto-install failed: ${err.message}`);
      return null;
    }).finally(() => {
      compilerInstallPromise = null;
    });
  }

  await compilerInstallPromise;
  ofsPath = getOfsPath();
  if (ofsPath === 'ofs' && !(await commandExists('ofs'))) {
    return null;
  }

  return ensureCompilerExecutable(ofsPath);
}

async function commandExists(commandName) {
  if (!commandName) {
    return false;
  }

  const checker = process.platform === 'win32' ? 'where' : 'which';
  const result = await runExecFile(checker, [commandName]);
  return !result.error;
}

async function ensureCompilerInstalledOnActivate() {
  const autoInstall = vscode.workspace.getConfiguration().get('ofs.autoInstallCompiler', true);
  if (!autoInstall) {
    return null;
  }

  const configured = vscode.workspace.getConfiguration().get('ofs.path', 'ofs');
  if (configured && configured !== 'ofs') {
    return configured;
  }

  const preferEmbedded = vscode.workspace.getConfiguration().get('ofs.preferEmbeddedCompiler', true);
  if (preferEmbedded) {
    const embedded = getEmbeddedCompilerPath();
    if (embedded) {
      return embedded;
    }
  }

  const bundled = getBundledCompilerPath();
  if (bundled) {
    return bundled;
  }

  const managed = getManagedCompilerPath();
  if (managed && fs.existsSync(managed)) {
    managedCompilerPathCache = managed;
    return managed;
  }

  if (await commandExists('ofs')) {
    return 'ofs';
  }

  if (compilerInstallPromise) {
    return compilerInstallPromise;
  }

  compilerInstallPromise = vscode.window.withProgress(
    {
      location: vscode.ProgressLocation.Notification,
      title: 'Installing OFS compiler',
      cancellable: false
    },
    async (progress) => installCompilerFromRelease(progress)
  ).then((installedPath) => {
    if (installedPath && installedPath !== 'ofs') {
      managedCompilerPathCache = installedPath;
    }
    vscode.window.showInformationMessage('OFS compiler installed and ready to use.');
    return installedPath;
  }).catch((err) => {
    vscode.window.showWarningMessage(`OFS compiler auto-install failed: ${err.message}`);
    return null;
  }).finally(() => {
    compilerInstallPromise = null;
  });

  return compilerInstallPromise;
}

async function setExecutionContext(running, paused) {
  await vscode.commands.executeCommand('setContext', 'ofs.executionRunning', running);
  await vscode.commands.executeCommand('setContext', 'ofs.executionPaused', paused);
}

function stopExecutionWatcher() {
  return undefined;
}

async function clearActiveExecution() {
  stopExecutionWatcher();
  activeExecution = null;
  await setExecutionContext(false, false);
}

function isProcessAlive(pid) {
  if (!pid) {
    return false;
  }

  try {
    process.kill(pid, 0);
    return true;
  } catch {
    return false;
  }
}

function startExecutionWatcher() {
  return undefined;
}

function normalizeTerminalChunk(chunk) {
  return String(chunk).replace(/\r?\n/g, '\r\n');
}

async function waitForShellIntegration(terminal, timeoutMs = 1500) {
  if (terminal.shellIntegration) {
    return terminal.shellIntegration;
  }

  return new Promise((resolve) => {
    let settled = false;
    const timer = setTimeout(() => {
      if (!settled) {
        settled = true;
        disposable.dispose();
        resolve(terminal.shellIntegration || null);
      }
    }, timeoutMs);

    const disposable = vscode.window.onDidChangeTerminalShellIntegration(({ terminal: changedTerminal, shellIntegration }) => {
      if (changedTerminal !== terminal || settled) {
        return;
      }

      settled = true;
      clearTimeout(timer);
      disposable.dispose();
      resolve(shellIntegration);
    });
  });
}

function getOrCreateExecutionTerminal(cwd, ofsPath) {
  if (cwd) {
    const env = {};
    const compilerDir = ofsPath && ofsPath !== 'ofs' ? path.dirname(ofsPath) : '';
    if (compilerDir && fs.existsSync(compilerDir)) {
      const pathKey = process.platform === 'win32' ? 'Path' : 'PATH';
      env[pathKey] = `${compilerDir}${path.delimiter}${process.env[pathKey] || process.env.PATH || ''}`;
    }
    const terminal = vscode.window.createTerminal({ name: 'OFS', cwd, env });
    terminal.show(true);
    return terminal;
  }

  const existingTerminal = vscode.window.activeTerminal;
  if (existingTerminal) {
    return existingTerminal;
  }

  const terminal = vscode.window.createTerminal({ name: 'OFS' });
  terminal.show(true);
  return terminal;
}

async function runCompiledExecutableInTerminal(compiled, terminalName) {
  await terminateActiveExecution({ silent: true });

  const terminal = getOrCreateExecutionTerminal();

  activeExecution = {
    paused: false,
    terminal,
    childProcess: null,
    shellExecution: null
  };

  await setExecutionContext(true, false);
  terminal.show(true);

  const shellIntegration = await waitForShellIntegration(terminal);
  if (shellIntegration) {
    const execution = shellIntegration.executeCommand(compiled.exePath, []);
    activeExecution.shellExecution = execution;
    execution.exitCode.then(async () => {
      await clearActiveExecution();
    }).catch(async () => {
      await clearActiveExecution();
    });
    return;
  }

  terminal.sendText(getExecutableRunCommand(compiled.exePath));
}

async function pauseActiveExecution() {
  if (!activeExecution) {
    vscode.window.showWarningMessage('Nenhuma execucao OFS ativa para pausar.');
    return;
  }

  if (process.platform === 'win32') {
    vscode.window.showWarningMessage('Pausar execucao no terminal atual nao e suportado no Windows.');
    return;
  }

  const pid = activeExecution.childProcess?.pid;
  if (!pid) {
    activeExecution.terminal?.sendText('\u001A', false);
    activeExecution.paused = true;
    await setExecutionContext(true, true);
    return;
  }

  try {
    process.kill(pid, 'SIGSTOP');
    activeExecution.paused = true;
    await setExecutionContext(true, true);
  } catch (err) {
    if (!isProcessAlive(pid)) {
      await clearActiveExecution();
      return;
    }
    vscode.window.showErrorMessage(`Falha ao pausar execucao OFS: ${err.message}`);
  }
}

async function resumeActiveExecution() {
  if (!activeExecution) {
    vscode.window.showWarningMessage('Nenhuma execucao OFS ativa para continuar.');
    return;
  }

  if (process.platform === 'win32') {
    vscode.window.showWarningMessage('Continuar execucao no terminal atual nao e suportado no Windows.');
    return;
  }

  const pid = activeExecution.childProcess?.pid;
  if (!pid) {
    activeExecution.terminal?.sendText('fg');
    activeExecution.paused = false;
    await setExecutionContext(true, false);
    return;
  }

  try {
    process.kill(pid, 'SIGCONT');
    activeExecution.paused = false;
    await setExecutionContext(true, false);
  } catch (err) {
    if (!isProcessAlive(pid)) {
      await clearActiveExecution();
      return;
    }
    vscode.window.showErrorMessage(`Falha ao continuar execucao OFS: ${err.message}`);
  }
}

async function terminateActiveExecution(options = {}) {
  if (!activeExecution) {
    if (!options.silent) {
      vscode.window.showWarningMessage('Nenhuma execucao OFS ativa para encerrar.');
    }
    return;
  }

  const pid = activeExecution.childProcess?.pid;
  if (activeExecution.liveServer) {
    if (activeExecution.watchers) {
      for (const watcher of activeExecution.watchers) {
        try { watcher.close(); } catch {}
      }
    }
    await new Promise((resolve) => activeExecution.liveServer.close(resolve));
    await clearActiveExecution();
    return;
  }
  if (!pid) {
    activeExecution.terminal?.sendText('\u0003', false);
    await clearActiveExecution();
    return;
  }

  try {
    if (process.platform === 'win32') {
      await runExecFile('taskkill', ['/PID', String(pid), '/T', '/F']);
    } else {
      process.kill(pid, 'SIGTERM');
    }
  } catch (err) {
    if (!isProcessAlive(pid)) {
      await clearActiveExecution();
      return;
    }

    vscode.window.showErrorMessage(`Falha ao encerrar execucao OFS: ${err.message}`);
    return;
  }

  activeExecution.paused = false;
  activeExecution.terminal?.show(true);
}

function getShellCommand(ofsPath, filePath) {
  const escapedOfs = ofsPath.replace(/"/g, '\\"');
  const escapedFile = filePath.replace(/"/g, '\\"');

  if (process.platform !== 'win32') {
    return `"${escapedOfs}" "${escapedFile}"`;
  }

  const terminalConfig = vscode.workspace.getConfiguration('terminal.integrated');
  const defaultProfile = terminalConfig.get('defaultProfile.windows', '');

  if (/cmd|command\s*prompt/i.test(defaultProfile)) {
    return `"${escapedOfs}" "${escapedFile}"`;
  }

  // Safe default for modern VS Code on Windows, where PowerShell/pwsh is common.
  return `& "${escapedOfs}" "${escapedFile}"`;
}

function getTerminalRunCommand(ofsPath, filePath) {
  const fileName = path.basename(filePath).replace(/"/g, '\\"');
  const compilerName = path.basename(ofsPath || 'ofs').toLowerCase();
  const isOfscc = compilerName.startsWith('ofscc');

  if (process.platform === 'win32') {
    if (isOfscc) {
      return `& "${ofsPath.replace(/"/g, '\\"')}" "${fileName}" -o "${fileName}.exe"`;
    }

    if (ofsPath === 'ofs' || compilerName === 'ofs.exe') {
      return `ofs run "${fileName}"`;
    }

    return `& "${ofsPath.replace(/"/g, '\\"')}" run "${fileName}"`;
  }

  if (isOfscc) {
    return `"${ofsPath.replace(/"/g, '\\"')}" "${fileName}" -o "${fileName}.bin"`;
  }

  if (ofsPath === 'ofs' || compilerName === 'ofs') {
    if (ofsPath === 'ofs') {
      return `ofs run "${fileName}"`;
    }
    return `"${ofsPath.replace(/"/g, '\\"')}" run "${fileName}"`;
  }

  return `"${ofsPath.replace(/"/g, '\\"')}" run "${fileName}"`;
}

function createRangeFromLineCol(document, line, col) {
  const lineIdx = Math.max(line - 1, 0);
  const colIdx = Math.max(col - 1, 0);
  const safeLine = Math.min(lineIdx, document.lineCount - 1);
  const safeCol = Math.min(colIdx, document.lineAt(safeLine).text.length);
  const start = new vscode.Position(safeLine, safeCol);
  const end = new vscode.Position(safeLine, Math.min(safeCol + 1, document.lineAt(safeLine).text.length));
  return new vscode.Range(start, end);
}

// Known stdlib library names -> filenames (mirrors compiler's STDLIB_NAMES map)
const STDLIB_NAMES = {
  'core':            'core.ofs',
  'math':            'math.ofs',
  'string':          'string.ofs',
  'io':              'io.ofs',
  'webserver':       'webserver.ofs',
  'oes':          'oes.ofs',
  'ofshtml':         'ofshtml.ofs',
  'odl':       'odl.ofs',
  'webui':           'webui.ofs',
  'serve':           'webserver.ofs',
  'bedrock':         'bedrock.ofs',
  'bedrock-packet':  'bedrock_packet.ofs',
  'terminal-colors': 'terminal_colors.ofs',
  'memory-modes':    'memory_modes.ofs',
  'rift':            'rift.ofs',
  'canvas':          'canvas.ofs',
  'window':          'window.ofs',
  'net-windows':     'net_windows.ofs',
  'oll-x11':         'oll_x11.ofs',
  'android':         'android.ofs',
  'android-http':    'android_http.ofs',
  'android-input':   'android_input.ofs',
  'android-webview': 'android_webview.ofs',
  'audio-android':   'audio_android.ofs',
  'avi':             'avi.ofs',
  'oal-android':     'oal_android.ofs',
  'oal-runtime':     'oal_runtime.ofs',
  'oal-x11':         'oal_x11.ofs',
  'png':             'png.ofs',
  'sha256':          'sha256.ofs',
  'tls':             'tls.ofs',
  'wav':             'wav.ofs',
  'zip':             'zip.ofs',
  'test-lib':        'test_lib.ofs',
  'fs':              'fs.ofs',
  'net':             'net.ofs',
  'http':            'http.ofs',
  'oll':             'oll.ofs',
  'ui':              'ui.ofs',
  'raster':          'raster.ofs',
  'ttf':             'ttf.ofs',
  'x11win':          'x11win.ofs',
  'icons':           'icons.ofs',
  'hashmap':         'hashmap.ofs',
  'strbuf':          'strbuf.ofs',
};

function getStdlibSearchDirs(docDir) {
  const dirs = [];
  const envPath = process.env.OFS_STDLIB_PATH;
  if (envPath) dirs.push(envPath);
  let d = docDir || '';
  for (let i = 0; i < 6 && d; i++) {
    const candidate = path.join(d, 'stdlib');
    if (fs.existsSync(candidate)) dirs.push(candidate);
    const repoCandidate = path.join(d, 'ofs', 'stdlib');
    if (fs.existsSync(repoCandidate)) dirs.push(repoCandidate);
    const parent = path.dirname(d);
    if (parent === d) break;
    d = parent;
  }
  dirs.push('/usr/local/share/ofs/stdlib', '/usr/share/ofs/stdlib');
  return dirs;
}

function resolveLibraryPath(libName, docDir) {
  const filename = STDLIB_NAMES[libName] || (libName + '.ofs');
  for (const dir of getStdlibSearchDirs(docDir)) {
    const candidate = path.join(dir, filename);
    if (fs.existsSync(candidate)) return candidate;
  }
  for (const base of getLibSearchPaths()) {
    const pkg = path.join(base, libName, 'libs', filename);
    if (fs.existsSync(pkg)) return pkg;
    const direct = path.join(base, filename);
    if (fs.existsSync(direct)) return direct;
  }

  let repo = docDir || '';
  for (let i = 0; i < 8 && repo; i++) {
    const pkg = path.join(repo, 'src/packages', 'src', libName, 'libs', filename);
    if (fs.existsSync(pkg)) return pkg;
    const parent = path.dirname(repo);
    if (parent === repo) break;
    repo = parent;
  }
  return null;
}

function getLibSearchPaths() {
  const env = process.env.OFS_LIB_PATH || '';
  const sep = process.platform === 'win32' ? ';' : ':';
  return env.split(sep).map((p) => p.trim()).filter(Boolean);
}

function resolveAttachPath(modulePath, docDir) {
  // attach {F:path} — explicit file reference
  const fileParts = modulePath.match(/^[Ff](?:ile)?:(.+)$/);
  if (fileParts) {
    const fp = fileParts[1].trim();
    const local = path.resolve(docDir, fp);
    if (fs.existsSync(local)) return local;
    for (const base of getLibSearchPaths()) {
      const candidate = path.resolve(base, fp);
      if (fs.existsSync(candidate)) return candidate;
    }
    return null;
  }

  // attach {name} — stdlib / package library
  const byName = resolveLibraryPath(modulePath, docDir);
  if (byName) return byName;

  // Fallback: treat as a direct relative file path
  const local = path.resolve(docDir, modulePath);
  if (fs.existsSync(local)) return local;

  for (const base of getLibSearchPaths()) {
    const candidate = path.resolve(base, modulePath);
    if (fs.existsSync(candidate)) return candidate;
  }

  return null;
}

function parseAttachTargets(sourceText) {
  const lines = sourceText.split(/\r?\n/);
  const targets = [];

  for (const line of lines) {
    let m = line.match(ATTACH_LIB_REGEX);
    if (m) { targets.push(m[1]); continue; }
    m = line.match(ATTACH_FILE_REGEX);
    if (m) { targets.push('F:' + m[1].trim()); }
  }

  return targets;
}

function extractModuleSymbols(sourceText) {
  const lines = sourceText.split(/\r?\n/);
  const symbols = [];

  for (const line of lines) {
    let m = line.match(VEIN_REGEX);
    if (m) {
      symbols.push({ name: m[1], kind: vscode.CompletionItemKind.Function });
      continue;
    }

    m = line.match(MONOLITH_REGEX);
    if (m) {
      symbols.push({ name: m[1], kind: vscode.CompletionItemKind.Struct });
    }
  }

  return symbols;
}

function collectAttachSymbols(document) {
  const docDir = path.dirname(document.fileName || '');
  const queue = parseAttachTargets(document.getText());
  const visitedFiles = new Set();
  const symbols = [];

  while (queue.length > 0) {
    const modulePath = queue.shift();
    const resolved = resolveAttachPath(modulePath, docDir);
    if (!resolved || visitedFiles.has(resolved)) {
      continue;
    }

    visitedFiles.add(resolved);

    let content = '';
    try {
      content = fs.readFileSync(resolved, 'utf8');
    } catch {
      continue;
    }

    symbols.push(...extractModuleSymbols(content));
    queue.push(...parseAttachTargets(content));
  }

  const dedup = new Map();
  for (const symbol of symbols) {
    if (!dedup.has(symbol.name)) {
      dedup.set(symbol.name, symbol);
    }
  }

  return Array.from(dedup.values());
}

const ANSI_REGEX = /\u001b\[[0-9;?]*[A-Za-z]/g;
// Cabecalhos do ofscc:
//   [ERRO SINTATICO arquivo:l:c]: msg   (antes so l:c; o arquivo e o do attach)
//   [ERRO SEMANTICO arquivo:l:c]: msg   (+ "voce quis dizer", "dica:")
//   [ERRO DE CONTRATO arquivo:l:c]: msg (+ "regra em arquivo:l:c")
//   [ERRO DE ATTACH arquivo:l]: msg
//   [AVISO DE TIPO arquivo:l:c]: msg
//   [fault E0042] l:c: msg
//   arquivo.ofs:l:c: msg                (formato legado)
const COMPILER_HEADER_REGEX = /^\[(ERRO SINTATICO|ERRO SEMANTICO|ERRO DE CONTRATO|ERRO DE ATTACH|AVISO DE TIPO|AVISO SEMANTICO)\s+(.+?)\]:\s*(.*)$/;
const COMPILER_FAULT_REGEX = /^\[fault (E\d+)\]\s*(\d+):(\d+):\s*(.*)$/;

function splitLocation(loc) {
  // "arquivo:l:c", "arquivo:l", "l:c" ou "l"; o arquivo pode conter ':' (Windows)
  let m = loc.match(/^(.*):(\d+):(\d+)$/);
  if (m) return { file: m[1].trim() || null, line: Number.parseInt(m[2], 10), col: Number.parseInt(m[3], 10) };
  m = loc.match(/^(\d+):(\d+)$/);
  if (m) return { file: null, line: Number.parseInt(m[1], 10), col: Number.parseInt(m[2], 10) };
  m = loc.match(/^(.*):(\d+)$/);
  if (m) return { file: m[1].trim() || null, line: Number.parseInt(m[2], 10), col: 1 };
  m = loc.match(/^(\d+)$/);
  if (m) return { file: null, line: Number.parseInt(m[1], 10), col: 1 };
  return null;
}

function parseCompilerMessages(output) {
  const messages = [];
  const lines = String(output || '').replace(ANSI_REGEX, '').split(/\r?\n/);
  let current = null;

  for (const rawLine of lines) {
    // ruido do indicador de progresso do compilador (spinner, "Reading", "done ...")
    if (rawLine.includes('\r') || /^\s*[\u2192\u2713\u2717]/.test(rawLine) || /^done\s/.test(rawLine)) {
      current = null;
      continue;
    }
    let m = rawLine.match(COMPILER_HEADER_REGEX);
    if (m) {
      const loc = splitLocation(m[2].trim());
      if (loc) {
        const kind = m[1];
        current = {
          severity: kind.startsWith('AVISO') ? 'warning' : 'error',
          kind,
          code: null,
          file: loc.file,
          line: loc.line,
          col: loc.col,
          wholeLine: kind === 'ERRO DE ATTACH',
          message: m[3].trim(),
          extra: []
        };
        messages.push(current);
        continue;
      }
    }
    m = rawLine.match(COMPILER_FAULT_REGEX);
    if (m) {
      current = {
        severity: 'error', kind: 'fault', code: m[1], file: null,
        line: Number.parseInt(m[2], 10), col: Number.parseInt(m[3], 10),
        wholeLine: false, message: m[4].trim(), extra: []
      };
      messages.push(current);
      continue;
    }
    m = rawLine.match(DIAGNOSTIC_REGEX);
    if (m && m[1].trim().endsWith('.ofs')) {
      current = {
        severity: 'error', kind: 'legacy', code: null, file: m[1].trim(),
        line: Number.parseInt(m[2], 10), col: Number.parseInt(m[3], 10),
        wholeLine: false, message: m[4].trim() || 'Unknown OFS error', extra: []
      };
      messages.push(current);
      continue;
    }
    // linhas de continuacao (indentadas) pertencem a ultima mensagem
    if (current && /^\s{2,}\S/.test(rawLine)) {
      const text = rawLine.trim();
      if (!text.startsWith('-->')) {
        current.extra.push(text);
      } else if (current.kind === 'ERRO SINTATICO') {
        current.extra.push(text.replace(/^-->\s*/, ''));
      }
      continue;
    }
    if (rawLine.trim() === '' ) {
      continue;
    }
    current = null;
  }
  return messages;
}

function compilerMessageText(msg) {
  const prefix = msg.code ? `[${msg.code}] ` : '';
  const extra = msg.extra.filter((l) => !/^\(um modulo verificado/.test(l));
  return prefix + [msg.message, ...extra].join('\n');
}

function sameFilePath(a, b, baseDir) {
  if (!a || !b) return false;
  const norm = (p) => path.resolve(baseDir, p);
  const na = norm(a);
  const nb = norm(b);
  return process.platform === 'win32' ? na.toLowerCase() === nb.toLowerCase() : na === nb;
}

function rangeForMessage(textLines, msg) {
  const lineIdx = Math.min(Math.max(msg.line - 1, 0), Math.max(textLines.length - 1, 0));
  const lineText = textLines[lineIdx] || '';
  if (msg.wholeLine) {
    const first = lineText.search(/\S/);
    return new vscode.Range(lineIdx, Math.max(first, 0), lineIdx, lineText.length);
  }
  const colIdx = Math.min(Math.max(msg.col - 1, 0), lineText.length);
  // realca a palavra/token que comeca na coluna indicada
  const rest = lineText.slice(colIdx);
  const word = rest.match(/^[A-Za-z_][A-Za-z0-9_]*/);
  const len = word ? word[0].length : 1;
  return new vscode.Range(lineIdx, colIdx, lineIdx, Math.min(colIdx + len, Math.max(lineText.length, colIdx + 1)));
}

// Devolve Map<caminho absoluto, Diagnostic[]>. Erros sem arquivo pertencem ao documento.
// Erros em arquivos `attach` vao para o proprio arquivo e, alem disso, um resumo
// aparece no documento (na linha do attach correspondente) para nao passar despercebido.
function parseDiagnosticsByFile(document, output) {
  const baseDir = path.dirname(document.fileName);
  const byFile = new Map();
  const docKey = document.fileName;
  const push = (key, diag) => {
    if (!byFile.has(key)) byFile.set(key, []);
    byFile.get(key).push(diag);
  };
  const docLines = document.getText().split(/\r?\n/);

  for (const msg of parseCompilerMessages(output)) {
    const severity = msg.severity === 'warning'
      ? vscode.DiagnosticSeverity.Warning
      : vscode.DiagnosticSeverity.Error;
    const text = compilerMessageText(msg);
    const isDoc = !msg.file || sameFilePath(msg.file, document.fileName, baseDir);

    if (isDoc) {
      const diag = new vscode.Diagnostic(rangeForMessage(docLines, msg), text, severity);
      diag.source = 'ofscc';
      if (msg.code) diag.code = msg.code;
      push(docKey, diag);
      continue;
    }

    let otherPath = path.resolve(baseDir, msg.file);
    if (!fs.existsSync(otherPath)) {
      const root = getWorkspaceRoot();
      if (root && fs.existsSync(path.resolve(root, msg.file))) {
        otherPath = path.resolve(root, msg.file);
      }
    }
    let otherLines = [];
    try { otherLines = fs.readFileSync(otherPath, 'utf8').split(/\r?\n/); } catch { /* arquivo ilegivel */ }
    const diag = new vscode.Diagnostic(rangeForMessage(otherLines, msg), text, severity);
    diag.source = 'ofscc';
    if (msg.code) diag.code = msg.code;
    push(otherPath, diag);

    // ponteiro no documento: linha do attach que traz o arquivo com problema
    const base = path.basename(msg.file);
    let attachLine = docLines.findIndex((l) => /^\s*attach\s*\{/.test(l) && l.includes(base));
    if (attachLine < 0) attachLine = docLines.findIndex((l) => /^\s*attach\s*\{/.test(l));
    if (attachLine < 0) attachLine = 0;
    const attachText = docLines[attachLine] || '';
    const pointer = new vscode.Diagnostic(
      new vscode.Range(attachLine, 0, attachLine, attachText.length),
      `${base}:${msg.line}:${msg.col}: ${msg.message}`,
      severity
    );
    pointer.source = 'ofscc';
    push(docKey, pointer);
  }
  return byFile;
}

function parseDiagnostics(document, output) {
  return parseDiagnosticsByFile(document, output).get(document.fileName) || [];
}

function parseFirstCompilerError(output) {
  const first = parseCompilerMessages(output).find((m) => m.severity === 'error');
  if (first) {
    return {
      line: first.line,
      col: first.col,
      file: first.file,
      message: compilerMessageText(first).split('\n')[0]
    };
  }

  const fallback = String(output || '').replace(ANSI_REGEX, '').split(/\r?\n/)
    .filter((l) => l.trim() && !l.includes('\r') && !/^\s*[\u2192\u2713\u2717]/.test(l)).join('\n').trim();
  return fallback ? { line: null, col: null, message: fallback } : null;
}

function runExecFile(command, args, options = {}) {
  return new Promise((resolve) => {
    cp.execFile(command, args, options, (error, stdout, stderr) => {
      resolve({ error, stdout, stderr });
    });
  });
}

function isWebSiteDocument(document) {
  if (document.languageId === 'html' || document.fileName.toLowerCase().endsWith('.html')) {
    return true;
  }
  if (document.languageId === 'odl' || document.fileName.toLowerCase().endsWith('.odl')) {
    return true;
  }
  const text = document.getText();
  return /^\s*(?:attach|import)\s*\{\s*webui\s*\}/m.test(text)
    || /\bwebui\.(?:serve|page|hero|nav|button|panel|grid|card|stack)\s*\(/.test(text)
    || /\bserve_html_(?:once|forever)\s*\(/.test(text)
    || /\bserve_(?:once|forever)\s*\([^,]+,\s*MIME_HTML\s*,/.test(text);
}

async function goLiveCurrentFile() {
  const editor = vscode.window.activeTextEditor;
  const activeFile = editor?.document?.fileName?.toLowerCase() || '';
  const isHtmlTarget = Boolean(editor) && (editor.document.languageId === 'html' || activeFile.endsWith('.html'));
  if (!editor || (!['ofs', 'odl'].includes(editor.document.languageId) && !isHtmlTarget)) {
    vscode.window.showErrorMessage('Open an OFS web program, an .odl document, or a generated .html target to Go Live.');
    return;
  }

  await editor.document.save();
  if (!isWebSiteDocument(editor.document)) {
    vscode.window.showWarningMessage('This OFS file does not declare a web site. Use attach {webui}, webui.serve(...), or an .odl document.');
    return;
  }

  const ofsPath = await resolveReadyOfsPath();
  if (!ofsPath) {
    vscode.window.showErrorMessage('OFS compiler not found. Enable ofs.autoInstallCompiler or configure ofs.path.');
    return;
  }

  const file = editor.document.fileName;
  const cwd = path.dirname(file);
  const port = vscode.workspace.getConfiguration().get('ofs.goLivePort', 8080);
  if (isHtmlTarget) {
    let liveVersion = Date.now();
    await terminateActiveExecution({ silent: true });

    const liveReloadScript = () => `<script>(()=>{let v=${liveVersion};setInterval(async()=>{try{const n=Number(await (await fetch('/__ofs_live')).text());if(n&&n!==v)location.reload()}catch{}},450)})();</script>`;
    const mime = { '.html': 'text/html; charset=utf-8', '.css': 'text/css; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.mjs': 'text/javascript; charset=utf-8', '.svg': 'image/svg+xml', '.png': 'image/png', '.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.webp': 'image/webp', '.ico': 'image/x-icon' };
    const rootDir = path.resolve(cwd);
    const liveServer = http.createServer((request, response) => {
      const requested = decodeURIComponent((request.url || '/').split('?')[0]);
      if (requested === '/__ofs_live') {
        response.writeHead(200, { 'Content-Type': 'text/plain; charset=utf-8', 'Cache-Control': 'no-store' });
        response.end(String(liveVersion));
        return;
      }
      const relative = requested === '/' ? path.basename(file) : requested.replace(/^\/+/, '');
      const target = path.resolve(rootDir, relative);
      if (!target.startsWith(rootDir + path.sep) && target !== rootDir) {
        response.writeHead(403).end('Forbidden');
        return;
      }
      fs.readFile(target, (error, data) => {
        if (error) {
          response.writeHead(404).end('Not found');
          return;
        }
        const ext = path.extname(target).toLowerCase();
        if (ext === '.html') {
          data = Buffer.from(String(data).replace('</body>', `${liveReloadScript()}</body>`));
        }
        response.writeHead(200, { 'Content-Type': mime[ext] || 'application/octet-stream', 'Cache-Control': 'no-store' });
        response.end(data);
      });
    });
    await new Promise((resolve, reject) => liveServer.once('error', reject).listen(port, '127.0.0.1', resolve));
    const watchers = [fs.watch(file, () => { liveVersion = Date.now(); })];
    for (const entry of fs.readdirSync(cwd)) {
      if (/\.(css|js|mjs|png|jpe?g|gif|svg|webp|ico)$/i.test(entry)) {
        watchers.push(fs.watch(path.join(cwd, entry), () => { liveVersion = Date.now(); }));
      }
    }
    activeExecution = { paused: false, terminal: null, childProcess: null, shellExecution: null, sourceFile: file, liveServer, watchers };
    await setExecutionContext(true, false);
    const url = `http://127.0.0.1:${port}`;
    vscode.window.showInformationMessage(`HTML Go Live running with live reload at ${url}`, 'Open Browser').then((choice) => {
      if (choice === 'Open Browser') vscode.env.openExternal(vscode.Uri.parse(url));
    });
    return;
  }

  if (editor.document.languageId === 'odl') {
    const outputDir = path.join(cwd, '.ofs-live');
    const outputFile = path.join(outputDir, 'index.html');
    let liveVersion = Date.now();
    let rebuildTimer = null;
    fs.mkdirSync(outputDir, { recursive: true });

    const compileLiveProject = async () => {
      const compileResult = await runExecFile(ofsPath, ['odl', file, '-o', outputFile], { cwd });
      if (compileResult.error) {
        const combined = `${compileResult.stdout || ''}\n${compileResult.stderr || ''}`.trim();
        vscode.window.showErrorMessage(combined || 'ODL compilation failed.');
        return false;
      }
      for (const entry of fs.readdirSync(cwd)) {
        const source = path.join(cwd, entry);
        const stat = fs.statSync(source);
        if (!stat.isFile()) continue;
        if (entry.endsWith('.oes')) {
          const result = await runExecFile(ofsPath, ['oes', source, '-o', path.join(outputDir, `${path.parse(entry).name}.css`)], { cwd });
          if (result.error) {
            vscode.window.showWarningMessage(`OES compile failed: ${entry}`);
          }
        } else if (/\.(js|mjs|css|png|jpe?g|gif|svg|webp|ico)$/i.test(entry)) {
          fs.copyFileSync(source, path.join(outputDir, entry));
        }
      }
      liveVersion = Date.now();
      return true;
    };

    if (!(await compileLiveProject())) return;

    await terminateActiveExecution({ silent: true });
    const liveReloadScript = `<script>(()=>{let v=${liveVersion};setInterval(async()=>{try{const n=Number(await (await fetch('/__ofs_live')).text());if(n&&n!==v)location.reload()}catch{}},450)})();</script>`;
    const mime = { '.html': 'text/html; charset=utf-8', '.css': 'text/css; charset=utf-8', '.js': 'text/javascript; charset=utf-8', '.mjs': 'text/javascript; charset=utf-8', '.odl': 'text/odl; charset=utf-8', '.oes': 'text/oes; charset=utf-8', '.svg': 'image/svg+xml', '.png': 'image/png', '.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.webp': 'image/webp' };
    const liveServer = http.createServer((request, response) => {
      const requested = decodeURIComponent((request.url || '/').split('?')[0]);
      if (requested === '/__ofs_live') {
        response.writeHead(200, { 'Content-Type': 'text/plain; charset=utf-8', 'Cache-Control': 'no-store' });
        response.end(String(liveVersion));
        return;
      }
      const relative = requested === '/' ? 'index.html' : requested.replace(/^\/+/, '');
      const target = path.resolve(outputDir, relative);
      if (!target.startsWith(path.resolve(outputDir) + path.sep) && target !== outputFile) {
        response.writeHead(403).end('Forbidden');
        return;
      }
      fs.readFile(target, (error, data) => {
        if (error) {
          response.writeHead(404).end('Not found');
          return;
        }
        const ext = path.extname(target).toLowerCase();
        if (ext === '.html') {
          data = Buffer.from(String(data).replace('</body>', `${liveReloadScript}</body>`));
        }
        response.writeHead(200, { 'Content-Type': mime[ext] || 'application/octet-stream', 'Cache-Control': 'no-store' });
        response.end(data);
      });
    });
    await new Promise((resolve, reject) => liveServer.once('error', reject).listen(port, '127.0.0.1', resolve));
    const scheduleRebuild = () => {
      clearTimeout(rebuildTimer);
      rebuildTimer = setTimeout(() => compileLiveProject(), 120);
    };
    const watchers = [fs.watch(file, scheduleRebuild)];
    for (const entry of fs.readdirSync(cwd)) {
      if (/\.(oes|js|mjs|css)$/i.test(entry)) {
        watchers.push(fs.watch(path.join(cwd, entry), scheduleRebuild));
      }
    }
    activeExecution = { paused: false, terminal: null, childProcess: null, shellExecution: null, sourceFile: file, liveServer, watchers };
    await setExecutionContext(true, false);
    const url = `http://127.0.0.1:${port}`;
    vscode.window.showInformationMessage(`ODL Go Live running with live reload at ${url}`, 'Open Browser').then((choice) => {
      if (choice === 'Open Browser') vscode.env.openExternal(vscode.Uri.parse(url));
    });
    return;
  }

  const checkResult = await runExecFile(ofsPath, ['check', file], { cwd });
  if (checkResult.error) {
    const combined = `${checkResult.stdout || ''}\n${checkResult.stderr || ''}`;
    const firstError = parseFirstCompilerError(combined);
    vscode.window.showErrorMessage(firstError?.message ? `OFS: ${firstError.message}` : 'OFS Go Live check failed.');
    return;
  }

  await terminateActiveExecution({ silent: true });

  const terminal = getOrCreateExecutionTerminal(cwd, ofsPath);
  activeExecution = {
    paused: false,
    terminal,
    childProcess: null,
    shellExecution: null,
    sourceFile: file
  };

  await setExecutionContext(true, false);
  terminal.show(true);

  const commandLine = getTerminalRunCommand(ofsPath, file);
  const shellIntegration = await waitForShellIntegration(terminal);
  if (shellIntegration) {
    const compilerName = path.basename(ofsPath || 'ofs').toLowerCase();
    const isOfscc = compilerName.startsWith('ofscc');
    const command = isOfscc ? ofsPath : ofsPath;
    const execution = shellIntegration.executeCommand(command, isOfscc ? [path.basename(file)] : ['run', path.basename(file)]);
    activeExecution.shellExecution = execution;
    execution.exitCode.then(async () => {
      await clearActiveExecution();
    }).catch(async () => {
      await clearActiveExecution();
    });
  } else {
    terminal.sendText(commandLine);
  }

  const url = `http://127.0.0.1:${port}`;
  vscode.window.showInformationMessage(`OFS Go Live running at ${url}`, 'Open Browser').then((choice) => {
    if (choice === 'Open Browser') {
      vscode.env.openExternal(vscode.Uri.parse(url));
    }
  });
}

let ollWebviewPanel = null;
let activeOllDoc = null;

function parseOllNodeTree(text) {
  const lines = text.split(/\r?\n/);
  const root = { kind: 'root', text: '', props: {}, children: [] };
  const stack = [root];

  for (const rawLine of lines) {
    const line = rawLine.trim();
    if (!line || line.startsWith('//') || line.startsWith('/*')) continue;

    if (line === '}') {
      if (stack.length > 1) stack.pop();
      continue;
    }

    const openIdx = line.indexOf('{');
    if (openIdx !== -1) {
      const header = line.substring(0, openIdx).trim();
      const tagMatch = header.match(/^([a-zA-Z0-9_-]+)(?:\s+"([^"]*)")?/);
      if (tagMatch) {
        const node = {
          kind: tagMatch[1].toLowerCase(),
          text: tagMatch[2] || '',
          props: {},
          children: []
        };
        stack[stack.length - 1].children.push(node);
        stack.push(node);
      }
      continue;
    }

    const colonIdx = line.indexOf(':');
    if (colonIdx !== -1 && stack.length > 1) {
      const key = line.substring(0, colonIdx).trim();
      let val = line.substring(colonIdx + 1).trim();
      if (val.startsWith('"') && val.endsWith('"')) val = val.slice(1, -1);
      stack[stack.length - 1].props[key] = val;
    }
  }

  return root.children[0] || root;
}

function renderOllNodeToHtml(node) {
  if (!node) return '';
  const p = node.props || {};
  const styles = [];

  if (p.width) styles.push(`width: ${p.width}px;`);
  if (p.height) styles.push(`height: ${p.height}px;`);
  if (p.bg_color) styles.push(`background-color: ${p.bg_color};`);
  if (p.fg_color) styles.push(`color: ${p.fg_color};`);
  if (p.padding) styles.push(`padding: ${p.padding}px;`);
  if (p.margin) styles.push(`margin: ${p.margin}px;`);
  if (p.border_width) styles.push(`border: ${p.border_width}px solid ${p.border_color || '#444'};`);
  if (p.gap) styles.push(`gap: ${p.gap}px;`);

  const inner = (node.children || []).map(renderOllNodeToHtml).join('\n');
  const styleStr = styles.join(' ');
  const title = node.text || '';

  switch (node.kind) {
    case 'window':
      return `
        <div class="oll-window" style="${styleStr}">
          <div class="oll-titlebar">
            <div class="oll-title">${title || 'OLL Window'}</div>
            <div class="oll-controls"><span>─</span><span>□</span><span>✕</span></div>
          </div>
          <div class="oll-content">${inner}</div>
        </div>`;
    case 'column':
      return `<div class="oll-column" style="display:flex; flex-direction:column; ${styleStr}">${inner}</div>`;
    case 'row':
      return `<div class="oll-row" style="display:flex; flex-direction:row; align-items:center; ${styleStr}">${inner}</div>`;
    case 'card':
      return `<div class="oll-card" style="display:flex; flex-direction:column; border-radius:6px; ${styleStr}"><div class="oll-card-title">${title}</div>${inner}</div>`;
    case 'header':
      return `<div class="oll-header" style="font-weight:700; font-size:1.15em; ${styleStr}">${title}${inner}</div>`;
    case 'button':
      return `<button class="oll-button" style="cursor:pointer; border-radius:4px; font-weight:600; ${styleStr}">${title}${inner}</button>`;
    case 'label':
      return `<div class="oll-label" style="${styleStr}">${title}${inner}</div>`;
    case 'badge':
      return `<span class="oll-badge" style="display:inline-flex; align-items:center; justify-content:center; border-radius:999px; font-size:0.75em; font-weight:700; padding:2px 8px; ${styleStr}">${title}</span>`;
    case 'progress': {
      const pct = p.gap || 50;
      return `<div class="oll-progress" style="background:#222; border-radius:4px; height:12px; overflow:hidden; ${styleStr}"><div style="background:${p.border_color || '#a6e3a1'}; width:${pct}%; height:100%;"></div></div>`;
    }
    case 'input':
      return `<input class="oll-input" type="text" placeholder="${title}" value="${p.value || ''}" style="border-radius:4px; padding:6px 10px; background:#111; color:#fff; border:1px solid #444; ${styleStr}" />`;
    case 'checkbox':
      return `<label class="oll-checkbox" style="display:inline-flex; align-items:center; gap:6px; cursor:pointer; ${styleStr}"><input type="checkbox" ${p.active === 'true' ? 'checked' : ''} /><span>${title}</span></label>`;
    case 'editor':
      return `<div class="oll-editor" style="font-family:monospace; white-space:pre; border-radius:4px; ${styleStr}">${title || inner}</div>`;
    default:
      return `<div class="oll-box oll-${node.kind}" style="${styleStr}">${title}${inner}</div>`;
  }
}

function generateOllPreviewHtml(ollText) {
  const tree = parseOllNodeTree(ollText);
  const body = renderOllNodeToHtml(tree);
  return `<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>OLL Live Preview</title>
  <style>
    * { box-sizing: border-box; }
    body {
      margin: 0;
      padding: 16px;
      background: #0e0e16;
      color: #cdd6f4;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      display: flex;
      justify-content: center;
      align-items: flex-start;
      min-height: 100vh;
    }
    .oll-window {
      border-radius: 8px;
      box-shadow: 0 16px 40px rgba(0,0,0,0.6);
      overflow: hidden;
      display: flex;
      flex-direction: column;
      border: 1px solid rgba(255,255,255,0.1);
      width: 100%;
      max-width: 1200px;
    }
    .oll-titlebar {
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 8px 14px;
      background: rgba(0,0,0,0.3);
      border-bottom: 1px solid rgba(255,255,255,0.08);
      font-size: 12px;
      font-weight: 600;
      color: #a6adc8;
      user-select: none;
    }
    .oll-controls span {
      display: inline-block;
      margin-left: 8px;
      opacity: 0.6;
      cursor: pointer;
    }
    .oll-controls span:hover { opacity: 1; }
    .oll-content {
      padding: 12px;
      flex: 1;
      display: flex;
      flex-direction: column;
    }
    .oll-button:hover {
      filter: brightness(1.15);
      transform: translateY(-1px);
    }
    .oll-button:active {
      transform: translateY(0);
    }
  </style>
</head>
<body>
  ${body}
</body>
</html>`;
}

function showOllLivePreview(document) {
  if (!document) {
    const editor = vscode.window.activeTextEditor;
    if (editor) document = editor.document;
  }
  if (!document) return;

  activeOllDoc = document;
  const fileName = path.basename(document.fileName);

  if (ollWebviewPanel) {
    ollWebviewPanel.reveal(vscode.ViewColumn.Beside);
  } else {
    ollWebviewPanel = vscode.window.createWebviewPanel(
      'ofs.ollPreview',
      `OLL Preview: ${fileName}`,
      vscode.ViewColumn.Beside,
      { enableScripts: true }
    );

    ollWebviewPanel.onDidDispose(() => {
      ollWebviewPanel = null;
    });
  }

  ollWebviewPanel.title = `OLL Preview: ${fileName}`;
  ollWebviewPanel.webview.html = generateOllPreviewHtml(document.getText());
}

function updateOllWebview(document) {
  if (ollWebviewPanel && activeOllDoc && document.uri.toString() === activeOllDoc.uri.toString()) {
    ollWebviewPanel.webview.html = generateOllPreviewHtml(document.getText());
  }
}

function runCurrentFile() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ofs') {
    vscode.window.showErrorMessage('Open an .ofs file to run.');
    return;
  }

  const document = editor.document;
  document.save().then(async () => {
    const ofsPath = await resolveReadyOfsPath();
    if (!ofsPath) {
      vscode.window.showErrorMessage('OFS compiler not found. Enable ofs.autoInstallCompiler or configure ofs.path.');
      return;
    }

    const file = document.fileName;
    const cwd = path.dirname(file);
    const terminal = getOrCreateExecutionTerminal(cwd, ofsPath);

    const checkResult = await runExecFile(ofsPath, ['check', file], { cwd });
    if (checkResult.error) {
      const combined = `${checkResult.stdout || ''}\n${checkResult.stderr || ''}`;
      const diagnostics = parseDiagnostics(document, combined);
      const firstError = parseFirstCompilerError(combined);

      if (diagnostics.length > 0) {
        const collection = vscode.languages.createDiagnosticCollection('ofs-run');
        collection.set(document.uri, diagnostics);
        setTimeout(() => collection.dispose(), 15000);
      }

      if (firstError?.line && firstError?.col) {
        vscode.window.showErrorMessage(`OFS line ${firstError.line}, col ${firstError.col}: ${firstError.message}`);
      } else if (firstError?.message) {
        vscode.window.showErrorMessage(`OFS: ${firstError.message}`);
      } else {
        vscode.window.showErrorMessage('OFS failed to run.');
      }
      return;
    }

    activeExecution = {
      paused: false,
      terminal,
      childProcess: null,
      shellExecution: null,
      sourceFile: file
    };

    await setExecutionContext(true, false);
    terminal.show(true);

    const commandLine = getTerminalRunCommand(ofsPath, file);
    const shellIntegration = await waitForShellIntegration(terminal);
    if (shellIntegration) {
      const compilerName = path.basename(ofsPath || 'ofs').toLowerCase();
      const isOfscc = compilerName.startsWith('ofscc');
      const execution = shellIntegration.executeCommand(ofsPath, isOfscc ? [path.basename(file)] : ['run', path.basename(file)]);
      activeExecution.shellExecution = execution;
      execution.exitCode.then(async () => {
        await clearActiveExecution();
      }).catch(async () => {
        await clearActiveExecution();
      });
      return;
    }

    terminal.sendText(commandLine);
  });
}

async function compileForNativeRun(document) {
  if (compilerInstallPromise) {
    await compilerInstallPromise;
  }

  const ofsPath = getOfsPathForNativeBuild();
  const cwd = path.dirname(document.fileName);
  const outDir = extensionContextRef?.globalStorageUri?.fsPath
    ? path.join(extensionContextRef.globalStorageUri.fsPath, 'native-bin')
    : path.join(cwd, '.ofs-bin');

  ensureDir(outDir);

  const baseName = path.basename(document.fileName, path.extname(document.fileName)).replace(/[^a-zA-Z0-9_-]/g, '_');
  const exeName = process.platform === 'win32' ? `${baseName}.exe` : baseName;
  const exePath = path.join(outDir, exeName);

  const buildResult = await runExecFile(ofsPath, ['build', document.fileName, '-o', exePath], { cwd });
  if (buildResult.error) {
    const combined = `${buildResult.stdout || ''}\n${buildResult.stderr || ''}`;
    const firstError = parseFirstCompilerError(combined);
    if (firstError?.line && firstError?.col) {
      vscode.window.showErrorMessage(`OFS build error line ${firstError.line}, col ${firstError.col}: ${firstError.message}`);
    } else {
      vscode.window.showErrorMessage(`OFS build failed: ${combined.trim() || 'unknown error'}`);
    }
    return null;
  }

  if (!fs.existsSync(exePath)) {
    const combined = `${buildResult.stdout || ''}\n${buildResult.stderr || ''}`.trim();
    const message = combined || 'OFS build terminou sem gerar o executavel esperado.';
    vscode.window.showErrorMessage(`OFS build failed: ${message}`);
    return null;
  }

  if (process.platform !== 'win32') {
    try {
      fs.chmodSync(exePath, 0o755);
    } catch {
      // Ignore permission adjustment failures and let execution report if needed.
    }
  }

  return { exePath, cwd, sourceFile: document.fileName };
}

function buildNativeDebugConfig(exePath, cwd) {
  const config = {
    name: 'OFS Native Launch',
    request: 'launch',
    program: exePath,
    cwd,
    args: [],
    stopAtEntry: false,
    externalConsole: false
  };

  if (process.platform === 'darwin') {
    return {
      ...config,
      type: 'cppdbg',
      MIMode: 'lldb'
    };
  }

  return {
    ...config,
    type: 'cppdbg',
    MIMode: 'gdb'
  };
}

function hasExtension(id) {
  return !!vscode.extensions.getExtension(id);
}

function getNativeDebuggerConfig(exePath, cwd) {
  if (hasExtension('ms-vscode.cpptools')) {
    return buildNativeDebugConfig(exePath, cwd);
  }

  if (hasExtension('vadimcn.vscode-lldb')) {
    return {
      name: 'OFS Native Launch (LLDB)',
      type: 'lldb',
      request: 'launch',
      program: exePath,
      cwd,
      args: []
    };
  }

  return null;
}

function getExecutableRunCommand(exePath) {
  const escaped = exePath.replace(/"/g, '\\"');
  if (process.platform === 'win32') {
    const terminalConfig = vscode.workspace.getConfiguration('terminal.integrated');
    const defaultProfile = terminalConfig.get('defaultProfile.windows', '');
    if (/cmd|command\s*prompt/i.test(defaultProfile)) {
      return `"${escaped}"`;
    }
    return `& "${escaped}"`;
  }
  return `"${escaped}"`;
}

async function ensureNativeDebuggerAvailable() {
  let debugConfig = getNativeDebuggerConfig('placeholder', process.cwd());
  if (debugConfig) {
    return true;
  }

  vscode.window.showInformationMessage('Instalando extensao de depuracao C/C++ (ms-vscode.cpptools)...');
  try {
    await vscode.commands.executeCommand('workbench.extensions.installExtension', 'ms-vscode.cpptools');
  } catch {
    // Checked below again.
  }

  debugConfig = getNativeDebuggerConfig('placeholder', process.cwd());
  return !!debugConfig;
}

function createNativeLaunchTemplate() {
  return [
    {
      name: 'OFS: Executar arquivo atual',
      type: 'ofs-native',
      request: 'launch',
      noDebug: true
    },
    {
      name: 'OFS: Depurar arquivo atual',
      type: 'ofs-native',
      request: 'launch'
    }
  ];
}

async function resolveOfsLaunchConfiguration(config) {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ofs') {
    vscode.window.showErrorMessage('Abra um arquivo .ofs para executar ou depurar.');
    return undefined;
  }

  if (config?.noDebug) {
    runCurrentFile();
    return undefined;
  }

  await editor.document.save();
  const compiled = await compileForNativeRun(editor.document);
  if (!compiled) {
    return undefined;
  }

  let debugConfig = getNativeDebuggerConfig(compiled.exePath, compiled.cwd);
  if (!config?.noDebug && !debugConfig) {
    const available = await ensureNativeDebuggerAvailable();
    if (!available) {
      vscode.window.showErrorMessage('Depurador nativo indisponivel. Recarregue o VS Code apos instalar o depurador C/C++.');
      return undefined;
    }
    debugConfig = getNativeDebuggerConfig(compiled.exePath, compiled.cwd);
  }

  if (!debugConfig) {
    vscode.window.showErrorMessage('Falha ao configurar depurador nativo para OFS.');
    return undefined;
  }

  return {
    ...debugConfig,
    ...config,
    type: debugConfig.type,
    request: 'launch',
    name: config?.name || (config?.noDebug ? 'OFS: Executar arquivo atual' : 'OFS: Depurar arquivo atual'),
    program: compiled.exePath,
    cwd: compiled.cwd,
    args: Array.isArray(config?.args) ? config.args : [],
    externalConsole: false,
    console: 'internalConsole',
    internalConsoleOptions: 'openOnSessionStart',
    logging: {
      engineLogging: false,
      trace: false,
      traceResponse: false,
      moduleLoad: false,
      programOutput: true
    }
  };
}

async function runCurrentFileNative() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ofs') {
    vscode.window.showErrorMessage('Open an .ofs file to run.');
    return;
  }

  await editor.document.save();
  const compiled = await compileForNativeRun(editor.document);
  if (!compiled) return;

  await runCompiledExecutableInTerminal(compiled, 'OFS Native Run');
}

async function emitCurrentAssembly() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ofs') {
    vscode.window.showErrorMessage('Open an OFS file first.');
    return;
  }

  await editor.document.save();

  const ofsPath = await resolveReadyOfsPath();
  if (!ofsPath) {
    vscode.window.showErrorMessage('OFS compiler not found. Enable ofs.autoInstallCompiler or configure ofs.path.');
    return;
  }

  const document = editor.document;
  const parsed = path.parse(document.fileName);
  const asmBase = path.join(parsed.dir, parsed.name);
  const cwd = parsed.dir || getWorkspaceRoot() || undefined;
  const result = await runExecFile(ofsPath, ['asm', document.fileName, '-o', asmBase], { cwd });
  const combined = `${result.stdout || ''}${result.stderr || ''}`;

  if (result.error) {
    vscode.window.showErrorMessage(`OFS asm failed: ${combined.trim() || result.error.message}`);
    return;
  }

  const asmPath = `${asmBase}.s`;
  if (!fs.existsSync(asmPath)) {
    vscode.window.showErrorMessage('OFS asm terminou sem gerar o arquivo .s esperado.');
    return;
  }

  const doc = await vscode.workspace.openTextDocument(asmPath);
  await vscode.window.showTextDocument(doc, { preview: false });
  vscode.window.showInformationMessage(`OFS assembly emitted: ${path.basename(asmPath)}`);
}

async function debugCurrentFileNative() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ofs') {
    vscode.window.showErrorMessage('Open an .ofs file to debug.');
    return;
  }

  await editor.document.save();
  const compiled = await compileForNativeRun(editor.document);
  if (!compiled) return;

  const workspaceFolder = vscode.workspace.getWorkspaceFolder(editor.document.uri);
  let debugConfig = getNativeDebuggerConfig(compiled.exePath, compiled.cwd);
  if (!debugConfig) {
    vscode.window.showInformationMessage('Installing C/C++ debugger extension (ms-vscode.cpptools)...');
    try {
      await vscode.commands.executeCommand('workbench.extensions.installExtension', 'ms-vscode.cpptools');
    } catch {
      // keep handling below with explicit message
    }

    debugConfig = getNativeDebuggerConfig(compiled.exePath, compiled.cwd);
    if (!debugConfig) {
      vscode.window.showErrorMessage('Native debugger is not available yet. Reload VS Code after debugger extension installation.');
      return;
    }
  }

  const ok = await vscode.debug.startDebugging(workspaceFolder, debugConfig);
  if (!ok) {
    vscode.window.showWarningMessage('Native Debug could not start. Verify debugger extension setup.');
  }
}

function checkCurrentFile(diagnosticCollection) {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ofs') {
    vscode.window.showErrorMessage('Open an .ofs file to check.');
    return;
  }

  editor.document.save().then(async () => {
    await runOfsCheck(editor.document, diagnosticCollection);
    vscode.window.showInformationMessage('OFS check completed.');
  });
}

function registerCompletionProvider(context) {
  const provider = vscode.languages.registerCompletionItemProvider('ofs', {
    provideCompletionItems(document, position) {
      const keywordItems = KEYWORDS.map((keyword) => {
        const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
        item.insertText = keyword;
        return item;
      });

      const linePrefix = document.lineAt(position).text.slice(0, position.character);
      const inAttach = /\b(?:attach|import)\s*\{\s*[\w.-]*$/.test(linePrefix);

      const moduleItems = MODULES.map((moduleName) => {
        const item = new vscode.CompletionItem(moduleName, vscode.CompletionItemKind.Module);
        item.insertText = moduleName;
        item.detail = 'OFS stdlib/package module';
        return item;
      });

      if (inAttach) {
        return moduleItems;
      }

      const typeItems = TYPES.map((typeName) => {
        const item = new vscode.CompletionItem(typeName, vscode.CompletionItemKind.TypeParameter);
        item.insertText = typeName;
        item.detail = 'OFS type';
        return item;
      });

      const builtinItems = [...BUILTINS, ...LOW_LEVEL_INTRINSICS, ...STDLIB_COMPLETIONS].map((name) => {
        const item = new vscode.CompletionItem(name, vscode.CompletionItemKind.Function);
        item.insertText = name;
        item.detail = LOW_LEVEL_INTRINSICS.includes(name) ? 'OFS fault intrinsic' : 'OFS function/helper';
        return item;
      });

      const attachItems = collectAttachSymbols(document).map((symbol) => {
        const item = new vscode.CompletionItem(symbol.name, symbol.kind);
        item.insertText = symbol.name;
        item.detail = 'From attached library';
        return item;
      });

      return [...keywordItems, ...typeItems, ...moduleItems, ...builtinItems, ...attachItems];
    }
  }, '.', '{');

  context.subscriptions.push(provider);
}

// ── Package / library helper functions ──────────────────────────────────

function getInstalledPackagesDir() {
  if (process.platform === 'win32') {
    return path.join(process.env.APPDATA || os.homedir(), 'ofs', 'libs');
  }
  return path.join(os.homedir(), '.ofs', 'libs');
}

function getInstalledPackageInfo(name) {
  const pkgJsonPath = path.join(getInstalledPackagesDir(), name, 'ofspkg.json');
  if (!fs.existsSync(pkgJsonPath)) return null;
  try {
    return JSON.parse(fs.readFileSync(pkgJsonPath, 'utf8'));
  } catch {
    return null;
  }
}

async function fetchRegistryCached() {
  const now = Date.now();
  if (registryCache && (now - registryCacheTime) < REGISTRY_CACHE_TTL_MS) {
    return registryCache;
  }
  try {
    const data = await httpGetJson(REGISTRY_URL);
    registryCache = data;
    registryCacheTime = now;
    return data;
  } catch {
    return registryCache; // return stale cache on error, or null
  }
}

function getRegistryStableVersion(registry, name) {
  if (!Array.isArray(registry?.packages)) return null;
  const pkg = registry.packages.find((p) => p.name === name);
  return pkg?.channels?.stable || pkg?.channels?.latest || null;
}

function isVersionNewer(installedVer, registryVer) {
  // Returns true if registryVer is strictly newer than installedVer
  const toNum = (v) => (v || '0').split('.').map(Number);
  const a = toNum(installedVer);
  const b = toNum(registryVer);
  for (let i = 0; i < 3; i++) {
    if ((b[i] || 0) > (a[i] || 0)) return true;
    if ((b[i] || 0) < (a[i] || 0)) return false;
  }
  return false;
}

function ensureLibDecorationTypes() {
  if (!installedLibDecoration) {
    installedLibDecoration = vscode.window.createTextEditorDecorationType({
      color: '#7B4FBF',
      fontWeight: 'bold'
    });
  }
  if (!outdatedLibDecoration) {
    outdatedLibDecoration = vscode.window.createTextEditorDecorationType({
      color: '#C48A00',
      fontWeight: 'bold',
      textDecoration: 'underline dotted'
    });
  }
}

async function updateAttachDecorations(editor) {
  if (!editor || editor.document.languageId !== 'ofs') return;
  ensureLibDecorationTypes();

  const document = editor.document;
  const installedRanges = [];
  const outdatedRanges = [];

  let registry = null;
  try { registry = await fetchRegistryCached(); } catch { /* continue without registry */ }

  for (let i = 0; i < document.lineCount; i++) {
    const lineText = document.lineAt(i).text;
    const m = lineText.match(ATTACH_LIB_REGEX);
    if (!m) continue;

    const libName = m[1];
    const braceIdx = lineText.indexOf('{');
    const nameStart = lineText.indexOf(libName, braceIdx);
    if (nameStart < 0) continue;

    const range = new vscode.Range(
      new vscode.Position(i, nameStart),
      new vscode.Position(i, nameStart + libName.length)
    );

    const isStdlib = Object.prototype.hasOwnProperty.call(STDLIB_NAMES, libName);
    if (isStdlib) {
      installedRanges.push(range);
      continue;
    }

    const info = getInstalledPackageInfo(libName);
    if (!info) continue; // not installed — no decoration

    const registryVersion = getRegistryStableVersion(registry, libName);
    if (registryVersion && isVersionNewer(info.version, registryVersion)) {
      outdatedRanges.push(range);
    } else {
      installedRanges.push(range);
    }
  }

  editor.setDecorations(installedLibDecoration, installedRanges);
  editor.setDecorations(outdatedLibDecoration, outdatedRanges);
}

function registerAttachDecorations(context) {
  const refresh = (editor) => { if (editor) updateAttachDecorations(editor).catch(() => {}); };

  context.subscriptions.push(
    vscode.window.onDidChangeActiveTextEditor(refresh),
    vscode.workspace.onDidChangeTextDocument((e) => {
      const editor = vscode.window.activeTextEditor;
      if (editor && e.document === editor.document) refresh(editor);
    })
  );

  if (vscode.window.activeTextEditor) {
    refresh(vscode.window.activeTextEditor);
  }

  context.subscriptions.push({
    dispose() {
      if (installedLibDecoration) { installedLibDecoration.dispose(); installedLibDecoration = null; }
      if (outdatedLibDecoration)  { outdatedLibDecoration.dispose();  outdatedLibDecoration  = null; }
    }
  });
}

// ── File decorations (explorer colors) ───────────────────────────────────

function registerFileDecorations(context) {
  const provider = {
    provideFileDecoration(uri) {
      if (!uri.fsPath.toLowerCase().endsWith('.ofs')) {
        return undefined;
      }

      return {
        tooltip: 'Obsidian Fault Script file',
        color: new vscode.ThemeColor('charts.purple')
      };
    }
  };

  context.subscriptions.push(vscode.window.registerFileDecorationProvider(provider));
}

function registerHoverProvider(context) {
  const provider = vscode.languages.registerHoverProvider('ofs', {
    async provideHover(document, position) {
      const range = document.getWordRangeAtPosition(position);
      if (!range) {
        // operador `?` de propagacao (nao e uma palavra)
        const qLine = document.lineAt(position.line).text;
        if (qLine[position.character] === '?' && !/^\s*\/\//.test(qLine)) {
          const qr = new vscode.Range(position.line, position.character, position.line, position.character + 1);
          const qmd = new vscode.MarkdownString();
          qmd.appendMarkdown('**?** (propagação)\n\n`expr?` desembrulha uma variante de `strata`: `Solid(v)?` e `Ore(v)?` valem `v`; `Crack(e)?` e `Barren?` saem da `vein` atual na hora (rodando os `seal`/`anchor` pendentes), propagando a falha.\n\n');
          qmd.appendMarkdown('Exemplo:\n');
          qmd.appendCodeblock('strata Outcome { Solid, Crack }\n\nvein ok() -> stone {\n    forge v = Solid(7)?\n    return v\n}', 'ofs');
          return new vscode.Hover(qmd, qr);
        }
        return undefined;
      }

      const word = document.getText(range);

      // Check if this word is a library name inside an attach {} on this line
      const lineText = document.lineAt(position.line).text;
      const attachMatch = lineText.match(ATTACH_LIB_REGEX);
      if (attachMatch && attachMatch[1] === word) {
        const libName = word;
        const md = new vscode.MarkdownString();
        md.isTrusted = false;

        const isStdlib = Object.prototype.hasOwnProperty.call(STDLIB_NAMES, libName);
        if (isStdlib) {
          md.appendMarkdown(`**${libName}** — *OFS stdlib*\n\n`);
          md.appendMarkdown('Biblioteca padrão instalada com o compilador OFS.');
          return new vscode.Hover(md, range);
        }

        const info = getInstalledPackageInfo(libName);
        if (info) {
          md.appendMarkdown(`**${libName}** — v${info.version || '?'}\n\n`);
          if (info.description) md.appendMarkdown(`${info.description}\n\n`);

          let registry = null;
          try { registry = await fetchRegistryCached(); } catch { /* no registry */ }
          const registryVersion = getRegistryStableVersion(registry, libName);
          if (registryVersion && isVersionNewer(info.version, registryVersion)) {
            md.appendMarkdown(`> $(warning) **Atualização disponível: v${registryVersion}** — execute \`reinfuse ${libName}\``);
          } else if (registryVersion) {
            md.appendMarkdown(`> $(check) Versão atual (estável: v${registryVersion})`);
          }
          return new vscode.Hover(md, range);
        }

        // Not installed
        md.appendMarkdown(`**${libName}** — *não instalado*\n\n`);
        md.appendMarkdown(`Execute \`infuse ${libName}\` para instalar.`);
        return new vscode.Hover(md, range);
      }

      // Fallback: keyword hover docs
      const doc = HOVER_DOCS[word];
      if (!doc) return undefined;

      const md = new vscode.MarkdownString();
      md.appendMarkdown(`**${word}**\n\n${doc.description}\n\n`);
      if (doc.context) {
        md.appendMarkdown(`**Contexto:** ${doc.context}\n\n`);
      }
      md.appendMarkdown('Exemplo:\n');
      md.appendCodeblock(doc.example, 'ofs');
      md.isTrusted = false;

      return new vscode.Hover(md, range);
    }
  });

  context.subscriptions.push(provider);
}

// Formatacao: `ofs fmt --stdin` (ferramenta escrita em OFS). Para formatar ao salvar,
// use a opcao padrao do VS Code: "[ofs]": { "editor.formatOnSave": true }.
// `fmt` e um comando do wrapper `ofs`; o compilador embutido/do workspace pode
// nao te-lo. Tenta o `ofs.path` configurado, o `ofs` do PATH e o caminho usado
// pelo resto da extensao, nessa ordem, e fica com o primeiro que funcionar.
async function runOfsFmt(text, cwd) {
  const configured = vscode.workspace.getConfiguration().get('ofs.path', 'ofs');
  const candidates = [...new Set([configured, 'ofs', getOfsPath()].filter(Boolean))];
  let lastError = null;
  for (const bin of candidates) {
    try {
      return await runOfsFmtWith(bin, text, cwd);
    } catch (e) {
      lastError = e;
    }
  }
  throw lastError || new Error('ofs fmt indisponivel');
}

function runOfsFmtWith(bin, text, cwd) {
  return new Promise((resolve, reject) => {
    const proc = cp.spawn(bin, ['fmt', '--stdin'], { cwd, windowsHide: true });
    const out = [];
    const err = [];
    const timer = setTimeout(() => {
      proc.kill();
      reject(new Error('ofs fmt: tempo esgotado'));
    }, 30000);
    proc.stdout.on('data', (d) => out.push(d));
    proc.stderr.on('data', (d) => err.push(d));
    proc.on('error', (e) => {
      clearTimeout(timer);
      reject(e);
    });
    proc.on('close', (code) => {
      clearTimeout(timer);
      if (code !== 0) {
        reject(new Error(Buffer.concat(err).toString('utf8').trim() || `ofs fmt saiu com codigo ${code}`));
        return;
      }
      resolve(Buffer.concat(out).toString('utf8'));
    });
    proc.stdin.on('error', () => {});
    proc.stdin.end(text, 'utf8');
  });
}

function registerFormattingProvider(context) {
  const provider = vscode.languages.registerDocumentFormattingEditProvider('ofs', {
    async provideDocumentFormattingEdits(document) {
      if (!vscode.workspace.getConfiguration().get('ofs.format.enable', true)) {
        return [];
      }
      const folder = vscode.workspace.getWorkspaceFolder(document.uri);
      const cwd = folder ? folder.uri.fsPath : path.dirname(document.uri.fsPath);
      let formatted;
      try {
        formatted = await runOfsFmt(document.getText(), cwd);
      } catch (e) {
        vscode.window.showWarningMessage(`OFS: nao foi possivel formatar (${e.message})`);
        return [];
      }
      if (!formatted || formatted === document.getText()) {
        return [];
      }
      const whole = new vscode.Range(document.positionAt(0), document.positionAt(document.getText().length));
      return [vscode.TextEdit.replace(whole, formatted)];
    }
  });
  context.subscriptions.push(provider);
}

// ── ofs fossil (Ideia 2): mostrar no editor um ponto de uma gravacao ──────
// Roda `ofs fossil replay <arq.fossil> --to <alvo>`, mostra o estado (locais
// e globais) no canal "OFS Fossil" e marca a linha do passo no fonte. Proximo
// e anterior reexecutam com --to passo+1 / passo-1 (o replay e deterministico).
let fossilChannel = null;
let fossilDecoration = null;
const fossilState = { file: '', step: 0, target: '1' };

function readFossilSource(fossilPath) {
  try {
    const buf = fs.readFileSync(fossilPath);
    const nl1 = buf.indexOf(10);
    if (nl1 < 0 || buf.slice(0, nl1).toString() !== 'OFS-FOSSIL 1') {
      return '';
    }
    const nl2 = buf.indexOf(10, nl1 + 1);
    const parts = buf.slice(nl1 + 1, nl2).toString().split(' ');
    if (parts[0] !== 'S') {
      return '';
    }
    const n = parseInt(parts[2], 10);
    return buf.slice(nl2 + 1, nl2 + 1 + n).toString();
  } catch (_err) {
    return '';
  }
}

function fossilSourceFor(mainSrc, base) {
  if (!mainSrc) {
    return '';
  }
  if (path.basename(mainSrc) === base) {
    return mainSrc;
  }
  const sibling = path.join(path.dirname(mainSrc), base);
  return fs.existsSync(sibling) ? sibling : '';
}

function runFossilTarget(fossilPath, target) {
  if (!fossilChannel) {
    fossilChannel = vscode.window.createOutputChannel('OFS Fossil');
  }
  if (!fossilDecoration) {
    fossilDecoration = vscode.window.createTextEditorDecorationType({
      isWholeLine: true,
      backgroundColor: new vscode.ThemeColor('editor.stackFrameHighlightBackground'),
      overviewRulerColor: new vscode.ThemeColor('editorOverviewRuler.infoForeground')
    });
  }
  fossilState.file = fossilPath;
  fossilState.target = target;
  const args = ['fossil', 'replay', fossilPath, '--to', String(target)];
  cp.execFile(getOfsPath(), args, { cwd: path.dirname(fossilPath), maxBuffer: 16 * 1024 * 1024 }, async (_error, stdout, stderr) => {
    fossilChannel.clear();
    fossilChannel.append(`$ ofs ${args.join(' ')}\n${stdout || ''}${stderr || ''}`);
    fossilChannel.show(true);
    const m = /^fossil: passo (\d+) \| ([^|]*):(\d+) \|/m.exec(stdout || '');
    if (!m) {
      vscode.window.showWarningMessage(`ofs fossil: ${(stderr || '').trim() || 'ponto nao encontrado'}`);
      return;
    }
    fossilState.step = parseInt(m[1], 10);
    const src = fossilSourceFor(readFossilSource(fossilPath), m[2].trim());
    if (!src) {
      return;
    }
    const doc = await vscode.workspace.openTextDocument(src);
    const editor = await vscode.window.showTextDocument(doc, { preserveFocus: false, preview: false });
    const line = Math.max(0, parseInt(m[3], 10) - 1);
    const range = new vscode.Range(line, 0, line, 0);
    editor.setDecorations(fossilDecoration, [range]);
    editor.revealRange(range, vscode.TextEditorRevealType.InCenterIfOutsideViewport);
    vscode.window.setStatusBarMessage(`fossil: passo ${fossilState.step}`, 8000);
  });
}

async function fossilReplayCommand() {
  let fossilPath = '';
  const active = vscode.window.activeTextEditor;
  if (active && active.document.fileName.endsWith('.fossil')) {
    fossilPath = active.document.fileName;
  } else if (fossilState.file) {
    fossilPath = fossilState.file;
  }
  if (!fossilPath) {
    const pick = await vscode.window.showOpenDialog({ canSelectMany: false, filters: { Fossil: ['fossil'] } });
    if (!pick || pick.length === 0) {
      return;
    }
    fossilPath = pick[0].fsPath;
  }
  const target = await vscode.window.showInputBox({
    prompt: 'ofs fossil: ir para (passo, L<linha>, arquivo.ofs:<linha> ou nome da vein)',
    value: fossilState.target
  });
  if (!target) {
    return;
  }
  runFossilTarget(fossilPath, target.trim());
}

function fossilStepCommand(delta) {
  if (!fossilState.file || fossilState.step <= 0) {
    return fossilReplayCommand();
  }
  runFossilTarget(fossilState.file, String(Math.max(1, fossilState.step + delta)));
}

// ── Fault Refract: lava (rascunho ao vivo do editor) ──────────────────────
// O `ofs refract` (local e --remote) le, para cada arquivo observado, o
// rascunho em <raiz>/lava/<caminho absoluto sem a 1a barra> quando ele mudou
// por ultimo; senao, o arquivo do disco. Raiz: POSIX
// ${XDG_RUNTIME_DIR:-/tmp}/ofs-refract-<uid>; Windows %TEMP%\ofs-refract
// (no Windows "C:\a\b.ofs" -> lava\C\a\b.ofs). Cada refract viva deixa
// <raiz>/pulse/<pid>; sem pulse de processo vivo a extensao nao grava nada.
// Mesma regra em ofs/ofscc/interpreter.ofs (rf_lava_init) e no wrapper.
const LAVA_DELAY_MS = 250;

function lavaRoot() {
  if (process.platform === 'win32') {
    return path.join(process.env.TEMP || process.env.TMP || os.tmpdir(), 'ofs-refract');
  }
  const base = process.env.XDG_RUNTIME_DIR || '/tmp';
  return path.join(base, `ofs-refract-${process.getuid()}`);
}

function lavaPathFor(root, filePath) {
  let abs = path.resolve(filePath);
  if (process.platform === 'win32') {
    abs = abs.replace(/\\/g, '/').replace(/^([A-Za-z]):/, '$1');
  }
  return path.join(root, 'lava', abs.replace(/^\/+/, ''));
}

// ha uma refract viva? (pasta 0700 do proprio usuario e um pulse de pid vivo)
function lavaSessionAlive(root) {
  try {
    const st = fs.lstatSync(root);
    if (!st.isDirectory()) {
      return false;
    }
    if (process.platform !== 'win32' && (st.uid !== process.getuid() || (st.mode & 0o077) !== 0)) {
      return false;
    }
    for (const name of fs.readdirSync(path.join(root, 'pulse'))) {
      if (!/^\d+$/.test(name)) {
        continue;
      }
      try {
        process.kill(Number(name), 0);
        return true;
      } catch (err) {
        if (err && err.code === 'EPERM') {
          return true;
        }
      }
    }
  } catch (_) {
    return false;
  }
  return false;
}

function isLavaDocument(document) {
  if (!document || document.uri.scheme !== 'file') {
    return false;
  }
  const name = document.fileName.toLowerCase();
  return name.endsWith('.ofs') || name.endsWith('.oal');
}

function registerRefractLava(context) {
  const timers = new Map();

  const enabled = () => vscode.workspace.getConfiguration().get('ofs.refract.liveEdits', true)
    && process.env.OFS_REFRACT_DRAFT !== '0';

  const cancel = (key) => {
    const t = timers.get(key);
    if (t) {
      clearTimeout(t);
      timers.delete(key);
    }
  };

  // apaga o rascunho: a refract volta a usar o arquivo do disco
  const drop = (document) => {
    const key = document.uri.toString();
    cancel(key);
    try {
      fs.unlinkSync(lavaPathFor(lavaRoot(), document.fileName));
    } catch (_) {
      // ja nao existia
    }
  };

  const write = (document) => {
    const key = document.uri.toString();
    timers.delete(key);
    if (document.isClosed || !document.isDirty) {
      drop(document);
      return;
    }
    const root = lavaRoot();
    if (!enabled() || !lavaSessionAlive(root)) {
      return;
    }
    const target = lavaPathFor(root, document.fileName);
    try {
      fs.mkdirSync(path.dirname(target), { recursive: true, mode: 0o700 });
      // tmp + rename: a refract nunca le um rascunho pela metade
      const tmp = path.join(path.dirname(target), `.${path.basename(target)}.${process.pid}.tmp`);
      fs.writeFileSync(tmp, document.getText(), { mode: 0o600 });
      fs.renameSync(tmp, target);
    } catch (_) {
      // sem lava: a refract segue com o disco
    }
  };

  context.subscriptions.push(
    vscode.workspace.onDidChangeTextDocument((event) => {
      const document = event.document;
      if (!isLavaDocument(document)) {
        return;
      }
      const key = document.uri.toString();
      if (!document.isDirty) {
        // desfez tudo: o conteudo voltou ao do disco
        drop(document);
        return;
      }
      if (!enabled()) {
        return;
      }
      cancel(key);
      timers.set(key, setTimeout(() => write(document), LAVA_DELAY_MS));
    }),
    vscode.workspace.onDidSaveTextDocument((document) => {
      if (isLavaDocument(document)) {
        drop(document);
      }
    }),
    vscode.workspace.onDidCloseTextDocument((document) => {
      if (isLavaDocument(document)) {
        drop(document);
      }
    }),
    {
      dispose() {
        for (const document of vscode.workspace.textDocuments) {
          if (isLavaDocument(document)) {
            drop(document);
          }
        }
      }
    }
  );
}

function activate(context) {
  extensionContextRef = context;
  if (context.globalStorageUri?.fsPath) {
    ensureDir(context.globalStorageUri.fsPath);
  }

  setExecutionContext(false, false);

  const diagnosticCollection = vscode.languages.createDiagnosticCollection('ofs');
  context.subscriptions.push(diagnosticCollection);

  const nativeRunCmd = vscode.commands.registerCommand('ofs.runNative', () => runCurrentFile());

  const nativeDebugCmd = vscode.commands.registerCommand('ofs.debugNative', async () => {
    const workspaceFolder = vscode.workspace.workspaceFolders?.[0];
    await vscode.debug.startDebugging(workspaceFolder, {
      type: 'ofs-native',
      request: 'launch',
      name: 'OFS: Depurar arquivo atual',
      noDebug: false
    });
  });

  const checkCmd = vscode.commands.registerCommand('ofs.checkFile', () => checkCurrentFile(diagnosticCollection));
  const asmCmd = vscode.commands.registerCommand('ofs.emitAssembly', () => emitCurrentAssembly());
  const goLiveCmd = vscode.commands.registerCommand('ofs.goLive', () => goLiveCurrentFile());
  const pauseCmd = vscode.commands.registerCommand('ofs.pauseExecution', () => pauseActiveExecution());
  const resumeCmd = vscode.commands.registerCommand('ofs.resumeExecution', () => resumeActiveExecution());
  const stopCmd = vscode.commands.registerCommand('ofs.stopExecution', () => terminateActiveExecution());
  const runOllCmd = vscode.commands.registerCommand('ofs.runOll', () => runOllNative());
  const previewOllCmd = vscode.commands.registerCommand('ofs.previewOll', () => showOllLivePreview());
  const fossilReplayCmd = vscode.commands.registerCommand('ofs.fossilReplay', () => fossilReplayCommand());
  const fossilNextCmd = vscode.commands.registerCommand('ofs.fossilNext', () => fossilStepCommand(1));
  const fossilBackCmd = vscode.commands.registerCommand('ofs.fossilBack', () => fossilStepCommand(-1));
  context.subscriptions.push(
    nativeRunCmd,
    nativeDebugCmd,
    checkCmd,
    asmCmd,
    goLiveCmd,
    pauseCmd,
    resumeCmd,
    stopCmd,
    runOllCmd,
    previewOllCmd,
    fossilReplayCmd,
    fossilNextCmd,
    fossilBackCmd,
    {
      dispose() {
        stopExecutionWatcher();
      }
    }
  );

  const debugProvider = vscode.debug.registerDebugConfigurationProvider(
    'ofs-native',
    {
      provideDebugConfigurations() {
        return createNativeLaunchTemplate();
      },
      async resolveDebugConfiguration(_folder, config) {
        if (!config || Object.keys(config).length === 0) {
          return resolveOfsLaunchConfiguration({ request: 'launch', noDebug: false });
        }
        return resolveOfsLaunchConfiguration(config);
      }
    },
    vscode.DebugConfigurationProviderTriggerKind.Dynamic
  );

  context.subscriptions.push(debugProvider);
  registerCompletionProvider(context);
  registerFileDecorations(context);
  registerHoverProvider(context);
  registerAttachDecorations(context);
  registerFormattingProvider(context);
  registerRefractLava(context);

  ensureCompilerInstalledOnActivate();

  let timer = null;
  const refreshDiagnostics = (document) => {
    const onType = vscode.workspace.getConfiguration().get('ofs.diagnosticsOnType', true);
    if (!onType) {
      return;
    }

    if (timer) {
      clearTimeout(timer);
    }

    timer = setTimeout(() => runOfsCheck(document, diagnosticCollection), 450);
  };

  context.subscriptions.push(
    vscode.workspace.onDidOpenTextDocument((document) => {
      runOfsCheck(document, diagnosticCollection);
    }),
    vscode.workspace.onDidSaveTextDocument((document) => {
      runOfsCheck(document, diagnosticCollection);
      updateOllWebview(document);
    }),
    vscode.workspace.onDidChangeTextDocument((event) => {
      refreshDiagnostics(event.document);
      updateOllWebview(event.document);
    }),
    vscode.workspace.onDidCloseTextDocument((document) => diagnosticCollection.delete(document.uri))
  );

  if (vscode.window.activeTextEditor) {
    runOfsCheck(vscode.window.activeTextEditor.document, diagnosticCollection);
  }
}

function deactivate() {}

module.exports = {
  activate,
  deactivate
};
