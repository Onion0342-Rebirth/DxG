# DxG build script (g++ on Windows).
# Usage:
#   .\build.ps1                 # build dxg_core + dxg.exe (needs SDL2 if SDL path is given)
#   .\build.ps1 -SDL C:\SDL2    # specify SDL2 install prefix
#
# The dxg_core static library (everything except SDL platform) builds WITHOUT SDL2.

param(
    [string]$SDL = ""
)

$ErrorActionPreference = "Stop"

$root = $PSScriptRoot
$buildDir = Join-Path $root "build"
if (-not (Test-Path $buildDir)) { New-Item -ItemType Directory -Path $buildDir | Out-Null }

# ---- locate g++ ----
$gxx = $env:CXX
if (-not $gxx) {
    $candidates = @(
        "D:\APPs\w64devkit\w64devkit\bin\g++.exe",
        "C:\w64devkit\bin\g++.exe",
        "g++"
    )
    foreach ($c in $candidates) {
        $found = Get-Command $c -ErrorAction SilentlyContinue
        if ($found) { $gxx = $found.Source; break }
        if (Test-Path $c) { $gxx = $c; break }
    }
}
if (-not $gxx) { throw "g++ not found. Set CXX env var or install w64devkit." }

Write-Host "Using compiler: $gxx"

# ---- include dirs (one per module) ----
$moduleDirs = Get-ChildItem -Path (Join-Path $root "src") -Directory |
    Where-Object { Test-Path (Join-Path $_.FullName "include") }
$includes = @()
foreach ($m in $moduleDirs) {
    $includes += @("-I", (Join-Path $m.FullName "include"))
}

# ---- collect non-SDL core sources ----
$coreSources = Get-ChildItem -Path (Join-Path $root "src") -Recurse -Filter "*.cpp" |
    Where-Object { $_.FullName -notmatch 'platform[\\/]src' -and $_.FullName -notmatch '[\\/]main\.cpp$' } |
    ForEach-Object { $_.FullName }

$coreObj = Join-Path $buildDir "libdxg_core.a"

Write-Host "Compiling dxg_core ($($coreSources.Count) sources)..."
$objFiles = @()
foreach ($src in $coreSources) {
    $name = [IO.Path]::GetFileNameWithoutExtension($src)
    $obj = Join-Path $buildDir ("$name.o")
    & $gxx -std=c++17 -O2 -Wall -Wextra $includes -c $src -o $obj
    if ($LASTEXITCODE -ne 0) { throw "compile failed: $src" }
    $objFiles += $obj
}
& $gxx -rcs $coreObj @objFiles
Write-Host " -> $coreObj"

# ---- dxg.exe (main + SDL platform) ----
$mainCpp = Join-Path $root "src\main.cpp"
$sdlCpp  = Join-Path $root "src\platform\src\SdlPlatform.cpp"
$exe = Join-Path $buildDir "dxg.exe"

$sdlArgs = @()
if ($SDL -ne "") {
    $sdlArgs += @("-I" + (Join-Path $SDL "include/SDL2"))
    $sdlArgs += @("-L" + (Join-Path $SDL "lib"))
    $sdlArgs += @("-lSDL2")
} else {
    Write-Host "No -SDL path given; attempting default SDL2 link flags (-lSDL2)."
    $sdlArgs += @("-lSDL2")
}

Write-Host "Linking dxg.exe..."
& $gxx -std=c++17 -O2 $includes $mainCpp $sdlCpp $coreObj @sdlArgs -o $exe -mwindows
if ($LASTEXITCODE -ne 0) {
    Write-Warning "Link failed (likely missing SDL2). dxg_core.a was built successfully."
    Write-Warning "Re-run with -SDL <path-to-SDL2-prefix>, or use CMake."
    exit 1
}
Write-Host " -> $exe"
Write-Host "Done."
