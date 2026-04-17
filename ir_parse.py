import sys
import os

class IrNode:
    def __init__(self, op, type_str, dst, src1, src2, imm_label, line):
        self.op = op
        self.type_str = type_str
        self.dst = dst
        self.src1 = src1
        self.src2 = src2
        self.imm_label = imm_label
        self.line = line

    def __repr__(self):
        return f"IrNode(op='{self.op}', dst='{self.dst}', src1='{self.src1}', src2='{self.src2}', imm_label='{self.imm_label}', line={self.line})"

class IrFunc:
    def __init__(self, name, ret_type):
        self.name = name
        self.ret_type = ret_type
        self.nodes = []

class IrModule:
    def __init__(self):
        self.funcs = []

def parse_ir(file_path: str) -> IrModule:
    module = IrModule()
    current_func = None

    with open(file_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            
            if line.startswith("; func"):
                parts = line.split("->")
                name_part = parts[0].replace("; func", "").strip()
                ret_type = parts[1].strip() if len(parts) > 1 else ""
                current_func = IrFunc(name=name_part, ret_type=ret_type)
                module.funcs.append(current_func)
                
            elif line.startswith("; end"):
                current_func = None
                
            elif not line.startswith(";"):
                if not current_func:
                    continue
                
                if ";" in line:
                    inst_part, comment_part = line.split(";", 1)
                else:
                    inst_part = line
                    comment_part = "line 0"
                
                line_no = 0
                if "line" in comment_part:
                    try:
                        line_no = int(comment_part.replace("line", "").strip())
                    except ValueError:
                        pass
                
                tokens = [t.strip() for t in inst_part.split() if t.strip()]
                if len(tokens) >= 6:
                    op = tokens[0]
                    type_str = tokens[1]
                    dst = tokens[2]
                    src1 = tokens[3]
                    src2 = tokens[4]
                    imm_label = tokens[-1] # ALWAYS THE LAST TOKEN
                    # Example: const i32 %t0 - - - imm=0
                    # If len == 7, tokens[-1] handles the extra dash issue!
                    
                    if len(tokens) == 6:
                        imm_label = tokens[5]
                        
                    node = IrNode(
                        op=op,
                        type_str=type_str,
                        dst=dst,
                        src1=src1,
                        src2=src2,
                        imm_label=imm_label,
                        line=line_no
                    )
                    current_func.nodes.append(node)

    return module

