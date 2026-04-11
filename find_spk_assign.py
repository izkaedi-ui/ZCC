#!/usr/bin/env python3
"""
Find the assignment 'sPk.aiRowLogEst = aiRowEstPk' in the assembly.
This should be:
  leaq -???(%rbp), %rax   (load aiRowEstPk address)
  ...
  addq $16, %rax           (offset to aiRowLogEst field in Index)
  movq %r11, (%rax)        (store)

OR equivalently:
  leaq aiRowEstPk_offset(%rbp), %rax
  movq %rax, %r11
  leaq sPk_base(%rbp), %rax
  addq $16, %rax
  movq %r11, (%rax)        ← the write to offset 16 of sPk
"""

asm_path = "sqlite3_zcc.s"
in_func = False
window = []
WINDOW = 15
found = []

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

            if stripped == "ret" and lineno > 470000:
                break

            # Look for the sPk.aiRowLogEst = aiRowEstPk pattern
            # Pattern: leaq N(%rbp), %rax; movq %rax, %r11; leaq M(%rbp), %rax; addq $16, %rax; movq %r11, (%rax)
            if "addq $16, %rax" in stripped and len(window) >= 5:
                # Check if next instruction is movq %r11, (%rax) or movq (something), %rax
                next_lines = [w[1] for w in window[-5:]]
                if any("leaq" in l and "%rbp" in l for l in next_lines[:-2]):
                    found.append((lineno, list(window[-8:])))

print(f"Found {len(found)} addq $16 after leaq(%rbp) patterns:")
for lineno, ctx in found[:10]:
    print(f"\n== Line {lineno} ==")
    for ln, ls in ctx:
        print(f"  {ln}: {ls}")

# Also: directly scan for 'aiRowLogEst' comment-like patterns
# Look for addq $16 followed by a store, specifically for sPk write
print("\n=== Direct scan: addq $16 + movq %r11, (%rax) sequences ===")
in_func = False
prev_lines = []
with open(asm_path) as f:
    for lineno, line in enumerate(f, 1):
        stripped = line.strip()
        if stripped == "whereLoopAddBtree:":
            in_func = True
            prev_lines = []
        if in_func and stripped == "ret" and lineno > 470000:
            break
        if in_func:
            prev_lines.append((lineno, stripped))
            if len(prev_lines) > 6:
                prev_lines.pop(0)
            # addq $16 + movq r11, (rax) within 3 lines
            if "addq $16, %rax" in stripped:
                for i, (ln2, s2) in enumerate(prev_lines[-4:]):
                    if "movq %r11, (%rax)" in s2 and i > 0:
                        print(f"\nLine {stripped} at {lineno}:")
                        for ln, ls in prev_lines:
                            print(f"  {ln}: {ls}")
