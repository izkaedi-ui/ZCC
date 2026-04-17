import sys, re

with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    code = f.read()

head = """#ifdef __clang__
#pragma clang diagnostic ignored "-Wdeprecated-non-prototype"
#pragma clang diagnostic ignored "-Wstrict-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"
#pragma clang diagnostic ignored "-Wvisibility"
#endif
"""

if 'Wdeprecated-non-prototype' not in code:
    code = head + code
    with open('doom_pp_clean.c', 'w', encoding='utf-8') as f:
        f.write(code)
    print("Added warning suppressions.")
