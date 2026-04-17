import sys
with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    for i, line in enumerate(f):
        if 'void R_InstallSpriteLump' in line:
            print(f"Line {i+1}: {line.strip()}")
