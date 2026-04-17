import sys
for file in ['miniz.h', 'miniz.c']:
    with open(file, 'r', encoding='utf-8', errors='ignore') as f:
        for i, line in enumerate(f):
            if 'define MINIZ_USE_UNALIGNED_LOADS_AND_STORES' in line:
                print(f'{file}:{i+1}:{line.strip()}')
            if 'MINIZ_USE_UNALIGNED_LOADS_AND_STORES 0' in line:
                print(f'{file}:{i+1}:{line.strip()}')
