#!/usr/bin/env python3
"""
safepatch.py — Idempotent line-range patcher with marker detection and auto-rollback.

USAGE AS LIBRARY:
    from safepatch import patch
    patch(
        filepath='compiler_passes.c',
        replacements=[
            (3496, 3500, ['   new line 1\\n', '   new line 2\\n']),
            (3465, 3469, ['   other new line\\n']),
        ],
        markers=['sentinel_after_fix_1', 'sentinel_after_fix_2'],
    )

DESIGN:
  - markers: idempotency check  → skip if all present
  - replacements: applied BOTTOM-TO-TOP automatically (sorted by start desc)
    so earlier splices never shift the line numbers of later ones
  - backup created before any write at {filepath}.pre_patch
  - if marker verification fails after write → auto-rollback from backup
  - exits 0 on success (or already-applied), 1 on failure
"""

import sys
import shutil
import hashlib
import os


def md5(path):
    return hashlib.md5(open(path, 'rb').read()).hexdigest()


def patch(filepath, replacements, markers, label="patch"):
    """
    Apply line-range replacements to filepath, guarded by marker idempotency.

    Args:
        filepath:     path to the file to patch
        replacements: list of (start_1indexed, end_1indexed_inclusive, [new_lines])
                      Applied bottom-to-top automatically.
        markers:      list of sentinel strings that exist ONLY after the fix.
                      If all found → already applied, skip.
                      If none found after applying → rollback.
        label:        human-readable name for logging

    Returns:
        True  — fix was applied (or was already applied)
        False — fix failed and was rolled back
    """
    src = open(filepath, 'r').read()

    # --- Idempotency check ---
    found = [m for m in markers if m in src]
    if len(found) == len(markers):
        print(f"[{label}] {len(found)}/{len(markers)} markers present — already applied, skipping")
        return True
    if found:
        print(f"[{label}] WARNING: {len(found)}/{len(markers)} markers found (partial apply).")
        print(f"  Present:  {found}")
        print(f"  Missing:  {[m for m in markers if m not in src]}")
        print(f"  Inspect {filepath} manually before retrying.")
        return False

    # --- Backup ---
    bak = filepath + ".pre_patch"
    shutil.copy2(filepath, bak)
    bak_hash = md5(bak)
    print(f"[{label}] Backup: {bak}  ({bak_hash})")

    # --- Apply replacements (bottom-to-top) ---
    lines = open(filepath, 'r').readlines()
    total_before = len(lines)

    for start, end, new_lines in sorted(replacements, key=lambda x: -x[0]):
        s = start - 1      # convert to 0-indexed
        e = end            # end is exclusive in slice: lines[s:e] replaces lines s..end-1
        replaced_count = e - s
        lines[s:e] = new_lines
        delta = len(new_lines) - replaced_count
        print(f"[{label}]   Splice L{start}–{end} ({replaced_count} lines → {len(new_lines)}, delta {delta:+d})")

    open(filepath, 'w').writelines(lines)
    print(f"[{label}] Written: {len(lines)} lines (was {total_before})")

    # --- Verify markers ---
    src2 = open(filepath, 'r').read()
    found2 = [m for m in markers if m in src2]
    if len(found2) != len(markers):
        missing = [m for m in markers if m not in src2]
        print(f"[{label}] ROLLBACK — only {len(found2)}/{len(markers)} markers after patch")
        print(f"  Missing: {missing}")
        shutil.copy2(bak, filepath)
        print(f"[{label}] Restored from {bak}")
        return False

    final_hash = md5(filepath)
    print(f"[{label}] {len(found2)}/{len(markers)} markers verified ✓  ({final_hash})")
    return True


def check(filepath, markers, label="check"):
    """Check how many markers are present. Returns (found, total)."""
    src = open(filepath, 'r').read()
    found = sum(1 for m in markers if m in src)
    total = len(markers)
    status = "FULLY APPLIED ✓" if found == total else (
        "NOT APPLIED" if found == 0 else "PARTIAL")
    print(f"[{label}] {found}/{total} markers — {status}")
    for m in markers:
        tick = "✓" if m in src else "✗"
        print(f"  {tick} {m!r}")
    return found, total


# ---------------------------------------------------------------------------
# CLI usage: python3 safepatch.py --check FILE SENTINEL [SENTINEL...]
# ---------------------------------------------------------------------------
if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)

    mode = sys.argv[1]
    filepath = sys.argv[2]
    markers = sys.argv[3:]

    if not os.path.exists(filepath):
        print(f"Error: {filepath} not found")
        sys.exit(1)

    if mode == '--check':
        found, total = check(filepath, markers)
        sys.exit(0 if found == total else 1)
    else:
        print("CLI only supports --check. Use as library for --apply.")
        print("See docstring for usage.")
        sys.exit(1)
