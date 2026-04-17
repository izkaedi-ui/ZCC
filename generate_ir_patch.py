import re

with open('part4.c', 'r') as f:
    text = f.read()

# We need to inject ir_save_result after codegen_expr_checked
# Wait, this is getting complicated.
