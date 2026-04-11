# Build zcc2 in WSL (Kali) when D: is not visible there.
# Copies zcc2.s to C:\, builds in Kali, copies zcc2 back to current dir.

$ErrorActionPreference = "Stop"
$projectRoot = $PSScriptRoot
$wslDir = "/mnt/c/Users/zkaedi/zcc2_build"

Write-Host "Copying zcc2.s to C:\Users\zkaedi\zcc2_build\ for WSL..."
New-Item -ItemType Directory -Force -Path "C:\Users\zkaedi\zcc2_build" | Out-Null
Copy-Item -LiteralPath "$projectRoot\zcc2.s" -Destination "C:\Users\zkaedi\zcc2_build\zcc2.s" -Force

Write-Host "Building in Kali..."
wsl -d kali-linux bash -c "cd $wslDir && gcc -O0 -w -o zcc2 zcc2.s -lm"
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Copying zcc2 back to project..."
Copy-Item -LiteralPath "C:\Users\zkaedi\zcc2_build\zcc2" -Destination "$projectRoot\zcc2" -Force

Write-Host "Done. zcc2 (Linux binary) is at: $projectRoot\zcc2"
Write-Host "Run it in WSL: wsl -d kali-linux bash -c `"cd $wslDir && ./zcc2 --help`""
