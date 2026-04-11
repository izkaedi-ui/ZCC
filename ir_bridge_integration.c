/* ================================================================ */
/* ir_bridge_integration.c                                           */
/*                                                                   */
/* EXACT INSERTION PATTERNS for part4.c                              */
/*                                                                   */
/* This file is a REFERENCE — not compiled directly.                 */
/* Copy each pattern into the corresponding switch case in part4.c.  */
/*                                                                   */
/* Convention:                                                       */
/*   EXISTING  = current x86-64 codegen (leave untouched)           */
/*   IR_INSERT = new IR emission (add after existing codegen)        */
/*                                                                   */
/* Architecture:                                                     */
/*   codegen_expr leaves result in %rax (x86) and ir_last_result    */
/*   (IR). Callers save ir_last_result before recursive calls.       */
/* ================================================================ */

#include "ir_bridge.h"

/* ================================================================ */
/* PART A: codegen_expr — value-producing expressions                */
/* ================================================================ */

/*
 * INSERTION POINT: At the top of codegen_expr, BEFORE the switch.
 * Add nothing — ir_bridge.h functions are called per-case.
 */


/* ── A1: ND_NUM (integer literal) ────────────────────────────────── */
/*
 * EXISTING: fprintf(cc->out, "    movq $%ld, %%rax\n", node->ival);
 *
 * IR_INSERT (add immediately after the fprintf):
 */
static void ir_pattern_nd_num(Compiler *cc, Node *node) {
    /* ... existing x86-64 movq ... */

    /* IR: %tN = const <type> <value> */
    {
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_CONST(ir_map_type(node->ty), dst, node->ival, node->line);
    }
}


/* ── A2: ND_STR (string literal) ─────────────────────────────────── */
/*
 * EXISTING: fprintf(cc->out, "    leaq .LS%d(%%rip), %%rax\n", node->str_id);
 *
 * IR_INSERT:
 */
static void ir_pattern_nd_str(Compiler *cc, Node *node) {
    /* ... existing x86-64 leaq ... */

    /* IR: %tN = const ptr &.LS<id> */
    {
        char str_ref[32];
        char *dst = ir_fresh_tmp();
        sprintf(str_ref, "&.LS%d", node->str_id);
        ZCC_EMIT_CONST(IR_TY_PTR, dst, 0, node->line);
        /* Note: CONST with PTR type and 0 immval + string ref in dst name
         * is a convention. Alternative: add ZCC_EMIT_CONST_STR if you want
         * the actual string content in the IR. */
    }
}


/* ── A3: ND_VAR (variable reference) ─────────────────────────────── */
/*
 * EXISTING (local): fprintf(cc->out, "    leaq %d(%%rbp), %%rax\n", ...);
 *                   fprintf(cc->out, "    movq (%%rax), %%rax\n");
 *
 * EXISTING (global): fprintf(cc->out, "    leaq %s(%%rip), %%rax\n", ...);
 *                    fprintf(cc->out, "    movq (%%rax), %%rax\n");
 *
 * IR_INSERT (at the END of the ND_VAR case, after all existing codegen):
 */
static void ir_pattern_nd_var(Compiler *cc, Node *node) {
    /* ... existing x86-64 leaq + movq ... */

    /* IR: %tN = load <type> %varname */
    {
        char *vname = ir_var_name(node);
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_LOAD(ir_map_type(node->ty), dst, vname, node->line);
    }
}


/* ── A4: ND_ADD, ND_SUB, ND_MUL, ND_DIV, ND_MOD ────────────────── */
/* ── A5: ND_BAND, ND_BOR, ND_BXOR, ND_SHL, ND_SHR ──────────────── */
/* ── A6: ND_EQ, ND_NE, ND_LT, ND_LE, ND_GT, ND_GE ──────────────── */
/*
 * ALL binary ops follow the same pattern.
 *
 * EXISTING:
 *   codegen_expr(cc, node->lhs);   // result in %rax
 *   fprintf(cc->out, "    push %%rax\n");
 *   codegen_expr(cc, node->rhs);   // result in %rax
 *   fprintf(cc->out, "    pop %%rcx\n");
 *   fprintf(cc->out, "    addq %%rcx, %%rax\n");  // (varies per op)
 *
 * IR_INSERT:
 *   Save lhs IR temp BEFORE codegen_expr(rhs) overwrites ir_last_result.
 *   After rhs codegen, emit the binary IR instruction.
 */
static void ir_pattern_binary(Compiler *cc, Node *node) {
    char lhs_tmp[32];
    char rhs_tmp[32];

    /* codegen_expr(cc, node->lhs) */
    /* ... existing push %rax ... */

    /* IR: save lhs result before rhs overwrites it */
    ir_save_result(lhs_tmp);

    /* codegen_expr(cc, node->rhs) */
    /* ... existing pop %rcx, op ... */

    /* IR: save rhs result */
    ir_save_result(rhs_tmp);

    /* IR: %tN = <op> <type> lhs_tmp, rhs_tmp */
    ir_emit_binary_op(node->kind, node->ty, lhs_tmp, rhs_tmp, node->line);
}

/*
 * CONCRETE EXAMPLE — what the ND_ADD case looks like after insertion:
 *
 *   case ND_ADD:
 *       codegen_expr(cc, node->lhs);
 *       ir_save_result(lhs_tmp);              // <── IR INSERT
 *       fprintf(cc->out, "    pushq %%rax\n");
 *       codegen_expr(cc, node->rhs);
 *       ir_save_result(rhs_tmp);              // <── IR INSERT
 *       fprintf(cc->out, "    popq %%rcx\n");
 *       // ... pointer arithmetic checks ...
 *       fprintf(cc->out, "    addq %%rcx, %%rax\n");
 *       ir_emit_binary_op(ND_ADD, node->ty, lhs_tmp, rhs_tmp, node->line);
 *                                             // <── IR INSERT
 *       break;
 */


/* ── A7: ND_NEG (unary negation) ─────────────────────────────────── */
/*
 * EXISTING: codegen_expr(cc, node->lhs);
 *           fprintf(cc->out, "    negq %%rax\n");
 *
 * IR_INSERT:
 */
static void ir_pattern_nd_neg(Compiler *cc, Node *node) {
    char src_tmp[32];

    /* codegen_expr(cc, node->lhs); */
    ir_save_result(src_tmp);

    /* ... existing negq ... */

    /* IR: %tN = neg <type> src_tmp */
    {
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_UNARY(IR_NEG, ir_map_type(node->ty), dst, src_tmp, node->line);
    }
}


/* ── A8: ND_BNOT (bitwise NOT) ───────────────────────────────────── */
/*
 * Same pattern as NEG but with IR_NOT.
 */
static void ir_pattern_nd_bnot(Compiler *cc, Node *node) {
    char src_tmp[32];

    /* codegen_expr(cc, node->lhs); */
    ir_save_result(src_tmp);

    /* ... existing notq ... */

    {
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_UNARY(IR_NOT, ir_map_type(node->ty), dst, src_tmp, node->line);
    }
}


/* ── A9: ND_LNOT (logical NOT: !x) ──────────────────────────────── */
/*
 * EXISTING: codegen_expr(cc, node->lhs);
 *           fprintf(cc->out, "    cmpq $0, %%rax\n");
 *           fprintf(cc->out, "    sete %%al\n");
 *           fprintf(cc->out, "    movzbq %%al, %%rax\n");
 *
 * IR: Decompose to: %tN = eq i32 src, $0
 */
static void ir_pattern_nd_lnot(Compiler *cc, Node *node) {
    char src_tmp[32];
    char zero_tmp[32];

    /* codegen_expr(cc, node->lhs); */
    ir_save_result(src_tmp);

    /* ... existing cmpq, sete, movzbq ... */

    /* IR: emit a zero constant, then eq comparison */
    {
        char *zt = ir_fresh_tmp();
        sprintf(zero_tmp, "%s", zt);
        ZCC_EMIT_CONST(IR_TY_I64, zero_tmp, 0, node->line);
    }
    {
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_BINARY(IR_EQ, IR_TY_I32, dst, src_tmp, zero_tmp, node->line);
    }
}


/* ── A10: ND_ADDR (&x) ──────────────────────────────────────────── */
/*
 * EXISTING: codegen_addr(cc, node->lhs);  // leaves address in %rax
 *
 * IR: The address IS the result. For locals, emit alloca reference.
 */
static void ir_pattern_nd_addr(Compiler *cc, Node *node) {
    /* ... existing codegen_addr ... */

    /* IR: if lhs is a var, result is the var's address name */
    if (node->lhs->kind == ND_VAR) {
        char *vname = ir_var_name(node->lhs);
        char *dst = ir_fresh_tmp();
        /* addr_of is just a copy of the alloca pointer */
        ZCC_EMIT_UNARY(IR_COPY, IR_TY_PTR, dst, vname, node->line);
    }
    /* else: ir_last_result from codegen_addr is already correct */
}


/* ── A11: ND_DEREF (*p) ─────────────────────────────────────────── */
/*
 * EXISTING: codegen_expr(cc, node->lhs);  // pointer in %rax
 *           fprintf(cc->out, "    movq (%%rax), %%rax\n");
 *
 * IR: %tN = load <pointed_type> ptr_tmp
 */
static void ir_pattern_nd_deref(Compiler *cc, Node *node) {
    char ptr_tmp[32];

    /* codegen_expr(cc, node->lhs); */
    ir_save_result(ptr_tmp);

    /* ... existing movq (%%rax) ... */

    {
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_LOAD(ir_map_type(node->ty), dst, ptr_tmp, node->line);
    }
}


/* ── A12: ND_ASSIGN (lhs = rhs) ─────────────────────────────────── */
/*
 * EXISTING: codegen_addr(cc, node->lhs);   // address in %rax
 *           push %rax
 *           codegen_expr(cc, node->rhs);    // value in %rax
 *           pop %rcx
 *           movq %rax, (%rcx)
 *
 * IR: store <type> addr_tmp, val_tmp
 */
static void ir_pattern_nd_assign(Compiler *cc, Node *node) {
    char addr_tmp[32];
    char val_tmp[32];

    /* codegen_addr(cc, node->lhs); */
    /* For IR, get the variable name if it's a simple var */
    if (node->lhs->kind == ND_VAR) {
        sprintf(addr_tmp, "%%%s", node->lhs->sym->name);
    } else {
        ir_save_result(addr_tmp);
    }

    /* push %rax */
    /* codegen_expr(cc, node->rhs); */
    ir_save_result(val_tmp);

    /* ... existing pop, movq ... */

    /* IR: store <type> addr_tmp, val_tmp */
    ZCC_EMIT_STORE(ir_map_type(node->ty), addr_tmp, val_tmp, node->line);

    /* Assignment result = the stored value (already in ir_last_result from rhs) */
    ir_save_result(val_tmp);  /* ir_last_result stays as rhs value */
}


/* ── A13: ND_CAST ────────────────────────────────────────────────── */
/*
 * EXISTING: codegen_expr(cc, node->lhs);
 *           // sign/zero extend as needed
 *
 * IR: %tN = cast <to_type> src_tmp
 */
static void ir_pattern_nd_cast(Compiler *cc, Node *node) {
    char src_tmp[32];

    /* codegen_expr(cc, node->lhs); */
    ir_save_result(src_tmp);

    /* ... existing extension instructions ... */

    {
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_UNARY(IR_CAST, ir_map_type(node->ty), dst, src_tmp, node->line);
    }
}


/* ── A14: ND_FUNC_CALL ──────────────────────────────────────────── */
/*
 * EXISTING: for each arg: codegen_expr(cc, arg); move to rdi/rsi/rdx/rcx/r8/r9
 *           call <func_name>
 *           // result in %rax
 *
 * IR: emit ARG for each argument, then CALL
 */
static void ir_pattern_nd_func_call(Compiler *cc, Node *node) {
    Node *arg;
    char arg_tmps[6][32];  /* max 6 register args for SysV */
    int nargs;
    int i;

    /* Emit args — save each IR temp BEFORE the next codegen_expr */
    nargs = 0;
    arg = node->args;
    while (arg && nargs < 6) {
        /* codegen_expr(cc, arg); */
        ir_save_result(arg_tmps[nargs]);
        nargs++;
        arg = arg->next;
    }

    /* ... existing register shuffling, call instruction ... */

    /* IR: emit ARG instructions, then CALL */
    for (i = 0; i < nargs; i++) {
        ZCC_EMIT_ARG(IR_TY_I64, arg_tmps[i], node->line);
    }
    {
        char *dst = ir_fresh_tmp();
        ZCC_EMIT_CALL(ir_map_type(node->ty), dst, node->func_name, node->line);
    }
}


/* ── A15: ND_LAND (&&), ND_LOR (||), ND_TERNARY ─────────────────── */
/*
 * These involve short-circuit evaluation with jumps.
 * For IR, emit as br_if chains. Full SSA would need phi nodes,
 * but for Phase 1 we emit the linearized control flow version.
 *
 * IR Pattern for (a && b):
 *   codegen_expr(a) → src_a
 *   br_if src_a, .L_rhs
 *   %tN = const i32 0        ; short-circuit false
 *   br .L_end
 *   .L_rhs:
 *   codegen_expr(b) → src_b
 *   .L_end:
 *   ; result is in ir_last_result (either 0 or src_b)
 *
 * Phase 2 will add PHI nodes for proper SSA.
 */


/* ── A16: ND_PRE_INC, ND_PRE_DEC, ND_POST_INC, ND_POST_DEC ─────── */
/*
 * Decompose in IR:
 *   PRE_INC(x):  load x → tmp; add tmp, 1 → tmp2; store x, tmp2; result = tmp2
 *   POST_INC(x): load x → tmp; add tmp, 1 → tmp2; store x, tmp2; result = tmp
 *
 * The existing x86-64 codegen already does the equivalent.
 * Just trace the IR operations alongside.
 */


/* ── A17: ND_MEMBER (struct.field) ───────────────────────────────── */
/*
 * EXISTING: codegen_addr(lhs); add offset; load
 *
 * IR: Similar to deref with explicit offset arithmetic.
 * Phase 1: emit as load from computed address.
 */


/* ── A18: ND_COMMA_EXPR (a, b) ───────────────────────────────────── */
/*
 * EXISTING: codegen_expr(a); codegen_expr(b);  // result is b
 * IR: Just trace both; ir_last_result naturally holds b's result.
 */


/* ── A19: ND_COMPOUND_ASSIGN (+=, -=, etc.) ──────────────────────── */
/*
 * Decompose in IR:
 *   x += y  →  tmp = load x; tmp2 = add tmp, y; store x, tmp2
 */


/* ================================================================ */
/* PART B: codegen_stmt — control flow                               */
/* ================================================================ */


/* ── B1: ND_RETURN ───────────────────────────────────────────────── */
/*
 * EXISTING: codegen_expr(cc, node->lhs);  // value in %rax
 *           jmp to epilogue
 *
 * IR_INSERT (after codegen_expr, before the jmp):
 */
static void ir_pattern_nd_return(Compiler *cc, Node *node) {
    char ret_tmp[32];

    if (node->lhs) {
        /* codegen_expr(cc, node->lhs); */
        ir_save_result(ret_tmp);

        /* IR: ret <type> ret_tmp */
        ZCC_EMIT_RET(ir_map_type(node->lhs->ty), ret_tmp, node->line);
    } else {
        /* IR: ret void */
        ZCC_EMIT_RET(IR_TY_VOID, "", node->line);
    }

    /* ... existing jmp to epilogue ... */
}


/* ── B2: ND_IF ───────────────────────────────────────────────────── */
/*
 * EXISTING:
 *   codegen_expr(cc, node->cond);
 *   cmpq $0, %rax
 *   je .Lelse_N / .Lend_N
 *   codegen_stmt(cc, node->then);
 *   jmp .Lend_N
 *   .Lelse_N:
 *   codegen_stmt(cc, node->els);
 *   .Lend_N:
 *
 * IR_INSERT: After codegen_expr(cond), emit br_if.
 *            Emit labels for else/end.
 */
static void ir_pattern_nd_if(Compiler *cc, Node *node, int lbl_else, int lbl_end) {
    char cond_tmp[32];
    char lbl_else_buf[32];
    char lbl_end_buf[32];

    sprintf(lbl_else_buf, ".L%d", lbl_else);
    sprintf(lbl_end_buf, ".L%d", lbl_end);

    /* codegen_expr(cc, node->cond); */
    ir_save_result(cond_tmp);

    /* IR: br_if cond_tmp, .Lelse (inverted — branch if false) */
    /* NOTE: x86 does je (jump if zero), so IR mirrors that logic */
    ZCC_EMIT_BR_IF(cond_tmp, lbl_else_buf, node->line);

    /* codegen_stmt(cc, node->then); */

    if (node->els) {
        /* IR: br .Lend */
        ZCC_EMIT_BR(lbl_end_buf, node->line);
        /* IR: .Lelse: */
        ZCC_EMIT_LABEL(lbl_else_buf, node->line);
        /* codegen_stmt(cc, node->els); */
    }

    /* IR: .Lend: */
    ZCC_EMIT_LABEL(lbl_end_buf, node->line);
}


/* ── B3: ND_FOR ──────────────────────────────────────────────────── */
/*
 * for (init; cond; inc) body
 *
 * IR:
 *   <init>
 *   .Lbegin:
 *   <cond> → cond_tmp
 *   br_if cond_tmp, .Lend     (branch to end if false)
 *   <body>
 *   .Lcontinue:
 *   <inc>
 *   br .Lbegin
 *   .Lend:
 */
static void ir_pattern_nd_for(Compiler *cc, Node *node,
                               int lbl_begin, int lbl_end, int lbl_cont) {
    char cond_tmp[32];
    char lbl_begin_buf[32];
    char lbl_end_buf[32];
    char lbl_cont_buf[32];

    sprintf(lbl_begin_buf, ".L%d", lbl_begin);
    sprintf(lbl_end_buf, ".L%d", lbl_end);
    sprintf(lbl_cont_buf, ".L%d", lbl_cont);

    /* codegen_stmt(init) or codegen_expr(init) */

    /* IR: .Lbegin: */
    ZCC_EMIT_LABEL(lbl_begin_buf, node->line);

    if (node->cond) {
        /* codegen_expr(cc, node->cond); */
        ir_save_result(cond_tmp);
        ZCC_EMIT_BR_IF(cond_tmp, lbl_end_buf, node->line);
    }

    /* codegen_stmt(body) */

    /* IR: .Lcontinue: */
    ZCC_EMIT_LABEL(lbl_cont_buf, node->line);

    if (node->inc) {
        /* codegen_expr(cc, node->inc); */
    }

    /* IR: br .Lbegin */
    ZCC_EMIT_BR(lbl_begin_buf, node->line);

    /* IR: .Lend: */
    ZCC_EMIT_LABEL(lbl_end_buf, node->line);
}


/* ── B4: ND_WHILE ────────────────────────────────────────────────── */
/*
 * Identical to FOR with no init/inc. Same IR pattern.
 */


/* ── B5: ND_DO_WHILE ────────────────────────────────────────────── */
/*
 * IR:
 *   .Lbegin:
 *   <body>
 *   .Lcontinue:
 *   <cond> → cond_tmp
 *   br_if cond_tmp, .Lbegin   (branch BACK if true — inverted)
 *   .Lend:
 */


/* ── B6: ND_BREAK ────────────────────────────────────────────────── */
/*
 * EXISTING: jmp .Lbreak_N
 * IR: br .Lbreak_N
 */
static void ir_pattern_nd_break(Compiler *cc, Node *node, int lbl_break) {
    char buf[32];
    sprintf(buf, ".L%d", lbl_break);
    ZCC_EMIT_BR(buf, node->line);
}


/* ── B7: ND_CONTINUE ─────────────────────────────────────────────── */
/*
 * EXISTING: jmp .Lcontinue_N
 * IR: br .Lcontinue_N
 */
static void ir_pattern_nd_continue(Compiler *cc, Node *node, int lbl_cont) {
    char buf[32];
    sprintf(buf, ".L%d", lbl_cont);
    ZCC_EMIT_BR(buf, node->line);
}


/* ── B8: ND_LABEL / ND_GOTO ─────────────────────────────────────── */
/*
 * EXISTING: .L_user_label: / jmp .L_user_label
 * IR: label <name> / br <name>
 */


/* ── B9: ND_BLOCK ────────────────────────────────────────────────── */
/*
 * Just recurse into children. No IR emission needed for the block itself.
 */


/* ── B10: ND_EXPR_STMT ──────────────────────────────────────────── */
/*
 * codegen_expr(cc, node->lhs);
 * No additional IR needed — the expression emission handles it.
 */


/* ── B11: ND_SWITCH / ND_CASE ────────────────────────────────────── */
/*
 * IR: Decompose into a chain of br_if comparisons.
 *
 *   codegen_expr(switch_val) → val_tmp
 *   %cmp0 = eq val_tmp, <case0_val>
 *   br_if %cmp0, .Lcase0
 *   %cmp1 = eq val_tmp, <case1_val>
 *   br_if %cmp1, .Lcase1
 *   br .Ldefault  (or .Lend if no default)
 *   .Lcase0: ...
 *   .Lcase1: ...
 */


/* ================================================================ */
/* PART C: Function Prologue/Epilogue                                */
/* ================================================================ */

/*
 * In codegen_function (or wherever you emit function prologues):
 *
 * EXISTING: fprintf(cc->out, "%s:\n", func->name);
 *           fprintf(cc->out, "    pushq %%rbp\n");
 *           ...
 *
 * IR_INSERT (at the very beginning, before any fprintf):
 *
 *   ir_bridge_func_begin(func->name, func->ret_type);
 *
 * And at the end of the function (after epilogue emission):
 *
 *   ir_bridge_func_end();
 *
 * For local variables allocated on the stack:
 *
 *   IR: %varname = alloca <bytes>
 *
 *   For each local variable in the function's scope:
 *   {
 *       char vname[64];
 *       sprintf(vname, "%%%s", sym->name);
 *       ZCC_EMIT_ALLOCA(vname, type_size(sym->ty), node->line);
 *   }
 */


/* ================================================================ */
/* PART D: Implementation Order (Phased Rollout)                     */
/* ================================================================ */

/*
 * Phase 1 — Skeleton (get IR output flowing):
 *   1. Add #include "ir_bridge.h" near top of part4.c
 *   2. Add ir_bridge_func_begin / ir_bridge_func_end in codegen_function
 *   3. Add IR emission for: ND_NUM, ND_VAR, ND_RETURN
 *   4. Rebuild, run on a trivial test:
 *        int main() { return 42; }
 *      Verify IR output: func main { %t0 = const i32 42; ret i32 %t0 }
 *   5. Self-host verify: stage2 == stage3 still passes
 *      (IR emission is side-effect only — must not alter asm output)
 *
 * Phase 2 — Arithmetic Core:
 *   6. Add IR for all binary ops (ND_ADD through ND_GE)
 *   7. Add IR for unary ops (ND_NEG, ND_BNOT, ND_LNOT)
 *   8. Add IR for ND_CAST
 *   9. Test with: int main() { return (3 + 4) * 2; }
 *
 * Phase 3 — Control Flow:
 *   10. ND_IF with br_if/label
 *   11. ND_FOR, ND_WHILE, ND_DO_WHILE
 *   12. ND_BREAK, ND_CONTINUE
 *   13. ND_SWITCH decomposition
 *
 * Phase 4 — Memory & Calls:
 *   14. ND_ASSIGN (store), ND_DEREF (load through pointer)
 *   15. ND_ADDR, ND_MEMBER
 *   16. ND_FUNC_CALL with ARG chain
 *   17. ND_COMPOUND_ASSIGN, ND_PRE_INC/DEC, ND_POST_INC/DEC
 *
 * Phase 5 — Full Self-Host IR:
 *   18. Run IR emission on zcc.c itself
 *   19. Validate IR output covers all ~50K lines
 *   20. Feed IR to optimization passes (DCE first)
 *
 *
 * CRITICAL INVARIANT at every phase:
 *   make verify   (stage2 == stage3)
 *   IR emission must be PURE SIDE EFFECT.
 *   If asm output changes, you broke something.
 */


/* ================================================================ */
/* PART E: Makefile Additions                                        */
/* ================================================================ */

/*
 * Add to PARTS (ir_bridge.h must be included in the concatenation):
 *
 *   PARTS = part1.c part2.c part3.c part4.c ir.h ir_bridge.h \
 *           ir_emit_dispatch.h part5.c ir.c
 *
 * New targets:
 *
 *   # Run compiler with IR output to stdout
 *   ir: zcc
 *   	./zcc test.c -o test.s 2>&1 | head -200
 *
 *   # Full IR dump of the compiler itself
 *   ir-self: zcc
 *   	./zcc zcc.c -o zcc_ir.s > zcc_ir.json 2>&1
 *
 *   # IR + verify (proves IR emission doesn't break asm)
 *   ir-verify: zcc
 *   	./zcc zcc.c -o zcc2_ir.s > /dev/null 2>&1
 *   	./zcc2 zcc.c -o zcc3_ir.s > /dev/null 2>&1
 *   	@cmp -s zcc2_ir.s zcc3_ir.s && echo "IR BRIDGE SELF-HOST VERIFIED" \
 *   	    || (echo "IR BRIDGE BROKE SELF-HOST" && exit 1)
 */
