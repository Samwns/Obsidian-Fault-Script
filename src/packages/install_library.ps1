param(
  [Parameter(Mandatory=$true)]
  [string]$Package
)

if (-not (Test-Path $Package)) {
  Write-Error "Package not found: $Package"
  exit 1
}

$libHome = if ($env:OFS_LIB_HOME) { $env:OFS_LIB_HOME } else { Join-Path $HOME ".ofs/libs" }
$tempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("ofs_pkg_" + [guid]::NewGuid().ToString())
New-Item -ItemType Directory -Force -Path $tempDir | Out-Null

try {
  tar -xzf $Package -C $tempDir
  New-Item -ItemType Directory -Force -Path $libHome | Out-Null
  Get-ChildItem -Path $tempDir -Recurse -Filter *.ofs | ForEach-Object {
    Copy-Item $_.FullName -Destination $libHome -Force
  }

  Write-Host "Installed OFS libraries to: $libHome"
  Write-Host "Set OFS_LIB_PATH to include this path:"
  Write-Host "  `$env:OFS_LIB_PATH = \"$libHome\""
}
finally {
  if (Test-Path $tempDir) {
    Remove-Item $tempDir -Recurse -Force
  }
}
