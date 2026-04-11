#!/usr/bin/env python3
"""
fix_csave_formula.py — CG-IR-016 csave_base formula correction
==============================================================
The V2 patch (fix_cgir016.py) computed csave_base incorrectly:

  WRONG:  csave_offset = -(stack_size + ir_extra - 8)
  RIGHT:  csave_offset = -(stack_size + 8 * ((int)max_reg + n_alloca + 8))

WHY THE WRONG FORMULA FAILS:
  ir_extra = 8*(max_reg + n_alloca + 8 + n_csave_slots) + alloca_bytes
  Subtracting 8 from ir_extra still includes alloca_bytes, placing the
  save slots inside the alloca variable-data buffer when alloca_bytes > 0.
  Any function with a local array/VLA will corrupt its alloca buffer on
  every call via the IR backend.

WHY THE CORRECT FORMULA WORKS:
  The callee-save slots sit in the EXPLICIT n_csave_slots region, which is
  ABOVE the alloca_bytes buffer.  csave_base = slot_base - 8*(max_reg+n_alloca+8)
  is exactly the start of the n_csave_slots reservation, independent of
  alloca_bytes.

Frame layout (downward from %rbp):
  [stack_size]            AST locals + params
  [8*max_reg]             IR spill slots (vregs r0..r_N)
  [8*n_alloca]            alloca header pointers
  [8*8]                   headroom (alignment reserve)
  [8*n_csave_slots = 40]  callee-save area   ← csave_base is HERE
  [alloca_bytes]          variable alloca buffer data
  [padding]
  %rsp  (16-byte aligned)

Sentinel: CG-IR-016-CSAVE-V2-FMLA — guards against double-application.

Usage:
    python3 fix_csave_formula.py [compiler_passes.c]
    python3 fix_csave_formula.py --check
"""

import sys
import os
import shutil
import datetime

SENTINEL = "CG-IR-016-CSAVE-V2-FMLA"
DEFAULT_SRC = "compiler_passes.c"

# -----------------------------------------------------------------------
# The wrong formula — what we're looking for
# -----------------------------------------------------------------------
OLD_FORMULA = (
    "    csave_offset = -(stack_size + ir_extra - 8);\n"
    "    ctx.csave_base = csave_offset;\n"
)

# -----------------------------------------------------------------------
# The correct formula — what we're replacing it with
# -----------------------------------------------------------------------
NEW_FORMULA = (
    "    /* CG-IR-016-CSAVE-V2-FMLA: Place saves at TOP of n_csave_slots region,\n"
    "     * ABOVE the alloca_bytes buffer.  Using ir_extra here is wrong because\n"
    "     * ir_extra includes alloca_bytes, which would push csave_base into the\n"
    "     * alloca variable-data area when any function has local arrays.\n"
    "     *\n"
    "     * Correct: csave_base = slot_base - 8*(max_reg + n_alloca + 8)\n"
    "     *        = -(stack_size + 8*(max_reg + n_alloca + 8))\n"
    "     * The n_csave_slots reservation in ir_extra guarantees the 5 save slots\n"
    "     * (40 bytes) fit without overlapping alloca headers, spills, or buffers.\n"
    "     * No-overlap proof:\n"
    "     *   r15 (lowest save) = csave_base - 32\n"
    "     *                     = -(stack_size + 8*(max_reg+n_alloca+8) + 32)\n"
    "     *   alloca_buf start  = -(stack_size + 8*(max_reg+n_alloca+8+5))\n"
    "     *                     = csave_base - 40  →  margin = 8 bytes ✓       */\n"
    "    csave_offset = -(stack_size + 8 * ((int)max_reg + n_alloca + 8));\n"
    "    ctx.csave_base = csave_offset;\n"
)


def backup(path):
    ts = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    dst = path + ".bak." + ts
    shutil.copy2(path, dst)
    return dst


def main():
    check_only = "--check" in sys.argv
    path = DEFAULT_SRC
    for arg in sys.argv[1:]:
        if not arg.startswith("-"):
            path = arg

    if not os.path.isfile(path):
        print(f"ERROR: {path} not found")
        sys.exit(1)

    with open(path, "r", encoding="utf-8", errors="replace") as f:
        src = f.read()

    print("fix_csave_formula.py — CG-IR-016 csave_base formula correction")
    print(f"  Source : {path}  ({len(src):,} bytes)")

    # Idempotency check
    if SENTINEL in src:
        print(f"  Status : ALREADY APPLIED (sentinel '{SENTINEL}' found)")
        sys.exit(0)

    # Prerequisite: V2 patch must be applied first
    if "CG-IR-016-CSAVE-V2" not in src:
        print("  ERROR: CG-IR-016-CSAVE-V2 sentinel not found.")
        print("  Run fix_cgir016.py first, then run this script.")
        sys.exit(1)

    if check_only:
        if OLD_FORMULA in src:
            print("  Status: WRONG FORMULA PRESENT — run without --check to fix")
        else:
            print("  WARNING: expected old formula not found — manual inspection needed")
        sys.exit(0)

    if OLD_FORMULA not in src:
        print("  ERROR: expected old formula not found — source may have diverged")
        print("  Expected:  csave_offset = -(stack_size + ir_extra - 8);")
        print("  Inspect compiler_passes.c manually.")
        sys.exit(1)

    bak = backup(path)
    print(f"  Backup : {bak}")

    patched = src.replace(OLD_FORMULA, NEW_FORMULA, 1)

    with open(path, "w", encoding="utf-8") as f:
        f.write(patched)

    print(f"  Patch  : csave_base formula corrected")
    print(f"  Written: {path}  ({len(patched):,} bytes)")
    print()
    print("Verify:")
    print("  grep -n 'csave_offset' compiler_passes.c")
    print("  # Should show: -(stack_size + 8 * ((int)max_reg + n_alloca + 8))")
    print()
    print("Rebuild:")
    print("  gcc -O0 -w -o zcc zcc.c compiler_passes.c -lm")
    print("  ./verify_cgir016.sh --no-apply")


if __name__ == "__main__":
    main()
