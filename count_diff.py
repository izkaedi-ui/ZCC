import sys

def count_func(filename, f_name):
    count = 0
    in_func = False
    try:
        with open(filename, 'r') as f:
            for line in f:
                if line.startswith(f_name + ':'):
                    in_func = True
                    continue
                if in_func:
                    # End if new label or .Lfunc_end
                    if line[0].isalpha() and not line.startswith('.L'):
                        in_func = False
                        break
                    # Count instruction lines starting with whitespace
                    # Skip directives like .loc, .p2align
                    if line.startswith(' ') or line.startswith('\t'):
                        sline = line.strip()
                        if sline and not sline.startswith('.'):
                            count += 1
    except Exception as e:
        pass
    return count

for f in ['fold_test', 'licm_test', 'dce_test', 'pressure_test', 'escape_test']:
    a = count_func('matrix_ast.s', f)
    i = count_func('matrix_ir.s', f)
    print(f'{f:15} AST={a:<4} IR={i:<4} DELTA={a-i}')
