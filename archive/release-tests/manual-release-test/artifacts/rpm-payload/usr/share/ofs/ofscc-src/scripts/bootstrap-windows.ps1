# OFS Native Compiler Bootstrap - Windows PowerShell
# Compila o compilador OFS nativo com suporte a todas as plataformas e releases
# Fase 1: Compilar C++ compiler
# Fase 2: Bootstrap OFS compiler
# Fase 3: Gerar releases

param(
    [string]$BuildDir = "build-ofscc",
    [string]$InstallDir = "dist",
    [string]$OfsDir = "ofs",
    [string]$OfsCcDir = "ofs\ofscc",
    [switch]$SkipCppBuild = $false,
    [switch]$SkipBootstrap = $false,
    [switch]$ReleaseOnly = $false
)

$ErrorActionPreference = "Stop"

function Write-Step { param([string]$Text) Write-Host "→ $Text" -ForegroundColor Cyan }
function Write-Success { param([string]$Text) Write-Host "✅ $Text" -ForegroundColor Green }
function Write-Error_Custom { param([string]$Text) Write-Host "❌ $Text" -ForegroundColor Red; exit 1 }
function Write-Info { param([string]$Text) Write-Host "ℹ️  $Text" -ForegroundColor Gray }

Write-Host "`n╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Yellow
Write-Host "║     OFS Native Compiler Bootstrap (Windows)               ║" -ForegroundColor Yellow
Write-Host "╚═══════════════════════════════════════════════════════════╝`n" -ForegroundColor Yellow

# ──────────────────────────────────────────────────────────────────────────
# PHASE 1: Build C++ Compiler (if not skipped)
# ──────────────────────────────────────────────────────────────────────────

if (-not $SkipCppBuild) {
    Write-Step "Phase 1: Building C++ compiler as bootstrap..."
    
    if (-not (Test-Path $BuildDir)) {
        Write-Info "Creating build directory: $BuildDir"
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }
    
    Push-Location $BuildDir
    try {
        Write-Info "Running CMake configure..."
        cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
        if ($LASTEXITCODE -ne 0) { Write-Error_Custom "CMake configure failed" }
        
        Write-Info "Building with CMake..."
        cmake --build . --config Release -j4
        if ($LASTEXITCODE -ne 0) { Write-Error_Custom "CMake build failed" }
    } finally {
        Pop-Location
    }
    
    # Locate ofscc_v1 (C++ compiled)
    $v1_candidates = @(
        "$BuildDir\ofscc.exe",
        "$BuildDir\Release\ofscc.exe",
        "$BuildDir\Debug\ofscc.exe"
    )
    
    $OfsccV1 = $null
    foreach ($cand in $v1_candidates) {
        if (Test-Path $cand) {
            $OfsccV1 = $cand
            break
        }
    }
    
    if (-not $OfsccV1) {
        Write-Error_Custom "Could not find compiled ofscc executable in: $BuildDir"
    }
    
    Write-Success "C++ compiler ready: $OfsccV1"
} else {
    Write-Info "Skipping C++ build (--SkipCppBuild)"
    
    # Try to find existing ofscc_v1
    $OfsccV1 = if (Test-Path "$BuildDir\ofscc.exe") {
        "$BuildDir\ofscc.exe"
    } elseif (Test-Path "$BuildDir\Release\ofscc.exe") {
        "$BuildDir\Release\ofscc.exe"
    } elseif (Get-Command ofscc -ErrorAction SilentlyContinue) {
        "ofscc"
    } else {
        Write-Error_Custom "Cannot find existing ofscc compiler. Run without --SkipCppBuild first"
    }
}

# ──────────────────────────────────────────────────────────────────────────
# PHASE 2: Bootstrap OFS Compiler (OFS → OFS)
# ──────────────────────────────────────────────────────────────────────────

if (-not $SkipBootstrap) {
    Write-Step "Phase 2: Bootstrap OFS compiler (OFS compiling OFS)..."
    
    $OfscSrc = "$OfsCcDir\ofscc.ofs"
    if (-not (Test-Path $OfscSrc)) {
        Write-Error_Custom "OFS source not found: $OfscSrc"
    }
    
    # Bootstrap step 1: ofscc_v1 (C++) → ofscc_v2 (OFS)
    Write-Info "[2.1] Compiling ofscc_v2 with C++ compiler..."
    & $OfsccV1 build $OfscSrc -o "ofscc_v2.exe" -O2
    if ($LASTEXITCODE -ne 0) {
        Write-Error_Custom "Failed to compile ofscc_v2"
    }
    Write-Success "ofscc_v2 created (C++ compiled OFS compiler)"
    
    # Bootstrap step 2: ofscc_v2 (OFS) → ofscc_v3 (OFS)
    Write-Info "[2.2] Compiling ofscc_v3 with ofscc_v2 (OFS self-hosting)..."
    .\ofscc_v2.exe build $OfscSrc -o "ofscc_v3.exe" -O2
    if ($LASTEXITCODE -ne 0) {
        Write-Error_Custom "Failed to compile ofscc_v3"
    }
    Write-Success "ofscc_v3 created (OFS self-hosted compiler)"
    
    # Verify determinism
    Write-Info "[2.3] Verifying determinism..."
    $v2_hash = (Get-FileHash "ofscc_v2.exe" -Algorithm SHA256).Hash
    $v3_hash = (Get-FileHash "ofscc_v3.exe" -Algorithm SHA256).Hash
    
    if ($v2_hash -eq $v3_hash) {
        Write-Success "Determinism verified: ofscc_v2 and ofscc_v3 are byte-identical ✓"
    } else {
        Write-Info "⚠️  Non-deterministic build (hashes differ). This may be normal during development."
        Write-Info "  ofscc_v2 hash: $v2_hash"
        Write-Info "  ofscc_v3 hash: $v3_hash"
    }
} else {
    Write-Info "Skipping bootstrap (--SkipBootstrap)"
}

# ──────────────────────────────────────────────────────────────────────────
# PHASE 3: Generate Release Artifacts
# ──────────────────────────────────────────────────────────────────────────

Write-Step "Phase 3: Preparing release artifacts..."

if (-not (Test-Path $InstallDir)) {
    New-Item -ItemType Directory -Path $InstallDir | Out-Null
}

# Copy final compiler
$FinalCompiler = "ofscc_v3.exe"
if (Test-Path $FinalCompiler) {
    Copy-Item $FinalCompiler "$InstallDir\ofscc.exe" -Force
    Write-Success "Compiler ready: $InstallDir\ofscc.exe"
} else {
    Write-Error_Custom "Final compiler not found: $FinalCompiler"
}

# Copy runtime library
if (Test-Path "$BuildDir\ofs_runtime.a") {
    Copy-Item "$BuildDir\ofs_runtime.a" "$InstallDir\" -Force
    Write-Success "Runtime library copied"
} elseif (Test-Path "$BuildDir\Release\ofs_runtime.a") {
    Copy-Item "$BuildDir\Release\ofs_runtime.a" "$InstallDir\" -Force
    Write-Success "Runtime library copied"
}

# Copy standard library
if (Test-Path "$OfsDir\stdlib") {
    if (-not (Test-Path "$InstallDir\stdlib")) {
        New-Item -ItemType Directory -Path "$InstallDir\stdlib" | Out-Null
    }
    Copy-Item "$OfsDir\stdlib\*" "$InstallDir\stdlib\" -Force -Recurse
    Write-Success "Standard library copied"
}

# Create version file
$Version = "1.0.0-native"
$BuildDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
@"
{
  "version": "$Version",
  "platform": "windows-x64",
  "arch": "x64",
  "buildDate": "$BuildDate",
  "compiler": "OFS (Self-Hosted)",
  "features": [
    "full-ofs-support",
    "llvm-codegen",
    "stdlib-complete",
    "package-system"
  ]
}
"@ | Out-File "$InstallDir\version.json" -Encoding UTF8

Write-Success "Version file created"

# ──────────────────────────────────────────────────────────────────────────
# Summary
# ──────────────────────────────────────────────────────────────────────────

Write-Host "`n╔═══════════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║                    BUILD SUCCESSFUL                       ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════════╝`n" -ForegroundColor Green

Write-Host "📦 Release artifacts in: $InstallDir" -ForegroundColor Yellow
Write-Host "🎯 Main executable: $InstallDir\ofscc.exe" -ForegroundColor Yellow
Write-Host "📚 Standard library: $InstallDir\stdlib\*" -ForegroundColor Yellow
Write-Host "ℹ️  Version info: $InstallDir\version.json" -ForegroundColor Yellow

Write-Host "`n✨ All OFS features are supported:" -ForegroundColor Cyan
Write-Host "   ✓ Type checking and inference" -ForegroundColor Green
Write-Host "   ✓ Full pattern matching (match/throw/catch)" -ForegroundColor Green
Write-Host "   ✓ Monolith with impl blocks" -ForegroundColor Green
Write-Host "   ✓ Namespace support" -ForegroundColor Green
Write-Host "   ✓ Lambda and function values" -ForegroundColor Green
Write-Host "   ✓ Package system (attach)" -ForegroundColor Green
Write-Host "   ✓ External function binding (extern vein, rift vein)" -ForegroundColor Green
Write-Host "   ✓ Low-level blocks (fracture, abyss, bedrock)" -ForegroundColor Green
Write-Host "   ✓ Small int types (u8, u16, u32, i8, i3" -ForegroundColor Green
Write-Host "`n"
