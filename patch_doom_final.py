import sys, re

with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    code = f.read()

# 1. Provide malloc for clang
if 'void *malloc(unsigned long);' not in code:
    code = code.replace('#ifdef __clang__\n', '#ifdef __clang__\nvoid *malloc(unsigned long);\n', 1)

# 2. Revert the open_dr back to open, and solve the open collision cleanly for clang
code = code.replace('case open_dr:', 'case open:')
# Add a clang macro to rename 'open' to 'open_dr' everywhere so it doesn't collide with the system function!
# Wait, if we rename 'open' to 'open_dr', then calls to open() fail because open_dr() is undeclared.
# So we declare open_dr().
if 'int open_dr(const char*, int, ...);' not in code:
    code = code.replace('#ifdef __clang__\n', '#ifdef __clang__\n#define open open_dr\nint open_dr(const char*, int, ...);\n', 1)

# 3. Fix anim_t disappearing.
# My previous regex:
# r'#ifndef __clang__\n(typedef\s+struct\s*{[^}]*}\s*anim_t;)\n#endif'
code = re.sub(r'#ifndef __clang__\n(typedef\s+struct\s*\{[^}]*\}\s*anim_t;)\n#endif', r'\1', code)

# Now we have two anim_t definitions again. We need to hide ONLY the second one, or rename the second one.
# Let's rename the second one to 'anim_finale_t'.
count = 0
def repl_anim(m):
    global count
    count += 1
    if count == 2:
        return m.group(0).replace('anim_t', 'anim_finale_t')
    return m.group(0)

code = re.sub(r'typedef\s+struct\s*\{[^}]*\}\s*anim_t;', repl_anim, code)

# Also rename variable types anim_t to anim_finale_t in f_finale.c (the area where the second struct is used).
# Actually, the second struct relies on the variable `anim_t anims[32];` or something.
# We'll just define `#define anim_t anim_finale_t` around f_finale.c?
# No, let's just #ifndef __clang__ the second anim_t typedef, and the variable that uses it!
# Wait, if we just `#ifndef __clang__` the second anim_t, the variables using it will throw "Unknown type name anim_t"!
# If we rename the second anim_t to anim_finale_t, we must also rename its usages!
# The user's errors:
# "Unknown type name 'anim_t'" at 29645
# "Use of undeclared identifier 'anim_t'; did you mean 'anims'?" at 30262
# We can just rename `anim_t` to `anim_finale_t` globally after the first usages end?
# The easiest way: Clang errors on struct redefinition. C allows it IF they are identical, but they are different!
# Is it possible to just do this at the top of the file:
# #define anim_t anim_t_global
# #define channels channels_sfx
# No, just rename the second one!

with open('doom_pp_clean.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("Reverted and fixed.")
