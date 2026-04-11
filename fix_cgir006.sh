#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

cp compiler_passes.c compiler_passes.c.bak_cgir006

python3 << 'PYEOF'
with open("compiler_passes.c", "r") as f:
    content = f.read()

# Find where ir_asm_emit_function_body is called in zcc_run_passes_emit_body_pgo
# Add frame extension BEFORE the call
old = """    ir_asm_emit_function_body(&ctx);

    fprintf(stderr, "[ZCC-IR] fn=%s  emitted from IR (PGO layout) %u blocks\\n", func_name ? func_name : "?", (unsigned)fn->n_blocks);
    int n_blocks = (int)(uint32_t)fn->n_blocks;
    free(result);
    zcc_ir_free(fn);
    return n_blocks;
}"""

new = """    /* CG-IR-006 fix: extend stack frame if IR needs more slots than AST allocated.
     * ir_asm_slot(r) = -8*(r+2), so max slot depth = 8*(n_regs+2).
     * AST prologue set subq $stack_size — if IR exceeds it, emit secondary subq.
     * The AST epilogue's movq %rbp,%rsp undoes all adjustments automatically. */
    {
        int ir_stack_needed = 8 * ((int)fn->n_regs + 2) + 64; /* +64 for alloca headroom */
        if (ir_stack_needed > stack_size) {
            int extra = ((ir_stack_needed - stack_size) + 15) & ~15;
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
    with open("compiler_passes.c", "w") as f:
        f.write(content)
    print("OK: CG-IR-006 frame extension added")
else:
    print("WARNING: pattern not found. Manual edit needed.")
    print("Add frame extension before ir_asm_emit_function_body() in zcc_run_passes_emit_body_pgo")
PYEOF

echo ''
echo '=== Rebuild and test ==='
make clean 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc zcc.c
./zcc zcc.c -o zcc2.s 2>/dev/null
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc2 zcc2.s compiler_passes.c compiler_passes_ir.c -lm
echo 'Built zcc2'

ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir.s 2>/tmp/stage4.txt
gcc -O0 -w -o zcc3_ir zcc3_ir.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || gcc -O0 -w -o zcc3_ir zcc3_ir.s

echo ''
echo '=== Test trivial ==='
echo 'int main(){return 0;}' > /tmp/t1.c
./zcc3_ir /tmp/t1.c -o /tmp/t1.s 2>/tmp/t1.err
RC=$?
L=0
test -f /tmp/t1.s && L=$(wc -l < /tmp/t1.s)
echo "trivial: rc=$RC lines=$L"

echo ''
echo '=== Test self-compile ==='
./zcc3_ir zcc_pp.c -o /tmp/check.s 2>/tmp/check_err.txt
RC=$?
L=0
test -f /tmp/check.s && L=$(wc -l < /tmp/check.s)
echo "self-compile: rc=$RC lines=$L"
head -3 /tmp/check_err.txt

echo DONE
