import subprocess, os
env = {**os.environ, "ZCC_IR_BACKEND": "1"}

# Patch: add debug print to latch block
c = open("compiler_passes.c").read()
marker = 'if (node->inc) {\n      (void)zcc_lower_expr('
if marker in c and 'DIAG_INC' not in c:
    patch = 'fprintf(stderr, "[DIAG_INC] node->inc=%p kind=%d\\n", (void*)node->inc, node->inc ? node->inc->kind : -1);\n    if (node->inc) {\n      (void)zcc_lower_expr('
    c = c.replace(marker, patch, 1)
    open("compiler_passes.c", "w").write(c)
    print("Patched")

    # Rebuild
    r = subprocess.run(["make", "clean"], capture_output=True)
    r = subprocess.run(["make", "selfhost"], capture_output=True, text=True)
    if "SELF-HOST VERIFIED" in r.stdout:
        print("Rebuild OK")
    else:
        print("Rebuild FAILED")
        print(r.stdout[-200:])

# Run with IR
r = subprocess.run(["./zcc2", "/tmp/t_loop.c", "-o", "/tmp/t_loop_ir.s"],
                   capture_output=True, text=True, env=env)
for l in r.stderr.split("\n"):
    if "DIAG_INC" in l or "latch" in l.lower():
        print(l)
