#!/usr/bin/env python3
"""CG-IR-017: 32-bit instruction selection for ADD/SUB/MUL/NEG/BOR/BXOR/BNOT/BAND/CMP."""
import sys

with open('compiler_passes.c', 'r') as f:
    src = f.read()

orig = src
changes = []

def rep(old, new, tag):
    global src
    if old not in src:
        print(f"  MISS: {tag}", file=sys.stderr)
        return
    src = src.replace(old, new, 1)
    changes.append(tag)
    print(f"  OK:   {tag}")

# ══════════════════════════════════════════════════════════════
# LAYER 1 — node-builder: stamp ir_type on each op
# ══════════════════════════════════════════════════════════════

# ZND_ADD (no ir_type yet; followed by case ZND_SUB)
rep(
    '    ins->op = OP_ADD;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_SUB: {',

    '    ins->op = OP_ADD;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_SUB: {',
    'ZND_ADD ir_type stamp'
)

# ZND_SUB (followed by case ZND_MOD)
rep(
    '    ins->op = OP_SUB;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_MOD: {',

    '    ins->op = OP_SUB;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_MOD: {',
    'ZND_SUB ir_type stamp'
)

# ZND_MUL (followed by case ZND_DIV)
rep(
    '    ins->op = OP_MUL;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_DIV: {',

    '    ins->op = OP_MUL;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_DIV: {',
    'ZND_MUL ir_type stamp'
)

# ZND_NEG lowers to OP_SUB (0 - x); anchor: line_no before dst
rep(
    '    ins->op = OP_SUB;\n'
    '    ins->line_no = node->line_no;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = zero_r;',

    '    ins->op = OP_SUB;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017: NEG → 0-x */\n'
    '    ins->line_no = node->line_no;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = zero_r;',
    'ZND_NEG ir_type stamp'
)

# ZND_BOR
rep(
    '    ins->op = OP_BOR;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_BXOR: {',

    '    ins->op = OP_BOR;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_BXOR: {',
    'ZND_BOR ir_type stamp'
)

# ZND_BXOR
rep(
    '    ins->op = OP_BXOR;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_BNOT: {',

    '    ins->op = OP_BXOR;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_BNOT: {',
    'ZND_BXOR ir_type stamp'
)

# ZND_BNOT
rep(
    '    ins->op = OP_BNOT;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->n_src = 1;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_LNOT: {',

    '    ins->op = OP_BNOT;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->n_src = 1;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_LNOT: {',
    'ZND_BNOT ir_type stamp'
)

# ZND_BAND
rep(
    '    ins->op = OP_BAND;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_SHL: {',

    '    ins->op = OP_BAND;\n'
    '    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_SHL: {',
    'ZND_BAND ir_type stamp'
)

# ZND_LT/LE/GT/GE/EQ/NE — use lhs operand width for cmp instruction
rep(
    '    ins->op = cmp_op;\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_POST_INC: {',

    '    ins->op = cmp_op;\n'
    '    ins->ir_type = irtype_from_node(node->lhs); /* CG-IR-017: operand width */\n'
    '    ins->dst = r;\n'
    '    ins->src[0] = l;\n'
    '    ins->src[1] = rh;\n'
    '    ins->n_src = 2;\n'
    '    ins->exec_freq = 1.0;\n'
    '    ins->line_no = node->line_no;\n'
    '    break;\n'
    '  }\n'
    '  case ZND_POST_INC: {',
    'ZND_CMP ir_type stamp (operand width from lhs)'
)

# ══════════════════════════════════════════════════════════════
# LAYER 2 — ir_asm_lower_insn: width-aware instruction emission
# Strategy for binary ops: movq src[1]→%rcx, then use %ecx/%rcx.
# This is identical to the CG-IR-015 pattern used for DIV/MOD.
# ══════════════════════════════════════════════════════════════

# OP_ADD
rep(
    '  case OP_ADD: {\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    addq ");\n'
    '    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '    fprintf(f, ", %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',

    '  case OP_ADD: {\n'
    '    /* CG-IR-017: addl for I32/U32, addq for I64/U64 */\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {\n'
    '      fprintf(f, "    movq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rcx\\n");\n'
    '      fprintf(f, "    addl %%ecx, %%eax\\n");\n'
    '    } else {\n'
    '      fprintf(f, "    addq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rax\\n");\n'
    '    }\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',
    'OP_ADD 32/64 dispatch'
)

# OP_SUB
rep(
    '  case OP_SUB: {\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    subq ");\n'
    '    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '    fprintf(f, ", %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',

    '  case OP_SUB: {\n'
    '    /* CG-IR-017: subl for I32/U32, subq for I64/U64 */\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {\n'
    '      fprintf(f, "    movq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rcx\\n");\n'
    '      fprintf(f, "    subl %%ecx, %%eax\\n");\n'
    '    } else {\n'
    '      fprintf(f, "    subq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rax\\n");\n'
    '    }\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',
    'OP_SUB 32/64 dispatch'
)

# OP_MUL
rep(
    '  case OP_MUL: {\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    movq ");\n'
    '    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '    fprintf(f, ", %%rcx\\n");\n'
    '    fprintf(f, "    imulq %%rcx, %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',

    '  case OP_MUL: {\n'
    '    /* CG-IR-017: imull for I32/U32, imulq for I64/U64 */\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    movq ");\n'
    '    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '    fprintf(f, ", %%rcx\\n");\n'
    '    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32)\n'
    '      fprintf(f, "    imull %%ecx, %%eax\\n");\n'
    '    else\n'
    '      fprintf(f, "    imulq %%rcx, %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',
    'OP_MUL 32/64 dispatch'
)

# OP_BAND — in ir_asm_lower_insn (form inferred from BOR/BXOR pattern)
rep(
    '  case OP_BAND: {\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    andq ");\n'
    '    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '    fprintf(f, ", %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',

    '  case OP_BAND: {\n'
    '    /* CG-IR-017 */\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {\n'
    '      fprintf(f, "    movq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rcx\\n");\n'
    '      fprintf(f, "    andl %%ecx, %%eax\\n");\n'
    '    } else {\n'
    '      fprintf(f, "    andq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rax\\n");\n'
    '    }\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',
    'OP_BAND 32/64 dispatch'
)

# OP_BOR
rep(
    '  case OP_BOR: {\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    orq ");\n'
    '    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '    fprintf(f, ", %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',

    '  case OP_BOR: {\n'
    '    /* CG-IR-017 */\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {\n'
    '      fprintf(f, "    movq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rcx\\n");\n'
    '      fprintf(f, "    orl %%ecx, %%eax\\n");\n'
    '    } else {\n'
    '      fprintf(f, "    orq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rax\\n");\n'
    '    }\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',
    'OP_BOR 32/64 dispatch'
)

# OP_BXOR
rep(
    '  case OP_BXOR: {\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    xorq ");\n'
    '    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '    fprintf(f, ", %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',

    '  case OP_BXOR: {\n'
    '    /* CG-IR-017 */\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {\n'
    '      fprintf(f, "    movq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rcx\\n");\n'
    '      fprintf(f, "    xorl %%ecx, %%eax\\n");\n'
    '    } else {\n'
    '      fprintf(f, "    xorq ");\n'
    '      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
    '      fprintf(f, ", %%rax\\n");\n'
    '    }\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',
    'OP_BXOR 32/64 dispatch'
)

# OP_BNOT
rep(
    '  case OP_BNOT: {\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    fprintf(f, "    notq %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',

    '  case OP_BNOT: {\n'
    '    /* CG-IR-017 */\n'
    '    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
    '    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32)\n'
    '      fprintf(f, "    notl %%eax\\n");\n'
    '    else\n'
    '      fprintf(f, "    notq %%rax\\n");\n'
    '    ir_asm_store_rax_to(ctx, ins->dst);\n'
    '    break;\n'
    '  }',
    'OP_BNOT 32/64 dispatch'
)

# ── Comparisons: cmpl for I32/U32, cmpq for I64/U64
# setX mnemonic unchanged (signed/unsigned distinction is CG-IR-018)
# movzbl replaces movzbq for 32-bit path (matches GCC output)

def patch_cmp(op, setmn):
    global src
    old = (
        f'  case {op}: {{\n'
        f'    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
        f'    fprintf(f, "    cmpq ");\n'
        f'    ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
        f'    fprintf(f, ", %%rax\\n");\n'
        f'    fprintf(f, "    {setmn} %%al\\n");\n'
        f'    fprintf(f, "    movzbq %%al, %%rax\\n");\n'
        f'    ir_asm_store_rax_to(ctx, ins->dst);\n'
        f'    break;\n'
        f'  }}'
    )
    new = (
        f'  case {op}: {{\n'
        f'    /* CG-IR-017 */\n'
        f'    ir_asm_load_to_rax(ctx, ins->src[0]);\n'
        f'    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {{\n'
        f'      fprintf(f, "    movq ");\n'
        f'      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
        f'      fprintf(f, ", %%rcx\\n");\n'
        f'      fprintf(f, "    cmpl %%ecx, %%eax\\n");\n'
        f'      fprintf(f, "    {setmn} %%al\\n");\n'
        f'      fprintf(f, "    movzbl %%al, %%eax\\n");\n'
        f'    }} else {{\n'
        f'      fprintf(f, "    cmpq ");\n'
        f'      ir_asm_emit_src_operand(ctx, ins->src[1]);\n'
        f'      fprintf(f, ", %%rax\\n");\n'
        f'      fprintf(f, "    {setmn} %%al\\n");\n'
        f'      fprintf(f, "    movzbq %%al, %%rax\\n");\n'
        f'    }}\n'
        f'    ir_asm_store_rax_to(ctx, ins->dst);\n'
        f'    break;\n'
        f'  }}'
    )
    rep(old, new, f'{op} 32/64 dispatch')

patch_cmp('OP_LT', 'setl')
patch_cmp('OP_EQ', 'sete')
patch_cmp('OP_NE', 'setne')
patch_cmp('OP_GT', 'setg')
patch_cmp('OP_GE', 'setge')
patch_cmp('OP_LE', 'setle')

# ══════════════════════════════════════════════════════════════
# Write result
# ══════════════════════════════════════════════════════════════
if src == orig:
    print('\nERROR: no changes made — check MISS lines above', file=sys.stderr)
    sys.exit(1)

with open('compiler_passes.c', 'w') as f:
    f.write(src)

print(f'\nDone: {len(changes)}/17 replacements applied.')
if len(changes) < 17:
    print('WARNING: some replacements missed — check MISS lines above', file=sys.stderr)
