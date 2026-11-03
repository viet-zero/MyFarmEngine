param(
    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',
    [ValidateSet('phase1','phase2','phase-a','phase-b')]
    [string]$Phase = 'phase-b',
    [switch]$SkipGraphicsTests
)
$ErrorActionPreference = 'Stop'

# Use the installed Visual Studio toolchain without hard-coded edition/version.
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
if (!(Test-Path -LiteralPath $vswhere)) {
    throw 'Install Visual Studio with Desktop development with C++ and C++ CMake tools.'
}
$installation = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 Microsoft.VisualStudio.Component.VC.CMake.Project -property installationPath
if (!$installation) { throw 'Visual Studio C++ compiler and CMake tools were not found.' }
& (Join-Path $installation 'Common7/Tools/Launch-VsDevShell.ps1') -Arch amd64 -HostArch amd64 -SkipAutomaticLocation
$tools = Join-Path $installation 'Common7/IDE/CommonExtensions/Microsoft/CMake'
$env:PATH = (Join-Path $tools 'CMake/bin') + ';' + (Join-Path $tools 'Ninja') + ';' + $env:PATH
$preset = $Phase + '-' + $Configuration.ToLowerInvariant()

Push-Location (Split-Path $PSScriptRoot -Parent)
try {
    & (Join-Path $PSScriptRoot 'PrepareAssets.ps1')
    cmake --preset $preset
    if ($LASTEXITCODE -ne 0) { throw 'CMake configure failed.' }
    cmake --build --preset $preset
    if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
    if ($SkipGraphicsTests) {
        ctest --preset $preset -LE graphics
    } else {
        ctest --preset $preset
    }
    if ($LASTEXITCODE -ne 0) { throw 'Tests failed; inspect out/build/<preset>/Testing/Temporary/LastTest.log.' }
} finally {
    Pop-Location
}
