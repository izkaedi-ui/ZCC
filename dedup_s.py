import sys

seen = set()
with open(sys.argv[1], 'r') as f:
    for line in f:
        l = line.strip()
        if l.endswith(':'):
            if l in seen:
                continue
            seen.add(l)
        sys.stdout.write(line)
