import sys
from ir_parse import parse_ir, IrModule, IrFunc

def build_def_use(func: IrFunc):
    defs = {}
    for i, node in enumerate(func.nodes):
        if node.dst != '-':
            defs[node.dst] = (i, node)
    return defs

def trace_to_root_op(src, nodes_list, defs, depth=0):
    """
    Walk the def-use chain upward through cast/copy/load.
    Returns:
      'const'     — provably constant (literal integer or string)
      'const_str' — string literal (non-null, bounded)
      'addr'      — address-of (non-null)
      'call'      — result of a function call (potentially unbounded/external)
      'external'  — came from store whose source is a call result (e.g. fscanf target)
      None        — unknown / could not resolve (treat as unbounded)
    """
    if depth > 12:
        return None
    curr = defs.get(src, (0, None))[1]
    if not curr:
        # Not in defs — could be a param or global. Treat as external/unbounded.
        return 'external'
    if curr.op in ("cast", "copy"):
        return trace_to_root_op(curr.src1, nodes_list, defs, depth + 1)
    if curr.op == "load":
        slot = curr.src1
        stores_to_slot = [n for n in nodes_list if n.op == "store" and n.dst == slot]
        if len(stores_to_slot) == 0:
            return 'external'
        if len(stores_to_slot) == 1:
            return trace_to_root_op(stores_to_slot[0].src1, nodes_list, defs, depth + 1)
        # Multiple stores: check if ALL are provably constant
        # If any is non-const, this slot may hold external data
        roots = [trace_to_root_op(s.src1, nodes_list, defs, depth + 1) for s in stores_to_slot]
        if all(r in ("const", "const_str", "addr") for r in roots):
            return 'const'   # all assignments are bounded literals
        return 'external'    # at least one store is external/unknown
    if curr.op == "call":
        return 'call'
    return curr.op  # 'const', 'const_str', 'addr', etc.

def is_provably_bounded(src, nodes_list, defs):
    """True only if the ultimate source is a compile-time constant."""
    root = trace_to_root_op(src, nodes_list, defs)
    return root in ("const", "const_str", "addr")

def extract_190(func: IrFunc):
    defs = build_def_use(func)
    nodes = func.nodes

    features = {
        "math_ops": 0,
        "width_extensions_before_math": 0,
        "const_operand_math": 0,
        "branch_validations": 0
    }

    for node in nodes:
        if node.op in ("add", "mul", "sub"):
            features["math_ops"] += 1

            s1_bounded = is_provably_bounded(node.src1, nodes, defs)
            s2_src = node.src2
            s2_bounded = (not s2_src or s2_src == '-') or is_provably_bounded(s2_src, nodes, defs)

            if s1_bounded and s2_bounded:
                features["const_operand_math"] += 1
                continue  # safe by construction, skip

            # Has a width-extension (cast) guarding the inputs?
            extended = False
            for src in (node.src1, node.src2):
                if src and src != '-':
                    curr = defs.get(src, (0, None))[1]
                    if curr and curr.op == "cast":
                        extended = True

            if extended:
                features["width_extensions_before_math"] += 1

    # Detect range guards: lt/gt/le/ge comparisons before the math
    # Pattern: compare external input against a const bound (if, early-exit)
    # Indicates the function guarantees overflow won't happen
    for node in nodes:
        if node.op in ("lt", "le", "gt", "ge"):
            b_s1 = is_provably_bounded(node.src1, nodes, defs)
            b_s2 = (not node.src2 or node.src2 == '-') or is_provably_bounded(node.src2, nodes, defs)
            # One operand bounded (const limit), one external = range guard
            if (b_s1 and not b_s2) or (b_s2 and not b_s1):
                features["branch_validations"] += 1

    return features


if __name__ == "__main__":
    mod = parse_ir(sys.argv[1])
    for f in mod.funcs:
        feats = extract_190(f)
        if feats["math_ops"] > 0:
            print(f"       {f.name:50s} {feats}")
