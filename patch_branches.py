import sys
with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

def repl(target, val):
    global code
    code = code.replace(
        f'fprintf(cc->out, "{target}\\n"',
        f'if (backend_ops) fprintf(cc->out, "{val}\\n" \\\n    ); else fprintf(cc->out, "{target}\\n"'
    )

repl('    cmpq $0, %%rax', '    cmp r0, #0')
repl('    cmpq %%r11, %%rax', '    cmp r0, r1')
repl('    je .L%d', '    beq .L%d')
repl('    jmp .L%d', '    b .L%d')
repl('    jmp %s', '    b %s')

with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)
