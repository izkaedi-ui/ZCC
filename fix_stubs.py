#!/usr/bin/env python3
"""CG-IR-012: Remove dead accessor stubs from compiler_passes.c.

compiler_passes.c has stub implementations of node_func_name, node_arg,
node_num_args, and node_ptr_elem_size that always return empty/zero/null.
These shadow the REAL implementations in zcc.c (part1.c), causing every
ND_CALL to be converted with empty func_name and 0 args → indirect call
through uninitialized vreg 0 → RIP=0x1 crash.

Fix: Replace stubs with extern declarations so the linker resolves to
the real implementations in zcc.c.
"""
import sys

path = "/mnt/h/__DOWNLOADS/selforglinux/compiler_passes.c"
src = open(path).read()

# The exact stub block to replace
old_stubs = '''const char *node_func_name(struct Node *n) {
  (void)n;
  return "";
}
struct Node *node_arg(struct Node *n, int i) {
  (void)n;
  (void)i;
  return NULL;
}
int node_num_args(struct Node *n) {
  (void)n;
  return 0;
}
int node_ptr_elem_size(struct Node *n) {
  (void)n;
  return 0;
}'''

new_decls = '''/* CG-IR-012: These accessors are implemented in zcc.c (part1.c).
 * The stubs that were here returned empty/zero/null, causing all
 * ND_CALL conversions to lose func_name and args → crash.
 * Now resolved by linker to the real implementations. */
extern const char *node_func_name(struct Node *n);
extern struct Node *node_arg(struct Node *n, int i);
extern int node_num_args(struct Node *n);
extern int node_ptr_elem_size(struct Node *n);'''

if "CG-IR-012" in src:
    print("CG-IR-012 already applied")
    sys.exit(0)

if old_stubs not in src:
    print("ERROR: Could not find exact stub block")
    print("Searching for individual stubs...")
    for name in ["node_func_name", "node_arg", "node_num_args", "node_ptr_elem_size"]:
        import re
        m = re.search(r'^.*' + name + r'.*\{.*?\n\}', src, re.MULTILINE | re.DOTALL)
        if m:
            print(f"  Found {name} at offset {m.start()}")
        else:
            print(f"  NOT FOUND: {name}")
    sys.exit(1)

src = src.replace(old_stubs, new_decls)
open(path, 'w').write(src)
print("OK: CG-IR-012 applied — dead stubs replaced with extern declarations")
