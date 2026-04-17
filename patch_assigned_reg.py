import re

with open("part4.c", "r") as f:
    content = f.read()

content = re.sub(
    r'fprintf\(cc->out, "    movq %s, %%rax\\n", ([^)]+)\);',
    r'if (backend_ops) fprintf(cc->out, "    mov r0, %s\\n", \1); else fprintf(cc->out, "    movq %s, %%rax\\n", \1);',
    content
)

content = re.sub(
    r'fprintf\(cc->out, "    movq %%rax, %s\\n", ([^)]+)\);',
    r'if (backend_ops) fprintf(cc->out, "    mov %s, r0\\n", \1); else fprintf(cc->out, "    movq %%rax, %s\\n", \1);',
    content
)

with open("part4.c", "w") as f:
    f.write(content)
