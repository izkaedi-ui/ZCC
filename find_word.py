import sys
with open('miniz_test_zcc.c', 'r', encoding='utf-8', errors='ignore') as f:
    for i, line in enumerate(f):
        if 'TDEFL_READ_UNALIGNED_WORD' in line:
            print(f'{i+1}: {line.strip()[:150]}')
