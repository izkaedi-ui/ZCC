with open('zcc.c', 'r') as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    if '} else if (strcmp(argv[i], "-v") == 0) {' in line:
        lines.insert(i+2, '    } else if (strncmp(argv[i], "-l", 2) == 0 || strncmp(argv[i], "-L", 2) == 0 || strncmp(argv[i], "-O", 2) == 0 || strncmp(argv[i], "-f", 2) == 0 || strncmp(argv[i], "-I", 2) == 0 || strncmp(argv[i], "-D", 2) == 0) {\n')
        lines.insert(i+3, '      /* ignore linker flags */\n')
        break
with open('zcc.c', 'w') as f:
    f.writelines(lines)
