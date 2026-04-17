import re

with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Fix lines 92 and 96 parameter bug
code = code.replace(
    '    if (backend_ops) fprintf(cc->out, "    beq .L%d\\n" \\\n    ); else fprintf(cc->out, "    je .L%d\\n", n);',
    '    if (backend_ops) fprintf(cc->out, "    beq .L%d\\n", n);\n    else fprintf(cc->out, "    je .L%d\\n", n);'
)
code = code.replace(
    '    if (backend_ops) fprintf(cc->out, "    b .L%d\\n" \\\n    ); else fprintf(cc->out, "    jmp .L%d\\n", n);',
    '    if (backend_ops) fprintf(cc->out, "    b .L%d\\n", n);\n    else fprintf(cc->out, "    jmp .L%d\\n", n);'
)

# Fix indentation globally for 'if (backend_ops)'
code = re.sub(r'([ \t]*)if \(backend_ops\)(.*?)\n[ \t]+else fprintf\(', r'\1if (backend_ops)\2\n\1else fprintf(', code)

# Ensure no hanging string splits caused syntax errors if '    else fprintf' was already correctly indented
code = re.sub(r'([ \t]*)if \(backend_ops\)(.*?)\n[ \t]+else \{', r'\1if (backend_ops)\2\n\1else {', code)


with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)
