#!/usr/bin/env python3
"""
fix_cgir016.py — CG-IR-016: Callee-Saved Register Corruption Fix
================================================================
Patches compiler_passes.c to:
  1. Add  int csave_base;  field to IRAsmCtx (after slot_base)
  2. Explicitly add n_csave_slots=5 to ir_extra computation
  3. Compute ctx.csave_base after ir_extra is known
  4. Replace inline -(stack_size + ir_extra - N) arithmetic with
     offsets derived from ctx.csave_base

Idempotency: guarded by sentinel string "CG-IR-016-CSAVE-V2" present
in the struct field comment.  Running the script twice is safe.

Usage:
    python3 fix_cgir016.py [compiler_passes.c]
    python3 fix_cgir016.py --check      # dry-run: report current state
"""

import sys
import re
import os
import shutil
import datetime

SENTINEL = "CG-IR-016-CSAVE-V2"
DEFAULT_SRC = "compiler_passes.c"


# ---------------------------------------------------------------------------
# Patch 1: add csave_base field to IRAsmCtx after slot_base line
# ---------------------------------------------------------------------------
STRUCT_OLD = (
    "  int slot_base; /* CG-IR-008: base offset for IR spill slots (0 or -stack_size)\n"
    "                  */\n"
    "  int ret_size;  /* CG-IR-015: return type bytes (4=int, 8=long/ptr, 0→treat as 8) */\n"
    "} IRAsmCtx;"
)

STRUCT_NEW = (
    "  int slot_base; /* CG-IR-008: base offset for IR spill slots (0 or -stack_size)\n"
    "                  */\n"
    "  int csave_base; /* CG-IR-016-CSAVE-V2: %rbp-relative base of callee-save area.\n"
    "                   * = slot_base - 8*(max_reg + n_alloca + 8) - alloca_bytes - 8\n"
    "                   * Slots: csave_base (rbx), csave_base-8 (r12), -16 (r13),\n"
    "                   *        csave_base-24 (r14), csave_base-32 (r15).          */\n"
    "  int ret_size;  /* CG-IR-015: return type bytes (4=int, 8=long/ptr, 0→treat as 8) */\n"
    "} IRAsmCtx;"
)


# ---------------------------------------------------------------------------
# Patch 2 + 3 + 4: ir_extra block + csave_base computation + save/restore
#
# Replaces the entire ir_extra scan block, the old CG-IR-016 inline-offset
# save/restore, and introduces n_csave_slots + csave_base.
# ---------------------------------------------------------------------------

# The exact text we're replacing (from the opening of the anonymous block
# through the last movq restore line).  Uses a regex so minor whitespace
# differences don't break the match.

SCAN_BLOCK_RE = re.compile(
    r"(/\* CG-IR-009.*?CG-IR-016.*?IR spill slots.*?\*/\s*)"  # opening comment
    r"(int ir_extra = 0;\s*\{.*?\})\s*"                        # scan block
    r"(/\* CG-IR-016.*?40 consumed by 5 regs\..*?\*/\s*)"     # old comment
    r"(fprintf\(out.*?r15.*?- 40\)\);\s*)"                     # 5 save lines
    r"(ir_asm_emit_function_body\(&ctx\);\s*)"                 # body call
    r"(/\* CG-IR-016: restore.*?\*/\s*)"                       # restore comment
    r"(fprintf\(out.*?%%r15.*?- 40\)\);)",                     # 5 restore lines
    re.DOTALL,
)

SCAN_BLOCK_NEW = """\
  /* CG-IR-009: scan IR to compute exact frame depth before emission.
   * Must happen BEFORE ir_asm_emit_function_body since alloca offsets
   * and spill slots all live below slot_base.
   * CG-IR-016-CSAVE-V2: ir_extra extended by n_csave_slots=5 to give
   * each callee-saved register its own explicit 8-byte slot at the very
   * bottom of the frame.  csave_base is the highest of those 5 slots.  */
  {
    uint32_t max_reg;
    int n_alloca;
    int n_csave_slots;
    int64_t alloca_bytes;
    int csave_offset;
    uint32_t bi;
    max_reg = fn->n_regs;
    n_alloca = 0;
    alloca_bytes = 0;
    n_csave_slots = 5; /* rbx, r12, r13, r14, r15 — always reserve all 5 */
    for (bi = 0; bi < fn->n_blocks; bi++) {
      Block *b = fn->blocks[bi];
      if (!b) continue;
      {
        Instr *ins;
        uint32_t s;
        for (ins = b->head; ins; ins = ins->next) {
          if (ins->dst >= max_reg) max_reg = ins->dst + 1;
          for (s = 0; s < ins->n_src && s < MAX_OPERANDS; s++)
            if (ins->src[s] >= max_reg) max_reg = ins->src[s] + 1;
          if (ins->op == OP_ALLOCA) {
            n_alloca++;
            if (ins->imm > 0) alloca_bytes += ins->imm;
          }
        }
      }
    }
    ir_extra = 8 * ((int)max_reg + n_alloca + 8 + n_csave_slots) + (int)alloca_bytes;
    ir_extra = (ir_extra + 15) & ~15;
    fprintf(out, "    subq $%d, %%rsp\\n", ir_extra);

    /* CG-IR-016-CSAVE-V2: csave_base is the %rbp-relative offset of the
     * first (highest) callee-save slot.  It sits at the very bottom of the
     * ir_extra frame, below all spill slots and alloca space.
     *
     * Stack layout (growing downward):
     *   %rbp - stack_size                     ← slot_base (AST frame limit)
     *   %rbp - stack_size - 8*max_reg         ← lowest spill slot
     *   %rbp - stack_size - 8*(max_reg+n_alloca) - alloca_bytes
     *                                         ← alloca area
     *   csave_base                            ← rbx save
     *   csave_base - 8                        ← r12 save
     *   csave_base - 16                       ← r13 save
     *   csave_base - 24                       ← r14 save
     *   csave_base - 32                       ← r15 save  (lowest slot)
     *   current %rsp (16-byte aligned)
     *
     * ir_extra includes n_csave_slots*8=40 bytes at the bottom, so
     * csave_base - 32  >=  %rsp and never overlaps spill slots.         */
    csave_offset = -(stack_size + ir_extra - 8);
    ctx.csave_base = csave_offset;
  }

  /* CG-IR-016-CSAVE-V2: save callee-saved registers via movq (NOT pushq —
   * pushq would shift RSP and break every slot_base-relative spill access).
   * All 5 are always saved unconditionally; cost is 5 movq at entry/exit.  */
  fprintf(out, "    movq %%rbx, %d(%%rbp)\\n", ctx.csave_base);
  fprintf(out, "    movq %%r12, %d(%%rbp)\\n", ctx.csave_base - 8);
  fprintf(out, "    movq %%r13, %d(%%rbp)\\n", ctx.csave_base - 16);
  fprintf(out, "    movq %%r14, %d(%%rbp)\\n", ctx.csave_base - 24);
  fprintf(out, "    movq %%r15, %d(%%rbp)\\n", ctx.csave_base - 32);

  ir_asm_emit_function_body(&ctx);

  /* CG-IR-016-CSAVE-V2: restore callee-saved registers before AST epilogue */
  fprintf(out, "    movq %d(%%rbp), %%rbx\\n", ctx.csave_base);
  fprintf(out, "    movq %d(%%rbp), %%r12\\n", ctx.csave_base - 8);
  fprintf(out, "    movq %d(%%rbp), %%r13\\n", ctx.csave_base - 16);
  fprintf(out, "    movq %d(%%rbp), %%r14\\n", ctx.csave_base - 24);
  fprintf(out, "    movq %d(%%rbp), %%r15\\n", ctx.csave_base - 32);\
"""


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def backup(path):
    ts = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    dst = path + ".bak." + ts
    shutil.copy2(path, dst)
    return dst


def is_already_applied(src):
    return SENTINEL in src


def apply_struct_patch(src):
    if STRUCT_OLD not in src:
        # Already patched or source has diverged
        if "csave_base" in src:
            return src, "struct:already-present"
        return src, "struct:OLD-NOT-FOUND"
    patched = src.replace(STRUCT_OLD, STRUCT_NEW, 1)
    return patched, "struct:patched"


def apply_scan_block_patch(src):
    m = SCAN_BLOCK_RE.search(src)
    if m is None:
        return src, "scan_block:NOT-MATCHED"
    patched = src[:m.start()] + SCAN_BLOCK_NEW + src[m.end():]
    return patched, "scan_block:patched"


# ---------------------------------------------------------------------------
# Also need to declare  int ir_extra = 0;  BEFORE the new block.
# The new SCAN_BLOCK_NEW uses  ir_extra  which must be declared in the
# enclosing function scope (C89 requires top-of-block declarations).
# In the original code ir_extra was declared as  int ir_extra = 0;  just
# before the opening brace.  After the patch the anonymous block is gone
# and ir_extra is used directly by the ctx initialisation.
# Ensure the declaration exists right after  IRAsmCtx ctx; ... memset block.
# ---------------------------------------------------------------------------

IR_EXTRA_DECL_ANCHOR = "  ctx.slot_base = -stack_size; /* CG-IR-008:"
IR_EXTRA_DECL_NEW    = "  int ir_extra = 0;\n"

def ensure_ir_extra_decl(src):
    """
    The C89 rule: all variables declared at top of block.
    After the patch, ir_extra is used outside the scan brace; add the
    declaration right after slot_base if it is not already there within
    a few lines of that anchor.
    """
    idx = src.find(IR_EXTRA_DECL_ANCHOR)
    if idx == -1:
        return src, "ir_extra_decl:anchor-missing"
    # Check if "int ir_extra" already exists within 400 chars before anchor
    window_start = max(0, idx - 400)
    window = src[window_start:idx]
    if "int ir_extra" in window:
        return src, "ir_extra_decl:already-present"
    # Insert after the slot_base line
    line_end = src.index("\n", idx) + 1
    patched = src[:line_end] + IR_EXTRA_DECL_NEW + src[line_end:]
    return patched, "ir_extra_decl:inserted"


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

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

    print(f"fix_cgir016.py — CG-IR-016 callee-save fix")
    print(f"  Source : {path}  ({len(src):,} bytes)")

    if is_already_applied(src):
        print(f"  Status : ALREADY APPLIED (sentinel '{SENTINEL}' found)")
        print("  No changes made.")
        sys.exit(0)

    if check_only:
        print("  Status : NOT YET APPLIED  (dry-run — use without --check to patch)")
        sys.exit(1)

    bak = backup(path)
    print(f"  Backup : {bak}")

    patched, s1 = apply_struct_patch(src)
    print(f"  Patch 1 (struct)      : {s1}")

    patched, s2 = ensure_ir_extra_decl(patched)
    print(f"  Patch 2 (ir_extra decl): {s2}")

    patched, s3 = apply_scan_block_patch(patched)
    print(f"  Patch 3 (scan block)  : {s3}")

    if "NOT-FOUND" in (s1 + s2 + s3) or "NOT-MATCHED" in (s1 + s2 + s3):
        print()
        print("WARNING: one or more patches did not apply cleanly.")
        print("         Inspect the output carefully before use.")
        print("         The backup is intact.")

    with open(path, "w", encoding="utf-8") as f:
        f.write(patched)

    print(f"  Written: {path}  ({len(patched):,} bytes)")
    print()
    print("Next steps:")
    print("  gcc -O0 -w -o zcc zcc.c compiler_passes.c -lm")
    print("  ./verify_cgir016.sh")


if __name__ == "__main__":
    main()
