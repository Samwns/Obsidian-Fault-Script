const vscode = require('vscode');
const cp = require('child_process');
const path = require('path');
const fs = require('fs');

const KEYWORDS = [
  'core', 'vein', 'forge', 'const', 'monolith', 'strata', 'extern',
  'if', 'else', 'while', 'cycle', 'match', 'case', 'default',
  'return', 'break', 'continue', 'throw', 'tremor', 'catch',
  'fracture', 'obsid', 'attach', 'import', 'intent', 'pure', 'impure', 'fractal',
  'true', 'false', 'null', 'as'
];

const HOVER_DOCS = {
  core: {
    description: 'Entry point do programa OFS.',
    example: 'core main() {\n    echo("hello")\n}'
  },
  vein: {
    description: 'Declara uma funcao.',
    example: 'vein add(a: stone, b: stone) -> stone {\n    return a + b\n}'
  },
  forge: {
    description: 'Declara variavel (tipada ou inferida).',
    example: 'forge x: stone = 10\nforge y = 20'
  },
  const: {
    description: 'Declara variavel imutavel.',
    example: 'const max: stone = 100'
  },
  cycle: {
    description: 'Loop range-based ou C-style (3 partes).',
    example: 'cycle (n in nums) {\n    echo(n)\n}\ncycle (forge i = 0; i < 10; i++) {\n    echo(i)\n}'
  },
  while: {
    description: 'Loop por condicao.',
    example: 'while (x > 0) {\n    x -= 1\n}'
  },
  fracture: {
    description: 'Bloco seguro para operacoes com ponteiros.',
    example: 'fracture {\n    shard p: *stone = &x;\n    *p = 42\n}'
  },
  abyss: {
    description: 'Bloco de memoria irrestrita (unsafe).',
    example: 'abyss {\n    // operacoes sem restricoes de seguranca\n}'
  },
  fractal: {
    description: 'Bloco intermediario de efeitos.',
    example: 'fractal {\n    echo("effect-lifted block")\n}'
  },
  obsid: {
    description: 'Fechamento de bloco no estilo ": ... obsid".',
    example: 'if (x > 0):\n    echo("ok")\nobsid'
  },
  attach: {
    description: 'Importa modulo OFS por caminho.',
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
    example: 'forge y: crystal = x as crystal'
  }
};

const DIAGNOSTIC_REGEX = /^(.*):(\d+):(\d+):\s*(.*)$/;
const ATTACH_REGEX = /^\s*(attach|import)\s+"([^"]+)"\s*$/;
const VEIN_REGEX = /^\s*(?:extern\s+)?vein\s+([A-Za-z_][A-Za-z0-9_]*)\s*\(/;
const MONOLITH_REGEX = /^\s*monolith\s+([A-Za-z_][A-Za-z0-9_]*)\s*\{/;

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

  return getBundledCompilerPath() || configured;
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

function runOfsCheck(document, diagnosticCollection) {
  if (!document || document.languageId !== 'ofs') {
    return;
  }

  const ofsPath = getOfsPath();
  const args = ['check', document.fileName];

  cp.execFile(ofsPath, args, { cwd: getWorkspaceRoot() || undefined }, (error, stdout, stderr) => {
    if (!error) {
      diagnosticCollection.set(document.uri, []);
      return;
    }

    const combined = `${stdout || ''}\n${stderr || ''}`;
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
  });
}

function runCurrentFile() {
  const editor = vscode.window.activeTextEditor;
  if (!editor || editor.document.languageId !== 'ofs') {
    vscode.window.showErrorMessage('Open an .ofs file to run.');
    return;
  }

  const document = editor.document;
  document.save().then(async () => {
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

  editor.document.save().then(() => {
    runOfsCheck(editor.document, diagnosticCollection);
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
      md.appendMarkdown('Exemplo:\n');
      md.appendCodeblock(doc.example, 'ofs');
      md.isTrusted = false;

      return new vscode.Hover(md, range);
    }
  });

  context.subscriptions.push(provider);
}

function activate(context) {
  const diagnosticCollection = vscode.languages.createDiagnosticCollection('ofs');
  context.subscriptions.push(diagnosticCollection);

  const runCmd = vscode.commands.registerCommand('ofs.runFile', () => runCurrentFile());
  const checkCmd = vscode.commands.registerCommand('ofs.checkFile', () => checkCurrentFile(diagnosticCollection));

  context.subscriptions.push(runCmd, checkCmd);
  registerCompletionProvider(context);
  registerFileDecorations(context);
  registerHoverProvider(context);

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
