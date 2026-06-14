param(
    [switch] $AcceptLicense = $false,
    [ValidateSet("User", "Machine")]
    [string] $Scope = "User",
    [string] $InstallDir = ""
)

$ErrorActionPreference = "Stop"
$PackageDir = $PSScriptRoot

if (-not $InstallDir) {
    if ($Scope -eq "Machine") {
        $InstallDir = Join-Path $env:ProgramFiles "Obsidian Fault Script"
    } else {
        $InstallDir = Join-Path $env:LOCALAPPDATA "Programs\Obsidian Fault Script"
    }
}

function Show-Banner {
@"

========================================
 Obsidian Fault Script Installer
========================================

"@ | Write-Host
}

function Confirm-License {
    Show-Banner
    Write-Host "License: Boost Software License 1.0"
    Write-Host "License file: $PackageDir\LICENSE.txt"
    Write-Host ""
    Write-Host "This installer will install OFS command-line tools, the self-hosted"
    Write-Host "compiler, standard library files, and native runtime files."
    Write-Host ""

    if ($AcceptLicense -or $env:OFS_ACCEPT_LICENSE -in @("1", "true", "TRUE", "yes", "YES")) {
        Write-Host "[OFS] License accepted non-interactively."
        return
    }

    $answer = Read-Host "Do you accept the license terms and continue? [y/N]"
    if ($answer -notin @("y", "Y", "yes", "YES", "Yes")) {
        Write-Host "[OFS] Installation cancelled."
        exit 2
    }
    Write-Host "[OFS] License accepted."
}

function Add-ToPath {
    param([string] $Dir, [string] $TargetScope)

    $target = if ($TargetScope -eq "Machine") { "Machine" } else { "User" }
    $current = [Environment]::GetEnvironmentVariable("Path", $target)
    $parts = @()
    if ($current) {
        $parts = $current -split ';' | Where-Object { $_ }
    }
    if ($parts -notcontains $Dir) {
        $newPath = (($parts + $Dir) -join ';')
        [Environment]::SetEnvironmentVariable("Path", $newPath, $target)
        $env:PATH = "$Dir;$env:PATH"
        Write-Host "[OFS] Added to $target PATH: $Dir"
    } else {
        Write-Host "[OFS] PATH already contains: $Dir"
    }
}

Confirm-License

if (-not (Test-Path (Join-Path $PackageDir "ofscc.exe"))) {
    throw "ofscc.exe not found next to installer script."
}

Write-Host "[OFS] Installing to $InstallDir"
New-Item -ItemType Directory -Path $InstallDir -Force | Out-Null

$files = @(
    "ofscc.exe", "ofs.cmd", "ofs.ps1",
    "infuse.cmd", "infuse.ps1",
    "uncover.cmd", "uncover.ps1",
    "reinfuse.cmd", "reinfuse.ps1",
    "libofs_runtime.a", "ofs_runtime.lib",
    "LICENSE.txt"
)

foreach ($file in $files) {
    $src = Join-Path $PackageDir $file
    if (Test-Path $src) {
        Copy-Item $src (Join-Path $InstallDir $file) -Force
    }
}

if (Test-Path (Join-Path $PackageDir "stdlib")) {
    Copy-Item (Join-Path $PackageDir "stdlib") (Join-Path $InstallDir "stdlib") -Recurse -Force
}

if (Test-Path (Join-Path $PackageDir "ofscc")) {
    Copy-Item (Join-Path $PackageDir "ofscc") (Join-Path $InstallDir "ofscc") -Recurse -Force
}

Add-ToPath -Dir $InstallDir -TargetScope $Scope

Write-Host "[OFS] Installed."
& (Join-Path $InstallDir "ofs.cmd") version
