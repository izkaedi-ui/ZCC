import re
def count(path, fns):
    with open(path) as f: t = f.read()
    r = {}
    for fn in fns:
        m = re.search(rf'^{fn}:.*?(?=^[a-z_]+:|\Z)', t, re.M|re.S)
        r[fn] = sum(1 for l in m.group().split(chr(10)) if l.startswith('    ')) if m else 0
    return r
fns = ['fold_test','dce_test','licm_test','pressure_test','escape_test']
ast = count('/mnt/h/__DOWNLOADS/selforglinux/matrix_ast.s', fns)
ir  = count('/mnt/h/__DOWNLOADS/selforglinux/matrix_ir.s', fns)
total_a = total_i = 0
print(f'{"Function":<18} {"AST":>5} {"IR":>5} {"Delta":>6} {"Result":<10}')
print('-' * 50)
for fn in fns:
    a, i = ast[fn], ir[fn]
    d = a - i
    total_a += a; total_i += i
    tag = 'OPT-WIN' if d > 0 else 'NEUTRAL' if d == 0 else 'REGRESS'
    print(f'{fn:<18} {a:>5} {i:>5} {d:>+6} {tag:<10}')
print('-' * 50)
print(f'{"TOTAL":<18} {total_a:>5} {total_i:>5} {total_a-total_i:>+6} {"":<10}')
pct = 100*(total_a-total_i)/total_a if total_a else 0
print(f'Optimization dividend: {pct:.1f}% instruction reduction')
