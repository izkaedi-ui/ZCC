import re

# Fix doom_shims.c
path1 = 'h:/__DOWNLOADS/selforglinux/doom_shims.c'
with open(path1, 'r', encoding='utf-8') as f:
    text1 = f.read()

text1 += """
void* DefaultVisual(void* d, int s) {
    extern void* XDefaultVisual(void*, int);
    return XDefaultVisual(d, s);
}
"""
with open(path1, 'w', encoding='utf-8') as f:
    f.write(text1)


# Fix SCREENWIDTH/SCREENHEIGHT
path2 = 'h:/__DOWNLOADS/selforglinux/doom_pp_clean.c'
with open(path2, 'r', encoding='utf-8') as f:
    text2 = f.read()

text2 = text2.replace("SCREENWIDTH", "320")
text2 = text2.replace("SCREENHEIGHT", "200")

with open(path2, 'w', encoding='utf-8') as f:
    f.write(text2)

print("Linked fixes!")
