import re
path = 'h:/__DOWNLOADS/selforglinux/doom_pp_clean.c'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()

target = r"""     for \(i=0 ; i<256 ; i\+\+\)
     \{
  c = gammatable\[usegamma\]\[\*palette\+\+\];
  colors\[i\].red = \(c<<8\) \+ c;
  c = gammatable\[usegamma\]\[\*palette\+\+\];
  colors\[i\].green = \(c<<8\) \+ c;
  c = gammatable\[usegamma\]\[\*palette\+\+\];
  colors\[i\].blue = \(c<<8\) \+ c;
     \}


     XStoreColors\(X_display, cmap, colors, 256\);

  \}
\}"""

text = re.sub(target, '', text)

with open(path, 'w', encoding='utf-8') as f:
    f.write(text)
print("Removed trailing braces")
