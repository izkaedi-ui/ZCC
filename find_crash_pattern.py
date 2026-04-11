#!/usr/bin/env python3
"""
Locate the exact assembly lines around the crash site in whereLoopAddBtree.
The crash is: movswq (%rax),%rax where rax is garbage because
a prior `movq $0,%rax` zeroed the index register instead of loading it.

Pattern to find:
    pushq %rax
    movq $0, %rax     ← THE BUG
    movq %rax, %r11
    popq %rax
    imulq $2, %r11
    addq %r11, %rax
    movswq (%rax), %rax   ← CRASHES HERE
"""
import re, sys

asm_path = "sqlite3_zcc.s"
in_func = False
window = []
WINDOW = 30
found = 0

with open(asm_path) as f:
    for lineno, line in enumerate(f, 1):
        stripped = line.strip()

        if stripped == "whereLoopAddBtree:":
            in_func = True
            window = []

        if in_func:
            window.append((lineno, stripped))
            if len(window) > WINDOW:
                window.pop(0)

            # Check for the crash pattern:
            # last 7 lines should match the push/zero/mov/pop/imul/add/movswq sequence
            if len(window) >= 7:
                tail = [w[1] for w in window[-7:]]
                if (tail[0].startswith("pushq %rax") and
                    tail[1].startswith("movq $0, %rax") and
                    tail[2].startswith("movq %rax, %r11") and
                    tail[3].startswith("popq %rax") and
                    "imulq" in tail[4] and "%r11" in tail[4] and
                    tail[5].startswith("addq %r11, %rax") and
                    tail[6].startswith("movswq")):
                    found += 1
                    print(f"\n=== CRASH PATTERN #{found} ===")
                    # Print context: 10 lines before the window + the 7
                    print(f"  (showing lines {window[-7][0]} to {window[-1][0]})")
                    for ln, ls in window[-10:]:
                        marker = " ←CRASH" if ln == window[-1][0] else ""
                        marker2 = " ←BUG:zero" if ln == window[-8][0] else ""
                        print(f"  {ln:7d}: {ls}{marker}{marker2}")

            # End of function
            if stripped.startswith("ret") and in_func and len(window) > 50:
                # Check if next line starts a new function
                pass
            if stripped.endswith(":") and ":whereLoopAdd" not in stripped and in_func and len(window) > 100:
                # could be a new label, not end
                pass

if found == 0:
    print("Pattern not found (may be different register). Scanning for movswq in whereLoopAddBtree:")
    in_func = False
    with open(asm_path) as f:
        for lineno, line in enumerate(f, 1):
            stripped = line.strip()
            if stripped == "whereLoopAddBtree:":
                in_func = True
            if in_func and stripped.startswith("movswq"):
                print(f"  {lineno}: {stripped}")
            if in_func and stripped == "ret" and lineno > 470000:
                break
else:
    print(f"\nFound {found} crash site(s).")
