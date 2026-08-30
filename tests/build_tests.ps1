# DxG 测试构建与运行脚本（g++，不依赖 SDL2）。
# 用法：在 DxG 目录下执行  .\tests\build_tests.ps1
# 作用：编译 dxg_core（非平台、非 main 的全部源码）+ tests 下全部用例，
#       链接为 build\dxg_tests.exe 并立即运行；退出码非 0 表示有用例失败。

$ErrorActionPreference = "Stop"

$root = $PSScriptRoot | Split-Path -Parent   # DxG 根目录
$buildDir = Join-Path $root "build"
if (-not (Test-Path $buildDir)) { New-Item -ItemType Directory -Path $buildDir | Out-Null }

# ---- 定位 g++ ----
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

# ---- include 路径：各模块 include 根 + tests 目录（供 #include "framework/..."）----
$moduleDirs = Get-ChildItem -Path (Join-Path $root "src") -Directory |
    Where-Object { Test-Path (Join-Path $_.FullName "include") }
$includes = @()
foreach ($m in $moduleDirs) { $includes += @("-I", (Join-Path $m.FullName "include")) }
$includes += @("-I", (Join-Path $root "tests"))

# ---- 源文件：dxg_core（排除 SDL 平台与 main）+ 全部测试 ----
$coreSources = Get-ChildItem -Path (Join-Path $root "src") -Recurse -Filter "*.cpp" |
    Where-Object { $_.FullName -notmatch 'platform[\\/]src' -and $_.FullName -notmatch '[\\/]main\.cpp$' }
$testSources = Get-ChildItem -Path (Join-Path $root "tests") -Recurse -Filter "*.cpp"

$allSources = @()
$allSources += $coreSources | ForEach-Object { $_.FullName }
$allSources += $testSources | ForEach-Object { $_.FullName }

Write-Host ("编译 {0} 个 dxg_core 源 + {1} 个测试源 ..." -f $coreSources.Count, $testSources.Count)

$exe = Join-Path $buildDir "dxg_tests.exe"
& $gxx -std=c++17 -O2 -Wall -Wextra $includes $allSources -o $exe
if ($LASTEXITCODE -ne 0) { throw "测试编译失败。" }
Write-Host " -> $exe"

# ---- 运行：在 build 目录下执行，使相对路径 "../assets" 命中 DxG/assets ----
# 控制台/PowerShell 宿主默认代码页可能是 GBK(936)，切到 UTF-8(65001) 以正确显示中文输出。
try { [Console]::OutputEncoding = [System.Text.Encoding]::UTF8 } catch {}
try { chcp 65001 > $null } catch {}
Write-Host "运行测试..."
Push-Location $buildDir
try {
    & $exe
    $code = $LASTEXITCODE
} finally {
    Pop-Location
}
if ($code -ne 0) { throw "存在测试失败（退出码 $code）。" }
Write-Host "全部测试通过。"
