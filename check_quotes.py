with open("pp_dump.c", "r") as f:
    lines = f.readlines()
for i, line in enumerate(lines):
    quotes = line.count('"')
    if quotes % 2 != 0:
        print(f"Line {i+1} has odd quotes ({quotes}): {line.strip()}")
