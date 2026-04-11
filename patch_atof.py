import sys
lines = open('sqlite3_zcc.s').readlines()
out = []
for line in lines:
    if line.startswith('sqlite3AtoF:'):
        out.append('.globl sqlite3AtoF\n')
    out.append(line)
open('sqlite3_zcc.s', 'w').writelines(out)
