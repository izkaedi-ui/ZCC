import re

with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Make sure we only have the raw code (undo my previous anim_t regex)
code = re.sub(r'#ifndef __clang__\n(typedef\s+struct\s*\{[^}]*\}\s*anim_t;)\n#endif', r'\1', code)

count = 0
def repl_anim(m):
    global count
    count += 1
    if count == 2:
        return '#define anim_t anim_finale_t\n' + m.group(0).replace('anim_t;', 'anim_finale_t;')
    return m.group(0)

code = re.sub(r'typedef\s+struct\s*\{[^}]*\}\s*anim_t;', repl_anim, code)

if 'void *malloc(unsigned long);' not in code:
    code = code.replace('#ifdef __clang__\n', '#ifdef __clang__\nvoid *malloc(unsigned long);\n#define open open_dr\nint open_dr(const char*, int, ...);\n', 1)

code = code.replace('case open_dr:', 'case open:')

with open('doom_pp_clean.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("Patch applied.")
