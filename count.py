import sys

def count_lines(filename):
    try:
        with open(filename, 'r') as f:
            return sum(1 for line in f if line.startswith(' '))
    except FileNotFoundError:
        return 0

ast_fold = count_lines('matrix_ast.s')  # Need to actually parse chunks, I'll do it simply
ir_fold = count_lines('matrix_ir.s')

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
                    if line[0].isalpha() or line.startswith('.Lfunc_end'):
                        # End of function (next function or end label)
                        if not line.startswith('.L'):
                            in_func = False
                            break
                    if line.startswith(' '):
                        count += 1
    except:
        pass
    return count

for f_name in ['fold_test', 'dce_test']:
    a = count_func('matrix_ast.s', f_name)
    i = count_func('matrix_ir.s', f_name)
    print(f'{f_name}: AST={a} IR={i} delta={a - i}')
