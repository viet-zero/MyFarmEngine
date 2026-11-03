param([string]$Downloads = (Join-Path $env:USERPROFILE 'Downloads'))
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
$root = [IO.Path]::GetFullPath((Split-Path $PSScriptRoot -Parent))
$records = [Collections.Generic.List[object]]::new()

function Import-Entries([string]$ArchiveName, [scriptblock]$SelectTarget) {
    $archivePath = Join-Path $Downloads $ArchiveName
    $archive = [IO.Compression.ZipFile]::OpenRead($archivePath)
    try {
        foreach ($entry in $archive.Entries) {
            $relative = & $SelectTarget $entry.FullName
            if (!$relative) { continue }
            $target = [IO.Path]::GetFullPath((Join-Path $root $relative))
            if (!$target.StartsWith($root + [IO.Path]::DirectorySeparatorChar,
                                   [StringComparison]::OrdinalIgnoreCase)) {
                throw 'Archive destination escapes the repository.'
            }
            [IO.Directory]::CreateDirectory((Split-Path $target -Parent)) | Out-Null
            if (Test-Path -LiteralPath $target) {
                throw "Refusing to overwrite existing asset: $relative"
            }
            [IO.Compression.ZipFileExtensions]::ExtractToFile($entry, $target, $false)
            $records.Add([pscustomobject]@{
                file = $relative.Replace('\','/')
                source_archive = $ArchiveName
                source_entry = $entry.FullName
                sha256 = (Get-FileHash -LiteralPath $target -Algorithm SHA256).Hash.ToLowerInvariant()
                review_status = 'imported_original_needs_review'
            })
        }
    } finally { $archive.Dispose() }
}

Import-Entries 'SNHR_Game_Assets_v11_Reclassified.zip' {
    param($name)
    if ($name -match '^(Characters|Animals)/.+/sprite\.png$' -or
        $name -match '^(PREVIEW_.+\.jpg|entity_manifest\.csv|README_VI\.txt)$') {
        return "Assets/Library/SNHR_v11/$name"
    }
}
Import-Entries 'snhr_game_assets_v8.zip' {
    param($name)
    if ($name -match '^snhr_game_assets_v6/16_Individual_Entities_Trimmed/((Items_Food|Crops_Plants|Environment)/.+\.png)$') {
        return "Assets/Library/SNHR_v8/$($Matches[1])"
    }
}
Import-Entries 'MyFarmEngine_All_Updates.zip' {
    param($name)
    if ($name -match '^MyFarmEngine_All_Updates/[^/]+/([^/]+\.png)$') {
        return "Docs/History/Previews/$($Matches[1])"
    }
}
$records | Sort-Object file | Export-Csv -LiteralPath (Join-Path $root 'Assets/Library/import_manifest.csv') -NoTypeInformation -Encoding UTF8
Write-Output "Imported $($records.Count) files. Original archives were not modified."
