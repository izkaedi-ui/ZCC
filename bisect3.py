#!/usr/bin/env python3
"""Bisect the second half (functions 88-175) where the bug lives."""
import subprocess, sys, os, re

BUILD = "/mnt/h/__DOWNLOADS/selforglinux"
PART4 = os.path.join(BUILD, "part4.c")
BASE_BL = ["main", "read_file", "init_compiler", "cc_alloc", "type_new"]

# The second half from the original 176-function list
SECOND_HALF = [
    "parse_struct_or_union", "parse_enum_def", "parse_type", "parse_declarator",
    "parse_primary", "parse_postfix", "parse_unary", "parse_mul", "parse_add",
    "parse_shift", "parse_relational", "parse_equality", "parse_bitand",
    "parse_bitxor", "parse_bitor", "parse_logand", "parse_logor",
    "parse_ternary", "parse_assign", "parse_expr", "parse_stmt",
    "parse_func_def", "parse_program",
    "ZCC_IR_INIT", "ZCC_IR_FUNC_BEGIN", "ZCC_IR_FUNC_END", "ZCC_IR_FLUSH",
    "ZCC_EMIT_BINARY", "ZCC_EMIT_UNARY", "ZCC_EMIT_CONST", "ZCC_EMIT_ALLOCA",
    "ZCC_EMIT_STORE", "ZCC_EMIT_LOAD", "ZCC_EMIT_LABEL", "ZCC_EMIT_BR",
    "ZCC_EMIT_BR_IF", "ZCC_EMIT_RET", "ZCC_EMIT_CALL", "ZCC_EMIT_ARG",
    "ir_bridge_reset", "ir_bridge_fresh_tmp", "ir_save_result", "ir_map_type",
    "ir_var_name", "ir_bridge_func_begin", "ir_bridge_func_end", "ir_map_binop",
    "ir_emit_binary_op", "ir_emit_var_load",
    "push_reg", "pop_reg", "new_label", "is_power_of_2_val", "log2_of",
    "emit_label_fmt", "ptr_in_fault_range",
    "codegen_load", "codegen_store", "bad_node_cutoff", "is_bad_ptr",
    "validate_node", "validate_type", "guard_node",
    "codegen_expr_checked", "codegen_addr_checked", "codegen_addr",
    "ptr_elem_size", "codegen_expr", "codegen_stmt",
    "get_callee_reg", "ra_add_local", "compute_liveness", "allocate_registers",
    "ir_whitelisted", "codegen_func", "emit_global_var", "emit_strings",
    "fold_constants", "codegen_program", "node_ptr_elem_size", "peephole_optimize",
    "ir_op_name", "ir_type_name", "ir_type_bytes", "ir_type_unsigned",
    "ir_op_is_terminator", "ir_node_alloc", "ir_append", "safe_copy",
    "ir_emit", "ir_fresh_tmp", "ir_fresh_label",
    "ir_module_create", "ir_func_create", "ir_module_free",
    "emit_field", "ir_func_emit_text", "ir_module_emit_text",
    "get_or_create_var", "load_address", "ir_module_lower_x86",
]

def set_blacklist(names):
    src = open(PART4).read()
    entries = ', '.join('"{}"'.format(n) for n in names)
    new_bl = 'static const char *blacklist[] = {{\n        {}, NULL}};'.format(entries)
    src = re.sub(r'static const char \*blacklist\[\] = \{[^}]+\};', new_bl, src)
    open(PART4, 'w').write(src)
    subprocess.run(["sh", "-c",
        "cd {} && rm -f zcc.c && make zcc.c zcc 2>/dev/null".format(BUILD)],
        capture_output=True, timeout=60)

def test():
    r = subprocess.run(["sh", "-c",
        "cd {} && make clean 2>/dev/null && make zcc2 2>/dev/null".format(BUILD)],
        capture_output=True, text=True, timeout=120)
    if r.returncode != 0:
        print("  Build failed")
        return False
    r2 = subprocess.run(["sh", "-c",
        'echo "int main() {{ return 42; }}" > /tmp/_t.c && '
        "cd {} && timeout 10 ./zcc2 /tmp/_t.c -o /tmp/_t.s 2>/dev/null".format(BUILD)],
        capture_output=True, text=True, timeout=30)
    return r2.returncode == 0

cmd = sys.argv[1] if len(sys.argv) > 1 else "help"

if cmd == "all":
    # Blacklist ALL second-half functions
    bl = list(set(BASE_BL + SECOND_HALF))
    print("Blacklisting all {} second-half functions...".format(len(SECOND_HALF)))
    set_blacklist(bl)
    ok = test()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    set_blacklist(BASE_BL)
    print("Restored base blacklist.")

elif cmd == "q1":
    # Blacklist first quarter of second half
    q = len(SECOND_HALF) // 4
    chunk = SECOND_HALF[:q]
    bl = list(set(BASE_BL + chunk))
    print("Blacklisting Q1 ({} fns): {}...".format(len(chunk), chunk[:5]))
    set_blacklist(bl)
    ok = test()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    set_blacklist(BASE_BL)

elif cmd == "q2":
    q = len(SECOND_HALF) // 4
    chunk = SECOND_HALF[q:2*q]
    bl = list(set(BASE_BL + chunk))
    print("Blacklisting Q2 ({} fns): {}...".format(len(chunk), chunk[:5]))
    set_blacklist(bl)
    ok = test()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    set_blacklist(BASE_BL)

elif cmd == "q3":
    q = len(SECOND_HALF) // 4
    chunk = SECOND_HALF[2*q:3*q]
    bl = list(set(BASE_BL + chunk))
    print("Blacklisting Q3 ({} fns): {}...".format(len(chunk), chunk[:5]))
    set_blacklist(bl)
    ok = test()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    set_blacklist(BASE_BL)

elif cmd == "q4":
    q = len(SECOND_HALF) // 4
    chunk = SECOND_HALF[3*q:]
    bl = list(set(BASE_BL + chunk))
    print("Blacklisting Q4 ({} fns): {}...".format(len(chunk), chunk[:5]))
    set_blacklist(bl)
    ok = test()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    set_blacklist(BASE_BL)

elif cmd == "test":
    # Test specific functions: bisect3.py test fn1,fn2,fn3
    extra = sys.argv[2].split(",") if len(sys.argv) > 2 else []
    bl = list(set(BASE_BL + extra))
    print("Blacklisting {} extra functions...".format(len(extra)))
    set_blacklist(bl)
    ok = test()
    print("Result: {}".format("PASS" if ok else "CRASH"))
    set_blacklist(BASE_BL)

else:
    print("Usage:")
    print("  bisect3.py all    — blacklist all second-half, verify PASS")
    print("  bisect3.py q1     — test quarter 1 (parsers)")
    print("  bisect3.py q2     — test quarter 2 (IR emit)")
    print("  bisect3.py q3     — test quarter 3 (codegen helpers)")
    print("  bisect3.py q4     — test quarter 4 (IR infrastructure)")
    print("  bisect3.py test f1,f2  — test specific functions")
