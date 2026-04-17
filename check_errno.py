import re
path = 'h:/__DOWNLOADS/selforglinux/doom_pp.c'
with open(path, 'r', encoding='utf-8') as f:
    text = f.read()
text = re.sub(r'\berrno\b', 'zcc_errno', text)
with open(path, 'w', encoding='utf-8') as f:
    f.write(text)
