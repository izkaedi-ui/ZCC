import sys
with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Fix SUB
code = code.replace(
    'fprintf(cc->out, "    subq %%r11, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_SUB);\n      else fprintf(cc->out, "    subq %%r11, %%rax\\n");'
)
# Fix MUL
code = code.replace(
    'fprintf(cc->out, "    imulq %%r11, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_MUL);\n      else fprintf(cc->out, "    imulq %%r11, %%rax\\n");'
)
# Fix BAND
code = code.replace(
    'fprintf(cc->out, "    andq %%r11, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_BAND);\n      else fprintf(cc->out, "    andq %%r11, %%rax\\n");'
)
# Fix BOR
code = code.replace(
    'fprintf(cc->out, "    orq %%r11, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_BOR);\n      else fprintf(cc->out, "    orq %%r11, %%rax\\n");'
)
# Fix BXOR
code = code.replace(
    'fprintf(cc->out, "    xorq %%r11, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_BXOR);\n      else fprintf(cc->out, "    xorq %%r11, %%rax\\n");'
)
# Fix SHL
code = code.replace(
    'fprintf(cc->out, "    movq %%r11, %%rcx\\n    shlq %%cl, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHL);\n      else fprintf(cc->out, "    movq %%r11, %%rcx\\n    shlq %%cl, %%rax\\n");'
)
# Fix SHR (sarq)
code = code.replace(
    'fprintf(cc->out, "    movq %%r11, %%rcx\\n    sarq %%cl, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHR);\n      else fprintf(cc->out, "    movq %%r11, %%rcx\\n    sarq %%cl, %%rax\\n");'
)
# Fix SHR (shrq)
code = code.replace(
    'fprintf(cc->out, "    movq %%r11, %%rcx\\n    shrq %%cl, %%rax\\n");',
    'if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHR);\n      else fprintf(cc->out, "    movq %%r11, %%rcx\\n    shrq %%cl, %%rax\\n");'
)

# Fix pointer arithmetic
code = code.replace(
    'fprintf(cc->out, "    addq $%d, %s\\n", esz, reg);',
    'if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\\n", esz); if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    adds r0, r0, r3\\n"); else fprintf(cc->out, "    adds r1, r1, r3\\n"); } else fprintf(cc->out, "    addq $%d, %s\\n", esz, reg);'
)
code = code.replace(
    'fprintf(cc->out, "    subq $%d, %s\\n", esz, reg);',
    'if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\\n", esz); if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    subs r0, r0, r3\\n"); else fprintf(cc->out, "    subs r1, r1, r3\\n"); } else fprintf(cc->out, "    subq $%d, %s\\n", esz, reg);'
)
code = code.replace(
    'fprintf(cc->out, "    addq $%d, %%rax\\n", esz);',
    'if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\\n    adds r0, r0, r3\\n", esz); } else fprintf(cc->out, "    addq $%d, %%rax\\n", esz);'
)
code = code.replace(
    'fprintf(cc->out, "    subq $%d, %%rax\\n", esz);',
    'if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\\n    subs r0, r0, r3\\n", esz); } else fprintf(cc->out, "    subq $%d, %%rax\\n", esz);'
)

# Fix literal add/sub inside inc/dec pointer loops
code = code.replace(
    'fprintf(cc->out, "    subq $1, %s\\n", reg);',
    'if (backend_ops) { if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    subs r0, r0, #1\\n"); else fprintf(cc->out, "    subs r1, r1, #1\\n"); } else fprintf(cc->out, "    subq $1, %s\\n", reg);'
)
code = code.replace(
    'fprintf(cc->out, "    addq $1, %s\\n", reg);',
    'if (backend_ops) { if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    adds r0, r0, #1\\n"); else fprintf(cc->out, "    adds r1, r1, #1\\n"); } else fprintf(cc->out, "    addq $1, %s\\n", reg);'
)
code = code.replace(
    'fprintf(cc->out, "    subq $1, %%rax\\n");',
    'if (backend_ops) { fprintf(cc->out, "    subs r0, r0, #1\\n"); } else fprintf(cc->out, "    subq $1, %%rax\\n");'
)
code = code.replace(
    'fprintf(cc->out, "    addq $1, %%rax\\n");',
    'if (backend_ops) { fprintf(cc->out, "    adds r0, r0, #1\\n"); } else fprintf(cc->out, "    addq $1, %%rax\\n");'
)

with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)
