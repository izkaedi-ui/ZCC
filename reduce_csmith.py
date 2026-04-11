import re
with open('failures/fail_9.c', 'r') as f:
    content = f.read()

funcs_to_stub = ['func_11', 'func_21', 'func_33', 'func_37', 'func_43', 'func_45', 'func_61', 'func_66']

for f_name in funcs_to_stub:
    pattern = re.compile(r'(static\s+[^)]+\s+' + f_name + r'\s*\([^)]*\))\s*\{.*?\n\}', re.DOTALL)
    content = pattern.sub(r'\1 { return 0; }', content)

with open('failures/fail_9_min.c', 'w') as f:
    f.write(content)
