param(
    [string]$RoadmapPath = "ROADMAP.md",
    [switch]$Require100
)

if (-not (Test-Path $RoadmapPath)) {
    Write-Error "Roadmap not found: $RoadmapPath"
    exit 2
}

$lines = Get-Content $RoadmapPath

$done = 0
$partial = 0
$todo = 0

foreach ($line in $lines) {
    if ($line -match ".*- \[ \]") {
        $todo++
        continue
    }

    if ($line -match ".*- \[~\]") {
        $partial++
        continue
    }

    if ($line -match ".*- \[" -and $line -match "\]") {
        $done++
        continue
    }
}

$total = $done + $partial + $todo
if ($total -eq 0) {
    Write-Host "No checklist items found in $RoadmapPath"
    exit 3
}

# Partial items count as 0.5 progress
$score = ($done + ($partial * 0.5))
$percent = [math]::Round(($score / $total) * 100, 2)

Write-Host "Roadmap checklist summary"
Write-Host "  Done:    $done"
Write-Host "  Partial: $partial"
Write-Host "  Todo:    $todo"
Write-Host "  Total:   $total"
Write-Host ("  Progress: {0}%" -f $percent)

if ($Require100) {
    if ($percent -lt 100) {
        Write-Error ("Roadmap is not 100 percent complete yet ({0}%)." -f $percent)
        exit 1
    }
    Write-Host "Roadmap is 100% complete."
}
