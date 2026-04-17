import sys, re

with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

p1 = r'(\s*)if \(([^)]+type->size == 4)\) if \(!backend_ops\) fprintf\(cc->out, "    cltq\\n"\);\n(\s*)else if'
r1 = r'\1if (\2) { if (!backend_ops) fprintf(cc->out, "    cltq\\n"); }\n\3else if'
code = re.sub(p1, r1, code)

p2 = r'(\s*)if \(([^)]+type->size == 4)\) if \(!backend_ops\) fprintf\(cc->out, "    movl %%eax, %%eax\\n"\);\n(\s*)else if'
r2 = r'\1if (\2) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\\n"); }\n\3else if'
code = re.sub(p2, r2, code)

with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("Done part2")
