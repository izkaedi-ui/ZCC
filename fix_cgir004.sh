#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════
# 🔱 CG-IR-004 FIX — Phantom Callee-Save Push/Pop in Body-Only Mode
# ═══════════════════════════════════════════════════════════════════════
#
# Problem: When ZCC_IR_BACKEND=1, codegen_func (part4.c) emits:
#   1. Prologue: pushq %rbp; subq $N, %rsp
#   2. Callee saves: movq %rbx, -904(%rbp)  (via movq, no RSP change)
#   3. Param stores: movq %rdi, -8(%rbp)
#   4. *** DELEGATES BODY to ir_asm_emit_function_body ***
#   5. Epilogue: restore callee regs via movq; leave; ret
#
# But ir_asm_emit_one_block also emits:
#   - pushq %rbx; pushq %r12  ← shifts RSP by 16, corrupts ALL offsets
#   - OP_RET: popq %r12; popq %rbx ← pops garbage from wrong RSP
#
# Fix: Add body_only flag to IRAsmCtx. When set:
#   - Skip callee-save pushes (AST prologue already saved them)
#   - Skip callee-save pops in RET (AST epilogue already restores them)
#   - RET still jumps to .Lfunc_end_N (already correct)
#
# 3 edits to compiler_passes.c:
#   Edit 1: Add body_only field to IRAsmCtx struct
#   Edit 2: Guard callee-save pushes with !ctx->body_only
#   Edit 3: Guard callee-save pops in OP_RET with !ctx->body_only  
#   Edit 4: Set body_only=1 in zcc_run_passes_emit_body_pgo
# ═══════════════════════════════════════════════════════════════════════

set -euo pipefail

FILE="compiler_passes.c"

if [ ! -f "$FILE" ]; then
    echo "ERROR: $FILE not found. Run this from the selforglinux directory."
    exit 1
fi

# Safety backup
cp "$FILE" "${FILE}.bak_cgir004"
echo "Backup saved to ${FILE}.bak_cgir004"

# ─────────────────────────────────────────────────────────────────
# Edit 1: Add body_only field to IRAsmCtx struct
# Target: after "int callee_saved_emitted;" line
# ─────────────────────────────────────────────────────────────────
echo "Edit 1: Adding body_only field to IRAsmCtx..."

sed -i 's/    int         callee_saved_emitted;    \/\* 1 after we'\''ve emitted push of callee-saved \*\//    int         callee_saved_emitted;    \/\* 1 after we'\''ve emitted push of callee-saved \*\/\n    int         body_only;               \/\* 1 when AST owns prologue\/epilogue (skip push\/pop) \*\//' "$FILE"

# Verify
if grep -q 'int         body_only;' "$FILE"; then
    echo "  ✓ body_only field added"
else
    echo "  ✗ FAILED to add body_only field"
    exit 1
fi

# ─────────────────────────────────────────────────────────────────
# Edit 2: Guard callee-save pushes with !ctx->body_only
# Target: the block starting "if (!ctx->callee_saved_emitted && ctx->used_callee_saved_mask)"
# ─────────────────────────────────────────────────────────────────
echo "Edit 2: Guarding callee-save pushes..."

sed -i 's/    if (!ctx->callee_saved_emitted && ctx->used_callee_saved_mask) {/    if (!ctx->body_only \&\& !ctx->callee_saved_emitted \&\& ctx->used_callee_saved_mask) {/' "$FILE"

# Verify
if grep -q '!ctx->body_only && !ctx->callee_saved_emitted' "$FILE"; then
    echo "  ✓ Callee-save push guarded"
else
    echo "  ✗ FAILED to guard callee-save push"
    exit 1
fi

# ─────────────────────────────────────────────────────────────────
# Edit 3: Guard callee-save pops in OP_RET with !ctx->body_only
# Target: the for loop in OP_RET that emits popq
# We need to wrap the pop loop in an if(!ctx->body_only) block
# ─────────────────────────────────────────────────────────────────
echo "Edit 3: Guarding callee-save pops in OP_RET..."

# This is trickier — we need to find the exact pattern in OP_RET
# The pattern is:
#   case OP_RET:
#       if (ins->n_src >= 1) ir_asm_load_to_rax(ctx, ins->src[0]);
#       for (int i = CALLEE_SAVED_PUSH_N - 1; i >= 0; i--) {
#           int p = callee_saved_push_order[i];
#           if (ctx->used_callee_saved_mask & (1 << p))
#               fprintf(f, "    popq %%%s\n", phys_reg_name[p]);
#       }

# Use a Python script for the multi-line replacement since sed is painful here
python3 << 'PYEOF'
import re

with open("compiler_passes.c", "r") as f:
    content = f.read()

# Find and replace the OP_RET pop loop
old_pattern = """        case OP_RET:
            if (ins->n_src >= 1) ir_asm_load_to_rax(ctx, ins->src[0]);
            for (int i = CALLEE_SAVED_PUSH_N - 1; i >= 0; i--) {
                int p = callee_saved_push_order[i];
                if (ctx->used_callee_saved_mask & (1 << p))
                    fprintf(f, "    popq %%%s\\n", phys_reg_name[p]);
            }
            fprintf(f, "    jmp .Lfunc_end_%d\\n", ctx->func_end_label);"""

new_pattern = """        case OP_RET:
            if (ins->n_src >= 1) ir_asm_load_to_rax(ctx, ins->src[0]);
            if (!ctx->body_only) {
                for (int i = CALLEE_SAVED_PUSH_N - 1; i >= 0; i--) {
                    int p = callee_saved_push_order[i];
                    if (ctx->used_callee_saved_mask & (1 << p))
                        fprintf(f, "    popq %%%s\\n", phys_reg_name[p]);
                }
            }
            fprintf(f, "    jmp .Lfunc_end_%d\\n", ctx->func_end_label);"""

if old_pattern in content:
    content = content.replace(old_pattern, new_pattern, 1)
    with open("compiler_passes.c", "w") as f:
        f.write(content)
    print("  OK: OP_RET pop loop guarded")
else:
    print("  WARNING: Could not find exact OP_RET pattern. Manual edit needed.")
    print("  Look for 'case OP_RET:' around line 4727 and wrap the popq loop with:")
    print("    if (!ctx->body_only) { ... }")
PYEOF

# Verify
if grep -q '!ctx->body_only' "$FILE" && grep -c 'body_only' "$FILE" | grep -q '[3-9]'; then
    echo "  ✓ OP_RET pop guarded"
else
    echo "  ⚠ Check OP_RET manually"
fi

# ─────────────────────────────────────────────────────────────────
# Edit 4: Set body_only=1 in zcc_run_passes_emit_body_pgo
# Target: after "memset(&ctx, 0, sizeof(ctx));" in that function
# ─────────────────────────────────────────────────────────────────
echo "Edit 4: Setting body_only=1 in zcc_run_passes_emit_body_pgo..."

# Find the specific memset in zcc_run_passes_emit_body_pgo 
# It's followed by ctx.out = out; ctx.fn = fn; etc.
# We add ctx.body_only = 1; after ctx.out = out;

python3 << 'PYEOF'
with open("compiler_passes.c", "r") as f:
    content = f.read()

# In zcc_run_passes_emit_body_pgo, find the ctx setup block
# The unique marker is: it's in a function that starts with
# "int zcc_run_passes_emit_body_pgo(" and has "ctx.out = out;"
# We need to add body_only = 1 after the memset

# Find the PGO function's ctx setup
marker = """    IRAsmCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.out = out;
    ctx.fn = fn;
    ctx.func_end_label = func_end_label;
    ctx.num_params = num_params;
    ctx.global_block_offset = 0;
    ctx.func_label_id = func_label_id;"""

replacement = """    IRAsmCtx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.out = out;
    ctx.fn = fn;
    ctx.func_end_label = func_end_label;
    ctx.num_params = num_params;
    ctx.global_block_offset = 0;
    ctx.func_label_id = func_label_id;
    ctx.body_only = 1;  /* AST owns prologue/epilogue — skip push/pop of callee-saved */"""

count = content.count(marker)
if count == 1:
    content = content.replace(marker, replacement, 1)
    with open("compiler_passes.c", "w") as f:
        f.write(content)
    print("  OK: body_only=1 set in zcc_run_passes_emit_body_pgo")
elif count == 0:
    print("  WARNING: Could not find exact ctx setup pattern in pgo function.")
    print("  Manually add 'ctx.body_only = 1;' after memset(&ctx, 0, ...) in zcc_run_passes_emit_body_pgo")
else:
    print(f"  WARNING: Found {count} matches — pattern is ambiguous. Manual edit needed.")
PYEOF

echo ""
echo "═══════════════════════════════════════════════════════════"
echo "🔱 CG-IR-004 PATCH APPLIED"
echo "═══════════════════════════════════════════════════════════"
echo ""
echo "Changes made to compiler_passes.c:"
echo "  1. Added 'int body_only;' field to IRAsmCtx struct"
echo "  2. Guarded callee-save pushq with !ctx->body_only"
echo "  3. Guarded callee-save popq in OP_RET with !ctx->body_only"
echo "  4. Set ctx.body_only=1 in zcc_run_passes_emit_body_pgo"
echo ""
echo "Now rebuild and verify:"
echo "  make clean && make selfhost"
echo "  bash verify_ir_backend.sh"
echo ""
echo "Expected: Stage 6 (zcc3_ir compiles source) should now PASS"
echo "Backup at: ${FILE}.bak_cgir004"
