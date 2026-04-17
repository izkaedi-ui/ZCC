import sys

try:
    in_func = False
    with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
        for line in f:
            if 'void R_InitTextures (void)' in line:
                in_func = True
            if in_func:
                print(line, end='')
                if line.strip() == '}':
                    break
except Exception as e:
    pass
