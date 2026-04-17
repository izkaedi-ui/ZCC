import sys
try:
    with open('doom_pp_clean.c', 'r', encoding='utf-8') as f:
        for i, line in enumerate(f):
            if 'alloca' in line and 'void' in line:
                print(f"{i+1}: {line.strip()}")
            elif 'alloca' in line and '(' in line and ')' in line and ';' in line and not 'if' in line and not '=' in line:
                print(f"{i+1} (decl): {line.strip()}")
except Exception as e:
    print("Error:", e)
