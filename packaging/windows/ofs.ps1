param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [object[]] $ArgsFromUser
)

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
