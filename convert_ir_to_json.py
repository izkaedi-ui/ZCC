import json
import sys
import re

def convert(text_path, out_path):
    with open(text_path, 'r') as f:
        lines = f.readlines()
        
    modules = []
    current_func = None
    
    for line in lines:
        line = line.strip()
        if not line:
            continue
        
        if line.startswith('; func'):
            m = re.match(r'; func (\w+) -> (.*)', line)
            if m:
                current_func = {"func": m.group(1), "ret_type": m.group(2), "instructions": []}
                modules.append(current_func)
            continue
            
        if line.startswith('; end'):
            current_func = None
            continue
            
        if line.startswith(';'):
            continue
            
        if current_func is not None:
            # Parse instruction
            # Example: LOAD        ptr     %t0  %n  -  -  ; line 289
            #          CONST       i32     %t2  -  -  -  imm=0  ; line 289
            
            # Split by ';' first
            parts = line.split(';', 1)
            inst_part = parts[0].strip()
            comment_part = parts[1].strip() if len(parts) > 1 else ""
            
            lineno = -1
            if comment_part.startswith('line '):
                l_str = comment_part.split(' ')[1]
                if l_str.isdigit():
                    lineno = int(l_str)
            
            tokens = inst_part.split()
            if not tokens:
                continue
                
            op = "IR_" + tokens[0]
            typ = "IR_TY_" + tokens[1].upper() if len(tokens) > 1 and tokens[1] != '-' else ""
            dst = tokens[2] if len(tokens) > 2 and tokens[2] != '-' else ""
            src1 = tokens[3] if len(tokens) > 3 and tokens[3] != '-' else ""
            src2 = tokens[4] if len(tokens) > 4 and tokens[4] != '-' else ""
            src3 = tokens[5] if len(tokens) > 5 and tokens[5] != '-' else ""
            
            # Check for imm=...
            # The extra tokens are after src3
            extra = ""
            if len(tokens) > 6:
                extra = " ".join(tokens[6:])
                
            inst = {
                "op": op,
                "type": typ,
                "dst": dst,
                "src1": src1,
                "src2": src2
            }
            if src3:
                inst["src3"] = src3
            if lineno != -1:
                inst["line"] = lineno
            if extra:
                inst["extra"] = extra
                
            current_func["instructions"].append(inst)

    with open(out_path, 'w') as f:
        json.dump(modules, f, indent=2)

if __name__ == "__main__":
    convert(sys.argv[1], sys.argv[2])
