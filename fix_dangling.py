import sys, re

with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Fix Pattern 1
p1 = r'(\s*)if \(node->type->size == 4\) if \(!backend_ops\) fprintf\(cc->out, "    cltq\\n"\);\n(\s*)else if'
r1 = r'\1if (node->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\\n"); }\n\2else if'
code = re.sub(p1, r1, code)

p2_v2 = r'(\s*)if \(node->type->size == 4\) if \(!backend_ops\) fprintf\(cc->out, "    movl %%eax, %%eax\\n"\);\n(\s*)else if'
r2_v2 = r'\1if (node->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\\n"); }\n\2else if'
code = re.sub(p2_v2, r2_v2, code)

p3 = r'(\s*)if \(node->lhs && node->lhs->type && is_unsigned_type\(node->lhs->type\)\)\n(\s*)if \(!backend_ops\) fprintf\(cc->out, "    movl %%eax, %%eax\\n"\);\n(\s*)else\n(\s*)if \(!backend_ops\) fprintf\(cc->out, "    cltq\\n"\);'
r3 = r'\1if (node->lhs && node->lhs->type && is_unsigned_type(node->lhs->type)) {\n\2if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\\n");\n\1} else {\n\4if (!backend_ops) fprintf(cc->out, "    cltq\\n");\n\1}'
code = re.sub(p3, r3, code)

with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("Done")
