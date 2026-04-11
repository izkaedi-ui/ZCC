import subprocess, os
env = {**os.environ, "ZCC_IR_BACKEND": "1"}

c = open("compiler_passes.c").read()
old = '  case ZND_ASSIGN: {\n    if (!node->lhs)\n      return 0;'
if old in c and 'DIAG_ASSIGN' not in c:
    new = '  case ZND_ASSIGN: {\n    fprintf(stderr, "[DIAG_ASSIGN] lhs=%p rhs=%p\\n", (void*)node->lhs, (void*)node->rhs);\n    if (!node->lhs)\n      return 0;'
    c = c.replace(old, new, 1)
    open("compiler_passes.c", "w").write(c)
    print("Patched")
    r = subprocess.run(["make", "clean"], capture_output=True)
    r = subprocess.run(["make", "selfhost"], capture_output=True, text=True)
    if "SELF-HOST VERIFIED" in r.stdout:
        print("Rebuild OK")
    else:
        print("Rebuild FAILED")

r = subprocess.run(["./zcc2", "/tmp/t_loop.c", "-o", "/tmp/t_loop_ir.s"],
                   capture_output=True, text=True, env=env)
for l in r.stderr.split("\n"):
    if "DIAG" in l:
        print(l)
