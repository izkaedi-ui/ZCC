import sys, collections, re

def analyze_asm(filename):
    counts = collections.defaultdict(int)
    current_func = None
    func_pattern = re.compile(r'^([a-zA-Z_][a-zA-Z0-9_]*):$')
    
    with open(filename, 'r') as f:
        for line in f:
            m = func_pattern.match(line)
            if m:
                current_func = m.group(1)
            elif current_func:
                if 'rbp' in line:
                    counts[current_func] += 1
    return counts

ir_counts = analyze_asm('zcc_ir.s')
ast_counts = analyze_asm('zcc2.s')

# Find top differences where IR spills more than AST
diffs = []
for func in ir_counts:
    if func in ast_counts:
        diff = ir_counts[func] - ast_counts[func]
        diffs.append((diff, func, ir_counts[func], ast_counts[func]))

diffs.sort(reverse=True)
print("Top 3 functions with most EXTRA stack traffic (rbp references) in IR vs AST:")
for diff, func, ir_c, ast_c in diffs[:3]:
    print(f"{func}: IR={ir_c}, AST={ast_c}, Diff=+{diff}")
