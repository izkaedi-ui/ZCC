import sys

try:
    in_func = False
    lines_read = 0
    with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
        for line in f:
            if 'void R_InitSprites (char** namelist)' in line:
                in_func = True
            if in_func:
                print(line, end='')
                lines_read += 1
                if lines_read > 200:
                    break
except Exception as e:
    pass
