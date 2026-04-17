import sys
from ir_parse import parse_ir, IrModule, IrFunc

def build_def_use(func: IrFunc):
    defs = {}
    for i, node in enumerate(func.nodes):
        if node.dst != '-':
            defs[node.dst] = (i, node)
    return defs

def extract_121(func: IrFunc):
    defs = build_def_use(func)

    # Map: alloca_result_temp -> constant size
    alloca_sizes = {}  # temp_name -> size
    alloca_order = []  # in order of appearance

    for i, node in enumerate(func.nodes):
        if node.op == "call" and node.imm_label == "ALLOCA":
            # Look backwards for the ARG preceding this CALL
            for j in range(i - 1, -1, -1):
                prev = func.nodes[j]
                if prev.op == "arg":
                    src = prev.src1
                    curr = defs.get(src, (0, None))[1]
                    while curr and curr.op in ("cast", "shl", "copy"):
                        curr = defs.get(curr.src1, (0, None))[1]
                    if curr and curr.op == "const" and curr.imm_label.startswith("imm="):
                        val = int(curr.imm_label.replace("imm=", ""))
                        alloca_sizes[node.dst] = val
                        alloca_order.append((node.dst, val))
                    break

    # Find which alloca'd pointer is actually stored into the stack slot
    # used by loop indexing. The vulnerable alloca is the SMALL one (first).
    # The safe alloca is the LARGE one (source data).
    # Heuristic: Juliet pattern has two allocas. FIRST alloca is dest (small/vuln),
    # SECOND alloca is source (large/data). Use MINIMUM alloca as the risk alloca.
    
    bounds_checks = []
    for i, node in enumerate(func.nodes):
        if node.op in ("lt", "le", "gt", "ge"):
            for src in (node.src1, node.src2):
                curr = defs.get(src, (0, None))[1]
                while curr and curr.op in ("cast", "copy"):
                    curr = defs.get(curr.src1, (0, None))[1]
                if curr and curr.op == "const" and curr.imm_label.startswith("imm="):
                    val = int(curr.imm_label.replace("imm=", ""))
                    bounds_checks.append(val)

    # Use minimum alloca as the potentially vulnerable buffer
    min_alloca = min(v for _, v in alloca_order) if alloca_order else 0
    max_bound = max(bounds_checks) if bounds_checks else 0

    # Overflow: the smallest buffer is smaller than the loop visits
    overflow = (max_bound > min_alloca) if (alloca_order and bounds_checks) else False

    features = {
        "alloca_min": min_alloca,
        "alloca_count": len(alloca_order),
        "max_bound": max_bound,
        "overflow": overflow,
    }
    return features


if __name__ == "__main__":
    mod = parse_ir(sys.argv[1])
    for f in mod.funcs:
        feats = extract_121(f)
        if feats['alloca_min'] > 0 or feats['max_bound'] > 0:
            status = "[VULN]" if feats['overflow'] else "[SAFE]"
            print(f"{status:8s} {f.name:50s} {feats}")
