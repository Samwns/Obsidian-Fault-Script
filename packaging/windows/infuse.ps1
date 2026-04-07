param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [object[]] $ArgsFromUser
)

$ofsExe = Join-Path $PSScriptRoot 'ofs.exe'
if (-not (Test-Path $ofsExe)) {
    $ofsExe = 'ofs'
}

$forwarded = @()
foreach ($arg in $ArgsFromUser) {
    if ($null -eq $arg) { continue }
    $forwarded += $arg.ToString()
}

& $ofsExe 'infuse' @forwarded
exit $LASTEXITCODE
