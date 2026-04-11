# -*- coding: utf-8 -*-
# fix_cgir006.py - CG-IR-006: Stack frame underflow in body-only IR emission
# Run: python3 fix_cgir006.py
# In: compiler_passes.c (same directory)

import sys
import shutil

FILENAME = "compiler_passes.c"

shutil.copy(FILENAME, FILENAME + ".bak_cgir006")
print("Backup saved to " + FILENAME + ".bak_cgir006")

with open(FILENAME, "r") as f:
    content = f.read()

# The pattern to find in zcc_run_passes_emit_body_pgo:
old = '    ir_asm_emit_function_body(&ctx);\n\n    fprintf(stderr, "[ZCC-IR] fn=%s  emitted from IR (PGO layout) %u blocks\\n", func_name ? func_name : "?", (unsigned)fn->n_blocks);\n    int n_blocks = (int)(uint32_t)fn->n_blocks;\n    free(result);\n    zcc_ir_free(fn);\n    return n_blocks;\n}'

new = """    /* CG-IR-006: extend stack if IR needs more slots than AST allocated.
     * ir_asm_slot(r) = -8*(r+2), so max depth = 8*(n_regs+2).
     * AST epilogue movq %rbp,%rsp undoes this automatically. */
    {
        int ir_need = 8 * ((int)fn->n_regs + 2) + 64;
        if (ir_need > stack_size) {
            int extra = ((ir_need - stack_size) + 15) & ~15;
            fprintf(out, "    subq $%d, %%rsp\\n", extra);
        }
    }

    ir_asm_emit_function_body(&ctx);

    fprintf(stderr, "[ZCC-IR] fn=%s  emitted from IR (PGO layout) %u blocks\\n", func_name ? func_name : "?", (unsigned)fn->n_blocks);
    int n_blocks = (int)(uint32_t)fn->n_blocks;
    free(result);
    zcc_ir_free(fn);
    return n_blocks;
}"""

if old in content:
    content = content.replace(old, new, 1)
    with open(FILENAME, "w") as f:
        f.write(content)
    print("OK: CG-IR-006 frame extension patched")
else:
    print("Pattern not found. Trying alternate search...")
    # Try to find just the key line
    key = "    ir_asm_emit_function_body(&ctx);\n\n    fprintf(stderr"
    if key in content:
        print("Found key pattern but full match failed.")
        print("Showing context around match:")
        idx = content.index(key)
        print(repr(content[idx:idx+300]))
    else:
        print("Key line not found either. Check file manually.")
    sys.exit(1)

# Also need to un-suppress stack_size in the function signature
# The function has (void)stack_size; which discards it
old2 = "    (void)stack_size;\n    Function *fn = zcc_ast_to_ir(body_ast, func_name);"
new2 = "    Function *fn = zcc_ast_to_ir(body_ast, func_name);"

count = content.count(old2)
if count == 1:
    content = content.replace(old2, new2, 1)
    with open(FILENAME, "w") as f:
        f.write(content)
    print("OK: Removed (void)stack_size suppression")
elif count == 0:
    # Maybe already removed or different format
    print("NOTE: (void)stack_size pattern not found, may already be active")
else:
    print("WARNING: multiple matches for (void)stack_size")

print("Done. Rebuild with: make clean && make selfhost && bash verify_ir_backend.sh")
