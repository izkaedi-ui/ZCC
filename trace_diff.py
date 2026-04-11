import os
import sys
for s in [44, 45, 47, 48, 50, 51]:
    os.system(f"./zcc fuzz_run6/mismatches/mismatch_seed{s}_debug.c -o tmp_zcc.s >/dev/null 2>&1")
    os.system(f"gcc -w -o tmp_zcc tmp_zcc.s -lm")
    os.system(f"gcc -w -O0 -o tmp_gcc fuzz_run6/mismatches/mismatch_seed{s}_debug.c -lm")
    os.system(f"./tmp_gcc > gcc_out.txt")
    os.system(f"./tmp_zcc > zcc_out.txt")
    with open('gcc_out.txt') as fg, open('zcc_out.txt') as fz:
        vg = fg.readlines()
        vz = fz.readlines()
        sys.stderr.write(f"--- SEED {s} ---\n")
        for i in range(len(vg)):
            if i >= len(vz):
                sys.stderr.write("ZCC truncated output\n")
                break
            if vg[i] != vz[i]:
                sys.stderr.write(f"GCC: {vg[i].strip()}  |  ZCC: {vz[i].strip()}\n")
                break
