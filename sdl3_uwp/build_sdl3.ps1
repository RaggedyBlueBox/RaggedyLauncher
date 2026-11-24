param(
    [string]$OutputDir
)

$ErrorActionPreference = "Stop"
. (Join-Path $PSScriptRoot "..\scripts\common.ps1")

if (-not $OutputDir) {
    $OutputDir = Join-Path (Get-ConfigPath "BuildDir") "sdl3-uwp"
}
Ensure-Dir $OutputDir

& (Join-Path (Split-Path $PSScriptRoot -Parent) "scripts\prepare-sdl3-uwp.ps1") -OutputPath (Join-Path $OutputDir "SDL3.dll")
if (-not $?) { throw "SDL3-UWP build failed." }
