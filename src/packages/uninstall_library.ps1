param(
  [Parameter(Mandatory=$true)]
  [string]$LibraryName
)

$libHome = if ($env:OFS_LIB_HOME) { $env:OFS_LIB_HOME } else { Join-Path $HOME ".ofs/libs" }

if (-not (Test-Path $libHome)) {
  Write-Error "Library directory not found: $libHome"
  exit 1
}

$found = $false
Get-ChildItem -Path $libHome -Filter "*$LibraryName*.ofs" -ErrorAction SilentlyContinue | ForEach-Object {
  Remove-Item $_.FullName -Force
  Write-Host "Removed: $($_.FullName)"
  $found = $true
}

if (-not $found) {
  Write-Error "No libraries found matching: $LibraryName"
  exit 1
}

Write-Host "Library '$LibraryName' uninstalled successfully"
