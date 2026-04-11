#!/usr/bin/env python3
"""
linesnap.py — Dump repr() of line ranges for writing exact patch targets.

Usage:
    python3 linesnap.py FILE START [END]

Examples:
    python3 linesnap.py compiler_passes.c 3494 3500
    python3 linesnap.py compiler_passes.c 3494        # just that one line

Output is copy-paste ready for patch script old_lines lists.
Zero guessing about tabs vs spaces, trailing whitespace, or line endings.
"""
import sys

def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    filepath = sys.argv[1]
    start = int(sys.argv[2])
    end = int(sys.argv[3]) if len(sys.argv) > 3 else start

    lines = open(filepath, 'r').readlines()
    total = len(lines)

    print(f"# {filepath}  ({total} lines total)")
    print(f"# Lines {start}–{end} (1-indexed):")
    print()
    for i in range(start - 1, min(end, total)):
        print(f"  lines[{i}] = {repr(lines[i])}")
    print()

if __name__ == '__main__':
    main()
