#!/usr/bin/env python3
import sys

path = "/mnt/h/__DOWNLOADS/selforglinux/part4.c"
src = open(path).read()

# Add type_new if not already present
to_add = ["type_new"]
for fn in to_add:
    if '"' + fn + '"' in src:
        print(f"{fn} already in blacklist")
        continue
    old = '"cc_alloc",\n        NULL'
    new = '"cc_alloc",\n        "' + fn + '",\n        NULL'
    if old not in src:
        print(f"ERROR: anchor not found for {fn}")
        sys.exit(1)
    src = src.replace(old, new)
    print(f"OK: added {fn} to blacklist")

open(path, "w").write(src)
