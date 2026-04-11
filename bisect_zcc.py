import os
import subprocess

fns = ["is_alpha", "is_digit", "is_alnum", "is_space", "hex_val", "is_power_of_2_val", "log2_of", "new_label", "push_reg", "pop_reg", "ir_op_name", "ir_type_name", "ir_type_bytes", "ir_type_unsigned", "ir_op_is_terminator"]

for fn in fns:
    print(f"Testing {fn}... ", end='', flush=True)
    os.system(f"git checkout part4.c >/dev/null 2>&1")
    with open("part4.c", "r") as f:
        content = f.read()
    content = content.replace("NULL", f'"{fn}", NULL')
    with open("part4.c", "w") as f:
        f.write(content)
    
    ret = os.system("make clean >/dev/null 2>&1 && make zcc_full >/dev/null 2>&1 && make selfhost >/dev/null 2>&1")
    if ret == 0:
        print("SAFE")
    else:
        print("DIVERGED")
os.system("git checkout part4.c >/dev/null 2>&1")
