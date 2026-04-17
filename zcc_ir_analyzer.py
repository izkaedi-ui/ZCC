#!/usr/bin/env python3
import sys
import json
import re

def analyze_ir(filepath):
    """
    Parses a ZCC IR file and extracts topological features simulating ZKAEDI Hamiltonian mapping.
    """
    nodes = []
    edges = []
    op_counts = {}
    
    # Energy topology metrics
    memory_ops = 0
    branch_ops = 0
    math_ops = 0
    bitwise_ops = 0
    energy_cost = 0.0
    
    current_func = None
    
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
    except Exception as e:
        print(f"ERROR: Could not read {filepath}: {e}")
        sys.exit(1)

    for idx, line in enumerate(lines):
        line = line.strip()
        if not line or line.startswith(';'):
            if line.startswith('; func '):
                current_func = line.split()[2]
            continue
            
        parts = re.split(r'\s+', line)
        op = parts[0].upper()
        
        # Track basic frequencies
        op_counts[op] = op_counts.get(op, 0) + 1
        
        # Categorical density
        if op in ['LOAD', 'STORE', 'ALLOCA', 'ADDR']:
            memory_ops += 1
            energy_cost += 3.5  # Memory accesses are thermodynamically expensive
        elif op in ['BR', 'BR_IF', 'RET', 'CALL']:
            branch_ops += 1
            energy_cost += 2.0  # Branch cost
        elif op in ['ADD', 'SUB', 'MUL', 'DIV', 'MOD', 'CONST']:
            math_ops += 1
            energy_cost += 1.2
        elif op in ['AND', 'OR', 'XOR', 'SHL', 'SHR']:
            bitwise_ops += 1
            energy_cost += 0.8  # Bitwise paths are lower energy
            
        nodes.append({
            "id": idx,
            "func": current_func,
            "op": op,
            "raw": line
        })

    # Output strict topological bindings
    total_ops = len(nodes)
    if total_ops == 0:
        total_ops = 1 # avoid div by zero

    print("\n[ZKAEDI TOPOLOGICAL IR EXTRACTOR]")
    print("===================================")
    print(f"Target Bound: {filepath}")
    print(f"Total Graph Nodes: {total_ops}")
    print(f"Estimated Thermodynamic Resistance (Energy Cost): {energy_cost:.2f} cycles")
    print("\n[Feature Distribution]")
    print(f" -> Memory Density:  {(memory_ops / total_ops)*100:.1f}%")
    print(f" -> Branch Entropy:  {(branch_ops / total_ops)*100:.1f}%")
    print(f" -> Math Load:       {(math_ops / total_ops)*100:.1f}%")
    print(f" -> Bitwise Density: {(bitwise_ops / total_ops)*100:.1f}%")
    print("===================================\n")
    print("Topological ingestion optimal. Manifold mapped for downstream Python mutation.\n")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python zcc_ir_analyzer.py <file.ir>")
        sys.exit(1)
    analyze_ir(sys.argv[1])
