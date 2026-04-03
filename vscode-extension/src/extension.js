const vscode = require('vscode');
const cp = require('child_process');
const path = require('path');
const fs = require('fs');
const os = require('os');
const https = require('https');

const KEYWORDS = [
  'core', 'vein', 'forge', 'const', 'monolith', 'strata', 'extern',
  'if', 'else', 'while', 'cycle', 'match', 'case', 'default',
  'return', 'break', 'continue', 'throw', 'tremor', 'catch',
  'fracture', 'abyss', 'obsid', 'attach', 'import', 'intent', 'tectonic', 'pure', 'impure', 'fractal',
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
    description: 'Importa modulo OFS por caminho.',
    context: 'Use para trazer funcoes/monoliths de arquivos .ofs externos.',
    example: 'attach "terminal_colors.ofs"'
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
  }
};

const DIAGNOSTIC_REGEX = /^(.*):(\d+):(\d+):\s*(.*)$/;
const ATTACH_REGEX = /^\s*(attach|import)\s+"([^"]+)"\s*$/;
const VEIN_REGEX = /^\s*(?:extern\s+)?vein\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/;
const MONOLITH_REGEX = /^\s*monolith\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{/;
const RELEASES_API = 'https://api.github.com/repos/Samwns/Obsidian-Fault-Script/releases/latest';

let extensionContextRef = null;
let managedCompilerPathCache = null;
let compilerInstallPromise = null;

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

function createRangeFromLineCol(document, line, col) {
  const lineIdx = Math.max(line - 1, 0);
  const colIdx = Math.max(col - 1, 0);
  const safeLine = Math.min(lineIdx, document.lineCount - 1);
  const safeCol = Math.min(colIdx, document.lineAt(safeLine).text.length);
  const start = new vscode.Position(safeLine, safeCol);
  const end = new vscode.Position(safeLine, Math.min(safeCol + 1, document.lineAt(safeLine).text.length));
  return new vscode.Range(start, end);
}

function getLibSearchPaths() {
  const env = process.env.OFS_LIB_PATH || '';
  const sep = process.platform === 'win32' ? ';' : ':';
  return env.split(sep).map((p) => p.trim()).filter(Boolean);
}

function resolveAttachPath(modulePath, docDir) {
  const local = path.resolve(docDir, modulePath);
  if (fs.existsSync(local)) {
    return local;
  }

  for (const base of getLibSearchPaths()) {
    const candidate = path.resolve(base, modulePath);
    if (fs.existsSync(candidate)) {
      return candidate;
    }
  }

  return null;
}

function parseAttachTargets(sourceText) {
  const lines = sourceText.split(/\r?\n/);
  const targets = [];

  for (const line of lines) {
    const m = line.match(ATTACH_REGEX);
    if (m) {
      targets.push(m[2]);
    }
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

  if (compilerInstallPromise) {
    await compilerInstallPromise;
  }

  const ofsPath = getOfsPath();
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
    if (compilerInstallPromise) {
      await compilerInstallPromise;
    }

    const ofsPath = getOfsPath();
    const file = document.fileName;
    const cwd = path.dirname(file);

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

    const terminal = vscode.window.activeTerminal || vscode.window.createTerminal({
      name: 'OFS Run',
      cwd,
      iconPath: new vscode.ThemeIcon('play')
    });
    terminal.show(true);
    terminal.sendText(getShellCommand(ofsPath, file));
  });
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
    provideHover(document, position) {
      const range = document.getWordRangeAtPosition(position);
      if (!range) return undefined;

      const word = document.getText(range);
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

  const diagnosticCollection = vscode.languages.createDiagnosticCollection('ofs');
  context.subscriptions.push(diagnosticCollection);

  const runCmd = vscode.commands.registerCommand('ofs.runFile', () => runCurrentFile());
  const checkCmd = vscode.commands.registerCommand('ofs.checkFile', () => checkCurrentFile(diagnosticCollection));

  context.subscriptions.push(runCmd, checkCmd);
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
