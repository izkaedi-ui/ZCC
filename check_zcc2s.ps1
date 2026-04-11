# Check if zcc2.s is from our compiler (not overwritten by objdump/nm/gcc -S etc.)
# PowerShell version - run from project root when WSL D: is not available.

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

if (-not (Test-Path -LiteralPath zcc2.s)) {
  Write-Host "No zcc2.s found. Run: ./zcc zcc.c -o zcc2.s"
  exit 1
}

$lines = Get-Content -LiteralPath zcc2.s -ReadCount 0
$first = ($lines[0] -replace '[\0\r]', '').Trim()
$last  = ($lines[-1] -replace '[\0\r]', '').Trim()
$lineCount = $lines.Count

Write-Host "First line: $first"
Write-Host "Last line:  $last"
Write-Host "Lines:     $lineCount"

if ($first -notmatch 'ZCC asm begin') {
  Write-Host ""
  Write-Host "ERROR: zcc2.s does NOT start with '# ZCC asm begin'."
  Write-Host "So it was NOT produced by './zcc zcc.c -o zcc2.s'."
  exit 1
}

if ($last -notmatch 'ZCC asm end') {
  Write-Host ""
  Write-Host "ERROR: zcc2.s does NOT end with '# ZCC asm end'."
  Write-Host "The file was truncated or something appended to it after our compiler ran."
  exit 1
}

Write-Host ""
Write-Host "OK: zcc2.s looks like output from our compiler."
Write-Host "Assemble in WSL (Linux) only - Windows gcc cannot assemble this:"
Write-Host "  wsl -d kali-linux bash -c ""cd /mnt/d/__DOWNLOADS/selforglinux && gcc -O0 -w -o zcc2 zcc2.s -lm"""
Write-Host "(If Kali cannot see D:, mount it first: wsl -d kali-linux -u root -- mount -t drvfs D: /mnt/d)"
