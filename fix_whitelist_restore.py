#!/usr/bin/env python3
import re

path = "/mnt/h/__DOWNLOADS/selforglinux/part4.c"
src = open(path).read()

new_func = (
    'static int ir_whitelisted(const char *name) {\n'
    '    if (!name) return 0;\n'
    '    static const char *blacklist[] = {\n'
    '        "main", "read_file", "init_compiler", "cc_alloc", "type_new", NULL};\n'
    '    int i;\n'
    '    for (i = 0; blacklist[i]; i++) {\n'
    '        if (strcmp(name, blacklist[i]) == 0) {\n'
    '            fprintf(stderr, "[ZCC-BLACKLIST] HIT (skipping IR): %s\\n", name);\n'
    '            return 0;\n'
    '        }\n'
    '    }\n'
    '    return 1;\n'
    '}'
)

src = re.sub(
    r'static int ir_whitelisted\(const char \*name\) \{.*?\n\}',
    new_func, src, flags=re.DOTALL)

open(path, 'w').write(src)
print("OK: ir_whitelisted fixed")
