/* ═══════════════════════════════════════════════════════════════════
 * CG-IR-004 FIX: Parameter Slot Pre-Seeding for ir_module_lower_x86
 * ═══════════════════════════════════════════════════════════════════
 *
 * Drop this into compiler_passes.c, adapting struct/field names
 * to match your actual IRAsmCtx and IRFunc definitions.
 *
 * The fix has TWO parts:
 *   1. force_register_var() — insert a named var at a SPECIFIC offset
 *   2. pre_seed_params()    — called BEFORE body emission loop
 *
 * After applying, the slot assignment invariant becomes:
 *   param slots:  -8(%rbp), -16(%rbp), ... -48(%rbp)  [ABI-fixed]
 *   callee saves: -56(%rbp) ... -88(%rbp)              [prologue-fixed]
 *   IR locals:    -(callee_end + 8)(%rbp) and below    [encounter-order OK]
 */


/* ── Part 1: Force-register a variable at a specific stack offset ──── */

/*
 * Unlike get_or_create_var which assigns the NEXT available slot,
 * this forces a specific offset. If the name already exists, overwrite.
 * If not, create it at the given offset WITHOUT advancing slot_base.
 */
static void force_register_var(IRAsmCtx *ctx, const char *name, int offset)
{
    int i;

    /* Search existing table */
    for (i = 0; i < ctx->var_count; i++) {
        if (strcmp(ctx->vars[i].name, name) == 0) {
            /* Already exists — just fix the offset */
            ctx->vars[i].offset = offset;
            return;
        }
    }

    /* New entry — do NOT touch slot_base */
    safe_copy(ctx->vars[ctx->var_count].name, name,
              sizeof(ctx->vars[ctx->var_count].name));
    ctx->vars[ctx->var_count].offset = offset;
    ctx->var_count++;
}


/* ── Part 2: Pre-seed function parameters at ABI offsets ──────────── */

/*
 * System V AMD64 ABI: first 6 integer/pointer params in registers.
 * The prologue (emitted separately) stores them at:
 *   param 0 (%rdi) → -8(%rbp)
 *   param 1 (%rsi) → -16(%rbp)
 *   param 2 (%rdx) → -24(%rbp)
 *   param 3 (%rcx) → -32(%rbp)
 *   param 4 (%r8)  → -40(%rbp)
 *   param 5 (%r9)  → -48(%rbp)
 *
 * This function pre-registers those names so get_or_create_var
 * returns the correct ABI offset when the body references them.
 *
 * MUST be called AFTER prologue emission, BEFORE body loop.
 */
static void pre_seed_params(IRAsmCtx *ctx, IRFunc *func)
{
    int p;
    int abi_offset;
    IRNode *node;

    /*
     * Strategy: scan entry block for the first num_params ALLOCA
     * instructions. These correspond to function parameters.
     * Their dst names are the parameter variable names.
     *
     * If func->param_names exists and is populated, use that instead.
     */

#if 0  /* Option A: use func->param_names if available */
    for (p = 0; p < func->num_params; p++) {
        if (!func->param_names[p]) continue;
        abi_offset = -(8 * (p + 1));
        force_register_var(ctx, func->param_names[p], abi_offset);
    }
#else  /* Option B: extract from IR instruction stream */
    p = 0;
    node = func->entry_block->head;  /* adjust field name as needed */

    while (node && p < func->num_params) {
        if (node->op == IR_ALLOCA && node->dst && node->dst[0] != '\0') {
            abi_offset = -(8 * (p + 1));
            force_register_var(ctx, node->dst, abi_offset);
            p++;
        } else if (node->op != IR_LABEL && node->op != IR_NOP) {
            /* First non-ALLOCA/LABEL/NOP means we're past param region */
            break;
        }
        node = node->next;
    }
#endif

    /*
     * CRITICAL: Set slot_base so new locals start BELOW params + callee saves.
     *
     * Adjust these numbers to match your prologue:
     *   params:       8 * num_params bytes
     *   callee saves: 5 regs * 8 = 40 bytes (r12, r13, r14, r15, rbx)
     *   padding:      may need alignment to 16
     *
     * Example for main(argc, argv):
     *   params = 2 * 8 = 16   → -8, -16
     *   callee = 5 * 8 = 40   → -56, -64, -72, -80, -88
     *   slot_base = -88 - 8 = -96  (or wherever your prologue's subq lands)
     *
     * Actually, your prologue already does `subq $912, %rsp` which covers
     * locals. Then the IR body does `subq $2624, %rsp` for IR vars.
     * So slot_base should be set to -(prologue_frame + 8) to start IR
     * locals just below the prologue frame.
     */
    ctx->slot_base = -(8 * func->num_params) - ctx->callee_save_size;

    /* If your prologue uses a fixed frame size (like 256 or 912),
     * you may want: ctx->slot_base = -prologue_frame_size;
     * The key invariant: no IR local slot can overlap [-8 .. -48] */
}


/* ── Part 3: Integration point ────────────────────────────────────── */

/*
 * In ir_module_lower_x86, AFTER emitting the function prologue
 * and BEFORE the body emission loop, insert:
 *
 *   pre_seed_params(&ctx, func);
 *
 * Example insertion point (find the equivalent in your code):
 */

#if 0  /* Pseudo-code showing insertion point */
static void ir_asm_emit_function(IRAsmCtx *ctx, IRFunc *func)
{
    /* 1. Emit prologue (pushq %rbp, movq %rsp, subq, save callee regs,
     *    store params from rdi/rsi/rdx/rcx/r8/r9 to stack) */
    ir_asm_emit_prologue(ctx, func);

    /* 2. ★ PRE-SEED PARAMS HERE ★ */
    pre_seed_params(ctx, func);

    /* 3. Emit IR body frame (additional subq for IR variables) */
    ir_asm_emit_body_frame(ctx, func);

    /* 4. Emit body instructions */
    ir_asm_emit_function_body(ctx);  /* line ~4978 in your code */

    /* 5. Emit epilogue */
    ir_asm_emit_epilogue(ctx, func);
}
#endif


/* ═══════════════════════════════════════════════════════════════════
 * VERIFICATION AFTER APPLYING FIX:
 *
 * 1. make clean && make selfhost  → SELF-HOST VERIFIED
 * 2. Compile test_cg_ir_004.c with IR backend
 *    → ./test_ir hello -o out.s foo
 *    → must print: argc=5 input_file=hello output_file=out.s
 * 3. make ir-verify → IR BRIDGE SELF-HOST VERIFIED
 * 4. zcc3_ir compiles zcc_pp.c without crash
 * ═══════════════════════════════════════════════════════════════════ */
