# -*- coding: utf-8 -*-
"""
fix_cgir008.py - CG-IR-008: AST/IR Stack Layout Collision (ROOT CAUSE)

The AST prologue stores params at -8(%rbp), -16(%rbp), etc.
The IR emitter stores register spills at ir_asm_slot(r) = -8*(r+2),
which starts at -16(%rbp) -- COLLIDING with param 1.

Fix: offset all IR register spill slots to start BELOW the AST frame.
Add slot_base to IRAsmCtx. In body_only mode, slot_base = -stack_size.
All spill slots become (slot_base - 8*(r+1)), guaranteed below AST locals.

Run: python3 fix_cgir008.py
"""

import sys
import shutil

F = "compiler_passes.c"
shutil.copy(F, F + ".bak_cgir008")
print("Backup: " + F + ".bak_cgir008")

with open(F, "r") as f:
    c = f.read()

changes = 0

# === 1. Add slot_base field to IRAsmCtx (after body_only) ===
old1 = '    int         body_only;               /* 1 when AST owns prologue/epilogue (skip push/pop) */'
new1 = ('    int         body_only;               /* 1 when AST owns prologue/epilogue (skip push/pop) */\n'
        '    int         slot_base;               /* CG-IR-008: base offset for IR spill slots (0 or -stack_size) */')
if old1 in c:
    c = c.replace(old1, new1, 1)
    changes += 1
    print("1. Added slot_base to IRAsmCtx")
else:
    print("1. SKIP: body_only field not found (already patched?)")

# === 2. Change ir_asm_vreg_location to use slot_base ===
old2 = '    *out_slot = ir_asm_slot(r);'
new2 = '    *out_slot = ctx->slot_base - 8 * (int)(r + 1);  /* CG-IR-008: offset below AST frame */'
if old2 in c:
    c = c.replace(old2, new2, 1)
    changes += 1
    print("2. Changed ir_asm_vreg_location to use slot_base")
else:
    print("2. SKIP: ir_asm_slot(r) not found in vreg_location")

# === 3. Update alloca_off for non-param allocas ===
old3 = '            int off = (n < np) ? (-8 * (n + 1)) : (-8 * (int)(fn->n_regs + 2 + (n - np)));'
new3 = '            int off = (n < np) ? (-8 * (n + 1)) : (ctx->slot_base - 8 * (int)(fn->n_regs + 2 + (n - np)));'
if old3 in c:
    c = c.replace(old3, new3, 1)
    changes += 1
    print("3. Updated alloca_off for non-param allocas")
else:
    print("3. SKIP: alloca_off pattern not found")

# === 4. Set slot_base in zcc_run_passes_emit_body_pgo ===
old4 = '    ctx.body_only = 1;  /* AST owns prologue/epilogue'
if old4 in c:
    new4 = ('    ctx.body_only = 1;  /* AST owns prologue/epilogue'
            ' */\n'
            '    ctx.slot_base = -stack_size;  /* CG-IR-008: IR slots start below AST frame')
    # Need to replace just the first part and keep the closing */
    # Actually let me find the full line
    idx = c.index(old4)
    # Find the end of this line
    eol = c.index('\n', idx)
    full_line = c[idx:eol]
    new_lines = ('    ctx.body_only = 1;  /* AST owns prologue/epilogue -- skip push/pop of callee-saved */\n'
                 '    ctx.slot_base = -stack_size;  /* CG-IR-008: IR slots start below AST frame */')
    c = c[:idx] + new_lines + c[eol:]
    changes += 1
    print("4. Set slot_base = -stack_size in pgo path")
else:
    print("4. SKIP: body_only assignment not found")

# === 5. Update CG-IR-006 frame extension to always extend in body_only ===
old5 = """    /* CG-IR-006: extend stack if IR needs more slots than AST allocated.
     * ir_asm_slot(r) = -8*(r+2), so max depth = 8*(n_regs+2).
     * AST epilogue movq %rbp,%rsp undoes this automatically. */
    {
        int ir_need = 8 * ((int)fn->n_regs + 2) + 64;
        if (ir_need > stack_size) {
            int extra = ((ir_need - stack_size) + 15) & ~15;
            fprintf(out, "    subq $%d, %%rsp\\n", extra);
        }
    }"""
new5 = """    /* CG-IR-008: extend stack for IR register spills + allocas.
     * Spill slots at slot_base - 8*(r+1), allocas deeper still.
     * Always extend in body_only mode since IR needs its own space. */
    {
        int ir_extra = 8 * ((int)fn->n_regs + ctx.n_allocas + 4);
        ir_extra = (ir_extra + 15) & ~15;
        fprintf(out, "    subq $%d, %%rsp\\n", ir_extra);
    }"""
if old5 in c:
    c = c.replace(old5, new5, 1)
    changes += 1
    print("5. Updated CG-IR-006 frame extension")
else:
    print("5. SKIP: CG-IR-006 pattern not found, trying alternate...")
    # Try without the comment
    alt5 = '        int ir_need = 8 * ((int)fn->n_regs + 2) + 64;'
    if alt5 in c:
        # Replace the whole block
        start = c.index(alt5)
        # Find the closing brace
        brace_start = c.rfind('{', 0, start)
        brace_end = c.index('}', start) + 1
        old_block = c[brace_start:brace_end]
        new_block = """{
        int ir_extra = 8 * ((int)fn->n_regs + ctx.n_allocas + 4);
        ir_extra = (ir_extra + 15) & ~15;
        fprintf(out, "    subq $%d, %%rsp\\n", ir_extra);
    }"""
        c = c[:brace_start] + new_block + c[brace_end:]
        changes += 1
        print("5. Updated frame extension (alternate pattern)")
    else:
        print("5. FAIL: Could not find frame extension code")

with open(F, "w") as f:
    f.write(c)

print("\nApplied %d changes to %s" % (changes, F))
if changes < 4:
    print("WARNING: Not all changes applied. Check manually.")
print("Rebuild: make clean && make selfhost && bash verify_ir_backend.sh")
