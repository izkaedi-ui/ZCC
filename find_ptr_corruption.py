#!/usr/bin/env python3
"""
Find where -32(%rbp) is written in whereLoopAddBtree to locate the corrupt assignment.
We know at crash time: *rbp-32 = 0x7fffffffb490 (stack addr, should be heap ptr).
"""
import re

asm_path = "sqlite3_zcc.s"
in_func = False
writes_to_32 = []

with open(asm_path) as f:
    for lineno, line in enumerate(f, 1):
        stripped = line.strip()

        if stripped == "whereLoopAddBtree:":
            in_func = True

        if in_func and stripped == "ret":
            break

        if in_func:
            # Detect writes to -32(%rbp)
            if "-32(%rbp)" in stripped:
                writes_to_32.append((lineno, stripped))

print(f"All writes/reads to -32(%rbp) in whereLoopAddBtree ({len(writes_to_32)} total):")
for lineno, asm in writes_to_32:
    is_write = "movq" in asm and "(%rbp)" not in asm.split(",")[0]
    print(f"  {lineno:7d}: {asm}")

print()

# Also find the block that writes a leaq result to -32(%rbp)
print("Looking for 'leaq ... -32(%rbp)' pattern:")
in_func = False
with open(asm_path) as f:
    prev = ""
    for lineno, line in enumerate(f, 1):
        stripped = line.strip()
        if stripped == "whereLoopAddBtree:":
            in_func = True
        if in_func and stripped == "ret":
            break
        if in_func:
            # Find: movq %r11, -32(%rbp) where r11 came from a leaq (leaq stores an address)
            if "-32(%rbp)" in stripped and stripped.startswith("movq"):
                print(f"  WRITE: line {lineno}: {stripped}")
                print(f"  PREV:  line {lineno-1}: {prev}")
            prev = stripped
