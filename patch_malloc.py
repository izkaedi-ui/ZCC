import sys, re

with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
    code = f.read()

# Replace the clang-exclusive malloc prototype to match 64-bit size_t expectations
code = code.replace('void *malloc(unsigned long);', 'void *malloc(unsigned long long);')

with open('doom_pp_clean.c', 'w', encoding='utf-8') as f:
    f.write(code)

print("Updated malloc prototype.")
