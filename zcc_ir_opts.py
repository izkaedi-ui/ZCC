#!/usr/bin/env python3
"""
🔱 ZCC IR Optimization Pipeline v1.0.0
═══════════════════════════════════════

Stacks on top of DCE pass (zcc_ir_optimized.json → zcc_ir_final.json).

Passes (executed in order):
  1. Constant Folding    — CONST op CONST → CONST
  2. Copy Propagation    — %t5 = COPY %t3 → replace all uses of %t5 with %t3
  3. Constant Propagation — %t0 = CONST 42 → propagate 42 to all uses of %t0
  4. Algebraic Simplification — x+0, x*1, x*0, x-0, x/1, x&0, x|0, x^0
  5. Dead Store Elimination — STORE to locals never LOADed again
  6. Final DCE Sweep      — clean up instructions orphaned by earlier passes

Each pass is idempotent. The pipeline iterates until fixpoint (no changes).

Input:  zcc_ir_optimized.json  (post-DCE, 15,402 instructions)
        OR zcc_ir.json         (raw, 17,264 instructions)
Output: zcc_ir_final.json

Usage:
    python3 zcc_ir_opts.py [input.json] [output.json]
    python3 zcc_ir_opts.py                              # defaults
"""

import json
import sys
import copy
from collections import defaultdict
from pathlib import Path


# ═══════════════════════════════════════════════════════════════════
# IR Instruction Model
# ═══════════════════════════════════════════════════════════════════

# Opcodes that produce a result in 'dst'
RESULT_OPS = {
    'ADD', 'SUB', 'MUL', 'DIV', 'MOD', 'NEG',
    'AND', 'OR', 'XOR', 'NOT', 'SHL', 'SHR',
    'EQ', 'NE', 'LT', 'LE', 'GT', 'GE',
    'CAST', 'COPY', 'CONST', 'CONST_STR',
    'LOAD', 'ALLOCA', 'CALL', 'PHI',
}

# Opcodes with side effects — never eliminate
SIDE_EFFECT_OPS = {
    'CALL', 'STORE', 'RET', 'BR', 'BR_IF',
    'LABEL', 'ARG', 'NOP',
}

# Binary arithmetic opcodes
BINARY_OPS = {
    'ADD', 'SUB', 'MUL', 'DIV', 'MOD',
    'AND', 'OR', 'XOR', 'SHL', 'SHR',
    'EQ', 'NE', 'LT', 'LE', 'GT', 'GE',
}

# Unary opcodes
UNARY_OPS = {'NEG', 'NOT', 'CAST'}


def get_operands(instr):
    """Extract all operand temporaries from an instruction."""
    ops = []
    for key in ('src', 'src1', 'src2', 'lhs', 'rhs', 'cond', 'value', 'addr', 'arg'):
        val = instr.get(key)
        if val and isinstance(val, str) and val.startswith('%'):
            ops.append((key, val))
    # Handle 'args' list (for CALL/PHI)
    args = instr.get('args', [])
    if isinstance(args, list):
        for i, a in enumerate(args):
            if isinstance(a, str) and a.startswith('%'):
                ops.append((f'args[{i}]', a))
    return ops


def get_dst(instr):
    """Get the destination temporary, if any."""
    dst = instr.get('dst')
    if dst and isinstance(dst, str) and dst.startswith('%'):
        return dst
    return None


def is_const_int(val):
    """Check if a value is a constant integer (not a temporary)."""
    if val is None:
        return False
    if isinstance(val, (int, float)):
        return True
    if isinstance(val, str):
        if val.startswith('%'):
            return False
        try:
            int(val)
            return True
        except ValueError:
            try:
                float(val)
                return True
            except ValueError:
                return False
    return False


def to_int(val):
    """Convert a constant value to integer."""
    if isinstance(val, int):
        return val
    if isinstance(val, float):
        return int(val)
    if isinstance(val, str):
        return int(val)
    return 0


# ═══════════════════════════════════════════════════════════════════
# Pass 1: Constant Folding
# ═══════════════════════════════════════════════════════════════════

def constant_fold(functions):
    """
    CONST op CONST → CONST

    If both operands of a binary op are integer constants,
    compute the result at compile time.
    """
    total_folded = 0

    for func in functions:
        instrs = func.get('instructions', [])
        new_instrs = []

        for instr in instrs:
            op = instr.get('op', '')

            if op in BINARY_OPS:
                lhs = instr.get('lhs') if instr.get('lhs') is not None else instr.get('src1')
                rhs = instr.get('rhs') if instr.get('rhs') is not None else instr.get('src2')

                if is_const_int(lhs) and is_const_int(rhs):
                    a, b = to_int(lhs), to_int(rhs)
                    result = None

                    try:
                        if op == 'ADD': result = a + b
                        elif op == 'SUB': result = a - b
                        elif op == 'MUL': result = a * b
                        elif op == 'DIV' and b != 0: result = a // b
                        elif op == 'MOD' and b != 0: result = a % b
                        elif op == 'AND': result = a & b
                        elif op == 'OR':  result = a | b
                        elif op == 'XOR': result = a ^ b
                        elif op == 'SHL' and 0 <= b < 64: result = a << b
                        elif op == 'SHR' and 0 <= b < 64: result = a >> b
                        elif op == 'EQ': result = 1 if a == b else 0
                        elif op == 'NE': result = 1 if a != b else 0
                        elif op == 'LT': result = 1 if a < b else 0
                        elif op == 'LE': result = 1 if a <= b else 0
                        elif op == 'GT': result = 1 if a > b else 0
                        elif op == 'GE': result = 1 if a >= b else 0
                    except (OverflowError, ZeroDivisionError):
                        pass

                    if result is not None:
                        folded = {
                            'op': 'CONST',
                            'dst': instr.get('dst'),
                            'value': result,
                        }
                        if 'type' in instr:
                            folded['type'] = instr['type']
                        if 'line' in instr:
                            folded['line'] = instr['line']
                        new_instrs.append(folded)
                        total_folded += 1
                        continue

            elif op == 'NEG' and is_const_int((instr.get('src') if instr.get('src') is not None else instr.get('src1'))):
                val = to_int((instr.get('src') if instr.get('src') is not None else instr.get('src1')))
                folded = {
                    'op': 'CONST',
                    'dst': instr.get('dst'),
                    'value': -val,
                }
                if 'type' in instr:
                    folded['type'] = instr['type']
                if 'line' in instr:
                    folded['line'] = instr['line']
                new_instrs.append(folded)
                total_folded += 1
                continue

            elif op == 'NOT' and is_const_int((instr.get('src') if instr.get('src') is not None else instr.get('src1'))):
                val = to_int((instr.get('src') if instr.get('src') is not None else instr.get('src1')))
                folded = {
                    'op': 'CONST',
                    'dst': instr.get('dst'),
                    'value': ~val,
                }
                if 'type' in instr:
                    folded['type'] = instr['type']
                if 'line' in instr:
                    folded['line'] = instr['line']
                new_instrs.append(folded)
                total_folded += 1
                continue

            new_instrs.append(instr)

        func['instructions'] = new_instrs

    return total_folded


# ═══════════════════════════════════════════════════════════════════
# Pass 2: Copy Propagation
# ═══════════════════════════════════════════════════════════════════

def copy_propagation(functions):
    """
    %t5 = COPY %t3  →  replace all uses of %t5 with %t3

    Propagates through chains: if %t5 = COPY %t3 and %t3 = COPY %t1,
    then %t5 → %t1.
    """
    total_propagated = 0

    for func in functions:
        instrs = func.get('instructions', [])

        # Build copy map: dst → src (follow chains)
        copy_map = {}
        for instr in instrs:
            if instr.get('op') == 'COPY':
                dst = get_dst(instr)
                src = (instr.get('src') if instr.get('src') is not None else instr.get('src1')) or instr.get('value')
                if dst and src and isinstance(src, str) and src.startswith('%'):
                    copy_map[dst] = src

        # Resolve chains: %t5 → %t3 → %t1 becomes %t5 → %t1
        def resolve(tmp, depth=0):
            if depth > 50:
                return tmp
            if tmp in copy_map:
                return resolve(copy_map[tmp], depth + 1)
            return tmp

        resolved_map = {}
        for dst in copy_map:
            resolved = resolve(dst)
            if resolved != dst:
                resolved_map[dst] = resolved

        if not resolved_map:
            continue

        # Replace all uses
        for instr in instrs:
            for key in ('src', 'src1', 'src2', 'lhs', 'rhs', 'cond', 'value', 'addr', 'arg'):
                val = instr.get(key)
                if val and isinstance(val, str) and val in resolved_map:
                    instr[key] = resolved_map[val]
                    total_propagated += 1

            args = instr.get('args', [])
            if isinstance(args, list):
                for i, a in enumerate(args):
                    if isinstance(a, str) and a in resolved_map:
                        args[i] = resolved_map[a]
                        total_propagated += 1

    return total_propagated


# ═══════════════════════════════════════════════════════════════════
# Pass 3: Constant Propagation
# ═══════════════════════════════════════════════════════════════════

def constant_propagation(functions):
    """
    %t0 = CONST 42  →  propagate 42 into all uses of %t0

    Only propagates if %t0 is defined exactly once (SSA property).
    After propagation, the CONST instruction may become dead (cleaned by DCE).
    """
    total_propagated = 0

    for func in functions:
        instrs = func.get('instructions', [])

        # Build constant map: tmp → value (only single-def)
        const_map = {}
        def_count = defaultdict(int)

        for instr in instrs:
            dst = get_dst(instr)
            if dst:
                def_count[dst] += 1

        for instr in instrs:
            if instr.get('op') == 'CONST':
                dst = get_dst(instr)
                val = instr.get('value')
                if dst and val is not None and def_count[dst] == 1:
                    if is_const_int(val):
                        const_map[dst] = val

        if not const_map:
            continue

        # Replace uses in binary/unary ops (NOT in CALL args, STORE addr, BR_IF cond, etc.)
        # We only propagate into arithmetic operands where a literal is valid
        propagable_keys_by_op = {
            'ADD': ('lhs', 'rhs', 'src1', 'src2'),
            'SUB': ('lhs', 'rhs', 'src1', 'src2'),
            'MUL': ('lhs', 'rhs', 'src1', 'src2'),
            'DIV': ('lhs', 'rhs', 'src1', 'src2'),
            'MOD': ('lhs', 'rhs', 'src1', 'src2'),
            'AND': ('lhs', 'rhs', 'src1', 'src2'),
            'OR':  ('lhs', 'rhs', 'src1', 'src2'),
            'XOR': ('lhs', 'rhs', 'src1', 'src2'),
            'SHL': ('lhs', 'rhs', 'src1', 'src2'),
            'SHR': ('lhs', 'rhs', 'src1', 'src2'),
            'EQ':  ('lhs', 'rhs', 'src1', 'src2'),
            'NE':  ('lhs', 'rhs', 'src1', 'src2'),
            'LT':  ('lhs', 'rhs', 'src1', 'src2'),
            'LE':  ('lhs', 'rhs', 'src1', 'src2'),
            'GT':  ('lhs', 'rhs', 'src1', 'src2'),
            'GE':  ('lhs', 'rhs', 'src1', 'src2'),
            'NEG': ('src', 'src1'),
            'NOT': ('src', 'src1'),
        }

        for instr in instrs:
            op = instr.get('op', '')
            keys = propagable_keys_by_op.get(op, ())
            for key in keys:
                val = instr.get(key)
                if val and isinstance(val, str) and val in const_map:
                    instr[key] = const_map[val]
                    total_propagated += 1

    return total_propagated


# ═══════════════════════════════════════════════════════════════════
# Pass 4: Algebraic Simplification
# ═══════════════════════════════════════════════════════════════════

def algebraic_simplify(functions):
    """
    Simplify trivial arithmetic:
      x + 0 → COPY x          x - 0 → COPY x
      x * 1 → COPY x          x * 0 → CONST 0
      x / 1 → COPY x          x & 0 → CONST 0
      x | 0 → COPY x          x ^ 0 → COPY x
      x << 0 → COPY x         x >> 0 → COPY x
      x - x → CONST 0         x ^ x → CONST 0
      x & x → COPY x          x | x → COPY x
    """
    total_simplified = 0

    for func in functions:
        instrs = func.get('instructions', [])
        new_instrs = []

        for instr in instrs:
            op = instr.get('op', '')
            dst = instr.get('dst')

            if op in BINARY_OPS and dst:
                lhs = (instr.get('lhs') if instr.get('lhs') is not None else instr.get('src1'))
                rhs = (instr.get('rhs') if instr.get('rhs') is not None else instr.get('src2'))

                replacement = None

                # Identity: x op 0 or 0 op x
                if is_const_int(rhs) and to_int(rhs) == 0:
                    if op in ('ADD', 'SUB', 'OR', 'XOR', 'SHL', 'SHR'):
                        replacement = _make_copy(dst, lhs, instr)
                    elif op in ('MUL', 'AND'):
                        replacement = _make_const(dst, 0, instr)

                elif is_const_int(lhs) and to_int(lhs) == 0:
                    if op in ('ADD', 'OR', 'XOR'):
                        replacement = _make_copy(dst, rhs, instr)
                    elif op in ('MUL', 'AND'):
                        replacement = _make_const(dst, 0, instr)

                # Identity: x op 1
                elif is_const_int(rhs) and to_int(rhs) == 1:
                    if op in ('MUL', 'DIV'):
                        replacement = _make_copy(dst, lhs, instr)

                elif is_const_int(lhs) and to_int(lhs) == 1:
                    if op == 'MUL':
                        replacement = _make_copy(dst, rhs, instr)

                # Self-ops: x op x (only when both are same temporary)
                elif (isinstance(lhs, str) and isinstance(rhs, str)
                      and lhs == rhs and lhs.startswith('%')):
                    if op == 'SUB':
                        replacement = _make_const(dst, 0, instr)
                    elif op == 'XOR':
                        replacement = _make_const(dst, 0, instr)
                    elif op == 'AND':
                        replacement = _make_copy(dst, lhs, instr)
                    elif op == 'OR':
                        replacement = _make_copy(dst, lhs, instr)
                    elif op == 'EQ':
                        replacement = _make_const(dst, 1, instr)
                    elif op == 'NE':
                        replacement = _make_const(dst, 0, instr)
                    elif op in ('LT', 'GT'):
                        replacement = _make_const(dst, 0, instr)
                    elif op in ('LE', 'GE'):
                        replacement = _make_const(dst, 1, instr)

                if replacement:
                    new_instrs.append(replacement)
                    total_simplified += 1
                    continue

            new_instrs.append(instr)

        func['instructions'] = new_instrs

    return total_simplified


def _make_copy(dst, src, orig):
    """Create a COPY instruction preserving metadata."""
    r = {'op': 'COPY', 'dst': dst, 'src': src}
    if 'type' in orig:
        r['type'] = orig['type']
    if 'line' in orig:
        r['line'] = orig['line']
    return r


def _make_const(dst, val, orig):
    """Create a CONST instruction preserving metadata."""
    r = {'op': 'CONST', 'dst': dst, 'value': val}
    if 'type' in orig:
        r['type'] = orig['type']
    if 'line' in orig:
        r['line'] = orig['line']
    return r


# ═══════════════════════════════════════════════════════════════════
# Pass 5: Dead Store Elimination
# ═══════════════════════════════════════════════════════════════════

def dead_store_elimination(functions):
    """
    Eliminate STORE instructions to stack locations that are
    never LOADed afterward within the same function.

    Conservative: only eliminates stores to named local variables
    (not pointer-indirect stores, which may alias).
    """
    total_eliminated = 0

    for func in functions:
        instrs = func.get('instructions', [])

        # Collect all LOAD targets (addresses being read)
        loaded_addrs = set()
        for instr in instrs:
            if instr.get('op') == 'LOAD':
                addr = instr.get('addr') or (instr.get('src') if instr.get('src') is not None else instr.get('src1'))
                if addr and isinstance(addr, str):
                    loaded_addrs.add(addr)

        # Also conservatively keep any store whose addr appears in
        # any other operand position (could be passed to CALL, etc.)
        used_anywhere = set()
        for instr in instrs:
            for _, val in get_operands(instr):
                used_anywhere.add(val)

        # Eliminate stores to addresses that are never loaded or used
        new_instrs = []
        for instr in instrs:
            if instr.get('op') == 'STORE':
                addr = (instr.get('addr') if instr.get('addr') is not None else instr.get('dst'))
                if (addr and isinstance(addr, str) and addr.startswith('%')
                        and addr not in loaded_addrs
                        and addr not in used_anywhere):
                    total_eliminated += 1
                    continue
            new_instrs.append(instr)

        func['instructions'] = new_instrs

    return total_eliminated


# ═══════════════════════════════════════════════════════════════════
# Pass 6: Final DCE Sweep
# ═══════════════════════════════════════════════════════════════════

def dce_sweep(functions):
    """
    Remove instructions that produce unused results.
    Iterates until fixpoint within each function.
    """
    total_eliminated = 0

    for func in functions:
        changed = True
        while changed:
            changed = False
            instrs = func.get('instructions', [])

            # Collect all used temporaries
            used = set()
            for instr in instrs:
                for _, val in get_operands(instr):
                    used.add(val)

            new_instrs = []
            for instr in instrs:
                op = instr.get('op', '')
                dst = get_dst(instr)

                # Keep side-effecting ops unconditionally
                if op in SIDE_EFFECT_OPS:
                    new_instrs.append(instr)
                    continue

                # Keep if result is used
                if dst and dst in used:
                    new_instrs.append(instr)
                    continue

                # Keep if no dst (shouldn't happen for RESULT_OPS, but safety)
                if not dst:
                    new_instrs.append(instr)
                    continue

                # Dead — eliminate
                total_eliminated += 1
                changed = True

            func['instructions'] = new_instrs

    return total_eliminated


# ═══════════════════════════════════════════════════════════════════
# Pipeline Orchestrator
# ═══════════════════════════════════════════════════════════════════

def count_instructions(functions):
    """Total instruction count across all functions."""
    return sum(len(f.get('instructions', [])) for f in functions)


def run_pipeline(ir_data, max_iterations=10):
    """
    Run all passes iteratively until fixpoint.
    Returns (optimized_data, stats).
    """
    functions = ir_data if isinstance(ir_data, list) else ir_data.get('functions', ir_data)

    initial_count = count_instructions(functions)
    stats = {
        'initial_instructions': initial_count,
        'passes': [],
        'iterations': 0,
    }

    cumulative = {
        'constant_fold': 0,
        'copy_propagation': 0,
        'constant_propagation': 0,
        'algebraic_simplify': 0,
        'dead_store_elimination': 0,
        'dce_sweep': 0,
    }

    for iteration in range(1, max_iterations + 1):
        stats['iterations'] = iteration
        iter_changes = 0

        # Pass 1: Constant Folding
        n = constant_fold(functions)
        cumulative['constant_fold'] += n
        iter_changes += n

        # Pass 2: Copy Propagation
        n = copy_propagation(functions)
        cumulative['copy_propagation'] += n
        iter_changes += n

        # Pass 3: Constant Propagation
        n = constant_propagation(functions)
        cumulative['constant_propagation'] += n
        iter_changes += n

        # Pass 4: Algebraic Simplification
        n = algebraic_simplify(functions)
        cumulative['algebraic_simplify'] += n
        iter_changes += n

        # Pass 5: Dead Store Elimination
        n = dead_store_elimination(functions)
        cumulative['dead_store_elimination'] += n
        iter_changes += n

        # Pass 6: DCE Sweep (clean up dead results from above)
        n = dce_sweep(functions)
        cumulative['dce_sweep'] += n
        iter_changes += n

        if iter_changes == 0:
            break

    final_count = count_instructions(functions)
    total_eliminated = initial_count - final_count

    stats['final_instructions'] = final_count
    stats['total_eliminated'] = total_eliminated
    stats['reduction_pct'] = (total_eliminated / initial_count * 100) if initial_count > 0 else 0
    stats['cumulative'] = cumulative

    return functions, stats


# ═══════════════════════════════════════════════════════════════════
# PRIME Energy Scoring (Bonus)
# ═══════════════════════════════════════════════════════════════════

def prime_energy_score(functions):
    """
    Assign PRIME Hamiltonian energy to each function based on IR complexity.

    H₀ = weighted instruction count
    Higher energy = more optimization opportunity / higher vulnerability surface.
    """
    # Opcode weights (higher = more computational cost)
    weights = {
        'CALL': 8.0,    # function calls are expensive + vulnerability surface
        'STORE': 3.0,   # memory writes
        'LOAD': 2.5,    # memory reads
        'DIV': 4.0,     # division is slow
        'MOD': 4.0,
        'MUL': 2.0,
        'BR_IF': 2.0,   # conditional branches = complexity
        'BR': 1.0,
        'ADD': 1.0, 'SUB': 1.0,
        'AND': 1.0, 'OR': 1.0, 'XOR': 1.0,
        'SHL': 1.5, 'SHR': 1.5,
        'EQ': 1.0, 'NE': 1.0, 'LT': 1.0, 'LE': 1.0, 'GT': 1.0, 'GE': 1.0,
        'CONST': 0.5, 'COPY': 0.5,
        'CAST': 1.0, 'NEG': 1.0, 'NOT': 1.0,
        'RET': 0.5, 'LABEL': 0.1, 'ARG': 1.0,
        'ALLOCA': 1.5, 'PHI': 1.5,
        'CONST_STR': 0.5, 'NOP': 0.0,
    }

    scores = []
    for func in functions:
        name = func.get('name', '<unknown>')
        instrs = func.get('instructions', [])
        n = len(instrs)

        # Base energy: weighted sum
        h0 = sum(weights.get(i.get('op', 'NOP'), 1.0) for i in instrs)

        # Branch density (complexity indicator)
        branches = sum(1 for i in instrs if i.get('op') in ('BR', 'BR_IF'))
        branch_density = branches / max(n, 1)

        # Call density (vulnerability surface)
        calls = sum(1 for i in instrs if i.get('op') == 'CALL')
        call_density = calls / max(n, 1)

        # PRIME composite energy
        energy = h0 * (1.0 + 0.3 * branch_density + 0.5 * call_density)

        scores.append({
            'function': name,
            'instructions': n,
            'base_cost': round(h0, 2),
            'branch_density': round(branch_density, 4),
            'call_density': round(call_density, 4),
            'prime_energy': round(energy, 2),
        })

    scores.sort(key=lambda x: -x['prime_energy'])
    return scores


# ═══════════════════════════════════════════════════════════════════
# Opcode Histogram
# ═══════════════════════════════════════════════════════════════════

def opcode_histogram(functions):
    """Count instructions by opcode across all functions."""
    hist = defaultdict(int)
    for func in functions:
        for instr in func.get('instructions', []):
            hist[instr.get('op', 'UNKNOWN')] += 1
    return dict(sorted(hist.items(), key=lambda x: -x[1]))


# ═══════════════════════════════════════════════════════════════════
# Main
# ═══════════════════════════════════════════════════════════════════

def main():
    input_path = sys.argv[1] if len(sys.argv) > 1 else 'zcc_ir_optimized.json'
    output_path = sys.argv[2] if len(sys.argv) > 2 else 'zcc_ir_final.json'

    if not Path(input_path).exists():
        # Fallback to raw IR
        if Path('zcc_ir.json').exists():
            input_path = 'zcc_ir.json'
            print(f"[WARN] {sys.argv[1] if len(sys.argv) > 1 else 'zcc_ir_optimized.json'} not found, using zcc_ir.json")
        else:
            print(f"[ERROR] No IR input found. Run 'make ir-self' first.")
            sys.exit(1)

    print(f"🔱 ZCC IR Optimization Pipeline v1.0.0")
    print(f"{'='*60}")
    print(f"  Input:  {input_path}")
    print(f"  Output: {output_path}")
    print()

    with open(input_path) as f:
        ir_data = json.load(f)

    # Handle both list and dict formats
    if isinstance(ir_data, dict):
        functions = ir_data.get('functions', [])
    elif isinstance(ir_data, list):
        functions = ir_data
    else:
        print("[ERROR] Unexpected IR format")
        sys.exit(1)

    print(f"  Functions:    {len(functions)}")
    print(f"  Instructions: {count_instructions(functions)}")
    print()

    # Deep copy for safety
    functions = copy.deepcopy(functions)

    # Run pipeline
    optimized, stats = run_pipeline(functions)

    # Print results
    print(f"{'='*60}")
    print(f"  OPTIMIZATION RESULTS")
    print(f"{'='*60}")
    print(f"  Iterations to fixpoint: {stats['iterations']}")
    print(f"  Initial instructions:   {stats['initial_instructions']}")
    print(f"  Final instructions:     {stats['final_instructions']}")
    print(f"  Total eliminated:       {stats['total_eliminated']}")
    print(f"  Reduction:              {stats['reduction_pct']:.2f}%")
    print()
    print(f"  Pass Breakdown:")
    for pass_name, count in stats['cumulative'].items():
        if count > 0:
            print(f"    {pass_name:<30} {count:>6} changes")
    print()

    # Opcode histogram
    hist = opcode_histogram(optimized)
    print(f"  Opcode Distribution (post-optimization):")
    for op, count in list(hist.items())[:15]:
        bar = '█' * min(count // 20, 40)
        print(f"    {op:<12} {count:>5}  {bar}")
    print()

    # PRIME energy scoring
    scores = prime_energy_score(optimized)
    print(f"  🔱 PRIME Energy Rankings (Top 10):")
    print(f"    {'Function':<40} {'Instr':>6} {'H₀':>8} {'Energy':>8}")
    print(f"    {'-'*66}")
    for s in scores[:10]:
        print(f"    {s['function']:<40} {s['instructions']:>6} {s['base_cost']:>8.1f} {s['prime_energy']:>8.1f}")
    print()

    # Write output
    output_data = {
        'functions': optimized,
        'stats': stats,
        'prime_energy': scores,
        'opcode_histogram': hist,
    }

    with open(output_path, 'w') as f:
        json.dump(output_data, f, indent=2)

    print(f"  ✅ Written: {output_path}")
    print(f"     {len(optimized)} functions, {stats['final_instructions']} instructions")

    # Summary line for Ouroboros scraping
    print(f"\n🔱 ZCC-OPT: {stats['initial_instructions']} → {stats['final_instructions']} "
          f"(-{stats['total_eliminated']}, -{stats['reduction_pct']:.1f}%) "
          f"[CF:{stats['cumulative']['constant_fold']} "
          f"CP:{stats['cumulative']['copy_propagation']} "
          f"CProp:{stats['cumulative']['constant_propagation']} "
          f"AS:{stats['cumulative']['algebraic_simplify']} "
          f"DSE:{stats['cumulative']['dead_store_elimination']} "
          f"DCE:{stats['cumulative']['dce_sweep']}]")


if __name__ == '__main__':
    main()
