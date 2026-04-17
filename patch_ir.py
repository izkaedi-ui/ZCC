import re

def patch_file():
    with open('part4.c', 'r') as f:
        src = f.read()

    # Create patched copy
    with open('part4.c.orig', 'w') as f:
        f.write(src)
        
    replacements = [
        # Binary Ops
        (r'codegen_expr_checked\(cc, node->lhs\);\n(\s*)push_reg\(cc, " rax\\);\n(\s*)codegen_expr_checked\(cc,