# Bootstrap test self-hosted para Windows PowerShell
# Fase 1: bootstrap/ofs.exe -> ofscc_v1.exe
# Fase 2: ofscc_v1.exe -> ofscc_v2.exe
# Fase 3: ofscc_v2.exe -> ofscc_v3.exe
# Resultado esperado: ofscc_v2.exe == ofscc_v3.exe (deterministico)

param(
    [string]$BootstrapExe = ".\bootstrap\ofs.exe",
    [string]$OfsCcSrc = "ofs\ofscc\ofscc.ofs"
)

Write-Host "=== Bootstrap Test Windows (Self-Hosted) ===" -ForegroundColor Green
Write-Host "Pipeline: bootstrap -> ofscc_v1 -> ofscc_v2 -> ofscc_v3"
Write-Host ""

function Check-Success {
    param([int]$ExitCode, [string]$Step)
    if ($ExitCode -ne 0) {
        Write-Host "[ERRO] $Step (exit code: $ExitCode)" -ForegroundColor Red
        exit 1
    }
    Write-Host "[OK] $Step" -ForegroundColor Green
}

if (-not (Test-Path $BootstrapExe)) {
    Write-Host "[ERRO] bootstrap nao encontrado: $BootstrapExe" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $OfsCcSrc)) {
    Write-Host "[ERRO] fonte do compilador nao encontrado: $OfsCcSrc" -ForegroundColor Red
    exit 1
}

Write-Host "Passo 1: bootstrap -> ofscc_v1.exe" -ForegroundColor Yellow
& $BootstrapExe build $OfsCcSrc -o ofscc_v1.exe
Check-Success $LASTEXITCODE "build ofscc_v1.exe"

Write-Host "Passo 2: ofscc_v1.exe -> ofscc_v2.exe" -ForegroundColor Yellow
$env:OFSCC_INPUT = $OfsCcSrc
$env:OFSCC_OUTPUT = "ofscc_v2.exe"
& .\ofscc_v1.exe
Check-Success $LASTEXITCODE "build ofscc_v2.exe"

Write-Host "Passo 3: ofscc_v2.exe -> ofscc_v3.exe" -ForegroundColor Yellow
$env:OFSCC_OUTPUT = "ofscc_v3.exe"
& .\ofscc_v2.exe
Check-Success $LASTEXITCODE "build ofscc_v3.exe"

Remove-Item Env:OFSCC_INPUT -ErrorAction SilentlyContinue
Remove-Item Env:OFSCC_OUTPUT -ErrorAction SilentlyContinue

Write-Host "Passo 4: validando determinismo" -ForegroundColor Yellow
$Hash2 = (Get-FileHash "ofscc_v2.exe" -Algorithm SHA256).Hash
$Hash3 = (Get-FileHash "ofscc_v3.exe" -Algorithm SHA256).Hash

if ($Hash2 -eq $Hash3) {
    Write-Host "[OK] deterministico: ofscc_v2.exe == ofscc_v3.exe" -ForegroundColor Green
    Write-Host "SHA256: $Hash2" -ForegroundColor Gray
} else {
    Write-Host "[ERRO] nao deterministico" -ForegroundColor Red
    Write-Host "  v2: $Hash2"
    Write-Host "  v3: $Hash3"
    exit 1
}

Write-Host ""
Write-Host "Bootstrap self-hosted verificado com sucesso." -ForegroundColor Green
