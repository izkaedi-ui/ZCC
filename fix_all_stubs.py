#!/usr/bin/env python3
"""CG-IR-012b: Replace ALL dead accessor stubs in compiler_passes.c
with extern declarations so the linker resolves to the real
implementations in zcc.c (part1.c)."""
import re

path = "/mnt/h/__DOWNLOADS/selforglinux/compiler_passes.c"
src = open(path).read()

# Find the block from __weak definition through the CG-IR-012 comment
# and replace everything with clean extern declarations
old_start = '#ifndef __weak'
old_end = 'extern int node_ptr_elem_size(struct Node *n);'

start_idx = src.find(old_start)
end_idx = src.find(old_end)

if start_idx < 0:
    print("ERROR: cannot find start marker '#ifndef __weak'")
    raise SystemExit(1)

if end_idx < 0:
    # Try alternate end marker
    old_end = 'extern int node_num_args(struct Node *n);'
    end_idx = src.find(old_end)
    if end_idx < 0:
        print("ERROR: cannot find end marker")
        raise SystemExit(1)

end_idx = end_idx + len(old_end)

new_block = '''/* CG-IR-012b: ALL Node/Type accessor stubs replaced with extern declarations.
 * The real implementations live in zcc.c (part1.c). The stubs here returned
 * dummy values (0/NULL/""), causing the AST-to-IR conversion to lose all
 * structural information — func names, args, children, offsets, etc. */
extern int node_kind(struct Node *n);
extern long long node_int_val(struct Node *n);
extern int node_str_id(struct Node *n);
extern void node_name(struct Node *n, char *buf, unsigned len);
extern int node_is_global(struct Node *n);
extern int node_is_array(struct Node *n);
extern int node_is_func(struct Node *n);
extern struct Node *node_lhs(struct Node *n);
extern struct Node *node_rhs(struct Node *n);
extern struct Node *node_cond(struct Node *n);
extern struct Node *node_then_body(struct Node *n);
extern struct Node *node_else_body(struct Node *n);
extern struct Node *node_body(struct Node *n);
extern struct Node *node_init(struct Node *n);
extern struct Node *node_inc(struct Node *n);
extern struct Node **node_cases(struct Node *n);
extern int node_num_cases(struct Node *n);
extern struct Node *node_default_case(struct Node *n);
extern long long node_case_val(struct Node *n);
extern struct Node *node_case_body(struct Node *n);
extern int node_member_offset(struct Node *n);
extern int node_member_size(struct Node *n);
extern int node_line_no(struct Node *n);
extern int node_compound_op(struct Node *n);
extern struct Node **node_stmts(struct Node *n);
extern int node_num_stmts(struct Node *n);
extern const char *node_func_name(struct Node *n);
extern struct Node *node_arg(struct Node *n, int i);
extern int node_num_args(struct Node *n);
extern int node_ptr_elem_size(struct Node *n);
extern int node_lhs_ptr_size(struct Node *n);
extern int node_rhs_ptr_size(struct Node *n);'''

src = src[:start_idx] + new_block + src[end_idx:]
open(path, 'w').write(src)
print("OK: CG-IR-012b — all {} stubs replaced with extern declarations".format(
    new_block.count('extern')))
