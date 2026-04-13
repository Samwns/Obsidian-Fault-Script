# Bootstrap Test para Windows PowerShell
# Verifica se OFS pode compilar a si mesmo (determinismo)
# Fase 1: ofscc_v1 (compilado com C++)
# Fase 2: ofscc_v2 (compilado com ofscc_v1)
# Fase 3: ofscc_v3 (compilado com ofscc_v2)
# Resultado: ofscc_v2 e ofscc_v3 devem ser idênticos

param(
    [string]$OfsDir = "ofs",
    [string]$OfsCcDir = "ofs\ofscc",
    [string]$BuildDir = "build-mingw"
)

Write-Host "=== Bootstrap Test para Windows ===" -ForegroundColor Green
Write-Host "Testando auto-compilacao: ofscc_v1 -> ofscc_v2 -> ofscc_v3"
Write-Host ""

# Helper function
function Check-Success {
    param([int]$ExitCode, [string]$Step)
    if ($ExitCode -ne 0) {
        Write-Host "❌ Erro em $Step (exit code: $ExitCode)" -ForegroundColor Red
        exit 1
    }
    Write-Host "✅ $Step" -ForegroundColor Green
}

# Passo 1: Build ofscc_v1 com C++
Write-Host "Passo 1: Compilando ofscc_v1 com C++..." -ForegroundColor Yellow
if (Test-Path $BuildDir) {
    Write-Host "  Build dir existe. Usando cmake debug build..."
    cd $BuildDir
    cmake --build . --config Debug
    Check-Success $LASTEXITCODE "cmake build"
    cd ..
} else {
    Write-Host "  Criando build dir..." -ForegroundColor Gray
    mkdir $BuildDir -ErrorAction SilentlyContinue | Out-Null
    cd $BuildDir
    cmake .. -G "MinGW Makefiles"
    Check-Success $LASTEXITCODE "cmake configure"
    cmake --build .
    Check-Success $LASTEXITCODE "cmake build"
    cd ..
}

# Copiar ofscc_v1 para posição esperada
$V1Exe = if (Test-Path "$BuildDir\ofscc.exe") { "$BuildDir\ofscc.exe" } elseif (Test-Path "$BuildDir\Debug\ofscc.exe") { "$BuildDir\Debug\ofscc.exe" } else { "ofscc.exe" }

if (-not (Test-Path $V1Exe)) {
    Write-Host "❌ Não encontrou ofscc.exe em: $V1Exe" -ForegroundColor Red
    Write-Host "  Procurando alternativas..." -ForegroundColor Gray
    $Found = Get-ChildItem -Path $BuildDir -Name "ofscc.exe" -Recurse
    if ($Found) {
        $V1Exe = $Found[0]
        Write-Host "  Encontrado: $V1Exe" -ForegroundColor Green
    } else {
        Write-Host "❌ ofscc.exe não encontrado!" -ForegroundColor Red
        exit 1
    }
}

# Passo 2: Build ofscc_v2 com ofscc_v1
Write-Host "Passo 2: Compilando ofscc_v2 com ofscc_v1..." -ForegroundColor Yellow
$OfsccSrc = "$OfsCcDir\ofscc.ofs"
if (-not (Test-Path $OfsccSrc)) {
    Write-Host "❌ Arquivo fonte não encontrado: $OfsccSrc" -ForegroundColor Red
    exit 1
}

Copy-Item $V1Exe "ofscc_v1.exe" -Force
$Cmd = ".\ofscc_v1.exe $OfsccSrc -o ofscc_v2.c"
Write-Host "  Executando: $Cmd" -ForegroundColor Gray
& $Cmd
Check-Success $LASTEXITCODE "ofscc_v1 -> C"

# Compilar output.c gerado com gcc
if (Test-Path "output.c") {
    $Cmd = "gcc -O2 output.c -o ofscc_v2.exe"
    Write-Host "  Compilando C: $Cmd" -ForegroundColor Gray
    & $Cmd
    Check-Success $LASTEXITCODE "gcc output.c"
} else {
    Write-Host "❌ output.c não foi gerado!" -ForegroundColor Red
    exit 1
}

# Passo 3: Build ofscc_v3 com ofscc_v2
Write-Host "Passo 3: Compilando ofscc_v3 com ofscc_v2..." -ForegroundColor Yellow
$Cmd = ".\ofscc_v2.exe $OfsccSrc -o ofscc_v3.c"
Write-Host "  Executando: $Cmd" -ForegroundColor Gray
& $Cmd
Check-Success $LASTEXITCODE "ofscc_v2 -> C"

# Compilar com gcc
if (Test-Path "output.c") {
    $Cmd = "gcc -O2 output.c -o ofscc_v3.exe"
    Write-Host "  Compilando C: $Cmd" -ForegroundColor Gray
    & $Cmd
    Check-Success $LASTEXITCODE "gcc output2.c"
} else {
    Write-Host "❌ output.c não foi gerado!" -ForegroundColor Red
    exit 1
}

# Passo 4: Verificar determinismo
Write-Host "Passo 4: Verificando determinismo (ofscc_v2 === ofscc_v3)..." -ForegroundColor Yellow

$Size2 = (Get-Item "ofscc_v2.exe").Length
$Size3 = (Get-Item "ofscc_v3.exe").Length

Write-Host "  Tamanho ofscc_v2.exe: $Size2 bytes"
Write-Host "  Tamanho ofscc_v3.exe: $Size3 bytes"

if ($Size2 -eq $Size3) {
    Write-Host "  ✅ Tamanhos idênticos" -ForegroundColor Green
    
    # Comparar conteúdo
    $Hash2 = (Get-FileHash "ofscc_v2.exe" -Algorithm SHA256).Hash
    $Hash3 = (Get-FileHash "ofscc_v3.exe" -Algorithm SHA256).Hash
    
    if ($Hash2 -eq $Hash3) {
        Write-Host "  ✅ SHA256 idênticos: $Hash2" -ForegroundColor Green
        Write-Host ""
        Write-Host "╔════════════════════════════════════════════════════════════╗" -ForegroundColor Green
        Write-Host "║  🎉 BOOTSTRAP DETERMINÍSTICO VERIFICADO! 🎉              ║" -ForegroundColor Green
        Write-Host "║  OFS pode compilar a si mesmo!                           ║" -ForegroundColor Green
        Write-Host "║  ofscc_v2 === ofscc_v3 (byte-for-byte idêntico)         ║" -ForegroundColor Green
        Write-Host "╚════════════════════════════════════════════════════════════╝" -ForegroundColor Green
    } else {
        Write-Host "  ⚠️  Tamanho igual mas conteúdo diferente!" -ForegroundColor Yellow
        Write-Host "     SHA256 v2: $Hash2"
        Write-Host "     SHA256 v3: $Hash3"
        Write-Host "     Investigar determinismo..." -ForegroundColor Yellow
        exit 1
    }
} else {
    Write-Host "  ❌ Tamanhos diferentes! Não determinístico." -ForegroundColor Red
    Write-Host "     Diferença: $([Math]::Abs($Size2 - $Size3)) bytes" -ForegroundColor Red
    exit 1
}

# Limpeza
Write-Host ""
Write-Host "Limpando arquivos temporários..." -ForegroundColor Gray
Remove-Item "ofscc_v1.exe" -ErrorAction SilentlyContinue
Remove-Item "output.c" -ErrorAction SilentlyContinue
Remove-Item "ofscc_v3.c" -ErrorAction SilentlyContinue
Write-Host "✅ Feito!" -ForegroundColor Green

Write-Host ""
Write-Host "Próximos passos:" -ForegroundColor Cyan
Write-Host "  1. .\ofscc_v2.exe ofs\examples\hello.ofs -o hello.exe"
Write-Host "  2. .\hello.exe"
Write-Host "  3. Compilar mais exemplos"
