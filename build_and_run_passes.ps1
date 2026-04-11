# Build and run. If direct gcc fails with no output, retry inside MSYS2 bash.
$ErrorActionPreference = "Continue"
$root = "D:\__DOWNLOADS\selforglinux"
$gcc = "C:\msys64\ucrt64\bin\gcc.exe"
$bash = "C:\msys64\usr\bin\bash.exe"
$outExe = "$root\passes.exe"

Write-Host "Building (direct gcc)..."
Set-Location $root
& $gcc -O2 -std=c17 -Wall -Wextra compiler_passes.c -o $outExe -lm *> "$root\build_log.txt"
$exit = $LASTEXITCODE
Write-Host "gcc exit code: $exit"

$log = Get-Content "$root\build_log.txt" -Raw -ErrorAction SilentlyContinue
if ($log) { Write-Host "--- gcc output ---"; Write-Host $log }

if (Test-Path $outExe) {
    Write-Host "Running passes.exe..."
    & $outExe
    exit
}

# No exe and no output: run gcc inside MSYS2 so we see errors
Write-Host "Retrying inside MSYS2 bash..."
$bashCmd = "cd /d/__DOWNLOADS/selforglinux && /ucrt64/bin/gcc -O2 -std=c17 -Wall -Wextra compiler_passes.c -o passes.exe -lm 2>&1; echo 'EXIT:'`$?; test -f passes.exe && echo 'RUNNING:' && ./passes.exe"
& $bash -lc $bashCmd
