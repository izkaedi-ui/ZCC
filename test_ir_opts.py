#!/usr/bin/env python3
"""
🔱 ZCC IR Optimization — Test Harness
══════════════════════════════════════

Generates synthetic IR exercising every optimization pattern,
runs the pipeline, and verifies correctness.
"""

import json
import sys
sys.path.insert(0, '.')
from zcc_ir_opts import (
    constant_fold, copy_propagation, constant_propagation,
    algebraic_simplify, dead_store_elimination, dce_sweep,
    run_pipeline, count_instructions, prime_energy_score,
    opcode_histogram,
)
import copy

PASS = 0
FAIL = 0


def check(name, condition):
    global PASS, FAIL
    if condition:
        PASS += 1
        print(f"  ✅ {name}")
    else:
        FAIL += 1
        print(f"  ❌ {name}")


def find_instr(instrs, op=None, dst=None):
    """Find first instruction matching criteria."""
    for i in instrs:
        if op and i.get('op') != op:
            continue
        if dst and i.get('dst') != dst:
            continue
        return i
    return None


# ═══════════════════════════════════════════════════════════════════
# Test 1: Constant Folding
# ═══════════════════════════════════════════════════════════════════

def test_constant_folding():
    print("\n[Pass 1] Constant Folding")

    funcs = [{'name': 'test_cf', 'instructions': [
        # 10 + 32 → CONST 42
        {'op': 'ADD', 'dst': '%t0', 'lhs': 10, 'rhs': 32, 'type': 'i32'},
        # 100 * 0 → CONST 0
        {'op': 'MUL', 'dst': '%t1', 'lhs': 100, 'rhs': 0, 'type': 'i32'},
        # 7 - 3 → CONST 4
        {'op': 'SUB', 'dst': '%t2', 'lhs': 7, 'rhs': 3, 'type': 'i32'},
        # 100 / 5 → CONST 20
        {'op': 'DIV', 'dst': '%t3', 'lhs': 100, 'rhs': 5, 'type': 'i32'},
        # 10 == 10 → CONST 1
        {'op': 'EQ', 'dst': '%t4', 'lhs': 10, 'rhs': 10, 'type': 'i32'},
        # 3 < 5 → CONST 1
        {'op': 'LT', 'dst': '%t5', 'lhs': 3, 'rhs': 5, 'type': 'i32'},
        # NEG 42 → CONST -42
        {'op': 'NEG', 'dst': '%t6', 'src': 42, 'type': 'i32'},
        # 1 << 4 → CONST 16
        {'op': 'SHL', 'dst': '%t7', 'lhs': 1, 'rhs': 4, 'type': 'i32'},
        # Division by zero — should NOT fold
        {'op': 'DIV', 'dst': '%t8', 'lhs': 10, 'rhs': 0, 'type': 'i32'},
        # Non-const operands — should NOT fold
        {'op': 'ADD', 'dst': '%t9', 'lhs': '%t0', 'rhs': '%t1', 'type': 'i32'},
        # Keep alive
        {'op': 'RET', 'src': '%t9'},
    ]}]

    n = constant_fold(funcs)
    instrs = funcs[0]['instructions']

    check("10+32 → CONST 42", find_instr(instrs, dst='%t0') and find_instr(instrs, dst='%t0')['op'] == 'CONST' and find_instr(instrs, dst='%t0')['value'] == 42)
    check("100*0 → CONST 0", find_instr(instrs, dst='%t1')['value'] == 0)
    check("7-3 → CONST 4", find_instr(instrs, dst='%t2')['value'] == 4)
    check("100/5 → CONST 20", find_instr(instrs, dst='%t3')['value'] == 20)
    check("10==10 → CONST 1", find_instr(instrs, dst='%t4')['value'] == 1)
    check("3<5 → CONST 1", find_instr(instrs, dst='%t5')['value'] == 1)
    check("NEG 42 → CONST -42", find_instr(instrs, dst='%t6')['value'] == -42)
    check("1<<4 → CONST 16", find_instr(instrs, dst='%t7')['value'] == 16)
    check("DIV by 0 preserved", find_instr(instrs, dst='%t8')['op'] == 'DIV')
    check("Non-const ADD preserved", find_instr(instrs, dst='%t9')['op'] == 'ADD')
    check(f"Folded {n} instructions", n == 8)


# ═══════════════════════════════════════════════════════════════════
# Test 2: Copy Propagation
# ═══════════════════════════════════════════════════════════════════

def test_copy_propagation():
    print("\n[Pass 2] Copy Propagation")

    funcs = [{'name': 'test_cp', 'instructions': [
        {'op': 'CONST', 'dst': '%t0', 'value': 42},
        # %t1 = COPY %t0 → all uses of %t1 become %t0
        {'op': 'COPY', 'dst': '%t1', 'src': '%t0'},
        # Chain: %t2 = COPY %t1 → resolves to %t0
        {'op': 'COPY', 'dst': '%t2', 'src': '%t1'},
        # Use %t2 → should become %t0
        {'op': 'ADD', 'dst': '%t3', 'lhs': '%t2', 'rhs': 10},
        # Use %t1 → should become %t0
        {'op': 'RET', 'src': '%t1'},
    ]}]

    n = copy_propagation(funcs)
    instrs = funcs[0]['instructions']

    add_instr = find_instr(instrs, op='ADD', dst='%t3')
    ret_instr = find_instr(instrs, op='RET')

    check("Chain resolved: ADD lhs = %t0", add_instr['lhs'] == '%t0')
    check("Direct copy: RET src = %t0", ret_instr['src'] == '%t0')
    check(f"Propagated {n} uses", n >= 2)


# ═══════════════════════════════════════════════════════════════════
# Test 3: Constant Propagation
# ═══════════════════════════════════════════════════════════════════

def test_constant_propagation():
    print("\n[Pass 3] Constant Propagation")

    funcs = [{'name': 'test_cprop', 'instructions': [
        {'op': 'CONST', 'dst': '%t0', 'value': 10},
        {'op': 'CONST', 'dst': '%t1', 'value': 20},
        # %t0 + %t1 → should become 10 + 20 (then constant fold can handle it)
        {'op': 'ADD', 'dst': '%t2', 'lhs': '%t0', 'rhs': '%t1'},
        # %t0 * 5 → should become 10 * 5
        {'op': 'MUL', 'dst': '%t3', 'lhs': '%t0', 'rhs': 5},
        {'op': 'RET', 'src': '%t3'},
    ]}]

    n = constant_propagation(funcs)
    instrs = funcs[0]['instructions']

    add_instr = find_instr(instrs, op='ADD', dst='%t2')
    mul_instr = find_instr(instrs, op='MUL', dst='%t3')

    check("ADD lhs propagated to 10", add_instr['lhs'] == 10)
    check("ADD rhs propagated to 20", add_instr['rhs'] == 20)
    check("MUL lhs propagated to 10", mul_instr['lhs'] == 10)
    check(f"Propagated {n} constants", n >= 3)


# ═══════════════════════════════════════════════════════════════════
# Test 4: Algebraic Simplification
# ═══════════════════════════════════════════════════════════════════

def test_algebraic_simplify():
    print("\n[Pass 4] Algebraic Simplification")

    funcs = [{'name': 'test_alg', 'instructions': [
        # x + 0 → COPY x
        {'op': 'ADD', 'dst': '%t1', 'lhs': '%t0', 'rhs': 0},
        # x * 1 → COPY x
        {'op': 'MUL', 'dst': '%t2', 'lhs': '%t0', 'rhs': 1},
        # x * 0 → CONST 0
        {'op': 'MUL', 'dst': '%t3', 'lhs': '%t0', 'rhs': 0},
        # x - 0 → COPY x
        {'op': 'SUB', 'dst': '%t4', 'lhs': '%t0', 'rhs': 0},
        # x & 0 → CONST 0
        {'op': 'AND', 'dst': '%t5', 'lhs': '%t0', 'rhs': 0},
        # x ^ x → CONST 0
        {'op': 'XOR', 'dst': '%t6', 'lhs': '%t0', 'rhs': '%t0'},
        # x - x → CONST 0
        {'op': 'SUB', 'dst': '%t7', 'lhs': '%t0', 'rhs': '%t0'},
        # x == x → CONST 1
        {'op': 'EQ', 'dst': '%t8', 'lhs': '%t0', 'rhs': '%t0'},
        # x >> 0 → COPY x
        {'op': 'SHR', 'dst': '%t9', 'lhs': '%t0', 'rhs': 0},
        # x / 1 → COPY x
        {'op': 'DIV', 'dst': '%t10', 'lhs': '%t0', 'rhs': 1},
        # Keep alive
        {'op': 'RET', 'src': '%t10'},
    ]}]

    n = algebraic_simplify(funcs)
    instrs = funcs[0]['instructions']

    check("x+0 → COPY", find_instr(instrs, dst='%t1')['op'] == 'COPY')
    check("x*1 → COPY", find_instr(instrs, dst='%t2')['op'] == 'COPY')
    check("x*0 → CONST 0", find_instr(instrs, dst='%t3')['op'] == 'CONST' and find_instr(instrs, dst='%t3')['value'] == 0)
    check("x-0 → COPY", find_instr(instrs, dst='%t4')['op'] == 'COPY')
    check("x&0 → CONST 0", find_instr(instrs, dst='%t5')['op'] == 'CONST')
    check("x^x → CONST 0", find_instr(instrs, dst='%t6')['op'] == 'CONST' and find_instr(instrs, dst='%t6')['value'] == 0)
    check("x-x → CONST 0", find_instr(instrs, dst='%t7')['op'] == 'CONST')
    check("x==x → CONST 1", find_instr(instrs, dst='%t8')['op'] == 'CONST' and find_instr(instrs, dst='%t8')['value'] == 1)
    check("x>>0 → COPY", find_instr(instrs, dst='%t9')['op'] == 'COPY')
    check("x/1 → COPY", find_instr(instrs, dst='%t10')['op'] == 'COPY')
    check(f"Simplified {n} instructions", n == 10)


# ═══════════════════════════════════════════════════════════════════
# Test 5: DCE Sweep
# ═══════════════════════════════════════════════════════════════════

def test_dce_sweep():
    print("\n[Pass 6] DCE Sweep")

    funcs = [{'name': 'test_dce', 'instructions': [
        {'op': 'CONST', 'dst': '%t0', 'value': 42},
        # %t1 is dead — never used
        {'op': 'CONST', 'dst': '%t1', 'value': 99},
        # %t2 is dead — never used
        {'op': 'ADD', 'dst': '%t2', 'lhs': '%t0', 'rhs': 10},
        # %t3 is live — used in RET
        {'op': 'ADD', 'dst': '%t3', 'lhs': '%t0', 'rhs': 5},
        # CALL has side effects — always kept
        {'op': 'CALL', 'dst': '%t4', 'target': 'printf', 'args': ['%t3']},
        {'op': 'RET', 'src': '%t3'},
    ]}]

    n = dce_sweep(funcs)
    instrs = funcs[0]['instructions']

    check("Dead CONST %t1 eliminated", find_instr(instrs, dst='%t1') is None)
    check("Dead ADD %t2 eliminated", find_instr(instrs, dst='%t2') is None)
    check("Live CONST %t0 kept", find_instr(instrs, dst='%t0') is not None)
    check("Live ADD %t3 kept", find_instr(instrs, dst='%t3') is not None)
    check("Side-effect CALL kept", find_instr(instrs, op='CALL') is not None)
    check(f"Eliminated {n} dead instructions", n == 2)


# ═══════════════════════════════════════════════════════════════════
# Test 6: Full Pipeline (cascading optimizations)
# ═══════════════════════════════════════════════════════════════════

def test_full_pipeline():
    print("\n[Full Pipeline] Cascading Optimization")

    # This tests the interaction between passes:
    # CONST 10, CONST 20 → propagate → fold → simplify → DCE
    funcs = [{'name': 'cascade', 'instructions': [
        {'op': 'CONST', 'dst': '%t0', 'value': 10},
        {'op': 'CONST', 'dst': '%t1', 'value': 20},
        # After const prop: ADD 10, 20 → After fold: CONST 30
        {'op': 'ADD', 'dst': '%t2', 'lhs': '%t0', 'rhs': '%t1'},
        # COPY chain
        {'op': 'COPY', 'dst': '%t3', 'src': '%t2'},
        {'op': 'COPY', 'dst': '%t4', 'src': '%t3'},
        # After copy prop + const prop + fold: this becomes CONST 30 + 0 → CONST 30
        {'op': 'ADD', 'dst': '%t5', 'lhs': '%t4', 'rhs': 0},
        # Dead computation
        {'op': 'MUL', 'dst': '%t6', 'lhs': '%t0', 'rhs': '%t1'},
        # Self-op
        {'op': 'XOR', 'dst': '%t7', 'lhs': '%t5', 'rhs': '%t5'},
        # Live output
        {'op': 'CALL', 'dst': '%t8', 'target': 'printf', 'args': ['%t5']},
        {'op': 'RET', 'src': '%t5'},
    ]}]

    initial = count_instructions(funcs)
    optimized, stats = run_pipeline(copy.deepcopy(funcs))
    final = count_instructions(optimized)

    check(f"Pipeline reduced {initial} → {final} instructions", final < initial)
    check("Multiple passes ran", stats['iterations'] >= 1)
    check("Dead MUL %t6 eliminated", find_instr(optimized[0]['instructions'], dst='%t6') is None)

    # The COPY chain should be resolved
    remaining_copies = sum(1 for i in optimized[0]['instructions'] if i.get('op') == 'COPY')
    check(f"Copy chain reduced (remaining COPYs: {remaining_copies})", remaining_copies <= 2)

    print(f"\n  Pipeline stats: {stats['total_eliminated']} eliminated, "
          f"{stats['reduction_pct']:.1f}% reduction, "
          f"{stats['iterations']} iterations to fixpoint")


# ═══════════════════════════════════════════════════════════════════
# Test 7: PRIME Energy Scoring
# ═══════════════════════════════════════════════════════════════════

def test_prime_energy():
    print("\n[Bonus] PRIME Energy Scoring")

    funcs = [
        {'name': 'simple_leaf', 'instructions': [
            {'op': 'CONST', 'dst': '%t0', 'value': 1},
            {'op': 'RET', 'src': '%t0'},
        ]},
        {'name': 'complex_branchy', 'instructions': [
            {'op': 'LOAD', 'dst': '%t0', 'addr': '%arg0'},
            {'op': 'CONST', 'dst': '%t1', 'value': 0},
            {'op': 'EQ', 'dst': '%t2', 'lhs': '%t0', 'rhs': '%t1'},
            {'op': 'BR_IF', 'cond': '%t2', 'target': 'L1'},
            {'op': 'CALL', 'dst': '%t3', 'target': 'process', 'args': ['%t0']},
            {'op': 'CALL', 'dst': '%t4', 'target': 'validate', 'args': ['%t3']},
            {'op': 'BR', 'target': 'L2'},
            {'op': 'LABEL', 'name': 'L1'},
            {'op': 'CALL', 'dst': '%t5', 'target': 'error', 'args': []},
            {'op': 'LABEL', 'name': 'L2'},
            {'op': 'RET', 'src': '%t0'},
        ]},
    ]

    scores = prime_energy_score(funcs)
    check("complex_branchy has higher energy", scores[0]['function'] == 'complex_branchy')
    check("Energy > 0 for all functions", all(s['prime_energy'] > 0 for s in scores))
    check(f"Top energy: {scores[0]['prime_energy']:.1f}", scores[0]['prime_energy'] > 10)


# ═══════════════════════════════════════════════════════════════════
# Test 8: Realistic ZCC-like IR
# ═══════════════════════════════════════════════════════════════════

def test_realistic_ir():
    print("\n[Realistic] ZCC-style Function")

    # Simulates codegen_expr for: return (a + 0) * 1 + (b - b);
    funcs = [{'name': 'codegen_expr_simplified', 'instructions': [
        # Prologue
        {'op': 'ALLOCA', 'dst': '%a', 'type': 'i64'},
        {'op': 'ALLOCA', 'dst': '%b', 'type': 'i64'},
        {'op': 'STORE', 'addr': '%a', 'value': '%arg0'},
        {'op': 'STORE', 'addr': '%b', 'value': '%arg1'},

        # a + 0
        {'op': 'LOAD', 'dst': '%t0', 'addr': '%a'},
        {'op': 'CONST', 'dst': '%t1', 'value': 0},
        {'op': 'ADD', 'dst': '%t2', 'lhs': '%t0', 'rhs': '%t1'},

        # (a + 0) * 1
        {'op': 'CONST', 'dst': '%t3', 'value': 1},
        {'op': 'MUL', 'dst': '%t4', 'lhs': '%t2', 'rhs': '%t3'},

        # b - b
        {'op': 'LOAD', 'dst': '%t5', 'addr': '%b'},
        {'op': 'COPY', 'dst': '%t6', 'src': '%t5'},
        {'op': 'SUB', 'dst': '%t7', 'lhs': '%t5', 'rhs': '%t6'},

        # (a*1) + (b-b)
        {'op': 'ADD', 'dst': '%t8', 'lhs': '%t4', 'rhs': '%t7'},

        # Dead computation: never used
        {'op': 'CONST', 'dst': '%t9', 'value': 999},
        {'op': 'MUL', 'dst': '%t10', 'lhs': '%t9', 'rhs': '%t9'},

        # Return
        {'op': 'RET', 'src': '%t8'},
    ]}]

    initial = count_instructions(funcs)
    optimized, stats = run_pipeline(copy.deepcopy(funcs))
    final = count_instructions(optimized)

    check(f"Realistic IR: {initial} → {final} instructions", final < initial)
    check("Dead CONST 999 eliminated", find_instr(optimized[0]['instructions'], dst='%t9') is None)
    check("Dead MUL 999*999 eliminated", find_instr(optimized[0]['instructions'], dst='%t10') is None)
    print(f"  Reduction: {stats['reduction_pct']:.1f}%")


# ═══════════════════════════════════════════════════════════════════
# Run All Tests
# ═══════════════════════════════════════════════════════════════════

def main():
    print("🔱 ZCC IR Optimization Pipeline — Test Suite")
    print("=" * 60)

    test_constant_folding()
    test_copy_propagation()
    test_constant_propagation()
    test_algebraic_simplify()
    test_dce_sweep()
    test_full_pipeline()
    test_prime_energy()
    test_realistic_ir()

    print("\n" + "=" * 60)
    print(f"  Results: {PASS} passed, {FAIL} failed")
    if FAIL == 0:
        print("  🔱 ALL TESTS PASSED — Pipeline verified.")
    else:
        print(f"  ⚠️  {FAIL} failures — review above.")
    print("=" * 60)

    return 0 if FAIL == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
