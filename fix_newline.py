#!/usr/bin/env python3
path = "/mnt/h/__DOWNLOADS/selforglinux/part4.c"
lines = open(path).readlines()
out = []
i = 0
while i < len(lines):
    if 'BLACKLIST' in lines[i] and lines[i].rstrip().endswith('%s'):
        # This line has a broken string — merge with next line
        fixed = lines[i].rstrip() + '\\n' + lines[i+1].lstrip()
        out.append(fixed)
        i += 2
    else:
        out.append(lines[i])
        i += 1
open(path, 'w').writelines(out)
print("OK: fixed broken newline in fprintf")
