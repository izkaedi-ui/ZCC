import sys, re

with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    code = f.read()

clang_head = """
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunknown-attributes"
#pragma clang diagnostic ignored "-Wincompatible-library-redeclaration"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wunsequenced"
#pragma clang diagnostic ignored "-Wint-to-pointer-cast"
#pragma clang diagnostic ignored "-Wpointer-to-int-cast"
#pragma clang diagnostic ignored "-Wenum-compare"
#pragma clang diagnostic ignored "-Wsizeof-pointer-memaccess"
#pragma clang diagnostic ignored "-Wincompatible-pointer-types-discards-qualifiers"
#pragma clang diagnostic ignored "-Wpointer-sign"
#pragma clang diagnostic ignored "-Wint-conversion"
#define _Float128 long double
#define __malloc__
#define __access__(...)
#endif
"""

if '#define _Float128' not in code:
    code = clang_head + code

code = re.sub(r'(\nstatic const char rcsid\[\] = [^;]+;)', r'\n#ifndef __clang__\1\n#endif\n', code)
code = re.sub(r'(\nvoid \*malloc\([^\)]*\);)', r'\n#ifndef __clang__\1\n#endif\n', code)
code = re.sub(r'(\nextern void \*malloc\([^\)]*\);)', r'\n#ifndef __clang__\1\n#endif\n', code)

with open('doom_pp_clean.c', 'w', encoding='utf-8') as f:
    f.write(code)
print('Done patching doom_pp_clean.c')
