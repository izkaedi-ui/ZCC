#!/usr/bin/env python3
"""
fix_cgir016_ret.py — CG-IR-016-RET: Emit callee-save restores inside OP_RET
============================================================================
ROOT CAUSE OF rc=139 SEGFAULT:

  In body_only mode, OP_RET emits:
      (no restores)
      jmp .Lfunc_end_N

  The restores in zcc_run_passes_emit_body_pgo are placed AFTER
  ir_asm_emit_function_body().  Every function with an explicit return
  statement jumps over them.  The callee-saved registers are NEVER
  restored on the normal (jump) return path.

FIX:
  Add an `else if (ctx->body_only && ctx->csave_base)` branch inside
  OP_RET that emits movq csave_base(%rbp), %reg for all 5 callee-saved
  regs BEFORE the jmp.  The "after-body" restores in
  zcc_run_passes_emit_body_pgo remain as a fallback for fall-through
  functions (rare), but the OP_RET path now correctly restores regs on
  ALL return paths.

Sentinel: CG-IR-016-RET — idempotency guard.

Usage:
    python3 fix_cgir016_ret.py [compiler_passes.c]
    python3 fix_cgir016_ret.py --check
"""

import sys, os, shutil, datetime

SENTINEL = "CG-IR-016-RET"
DEFAULT_SRC = "compiler_passes.c"

# ---------------------------------------------------------------------------
# Exact string to replace in OP_RET handler
# Must match the current patched source exactly.
# ---------------------------------------------------------------------------
OLD_RET = (
    "    if (!ctx->body_only) {\n"
    "      for (int i = CALLEE_SAVED_PUSH_N - 1; i >= 0; i--) {\n"
    "        int p = callee_saved_push_order[i];\n"
    "        if (ctx->used_callee_saved_mask & (1 << p))\n"
    "          fprintf(f, \"    popq %%%s\\n\", phys_reg_name[p]);\n"
    "      }\n"
    "    }\n"
    "    fprintf(f, \"    jmp .Lfunc_end_%d\\n\", ctx->func_end_label);\n"
)

NEW_RET = (
    "    if (!ctx->body_only) {\n"
    "      for (int i = CALLEE_SAVED_PUSH_N - 1; i >= 0; i--) {\n"
    "        int p = callee_saved_push_order[i];\n"
    "        if (ctx->used_callee_saved_mask & (1 << p))\n"
    "          fprintf(f, \"    popq %%%s\\n\", phys_reg_name[p]);\n"
    "      }\n"
    "    } else if (ctx->csave_base) {\n"
    "      /* CG-IR-016-RET: body_only path — restore callee-saved registers\n"
    "       * BEFORE the jmp so they execute on every return path, not just\n"
    "       * fall-through.  The 'after-body' restores in\n"
    "       * zcc_run_passes_emit_body_pgo are only reached by fall-through\n"
    "       * (functions with no explicit return) — dead code for most funcs. */\n"
    "      fprintf(f, \"    movq %d(%%rbp), %%rbx\\n\", ctx->csave_base);\n"
    "      fprintf(f, \"    movq %d(%%rbp), %%r12\\n\", ctx->csave_base - 8);\n"
    "      fprintf(f, \"    movq %d(%%rbp), %%r13\\n\", ctx->csave_base - 16);\n"
    "      fprintf(f, \"    movq %d(%%rbp), %%r14\\n\", ctx->csave_base - 24);\n"
    "      fprintf(f, \"    movq %d(%%rbp), %%r15\\n\", ctx->csave_base - 32);\n"
    "    }\n"
    "    fprintf(f, \"    jmp .Lfunc_end_%d\\n\", ctx->func_end_label);\n"
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

    print("fix_cgir016_ret.py — CG-IR-016-RET: OP_RET body_only restore fix")
    print(f"  Source : {path}  ({len(src):,} bytes)")

    if SENTINEL in src:
        print(f"  Status : ALREADY APPLIED (sentinel '{SENTINEL}' found)")
        sys.exit(0)

    # Prerequisites
    for prereq in ["CG-IR-016-CSAVE-V2", "CG-IR-016-CSAVE-V2-FMLA"]:
        if prereq not in src:
            print(f"  ERROR: prerequisite '{prereq}' not found — run earlier fixes first")
            sys.exit(1)

    if check_only:
        if OLD_RET in src:
            print("  Status: NOT APPLIED — run without --check to fix")
        else:
            print("  WARNING: expected OP_RET pattern not found — source may have diverged")
            # Show what the OP_RET area actually looks like
            import re
            m = re.search(r'case OP_RET:.*?break;', src, re.DOTALL)
            if m:
                print("  Current OP_RET handler:")
                print(m.group(0)[:500])
        sys.exit(0)

    if OLD_RET not in src:
        print("  ERROR: expected OP_RET pattern not found in source")
        print("  The OP_RET handler may have been modified since this script was written.")
        print("  Manual patch required:")
        print()
        print("  In ir_asm_lower_insn, OP_RET case, BEFORE the jmp emission:")
        print("  Add:")
        print("    } else if (ctx->csave_base) {")
        print('      fprintf(f, "    movq %d(%%rbp), %%rbx\\n", ctx->csave_base);')
        print('      fprintf(f, "    movq %d(%%rbp), %%r12\\n", ctx->csave_base - 8);')
        print('      fprintf(f, "    movq %d(%%rbp), %%r13\\n", ctx->csave_base - 16);')
        print('      fprintf(f, "    movq %d(%%rbp), %%r14\\n", ctx->csave_base - 24);')
        print('      fprintf(f, "    movq %d(%%rbp), %%r15\\n", ctx->csave_base - 32);')
        print("    }")
        sys.exit(1)

    bak = backup(path)
    print(f"  Backup : {bak}")

    patched = src.replace(OLD_RET, NEW_RET, 1)

    with open(path, "w", encoding="utf-8") as f:
        f.write(patched)

    print(f"  Patch  : OP_RET body_only restore block added")
    print(f"  Written: {path}  ({len(patched):,} bytes)")
    print()
    print("Verify the patch:")
    print(f"  grep -A 20 'CG-IR-016-RET' {path}")
    print()
    print("Rebuild and test:")
    print("  gcc -O0 -w -o zcc zcc.c compiler_passes.c -lm")
    print("  ./verify_cgir016.sh --no-apply")


if __name__ == "__main__":
    main()
