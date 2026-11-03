$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$root = Split-Path $PSScriptRoot -Parent
$assets = Join-Path $root 'Assets'
$manifestPath = Join-Path $root 'Gameplay/Data/Assets.csv'
foreach ($folder in @('Original','Workspace','Export')) {
    [IO.Directory]::CreateDirectory((Join-Path $assets $folder)) | Out-Null
}

function Copy-Once([string]$source, [string]$destination) {
    [IO.Directory]::CreateDirectory((Split-Path $destination -Parent)) | Out-Null
    if (!(Test-Path -LiteralPath $destination)) {
        Copy-Item -LiteralPath $source -Destination $destination
    }
}

if (!(Test-Path -LiteralPath $manifestPath)) {
    $records = [Collections.Generic.List[object]]::new()
    $originals = [Collections.Generic.List[object]]::new()
    $files = @(Get-ChildItem (Join-Path $assets 'Art'),(Join-Path $assets 'Backgrounds'),(Join-Path $assets 'Library') -Recurse -File -Filter '*.png')
    $files += @(Get-ChildItem (Join-Path $root 'Docs/History/Previews') -File -Filter '*.png')
    foreach ($file in ($files | Sort-Object FullName)) {
        $relative = $file.FullName.Substring($assets.Length+1).Replace('\','/')
        $history = $file.FullName.StartsWith((Join-Path $root 'Docs/History/Previews'),[StringComparison]::OrdinalIgnoreCase)
        if ($history) { $relative = 'History/' + $file.Name }
        $id = [IO.Path]::ChangeExtension($relative,$null).TrimEnd('.').ToLowerInvariant() -replace '[^a-z0-9]+','_'
        $kind='reference'; $ready=$false; $alpha=$false; $columns=1; $rows=1
        if ($relative.StartsWith('Library/')) {
            if ($relative -match '/Characters/') {$kind='character'}
            elseif ($relative -match '/Animals/') {$kind='animal'}
            elseif ($relative -match '/Crops_Plants/') {$kind='crop'}
            elseif ($relative -match '/Environment/') {$kind='environment'}
            else {$kind='item'}
        } else { $ready=$true }
        switch ($relative) {
            'Art/Characters/farmer_linh.png' {$id='player_linh';$kind='character';$alpha=$true}
            'Art/Characters/farmer_an.png' {$id='player_an';$kind='character';$alpha=$true}
            'Art/Items/farm_items_atlas.png' {$id='items_farm';$kind='item_atlas';$alpha=$true;$columns=2;$rows=2}
            'Art/World/garden_world.png' {$id='world_garden';$kind='background'}
            'Backgrounds/cozy_farm_morning.png' {$id='background_morning';$kind='background'}
        }
        $image = [Drawing.Image]::FromFile($file.FullName)
        try {
            $width=$image.Width; $height=$image.Height
            $alpha=[bool]($image.PixelFormat -band [Drawing.Imaging.PixelFormat]::Alpha)
        } finally { $image.Dispose() }
        $original = Join-Path $assets ('Original/' + $relative)
        Copy-Once $file.FullName $original
        $hash=(Get-FileHash -LiteralPath $file.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        if ((Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash.ToLowerInvariant() -ne $hash) {
            throw "Refusing to replace a different original: $relative"
        }
        $originals.Add([pscustomobject]@{source_file=$file.FullName.Substring($root.Length+1).Replace('\','/');original_path='Original/'+$relative;sha256=$hash})
        $records.Add([pscustomobject]@{
            asset_id=$id;kind=$kind;source='Original/'+$relative;export_path='Export/'+$relative
            columns=$columns;rows=$rows;frame_width=[int]($width/$columns);frame_height=[int]($height/$rows)
            anchor_x=0.5;anchor_y=1;has_alpha=$alpha.ToString().ToLowerInvariant()
            status=$(if($ready){'ready'}else{'needs_review'})
            first_frame=0;frame_count=$columns*$rows;margin_x=0;margin_y=0;spacing_x=0;spacing_y=0
        })
    }
    $records | Sort-Object asset_id | Export-Csv -LiteralPath $manifestPath -NoTypeInformation -Encoding UTF8
    $originals | Sort-Object original_path | Export-Csv -LiteralPath (Join-Path $assets 'Original/manifest.csv') -NoTypeInformation -Encoding UTF8
}

# Originals are immutable. Existing exports are never overwritten by this script.
$originals=Import-Csv -LiteralPath (Join-Path $assets 'Original/manifest.csv')
foreach($entry in $originals) {
    $path=Join-Path $assets $entry.original_path
    if (!(Test-Path -LiteralPath $path) -or (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash -ne $entry.sha256) {
        throw "Original missing or changed: $($entry.original_path)"
    }
}
$records=Import-Csv -LiteralPath $manifestPath
foreach($record in $records) {
    if ($record.status -notin @('ready','needs_review')) { continue }
    $targetPath=$record.export_path
    $targetPrefix='Export/'
    if($record.status -eq 'needs_review') {
        if(!$record.source.StartsWith('Original/')) { throw 'Invalid review source path.' }
        $targetPath='Workspace/'+$record.source.Substring(9)
        $targetPrefix='Workspace/'
    }
    # Refuse path traversal before this script copies any manifest-directed file.
    foreach($field in @('source','target')) {
        $value=$(if($field -eq 'source'){$record.source}else{$targetPath}).Replace('\','/')
        $prefix=if($field -eq 'source'){'Original/'}else{$targetPrefix}
        if (!$value.StartsWith($prefix) -or $value.Contains(':') -or ($value.Split('/') -contains '..')) {
            throw "Invalid pipeline path: $value"
        }
        $base=[IO.Path]::GetFullPath((Join-Path $assets $prefix))
        $resolved=[IO.Path]::GetFullPath((Join-Path $assets $value))
        if (!$resolved.StartsWith($base,[StringComparison]::OrdinalIgnoreCase)) { throw 'Path escaped pipeline directory.' }
        # No junction/reparse points in a copy destination or source chain.
        $parent=$resolved
        while($parent -and $parent.Length -ge $assets.Length) {
            if ((Test-Path -LiteralPath $parent) -and ((Get-Item -LiteralPath $parent -Force).Attributes -band [IO.FileAttributes]::ReparsePoint)) {
                throw "Reparse point is not a pipeline copy target: $parent"
            }
            $parent=Split-Path $parent -Parent
        }
    }
    Copy-Once (Join-Path $assets $record.source) (Join-Path $assets $targetPath)
}
Write-Output "Prepared $($originals.Count) preserved originals; $( @($records | Where-Object status -eq 'ready').Count ) export candidates. C++ validation is still required."
