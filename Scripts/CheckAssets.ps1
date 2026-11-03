$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$manifest = Import-Csv -LiteralPath (Join-Path $root 'Assets/Library/import_manifest.csv')
$failed = @()
foreach ($record in $manifest) {
    $path = Join-Path $root $record.file
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) {
        $failed += "Missing: $($record.file)"
    } elseif ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $record.sha256) {
        $failed += "Changed since import: $($record.file)"
    }
}
if ($failed.Count -gt 0) { throw ($failed -join [Environment]::NewLine) }
Write-Output "PASS: $($manifest.Count) imported files match their recorded SHA256."
Write-Output 'These checks verify archive integrity, not sprite visual quality or animation readiness.'
