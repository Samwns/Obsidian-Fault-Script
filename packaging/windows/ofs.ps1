param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [object[]] $ArgsFromUser
)

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

$ofsExe = Join-Path $PSScriptRoot 'ofs.exe'
if (-not (Test-Path $ofsExe)) {
    throw 'ofs.exe não encontrado ao lado do wrapper PowerShell.'
}

$forwarded = @()
foreach ($arg in $ArgsFromUser) {
    if ($null -eq $arg) { continue }
    $forwarded += $arg.ToString()
}

& $ofsExe @forwarded
exit $LASTEXITCODE
