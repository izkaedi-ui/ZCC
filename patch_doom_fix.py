import sys, re

with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    code = f.read()


# 1. Fix 'open' in case open:
code = code.replace('case open:', 'case vldoor_open:')

# 2. Re-erase the malloc conflicting typedefs. I'll just change "void *malloc();" to "#ifndef __clang__ void *malloc(); #endif" directly where it occurs.
code = re.sub(r'([\n\s]*void\s*\*\s*malloc\w*\([^;]*\);)', r'\n#ifndef __clang__\1\n#endif\n', code)
code = re.sub(r'([\n\s]*extern\s*void\s*\*\s*malloc\w*\([^;]*\);)', r'\n#ifndef __clang__\1\n#endif\n', code)

# 3. Rename second anims and time variables in f_finale.c (the bottom of the amalgamation)
# I can simply do this:
# Because time is used everywhere in the finale, and anims is used everywhere in the finale,
# And this file is already preprocessed, what if I just use a macro for Clang?
# Yes! Right before f_finale.c begins (around where I see anim_finale_t), I can add `#define anims anims_finale` and `#define time finale_time` !
# Where is anim_finale_t defined?
# Wait, I already renamed anim_t to anim_finale_t! I can just find `anim_finale_t` and inserts the macros under it!

def insert_macros(m):
    return m.group(0) + '\n#ifdef __clang__\n#define anims anims_finale\n#define time finale_time\n#endif\n'

code = re.sub(r'(typedef\s+struct\s*\{[^}]*\}\s*anim_finale_t;)', insert_macros, code)

# 4. Remove my previous "open_dr" macro which threw an error since open didn't match case open_dr
code = code.replace('#define open open_dr\nint open_dr(const char*, int, ...);\n', '')

with open('doom_pp_clean.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("Final adjustments applied!")
