import sys
from ir_parse import parse_ir, IrModule, IrFunc

def build_def_use(func: IrFunc):
    defs = {}
    for i, node in enumerate(func.nodes):
        if node.dst != '-':
            defs[node.dst] = (i, node)
    return defs

ALLOC_CALLS = frozenset({"malloc", "calloc", "realloc", "strdup", "strndup",
                          "new", "_Znwm", "_Znam"})

def slot_ever_stored_nonnull(slot, nodes, defs):
    """
    Returns True if the slot was EVER stored a provably non-null value:
      - const_str / addr (Juliet G2B string literal pattern)
      - result of a known allocation call (malloc/calloc/strdup — good* malloc pattern)
    """
    for node in nodes:
        if node.op == "store" and node.dst == slot:
            val_src = node.src1
            curr = defs.get(val_src, (0, None))[1]
            depth = 0
            while curr and curr.op in ("cast", "copy") and depth < 8:
                curr = defs.get(curr.src1, (0, None))[1]
                depth += 1
            if curr and curr.op == "const_str":
                return True
            if curr and curr.op == "call" and curr.imm_label in ALLOC_CALLS:
                return True
    return False

def slot_guarded_before_deref(slot, nodes):
    """
    Returns True if the slot is ever tested for null via its loaded value.
    Checks if any ne/eq instruction:
      a) directly uses the slot name as operand, OR
      b) uses a register that was loaded from the slot (common pattern: load %t = %stack, ne %t == 0)
    """
    # Build a map of temp -> what it was loaded from
    load_from = {}  # reg -> slot_name
    for node in nodes:
        if node.op == "load" and node.src1.startswith("%stack"):
            load_from[node.dst] = node.src1

    for node in nodes:
        if node.op in ("ne", "eq"):
            for src in (node.src1, node.src2):
                # Direct slot ref
                if src == slot:
                    return True
                # Indirect via load
                if load_from.get(src) == slot:
                    return True
    return False

def slot_null_stored(slot, nodes, defs):
    """
    Returns True if the slot is ever stored a zero/null value (imm=0 through cast chain).
    This is the vulnerability indicator in deref_after_check: null is assigned then dereferenced.
    """
    for node in nodes:
        if node.op == "store" and node.dst == slot:
            val_src = node.src1
            curr = defs.get(val_src, (0, None))[1]
            depth = 0
            while curr and curr.op in ("cast", "copy") and depth < 8:
                curr = defs.get(curr.src1, (0, None))[1]
                depth += 1
            if curr and curr.op == "const" and curr.imm_label == "imm=0":
                return True
    return False

def extract_476(func: IrFunc):
    defs = build_def_use(func)

    features = {
        "null_guards": 0,
        "unguarded_derefs": 0,
        "guarded_derefs": 0,
        "deref_after_check_vuln": 0   # new: null stored + deref without runtime null-guard
    }

    # Build non-null slots (const_str-backed — Juliet G2B pattern)
    nonnull_slots = set()
    for slot_name in set(n.dst for n in func.nodes if n.op == "store"):
        if slot_name.startswith("%stack") and slot_ever_stored_nonnull(slot_name, func.nodes, defs):
            nonnull_slots.add(slot_name)

    # Build null-stored slots (imm=0 assigned — potential null deref source)
    null_stored_slots = set()
    for slot_name in set(n.dst for n in func.nodes if n.op == "store"):
        if slot_name.startswith("%stack") and slot_null_stored(slot_name, func.nodes, defs):
            null_stored_slots.add(slot_name)

    # Build guarded aliases from ne/eq-to-null comparisons
    null_comparisons = set()
    for i, node in enumerate(func.nodes):
        if node.op in ("ne", "eq"):
            is_null_check = False
            ptr_reg = None
            for src in (node.src1, node.src2):
                curr = defs.get(src, (0, None))[1]
                while curr and curr.op in ("cast", "copy"):
                    curr = defs.get(curr.src1, (0, None))[1]
                if curr and curr.op == "const" and curr.imm_label == "imm=0":
                    is_null_check = True
                else:
                    ptr_reg = src
            if is_null_check and ptr_reg:
                null_comparisons.add((ptr_reg, i))

    alias_guards = {}
    for ptr_reg, idx in null_comparisons:
        curr = defs.get(ptr_reg, (0, None))[1]
        root = None
        if curr and curr.op == "load":
            root = curr.src1
        elif curr and curr.op == "add":
            add_src = curr.src1
            load_node = defs.get(add_src, (0, None))[1]
            if load_node and load_node.op == "load":
                root = load_node.src1
        if root and root.startswith("%stack"):
            alias_guards[root] = True
            features["null_guards"] += 1

    # Check for deref_after_check pattern:
    # Slot has null stored + is loaded and dereferenced but a prior null test was done
    # on the local variable (tested before assignment), so the runtime guard is present
    # but AFTER the null was already assigned to the pointer variable.
    # In the VULN case: null is stored unconditionally, then loaded and dereferenced
    # without a test on the LOADED pointer value.
    for slot in null_stored_slots:
        if slot in nonnull_slots:
            continue  # G2B — also has non-null store, skip
        guarded = slot_guarded_before_deref(slot, func.nodes)
        has_deref = any(
            n.op == "load" and n.src1.startswith("%stack") and n.src1 == slot
            for n in func.nodes
        )
        if has_deref and not guarded:
            features["deref_after_check_vuln"] += 1

    # Main alias tracking for traditional null-check pattern
    for node in func.nodes:
        if node.op in ("load", "store"):
            addr = node.dst if node.op == "store" else node.src1
            if not addr or addr == '-':
                continue
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
            if root_alias and root_alias.startswith("%stack"):
                if root_alias in nonnull_slots:
                    continue
                if root_alias in alias_guards:
                    features["guarded_derefs"] += 1
                else:
                    features["unguarded_derefs"] += 1

    return features


if __name__ == "__main__":
    mod = parse_ir(sys.argv[1])
    for f in mod.funcs:
        feats = extract_476(f)
        total_derefs = feats['guarded_derefs'] + feats['unguarded_derefs'] + feats['deref_after_check_vuln']
        is_vuln = (feats['unguarded_derefs'] > 0 and feats['guarded_derefs'] == 0) or feats['deref_after_check_vuln'] > 0
        is_safe = feats['guarded_derefs'] > 0 and feats['unguarded_derefs'] == 0 and feats['deref_after_check_vuln'] == 0
        if total_derefs > 0:
            status = "[VULN]" if is_vuln else ("[SAFE]" if is_safe else "[MIXED]")
            print(f"{status:8s} {f.name:50s} {feats}")
