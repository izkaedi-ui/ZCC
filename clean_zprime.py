import os

files_to_fix = [
    'zprime.h', 'ir_to_zprime.c',
    'zquantum.h', 'ir_to_zquantum.c',
    'zqec.h', 'ir_to_zqec.c',
    'zsurface.h', 'ir_to_zsurface.c'
]

for filename in files_to_fix:
    if not os.path.exists(filename): continue
    with open(filename, 'r', encoding='utf-8') as f:
        code = f.read()

    # Generic Fixes
    code = code.replace('IRNode', 'ir_insn')
    code = code.replace('Compiler *cc', 'ir_func *f')
    code = code.replace('cc->ir', 'f->first')

    # Header and specific fixes
    if filename.endswith('.h'):
        code = code.replace('#include "ir_bridge.h"', '#include <stdio.h>\n#include "ir.h"')
        # Some headers weren't capturing stdio
        if '<stdio.h>' not in code:
            code = code.replace('#ifndef ', '#include <stdio.h>\n#ifndef ')
    elif filename.endswith('.c'):
        # Fix missing includes
        if '<stdio.h>' not in code:
            code = '#include <stdio.h>\n' + code

    # Check for ops
    code = code.replace('insn->op % 32', 'insn->dst.reg % 32')

    if 'quantum' in filename or 'zqec' in filename or 'surface' in filename:
        code = code.replace('amp * cexp(I * phase_rot)', '(creal(amp) + I*cimag(amp)) * cexp(I * phase_rot)')
        code = code.replace('_Fcomplex', 'double _Complex')

    with open(filename, 'w', encoding='utf-8') as f:
        f.write(code)

print('Files patched successfully!')
