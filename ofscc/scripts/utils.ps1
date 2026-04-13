# OFS Development Utilities - Windows PowerShell
# Quick build and test helpers for Windows

param(
    [string]$Command = "",
    [string[]]$Arguments = @()
)

$ErrorActionPreference = "Stop"

# ──────────────────────────────────────────────────────────────────────────
# Utilities
# ──────────────────────────────────────────────────────────────────────────

function Write-Step { Write-Host "→ $args" -ForegroundColor Cyan }
function Write-Success { Write-Host "✅ $args" -ForegroundColor Green }
function Write-Error_Custom { Write-Host "❌ $args" -ForegroundColor Red; exit 1 }
function Write-Info { Write-Host "ℹ️  $args" -ForegroundColor Gray }

function Show-Usage {
    Write-Host @"
`nOFS Development Utilities

Usage: .\utils.ps1 <command> [options]

Commands:
  compile <file.ofs>        Compile OFS file to executable
  check <file.ofs>          Type-check OFS file without codegen
  tokens <file.ofs>         Show lexer tokens
  ast <file.ofs>            Show AST
  ir <file.ofs>             Show LLVM IR output
  asm <file.ofs>            Show native assembly

  test [pattern]            Run tests matching pattern
  test-lexer                Run lexer tests
  test-parser               Run parser tests
  test-semantic             Run semantic tests

  benchmark                 Run performance benchmarks
  clean                     Clean build artifacts
  rebuild                   Clean and rebuild everything
  version                   Show compiler version

Examples:
  .\utils.ps1 compile hello.ofs -o hello
  .\utils.ps1 check example.ofs
  .\utils.ps1 tokens lexer_test.ofs
  .\utils.ps1 ast my_program.ofs
  .\utils.ps1 test-lexer
  .\utils.ps1 benchmark

"@ -ForegroundColor Cyan
    exit 0
}

function Find-Compiler {
    if (Test-Path "dist\ofscc.exe") { return "dist\ofscc.exe" }
    if (Test-Path "ofscc_v3.exe") { return "ofscc_v3.exe" }
    if (Test-Path "ofscc_v2.exe") { return "ofscc_v2.exe" }
    if (Get-Command ofscc -ErrorAction SilentlyContinue) { return "ofscc.exe" }
    return $null
}

# ──────────────────────────────────────────────────────────────────────────
# Commands
# ──────────────────────────────────────────────────────────────────────────

function Invoke-Compile {
    if ($Arguments.Count -eq 0) {
        Write-Error_Custom "Usage: compile <file.ofs> [-o <output>] [-O0/-O2/-O3]"
    }
    
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No OFS compiler found. Run bootstrap first."
    }
    
    $FileToCompile = $Arguments[0]
    $OtherArgs = $Arguments[1..($Arguments.Count - 1)]
    
    Write-Step "Compiling: $FileToCompile"
    & $Compiler build $FileToCompile @OtherArgs
    if ($LASTEXITCODE -ne 0) {
        Write-Error_Custom "Compilation failed (exit code: $LASTEXITCODE)"
    }
    Write-Success "Compilation complete"
}

function Invoke-Check {
    if ($Arguments.Count -eq 0) {
        Write-Error_Custom "Usage: check <file.ofs>"
    }
    
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No OFS compiler found"
    }
    
    Write-Step "Type-checking: $($Arguments[0])"
    & $Compiler check $Arguments[0]
    if ($LASTEXITCODE -ne 0) {
        Write-Error_Custom "Type check failed (exit code: $LASTEXITCODE)"
    }
    Write-Success "Type check passed"
}

function Invoke-Tokens {
    if ($Arguments.Count -eq 0) {
        Write-Error_Custom "Usage: tokens <file.ofs>"
    }
    
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No OFS compiler found"
    }
    
    Write-Step "Lexer tokens for: $($Arguments[0])"
    & $Compiler tokens $Arguments[0]
}

function Invoke-Ast {
    if ($Arguments.Count -eq 0) {
        Write-Error_Custom "Usage: ast <file.ofs>"
    }
    
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No OFS compiler found"
    }
    
    Write-Step "AST for: $($Arguments[0])"
    & $Compiler ast $Arguments[0]
}

function Invoke-IR {
    if ($Arguments.Count -eq 0) {
        Write-Error_Custom "Usage: ir <file.ofs>"
    }
    
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No OFS compiler found"
    }
    
    Write-Step "LLVM IR for: $($Arguments[0])"
    & $Compiler ir $Arguments[0]
}

function Invoke-Asm {
    if ($Arguments.Count -eq 0) {
        Write-Error_Custom "Usage: asm <file.ofs>"
    }
    
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No OFS compiler found"
    }
    
    Write-Step "Assembly for: $($Arguments[0])"
    & $Compiler asm $Arguments[0]
}

function Invoke-Test {
    $Pattern = if ($Arguments.Count -gt 0) { $Arguments[0] } else { "." }
    
    Write-Step "Running tests matching: $Pattern"
    
    if (Test-Path "test-build") {
        Push-Location "test-build"
        try {
            ctest -R $Pattern --output-on-failure
            if ($LASTEXITCODE -ne 0) {
                Write-Error_Custom "Tests failed (exit code: $LASTEXITCODE)"
            }
        } finally {
            Pop-Location
        }
        Write-Success "Tests passed"
    } else {
        Write-Error_Custom "test-build directory not found. Run 'utils.ps1 rebuild' first."
    }
}

function Invoke-TestLexer {
    Invoke-Test "test_lexer"
}

function Invoke-TestParser {
    Invoke-Test "test_parser"
}

function Invoke-TestSemantic {
    Invoke-Test "test_semantic"
}

function Invoke-Benchmark {
    Write-Step "Running performance benchmarks"
    
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No OFS compiler found"
    }
    
    Write-Info "Benchmarking compiler on various inputs..."
    
    $ExampleFiles = Get-ChildItem -Path "ofs\examples\*.ofs" -ErrorAction SilentlyContinue
    if ($ExampleFiles.Count -eq 0) {
        Write-Info "No example files found to benchmark"
        return
    }
    
    foreach ($File in $ExampleFiles) {
        $Start = [DateTime]::UtcNow
        & $Compiler check $File.FullName > $null 2>&1
        $End = [DateTime]::UtcNow
        $Elapsed = ($End - $Start).TotalMilliseconds
        Write-Host "  $($File.Name): $([Math]::Round($Elapsed, 2))ms"
    }
    
    Write-Success "Benchmarks complete"
}

function Invoke-Clean {
    Write-Step "Cleaning build artifacts"
    
    $ItemsToRemove = @(
        "build-ofscc",
        "build-mingw", 
        "test-build",
        "dist",
        "ofscc_v1.exe",
        "ofscc_v2.exe",
        "ofscc_v3.exe",
        "*.c",
        "*.o",
        "*.a",
        "output.c"
    )
    
    foreach ($Item in $ItemsToRemove) {
        if (Test-Path $Item) {
            Remove-Item -Path $Item -Recurse -Force -ErrorAction SilentlyContinue
            Write-Info "Removed: $Item"
        }
    }
    
    Write-Success "Clean complete"
}

function Invoke-Rebuild {
    Invoke-Clean
    Write-Step "Running full rebuild..."
    
    # Determine if we're on Windows and use PowerShell script
    $BootstrapScript = ".\scripts\bootstrap-windows.ps1"
    if (Test-Path $BootstrapScript) {
        & $BootstrapScript
    } else {
        Write-Error_Custom "Bootstrap script not found: $BootstrapScript"
    }
}

function Invoke-Version {
    $Compiler = Find-Compiler
    if (-not $Compiler) {
        Write-Error_Custom "No compiler found"
    }
    
    Write-Step "Compiler version"
    Write-Host "Compiler: $Compiler"
    
    if (Test-Path "dist\version.json") {
        Write-Host ""
        $VersionInfo = Get-Content "dist\version.json" | ConvertFrom-Json
        Write-Host "Version: $($VersionInfo.version)"
        Write-Host "Platform: $($VersionInfo.platform)"
        Write-Host "Build Date: $($VersionInfo.buildDate)"
    }
}

# ──────────────────────────────────────────────────────────────────────────
# Main dispatcher
# ──────────────────────────────────────────────────────────────────────────

if (-not $Command -or $Command -eq "help" -or $Command -eq "-h" -or $Command -eq "--help") {
    Show-Usage
}

switch ($Command.ToLower()) {
    "compile"       { Invoke-Compile }
    "check"         { Invoke-Check }
    "tokens"        { Invoke-Tokens }
    "ast"           { Invoke-Ast }
    "ir"            { Invoke-IR }
    "asm"           { Invoke-Asm }
    "test"          { Invoke-Test }
    "test-lexer"    { Invoke-TestLexer }
    "test-parser"   { Invoke-TestParser }
    "test-semantic" { Invoke-TestSemantic }
    "benchmark"     { Invoke-Benchmark }
    "clean"         { Invoke-Clean }
    "rebuild"       { Invoke-Rebuild }
    "version"       { Invoke-Version }
    default         { Write-Error_Custom "Unknown command: $Command`nRun with --help for usage" }
}
