import sys, re

with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Fix multiline rcsid:
# Pattern: \nstatic const char(\s*)rcsid\[\] = [^;]+;
code = re.sub(r'(\nstatic const char\s*rcsid\[\] = [^;]+;)', r'\n#ifndef __clang__\1\n#endif\n', code)

# Fix the broken __malloc__ erasures I added:
code = code.replace('#define __malloc__\n', '')
code = code.replace('#define __access__(...)\n', '')

# What about conflicting types for malloc on line 7042?
# Let's find 'void *malloc(' and wrap it.
code = re.sub(r'([\n\s]*void \*malloc\([^\)]*\);)', r'\n#ifndef __clang__\1\n#endif\n', code)

with open('doom_pp_clean.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("Done fixing rcsid and macros")
