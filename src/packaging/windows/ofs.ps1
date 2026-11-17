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
# O ofscc le OFS_STDLIB_PATH (o attach {modulo} procura ali): espelha OFS_STDLIB.
if (-not $env:OFS_STDLIB_PATH -and $env:OFS_STDLIB) {
    $env:OFS_STDLIB_PATH = $env:OFS_STDLIB
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
  ofs <file.ofs>                        Run a script directly (interpreted)
  ofs run    <file.ofs> [--native]      Run interpreted (or --native temporary build)
  ofs build  <file.ofs> [-o output]     Compile to native executable
  ofs build --target android <app.oal|app.ofs> [-o app.apk] [--manifest f] [--install] [--serial id]
                                        Build an Android APK (needs Android SDK + NDK + JDK)
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

# ---------------------------------------------------------------------------
# Alvo Android (aarch64, API 26+): `ofs build --target android app.oal|app.ofs`
# Mesmo fluxo do wrapper Linux (src/packaging/linux/ofs, android_build):
#   oalc -> .ofs -> ofscc (OFSCC_TARGET=android) -> clang do NDK -> lib<nome>.so
#   -> aapt2 link + jar (lib/ sem compressao) -> zipalign -> apksigner.
# Env: ANDROID_SDK / ANDROID_HOME / ANDROID_SDK_ROOT (padrao
#      %LOCALAPPDATA%\Android\Sdk), ANDROID_NDK / ANDROID_NDK_HOME (opcional),
#      JAVA_HOME, OFS_SOURCE (pasta ofs/ do repo), OFS_ANDROID_KEYSTORE,
#      OFS_ANDROID_BUILD.
# ---------------------------------------------------------------------------
function Stop-Android([string] $Message) {
    [Console]::Error.WriteLine("ofs android: $Message")
    exit 1
}
function Write-AndroidStep([string] $Message) { [Console]::Error.WriteLine("==> $Message") }

function Get-NewestChild([string] $Dir, [string] $Filter) {
    if (-not (Test-Path $Dir)) { return $null }
    $items = @(Get-ChildItem -Path $Dir -Directory -Filter $Filter -ErrorAction SilentlyContinue)
    if ($items.Count -eq 0) { return $null }
    $sorted = $items | Sort-Object {
        $v = [version]'0.0'
        $m = [regex]::Match($_.Name, '\d+(\.\d+)*')
        if ($m.Success) {
            $txt = $m.Value
            if ($txt -notmatch '\.') { $txt = "$txt.0" }
            $tmp = $null
            if ([version]::TryParse($txt, [ref] $tmp)) { $v = $tmp }
        }
        $v
    }
    return ($sorted | Select-Object -Last 1).FullName
}

function Find-JavaTool([string] $Name) {
    $dirs = @()
    if ($env:JAVA_HOME) { $dirs += (Join-Path $env:JAVA_HOME 'bin') }
    foreach ($base in @($env:ProgramFiles, ${env:ProgramFiles(x86)})) {
        if (-not $base) { continue }
        $dirs += (Join-Path $base 'Android\Android Studio\jbr\bin')
        foreach ($vendor in @('Eclipse Adoptium', 'Java', 'Microsoft', 'Zulu', 'Amazon Corretto')) {
            $vdir = Join-Path $base $vendor
            if (Test-Path $vdir) {
                Get-ChildItem $vdir -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending |
                    ForEach-Object { $dirs += (Join-Path $_.FullName 'bin') }
            }
        }
    }
    foreach ($d in $dirs) {
        $c = Join-Path $d "$Name.exe"
        if (Test-Path $c) { return $c }
    }
    $cmd = Get-Command "$Name.exe" -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Path }
    return $null
}

# Pasta com tools\<nome>.ofs (instalada ao lado do ofs.ps1, ou o repo via OFS_SOURCE).
function Find-OfsTool([string] $Name) {
    $cands = @($env:OFS_SOURCE, $PSScriptRoot, (Join-Path $PSScriptRoot '..\..\..\ofs'), (Join-Path $PSScriptRoot '..\ofs'))
    foreach ($c in $cands) {
        if ($c -and (Test-Path (Join-Path $c "tools\$Name.ofs"))) { return (Join-Path $c "tools\$Name.ofs") }
    }
    return $null
}
function Find-OfsccSource {
    $cands = @($env:OFS_SOURCE, $PSScriptRoot, (Join-Path $PSScriptRoot '..\..\..\ofs'), (Join-Path $PSScriptRoot '..\ofs'))
    foreach ($c in $cands) {
        if ($c -and (Test-Path (Join-Path $c 'ofscc\ofscc.ofs'))) { return (Join-Path $c 'ofscc\ofscc.ofs') }
    }
    return $null
}

# Roda o ofscc com variaveis de ambiente (restauradas depois). Retorna @(codigo, log).
function Invoke-OfsccAndroid {
    param([string] $Compiler, [string] $InputPath, [string] $IrOut, [string] $Target)
    $saved = @{}
    $vars = @{
        OFSCC_INPUT = $InputPath.Replace('\', '/'); OFSCC_MODE = 'ir'; OFSCC_C_OUT = $IrOut
        OFSCC_TARGET = $Target
        OFS_STDLIB_PATH = $env:OFS_STDLIB; OFS_STDLIB = $env:OFS_STDLIB
    }
    foreach ($k in $vars.Keys) {
        $saved[$k] = [Environment]::GetEnvironmentVariable($k)
        [Environment]::SetEnvironmentVariable($k, $(if ($vars[$k]) { $vars[$k] } else { $null }))
    }
    $prev = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    try {
        if (Test-Path $IrOut) { Remove-Item $IrOut -Force }
        $log = (& $Compiler 2>&1 | Out-String)
        $code = $LASTEXITCODE
    } finally {
        $ErrorActionPreference = $prev
        foreach ($k in $saved.Keys) { [Environment]::SetEnvironmentVariable($k, $saved[$k]) }
    }
    return @($code, $log)
}

function Invoke-Native([string] $Exe, [string[]] $ToolArgs, [string] $What) {
    $prev = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
    try { & $Exe @ToolArgs | Out-Host; $code = $LASTEXITCODE } finally { $ErrorActionPreference = $prev }
    if ($code -ne 0) { Stop-Android "$What falhou (codigo $code)" }
}

function Get-StaleBy([string] $Target, [string[]] $Dirs) {
    if (-not (Test-Path $Target)) { return $true }
    $t = (Get-Item $Target).LastWriteTimeUtc
    foreach ($d in $Dirs) {
        if (-not $d -or -not (Test-Path $d)) { continue }
        $n = Get-ChildItem $d -Recurse -Filter *.ofs -File -ErrorAction SilentlyContinue |
            Where-Object { $_.LastWriteTimeUtc -gt $t } | Select-Object -First 1
        if ($n) { return $true }
    }
    return $false
}

# Liga um .ll de host (Windows) com o runtime magma.o.
function Link-HostIr([string] $Ll, [string] $Exe) {
    $runtime = Join-Path $PSScriptRoot 'magma.o'
    Invoke-Native 'clang' (@('-Wno-override-module') + $LlvmIrFlags + @('-O2', '-Wl,/OPT:REF', '-Wl,/force:multiple', $Ll, $runtime, '-lws2_32', '-o', $Exe)) 'clang (link de ferramenta)'
}

# ofscc com suporte a OFSCC_TARGET (o instalado pode ser anterior a isso).
function Get-AndroidOfscc([string] $Cache) {
    $probe = Join-Path $Cache 'probe.ofs'
    [IO.File]::WriteAllText($probe, "core main() { echo(`"x`") }`n", (New-Object System.Text.UTF8Encoding $false))
    $probeLl = Join-Path $Cache 'probe.ll'
    $r = Invoke-OfsccAndroid $Ofscc $probe $probeLl 'android'
    if ($r[0] -eq 0 -and (Test-Path $probeLl) -and (Select-String -Path $probeLl -Pattern 'aarch64' -Quiet)) { return $Ofscc }
    $src = Find-OfsccSource
    if (-not $src) { Stop-Android "o ofscc instalado nao tem OFSCC_TARGET e o fonte do OFS nao foi achado. Rode 'ofs update' ou defina OFS_SOURCE=<repo>\ofs" }
    $bin = Join-Path $Cache 'ofscc.exe'
    if (Get-StaleBy $bin @((Split-Path $src -Parent))) {
        Write-AndroidStep "compilando ofscc do fonte ($src)"
        $ll = Join-Path $Cache 'ofscc.ll'
        $r = Invoke-OfsccAndroid $Ofscc $src $ll ''
        if ($r[0] -ne 0 -or -not (Test-Path $ll)) { Write-Host $r[1]; Stop-Android 'falha ao compilar o ofscc do fonte' }
        Link-HostIr $ll $bin
        Remove-Item $ll -ErrorAction SilentlyContinue
    }
    return $bin
}

# Compila uma ferramenta .ofs (ex.: tools\oalc.ofs) para o host, com cache.
function Get-AndroidTool([string] $Name, [string] $Cache, [string] $Compiler) {
    $src = Find-OfsTool $Name
    if (-not $src) { Stop-Android "tools\$Name.ofs nao encontrado (reinstale o OFS ou defina OFS_SOURCE=<repo>\ofs)" }
    $bin = Join-Path $Cache "$Name.exe"
    $key = Join-Path $Cache "$Name.key"
    $want = (Get-Item $Compiler).LastWriteTimeUtc.Ticks.ToString() + '|' + $Compiler
    $have = if (Test-Path $key) { (Get-Content $key -Raw).Trim() } else { '' }
    if ((Get-StaleBy $bin @((Split-Path $src -Parent), $env:OFS_STDLIB)) -or $have -ne $want) {
        Write-AndroidStep "compilando $Name"
        $ll = Join-Path $Cache "$Name.ll"
        $r = Invoke-OfsccAndroid $Compiler $src $ll ''
        if ($r[0] -ne 0 -or -not (Test-Path $ll)) { Write-Host $r[1]; Stop-Android "falha ao compilar $Name" }
        Link-HostIr $ll $bin
        Remove-Item $ll -ErrorAction SilentlyContinue
        [IO.File]::WriteAllText($key, $want)
    }
    return $bin
}

function Invoke-AndroidBuild {
    param([string] $InputFile, [string] $Output, [string] $Manifest, [bool] $Install, [string] $Serial)

    # --- ferramentas externas ---
    $sdk = $null
    foreach ($c in @($env:ANDROID_SDK, $env:ANDROID_HOME, $env:ANDROID_SDK_ROOT, (Join-Path $env:LOCALAPPDATA 'Android\Sdk'))) {
        if ($c -and (Test-Path $c)) { $sdk = $c; break }
    }
    if (-not $sdk) {
        Stop-Android "Android SDK nao encontrado. Instale o Android Studio (ou as command-line tools) e defina ANDROID_SDK (padrao: %LOCALAPPDATA%\Android\Sdk). Precisa de ndk\, build-tools\ e platforms\ (sdkmanager `"ndk;29.0.14206865`" `"build-tools;37.0.0`" `"platforms;android-34`")."
    }
    $ndk = $null
    foreach ($c in @($env:ANDROID_NDK, $env:ANDROID_NDK_HOME, $env:ANDROID_NDK_ROOT)) {
        if ($c -and (Test-Path $c)) { $ndk = $c; break }
    }
    if (-not $ndk) { $ndk = Get-NewestChild (Join-Path $sdk 'ndk') '*' }
    if (-not $ndk) { Stop-Android "NDK nao encontrado em $sdk\ndk. Instale: sdkmanager `"ndk;29.0.14206865`" (ou Android Studio > SDK Manager > SDK Tools > NDK)." }
    $bt = Get-NewestChild (Join-Path $sdk 'build-tools') '*'
    if (-not $bt) { Stop-Android "build-tools nao encontrado em $sdk\build-tools. Instale: sdkmanager `"build-tools;37.0.0`"" }
    $aapt2 = Join-Path $bt 'aapt2.exe'; $zipalign = Join-Path $bt 'zipalign.exe'; $apksigner = Join-Path $bt 'apksigner.bat'
    foreach ($t in @($aapt2, $zipalign, $apksigner)) {
        if (-not (Test-Path $t)) { Stop-Android "ferramenta ausente em build-tools: $t (reinstale build-tools)" }
    }
    $plat = Get-NewestChild (Join-Path $sdk 'platforms') 'android-*'
    if (-not $plat -or -not (Test-Path (Join-Path $plat 'android.jar'))) { Stop-Android "android.jar nao encontrado em $sdk\platforms. Instale: sdkmanager `"platforms;android-34`"" }
    $triple = 'aarch64-linux-android26'
    $cc = Join-Path $ndk 'toolchains\llvm\prebuilt\windows-x86_64\bin\clang.exe'
    if (-not (Test-Path $cc)) { Stop-Android "clang do NDK nao encontrado: $cc (esse NDK e para Windows x64?)" }
    $keytool = Find-JavaTool 'keytool'
    $jar = Find-JavaTool 'jar'
    if (-not $keytool) { Stop-Android "JDK nao encontrado (keytool/apksigner precisam de Java 17+). Instale um JDK (ex.: winget install EclipseAdoptium.Temurin.17.JDK) ou defina JAVA_HOME." }
    if (-not $env:JAVA_HOME) { $env:JAVA_HOME = (Split-Path (Split-Path $keytool -Parent) -Parent) }
    $jdkBin = Split-Path $keytool -Parent
    $env:PATH = "$jdkBin;$env:PATH"
    if (-not (Get-Command clang -ErrorAction SilentlyContinue) -and $InputFile -match '\.oal$') {
        Stop-Android "clang (LLVM) nao esta no PATH; ele compila o oalc no host. Instale LLVM (winget install LLVM.LLVM)."
    }

    $stdlib = $env:OFS_STDLIB
    if (-not $stdlib) { Stop-Android 'stdlib do OFS nao encontrada (defina OFS_STDLIB).' }
    $cache = Join-Path $env:LOCALAPPDATA 'ofs\android'
    New-Item -ItemType Directory -Force -Path $cache | Out-Null
    $aOfscc = Get-AndroidOfscc $cache

    if (-not (Test-Path $InputFile)) { Stop-Android "arquivo nao encontrado: $InputFile" }
    $absInput = (Resolve-Path $InputFile).Path
    $inDir = Split-Path $absInput -Parent
    $app = [IO.Path]::GetFileNameWithoutExtension($absInput)
    $out = if ($env:OFS_ANDROID_BUILD) { $env:OFS_ANDROID_BUILD } else { Join-Path $inDir "build\android\$app" }
    foreach ($d in @('obj', 'apk')) { if (Test-Path (Join-Path $out $d)) { Remove-Item (Join-Path $out $d) -Recurse -Force } }
    New-Item -ItemType Directory -Force -Path (Join-Path $out 'obj'), (Join-Path $out 'apk\lib\arm64-v8a') | Out-Null
    $entry = $absInput
    $manifestPath = $Manifest

    if ($absInput -match '\.oal$') {
        $oalc = Get-AndroidTool 'oalc' $cache $aOfscc
        Write-AndroidStep "oalc: $([IO.Path]::GetFileName($absInput)) -> AndroidManifest.xml + ${app}_oal.ofs"
        $env:OALC_INPUT = $absInput.Replace('\', '/'); $env:OALC_OUT_DIR = $out.Replace('\', '/'); $env:OALC_TARGET = 'android'
        $prev = $ErrorActionPreference; $ErrorActionPreference = 'Continue'
        try { & $oalc; $code = $LASTEXITCODE } finally { $ErrorActionPreference = $prev; Remove-Item Env:OALC_INPUT, Env:OALC_OUT_DIR, Env:OALC_TARGET -ErrorAction SilentlyContinue }
        if ($code -ne 0) { Stop-Android "oalc falhou em $absInput" }
        $entry = Join-Path $out "${app}_oal.ofs"
        if (-not $manifestPath) { $manifestPath = Join-Path $out 'AndroidManifest.xml' }
        foreach ($d in @('res', 'assets')) { if (Test-Path (Join-Path $out $d)) { Remove-Item (Join-Path $out $d) -Recurse -Force } }
        $iconTxt = Join-Path $out 'oal_icon.txt'
        if (Test-Path $iconTxt) {
            New-Item -ItemType Directory -Force -Path (Join-Path $out 'res\mipmap') | Out-Null
            $icon = (Get-Content $iconTxt -Raw).Trim()
            if (-not (Test-Path $icon)) { Stop-Android "icone nao encontrado: $icon" }
            Copy-Item $icon (Join-Path $out 'res\mipmap\ic_launcher.png') -Force
        }
        $assetsTxt = Join-Path $out 'oal_assets.txt'
        if (Test-Path $assetsTxt) {
            $assetsDir = (Get-Content $assetsTxt -Raw).Trim()
            if (-not (Test-Path $assetsDir)) { Stop-Android "pasta de assets nao encontrada: $assetsDir" }
            New-Item -ItemType Directory -Force -Path (Join-Path $out 'assets') | Out-Null
            Copy-Item (Join-Path $assetsDir '*') (Join-Path $out 'assets') -Recurse -Force
        }
    }
    if (-not $manifestPath) {
        if (Test-Path (Join-Path $inDir 'AndroidManifest.xml')) { $manifestPath = Join-Path $inDir 'AndroidManifest.xml' }
        elseif (Test-Path (Join-Path $inDir 'android\AndroidManifest.xml')) { $manifestPath = Join-Path $inDir 'android\AndroidManifest.xml' }
        else {
            $manifestPath = Join-Path $out 'AndroidManifest.xml'
            $pkgname = ([regex]::Replace($app, '[^a-zA-Z0-9]', '_')).ToLowerInvariant()
            $xml = @"
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
    package="org.ofs.$pkgname" android:versionCode="1" android:versionName="0.1.0">
    <uses-sdk android:minSdkVersion="26" android:targetSdkVersion="34" />
    <application android:label="$app" android:hasCode="false" android:extractNativeLibs="true"
        android:theme="@android:style/Theme.NoTitleBar.Fullscreen">
        <activity android:name="android.app.NativeActivity" android:label="$app" android:exported="true"
            android:configChanges="orientation|screenSize|keyboardHidden|screenLayout|uiMode">
            <meta-data android:name="android.app.lib_name" android:value="$pkgname" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
"@
            [IO.File]::WriteAllText($manifestPath, $xml, (New-Object System.Text.UTF8Encoding $false))
        }
    }
    if (-not (Test-Path $manifestPath)) { Stop-Android "manifesto nao encontrado: $manifestPath" }
    $manifestText = [IO.File]::ReadAllText($manifestPath)
    $libname = 'main'
    $m = [regex]::Match($manifestText, 'android\.app\.lib_name"[^>]*android:value="([^"]*)"')
    if ($m.Success) { $libname = $m.Groups[1].Value }
    $pkg = ''
    $m = [regex]::Match($manifestText, 'package="([^"]*)"')
    if ($m.Success) { $pkg = $m.Groups[1].Value }
    if (-not $Output) { $Output = Join-Path $out "$app-debug.apk" }
    if (-not [IO.Path]::IsPathRooted($Output)) { $Output = Join-Path (Get-Location).Path $Output }

    # --- runtime magma (cache) e app em IR aarch64 ---
    Write-AndroidStep 'IR aarch64: runtime magma'
    $magmaSrc = Join-Path $stdlib 'runtime\magma.ofs'
    if (-not (Test-Path $magmaSrc)) { Stop-Android "runtime nao encontrado: $magmaSrc (reinstale o OFS: stdlib\runtime\)" }
    $magmaO = Join-Path $cache 'magma-aarch64.o'
    $stale = (-not (Test-Path $magmaO)) -or ((Get-Item $magmaSrc).LastWriteTimeUtc -gt (Get-Item $magmaO).LastWriteTimeUtc) -or ((Get-Item $aOfscc).LastWriteTimeUtc -gt (Get-Item $magmaO).LastWriteTimeUtc)
    if ($stale) {
        $magmaLl = Join-Path $cache 'magma.ll'
        $r = Invoke-OfsccAndroid $aOfscc $magmaSrc $magmaLl 'android'
        if ($r[0] -ne 0 -or -not (Test-Path $magmaLl)) { Write-Host $r[1]; Stop-Android "ofscc falhou em $magmaSrc" }
        Invoke-Native $cc @("--target=$triple", '-O2', '-fPIC', '-Wno-override-module', '-c', $magmaLl, '-o', $magmaO) 'clang do NDK (magma)'
    }
    Write-AndroidStep "IR aarch64: $([IO.Path]::GetFileName($entry))"
    $appLl = Join-Path $out 'obj\app.ll'
    $r = Invoke-OfsccAndroid $aOfscc $entry $appLl 'android'
    if ($r[0] -ne 0 -or -not (Test-Path $appLl) -or (Get-Item $appLl).Length -eq 0) { Write-Host $r[1]; Stop-Android "ofscc falhou em $entry" }

    Write-AndroidStep "clang do NDK ($triple) -> lib$libname.so"
    $appO = Join-Path $out 'obj\app.o'
    Invoke-Native $cc @("--target=$triple", '-O2', '-fPIC', '-Wno-override-module', '-c', $appLl, '-o', $appO) 'clang do NDK (app)'
    $so = Join-Path $out "apk\lib\arm64-v8a\lib$libname.so"
    Invoke-Native $cc @("--target=$triple", '-shared', '-o', $so, $appO, $magmaO,
        '-Wl,--allow-multiple-definition', '-Wl,-init,__ofs_init_globals',
        '-u', 'ANativeActivity_onCreate', '-Wl,-z,max-page-size=16384',
        '-landroid', '-llog', '-laaudio', '-lm') 'link da .so'

    # --- APK ---
    Write-AndroidStep 'APK: aapt2 + jar + zipalign + apksigner'
    $unsigned = Join-Path $out 'unsigned.apk'; $aligned = Join-Path $out 'aligned.apk'
    $resArgs = @()
    if (Test-Path (Join-Path $out 'res')) {
        $resZip = Join-Path $out 'res.zip'
        Invoke-Native $aapt2 @('compile', '--dir', (Join-Path $out 'res'), '-o', $resZip) 'aapt2 compile'
        $resArgs = @($resZip)
    }
    Remove-Item $unsigned, $aligned -ErrorAction SilentlyContinue
    Invoke-Native $aapt2 (@('link', '-o', $unsigned, '--manifest', $manifestPath, '-I', (Join-Path $plat 'android.jar')) + $resArgs) 'aapt2 link'
    # lib/ precisa ficar SEM compressao (zipalign alinha o .so a 16 KB). O `jar`
    # do JDK grava "stored"; sem ele, cai no .NET (deflate; ok com extractNativeLibs=true).
    if ($jar) {
        if (Test-Path (Join-Path $out 'assets')) { Invoke-Native $jar @('--update', '--no-manifest', '--file', $unsigned, '-C', $out, 'assets') 'jar (assets)' }
        Invoke-Native $jar @('--update', '--no-manifest', '--no-compress', '--file', $unsigned, '-C', (Join-Path $out 'apk'), 'lib') 'jar (lib)'
    } else {
        Add-Type -AssemblyName System.IO.Compression, System.IO.Compression.FileSystem
        $zip = [System.IO.Compression.ZipFile]::Open($unsigned, 'Update')
        try {
            $roots = @(@{ Dir = (Join-Path $out 'apk'); Sub = 'lib' })
            if (Test-Path (Join-Path $out 'assets')) { $roots += @{ Dir = $out; Sub = 'assets' } }
            foreach ($rt in $roots) {
                $base = $rt.Dir
                Get-ChildItem (Join-Path $base $rt.Sub) -Recurse -File | ForEach-Object {
                    $rel = $_.FullName.Substring($base.Length).TrimStart('\', '/').Replace('\', '/')
                    [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($zip, $_.FullName, $rel, 'Optimal')
                }
            }
        } finally { $zip.Dispose() }
    }
    Invoke-Native $zipalign @('-f', '-P', '16', '4', $unsigned, $aligned) 'zipalign'

    $ks = if ($env:OFS_ANDROID_KEYSTORE) { $env:OFS_ANDROID_KEYSTORE } else { Join-Path $env:USERPROFILE '.android\debug.keystore' }
    if (-not (Test-Path $ks)) {
        $ks = Join-Path $cache 'debug.keystore'
        if (-not (Test-Path $ks)) {
            Write-AndroidStep "gerando keystore de debug ($ks)"
            Invoke-Native $keytool @('-genkeypair', '-keystore', $ks, '-storepass', 'android', '-keypass', 'android',
                '-alias', 'androiddebugkey', '-keyalg', 'RSA', '-keysize', '2048', '-validity', '10000',
                '-dname', 'CN=Android Debug,O=Android,C=US') 'keytool'
        }
    }
    New-Item -ItemType Directory -Force -Path (Split-Path $Output -Parent) | Out-Null
    Invoke-Native $apksigner @('sign', '--ks', $ks, '--ks-pass', 'pass:android', '--key-pass', 'pass:android',
        '--ks-key-alias', 'androiddebugkey', '--out', $Output, $aligned) 'apksigner'
    Remove-Item $unsigned, $aligned, "$Output.idsig" -ErrorAction SilentlyContinue
    Write-Output "APK: $Output"

    if ($Install) {
        $adb = Join-Path $sdk 'platform-tools\adb.exe'
        if (-not (Test-Path $adb)) {
            $c = Get-Command adb.exe -ErrorAction SilentlyContinue
            if ($c) { $adb = $c.Path } else { Stop-Android 'adb nao encontrado. Instale: sdkmanager "platform-tools"' }
        }
        $adbArgs = @()
        $ser = if ($Serial) { $Serial } else { $env:ANDROID_SERIAL }
        if ($ser) { $adbArgs += @('-s', $ser) }
        Write-AndroidStep "instalando e abrindo $pkg"
        Invoke-Native $adb ($adbArgs + @('install', '-r', $Output)) 'adb install'
        Invoke-Native $adb ($adbArgs + @('shell', 'am', 'start', '-n', "$pkg/android.app.NativeActivity")) 'adb shell am start'
    }
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
$targetOs = "native"
$androidManifest = ""
$androidInstall = $false
$androidSerial = ""

if ($mode -notin @("check", "tokens", "ast", "ir", "asm", "build", "run", "assay", "odl", "oes", "translate")) {
    $inputFile = $mode
    $extension = [IO.Path]::GetExtension($inputFile).ToLowerInvariant()
    if ($extension -eq ".odl") { $mode = "odl" }
    elseif ($extension -eq ".oes") { $mode = "oes" }
    else { $mode = "run" }
}

$runNative = $false

for ($i = 0; $i -lt $rest.Count; $i++) {
    if ($rest[$i] -in @("-o", "--output")) {
        $i++
        if ($i -lt $rest.Count) { $output = $rest[$i] }
    } elseif ($rest[$i] -eq "--to") {
        $i++
        if ($i -lt $rest.Count) { $translateTo = $rest[$i] }
    } elseif ($rest[$i] -in @("--target", "-t")) {
        $i++
        if ($i -lt $rest.Count) { $targetOs = $rest[$i] }
    } elseif ($rest[$i] -like "--target=*") {
        $targetOs = $rest[$i].Substring(9)
    } elseif ($rest[$i] -eq "--manifest") {
        $i++
        if ($i -lt $rest.Count) { $androidManifest = $rest[$i] }
    } elseif ($rest[$i] -eq "--install") {
        $androidInstall = $true
    } elseif ($rest[$i] -eq "--serial") {
        $i++
        if ($i -lt $rest.Count) { $androidSerial = $rest[$i] }
    } elseif ($rest[$i] -eq "--native") {
        $runNative = $true
    } elseif ($rest[$i] -eq "--release") {
        # contratos (bearing/settle/grain): sem checagem em execucao
        $env:OFS_CONTRACTS = "0"
    } elseif (-not $inputFile) {
        $inputFile = $rest[$i]
    }
}

if (-not $inputFile) {
    Show-Usage
    exit 2
}

if ($mode -eq "build" -and $targetOs -eq "android") {
    Invoke-AndroidBuild -InputFile $inputFile -Output $output -Manifest $androidManifest -Install $androidInstall -Serial $androidSerial
    exit 0
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
    $runtime = Join-Path $PSScriptRoot "magma.o"
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

if ($mode -eq "assay") {
    # casos de borda das veins com contrato (A5.1), interpretado
    Invoke-OfsccEnv -InputPath $inputFile -Mode "assay"
    exit $LASTEXITCODE
}

if ($mode -eq "run" -and -not $runNative) {
    Invoke-OfsccEnv -InputPath $inputFile -Mode "run"
    exit $LASTEXITCODE
}

if ($mode -in @("build", "run")) {
    if (-not $output) { $output = $name }
    $runtime = Join-Path $PSScriptRoot "magma.o"
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
