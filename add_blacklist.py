#!/usr/bin/env python3
import re, sys

path = "/mnt/h/__DOWNLOADS/selforglinux/part4.c"
src = open(path).read()

old = '"init_compiler",\n        NULL'
new = '"init_compiler",\n        "cc_alloc",\n        NULL'

if "cc_alloc" in src:
    print("cc_alloc already in blacklist")
    sys.exit(0)

if old not in src:
    print("ERROR: could not find blacklist anchor")
    sys.exit(1)

src = src.replace(old, new)
open(path, "w").write(src)
print("OK: added cc_alloc to blacklist")
