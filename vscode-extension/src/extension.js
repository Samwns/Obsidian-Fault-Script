const vscode = require('vscode');
const cp = require('child_process');

const KEYWORDS = [
  'core', 'vein', 'forge', 'const', 'monolith', 'strata', 'extern',
  'if', 'else', 'while', 'cycle', 'match', 'case', 'default',
  'return', 'break', 'continue', 'throw', 'tremor', 'catch',
  'fracture', 'obsid', 'attach', 'import', 'intent', 'pure', 'impure', 'fractal',
  'true', 'false', 'null', 'as'
];

const DIAGNOSTIC_REGEX = /^(.*):(\d+):(\d+):\s*(.*)$/;

function getOfsPath() {
  return vscode.workspace.getConfiguration().get('ofs.path', 'ofs');
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

function runOfsCheck(document, diagnosticCollection) {
  if (!document || document.languageId !== 'ofs') {
    return;
  }

  const ofsPath = getOfsPath();
  const args = ['check', document.fileName];

  cp.execFile(ofsPath, args, { cwd: vscode.workspace.rootPath || undefined }, (error, stdout, stderr) => {
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
  document.save().then(() => {
    const ofsPath = getOfsPath();
    const file = document.fileName.replace(/"/g, '\\"');
    const terminal = vscode.window.createTerminal({ name: 'OFS Run' });
    terminal.show(true);
    terminal.sendText(`\"${ofsPath}\" \"${file}\"`);
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
    provideCompletionItems() {
      return KEYWORDS.map((keyword) => {
        const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
        item.insertText = keyword;
        return item;
      });
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
