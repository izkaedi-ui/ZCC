#!/usr/bin/env python3
import re
path = "/mnt/h/__DOWNLOADS/selforglinux/part4.c"
src = open(path).read()
src = re.sub(
    r'static const char \*blacklist\[\] = \{[^}]+\};',
    'static const char *blacklist[] = {\n'
    '        "main", "read_file", "init_compiler", NULL};',
    src)
open(path, 'w').write(src)
print("OK: blacklist reduced to main, read_file, init_compiler")
