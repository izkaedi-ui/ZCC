import re

with open('zcc_patched.c', 'r') as f:
    text = f.read()

m = re.search(r'void codegen_expr\(Compiler \*cc, Node \*node\).*?\n\}', text, re.DOTALL)
if m:
    with open('patched_codegen_expr.c', 'w') as out:
        out.write(m.group(0))

m2 = re.search(r'void codegen_stmt\(Compiler \*cc, Node \*node\).*?\n\}', text, re.DOTALL)
if m2:
    with open('patched_codegen_stmt.c', 'w') as out:
        out.write(m2.group(0))

