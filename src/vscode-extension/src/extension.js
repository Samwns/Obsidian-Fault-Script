const vscode = require('vscode');
const cp = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');
const https = require('https');
const http = require('http');

const KEYWORDS = [
  'core', 'vein', 'forge', 'const', 'monolith', 'impl', 'namespace', 'strata', 'extern', 'rift',
  'if', 'else', 'while', 'cycle', 'match', 'case', 'default',
  'return', 'break', 'continue', 'throw', 'tremor', 'catch',
  'fracture', 'abyss', 'bedrock', 'obsid', 'attach', 'import', 'intent', 'tectonic', 'pure', 'impure', 'fractal',
  'layout', 'bind', 'abi', 'packed', 'native', 'system',
  'asm', 'echo', 'in', 'shard',
  'u8', 'u16', 'u32', 'u64', 'i8', 'i16', 'i32',
  'true', 'false', 'null', 'as'
];

const TYPES = ['stone', 'crystal', 'obsidian', 'bool', 'void', 'Array', 'u8', 'u16', 'u32', 'u64', 'i8', 'i16', 'i32'];

const MODULES = [
  'core', 'math', 'string', 'io', 'oes', 'ofshtml', 'odl', 'webserver', 'webui', 'bedrock', 'bedrock-packet',
  'terminal-colors', 'memory-modes', 'rift', 'canvas', 'window', 'fmt', 'test-lib'
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
  'fault_unreachable', 'fault_memcpy', 'fault_memset'
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
  'fmt_pad_right', 'fmt_pad_left', 'fmt_center', 'fmt_zero_pad', 'fmt_sign',
  'fmt_truncate', 'fmt_repeat', 'fmt_separator', 'fmt_upper', 'fmt_lower', 'fmt_trim',
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
  'input.mouse_x', 'input.mouse_y', 'input.is_down', 'input.key'
];

const HOVER_DOCS = {
  core: {
    description: 'Entry point do programa OFS.',
    context: 'Use para definir o ponto inicial do programa (main).',
    example: 'core main() {\n    echo("hello")\n}'
  },
  vein: {
    description: 'Declara uma funcao.',
    context: 'Use quando precisar reutilizar logica em blocos nomeados.',
    example: 'vein add(a: stone, b: stone) -> stone {\n    return a + b\n}'
  },
  forge: {
    description: 'Declara variavel (tipada ou inferida).',
    context: 'Use para criar variaveis locais em funcoes/blocos.',
    example: 'forge x: stone = 10\nforge y = 20'
  },
  const: {
    description: 'Declara variavel imutavel.',
    context: 'Use para valores que nao devem mudar durante a execucao.',
    example: 'const max: stone = 100'
  },
  impl: {
    description: 'Declara metodos para um monolith.',
    context: 'Use para associar comportamento ao tipo e permitir chamadas como `obj.metodo()`.',
    example: 'impl Rect {\n    vein area(self) -> stone {\n        return self.w * self.h\n    }\n}'
  },
  namespace: {
    description: 'Agrupa simbolos em um escopo nomeado.',
    context: 'Use para organizar APIs e evitar colisao de nomes.',
    example: 'namespace mathx {\n    vein square(x: stone) -> stone {\n        return x * x\n    }\n}'
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
  cycle: {
    description: 'Loop range-based ou C-style (3 partes).',
    context: 'Use para iteracoes controladas por faixa, colecoes ou contador.',
    example: 'cycle (n in nums) {\n    echo(n)\n}\ncycle (forge i = 0; i < 10; i++) {\n    echo(i)\n}'
  },
  while: {
    description: 'Loop por condicao.',
    context: 'Use quando o numero de repeticoes depende de estado dinamico.',
    example: 'while (x > 0) {\n    x -= 1\n}'
  },
  fracture: {
    description: 'Bloco seguro para operacoes com ponteiros.',
    context: 'Use para manipulacao de ponteiros com restricoes de seguranca.',
    example: 'fracture {\n    shard p: *stone = &x;\n    *p = 42\n}'
  },
  abyss: {
    description: 'Bloco de memoria irrestrita (unsafe).',
    context: 'Use apenas quando precisar de operacoes de memoria sem guardrails.',
    example: 'abyss {\n    // operacoes sem restricoes de seguranca\n}'
  },
  fractal: {
    description: 'Bloco intermediario de efeitos.',
    context: 'Sintaxe planejada/documentada; no compilador atual, `tectonic fractal` ainda e tratado como bloco comum.',
    example: 'fractal {\n    echo("effect-lifted block")\n}'
  },
  bedrock: {
    description: 'Bloco typed low-level nativo da OFS.',
    context: 'Use para combinar ponteiros, intrinsics fault_* e armazenamento explicito sem cair no modo raw de abyss.',
    example: 'bedrock {\n    shard p: *stone = &x\n    *p = *p + fault_spin_left(1, 3)\n}'
  },
  rift: {
    description: 'Declaracao de interop nativa da OFS.',
    context: 'Use como fronteira de chamada para simbolos externos sem apresentar isso apenas como extern C.',
    example: 'rift vein strlen(text: obsidian) -> stone'
  },
  abi: {
    description: 'Metadado de ABI para fronteiras de interop.',
    context: 'Use em `rift vein` e `extern vein` para declarar a ABI pretendida, como `c` ou `system`.',
    example: 'rift vein text_size(text: obsidian) -> stone bind "strlen" abi c'
  },
  bind: {
    description: 'Metadado de símbolo externo.',
    context: 'Use para ligar um nome OFS a um símbolo externo diferente.',
    example: 'rift vein text_size(text: obsidian) -> stone bind "strlen" abi c'
  },
  layout: {
    description: 'Metadado de layout para monolith.',
    context: 'Use para declarar intenção de layout em tipos ABI-facing.',
    example: 'monolith Header layout packed {\n    tag: stone\n    flags: stone\n}'
  },
  packed: {
    description: 'Modo de layout compacto para monolith.',
    context: 'Use em `monolith ... layout packed` quando precisar de estrutura compactada no lowering atual.',
    example: 'monolith Header layout packed {\n    tag: stone\n    flags: stone\n}'
  },
  tectonic: {
    description: 'Diretiva prefixo para modos: fracture, abyss ou fractal.',
    context: 'Use para selecionar explicitamente o modo de memoria do bloco. Hoje o compiler aceita fracture, abyss e bedrock como blocos equivalentes para codegen.',
    example: 'tectonic fracture {\n    shard p: *stone = &x\n}\ntectonic bedrock {\n    fault_fence()\n}'
  },
  obsid: {
    description: 'Fechamento de bloco no estilo ": ... obsid".',
    example: 'if (x > 0):\n    echo("ok")\nobsid'
  },
  attach: {
    description: 'Importa modulo OFS por nome ou arquivo.',
    context: 'Use para trazer funcoes/monoliths de bibliotecas stdlib ou arquivos .ofs externos.',
    example: 'attach {terminal-colors}\n// ou para arquivo especifico:\nattach {F:helpers.ofs}'
  },
  extern: {
    description: 'Declara funcao externa (runtime/C).',
    example: 'extern vein ofs_pow(base: crystal, exp: crystal) -> crystal'
  },
  match: {
    description: 'Pattern matching com case/default.',
    example: 'match code {\n    case 200: { echo("ok") }\n    default: { echo("other") }\n}'
  },
  tremor: {
    description: 'Bloco de tratamento de erro.',
    example: 'tremor {\n    throw "boom"\n} catch (e: obsidian) {\n    echo(e)\n}'
  },
  throw: {
    description: 'Lanca erro dentro de fluxo tremor/catch.',
    example: 'throw "invalid value"'
  },
  as: {
    description: 'Cast explicito de tipo.',
    context: 'Use quando precisar converter um valor entre tipos compativeis.',
    example: 'forge y: crystal = x as crystal'
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
  asm: {
    description: 'Comando CLI para emitir assembly nativo do alvo.',
    context: 'Use para inspecionar o lowering final sem abandonar OFS como fonte de verdade.',
    example: 'ofs asm main.ofs -o main'
  },
  fault_count: {
    description: 'Intrinsic low-level: population count.',
    context: 'Disponivel em blocos bedrock, fracture e abyss.',
    example: 'bedrock {\n    echo(fault_count(0xF0F0))\n}'
  },
  fault_lead: {
    description: 'Intrinsic low-level: conta zeros a esquerda.',
    context: 'Disponivel em blocos bedrock, fracture e abyss.',
    example: 'bedrock {\n    echo(fault_lead(x))\n}'
  },
  fault_trail: {
    description: 'Intrinsic low-level: conta zeros a direita.',
    context: 'Disponivel em blocos bedrock, fracture e abyss.',
    example: 'bedrock {\n    echo(fault_trail(x))\n}'
  },
  fault_swap: {
    description: 'Intrinsic low-level: byte swap.',
    context: 'Disponivel em blocos bedrock, fracture e abyss.',
    example: 'bedrock {\n    echo(fault_swap(x))\n}'
  },
  fault_spin_left: {
    description: 'Intrinsic low-level: rotacao para a esquerda.',
    context: 'Disponivel em blocos bedrock, fracture e abyss.',
    example: 'bedrock {\n    echo(fault_spin_left(x, 5))\n}'
  },
  fault_spin_right: {
    description: 'Intrinsic low-level: rotacao para a direita.',
    context: 'Disponivel em blocos bedrock, fracture e abyss.',
    example: 'bedrock {\n    echo(fault_spin_right(x, 5))\n}'
  },
  fault_step: {
    description: 'Intrinsic low-level: avanca ponteiro tipado por quantidade de elementos.',
    context: 'Disponivel em blocos bedrock, fracture e abyss. Ideal para regioes e tabelas bedrock.',
    example: 'bedrock {\n    shard slot: *stone = fault_step(base, 2)\n    *slot = 42\n}'
  },
  fault_cut: {
    description: 'Intrinsic low-level: extrai campo de bits.',
    context: 'Disponivel em blocos bedrock, fracture e abyss para leitura de layouts e headers.',
    example: 'bedrock {\n    forge opcode: stone = fault_cut(header, 8, 8)\n}'
  },
  fault_patch: {
    description: 'Intrinsic low-level: reescreve campo de bits.',
    context: 'Disponivel em blocos bedrock, fracture e abyss para montar headers e estruturas compactadas.',
    example: 'bedrock {\n    forge patched: stone = fault_patch(header, 40, 8, 0x2A)\n}'
  },
  fault_fence: {
    description: 'Intrinsic low-level: emite barreira de memoria.',
    context: 'Aceito pelo compilador atual como intrinsic no-op/headless; reservado para barreira real no backend.',
    example: 'bedrock {\n    fault_fence()\n}'
  },
  fault_prefetch: {
    description: 'Intrinsic low-level: solicita prefetch de um alvo em memoria.',
    context: 'Aceito pelo compilador atual como intrinsic no-op/headless; reservado para prefetch real no backend.',
    example: 'bedrock {\n    fault_prefetch(ptr)\n}'
  },
  fault_trap: {
    description: 'Intrinsic low-level: emite trap de maquina.',
    context: 'Documentado como roadmap; use com cuidado ate o lowering hard-fail ser exposto no compilador self-hosted.',
    example: 'bedrock {\n    fault_trap()\n}'
  },
  fault_weave: {
    description: 'Intrinsic low-level da OFS para entrelacar bits por mascara.',
    context: 'Combina bits de dois valores usando uma mascara, indo alem das operacoes classicas expostas diretamente em assembly.',
    example: 'bedrock {\n    echo(fault_weave(0xFF00, left, right))\n}'
  },
  rift_text_size: {
    description: 'Wrapper de stdlib para tamanho de texto via rift.',
    context: 'Use `attach {rift}` para consumir fronteiras externas com API OFS.',
    example: 'attach {rift}\necho(rift_text_size("fault"))'
  },
  window: {
    description: 'Modulo nativo de janela e input da OFS.',
    context: 'Use `attach {window}` para criar janela, ler input e apresentar um buffer.',
    example: 'attach {window}\nwindow.create("demo", 640, 480)'
  },
  canvas: {
    description: 'Modulo nativo/instavel de canvas da OFS.',
    context: 'Use `attach {canvas}` para trabalhar com buffer de pixels junto com `window`.',
    example: 'attach {canvas}\nforge cv = canvas.create(64, 48)\ncanvas.set_pixel(cv, 1, 1, 0xFF0000)'
  },
  bedrock_region_new: {
    description: 'Cria uma regiao low-level de stones zerada.',
    context: 'Use para buffers/tabelas de tamanho fixo geridos em OFS low-level.',
    example: 'attach {bedrock}\nforge region = bedrock_region_new(4)'
  },
  bedrock_prefetch: {
    description: 'Helper OFS para solicitar prefetch de uma regiao bedrock.',
    context: 'Use antes de ler slots quentes em regioes low-level sem expor chamadas de intrinsic em toda a base.',
    example: 'bedrock_prefetch(region, 0)'
  },
  bedrock_view_read: {
    description: 'Le um valor a partir de uma view bedrock sobre uma regiao.',
    context: 'Use para janelas/slices de tabelas low-level sem sair da API OFS.',
    example: 'echo(bedrock_view_read(region, 1, 0))'
  },
  bedrock_lane8_get: {
    description: 'Helper OFS para extrair um lane de 8 bits.',
    context: 'Use em headers e layouts compactados sem escrever shifts diretamente.',
    example: 'echo(bedrock_lane8_get(header, 0))'
  },
  bedrock_lane16_le_get: {
    description: 'Helper OFS para extrair uma janela little-endian de 16 bits.',
    context: 'Use em tipos ABI-facing e headers quando o layout local segue ordem little-endian.',
    example: 'echo(bedrock_lane16_le_get(header, 0))'
  },
  bedrock_lane16_be_get: {
    description: 'Helper OFS para extrair uma janela big-endian de 16 bits.',
    context: 'Use em headers de rede e protocolos sem espalhar byte swap manual pelo codigo.',
    example: 'echo(bedrock_lane16_be_get(header, 3))'
  },
  bedrock_packet_opcode_be16: {
    description: 'Helper de pacote para ler a janela big-endian de 16 bits do cabecalho.',
    context: 'Use quando um header OFS precisa expor um campo protocolar em ordem de rede.',
    example: 'echo(bedrock_packet_opcode_be16(header))'
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
  asm: {
    description: 'Escape hatch de inline assembly nativo.',
    context: 'Use dentro de blocos bedrock/fracture/abyss para emitir instrucoes de maquina diretamente quando nenhum intrinsic fault_* cobre o caso.',
    example: 'bedrock {\n    asm "nop"\n    asm "int3"\n}'
  },
  echo: {
    description: 'Funcao built-in de saida (print).',
    context: 'Use para imprimir qualquer valor OFS: stone, crystal, obsidian ou bool. Exibe nova linha automaticamente.',
    example: 'echo("Hello, World!")\necho(42)\necho(3.14)'
  },
  in: {
    description: 'Palavra-chave de iteracao range-based em cycle.',
    context: 'Use para iterar sobre colecoes (forjas de array) de forma expressiva.',
    example: 'cycle (n in nums) {\n    echo(n)\n}'
  },
  shard: {
    description: 'Declara variavel ponteiro tipado.',
    context: 'Use dentro de fracture ou bedrock para criar ponteiros seguros para valores existentes.',
    example: 'fracture {\n    shard p: *stone = &x\n    *p = *p + 1\n}'
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
  return [
    path.join(compilerDir, 'libofs_runtime.a'),
    path.join(compilerDir, 'ofs_runtime.lib'),
    path.join(compilerDir, 'ofs_runtime.o')
  ].some((candidate) => fs.existsSync(candidate));
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
    for (const runtimeFile of ['libofs_runtime.a', 'ofs_runtime.o', 'ofs_runtime.lib']) {
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
  'fmt':             'fmt.ofs',
  'test-lib':        'test_lib.ofs',
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

function parseDiagnostics(document, output) {
  const diagnostics = [];
  const lines = output.split(/\r?\n/);

  for (const line of lines) {
    const match = line.match(DIAGNOSTIC_REGEX);
    if (!match) {
      continue;
    }

    const filePath = match[1].trim();
    if (!filePath.endsWith('.ofs')) {
      continue;
    }

    const lineNum = Number.parseInt(match[2], 10);
    const colNum = Number.parseInt(match[3], 10);
    const message = match[4].trim() || 'Unknown OFS error';

    const range = createRangeFromLineCol(document, lineNum, colNum);
    diagnostics.push(new vscode.Diagnostic(range, message, vscode.DiagnosticSeverity.Error));
  }

  return diagnostics;
}

function parseFirstCompilerError(output) {
  const lines = output.split(/\r?\n/);

  for (const line of lines) {
    const match = line.match(DIAGNOSTIC_REGEX);
    if (!match) {
      continue;
    }

    return {
      line: Number.parseInt(match[2], 10),
      col: Number.parseInt(match[3], 10),
      message: match[4].trim() || 'Unknown OFS error'
    };
  }

  const fallback = output.trim();
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

let studioIdlePanel = null;
let studioIdleStatusBarItem = null;

function showStudioIdleWebview(context) {
  if (studioIdlePanel) {
    studioIdlePanel.reveal(vscode.ViewColumn.One);
    return;
  }

  const editor = vscode.window.activeTextEditor;
  let activeName = 'main.ofs';
  let activeContent = '';
  let activeRelPath = 'idle/main.ofs';

  const ws = getWorkspaceRoot();
  const wsFiles = [];
  if (ws) {
    const scanDir = (dir, depth = 0) => {
      if (depth > 2) return;
      try {
        const entries = fs.readdirSync(dir, { withFileTypes: true });
        for (const e of entries) {
          if (e.name.startsWith('.') || e.name === 'node_modules' || e.name === 'dist' || e.name === 'target') continue;
          const full = path.join(dir, e.name);
          if (e.isDirectory()) {
            scanDir(full, depth + 1);
          } else if (e.name.endsWith('.ofs') || e.name.endsWith('.oll') || e.name.endsWith('.md')) {
            wsFiles.push({
              name: e.name,
              relPath: path.relative(ws, full),
              fullPath: full
            });
          }
        }
      } catch {}
    };
    scanDir(ws);
  }

  if (editor && editor.document) {
    activeName = path.basename(editor.document.fileName);
    activeContent = editor.document.getText();
    activeRelPath = ws ? path.relative(ws, editor.document.fileName) : activeName;
  } else if (ws) {
    const candidates = [
      path.join(ws, 'idle', 'main.ofs'),
      path.join(ws, 'src', 'main.ofs'),
      path.join(ws, 'main.ofs')
    ];
    for (const c of candidates) {
      if (fs.existsSync(c)) {
        activeName = path.basename(c);
        activeRelPath = path.relative(ws, c);
        try { activeContent = fs.readFileSync(c, 'utf8'); } catch { }
        break;
      }
    }
  }

  studioIdlePanel = vscode.window.createWebviewPanel(
    'ofs.studioIdle',
    'Obsidian Fault Script :: Studio IDLE',
    vscode.ViewColumn.One,
    {
      enableScripts: true,
      retainContextWhenHidden: true
    }
  );

  studioIdlePanel.onDidDispose(() => {
    studioIdlePanel = null;
  });

  studioIdlePanel.webview.onDidReceiveMessage(async (message) => {
    switch (message.command) {
      case 'run': {
        const doc = vscode.window.activeTextEditor?.document;
        if (doc && doc.languageId === 'ofs') {
          runCurrentFile();
        } else {
          const ws = getWorkspaceRoot();
          const ofsPath = await resolveReadyOfsPath();
          const target = fs.existsSync(path.join(ws, 'idle', 'main.ofs')) ? 'idle/main.ofs' : (fs.existsSync(path.join(ws, 'src', 'main.ofs')) ? 'src/main.ofs' : 'main.ofs');
          const terminal = getOrCreateExecutionTerminal(ws, ofsPath);
          terminal.show(true);
          terminal.sendText(`"${ofsPath}" run "${target}"`);
        }
        break;
      }
      case 'build': {
        const ws = getWorkspaceRoot();
        const ofsPath = await resolveReadyOfsPath();
        const target = fs.existsSync(path.join(ws, 'idle', 'main.ofs')) ? 'idle/main.ofs' : (fs.existsSync(path.join(ws, 'src', 'main.ofs')) ? 'src/main.ofs' : 'main.ofs');
        const terminal = getOrCreateExecutionTerminal(ws, ofsPath);
        terminal.show(true);
        terminal.sendText(`"${ofsPath}" build "${target}"`);
        break;
      }
      case 'save': {
        if (vscode.window.activeTextEditor) {
          await vscode.window.activeTextEditor.document.save();
          vscode.window.showInformationMessage('OFS Studio: Arquivo salvo com sucesso.');
        } else if (message.content && message.filePath) {
          const ws = getWorkspaceRoot();
          if (ws) {
            const fullP = path.isAbsolute(message.filePath) ? message.filePath : path.join(ws, message.filePath);
            try {
              fs.writeFileSync(fullP, message.content, 'utf8');
              vscode.window.showInformationMessage(`OFS Studio: ${path.basename(fullP)} salvo.`);
            } catch (err) {
              vscode.window.showErrorMessage(`Falha ao salvar: ${err.message}`);
            }
          }
        }
        break;
      }
      case 'openFile': {
        const ws = getWorkspaceRoot();
        if (ws && message.filePath) {
          const fullPath = path.isAbsolute(message.filePath) ? message.filePath : path.join(ws, message.filePath);
          if (fs.existsSync(fullPath)) {
            try {
              const content = fs.readFileSync(fullPath, 'utf8');
              studioIdlePanel.webview.postMessage({
                type: 'loadFile',
                filePath: message.filePath,
                fileName: path.basename(fullPath),
                content: content
              });
              const doc = await vscode.workspace.openTextDocument(fullPath);
              await vscode.window.showTextDocument(doc, vscode.ViewColumn.Beside);
            } catch (e) {
              vscode.window.showErrorMessage(`Erro ao abrir arquivo: ${e.message}`);
            }
          }
        }
        break;
      }
      case 'showToast': {
        if (message.text) {
          vscode.window.showInformationMessage(message.text);
        }
        break;
      }
    }
  });

  studioIdlePanel.webview.html = generateStudioIdleHtml(activeName, activeContent, activeRelPath, wsFiles);
}

function updateStudioIdleWebview(document) {
  if (studioIdlePanel && document) {
    studioIdlePanel.webview.postMessage({
      type: 'syncDocument',
      fileName: path.basename(document.fileName),
      filePath: vscode.workspace.asRelativePath(document.fileName),
      content: document.getText()
    });
  }
}

function generateStudioIdleHtml(initialFileName, initialContent, initialRelPath = '', wsFiles = []) {
  const safeContent = (initialContent || '')
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');

  const fileTreeHtml = (wsFiles && wsFiles.length > 0)
    ? wsFiles.map(f => {
        const isOfs = f.name.endsWith('.ofs');
        const isOll = f.name.endsWith('.oll');
        const stroke = isOfs ? '#C678DD' : (isOll ? '#98C379' : '#61AFEF');
        const isActive = f.name === initialFileName ? 'active' : '';
        const escaped = f.relPath.replace(/'/g, "\\'");
        return `<div class="tree-node ${isActive}" onclick="loadFile('${escaped}')">
          <svg class="svg-icon" viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="${stroke}" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/></svg>
          <span>${f.name}</span>
        </div>`;
      }).join('\n')
    : `<div class="tree-node active" onclick="loadFile('${(initialRelPath || initialFileName).replace(/'/g, "\\'")}')">
        <svg class="svg-icon" viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="#C678DD" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"/><polyline points="14 2 14 8 20 8"/></svg>
        <span>${initialFileName}</span>
      </div>`;

  return `<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Obsidian Fault Script :: Studio IDLE</title>
  <style>
    :root {
      --bg-window: #181825;
      --bg-header: #181825;
      --bg-sidebar: #1e1e2e;
      --bg-editor: #181825;
      --bg-panel: #181825;
      --bg-statusbar: #181825;
      --accent: #8957e5;
      --accent-hover: #7b42f6;
      --accent-fg: #ffffff;
      --fg-primary: #cdd6f4;
      --fg-secondary: #a6adc8;
      --fg-muted: #6c7086;
      --border-color: #313244;
      --btn-bg: #313244;
      --btn-hover: #45475a;
      --btn-active: #585b70;
      --gutter-bg: #1e1e2e;
      --card-radius: 8px;
      --btn-radius: 5px;
    }

    [data-theme="gtk"] {
      --bg-window: #242424;
      --bg-header: #303030;
      --bg-sidebar: #282828;
      --bg-editor: #1e1e1e;
      --bg-panel: #242424;
      --bg-statusbar: #242424;
      --accent: #3584e4;
      --accent-hover: #1c71d8;
      --accent-fg: #ffffff;
      --fg-primary: #ffffff;
      --fg-secondary: #deddda;
      --fg-muted: #9a9996;
      --border-color: #383838;
      --btn-bg: #383838;
      --btn-hover: #484848;
      --btn-active: #545454;
      --gutter-bg: #282828;
      --card-radius: 10px;
      --btn-radius: 6px;
    }

    [data-theme="windows"] {
      --bg-window: #202020;
      --bg-header: #202020;
      --bg-sidebar: #262626;
      --bg-editor: #1c1c1c;
      --bg-panel: #1c1c1c;
      --bg-statusbar: #202020;
      --accent: #60cdff;
      --accent-hover: #4cc2ff;
      --accent-fg: #000000;
      --fg-primary: #ffffff;
      --fg-secondary: #cccccc;
      --fg-muted: #888888;
      --border-color: #333333;
      --btn-bg: #2d2d2d;
      --btn-hover: #383838;
      --btn-active: #444444;
      --gutter-bg: #262626;
      --card-radius: 8px;
      --btn-radius: 4px;
    }

    [data-theme="neon"] {
      --bg-window: #0b0b14;
      --bg-header: #0f0f1c;
      --bg-sidebar: #0d0d19;
      --bg-editor: #0a0a10;
      --bg-panel: #0d0d19;
      --bg-statusbar: #0b0b14;
      --accent: #00ffcc;
      --accent-hover: #00e6b8;
      --accent-fg: #000000;
      --fg-primary: #00ffcc;
      --fg-secondary: #ff79c6;
      --fg-muted: #6272a4;
      --border-color: #1f1f38;
      --btn-bg: #16162a;
      --btn-hover: #222240;
      --btn-active: #303058;
      --gutter-bg: #0d0d19;
      --card-radius: 4px;
      --btn-radius: 2px;
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
      background: var(--bg-window);
      color: var(--fg-primary);
      height: 100vh;
      display: flex;
      flex-direction: column;
      overflow: hidden;
      user-select: none;
    }

    .svg-icon {
      display: inline-block;
      vertical-align: middle;
      flex-shrink: 0;
    }

    header {
      height: 38px;
      background: var(--bg-header);
      border-bottom: 1px solid var(--border-color);
      display: flex;
      align-items: center;
      padding: 0 12px;
      gap: 12px;
      font-size: 13px;
    }
    .logo-container {
      display: flex;
      align-items: center;
      gap: 8px;
      font-weight: 700;
      color: #c678dd;
    }
    .menu-bar {
      display: flex;
      gap: 4px;
    }
    .menu-item {
      padding: 4px 8px;
      border-radius: var(--btn-radius);
      color: var(--fg-secondary);
      cursor: pointer;
      transition: background 0.15s, color 0.15s;
    }
    .menu-item:hover {
      background: var(--btn-hover);
      color: #fff;
    }
    .search-box {
      flex: 1;
      max-width: 480px;
      margin: 0 auto;
      background: var(--btn-bg);
      border: 1px solid var(--border-color);
      border-radius: var(--btn-radius);
      padding: 4px 12px;
      display: flex;
      align-items: center;
      gap: 8px;
      color: var(--fg-muted);
      font-size: 12px;
      cursor: pointer;
    }
    .theme-selector {
      display: flex;
      gap: 4px;
      align-items: center;
    }
    .theme-btn {
      padding: 3px 8px;
      font-size: 11px;
      border-radius: var(--btn-radius);
      background: var(--btn-bg);
      color: var(--fg-secondary);
      border: 1px solid var(--border-color);
      cursor: pointer;
    }
    .theme-btn.active, .theme-btn:hover {
      background: var(--accent);
      color: var(--accent-fg);
      border-color: var(--accent);
    }

    .main-workspace {
      flex: 1;
      display: flex;
      overflow: hidden;
    }

    .activity-bar {
      width: 50px;
      background: var(--bg-sidebar);
      border-right: 1px solid var(--border-color);
      display: flex;
      flex-direction: column;
      align-items: center;
      padding: 10px 0;
      gap: 12px;
    }
    .act-btn {
      width: 40px;
      height: 40px;
      border-radius: var(--btn-radius);
      display: flex;
      align-items: center;
      justify-content: center;
      color: var(--fg-muted);
      cursor: pointer;
      position: relative;
      transition: all 0.15s;
    }
    .act-btn:hover {
      color: var(--accent-hover);
      background: var(--btn-hover);
    }
    .act-btn.active {
      color: #fff;
    }
    .act-btn.active::before {
      content: "";
      position: absolute;
      left: 0;
      top: 6px;
      bottom: 6px;
      width: 3px;
      background: var(--accent);
      border-radius: 0 2px 2px 0;
    }

    .sidebar {
      width: 230px;
      background: var(--bg-sidebar);
      border-right: 1px solid var(--border-color);
      display: flex;
      flex-direction: column;
    }
    .sidebar-header {
      padding: 10px 14px;
      font-size: 11px;
      font-weight: 700;
      color: var(--fg-muted);
      letter-spacing: 0.8px;
      border-bottom: 1px solid var(--border-color);
    }
    .file-tree {
      flex: 1;
      overflow-y: auto;
      padding: 8px 0;
      font-size: 13px;
    }
    .tree-node {
      padding: 5px 12px;
      display: flex;
      align-items: center;
      gap: 8px;
      cursor: pointer;
      color: var(--fg-secondary);
      border-radius: 4px;
      margin: 1px 6px;
    }
    .tree-node:hover {
      background: var(--btn-hover);
      color: #fff;
    }
    .tree-node.active {
      background: var(--btn-active);
      color: #fff;
      font-weight: 600;
      border-left: 3px solid var(--accent);
    }
    .tree-node.indent-1 { padding-left: 24px; }
    .tree-node.indent-2 { padding-left: 38px; }

    .editor-container {
      flex: 1;
      display: flex;
      flex-direction: column;
      overflow: hidden;
      background: var(--bg-editor);
    }
    .tabs-bar {
      height: 38px;
      background: var(--bg-header);
      border-bottom: 1px solid var(--border-color);
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 8px;
    }
    .tabs-list {
      display: flex;
      height: 100%;
    }
    .tab {
      display: flex;
      align-items: center;
      gap: 8px;
      padding: 0 14px;
      font-size: 13px;
      color: var(--fg-muted);
      cursor: pointer;
      border-right: 1px solid var(--border-color);
      background: var(--bg-header);
      position: relative;
    }
    .tab.active {
      background: var(--bg-editor);
      color: #fff;
    }
    .tab.active::top {
      content: "";
      position: absolute;
      top: 0; left: 0; right: 0;
      height: 2px;
      background: var(--accent);
    }
    .tab .close-tab {
      font-size: 12px;
      border-radius: 3px;
      padding: 2px;
    }
    .tab .close-tab:hover {
      background: var(--btn-hover);
      color: #fff;
    }
    .editor-actions {
      display: flex;
      gap: 8px;
      align-items: center;
    }
    .action-btn {
      padding: 5px 12px;
      border-radius: var(--btn-radius);
      font-size: 12px;
      font-weight: 600;
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 6px;
      border: 1px solid transparent;
      transition: all 0.15s;
    }
    .action-btn.primary {
      background: var(--accent);
      color: var(--accent-fg);
      border-color: var(--accent);
    }
    .action-btn.primary:hover {
      background: var(--accent-hover);
    }
    .action-btn.secondary {
      background: var(--btn-bg);
      color: var(--fg-primary);
      border-color: var(--border-color);
    }
    .action-btn.secondary:hover {
      background: var(--btn-hover);
    }

    .editor-body {
      flex: 1;
      display: flex;
      overflow: hidden;
      position: relative;
    }
    .gutter {
      width: 48px;
      background: var(--gutter-bg);
      border-right: 1px solid var(--border-color);
      padding: 8px 0;
      font-family: "JetBrains Mono", Consolas, "Courier New", monospace;
      font-size: 13px;
      line-height: 20px;
      color: var(--fg-muted);
      text-align: right;
      padding-right: 12px;
      user-select: none;
    }
    .code-textarea {
      flex: 1;
      padding: 8px 14px;
      font-family: "JetBrains Mono", Consolas, "Courier New", monospace;
      font-size: 13px;
      line-height: 20px;
      background: transparent;
      color: var(--fg-primary);
      border: none;
      outline: none;
      resize: none;
      white-space: pre;
      overflow: auto;
      tab-size: 4;
      caret-color: var(--accent);
    }

    .bottom-panel {
      height: 190px;
      background: var(--bg-panel);
      border-top: 1px solid var(--border-color);
      display: flex;
      flex-direction: column;
    }
    .panel-tabs {
      height: 32px;
      border-bottom: 1px solid var(--border-color);
      display: flex;
      align-items: center;
      padding: 0 12px;
      gap: 16px;
      font-size: 11px;
      font-weight: 700;
      letter-spacing: 0.5px;
    }
    .ptab {
      color: var(--fg-muted);
      cursor: pointer;
      display: flex;
      align-items: center;
      gap: 6px;
      height: 100%;
      position: relative;
    }
    .ptab.active {
      color: #fff;
    }
    .ptab.active::after {
      content: "";
      position: absolute;
      bottom: 0; left: 0; right: 0;
      height: 2px;
      background: var(--accent);
    }
    .panel-content {
      flex: 1;
      padding: 10px 14px;
      overflow-y: auto;
      font-family: "JetBrains Mono", Consolas, "Courier New", monospace;
      font-size: 12px;
      line-height: 18px;
    }
    .terminal-line { color: var(--fg-primary); }
    .terminal-line.success { color: #98c379; }
    .terminal-line.error { color: #e06c75; }
    .terminal-line.info { color: #61afef; }
    .terminal-actions {
      display: flex;
      gap: 8px;
      margin-bottom: 8px;
    }

    footer {
      height: 24px;
      background: var(--bg-statusbar);
      border-top: 1px solid var(--border-color);
      color: var(--fg-secondary);
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 0 12px;
      font-size: 11px;
    }
    .status-left, .status-right {
      display: flex;
      align-items: center;
      gap: 14px;
    }
  </style>
</head>
<body data-theme="dark_modern">
  <header>
    <div class="logo-container">
      <svg class="svg-icon" viewBox="0 0 24 24" width="18" height="18" fill="none" stroke="#c678dd" stroke-width="2">
        <polygon points="12,2 22,12 12,22 2,12"/>
        <polygon points="12,6 18,12 12,18 6,12" stroke="#e5c07b"/>
      </svg>
      <span>OFS Studio</span>
    </div>
    <div class="menu-bar">
      <div class="menu-item">Arquivo</div>
      <div class="menu-item">Editar</div>
      <div class="menu-item">Seleção</div>
      <div class="menu-item">Exibir</div>
      <div class="menu-item">Ir</div>
      <div class="menu-item" onclick="triggerRun()">Executar</div>
      <div class="menu-item">Terminal</div>
      <div class="menu-item">Ajuda</div>
    </div>
    <div class="search-box">
      <svg class="svg-icon" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
      <span>Obsidian Fault Script :: Studio IDLE (Ctrl+P)</span>
    </div>
    <div class="theme-selector">
      <span style="font-size:11px; color:var(--fg-muted); margin-right:4px;">Tema:</span>
      <button class="theme-btn active" onclick="setTheme('dark_modern')">Dark Modern</button>
      <button class="theme-btn" onclick="setTheme('gtk')">GTK</button>
      <button class="theme-btn" onclick="setTheme('windows')">Windows</button>
      <button class="theme-btn" onclick="setTheme('neon')">Neon</button>
    </div>
  </header>

  <div class="main-workspace">
    <div class="activity-bar">
      <div class="act-btn active" title="Explorador">
        <svg class="svg-icon" viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="1.8"><path d="M16 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V6a2 2 0 0 0-2-2z"/><path d="M8 2h12a2 2 0 0 1 2 2v14"/></svg>
      </div>
      <div class="act-btn" title="Buscar">
        <svg class="svg-icon" viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8"/><line x1="21" y1="21" x2="16.65" y2="16.65"/></svg>
      </div>
      <div class="act-btn" title="Controle de Versão (Git)">
        <svg class="svg-icon" viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="1.8"><line x1="6" y1="3" x2="6" y2="21"/><circle cx="6" cy="6" r="3"/><circle cx="6" cy="18" r="3"/><path d="M6 12a9 9 0 0 1 9-9"/><circle cx="18" cy="6" r="3"/></svg>
      </div>
      <div class="act-btn" title="Executar & Depurar" onclick="triggerRun()">
        <svg class="svg-icon" viewBox="0 0 24 24" width="22" height="22" fill="currentColor"><polygon points="6,4 20,12 6,20"/></svg>
      </div>
      <div class="act-btn" title="Pacotes & Módulos">
        <svg class="svg-icon" viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="1.8"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/></svg>
      </div>
      <div style="flex:1"></div>
      <div class="act-btn" title="Configurações">
        <svg class="svg-icon" viewBox="0 0 24 24" width="22" height="22" fill="none" stroke="currentColor" stroke-width="1.8"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 1 1-4 0v-.09A1.65 1.65 0 0 0 9 19.4a1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 1 1 0-4h.09A1.65 1.65 0 0 0 4.6 9a1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33H9a1.65 1.65 0 0 0 1-1.51V3a2 2 0 1 1 4 0v.09a1.65 1.65 0 0 0 1 1.51 1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82V9a1.65 1.65 0 0 0 1.51 1H21a2 2 0 1 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>
      </div>
    </div>

    <div class="sidebar">
      <div class="sidebar-header">EXPLORADOR</div>
      <div class="file-tree">
        <div class="tree-node" style="font-weight:600">
          <svg class="svg-icon" viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="#E5C07B" stroke-width="2"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/></svg>
          <span>WORKSPACE</span>
        </div>
        ${fileTreeHtml}
      </div>
    </div>

    <div class="editor-container">
      <div class="tabs-bar">
        <div class="tabs-list" id="tabsListEl">
          <div class="tab active" id="activeTabEl">
            <span style="display:inline-block;width:8px;height:8px;border-radius:50%;background:#c678dd"></span>
            <span id="tabTitle">${initialFileName}</span>
          </div>
        </div>
        <div class="editor-actions">
          <button class="action-btn primary" onclick="triggerRun()">
            <svg class="svg-icon" viewBox="0 0 24 24" width="14" height="14" fill="currentColor"><polygon points="6,4 20,12 6,20"/></svg>
            <span>Run (AOT)</span>
          </button>
          <button class="action-btn secondary" onclick="triggerBuild()">
            <span>🔨 Build</span>
          </button>
          <button class="action-btn secondary" onclick="triggerSave()">
            <span>💾 Salvar</span>
          </button>
        </div>
      </div>

      <div class="editor-body">
        <div class="gutter" id="gutterEl">1<br>2<br>3<br>4<br>5<br>6<br>7<br>8<br>9<br>10<br>11<br>12<br>13<br>14<br>15<br>16<br>17<br>18<br>19<br>20</div>
        <textarea class="code-textarea" id="codeEditor" spellcheck="false" oninput="onCodeChanged()" onkeydown="onKeyDown(event)">${safeContent}</textarea>
      </div>

      <div class="bottom-panel">
        <div class="panel-tabs">
          <div class="ptab active" onclick="switchPanelTab(0)">
            <svg class="svg-icon" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><polyline points="4 17 10 11 4 5"/><line x1="12" y1="19" x2="20" y2="19"/></svg>
            <span>TERMINAL</span>
          </div>
          <div class="ptab" onclick="switchPanelTab(1)">
            <svg class="svg-icon" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="#e06c75" stroke-width="2"><circle cx="12" cy="12" r="10"/><line x1="12" y1="8" x2="12" y2="12"/><line x1="12" y1="16" x2="12.01" y2="16"/></svg>
            <span>PROBLEMAS (0)</span>
          </div>
          <div class="ptab" onclick="switchPanelTab(2)">
            <svg class="svg-icon" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="3" width="20" height="14" rx="2" ry="2"/><line x1="8" y1="21" x2="16" y2="21"/><line x1="12" y1="17" x2="12" y2="21"/></svg>
            <span>SAÍDA</span>
          </div>
          <div class="ptab" onclick="switchPanelTab(3)">
            <span>OLL PREVIEW</span>
          </div>
        </div>
        <div class="panel-content" id="panelContentEl">
          <div class="terminal-actions">
            <button class="action-btn secondary" style="font-size:11px;padding:3px 8px;" onclick="triggerRun()">▶ Executar (AOT)</button>
            <button class="action-btn secondary" style="font-size:11px;padding:3px 8px;" onclick="triggerBuild()">🔨 Compilar</button>
            <button class="action-btn secondary" style="font-size:11px;padding:3px 8px;" onclick="clearTerminal()">🗑 Limpar</button>
          </div>
          <div class="terminal-line info">samns@linux-ofs:~/Obsidian-Fault-Script$ ofs --version</div>
          <div class="terminal-line">Obsidian Fault Script Compiler (ofscc) v0.1 - AOT Linux x86_64</div>
          <div class="terminal-line success">[OFS IDLE] Studio IDLE integrado com sucesso ao VS Code.</div>
          <div class="terminal-line">Pressione 'Run (AOT)' ou use o terminal para compilar arquivos nativos.</div>
          <div class="terminal-line info">samns@linux-ofs:~/Obsidian-Fault-Script$ <span style="display:inline-block;width:7px;height:14px;background:#cdd6f4;vertical-align:middle;"></span></div>
        </div>
      </div>
    </div>
  </div>

  <footer>
    <div class="status-left">
      <span>
        <svg class="svg-icon" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><line x1="6" y1="3" x2="6" y2="21"/><circle cx="6" cy="6" r="3"/><circle cx="6" cy="18" r="3"/><path d="M6 12a9 9 0 0 1 9-9"/><circle cx="18" cy="6" r="3"/></svg>
        main*
      </span>
      <span style="color:#98c379">0 (X)</span>
      <span style="color:#e5c07b">0 (!)</span>
      <span>OFS LSP: Pronto</span>
      <span id="dirtyIndicator" style="color:#e5c07b; display:none;">[Modificado]</span>
    </div>
    <div class="status-right">
      <span id="cursorPosEl">Ln 1, Col 1</span>
      <span>Espaços: 4</span>
      <span>UTF-8</span>
      <span>LF</span>
      <span>Obsidian Fault Script (OFS)</span>
    </div>
  </footer>

  <script>
    const vscode = acquireVsCodeApi();
    let currentFilePath = "${(initialRelPath || initialFileName).replace(/\\/g, '/')}";

    function setTheme(theme) {
      document.body.setAttribute("data-theme", theme);
      document.querySelectorAll(".theme-btn").forEach(btn => {
        btn.classList.toggle("active", btn.textContent.toLowerCase().includes(theme.replace("_", "")));
      });
    }

    function triggerRun() {
      vscode.postMessage({ command: 'run' });
      appendTerminal("samns@linux-ofs:~/Obsidian-Fault-Script$ ofs run " + currentFilePath, "info");
      appendTerminal("[OFS Runner] Executando processo nativo...", "info");
    }

    function triggerBuild() {
      vscode.postMessage({ command: 'build' });
      appendTerminal("samns@linux-ofs:~/Obsidian-Fault-Script$ ofs build " + currentFilePath, "info");
    }

    function triggerSave() {
      const content = document.getElementById("codeEditor").value;
      vscode.postMessage({ command: 'save', filePath: currentFilePath, content: content });
      document.getElementById("dirtyIndicator").style.display = "none";
    }

    function loadFile(filePath) {
      vscode.postMessage({ command: 'openFile', filePath: filePath });
    }

    function updateGutter() {
      const editor = document.getElementById("codeEditor");
      const lines = editor.value.split("\\n").length;
      let gutterHtml = "";
      for (let i = 1; i <= Math.max(lines, 20); i++) {
        gutterHtml += i + "<br>";
      }
      document.getElementById("gutterEl").innerHTML = gutterHtml;
    }

    function onCodeChanged() {
      updateGutter();
      document.getElementById("dirtyIndicator").style.display = "inline";
      updateCursor();
    }

    function updateCursor() {
      const editor = document.getElementById("codeEditor");
      const text = editor.value.substring(0, editor.selectionStart);
      const lines = text.split("\\n");
      const row = lines.length;
      const col = lines[lines.length - 1].length + 1;
      document.getElementById("cursorPosEl").textContent = "Ln " + row + ", Col " + col;
    }

    function onKeyDown(e) {
      if (e.ctrlKey && e.key === 's') {
        e.preventDefault();
        triggerSave();
      } else if (e.key === 'F5') {
        e.preventDefault();
        triggerRun();
      } else if (e.key === 'Tab') {
        e.preventDefault();
        const editor = document.getElementById("codeEditor");
        const start = editor.selectionStart;
        const end = editor.selectionEnd;
        editor.value = editor.value.substring(0, start) + "    " + editor.value.substring(end);
        editor.selectionStart = editor.selectionEnd = start + 4;
        onCodeChanged();
      }
      setTimeout(updateCursor, 10);
    }

    function appendTerminal(line, type) {
      const panel = document.getElementById("panelContentEl");
      const div = document.createElement("div");
      div.className = "terminal-line " + (type || "");
      div.textContent = line;
      panel.appendChild(div);
      panel.scrollTop = panel.scrollHeight;
    }

    function clearTerminal() {
      document.getElementById("panelContentEl").innerHTML = '<div class="terminal-actions"><button class="action-btn secondary" style="font-size:11px;padding:3px 8px;" onclick="triggerRun()">▶ Executar (AOT)</button><button class="action-btn secondary" style="font-size:11px;padding:3px 8px;" onclick="triggerBuild()">🔨 Compilar</button><button class="action-btn secondary" style="font-size:11px;padding:3px 8px;" onclick="clearTerminal()">🗑 Limpar</button></div>';
    }

    function switchPanelTab(idx) {
      document.querySelectorAll(".ptab").forEach((tab, i) => {
        tab.classList.toggle("active", i === idx);
      });
      if (idx === 1) {
        document.getElementById("panelContentEl").innerHTML = '<div style="color:#98c379;font-weight:600;">[OFS LSP] Zero erros ou avisos encontrados no workspace.</div><div style="color:var(--fg-muted);margin-top:6px;">Todos os tipos e monólitos foram analisados com sucesso pelo compilador nativo OFS.</div>';
      } else if (idx === 3) {
        document.getElementById("panelContentEl").innerHTML = '<div style="padding:10px;background:var(--bg-editor);border:1px solid var(--border-color);border-radius:6px;max-width:300px;"><div style="font-weight:bold;margin-bottom:8px;">Pré-visualização OLL Component</div><button style="padding:6px 12px;background:var(--accent);color:#fff;border:none;border-radius:4px;cursor:pointer;">Botão Primário OLL</button></div>';
      }
    }

    window.addEventListener("message", (event) => {
      const msg = event.data;
      if (msg.type === "loadFile") {
        currentFilePath = msg.filePath;
        document.getElementById("tabTitle").textContent = msg.fileName;
        document.getElementById("codeEditor").value = msg.content;
        document.getElementById("dirtyIndicator").style.display = "none";
        updateGutter();
        updateCursor();
      } else if (msg.type === "syncDocument") {
        document.getElementById("codeEditor").value = msg.content;
        updateGutter();
        updateCursor();
      } else if (msg.type === "setTheme") {
        setTheme(msg.theme);
      }
    });

    document.getElementById("codeEditor").addEventListener("click", updateCursor);
    document.getElementById("codeEditor").addEventListener("keyup", updateCursor);
    updateGutter();
  </script>
</body>
</html>`;
}

async function runOllNative(document) {
  if (!document) {
    const editor = vscode.window.activeTextEditor;
    if (editor) document = editor.document;
  }
  if (!document) return;

  const file = document.fileName;
  const cwd = path.dirname(file);
  const ofsPath = resolveEffectiveOfsCompilerPath(cwd);

  const terminal = getOrCreateExecutionTerminal(cwd, ofsPath);
  terminal.show(true);
  terminal.sendText(`"${ofsPath}" run "${path.basename(file)}"`);
}

async function runOfsCheck(document, diagnosticCollection) {
  if (!document || document.languageId !== 'ofs') {
    return;
  }

  const ofsPath = await resolveReadyOfsPath();
  if (!ofsPath) {
    return;
  }

  const args = ['check', document.fileName];

  const result = await runExecFile(ofsPath, args, { cwd: getWorkspaceRoot() || undefined });
  if (!result.error) {
    diagnosticCollection.set(document.uri, []);
    return;
  }

  const combined = `${result.stdout || ''}\n${result.stderr || ''}`;
  const diagnostics = parseDiagnostics(document, combined);

  if (diagnostics.length === 0) {
    const fallbackRange = new vscode.Range(new vscode.Position(0, 0), new vscode.Position(0, 1));
    diagnosticCollection.set(document.uri, [
      new vscode.Diagnostic(
        fallbackRange,
        combined.trim() || 'OFS check failed. Verify ofs.path and compiler installation.',
        vscode.DiagnosticSeverity.Error
      )
    ]);
    return;
  }

  diagnosticCollection.set(document.uri, diagnostics);
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
    return path.join(process.env.APPDATA || os.homedir(), 'ofs', 'src/packages');
  }
  return path.join(os.homedir(), '.ofs', 'src/packages');
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
  if (!Array.isArray(registry?.src/packages)) return null;
  const pkg = registry.src/packages.find((p) => p.name === name);
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
      if (!range) return undefined;

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

class OfsWorkspaceTreeDataProvider {
  constructor() {
    this._onDidChangeTreeData = new vscode.EventEmitter();
    this.onDidChangeTreeData = this._onDidChangeTreeData.event;
  }

  refresh() {
    this._onDidChangeTreeData.fire();
  }

  getTreeItem(element) {
    return element;
  }

  async getChildren(element) {
    const ws = getWorkspaceRoot();
    if (!ws) {
      return [new vscode.TreeItem('Nenhum workspace aberto', vscode.TreeItemCollapsibleState.None)];
    }

    if (!element) {
      const ofsFiles = await vscode.workspace.findFiles('**/*.ofs', '**/node_modules/**');
      const ollFiles = await vscode.workspace.findFiles('**/*.oll', '**/node_modules/**');
      const docFiles = await vscode.workspace.findFiles('**/*.{odl,oes,md}', '**/node_modules/**');

      const roots = [];
      if (ofsFiles.length > 0) {
        const item = new vscode.TreeItem(`Scripts OFS (${ofsFiles.length})`, vscode.TreeItemCollapsibleState.Expanded);
        item.iconPath = new vscode.ThemeIcon('symbol-event');
        item.contextValue = 'ofsFolder';
        item.children = ofsFiles.sort((a, b) => a.fsPath.localeCompare(b.fsPath)).map(u => this.createFileItem(u, 'ofs'));
        roots.push(item);
      }
      if (ollFiles.length > 0) {
        const item = new vscode.TreeItem(`Layouts OLL (${ollFiles.length})`, vscode.TreeItemCollapsibleState.Expanded);
        item.iconPath = new vscode.ThemeIcon('layout');
        item.contextValue = 'ollFolder';
        item.children = ollFiles.sort((a, b) => a.fsPath.localeCompare(b.fsPath)).map(u => this.createFileItem(u, 'oll'));
        roots.push(item);
      }
      if (docFiles.length > 0) {
        const item = new vscode.TreeItem(`Documentos & Estilos (${docFiles.length})`, vscode.TreeItemCollapsibleState.Collapsed);
        item.iconPath = new vscode.ThemeIcon('book');
        item.contextValue = 'docFolder';
        item.children = docFiles.sort((a, b) => a.fsPath.localeCompare(b.fsPath)).map(u => this.createFileItem(u, 'doc'));
        roots.push(item);
      }
      return roots;
    }

    return element.children || [];
  }

  createFileItem(uri, kind) {
    const base = path.basename(uri.fsPath);
    const rel = vscode.workspace.asRelativePath(uri);
    const item = new vscode.TreeItem(base, vscode.TreeItemCollapsibleState.None);
    item.resourceUri = uri;
    item.description = rel !== base ? path.dirname(rel) : '';
    item.command = {
      command: 'vscode.open',
      arguments: [uri],
      title: 'Abrir Arquivo'
    };
    if (kind === 'ofs') {
      item.iconPath = new vscode.ThemeIcon('file-code');
    } else if (kind === 'oll') {
      item.iconPath = new vscode.ThemeIcon('layout-sidebar-left');
    } else {
      item.iconPath = new vscode.ThemeIcon('file-text');
    }
    return item;
  }
}

class OfsActionsTreeDataProvider {
  getTreeItem(element) {
    return element;
  }

  getChildren() {
    return [
      this.createActionItem('Compilar & Executar (Run)', 'ofs.runNative', 'play', 'Compila e executa o arquivo ativo imediatamente'),
      this.createActionItem('Depurar Programa (Debug)', 'ofs.debugNative', 'debug-alt-small', 'Inicia sessão de depuração nativa GDB/LLDB'),
      this.createActionItem('Verificar Sintaxe & Tipos (Check)', 'ofs.checkFile', 'check-all', 'Typecheck pelo compilador OFS Magma'),
      this.createActionItem('Gerar Assembly LLVM Nativo (ASM)', 'ofs.emitAssembly', 'symbol-field', 'Gera código de montagem .s nativo'),
      this.createActionItem('Live Preview OLL Layout', 'ofs.previewOll', 'layout', 'Pré-visualização em tempo real de layouts OLL'),
      this.createActionItem('Executar Janela Nativa OLL', 'ofs.runOll', 'window', 'Abre layout em janela nativa X11 com aceleração'),
      this.createActionItem('Abrir Obsidian Studio IDLE', 'ofs.openStudio', 'layout-sidebar-left', 'Abre a IDE integrada no VS Code'),
      this.createActionItem('Executar Studio IDLE Nativo (Linux)', 'ofs.launchNativeIdle', 'terminal', 'Inicia o binário nativo da Studio IDLE')
    ];
  }

  createActionItem(label, command, icon, tooltip) {
    const item = new vscode.TreeItem(label, vscode.TreeItemCollapsibleState.None);
    item.command = { command, title: label };
    item.tooltip = tooltip;
    item.iconPath = new vscode.ThemeIcon(icon);
    return item;
  }
}

class OfsThemesTreeDataProvider {
  constructor() {
    this._onDidChangeTreeData = new vscode.EventEmitter();
    this.onDidChangeTreeData = this._onDidChangeTreeData.event;
    this.currentTheme = 'dark_modern';
  }

  setTheme(t) {
    this.currentTheme = t;
    this._onDidChangeTreeData.fire();
  }

  getTreeItem(element) {
    return element;
  }

  getChildren() {
    const themes = [
      { id: 'dark_modern', name: 'Dark Modern (Obsidian Studio)', icon: 'color-mode' },
      { id: 'gtk', name: 'GTK+ Dark Theme (Adwaita)', icon: 'symbol-color' },
      { id: 'windows', name: 'Windows 11 Fluent Design', icon: 'window' },
      { id: 'neon', name: 'Cyberpunk Neon Synthwave', icon: 'zap' }
    ];

    return themes.map(t => {
      const active = this.currentTheme === t.id;
      const label = active ? `● ${t.name}` : `○ ${t.name}`;
      const item = new vscode.TreeItem(label, vscode.TreeItemCollapsibleState.None);
      item.command = {
        command: 'ofs.setTheme',
        arguments: [t.id],
        title: 'Selecionar Tema'
      };
      item.tooltip = `Aplicar tema ${t.name} na Studio IDLE`;
      item.iconPath = new vscode.ThemeIcon(t.icon);
      return item;
    });
  }
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
  const openStudioCmd = vscode.commands.registerCommand('ofs.openStudio', () => showStudioIdleWebview(context));

  studioIdleStatusBarItem = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
  studioIdleStatusBarItem.command = 'ofs.openStudio';
  studioIdleStatusBarItem.text = '$(layout) OFS Studio';
  studioIdleStatusBarItem.tooltip = 'Abrir Obsidian Fault Script :: Studio IDLE';
  studioIdleStatusBarItem.show();

  const ofsWorkspaceProvider = new OfsWorkspaceTreeDataProvider();
  const ofsActionsProvider = new OfsActionsTreeDataProvider();
  const ofsThemesProvider = new OfsThemesTreeDataProvider();

  vscode.window.registerTreeDataProvider('ofs.workspaceView', ofsWorkspaceProvider);
  vscode.window.registerTreeDataProvider('ofs.actionsView', ofsActionsProvider);
  vscode.window.registerTreeDataProvider('ofs.themesView', ofsThemesProvider);

  const refreshWorkspaceCmd = vscode.commands.registerCommand('ofs.refreshWorkspace', () => {
    ofsWorkspaceProvider.refresh();
  });

  const setThemeCmd = vscode.commands.registerCommand('ofs.setTheme', (themeId) => {
    if (themeId) {
      ofsThemesProvider.setTheme(themeId);
      if (studioIdlePanel) {
        studioIdlePanel.webview.postMessage({ type: 'setTheme', theme: themeId });
      }
      vscode.window.showInformationMessage(`Tema OLL alterado para: ${themeId}`);
    }
  });

  const launchNativeCmd = vscode.commands.registerCommand('ofs.launchNativeIdle', () => {
    const ws = getWorkspaceRoot();
    const candidatePaths = [
      path.join(ws, 'idle', 'dist', 'ofs_idle_linux'),
      path.join(os.homedir(), '.local', 'bin', 'ofs_idle_linux'),
      path.join(context.extensionPath, 'bin', 'linux-x64', 'ofs_idle_linux')
    ];
    let bin = candidatePaths.find(p => fs.existsSync(p));
    if (!bin) {
      vscode.window.showErrorMessage('Binário ofs_idle_linux não encontrado. Compile com ofs build idle/main.ofs.');
      return;
    }
    const proc = cp.spawn(bin, [], {
      cwd: path.join(ws, 'idle'),
      detached: true,
      stdio: 'ignore'
    });
    proc.unref();
    vscode.window.showInformationMessage('Obsidian Fault Script :: Studio IDLE nativa iniciada com sucesso.');
  });

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
    openStudioCmd,
    refreshWorkspaceCmd,
    setThemeCmd,
    launchNativeCmd,
    studioIdleStatusBarItem,
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
      updateStudioIdleWebview(document);
    }),
    vscode.workspace.onDidChangeTextDocument((event) => {
      refreshDiagnostics(event.document);
      updateOllWebview(event.document);
      updateStudioIdleWebview(event.document);
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
