import sys
with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

def repl(target, val):
    global code
    code = code.replace(
        f'fprintf(cc->out, "{target}\\n");',
        f'if (backend_ops) fprintf(cc->out, "{val}\\n");\n      else fprintf(cc->out, "{target}\\n");'
    )

repl('    movq %%rax, %%r11', '    mov r1, r0')
repl('    movq %%rax, %%rdx', '    mov r2, r0')
repl('    movq %%r11, %%rax', '    mov r0, r1')
repl('    movq %%rdx, %%rax', '    mov r0, r2')
repl('    popq %%rdx', '    pop {r2}')
repl('    movl %%r11d, (%%rax)', '    str r1, [r0]')
repl('    movw %%r11w, (%%rax)', '    strh r1, [r0]')
repl('    movb %%r11b, (%%rax)', '    strb r1, [r0]')
repl('    movq %%r11, (%%rax)', '    str r1, [r0]')

with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)
