import sys

def fix_cases(file_path, log_path):
    with open(log_path, 'r') as f:
        log = f.read().splitlines()

    error_lines = []
    for line in log:
        if 'unexpected token 25 in expression' in line:
            parts = line.split(':')
            if len(parts) >= 2 and parts[1].isdigit():
                error_lines.append(int(parts[1]))

    if not error_lines:
        print("No nested cases found.")
        return
        
    print(f"Found error lines: {error_lines}")

    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    patched = set()

    for err_line in error_lines:
        idx = err_line - 1
        
        brace_idx = -1
        for i in range(idx, -1, -1):
            if 'case ' in lines[i] and '{' in lines[i]:
                brace_idx = i
                break
        
        if brace_idx == -1 or brace_idx in patched:
            continue
            
        print(f"For error at {err_line}, found block start at {brace_idx+1}")
        
        level = 0
        match_idx = -1
        for i in range(brace_idx, len(lines)):
            if '{' in lines[i]: level += lines[i].count('{')
            if '}' in lines[i]: level -= lines[i].count('}')
            if level == 0:
                match_idx = i
                break
                
        if match_idx != -1:
            print(f"Found matching close at {match_idx+1}")
            lines[brace_idx] = lines[brace_idx].replace('{', ' ', 1) 
            # carefully replace only the closing brace that matched!
            lines[match_idx] = lines[match_idx].replace('}', ' ', 1) 
            patched.add(brace_idx)

    with open(file_path, 'w', encoding='utf-8') as f:
        f.writelines(lines)
    print("Done patching cases.")

fix_cases('h:/__DOWNLOADS/selforglinux/sqlite3_zcc.c', 'h:/__DOWNLOADS/selforglinux/sys_sqlite_out.log')
