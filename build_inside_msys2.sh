#!/usr/bin/env bash
# Run from PowerShell: & "C:\msys64\usr\bin\bash.exe" -lc "D:/__DOWNLOADS/selforglinux/build_inside_msys2.sh"
cd /d/__DOWNLOADS/selforglinux || exit 1
/ucrt64/bin/gcc -O2 -std=c17 -Wall -Wextra compiler_passes.c -o passes.exe -lm
echo "gcc exit: $?"
if [ -f passes.exe ]; then
  echo "Running passes.exe..."
  ./passes.exe
else
  echo "passes.exe not created"
fi
