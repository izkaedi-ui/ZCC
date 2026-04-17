import sys
from ir_parse import parse_ir, IrModule, IrFunc

def build_def_use(func: IrFunc):
    defs = {}
    for i, node in enumerate(func.nodes):
        if node.dst != '-':
            defs[node.dst] = (i, node)
    return defs

def extract_415(func: IrFunc):
    defs = build_def_use(func)
    
    features = {
        "call_frees": 0,
        "use_after_free": 0
    }
    
    freed_aliases = set()
    
    for i, node in enumerate(func.nodes):
        if node.op == "call" and node.imm_label == "free":
            features["call_frees"] += 1
            for j in range(i-1, -1, -1):
                prev = func.nodes[j]
                if prev.op == "arg":
                    src = prev.src1
                    curr = defs.get(src, (0, None))[1]
                    while curr and curr.op in ("cast", "copy"):
                        curr = defs.get(curr.src1, (0, None))[1]
                    if curr and curr.op == "load":
                        root_alias = curr.src1
                        freed_aliases.add(root_alias)
                    break
                    
        if node.op in ("load", "store"):
            addr = node.dst if node.op == "store" else node.src1
            if not addr or addr == '-': continue
                
            root_alias = None
            curr = defs.get(addr, (0, None))[1]
            if curr:
                if curr.op == "add":
                    add_src = curr.src1
                    load_node = defs.get(add_src, (0, None))[1]
                    if load_node and load_node.op == "load":
                        root_alias = load_node.src1
                elif curr.op == "load":
                    root_alias = curr.src1
                    
            if root_alias and root_alias in freed_aliases:
                features["use_after_free"] += 1
                
    return features

if __name__ == "__main__":
    mod = parse_ir(sys.argv[1])
    for f in mod.funcs:
        feats = extract_415(f)
        if feats["call_frees"] > 0:
            print(f"       {f.name:50s} {feats}")
