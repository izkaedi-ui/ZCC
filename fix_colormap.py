import re

path = 'h:/__DOWNLOADS/selforglinux/doom_pp_clean.c'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# 1. Remove CWColormap from attribmask
text = text.replace('attribmask = CWEventMask | CWColormap | CWBorderPixel;', 'attribmask = CWEventMask | CWBorderPixel;')

# 2. Remove attribs.colormap =
text = re.sub(r'attribs\.colormap = X_cmap;', '// attribs.colormap removed', text)

with open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Removed colormap attrib!")
