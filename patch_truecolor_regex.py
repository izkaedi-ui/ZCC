import re

path = 'h:/__DOWNLOADS/selforglinux/doom_pp_clean.c'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()

# 1. Visual Info + disable SHM
text = re.sub(r'if \(!XMatchVisualInfo.+?I_Error\("xdoom currently only supports 256-color PseudoColor screens"\);\n?\s*X_visual = X_visualinfo\.visual;',
              'X_visual = DefaultVisual(X_display, X_screen);',
              text, flags=re.DOTALL)

text = re.sub(r'doShm = XShmQueryExtension\(X_display\);', 'doShm = 0;', text)

# 2. XCreateImage depth 8 -> 24 and allocation 4x
text = re.sub(r'image = XCreateImage\(\s*X_display,\s*X_visual,\s*8,\s*ZPixmap,\s*0,\s*\(char\*\)malloc\(X_width\*X_height\),\s*X_width, X_height,\s*8, 0 \);',
              'image = XCreateImage( X_display, X_visual, 24, ZPixmap, 0, (char*)malloc(X_width*X_height*4), X_width, X_height, 32, 0 );',
              text)

# 3. UploadNewPalette
palette_new = """static int truecolor_palette[256];

void UploadNewPalette(Colormap cmap, byte *palette)
{
    int i, c;
    for (i=0 ; i<256 ; i++)
    {
        c = gammatable[usegamma][*palette++];
        int r = c;
        c = gammatable[usegamma][*palette++];
        int g = c;
        c = gammatable[usegamma][*palette++];
        int b = c;
        truecolor_palette[i] = (r << 16) | (g << 8) | b;
    }
}"""
text = re.sub(r'static XColor\s+colors\[256\];\s*void UploadNewPalette\(Colormap cmap, byte \*palette\).*?\}\s*\}', palette_new, text, flags=re.DOTALL)

# 4. I_FinishUpdate
finish_new = """void I_FinishUpdate (void)
{
    int x, y, dx, dy;
    unsigned char *src = (unsigned char *)screens[0];
    int *dest = (int *)image->data;
    
    for (y = 0; y < SCREENHEIGHT; y++) {
        for (x = 0; x < SCREENWIDTH; x++) {
            int color = truecolor_palette[src[y * SCREENWIDTH + x]];
            for (dy = 0; dy < multiply; dy++) {
                for (dx = 0; dx < multiply; dx++) {
                    dest[(y * multiply + dy) * X_width + (x * multiply + dx)] = color;
                }
            }
        }
    }

    XPutImage(X_display, X_mainWindow, X_gc, image, 0, 0, 0, 0, X_width, X_height);
    XSync(X_display, False);
}"""

text = re.sub(r'void I_FinishUpdate \(void\)\s*\{.*?XSync\(X_display, False\);\s*\}\s*\}', finish_new, text, flags=re.DOTALL)


with open(path, 'w', encoding='utf-8') as f:
    f.write(text)

print("Regex patched!")
