# -*- coding: utf-8 -*-
# fix_cgir007.py - CG-IR-007: OP_CALL alignment uses phantom callee-save count
# The OP_CALL handler computes alignment based on n_csave pushes,
# but in body_only mode those pushes never happened.
# Fix: use 0 for n_csave in body_only mode.
#
# Run: python3 fix_cgir007.py

import sys
import shutil

FILENAME = "compiler_passes.c"

shutil.copy(FILENAME, FILENAME + ".bak_cgir007")
print("Backup: " + FILENAME + ".bak_cgir007")

with open(FILENAME, "r") as f:
    content = f.read()

# Fix the OP_CALL alignment: n_csave should be 0 in body_only mode
old = '            int n_csave = __builtin_popcount(ctx->used_callee_saved_mask);'
new = '            int n_csave = ctx->body_only ? 0 : __builtin_popcount(ctx->used_callee_saved_mask);'

count = content.count(old)
if count == 1:
    content = content.replace(old, new, 1)
    with open(FILENAME, "w") as f:
        f.write(content)
    print("OK: CG-IR-007 alignment fix applied")
elif count == 0:
    print("ERROR: pattern not found")
    sys.exit(1)
else:
    print("WARNING: found %d matches, replacing first only" % count)
    content = content.replace(old, new, 1)
    with open(FILENAME, "w") as f:
        f.write(content)
    print("OK: CG-IR-007 applied (first match)")

print("Rebuild and test.")
