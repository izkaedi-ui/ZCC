import sys

with open('part4.c', 'r') as f:
    lines = f.readlines()

out = []
i = 0
while i < len(lines):
    line = lines[i]
    if '  case ND_ADD:' in line and out[-1].strip() == 'return;':
        out.extend([
            "  case ND_FADD:\n",
            "  case ND_FSUB:\n",
            "  case ND_FMUL:\n",
            "  case ND_FDIV: {\n",
            "    codegen_expr_checked(cc, node->lhs);\n",
            "    if (backend_ops) {\n",
            "        fprintf(cc->out, \"    push {r0, r1}\\n\");\n",  # thumb push allows r0-r7
            "    } else {\n",
            "        fprintf(cc->out, \"    movsd %%xmm0, -8(%%rsp)\\n\");\n",
            "    }\n",
            "    codegen_expr_checked(cc, node->rhs);\n",
            "    if (backend_ops) {\n",
            "        fprintf(cc->out, \"    mov r2, r0\\n\");\n",  # wait, cortex-m0 allows mov hi,lo? yes, mov rx, ry allows any. Wait, args for double add are r0:r1 (lhs) and r2:r3 (rhs). Softfloat: double a (r0:r1), double b (r2:r3). Or float a (r0), float b (r1).
            "        /* Since type_size(ty_float)==4, soft-float float args are r0 and r1 */\n",
            "        fprintf(cc->out, \"    mov r1, r0\\n\");\n",
            "        fprintf(cc->out, \"    pop {r0}\\n\");\n",
            "        backend_ops->emit_float_binop(cc, node->kind);\n",
            "    } else {\n",
            "        fprintf(cc->out, \"    movsd %%xmm0, %%xmm1\\n\");\n",
            "        fprintf(cc->out, \"    movsd -8(%%rsp), %%xmm0\\n\");\n",
            "        if (node->kind == ND_FADD) fprintf(cc->out, \"    addsd %%xmm1, %%xmm0\\n\");\n",
            "        if (node->kind == ND_FSUB) fprintf(cc->out, \"    subsd %%xmm1, %%xmm0\\n\");\n",
            "        if (node->kind == ND_FMUL) fprintf(cc->out, \"    mulsd %%xmm1, %%xmm0\\n\");\n",
            "        if (node->kind == ND_FDIV) fprintf(cc->out, \"    divsd %%xmm1, %%xmm0\\n\");\n",
            "    }\n",
            "    return;\n",
            "  }\n\n",
            "  case ND_ITOF: {\n",
            "    codegen_expr_checked(cc, node->lhs);\n",
            "    if (backend_ops) {\n",
            "        fprintf(cc->out, \"    bl __aeabi_i2f\\n\");\n",
            "    } else {\n",
            "        fprintf(cc->out, \"    cvtsi2sd %%eax, %%xmm0\\n\");\n",
            "    }\n",
            "    return;\n",
            "  }\n\n",
            "  case ND_FTOI: {\n",
            "    codegen_expr_checked(cc, node->lhs);\n",
            "    if (backend_ops) {\n",
            "        fprintf(cc->out, \"    bl __aeabi_f2iz\\n\");\n",
            "    } else {\n",
            "        fprintf(cc->out, \"    cvttsd2si %%xmm0, %%eax\\n\");\n",
            "    }\n",
            "    return;\n",
            "  }\n\n",
            "  case ND_FCMP: {\n",
            "    codegen_expr_checked(cc, node->lhs);\n",
            "    if (backend_ops) {\n",
            "        fprintf(cc->out, \"    push {r0, r1}\\n\");\n",
            "    } else {\n",
            "        fprintf(cc->out, \"    movsd %%xmm0, -8(%%rsp)\\n\");\n",
            "    }\n",
            "    codegen_expr_checked(cc, node->rhs);\n",
            "    if (backend_ops) {\n",
            "        fprintf(cc->out, \"    mov r1, r0\\n\");\n",
            "        fprintf(cc->out, \"    pop {r0}\\n\");\n",
            "        /* Note: we rely on jb etc after this, but softfloat aeabi_fcmplt returns 1/0 in r0! */\n",
            "    } else {\n",
            "        fprintf(cc->out, \"    ucomisd -8(%%rsp), %%xmm0\\n\");\n",
            "    }\n",
            "    return;\n",
            "  }\n\n"
        ])
    out.append(line)
    i += 1

with open('part4_float.c', 'w') as f:
    f.writelines(out)
