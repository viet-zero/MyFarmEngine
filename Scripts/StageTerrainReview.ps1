param(
    [ValidateSet('Debug','Release')][string]$Configuration='Release',
    [ValidateSet('spring','summer','autumn','winter')][string]$Season='spring'
)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$source = Join-Path $root "AssetLibrary_Review/Terrain_v3_Local/Tiles/$Season/Soil"
$build = Join-Path $root ('out/build/phase-f-' + $Configuration.ToLowerInvariant())
if (!(Test-Path -LiteralPath (Join-Path $build 'MyFarmEngine.exe'))) {
    throw "Build phase-f $Configuration first."
}
$destination = Join-Path $build 'Assets/LocalReview'
[IO.Directory]::CreateDirectory($destination) | Out-Null
Copy-Item -LiteralPath (Join-Path $source 'tilled_horizontal.png') -Destination (Join-Path $destination 'soil_dry.png')
Copy-Item -LiteralPath (Join-Path $source 'tilled_watered_horizontal.png') -Destination (Join-Path $destination 'soil_wet.png')
Write-Output "Staged $Season soil review in $destination"
