param([ValidateSet('Debug','Release')][string]$Configuration='Release')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$source = Join-Path $root 'AssetLibrary_Review/MyFarmEngine_Complete_Library/Rework/04_Curated'
$build = Join-Path $root ('out/build/phase-f-' + $Configuration.ToLowerInvariant())
if (!(Test-Path -LiteralPath (Join-Path $build 'MyFarmEngine.exe'))) {
    throw "Build phase-f $Configuration first."
}
$manifest = Import-Csv -LiteralPath (Join-Path $source 'manifest.csv')
$selected = [ordered]@{
    crp_carrot_sprout = 'carrot_sprout.png'
    crp_carrot_young = 'carrot_young.png'
    crp_carrot_mature = 'carrot_mature.png'
    carrot = 'carrot_icon.png'
    seed_bag = 'seed_bag.png'
}
$destination = Join-Path $build 'Assets/LocalReview'
[IO.Directory]::CreateDirectory($destination) | Out-Null
foreach ($id in $selected.Keys) {
    $relative = if ($id.StartsWith('crp_')) { "Assets/Crops/$id/$id.png" } else { "Assets/Items/$id/$id.png" }
    $record = @($manifest | Where-Object { $_.id -eq $id -and $_.file -eq $relative })
    if ($record.Count -ne 1) { throw "Unexpected manifest entry for $id" }
    Copy-Item -LiteralPath (Join-Path $source $relative) -Destination (Join-Path $destination $selected[$id])
}
Write-Output "Staged five local review images in $destination"
