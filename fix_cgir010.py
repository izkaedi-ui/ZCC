# -*- coding: utf-8 -*-
"""
fix_cgir010.py - Force 8-byte OP_LOAD/OP_STORE

The IR bridge emits imm=4 for some pointer operations, causing:
  OP_LOAD:  movl (%rax),%eax + movslq -> truncates 8-byte pointers
  OP_STORE: movl %ecx,(%rax)          -> only writes lower 4 bytes

GDB proof: rdi=0x2a00007fffffffe8 at strcmp crash.
0x2a (42, test return value) was written as movl to argv's slot,
corrupting the upper bytes of the pointer.

Fix: always use movq. x86-64 registers are 64-bit; 4-byte path
is a premature optimization that silently corrupts pointers.

Run: python3 fix_cgir010.py
"""

import sys
import shutil

F = "compiler_passes.c"
shutil.copy(F, F + ".bak_cgir010")
print("Backup: " + F + ".bak_cgir010")

with open(F, "r") as f:
    c = f.read()

changes = 0

# === Fix OP_LOAD: remove 4-byte path, always movq ===
old_load = """        case OP_LOAD: {
            if (s_debug_main_emit)
                fprintf(stderr, "[PGO-DEBUG] block %u OP_LOAD dst=%u src0=%u\\n", (unsigned)cur_block, ins->dst, ins->src[0]);
            int sz = (ins->imm == 4) ? 4 : 8;
            ir_asm_load_to_rax(ctx, ins->src[0]);
            if (sz == 4) {
                fprintf(f, "    movl (%%rax), %%eax\\n");
                fprintf(f, "    movslq %%eax, %%rax\\n");
            } else {
                fprintf(f, "    movq (%%rax), %%rax\\n");
            }
            ir_asm_store_rax_to(ctx, ins->dst);
            break;
        }"""

new_load = """        case OP_LOAD: {
            if (s_debug_main_emit)
                fprintf(stderr, "[PGO-DEBUG] block %u OP_LOAD dst=%u src0=%u\\n", (unsigned)cur_block, ins->dst, ins->src[0]);
            /* CG-IR-010: always 8-byte load. The 4-byte movl+movslq path
             * truncates pointers (caused argv corruption: rdi=0x2a00007fffffffe8).
             * On x86-64 all values fit in 64-bit regs; ints zero/sign-extend naturally. */
            ir_asm_load_to_rax(ctx, ins->src[0]);
            fprintf(f, "    movq (%%rax), %%rax\\n");
            ir_asm_store_rax_to(ctx, ins->dst);
            break;
        }"""

if old_load in c:
    c = c.replace(old_load, new_load, 1)
    changes += 1
    print("1. OP_LOAD: forced 8-byte movq")
else:
    print("1. SKIP: OP_LOAD pattern not found")

# === Fix OP_STORE: remove 4-byte path, always movq ===
old_store = """        case OP_STORE:
            if (ins->n_src >= 2) {
                int sz = (ins->imm == 4) ? 4 : 8;
                ir_asm_load_to_rax(ctx, ins->src[1]);
                ir_asm_load_to_rcx(ctx, ins->src[0]);
                if (sz == 4)
                    fprintf(f, "    movl %%ecx, (%%rax)\\n");
                else
                    fprintf(f, "    movq %%rcx, (%%rax)\\n");
            }
            break;"""

new_store = """        case OP_STORE:
            if (ins->n_src >= 2) {
                /* CG-IR-010: always 8-byte store. movl corrupts pointer slots. */
                ir_asm_load_to_rax(ctx, ins->src[1]);
                ir_asm_load_to_rcx(ctx, ins->src[0]);
                fprintf(f, "    movq %%rcx, (%%rax)\\n");
            }
            break;"""

if old_store in c:
    c = c.replace(old_store, new_store, 1)
    changes += 1
    print("2. OP_STORE: forced 8-byte movq")
else:
    print("2. SKIP: OP_STORE pattern not found")

with open(F, "w") as f:
    f.write(c)

print("\nApplied %d/2 changes." % changes)
if changes == 2:
    print("Full success. Rebuild and test.")
else:
    print("WARNING: Some patterns not found.")
