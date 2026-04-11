import subprocess, re
r = subprocess.run(["./zcc2", "/tmp/t_loop.c", "-o", "/tmp/t_loop_ir.s"],
                   capture_output=True, text=True,
                   env={**__import__('os').environ, "ZCC_IR_BACKEND": "1"})
lines = r.stderr.split("\n")
printing = False
count = 0
for l in lines:
    if "PRE-OPT IR" in l:
        printing = True
        count = 0
    if printing:
        print(l)
        count += 1
        if count > 80:
            break
