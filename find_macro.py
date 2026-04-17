import sys
with open('miniz.c', 'rb') as f:
    text = f.read()
import re
for m in re.finditer(b'MZ_MACRO_END', text):
    idx = m.start()
    print(repr(text[max(0, idx-60):idx+20]))
