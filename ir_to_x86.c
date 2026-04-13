/*
 * ir_to_x86.c — ZCC IR-to-x86_64 Lowering Backend
 *
 * Walks the ir_module_t and generates System V x86-64 assembly,
 * completely replacing the AST-based codegen in part4.c.
 */

#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[IR_NAME_MAX];
    int offset;
} ir_var_t;

static ir_var_t *vars = 0;
static int max_vars = 0;
static int num_vars = 0;
static int next_offset_from_rbp = -8;

static int get_or_create_var(const char *name) {
    int i;
    if (!name || name[0] == '\0' || name[0] == '-') return 0;
    
    for (i = 0; i < num_vars; i++) {
        if (strcmp(vars[i].name, name) == 0) return vars[i].offset;
    }
    
    if (num_vars >= max_vars) {
        max_vars = (max_vars == 0) ? 2048 : max_vars * 2;
        ir_var_t *new_vars = (ir_var_t *)realloc(vars, max_vars * sizeof(ir_var_t));
        if (!new_vars) {
            fprintf(stderr, "\n[FATAL] IR Backend: Variable limit exceeded and out of memory!\n");
            fprintf(stderr, "Tried to allocate %d variables.\n", max_vars);
            exit(1);
        }
        vars = new_vars;
    }
    
    int off;
    strcpy(vars[num_vars].name, name);
    vars[num_vars].offset = next_offset_from_rbp;
    off = next_offset_from_rbp;
    next_offset_from_rbp -= 8;
    num_vars++;
    return off;
}

static void load_address(FILE *out, const char *src, const char *reg) {
    if (strncmp(src, "%stack_", 7) == 0) {
        int off = get_or_create_var(src);
        fprintf(out, "    leaq %d(%%rbp), %s\n", off, reg);
    } else if (strncmp(src, "%t", 2) == 0) {
        int off = get_or_create_var(src);
        fprintf(out, "    movq %d(%%rbp), %s\n", off, reg);
    } else {
        const char *name = src;
        if (name[0] == '%') name++; /* remove '%' prefix for globals */
        fprintf(out, "    leaq %s(%%rip), %s\n", name, reg);
    }
}

void ir_module_lower_x86(const ir_module_t *mod, FILE *out) {
    int i;
    const char *arg_regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    fprintf(out, "    .text\n");
    
    for (i = 0; i < mod->func_count; i++) {
        ir_func_t *fn = mod->funcs[i];
        ir_node_t *n;
        int stack_size;
        int arg_idx;

        num_vars = 0;
        next_offset_from_rbp = -8;
        
        /* Pass 1: Linear scan to allocate stack slots for all variables */
        n = fn->head;
        while (n) {
            if (n->dst[0] != '\0' && n->dst[0] != '-') get_or_create_var(n->dst);
            if (n->src1[0] != '\0' && n->src1[0] != '-') get_or_create_var(n->src1);
            if (n->src2[0] != '\0' && n->src2[0] != '-') get_or_create_var(n->src2);
            /* Add space for ALLOCA */
            if (n->op == IR_ALLOCA) {
                next_offset_from_rbp -= n->imm;
            }
            n = n->next;
        }
        
        stack_size = -next_offset_from_rbp + 40; /* extra padding for calls */
        stack_size = (stack_size + 15) & ~15; /* 16-byte alignment */
        
        /* Prologue */
        fprintf(out, "    .globl %s\n", fn->name);
        fprintf(out, "%s:\n", fn->name);
        fprintf(out, "    pushq %%rbp\n");
        fprintf(out, "    movq %%rsp, %%rbp\n");
        fprintf(out, "    subq $%d, %%rsp\n", stack_size);
        
        /* Note: System V Parameters */
        for (int pnum = 0; pnum < fn->num_params; pnum++) {
            char param_name[32];
            sprintf(param_name, "%%stack_%d", -8 * (pnum + 1));
            int stack_off = get_or_create_var(param_name);
            if (pnum < 6) {
                fprintf(out, "    movq %s, %d(%%rbp)\n", arg_regs[pnum], stack_off);
            }
        }

        arg_idx = 0;

        /* Pass 2: Emit corresponding assembly */
        n = fn->head;
        while (n) {
            fprintf(out, "    # %s\n", ir_op_name(n->op));
            switch (n->op) {
                case IR_CONST: {
                    int off = get_or_create_var(n->dst);
                    fprintf(out, "    movabsq $%ld, %%rax\n", n->imm);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", off);
                    break;
                }
                case IR_CONST_STR: {
                    int off = get_or_create_var(n->dst);
                    fprintf(out, "    leaq %s(%%rip), %%rax\n", n->src1);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", off);
                    break;
                }
                case IR_ADD:
                case IR_SUB:
                case IR_MUL:
                case IR_AND:
                case IR_OR:
                case IR_XOR:
                case IR_SHL:
                case IR_SHR:
                case IR_EQ:
                case IR_NE:
                case IR_LT:
                case IR_LE:
                case IR_GT:
                case IR_GE: {
                    int off1 = get_or_create_var(n->src1);
                    int off2 = get_or_create_var(n->src2);
                    int offd = get_or_create_var(n->dst);
                    
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    if (n->op == IR_ADD)        fprintf(out, "    addq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_SUB)   fprintf(out, "    subq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_MUL)   fprintf(out, "    imulq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_AND)   fprintf(out, "    andq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_OR)    fprintf(out, "    orq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_XOR)   fprintf(out, "    xorq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_SHL) {
                        fprintf(out, "    movq %d(%%rbp), %%rcx\n", off2);
                        fprintf(out, "    shlq %%cl, %%rax\n");
                    } else if (n->op == IR_SHR) {
                        fprintf(out, "    movq %d(%%rbp), %%rcx\n", off2);
                        if (ir_type_unsigned(n->type)) fprintf(out, "    shrq %%cl, %%rax\n");
                        else fprintf(out, "    sarq %%cl, %%rax\n");
                    } else {
                        fprintf(out, "    cmpq %d(%%rbp), %%rax\n", off2);
                        if (n->op == IR_EQ) fprintf(out, "    sete %%al\n");
                        else if (n->op == IR_NE) fprintf(out, "    setne %%al\n");
                        else if (n->op == IR_LT) fprintf(out, "    setl %%al\n");
                        else if (n->op == IR_LE) fprintf(out, "    setle %%al\n");
                        else if (n->op == IR_GT) fprintf(out, "    setg %%al\n");
                        else if (n->op == IR_GE) fprintf(out, "    setge %%al\n");
                        fprintf(out, "    movzbq %%al, %%rax\n");
                    }
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_DIV:
                case IR_MOD: {
                    int off1 = get_or_create_var(n->src1);
                    int off2 = get_or_create_var(n->src2);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    if (ir_type_unsigned(n->type)) {
                        fprintf(out, "    xorq %%rdx, %%rdx\n");
                        fprintf(out, "    divq %d(%%rbp)\n", off2);
                    } else {
                        fprintf(out, "    cqo\n");
                        fprintf(out, "    idivq %d(%%rbp)\n", off2);
                    }
                    if (n->op == IR_MOD) fprintf(out, "    movq %%rdx, %d(%%rbp)\n", offd);
                    else fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_NOT:
                case IR_NEG: {
                    int off1 = get_or_create_var(n->src1);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    if (n->op == IR_NOT) fprintf(out, "    notq %%rax\n");
                    else fprintf(out, "    negq %%rax\n");
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_LOAD: {
                    int offd = get_or_create_var(n->dst);
                    load_address(out, n->src1, "%rax");
                    if (n->type == IR_TY_I32 || n->type == IR_TY_U32) {
                        fprintf(out, "    movl (%%rax), %%eax\n");
                        if (!ir_type_unsigned(n->type)) fprintf(out, "    movslq %%eax, %%rax\n");
                    } else if (n->type == IR_TY_I8 || n->type == IR_TY_U8) {
                        fprintf(out, "    movzbl (%%rax), %%eax\n");
                        if (!ir_type_unsigned(n->type)) fprintf(out, "    movsbq %%al, %%rax\n");
                    } else if (n->type == IR_TY_I16 || n->type == IR_TY_U16) {
                        fprintf(out, "    movzwl (%%rax), %%eax\n");
                        if (!ir_type_unsigned(n->type)) fprintf(out, "    movswq %%ax, %%rax\n");
                    } else {
                        fprintf(out, "    movq (%%rax), %%rax\n");
                    }
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_ADDR: {
                    int offd = get_or_create_var(n->dst);
                    load_address(out, n->src1, "%rax");
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_STORE: {
                    int off1 = get_or_create_var(n->src1); /* src1 holds the value */
                    fprintf(out, "    movq %d(%%rbp), %%rcx\n", off1);
                    load_address(out, n->dst, "%rax");     /* dst holds the address */
                    
                    if (n->type == IR_TY_I32 || n->type == IR_TY_U32) {
                        fprintf(out, "    movl %%ecx, (%%rax)\n");
                    } else if (n->type == IR_TY_I8 || n->type == IR_TY_U8) {
                        fprintf(out, "    movb %%cl, (%%rax)\n");
                    } else {
                        fprintf(out, "    movq %%rcx, (%%rax)\n");
                    }
                    break;
                }
                case IR_ALLOCA: {
                    int offd = get_or_create_var(n->dst);
                    /* Space was already skipped in pass 1 setup */
                    fprintf(out, "    leaq %d(%%rbp), %%rax\n", vars[offd].offset - (int)n->imm);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_COPY:
                case IR_CAST: {
                    int off1 = get_or_create_var(n->src1);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_LABEL: {
                    fprintf(out, "%s:\n", n->label);
                    break;
                }
                case IR_BR: {
                    fprintf(out, "    jmp %s\n", n->label);
                    break;
                }
                case IR_BR_IF: {
                    int off1 = get_or_create_var(n->src1);
                    fprintf(out, "    cmpq $0, %d(%%rbp)\n", off1);
                    fprintf(out, "    je %s\n", n->label);
                    break;
                }
                case IR_ARG: {
                    int off1 = get_or_create_var(n->src1);
                    if (arg_idx < 6) {
                        fprintf(out, "    movq %d(%%rbp), %s\n", off1, arg_regs[arg_idx]);
                    } else {
                        fprintf(out, "    pushq %d(%%rbp)\n", off1);
                    }
                    arg_idx++;
                    break;
                }
                case IR_CALL: {
                    fprintf(out, "    movb $0, %%al\n");
                    fprintf(out, "    callq %s\n", n->label);
                    if (arg_idx > 6) {
                        fprintf(out, "    addq $%d, %%rsp\n", (arg_idx - 6) * 8);
                    }
                    if (n->dst[0] != '\0' && n->dst[0] != '-') {
                        int offd = get_or_create_var(n->dst);
                        fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    }
                    arg_idx = 0; /* Reset for next call */
                    break;
                }
                case IR_RET: {
                    if (n->src1[0] != '\0' && n->src1[0] != '-') {
                        int off1 = get_or_create_var(n->src1);
                        fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    }
                    fprintf(out, "    movq %%rbp, %%rsp\n");
                    fprintf(out, "    popq %%rbp\n");
                    fprintf(out, "    ret\n");
                    break;
                }
                default: break;
                case IR_FCONST: {
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movabsq $%ld, %%rax\n", n->imm);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_FADD:
                case IR_FSUB:
                case IR_FMUL:
                case IR_FDIV: {
                    int off1 = get_or_create_var(n->src1);
                    int off2 = get_or_create_var(n->src2);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%xmm0\n", off1);
                    fprintf(out, "    movq %d(%%rbp), %%xmm1\n", off2);
                    if (n->op == IR_FADD)      fprintf(out, "    addsd %%xmm1, %%xmm0\n");
                    else if (n->op == IR_FSUB) fprintf(out, "    subsd %%xmm1, %%xmm0\n");
                    else if (n->op == IR_FMUL) fprintf(out, "    mulsd %%xmm1, %%xmm0\n");
                    else                        fprintf(out, "    divsd %%xmm1, %%xmm0\n");
                    fprintf(out, "    movq %%xmm0, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_ITOF: {
                    int off1 = get_or_create_var(n->src1);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    fprintf(out, "    cvtsi2sdq %%rax, %%xmm0\n");
                    fprintf(out, "    movq %%xmm0, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_FTOI: {
                    int off1 = get_or_create_var(n->src1);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%xmm0\n", off1);
                    fprintf(out, "    cvttsd2si %%xmm0, %%rax\n");
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }

            }
            n = n->next;
        }
        
        /* Fallback epilogue incase missing explicit RET */
        fprintf(out, "    movq %%rbp, %%rsp\n");
        fprintf(out, "    popq %%rbp\n");
        fprintf(out, "    ret\n");
    }
}
