# -*- coding: utf-8 -*-
"""
fix_cgir009.py - Compute exact IR frame depth BEFORE emission.

The CG-IR-008 frame extension used ctx.n_allocas which is 0 at extension
time (set later by ir_asm_assign_alloca_offsets inside ir_asm_emit_function_body).

Fix: scan the IR for max register ID and alloca count/sizes BEFORE emitting
the frame extension, then extend by the exact amount needed.

Run: python3 fix_cgir009.py
"""

import sys
import shutil

F = "compiler_passes.c"
shutil.copy(F, F + ".bak_cgir009")
print("Backup: " + F + ".bak_cgir009")

with open(F, "r") as f:
    c = f.read()

# Replace the CG-IR-008 frame extension block with a pre-scan version
old = """    /* CG-IR-008: extend stack for IR register spills + allocas.
     * Spill slots at slot_base - 8*(r+1), allocas deeper still.
     * Always extend in body_only mode since IR needs its own space. */
    {
        int ir_extra = 8 * ((int)fn->n_regs + ctx.n_allocas + 4);
        ir_extra = (ir_extra + 15) & ~15;
        fprintf(out, "    subq $%d, %%rsp\\n", ir_extra);
    }"""

new = """    /* CG-IR-009: scan IR to compute exact frame depth before emission.
     * Must happen BEFORE ir_asm_emit_function_body since alloca offsets
     * and spill slots all live below slot_base. */
    {
        uint32_t max_reg = fn->n_regs;
        int n_alloca = 0;
        int64_t alloca_bytes = 0;
        for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
            Block *b = fn->blocks[bi];
            if (!b) continue;
            for (Instr *ins = b->head; ins; ins = ins->next) {
                if (ins->dst >= max_reg) max_reg = ins->dst + 1;
                for (uint32_t s = 0; s < ins->n_src && s < MAX_OPERANDS; s++)
                    if (ins->src[s] >= max_reg) max_reg = ins->src[s] + 1;
                if (ins->op == OP_ALLOCA) {
                    n_alloca++;
                    if (ins->imm > 0) alloca_bytes += ins->imm;
                }
            }
        }
        int ir_extra = 8 * ((int)max_reg + n_alloca + 8) + (int)alloca_bytes;
        ir_extra = (ir_extra + 15) & ~15;
        fprintf(out, "    subq $%d, %%rsp\\n", ir_extra);
    }"""

if old in c:
    c = c.replace(old, new, 1)
    print("OK: Replaced CG-IR-008 block with CG-IR-009 pre-scan")
else:
    print("ERROR: CG-IR-008 block not found")
    sys.exit(1)

with open(F, "w") as f:
    f.write(c)

print("Done. Rebuild and test.")
