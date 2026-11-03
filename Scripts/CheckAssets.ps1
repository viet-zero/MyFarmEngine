$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$manifest = Import-Csv -LiteralPath (Join-Path $root 'Assets/Library/import_manifest.csv')
$generated = Import-Csv -LiteralPath (Join-Path $root 'Assets/Art/manifest.csv')
$originals = @(Import-Csv -LiteralPath (Join-Path $root 'Assets/Original/manifest.csv') | ForEach-Object {
    [pscustomobject]@{file='Assets/'+$_.original_path;sha256=$_.sha256}
})
$failed = @()
foreach ($record in @($manifest) + @($generated) + @($originals)) {
    $path = Join-Path $root $record.file
    if (!(Test-Path -LiteralPath $path -PathType Leaf)) {
        $failed += "Missing: $($record.file)"
    } elseif ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $record.sha256) {
        $failed += "Changed since import: $($record.file)"
    }
}
if ($failed.Count -gt 0) { throw ($failed -join [Environment]::NewLine) }
Write-Output "PASS: $($manifest.Count) imported files and $($generated.Count) new artwork files match their recorded SHA256."
Write-Output "PASS: $($originals.Count) immutable Original snapshots match their recorded SHA256."
Write-Output 'These checks verify archive integrity, not sprite visual quality or animation readiness.'
