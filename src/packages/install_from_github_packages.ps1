param(
  [Parameter(Mandatory=$true)]
  [string]$LibraryName,
  [string]$Version = "latest"
)

$owner = if ($env:OFS_PKG_OWNER) { $env:OFS_PKG_OWNER.ToLower() } else { "samwns" }
$package = "@$owner/ofs-lib-$LibraryName@$Version"

if (-not $env:NPM_TOKEN) {
  Write-Error "NPM_TOKEN is required to download from GitHub Packages"
  exit 1
}

$libHome = if ($env:OFS_LIB_HOME) { $env:OFS_LIB_HOME } else { Join-Path $HOME ".ofs/libs" }
$tempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("ofs_pkg_" + [guid]::NewGuid().ToString())
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

try {
  @"
@$owner:registry=https://npm.pkg.github.com
//npm.pkg.github.com/:_authToken=$($env:NPM_TOKEN)
always-auth=true
"@ | Set-Content -Path (Join-Path $tempDir ".npmrc") -NoNewline

  Push-Location $tempDir
  npm pack $package | Out-Null
  tar -xzf *.tgz
  Pop-Location

  New-Item -ItemType Directory -Force -Path $libHome | Out-Null
  Get-ChildItem -Path (Join-Path $tempDir "package/libs") -Filter *.ofs | ForEach-Object {
    Copy-Item $_.FullName -Destination $libHome -Force
  }

  Write-Host "Installed $package into $libHome"
}
finally {
  if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
  }
}
