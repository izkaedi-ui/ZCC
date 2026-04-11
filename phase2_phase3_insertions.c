/* ================================================================ */
/* PHASE 2 + PHASE 3 — IR Bridge Insertions for part4.c              */
/*                                                                   */
/* Updated with actual ZCC naming conventions:                       */
/*   ir_bridge_fresh_tmp()  (not ir_fresh_tmp — collision with ir.c) */
/*   node->sym->stack_offset  (not node->offset)                    */
/*   Symbol null checks before name access                           */
/*                                                                   */
/* Each section shows the EXACT code to paste into the corresponding */
/* switch case. The { } blocks scope the IR locals cleanly.          */
/* ================================================================ */


/* ================================================================ */
/* PHASE 2: ARITHMETIC CORE                                          */
/* ================================================================ */

/* ── ALL BINARY OPS ──────────────────────────────────────────────── */
/*                                                                     */
/* Applies to these cases in codegen_expr:                             */
/*   ND_ADD, ND_SUB, ND_MUL, ND_DIV, ND_MOD                         */
/*   ND_BAND, ND_BOR, ND_BXOR, ND_SHL, ND_SHR                       */
/*   ND_EQ, ND_NE, ND_LT, ND_LE, ND_GT, ND_GE                       */
/*                                                                     */
/* Insert THREE lines into each binary case:                          */
/*   1. ir_save_result(lhs_ir)  — AFTER codegen_expr(lhs), BEFORE push */
/*   2. ir_save_result(rhs_ir)  — AFTER codegen_expr(rhs), BEFORE pop  */
/*   3. ir_emit_binary_op(...)  — at the END of the case, before break */
/*                                                                     */
/* IMPORTANT: Declare lhs_ir and rhs_ir at the TOP of the case or at  */
/* the top of codegen_expr (C89 style — ZCC needs declarations first) */

/* TEMPLATE — copy this block and adjust the case label: */

/*
    case ND_ADD: {
        char lhs_ir[32];
        char rhs_ir[32];

        codegen_expr(cc, node->lhs);
        ir_save_result(lhs_ir);           // <── PHASE 2 INSERT

        fprintf(cc->out, "    pushq %%rax\n");
        codegen_expr(cc, node->rhs);
        ir_save_result(rhs_ir);           // <── PHASE 2 INSERT

        fprintf(cc->out, "    popq %%rcx\n");

        // ... existing pointer-arithmetic scaling, addq, etc. ...

        ir_emit_binary_op(ND_ADD, node->ty, lhs_ir, rhs_ir, node->line);
                                          // <── PHASE 2 INSERT
        break;
    }
*/

/* REPEAT for ND_SUB, ND_MUL, ND_DIV, ND_MOD:
 *   ir_emit_binary_op(ND_SUB, ...)
 *   ir_emit_binary_op(ND_MUL, ...)
 *   ir_emit_binary_op(ND_DIV, ...)
 *   ir_emit_binary_op(ND_MOD, ...)
 *
 * REPEAT for ND_BAND, ND_BOR, ND_BXOR, ND_SHL, ND_SHR:
 *   ir_emit_binary_op(ND_BAND, ...)
 *   etc.
 *
 * REPEAT for ND_EQ, ND_NE, ND_LT, ND_LE, ND_GT, ND_GE:
 *   ir_emit_binary_op(ND_EQ, ...)
 *   etc.
 */

/* ── IF YOUR BINARY CASES SHARE A CODEPATH ───────────────────────── */
/*                                                                     */
/* Some compilers use a combined handler for all binary ops:           */
/*                                                                     */
/*   case ND_ADD: case ND_SUB: case ND_MUL: ... {                    */
/*       codegen_expr(cc, node->lhs);                                */
/*       push; codegen_expr(cc, node->rhs); pop;                     */
/*       switch (node->kind) { ... emit specific instr ... }          */
/*       break;                                                       */
/*   }                                                                 */
/*                                                                     */
/* If that's your structure, the insert is even simpler — one block:   */
/*                                                                     */
/*   codegen_expr(cc, node->lhs);                                     */
/*   ir_save_result(lhs_ir);          // INSERT HERE                  */
/*   push;                                                             */
/*   codegen_expr(cc, node->rhs);                                     */
/*   ir_save_result(rhs_ir);          // INSERT HERE                  */
/*   pop;                                                              */
/*   switch (node->kind) { ... }                                       */
/*   ir_emit_binary_op(node->kind, node->ty, lhs_ir, rhs_ir,         */
/*                      node->line);  // INSERT HERE                  */


/* ── ND_NEG (unary minus) ────────────────────────────────────────── */
/*
    case ND_NEG: {
        char src_ir[32];

        codegen_expr(cc, node->lhs);
        ir_save_result(src_ir);

        fprintf(cc->out, "    negq %%rax\n");

        {
            char *dst = ir_bridge_fresh_tmp();
            ZCC_EMIT_UNARY(IR_NEG, ir_map_type(node->ty),
                           dst, src_ir, node->line);
        }
        break;
    }
*/


/* ── ND_BNOT (bitwise ~) ────────────────────────────────────────── */
/*
    case ND_BNOT: {
        char src_ir[32];

        codegen_expr(cc, node->lhs);
        ir_save_result(src_ir);

        fprintf(cc->out, "    notq %%rax\n");

        {
            char *dst = ir_bridge_fresh_tmp();
            ZCC_EMIT_UNARY(IR_NOT, ir_map_type(node->ty),
                           dst, src_ir, node->line);
        }
        break;
    }
*/


/* ── ND_LNOT (logical !) ────────────────────────────────────────── */
/*
    case ND_LNOT: {
        char src_ir[32];

        codegen_expr(cc, node->lhs);
        ir_save_result(src_ir);

        fprintf(cc->out, "    cmpq $0, %%rax\n");
        fprintf(cc->out, "    sete %%al\n");
        fprintf(cc->out, "    movzbq %%al, %%rax\n");

        // IR: decompose !x as (x == 0)
        {
            char zero_ir[32];
            char *zt = ir_bridge_fresh_tmp();
            int i;
            for (i = 0; zt[i]; i++) zero_ir[i] = zt[i];
            zero_ir[i] = 0;
            ZCC_EMIT_CONST(IR_TY_I64, zero_ir, 0, node->line);
        }
        {
            char *dst = ir_bridge_fresh_tmp();
            ZCC_EMIT_BINARY(IR_EQ, IR_TY_I32, dst,
                            src_ir, zero_ir, node->line);
        }
        break;
    }
*/
/* NOTE: zero_ir scope issue — declare it alongside src_ir at case top:
 *   char src_ir[32]; char zero_ir[32];
 */


/* ── ND_CAST ────────────────────────────────────────────────────── */
/*
    case ND_CAST: {
        char src_ir[32];

        codegen_expr(cc, node->lhs);
        ir_save_result(src_ir);

        // ... existing sign/zero extension movsx/movzx/cdqe ...

        {
            char *dst = ir_bridge_fresh_tmp();
            ZCC_EMIT_UNARY(IR_CAST, ir_map_type(node->ty),
                           dst, src_ir, node->line);
        }
        break;
    }
*/


/* ================================================================ */
/* PHASE 3: CONTROL FLOW                                             */
/* ================================================================ */

/* ── ND_IF ───────────────────────────────────────────────────────── */
/*                                                                     */
/* Find the existing pattern in your codegen_stmt:                     */
/*   codegen_expr(cc, node->cond);                                    */
/*   fprintf(cc->out, "    cmpq $0, %%rax\n");                       */
/*   fprintf(cc->out, "    je .L%d\n", lbl_else);                    */
/*                                                                     */
/* INSERT after codegen_expr(cond), before the cmpq:                  */
/*                                                                     */
/*   {                                                                 */
/*       char cond_ir[32];                                            */
/*       char lbl_buf[32];                                            */
/*       ir_save_result(cond_ir);                                     */
/*       sprintf(lbl_buf, ".L%d", lbl_else);                         */
/*       ZCC_EMIT_BR_IF(cond_ir, lbl_buf, node->line);               */
/*   }                                                                 */
/*                                                                     */
/* Then at each label emission point (fprintf ".L%d:\n"):             */
/*   {                                                                 */
/*       char lbl_buf[32];                                            */
/*       sprintf(lbl_buf, ".L%d", lbl_N);                            */
/*       ZCC_EMIT_LABEL(lbl_buf, node->line);                        */
/*   }                                                                 */
/*                                                                     */
/* And at each unconditional jump (fprintf "jmp .L%d\n"):             */
/*   {                                                                 */
/*       char lbl_buf[32];                                            */
/*       sprintf(lbl_buf, ".L%d", lbl_end);                          */
/*       ZCC_EMIT_BR(lbl_buf, node->line);                           */
/*   }                                                                 */


/* ── ND_FOR ──────────────────────────────────────────────────────── */
/*                                                                     */
/* Your existing structure likely looks like:                          */
/*                                                                     */
/*   if (node->init) codegen_stmt(cc, node->init);                   */
/*   fprintf(cc->out, ".L%d:\n", lbl_begin);    // loop top          */
/*   if (node->cond) {                                                */
/*       codegen_expr(cc, node->cond);                                */
/*       fprintf(cc->out, "    cmpq $0, %%rax\n");                   */
/*       fprintf(cc->out, "    je .L%d\n", lbl_end);                 */
/*   }                                                                 */
/*   codegen_stmt(cc, node->body);                                    */
/*   fprintf(cc->out, ".L%d:\n", lbl_continue);  // continue target  */
/*   if (node->inc) codegen_expr(cc, node->inc);                     */
/*   fprintf(cc->out, "    jmp .L%d\n", lbl_begin);                  */
/*   fprintf(cc->out, ".L%d:\n", lbl_end);                           */
/*                                                                     */
/* Mirror every fprintf label/jmp with the corresponding IR call:     */

/*
    case ND_FOR: {
        int lbl_begin = cc->label_count++;
        int lbl_end   = cc->label_count++;
        int lbl_cont  = cc->label_count++;
        char ir_lbl[32];

        if (node->init) codegen_stmt(cc, node->init);

        // loop top
        fprintf(cc->out, ".L%d:\n", lbl_begin);
        sprintf(ir_lbl, ".L%d", lbl_begin);
        ZCC_EMIT_LABEL(ir_lbl, node->line);

        if (node->cond) {
            char cond_ir[32];
            codegen_expr(cc, node->cond);
            ir_save_result(cond_ir);
            fprintf(cc->out, "    cmpq $0, %%rax\n");
            fprintf(cc->out, "    je .L%d\n", lbl_end);
            sprintf(ir_lbl, ".L%d", lbl_end);
            ZCC_EMIT_BR_IF(cond_ir, ir_lbl, node->line);
        }

        codegen_stmt(cc, node->body);

        // continue target
        fprintf(cc->out, ".L%d:\n", lbl_cont);
        sprintf(ir_lbl, ".L%d", lbl_cont);
        ZCC_EMIT_LABEL(ir_lbl, node->line);

        if (node->inc) codegen_expr(cc, node->inc);

        fprintf(cc->out, "    jmp .L%d\n", lbl_begin);
        sprintf(ir_lbl, ".L%d", lbl_begin);
        ZCC_EMIT_BR(ir_lbl, node->line);

        // loop end
        fprintf(cc->out, ".L%d:\n", lbl_end);
        sprintf(ir_lbl, ".L%d", lbl_end);
        ZCC_EMIT_LABEL(ir_lbl, node->line);

        break;
    }
*/

/* ── ND_WHILE ────────────────────────────────────────────────────── */
/*                                                                     */
/* Structurally identical to FOR with no init/inc.                     */
/* Same pattern: label at top, br_if after cond, br back at bottom.   */
/*                                                                     */
/* Just mirror each fprintf ".L%d" with ZCC_EMIT_LABEL                */
/* and each "jmp"/"je" with ZCC_EMIT_BR/ZCC_EMIT_BR_IF.              */


/* ── ND_DO_WHILE ─────────────────────────────────────────────────── */
/*
    // Body first, then condition
    fprintf(cc->out, ".L%d:\n", lbl_begin);
    sprintf(ir_lbl, ".L%d", lbl_begin);
    ZCC_EMIT_LABEL(ir_lbl, node->line);

    codegen_stmt(cc, node->body);

    // continue target (before cond eval)
    fprintf(cc->out, ".L%d:\n", lbl_cont);
    sprintf(ir_lbl, ".L%d", lbl_cont);
    ZCC_EMIT_LABEL(ir_lbl, node->line);

    codegen_expr(cc, node->cond);
    ir_save_result(cond_ir);
    fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    jne .L%d\n", lbl_begin);  // loop back if true
    // IR: br_if with inverted sense (branch to begin if nonzero)
    sprintf(ir_lbl, ".L%d", lbl_begin);
    ZCC_EMIT_BR_IF(cond_ir, ir_lbl, node->line);

    fprintf(cc->out, ".L%d:\n", lbl_end);
    sprintf(ir_lbl, ".L%d", lbl_end);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
*/


/* ── ND_BREAK ────────────────────────────────────────────────────── */
/*
    case ND_BREAK: {
        char ir_lbl[32];
        fprintf(cc->out, "    jmp .L%d\n", cc->break_label);
        sprintf(ir_lbl, ".L%d", cc->break_label);
        ZCC_EMIT_BR(ir_lbl, node->line);
        break;
    }
*/


/* ── ND_CONTINUE ─────────────────────────────────────────────────── */
/*
    case ND_CONTINUE: {
        char ir_lbl[32];
        fprintf(cc->out, "    jmp .L%d\n", cc->continue_label);
        sprintf(ir_lbl, ".L%d", cc->continue_label);
        ZCC_EMIT_BR(ir_lbl, node->line);
        break;
    }
*/


/* ── ND_GOTO ─────────────────────────────────────────────────────── */
/*
    case ND_GOTO: {
        char ir_lbl[64];
        fprintf(cc->out, "    jmp .L_%s\n", node->label_name);
        sprintf(ir_lbl, ".L_%s", node->label_name);
        ZCC_EMIT_BR(ir_lbl, node->line);
        break;
    }
*/


/* ── ND_LABEL (user label:) ──────────────────────────────────────── */
/*
    case ND_LABEL: {
        char ir_lbl[64];
        fprintf(cc->out, ".L_%s:\n", node->label_name);
        sprintf(ir_lbl, ".L_%s", node->label_name);
        ZCC_EMIT_LABEL(ir_lbl, node->line);
        codegen_stmt(cc, node->body);
        break;
    }
*/


/* ── ND_SWITCH / ND_CASE ────────────────────────────────────────── */
/*                                                                     */
/* Switch is the most complex. Your x86-64 likely emits a comparison  */
/* chain. Mirror each comparison + conditional jump:                   */
/*                                                                     */
/*   For each case value:                                             */
/*     fprintf(cc->out, "    cmpq $%ld, %%rax\n", case_val);         */
/*     fprintf(cc->out, "    je .L%d\n", case_label);                 */
/*                                                                     */
/*   IR mirror:                                                       */
/*     char cmp_ir[32]; char *ct;                                     */
/*     ct = ir_bridge_fresh_tmp();                                    */
/*     sprintf(cmp_ir, "%s", ct);                                     */
/*     ZCC_EMIT_BINARY(IR_EQ, IR_TY_I64, cmp_ir,                     */
/*                      switch_val_ir, case_const_ir, node->line);    */
/*     sprintf(ir_lbl, ".L%d", case_label);                           */
/*     ZCC_EMIT_BR_IF(cmp_ir, ir_lbl, node->line);                   */
/*                                                                     */
/* For the default/end jump:                                           */
/*     ZCC_EMIT_BR(default_or_end_lbl, node->line);                   */


/* ── ND_BLOCK ────────────────────────────────────────────────────── */
/* No IR emission needed. Just recurse into children as existing. */


/* ── ND_EXPR_STMT ────────────────────────────────────────────────── */
/* No additional IR needed. codegen_expr handles it. */


/* ================================================================ */
/* VERIFICATION CHECKLIST                                            */
/* ================================================================ */

/*
 * After inserting Phase 2 + Phase 3:
 *
 * 1. make clean && make zcc
 *    → Compilation succeeds (no parse errors from new IR calls)
 *
 * 2. echo 'int main(){int x; x=3+4; if(x>5) return 1; return 0;}' > test_p23.c
 *    ./zcc test_p23.c -o test_p23.s
 *    → Assembly file generated, IR output on stdout shows:
 *      func main {
 *        %t0 = const i32 3
 *        %t1 = const i32 4
 *        %t2 = add i32 %t0, %t1
 *        store i32 %x, %t2
 *        %t3 = load i32 %x
 *        %t4 = const i32 5
 *        %t5 = gt i32 %t3, %t4
 *        br_if %t5, .L<N>
 *        ...
 *      }
 *
 * 3. make ir-verify
 *    → "IR BRIDGE SELF-HOST VERIFIED"
 *    → This is the ONLY acceptance criterion.
 *
 * If ir-verify fails, the IR insertion altered the asm output.
 * Most likely cause: a codegen_expr call order changed, or a
 * local variable declaration moved past a label (C89 violation
 * that ZCC enforces but GCC doesn't).
 *
 * Debug: diff zcc2_ir.s zcc3_ir.s | head -40
 * The first divergent line points to the exact function.
 */


/* ================================================================ */
/* DECLARATION PLACEMENT WARNING                                     */
/* ================================================================ */

/*
 * ZCC enforces C89-style declarations-before-statements.
 *
 * WRONG (will fail to self-host):
 *   case ND_ADD:
 *       codegen_expr(cc, node->lhs);
 *       char lhs_ir[32];        // ← declaration after statement!
 *
 * RIGHT:
 *   case ND_ADD: {
 *       char lhs_ir[32];        // ← at top of block
 *       char rhs_ir[32];
 *       codegen_expr(cc, node->lhs);
 *       ir_save_result(lhs_ir);
 *       ...
 *   }
 *
 * Every case that adds IR locals MUST use a { } block with
 * declarations at the top. This is the #1 self-host trap.
 */
