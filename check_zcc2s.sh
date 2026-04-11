#!/usr/bin/env bash
# Check if zcc2.s is from our compiler (not overwritten by objdump/nm/gcc -S etc.)
set -e
cd "$(dirname "$0")"

if [ ! -f zcc2.s ]; then
  echo "No zcc2.s found. Run: ./zcc zcc.c -o zcc2.s"
  exit 1
fi

first=$(head -1 zcc2.s | tr -d '\0\r')
last=$(tail -1 zcc2.s | tr -d '\0\r')
ftype=$(file -b zcc2.s 2>/dev/null || echo "unknown")
lines=$(wc -l < zcc2.s 2>/dev/null || echo "0")

echo "First line: $first"
echo "Last line:  $last"
echo "File type:  $ftype"
echo "Lines:      $lines"

if [[ "$first" != *"ZCC asm begin"* ]]; then
  echo ""
  echo "ERROR: zcc2.s does NOT start with '# ZCC asm begin'."
  echo "So it was NOT produced by './zcc zcc.c -o zcc2.s'."
  echo "Something else wrote this file (e.g. gcc -S, objdump -d, nm)."
  echo "Fix: rm -f zcc2.s && ./zcc zcc.c -o zcc2.s  (and run no other command that writes to zcc2.s)"
  exit 1
fi

if [[ "$last" != *"ZCC asm end"* ]]; then
  echo ""
  echo "ERROR: zcc2.s does NOT end with '# ZCC asm end'."
  echo "The file was truncated or something appended to it after our compiler ran."
  echo "Fix: rm -f zcc2.s && ./zcc zcc.c -o zcc2.s"
  exit 1
fi

if [[ "$ftype" != *"ASCII"* ]] && [[ "$ftype" != *"text"* ]]; then
  echo ""
  echo "WARNING: zcc2.s is not plain text ($ftype). May contain binary."
  exit 1
fi

echo ""
echo "OK: zcc2.s looks like output from our compiler. You can run: gcc -O0 -w -o zcc2 zcc2.s"
