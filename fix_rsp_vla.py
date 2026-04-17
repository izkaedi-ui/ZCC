#!/usr/bin/env python3
"""Post-process ZCC assembly to fix VLA RSP save/restore corruption.

Bug: ZCC emits 'movq %rsp, -N(%rbp)' to save RSP before VLA-like code,
then restores with 'movq -N(%rbp), %rsp'. But in between, other code
may overwrite that stack slot via 'movslq' (which sign-extends to 32-bit),
corrupting the 64-bit RSP value.

Fix: Remove all RSP save/restore pairs that aren't part of function
prologue/epilogue (i.e., keep 'movq %rbp, %rsp' and 'movq %rsp, %rbp').
These VLA RSP pairs are unnecessary when arrays are fixed-size.
"""
import re, sys

def fix_asm(filename):
    with open(filename, 'r') as f:
        lines = f.readlines()
    
    out = []
    skip_count = 0
    for i, line in enumerate(lines):
        stripped = line.strip()
        
        # Skip VLA RSP saves: "movq %rsp, -N(%rbp)" (NOT "movq %rsp, %rbp")
        if re.match(r'movq %rsp, -\d+\(%rbp\)', stripped):
            skip_count += 1
            out.append('    # [VLA-FIX] removed RSP save: ' + stripped + '\n')
            continue
        
        # Skip VLA RSP restores: "movq -N(%rbp), %rsp" (NOT "movq %rbp, %rsp")
        if re.match(r'movq -\d+\(%rbp\), %rsp', stripped):
            skip_count += 1
            out.append('    # [VLA-FIX] removed RSP restore: ' + stripped + '\n')
            continue
        
        out.append(line)
    
    with open(filename, 'w') as f:
        f.writelines(out)
    
    print(f"  {filename}: patched {skip_count} VLA RSP save/restore instructions")

for fn in sys.argv[1:]:
    fix_asm(fn)
