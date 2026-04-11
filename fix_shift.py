path = "compiler_passes.c"
with open(path, "r") as f:
    lines = f.readlines()
for i in [6114, 6130]:
    if i < len(lines) and "n->type" in lines[i] or "ins->type" in lines[i]:
        if "shl" in lines[i]:
            lines[i] = '      fprintf(f, "    shlq %%cl, %%rax\\n");\n'
        elif "shr" in lines[i]:
            lines[i] = '      fprintf(f, "    shrq %%cl, %%rax\\n");\n'
        print(f"Fixed line {i+1}")
with open(path, "w") as f:
    f.writelines(lines)
print("Done")
