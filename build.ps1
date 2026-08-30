# DxG build script (g++ on Windows).
# Usage:
#   .\build.ps1                 # build dxg_core + dxg.exe (needs SDL2; uses -lSDL2 on PATH)
#   .\build.ps1 -SDL C:\SDL2    # specify SDL2 install prefix
#                                # (accepts either the dir containing include/lib/bin,
#                                 #  or the extracted folder that contains x86_64-w64-mingw32/)
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
$toolDir = Split-Path $gxx -Parent

Write-Host "Using compiler: $gxx"

# ---- locate ar (archivist) next to the compiler, else on PATH ----
$ar = Join-Path $toolDir "ar.exe"
if (-not (Test-Path $ar)) { $ar = "ar.exe" }
$arCmd = Get-Command $ar -ErrorAction SilentlyContinue
if ($arCmd) { $ar = $arCmd.Source }

# ---- resolve SDL2 prefix: accept either the prefix itself or the extracted mingw folder ----
function Resolve-SdlPrefix([string]$p) {
    if (-not $p) { return "" }
    if (Test-Path (Join-Path $p "include/SDL2/SDL.h")) { return $p }
    foreach ($sub in @("x86_64-w64-mingw32", "i686-w64-mingw32", "mingw64", "mingw32")) {
        $cand = Join-Path $p $sub
        if (Test-Path (Join-Path $cand "include/SDL2/SDL.h")) { return $cand }
    }
    return $p
}
$sdlPrefix = Resolve-SdlPrefix $SDL

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
    $obj = Join-Path $buildDir "$name.o"
    & $gxx -std=c++17 -O2 -Wall -Wextra $includes -c $src -o $obj
    if ($LASTEXITCODE -ne 0) { throw "compile failed: $src" }
    $objFiles += $obj
}
Remove-Item $coreObj -ErrorAction SilentlyContinue
& $ar rcs $coreObj @objFiles
if ($LASTEXITCODE -ne 0) { throw "archive failed: $ar rcs $coreObj" }
Write-Host " -> $coreObj"

# ---- dxg.exe (main + SDL platform) ----
$mainCpp = Join-Path $root "src\main.cpp"
$sdlCpp  = Join-Path $root "src\platform\src\SdlPlatform.cpp"
$exe = Join-Path $buildDir "dxg.exe"

$sdlArgs = @()
if ($sdlPrefix -ne "") {
    Write-Host "Using SDL2 prefix: $sdlPrefix"
    $sdlArgs += @("-I" + (Join-Path $sdlPrefix "include/SDL2"))
    $sdlArgs += @("-L" + (Join-Path $sdlPrefix "lib"))
} else {
    Write-Host "No -SDL path given; attempting default SDL2 link flags."
}
# MinGW GUI subsystem (-mwindows) needs SDL2main to provide WinMain; order matters.
$sdlArgs += @("-lSDL2main", "-lSDL2")

Write-Host "Linking dxg.exe..."
& $gxx -std=c++17 -O2 $includes $mainCpp $sdlCpp $coreObj @sdlArgs -o $exe -mwindows
if ($LASTEXITCODE -ne 0) {
    Write-Warning "Link failed (likely missing SDL2). dxg_core.a was built successfully."
    Write-Warning "Re-run with -SDL <path-to-SDL2-prefix>, or use CMake."
    exit 1
}
Write-Host " -> $exe"

# ---- copy SDL2.dll next to dxg.exe so it runs without extra PATH setup ----
$dllCandidates = @()
if ($sdlPrefix -ne "") { $dllCandidates += (Join-Path $sdlPrefix "bin/SDL2.dll") }
$onPath = Get-Command "SDL2.dll" -ErrorAction SilentlyContinue
if ($onPath) { $dllCandidates += $onPath.Source }
foreach ($dll in $dllCandidates) {
    if ($dll -and (Test-Path $dll)) {
        Copy-Item $dll (Join-Path $buildDir "SDL2.dll") -Force
        Write-Host "Copied SDL2.dll -> build/SDL2.dll"
        break
    }
}
Write-Host "Done. Run: $exe"
