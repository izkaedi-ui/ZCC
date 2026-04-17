import sys
from ir_parse import parse_ir, IrModule, IrFunc

def build_def_use(func: IrFunc):
    defs = {}
    for i, node in enumerate(func.nodes):
        if node.dst != '-':
            defs[node.dst] = (i, node)
    return defs

def resolve_const_arg(src, defs, nodes, depth=0):
    """
    Walk back from an ARG src to find a constant size.
    Returns the integer value if the chain terminates at a const, else None.
    Handles: const -> shl -> cast -> arg (Juliet alloca sizing pattern)
              const directly
    """
    if depth > 8:
        return None
    curr = defs.get(src, (0, None))[1]
    if not curr:
        return None
    if curr.op == "const" and curr.imm_label.startswith("imm="):
        return int(curr.imm_label.replace("imm=", ""))
    if curr.op in ("cast", "shl", "copy"):
        return resolve_const_arg(curr.src1, defs, nodes, depth + 1)
    if curr.op == "load":
        # Walk through store into this slot
        for n in nodes:
            if n.op == "store" and n.dst == curr.src1:
                return resolve_const_arg(n.src1, defs, nodes, depth + 1)
    return None

# Known buffer-dangerous stdlib calls and which argument carries the size
# (0-indexed from the ARG nodes preceding the CALL)
# ZCC emits ARG nodes in REVERSE argument order (last C arg first).
# strncat(dst, src, n) -> ZCC emits: arg n, arg src, arg dst
# So size_arg=0 captures the FIRST emitted ARG = last C argument = size/count.
DANGEROUS_CALLS = {
    "memcpy":   {"size_arg": 0},  # memcpy(dst, src, size) — last C arg = first emitted
    "memmove":  {"size_arg": 0},
    "strncpy":  {"size_arg": 0},  # strncpy(dst, src, n)
    "strncat":  {"size_arg": 0},  # strncat(dst, src, n)
    "snprintf": {"size_arg": 1},  # snprintf(buf, size, fmt, ...) — size is 2nd from end
    "wcsncpy":  {"size_arg": 0},
    "wcsncat":  {"size_arg": 0},
}

def extract_121_memcpy(func: IrFunc):
    """
    Detects buffer overflow via function-call width:
    - Find ALLOCA with constant size S
    - Find a call to memcpy/strncpy/etc. where the byte-count arg is N
    - Emit overflow=True if N > S
    """
    defs = build_def_use(func)
    nodes = func.nodes

    alloca_sizes = []
    call_sizes = {}  # call_name -> [sizes found]

    # Collect ALLOCA sizes
    for i, node in enumerate(nodes):
        if node.op == "call" and node.imm_label == "ALLOCA":
            for j in range(i - 1, -1, -1):
                prev = nodes[j]
                if prev.op == "arg":
                    val = resolve_const_arg(prev.src1, defs, nodes)
                    if val is not None:
                        alloca_sizes.append(val)
                    break

    # Collect dangerous call size arguments
    for i, node in enumerate(nodes):
        call_target = node.imm_label if node.op == "call" else None
        if call_target and call_target in DANGEROUS_CALLS:
            cfg = DANGEROUS_CALLS[call_target]
            target_argidx = cfg["size_arg"]
            # Collect ARG nodes preceding this CALL
            args = []
            for j in range(i - 1, -1, -1):
                if nodes[j].op == "arg":
                    args.insert(0, nodes[j])
                elif nodes[j].op == "call":
                    break
            if len(args) > target_argidx:
                size_src = args[target_argidx].src1
                val = resolve_const_arg(size_src, defs, nodes)
                if val is not None:
                    call_sizes.setdefault(call_target, []).append(val)

    min_alloca = min(alloca_sizes) if alloca_sizes else 0
    all_call_sizes = [v for vals in call_sizes.values() for v in vals]
    max_call_size = max(all_call_sizes) if all_call_sizes else 0

    overflow = (max_call_size > min_alloca) if (alloca_sizes and all_call_sizes) else False

    features = {
        "alloca_min": min_alloca,
        "alloca_count": len(alloca_sizes),
        "max_call_size": max_call_size,
        "call_targets": sorted(call_sizes.keys()),
        "overflow": overflow,
    }
    return features


if __name__ == "__main__":
    mod = parse_ir(sys.argv[1])
    for f in mod.funcs:
        feats = extract_121_memcpy(f)
        if feats["alloca_min"] > 0 or feats["max_call_size"] > 0:
            status = "[VULN]" if feats["overflow"] else "[SAFE]"
            print(f"{status:8s} {f.name:50s} {feats}")
