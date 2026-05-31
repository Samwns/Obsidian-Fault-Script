param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [object[]] $ArgsFromUser
)

$ErrorActionPreference = "Stop"
$OfsVersion = "__OFS_VERSION__"
$Repo = "Samwns/Obsidian-Fault-Script"

$dllDirs = @(
    $PSScriptRoot,
    $env:OFS_DLL_DIR,
    (Join-Path $env:ProgramFiles 'LLVM\bin'),
    (Join-Path $env:ProgramFiles 'Git\mingw64\bin'),
    'C:\msys64\ucrt64\bin'
) | Where-Object { $_ -and (Test-Path $_) } | Select-Object -Unique

if ($dllDirs.Count -gt 0) {
    $env:PATH = (($dllDirs -join ';') + ';' + $env:PATH)
}

$Ofscc = Join-Path $PSScriptRoot 'ofscc.exe'
if (-not (Test-Path $Ofscc)) {
    $resolved = Get-Command 'ofscc.exe' -ErrorAction SilentlyContinue
    if ($resolved) { $Ofscc = $resolved.Path }
}

function Show-Banner {
@"

 ______        ______      ______
/\  __ \      /\  ___\    /\  ___\
\ \ \/\ \     \ \  __\    \ \___  \
 \ \_____\     \ \_\       \/\_____\
  \/_____/      \/_/        \/_____/

"@ | Write-Host
}

function Show-Version {
    Show-Banner
    Write-Output "ofs $OfsVersion - Obsidian Fault Script"
    Write-Output "status: stable"
}

function Show-Usage {
    Show-Banner
    Write-Output @"
ofs - Obsidian Fault Script compiler v$OfsVersion

Usage:
  ofs <file.ofs>                        Run a script directly
  ofs run    <file.ofs>                 Compile and run immediately
  ofs build  <file.ofs> [-o output]     Compile to native executable
  ofs check  <file.ofs>                 Type-check only (no output)
  ofs tokens <file.ofs>                 Print token stream (debug)
  ofs ast    <file.ofs>                 Print AST (debug)
  ofs ir     <file.ofs>                 Emit LLVM IR (debug)
  ofs asm    <file.ofs>                 Emit target-native assembly
  ofs version                           Print compiler version
  ofs update                            Update to the latest release
  ofs help                              Show this help message
"@
}

function Get-LatestTag {
    $url = "https://github.com/$Repo/releases/latest"
    $response = Invoke-WebRequest -UseBasicParsing -Uri $url -MaximumRedirection 5
    $final = $response.BaseResponse.ResponseUri.AbsoluteUri
    return ($final -split '/')[-1]
}

function Invoke-Update {
    $latest = Get-LatestTag
    if (-not $latest) {
        throw "ofs update: failed to fetch latest release tag"
    }
    if ($latest -eq "v$OfsVersion") {
        Write-Output "OFS is already up to date ($latest)"
        return
    }

    $tmp = Join-Path ([IO.Path]::GetTempPath()) ("ofs-update-" + [Guid]::NewGuid().ToString("N"))
    New-Item -ItemType Directory -Path $tmp | Out-Null
    $asset = "ofs-windows-x64-portable-$latest.zip"
    $zip = Join-Path $tmp $asset
    $url = "https://github.com/$Repo/releases/download/$latest/$asset"
    Write-Output "Downloading $asset..."
    Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $zip
    Expand-Archive -LiteralPath $zip -DestinationPath (Join-Path $tmp "unpack") -Force
    Copy-Item (Join-Path $tmp "unpack\*") -Destination $PSScriptRoot -Recurse -Force
    Remove-Item $tmp -Recurse -Force

    $output = & (Join-Path $PSScriptRoot 'ofs.cmd') version | Out-String
    if ($output -notmatch "ofs $($latest.TrimStart('v')) - Obsidian Fault Script") {
        throw "ofs update: installed version did not match $latest"
    }
    Write-Output "Update completed to $latest"
}

$argsList = @()
foreach ($arg in $ArgsFromUser) {
    if ($null -ne $arg) { $argsList += $arg.ToString() }
}

$cmd = if ($argsList.Count -gt 0) { $argsList[0] } else { "help" }
if ($cmd -in @("version", "--version", "-v")) {
    Show-Version
    exit 0
}
if ($cmd -in @("help", "--help", "-h")) {
    Show-Usage
    exit 0
}
if ($cmd -eq "update") {
    Invoke-Update
    exit 0
}

if (-not (Test-Path $Ofscc)) {
    throw "ofs: self-hosted compiler not found: $Ofscc"
}

$rest = if ($argsList.Count -gt 1) { $argsList[1..($argsList.Count - 1)] } else { @() }
$mode = $cmd
$inputFile = ""
$output = ""

if ($mode -notin @("check", "tokens", "ast", "ir", "asm", "build", "run")) {
    $inputFile = $mode
    $mode = "run"
}

for ($i = 0; $i -lt $rest.Count; $i++) {
    if ($rest[$i] -in @("-o", "--output")) {
        $i++
        if ($i -lt $rest.Count) { $output = $rest[$i] }
    } elseif (-not $inputFile) {
        $inputFile = $rest[$i]
    }
}

if (-not $inputFile) {
    Show-Usage
    exit 2
}

$name = [IO.Path]::GetFileNameWithoutExtension($inputFile)
$work = Join-Path ([IO.Path]::GetTempPath()) "ofs-$name-$PID"
$ll = "$work.ll"

if ($mode -in @("check", "tokens", "ast")) {
    $env:OFSCC_INPUT = $inputFile
    $env:OFSCC_MODE = $mode
    & $Ofscc
    exit $LASTEXITCODE
}

if ($mode -eq "ir") {
    if (-not $output) { $output = "$name.ll" }
    $env:OFSCC_INPUT = $inputFile
    $env:OFSCC_MODE = "ir"
    $env:OFSCC_C_OUT = $output
    & $Ofscc
    exit $LASTEXITCODE
}

if ($mode -eq "asm") {
    if (-not $output) { $output = "$name.s" }
    if (-not $output.EndsWith(".s")) { $output = "$output.s" }
    $env:OFSCC_INPUT = $inputFile
    $env:OFSCC_MODE = "ir"
    $env:OFSCC_C_OUT = $ll
    & $Ofscc
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & clang -Wno-override-module -S $ll -o $output
    Remove-Item $ll -ErrorAction SilentlyContinue
    Write-Output "ASM: $output"
    exit $LASTEXITCODE
}

if ($mode -in @("build", "run")) {
    if (-not $output) { $output = $name }
    $runtime = Join-Path $PSScriptRoot "libofs_runtime.a"
    if (-not (Test-Path $runtime)) { $runtime = Join-Path $PSScriptRoot "ofs_runtime.lib" }
    $env:OFSCC_INPUT = $inputFile
    $env:OFSCC_MODE = "ir"
    $env:OFSCC_C_OUT = $ll
    & $Ofscc
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & clang -Wno-override-module -O2 $ll $runtime -lm -o $output
    Remove-Item $ll -ErrorAction SilentlyContinue
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    if ($mode -eq "run") {
        & ".\$output"
        exit $LASTEXITCODE
    }
    Write-Output "Built: $output"
    exit 0
}

throw "ofs: unknown command '$cmd'"
