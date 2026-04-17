import sys
import re
with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Fix the broken string splitting
code = code.replace(
'''    if (backend_ops) fprintf(cc->out, "    beq .L%d\\n" \\
    ); else fprintf(cc->out, "    je .L%d\\n", lbl1);''',
'''    if (backend_ops) emit_label_fmt(cc, lbl1, FMT_JE);
    else emit_label_fmt(cc, lbl1, FMT_JE);'''
)
code = code.replace(
'''    if (backend_ops) fprintf(cc->out, "    b .L%d\\n" \\
    ); else fprintf(cc->out, "    jmp .L%d\\n", lbl2);''',
'''    if (backend_ops) emit_label_fmt(cc, lbl2, FMT_JMP);
    else emit_label_fmt(cc, lbl2, FMT_JMP);'''
)
# Make sure to catch any other %d replacements
code = code.replace(
'''if (backend_ops) fprintf(cc->out, "    beq .L%d\\n" \\
    ); else fprintf(cc->out, "    je .L%d\\n", lbl1);''',
'''    emit_label_fmt(cc, lbl1, FMT_JE);'''
)
code = code.replace(
'''if (backend_ops) fprintf(cc->out, "    b .L%d\\n" \\
    ); else fprintf(cc->out, "    jmp .L%d\\n", lbl2);''',
'''    emit_label_fmt(cc, lbl2, FMT_JMP);'''
)

with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)
