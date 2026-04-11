import sys
path = "compiler_passes.c"
with open(path, "r") as f:
    lines = f.readlines()

# Find line 6787: ir_asm_emit_function_body(&ctx);
target = None
for i, line in enumerate(lines):
    if "ir_asm_emit_function_body(&ctx);" in line and i > 6700:
        target = i
        break

if target is None:
    print("ERROR: ir_asm_emit_function_body not found after line 6700")
    sys.exit(1)

print(f"Found body emit at line {target+1}")

save_lines = [
    "  /* CG-IR-016: Save callee-saved regs before IR body */\n",
    '  fprintf(out, "    movq %%%%rbx, %d(%%%%rbp)\\n", ctx.slot_base - 8);\n',
    '  fprintf(out, "    movq %%%%r12, %d(%%%%rbp)\\n", ctx.slot_base - 16);\n',
    '  fprintf(out, "    movq %%%%r13, %d(%%%%rbp)\\n", ctx.slot_base - 24);\n',
    '  fprintf(out, "    movq %%%%r14, %d(%%%%rbp)\\n", ctx.slot_base - 32);\n',
    '  fprintf(out, "    movq %%%%r15, %d(%%%%rbp)\\n", ctx.slot_base - 40);\n',
]

restore_lines = [
    "  /* CG-IR-016: Restore callee-saved regs after IR body */\n",
    '  fprintf(out, "    movq %d(%%%%rbp), %%%%rbx\\n", ctx.slot_base - 8);\n',
    '  fprintf(out, "    movq %d(%%%%rbp), %%%%r12\\n", ctx.slot_base - 16);\n',
    '  fprintf(out, "    movq %d(%%%%rbp), %%%%r13\\n", ctx.slot_base - 24);\n',
    '  fprintf(out, "    movq %d(%%%%rbp), %%%%r14\\n", ctx.slot_base - 32);\n',
    '  fprintf(out, "    movq %d(%%%%rbp), %%%%r15\\n", ctx.slot_base - 40);\n',
]

# Insert save before, restore after
new_lines = lines[:target] + save_lines + [lines[target]] + restore_lines + lines[target+1:]

with open(path, "w") as f:
    f.writelines(new_lines)
print("CG-IR-016 save/restore inserted.")
