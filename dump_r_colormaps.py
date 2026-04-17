import sys
with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    text = f.read()
start = text.find('void R_InitColormaps (void)')
if start != -1:
    end = text.find('\n}\n', start)
    print(text[start:end+3])
