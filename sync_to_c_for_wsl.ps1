# Sync project to C: so Kali (WSL) can see it. Run from project root in PowerShell.
# Then in Kali: cd /mnt/c/Users/zkaedi/selforglinux && ./run_selfhost.sh
# (You need zcc and zcc2.s already built, or run full bootstrap there.)

$ErrorActionPreference = "Stop"
$dest = "C:\Users\zkaedi\selforglinux"
$root = $PSScriptRoot

New-Item -ItemType Directory -Force -Path $dest | Out-Null
$copy = @(
  "part1.c", "part2.c", "part3.c", "part4.c", "part5.c",
  "run_selfhost.sh", "check_zcc2s.sh"
)
foreach ($f in $copy) {
  $src = Join-Path $root $f
  if (Test-Path -LiteralPath $src) {
    Copy-Item -LiteralPath $src -Destination (Join-Path $dest $f) -Force
    Write-Host "  $f"
  }
}
Write-Host "Synced to $dest"
Write-Host "In Kali run:  cd /mnt/c/Users/zkaedi/selforglinux && chmod +x run_selfhost.sh && ./run_selfhost.sh"
Write-Host "run_selfhost.sh will build zcc, zcc2, then zcc3 and cmp zcc2.s zcc3.s."
