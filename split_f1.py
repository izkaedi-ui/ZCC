import re

with open('fuzz_results_gods_eye/mismatches/mismatch_energy0.10_seed30212.c', 'r') as f:
    lines = f.readlines()

f1_content = []
in_f1 = False
for line in lines:
    if 'static int f1(int a, int b, int c) {' in line:
        in_f1 = True
    if in_f1:
        f1_content.append(line)
        if line.startswith('}'):
            break

print(''.join(f1_content))
