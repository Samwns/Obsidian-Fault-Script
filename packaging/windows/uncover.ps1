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
    $localWrapper = Join-Path $PSScriptRoot 'ofs.ps1'
    $resolvedOfs = Get-Command 'ofs' -All -ErrorAction SilentlyContinue |
        Where-Object { $_.Path -and $_.Path -ne $localWrapper } |
        Select-Object -First 1
    if ($null -eq $resolvedOfs) {
        throw 'Nenhum comando ofs instalado foi encontrado fora desta pasta.'
    }
    $ofsExe = $resolvedOfs.Path
}

$forwarded = @()
foreach ($arg in $ArgsFromUser) {
    if ($null -eq $arg) { continue }
    $forwarded += $arg.ToString()
}

& $ofsExe 'uncover' @forwarded
exit $LASTEXITCODE
