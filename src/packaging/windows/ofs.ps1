param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [object[]] $ArgsFromUser
)

$ErrorActionPreference = "Stop"
$OfsVersion = "__OFS_VERSION__"
$Repo = "Samwns/Obsidian-Fault-Script"

if (-not $env:OFS_STDLIB) {
    $bundledStdlib = Join-Path $PSScriptRoot 'stdlib'
    if (Test-Path $bundledStdlib) {
        $env:OFS_STDLIB = $bundledStdlib.Replace('\', '/')
    }
}

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
function Get-LlvmIrFlags {
    if ($env:OFS_LLVM_IR_FLAGS) {
        return @($env:OFS_LLVM_IR_FLAGS -split '\s+')
    }
    $clangVersion = (& clang --version 2>$null | Out-String)
    if ($clangVersion -match 'version 14\.') {
        return @("-mllvm", "-opaque-pointers")
    }
    return @()
}

$LlvmIrFlags = Get-LlvmIrFlags

function Invoke-OfsccEnv {
    param(
        [Parameter(Mandatory = $true)][string] $InputPath,
        [Parameter(Mandatory = $true)][string] $Mode,
        [string] $IrOutput
    )

    $cmdLine = "set OFSCC_INPUT=$InputPath&& set OFSCC_MODE=$Mode&& "
    if ($IrOutput) {
        $cmdLine += "set OFSCC_C_OUT=$IrOutput&& "
    }
    $cmdLine += "`"$Ofscc`""
    & cmd.exe /c $cmdLine
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
  ofs odl    <file.odl> [-o page.html]  Compile ODL to HTML
  ofs oes    <file.oes> [-o style.css]  Compile OES to CSS
  ofs translate <file> --to odl|oes     Import HTML or CSS
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
    $asset = "ofs-windows-x64-installer-$latest.exe"
    $installer = Join-Path $tmp $asset
    $url = "https://github.com/$Repo/releases/download/$latest/$asset"
    Write-Output "Downloading $asset..."
    Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $installer
    $process = Start-Process -FilePath $installer -ArgumentList "/S" -Wait -PassThru
    if ($process.ExitCode -ne 0) {
        throw "ofs update: installer failed with exit code $($process.ExitCode)"
    }
    Remove-Item $tmp -Recurse -Force

    $resolvedOfs = Get-Command 'ofs.cmd' -ErrorAction SilentlyContinue
    $ofsCmd = if ($resolvedOfs) { $resolvedOfs.Path } else { Join-Path $PSScriptRoot 'ofs.cmd' }
    $output = & $ofsCmd version | Out-String
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
$translateTo = ""

if ($mode -notin @("check", "tokens", "ast", "ir", "asm", "build", "run", "odl", "oes", "translate")) {
    $inputFile = $mode
    $extension = [IO.Path]::GetExtension($inputFile).ToLowerInvariant()
    if ($extension -eq ".odl") { $mode = "odl" }
    elseif ($extension -eq ".oes") { $mode = "oes" }
    else { $mode = "run" }
}

for ($i = 0; $i -lt $rest.Count; $i++) {
    if ($rest[$i] -in @("-o", "--output")) {
        $i++
        if ($i -lt $rest.Count) { $output = $rest[$i] }
    } elseif ($rest[$i] -eq "--to") {
        $i++
        if ($i -lt $rest.Count) { $translateTo = $rest[$i] }
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

if ($mode -in @("odl", "oes", "translate")) {
    $toolName = if ($mode -eq "translate") {
        if ($translateTo -eq "odl") { "odlc" }
        elseif ($translateTo -eq "oes") { "oesc" }
        else { throw "ofs translate: --to must be odl or oes" }
    } elseif ($mode -eq "odl") { "odlc" } else { "oesc" }
    $tool = Join-Path $PSScriptRoot "tools\$toolName.ofs"
    if (-not (Test-Path $tool)) { throw "ofs: web language tool not found: $tool" }
    $toolLl = "$work-tool.ll"
    $toolExe = "$work-tool.exe"
    Invoke-OfsccEnv -InputPath $tool -Mode "ir" -IrOutput $toolLl
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    $runtime = Join-Path $PSScriptRoot "libofs_runtime.a"
    & clang -Wno-override-module @LlvmIrFlags -O3 "-Wl,/OPT:REF" $toolLl $runtime -lm -o $toolExe
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    if (-not $output) {
        if ($mode -eq "odl") { $output = "$name.html" }
        elseif ($mode -eq "oes") { $output = "$name.css" }
        else { $output = "$name.$translateTo" }
    }
    if ($toolName -eq "odlc") {
        $env:ODLC_INPUT = (Resolve-Path $inputFile).Path
        $env:ODLC_OUTPUT = [IO.Path]::GetFullPath($output)
        $env:ODLC_DIRECTION = if ($mode -eq "translate") { "from-html" } else { "" }
    } else {
        $env:OESC_INPUT = (Resolve-Path $inputFile).Path
        $env:OESC_OUTPUT = [IO.Path]::GetFullPath($output)
        $env:OESC_DIRECTION = if ($mode -eq "translate") { "from-css" } else { "" }
    }
    & $toolExe
    $exitCode = $LASTEXITCODE
    Remove-Item $toolLl, $toolExe -ErrorAction SilentlyContinue
    exit $exitCode
}

if ($mode -in @("check", "tokens", "ast")) {
    Invoke-OfsccEnv -InputPath $inputFile -Mode $mode
    exit $LASTEXITCODE
}

if ($mode -eq "ir") {
    if (-not $output) { $output = "$name.ll" }
    Invoke-OfsccEnv -InputPath $inputFile -Mode "ir" -IrOutput $output
    exit $LASTEXITCODE
}

if ($mode -eq "asm") {
    if (-not $output) { $output = "$name.s" }
    if (-not $output.EndsWith(".s")) { $output = "$output.s" }
    Invoke-OfsccEnv -InputPath $inputFile -Mode "ir" -IrOutput $ll
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & clang -Wno-override-module @LlvmIrFlags -S $ll -o $output
    Remove-Item $ll -ErrorAction SilentlyContinue
    Write-Output "ASM: $output"
    exit $LASTEXITCODE
}

if ($mode -in @("build", "run")) {
    if (-not $output) { $output = $name }
    $runtime = Join-Path $PSScriptRoot "libofs_runtime.a"
    if (-not (Test-Path $runtime)) { $runtime = Join-Path $PSScriptRoot "ofs_runtime.lib" }
    Invoke-OfsccEnv -InputPath $inputFile -Mode "ir" -IrOutput $ll
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & clang -Wno-override-module @LlvmIrFlags -O3 "-Wl,/OPT:REF" $ll $runtime -lm -o $output
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
