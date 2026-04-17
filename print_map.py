import sys
with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    text = f.read()
start = text.find('typedef struct\n{\n    char\t\tname[8];')
if start != -1:
    end = text.find('} maptexture_t;', start)
    print(text[start:end+20])
