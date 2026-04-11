lines = open('compiler_passes.c').readlines()
for i in range(1154, 1170):
    print(f'L{i+1}: {repr(lines[i])}')
