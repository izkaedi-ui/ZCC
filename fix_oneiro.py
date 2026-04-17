with open("/mnt/h/__DOWNLOADS/selforglinux/zcc_oneirogenesis.py", "r") as f:
    t = f.read()

old = 'PASSES = ["compiler_passes.c", "compiler_passes_ir.c"]'
new = 'PASSES = ["compiler_passes.c", "compiler_passes_ir.c", "ir_pass_manager.c"]'
t = t.replace(old, new)

with open("/mnt/h/__DOWNLOADS/selforglinux/zcc_oneirogenesis.py", "w") as f:
    f.write(t)
print("fixed")
