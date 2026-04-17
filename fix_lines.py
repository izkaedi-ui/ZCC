path = 'h:/__DOWNLOADS/selforglinux/doom_pp_clean.c'
with open(path, 'r', encoding='utf-8') as f:
    lines = f.readlines()
lines = lines[:13728] + lines[13745:]
with open(path, 'w', encoding='utf-8') as f:
    f.writelines(lines)
