---
name: oneshot-patcher
description: >
  Deterministic, idempotent C source patching that works first try every time.
  Eliminates whitespace-mismatch failures, double-apply corruption, and silent
  patch misses. Uses line-number splicing (not string matching), marker-based
  idempotency detection, auto-backup with rollback, and build-gate verification.

  TRIGGER ON:
  - "patch", "fix", "edit compiler_passes.c", "apply the terminator guard"
  - Any source transformation on ZCC or compiler C files
  - "apply_termfix", "safepatch", "one shot", "line splice"
  - Modifying specific line ranges in large C files
  - Any past failure involving whitespace mismatch or string-match patch miss
  - "CG-IR-005", "CG-IR-006", "CG-IR-007", terminator guard, fall-through fix
  - Resuming a patch session from transcript
  - Any request to edit compiler_passes.c, codegen.c, or part4.c

  ALWAYS use this skill instead of raw str_replace or sed for compiler C files.
  These files have inconsistent indentation that breaks string matching 100% of
  the time. This skill exists because 8+ string-match failures proved it necessary.
---

# Oneshot Patcher — Deterministic C Source Transformation

## Core Principle

**Never match strings. Always splice lines. Always detect before applying.**

This skill was born from 8+ failed string-match patch attempts on `compiler_passes.c`
where invisible whitespace differences (17 vs 18 spaces, trailing tabs, line wraps)
caused `str.count()` to return 0 and patches to silently skip.

## The Protocol (3 steps, zero ambiguity)

### Step 1: RECON — See the exact bytes

Before writing ANY patch, dump the target lines with `repr()`:

```python
# linesnap.py — Run FIRST, before any patch attempt
import sys
f, start, end = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
lines = open(f).readlines()
print(f"--- {f} lines {start}-{end} ({end-start+1} lines) ---")
for i in range(start-1, end):
    print(f"  [{i+1}] {repr(lines[i])}")
```

Usage: `python3 linesnap.py compiler_passes.c 3494 3500`

This shows exact whitespace, line endings, and hidden characters.
**Copy-paste the repr() output into your patch script — never type indentation from memory.**

### Step 2: APPLY — Line-number splice with safety

```python
# safepatch.py — Idempotent patcher with rollback
import sys, shutil, hashlib

def md5(path):
    return hashlib.md5(open(path, 'rb').read()).hexdigest()

def check(filepath, markers):
    """Return (found_count, total_count)."""
    src = open(filepath).read()
    found = sum(1 for m in markers if m in src)
    return found, len(markers)

def apply_patch(filepath, replacements, markers):
    """
    replacements: [(start, end, [new_lines]), ...]
        start/end are 1-indexed, inclusive. new_lines are complete strings with \\n.
    markers: ['unique_sentinel', ...]
        Short unique strings that MUST appear after patch. Used for idempotency.
    
    Returns True on success, False on failure (with auto-rollback).
    """
    # --- Idempotency gate ---
    found, total = check(filepath, markers)
    if found == total:
        print(f"SKIP: {found}/{total} markers already present — patch applied")
        return True
    if found > 0:
        print(f"WARNING: {found}/{total} markers found — partial apply detected")
    
    # --- Backup ---
    bak = f"{filepath}.pre_patch"
    shutil.copy2(filepath, bak)
    print(f"Backup: {bak} ({md5(bak)})")
    
    # --- Splice (bottom-to-top to preserve line numbers) ---
    lines = open(filepath).readlines()
    for start, end, new_lines in sorted(replacements, key=lambda r: r[0], reverse=True):
        print(f"  Splicing lines {start}-{end} ({end-start+1} old -> {len(new_lines)} new)")
        lines[start-1:end] = new_lines
    
    open(filepath, 'w').writelines(lines)
    
    # --- Verify markers landed ---
    found2, total2 = check(filepath, markers)
    if found2 != total2:
        shutil.copy2(bak, filepath)
        print(f"ROLLBACK: {found2}/{total2} markers after patch — restored from backup")
        return False
    
    print(f"APPLIED: {found2}/{total2} markers verified — {md5(filepath)}")
    return True

# --- CLI interface ---
if __name__ == '__main__':
    if '--check' in sys.argv:
        filepath = sys.argv[sys.argv.index('--check') + 1]
        # Override markers in your patch-specific script
        print("Override check() in your patch script with actual markers")
        sys.exit(0)
```

### Step 3: GATE — Build + selfhost verification

```bash
#!/bin/bash
# gatebuild.sh — Atomic verify + build + test
FILE="$1"; shift
MARKERS=("$@")

echo "=== MARKER CHECK ==="
FOUND=0
for m in "${MARKERS[@]}"; do
    if grep -q "$m" "$FILE"; then
        ((FOUND++))
        echo "  ✓ $m"
    else
        echo "  ✗ $m"
    fi
done
echo "$FOUND/${#MARKERS[@]} markers"
[ "$FOUND" -ne "${#MARKERS[@]}" ] && echo "GATE FAIL: missing markers" && exit 1

echo "=== BUILD ==="
make zcc_full || exit 1

echo "=== SELFHOST ==="
timeout 90 make selfhost || exit 1

echo "=== GATE PASS ==="
```

## Writing a Patch Script

Every specific patch gets its own script that imports `safepatch.py`:

```python
#!/usr/bin/env python3
"""CG-IR-005: Terminator guard for ZND_IF lowering."""
from safepatch import apply_patch, check

FILE = 'compiler_passes.c'
MARKERS = ['op==OP_RET||_t->op==OP_BR', 'CG-IR-005 fall-through', 'need_jmp']

# Replacement 1: then-body guard (lines 3496-3500)
fix1_new = [
    '                  { Block *_tb=fn->blocks[ctx->cur_block]; Instr *_tt=_tb?_tb->tail:NULL;\n',
    '                    if(!(_tt&&(_tt->op==OP_RET||_tt->op==OP_BR||_tt->op==OP_CONDBR))){\n',
    '                      Instr *br_then=calloc(1,sizeof(Instr));\n',
    '                      // ... rest of guard\n',
    '                    } }\n',
]

# Replacement 2: else-body guard (lines 3472-3477)  
fix2_new = [ ... ]

# Replacement 3: fall-through fixup (lines 4943-4944)
fix3_new = [ ... ]

REPLACEMENTS = [
    (3496, 3500, fix1_new),  # then guard
    (3472, 3477, fix2_new),  # else guard  
    (4943, 4944, fix3_new),  # fall-through
]

import sys
if '--check' in sys.argv:
    f, t = check(FILE, MARKERS)
    print(f"{f}/{t} {'FULLY APPLIED ✓' if f==t else 'NEEDS APPLY'}")
elif '--apply' in sys.argv:
    apply_patch(FILE, REPLACEMENTS, MARKERS)
else:
    print("Usage: --check | --apply")
```

## Critical Rules

### NEVER DO
- `src.replace(old_string, new_string)` — whitespace will not match
- `sed -i 's/pattern/replacement/'` through PowerShell — `&&` and `||` get intercepted
- Guess indentation from terminal display — terminals wrap long lines
- Apply patches top-to-bottom — earlier splices shift later line numbers
- Emit patches without `repr()` verification of target lines
- Trust `cat -A` output pasted into Python strings — encoding artifacts corrupt

### ALWAYS DO
- `linesnap.py` FIRST to see exact bytes
- Sort replacements bottom-to-top (highest line numbers first)
- Include at least one unique marker string per fix site
- Auto-backup before any write
- Rollback if marker count doesn't match after write
- Gate on `make zcc_full && timeout 90 make selfhost`
- Snapshot after gate passes: `cp file file.bak_CGIR0XX`

### Marker Design
- Markers must be SHORT, UNIQUE, and WHITESPACE-IMMUNE
- Good: `'op==OP_RET'`, `'need_jmp'`, `'CG-IR-005 fall-through'`
- Bad: `'                  Instr *br_then'` (whitespace-dependent)
- Each fix site gets at least one marker
- `--check` reports N/M format: `3/3 FULLY APPLIED ✓`

### Line Number Drift
When applying multiple patches to the same file:
1. Sort all replacements by start line DESCENDING
2. Apply bottom-to-top so earlier splices don't shift targets
3. OR: re-read the file between each splice (simpler but slower)

## Workflow Summary

```
linesnap.py FILE START END          # 1. See exact bytes
# Write patch script with repr() strings
python3 my_patch.py --check         # 2. 0/3? → --apply
python3 my_patch.py --apply         # 3. Splice + verify + rollback
./gatebuild.sh FILE "marker1" ...   # 4. Build + selfhost
cp FILE FILE.bak_CGIR0XX            # 5. Snapshot on pass
```

Zero ambiguity. Zero whitespace failures. One shot every time.

## History

This methodology was forged from two ZCC sessions where 8+ string-match
patch attempts failed on `compiler_passes.c` due to invisible whitespace
differences (17 vs 18 spaces). The `apply_termfix.py` tool that emerged
from those failures became the template for all subsequent CG-IR fixes.
Coccinelle (spatch) solves the same class of problem via AST-level semantic
patches but is heavier. For single-file compilers where you control every
line number, this approach is tighter and more deterministic.
