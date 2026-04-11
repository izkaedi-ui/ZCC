#!/usr/bin/env sh
# Build compiler_passes.c from the directory where this script lives.
set -e
cd "$(dirname "$0")"
gcc -O2 -std=c17 -Wall -Wextra compiler_passes.c -o passes -lm
# On Windows, gcc may create "passes" with no extension; rename for execution.
if [ -f passes ] && [ ! -f passes.exe ]; then
    mv passes passes.exe 2>/dev/null || true
fi
echo "Built: ./passes or ./passes.exe"
