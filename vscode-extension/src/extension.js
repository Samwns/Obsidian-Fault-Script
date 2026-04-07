const vscode = require('vscode');
const cp = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');
const https = require('https');

const KEYWORDS = [
  'core', 'vein', 'forge', 'const', 'monolith', 'strata', 'extern', 'rift',
  'if', 'else', 'while', 'cycle', 'match', 'case', 'default',
  'return', 'break', 'continue', 'throw', 'tremor', 'catch',
  'fracture', 'abyss', 'bedrock', 'obsid', 'attach', 'import', 'intent', 'tectonic', 'pure', 'impure', 'fractal',
  'layout', 'bind', 'abi', 'packed', 'native', 'system',
  'asm', 'echo', 'in', 'shard',
  'true', 'false', 'null', 'as'
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
    context: 'Use para fluxo intermediario entre modos estritos e irrestritos.',
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
    context: 'Use para selecionar explicitamente o modo de memoria do bloco.',
    example: 'tectonic fracture {\n    shard p: *stone = &x\n}\ntectonic safe {\n    shard q: *stone = &x\n}\ntectonic unsafe {\n    // alias de abyss\n}\ntectonic bedrock {\n    // alias de fractal\n}'
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
    context: 'Disponivel em blocos bedrock, fracture e abyss para sincronizacao machine-like.',
    example: 'bedrock {\n    fault_fence()\n}'
  },
  fault_prefetch: {
    description: 'Intrinsic low-level: solicita prefetch de um alvo em memoria.',
    context: 'Disponivel em blocos bedrock, fracture e abyss para preparar leituras de ponteiros em caminhos quentes.',
    example: 'bedrock {\n    fault_prefetch(ptr)\n}'
  },
  fault_trap: {
    description: 'Intrinsic low-level: emite trap de maquina.',
    context: 'Disponivel em blocos bedrock, fracture e abyss para interrupcao hard-fail.',
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
const REGISTRY_URL = 'https://raw.githubusercontent.com/Samwns/Obsidian-Fault-Script/main/packages/registry.json';
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
    const req = https.get(url, { headers: { 'User-Agent': 'obsidian-fault-vscode-extension' } }, (res) => {
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
    const request = https.get(url, { headers: { 'User-Agent': 'obsidian-fault-vscode-extension' } }, (res) => {
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
        path.join(root, 'ofs', 'build', 'ofs.exe'),
        path.join(root, 'ofs', 'build', 'Release', 'ofs.exe')
      ]
    : [path.join(root, 'ofs', 'build', 'ofs')];

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

function getOrCreateExecutionTerminal() {
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

  if (process.platform === 'win32') {
    if (ofsPath === 'ofs' || compilerName === 'ofs.exe') {
      return `ofs run "${fileName}"`;
    }

    return `& "${ofsPath.replace(/"/g, '\\"')}" run "${fileName}"`;
  }

  if (ofsPath === 'ofs' || compilerName === 'ofs') {
    return `ofs run "${fileName}"`;
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
  'serve':           'webserver.ofs',
  'bedrock':         'bedrock.ofs',
  'bedrock-packet':  'bedrock_packet.ofs',
  'terminal-colors': 'terminal_colors.ofs',
  'memory-modes':    'memory_modes.ofs',
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
    const terminal = getOrCreateExecutionTerminal();

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
      const execution = shellIntegration.executeCommand(ofsPath, ['run', file]);
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
    provideCompletionItems(document) {
      const keywordItems = KEYWORDS.map((keyword) => {
        const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
        item.insertText = keyword;
        return item;
      });

      const attachItems = collectAttachSymbols(document).map((symbol) => {
        const item = new vscode.CompletionItem(symbol.name, symbol.kind);
        item.insertText = symbol.name;
        item.detail = 'From attached library';
        return item;
      });

      return [...keywordItems, ...attachItems];
    }
  });

  context.subscriptions.push(provider);
}

// ── Package / library helper functions ──────────────────────────────────

function getInstalledPackagesDir() {
  if (process.platform === 'win32') {
    return path.join(process.env.APPDATA || os.homedir(), 'ofs', 'packages');
  }
  return path.join(os.homedir(), '.ofs', 'packages');
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

  const checkCmd = vscode.command
  registerAttachDecorations(context);s.registerCommand('ofs.checkFile', () => checkCurrentFile(diagnosticCollection));
  const asmCmd = vscode.commands.registerCommand('ofs.emitAssembly', () => emitCurrentAssembly());
  const pauseCmd = vscode.commands.registerCommand('ofs.pauseExecution', () => pauseActiveExecution());
  const resumeCmd = vscode.commands.registerCommand('ofs.resumeExecution', () => resumeActiveExecution());
  const stopCmd = vscode.commands.registerCommand('ofs.stopExecution', () => terminateActiveExecution());
  context.subscriptions.push(
    nativeRunCmd,
    nativeDebugCmd,
    checkCmd,
    asmCmd,
    pauseCmd,
    resumeCmd,
    stopCmd,
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
    vscode.workspace.onDidOpenTextDocument((document) => runOfsCheck(document, diagnosticCollection)),
    vscode.workspace.onDidSaveTextDocument((document) => runOfsCheck(document, diagnosticCollection)),
    vscode.workspace.onDidChangeTextDocument((event) => refreshDiagnostics(event.document)),
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
