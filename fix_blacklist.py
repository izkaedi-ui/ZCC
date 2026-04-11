#!/usr/bin/env python3
path = "/mnt/h/__DOWNLOADS/selforglinux/part4.c"
src = open(path).read()

old = '"main", "read_file", "init_compiler", NULL};'
new = '"main", "read_file", "init_compiler", "cc_alloc", "type_new", NULL};'

if old not in src:
    print("ERROR: anchor not found. Current blacklist line:")
    for line in src.split('\n'):
        if 'blacklist[]' in line or 'NULL};' in line:
            print("  ", line.strip())
    raise SystemExit(1)

src = src.replace(old, new)
open(path, "w").write(src)
print("OK: added cc_alloc and type_new to blacklist")
