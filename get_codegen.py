import re

def extract(filename):
    with open(filename, 'r') as f: text = f.read()
    expr = re.search(r'^void codegen_expr\(Compiler \*cc, Node \*node\).*?\n\}', text, re.DOTALL | re.MULTILINE)
    stmt = re.search(r'^void codegen_stmt\(Compiler \*cc, Node \*node\).*?\n\}', text, re.DOTALL | re.MULTILINE)
    return expr.group(0) if expr else '', stmt.group(0) if stmt else ''

e1, s1 = extract('part4.c')
e2, s2 = extract('zcc_patched.c')

with open('part4_expr.c', 'w') as f: f.write(e1)
with open('zcc_expr.c', 'w') as f: f.write(e2)
with open('part4_stmt.c', 'w') as f: f.write(s1)
with open('zcc_stmt.c', 'w') as f: f.write(s2)
