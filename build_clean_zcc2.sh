#!/usr/bin/env bash
# Produce a clean zcc2.s from the GCC-built zcc only. Run this when zcc2.s is corrupted.
set -e
cd "$(dirname "$0")"

echo "Concat parts -> zcc.c"
cat part1.c part2.c part3.c part4.c part5.c > zcc.c

echo "Build stage1 (GCC): zcc"
gcc -O0 -w -o zcc zcc.c

echo "Remove old zcc2.s so it cannot be from zcc2 or gcc -S"
rm -f zcc2.s

echo "Generate zcc2.s using ONLY ./zcc (GCC-built compiler)"
./zcc zcc.c -o zcc2.s

echo "Verify zcc2.s is from our compiler (not overwritten by zcc2!)..."
first=$(head -1 zcc2.s | tr -d '\0\r')
last=$(tail -1 zcc2.s | tr -d '\0\r')
if [[ "$first" != *"ZCC asm begin"* ]] || [[ "$last" != *"ZCC asm end"* ]]; then
  echo "ERROR: zcc2.s missing ZCC markers (or contains binary)."
  echo "First: $first"
  echo "Last:  $last"
  echo "NEVER run: ./zcc2 zcc.c -o zcc2.s   (see NEVER_ZCC2_TO_ZCC2S.txt)"
  echo "Only run: ./zcc zcc.c -o zcc2.s     (GCC-built zcc)"
  exit 1
fi
echo "First line: $first"
echo "Last line: $last"
echo "Line count: $(wc -l < zcc2.s)"

echo "Assemble and link zcc2"
gcc -O0 -w -o zcc2 zcc2.s

echo "Done. zcc2 is ready. Use: ./zcc2 zcc.c -o zcc3.s  (never -o zcc2.s)"
