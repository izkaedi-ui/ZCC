import json
import math
import sys

def build_bridge(ir_path, out_path):
    print(f"Loading verified ground-truth from {ir_path}...")
    try:
        with open(ir_path, 'r') as f:
            data = json.load(f)
    except Exception as e:
        print(f"Failed to load JSON: {e}")
        return

    features = []
    
    # Global normalization constants (calibrate against the ZCC dataset)
    # H0 energy calculation (a baseline clean signature):
    # We construct a Hamiltonian model where:
    # 𝐻₀ = E_kinetic + E_potential
    # E_kinetic = related to runtime jumps (branching)
    # E_potential = memory interactions (load/store)
    
    for fn in data:
        name = fn.get("func", "unknown")
        insts = fn.get("instructions", fn.get("body", []))
        total_inst = len(insts)
        
        if total_inst == 0:
            continue
            
        load_ct = 0
        store_ct = 0
        branch_ct = 0
        call_ct = 0
        math_ct = 0
        
        for i in insts:
            op = i.get("op", "")
            if op == "IR_LOAD": load_ct += 1
            elif op == "IR_STORE": store_ct += 1
            elif op in ["IR_BR", "IR_BR_IF"]: branch_ct += 1
            elif op == "IR_CALL": call_ct += 1
            elif op in ["IR_ADD", "IR_SUB", "IR_MUL", "IR_DIV", "IR_AND", "IR_OR", "IR_XOR", "IR_SHL", "IR_SHR"]:
                math_ct += 1
                
        # Metrics
        branch_density = branch_ct / total_inst
        call_density = call_ct / total_inst
        math_density = math_ct / total_inst
        ls_ratio = load_ct / (store_ct + 1.0) # +1 smoothing
        
        # H0 Energy Calculation (Baseline state formulation)
        # Verified ZCC IR acts as the ground truth "cooling" baseline
        # Lower H0 indicates highly cohesive, necessary structural logic
        kinetic_energy = (branch_density * 0.4) + (call_density * 0.6)
        potential_energy = math.log1p(ls_ratio) * 0.3 + (math_density * 0.2)
        
        h0_energy = round(kinetic_energy + potential_energy, 4)
        
        features.append({
            "target": name,
            "signature_type": "ground_truth_clean",
            "metrics": {
                "instruction_count": total_inst,
                "branch_density": round(branch_density, 4),
                "call_density": round(call_density, 4),
                "math_density": round(math_density, 4),
                "load_store_ratio": round(ls_ratio, 4)
            },
            "prime_energy_H0": h0_energy
        })

    # Output array
    print(f"Extracted features for {len(features)} functions.")
    
    with open(out_path, 'w') as f:
        json.dump(features, f, indent=2)
        
    print(f"Hamiltonian bridge tensor saved to {out_path}.")
    
    # Statistical analysis of the clean dataset
    avg_h0 = sum(x["prime_energy_H0"] for x in features) / len(features)
    print(f"Ground-Truth Baseline ZCC Energy (Mean H₀): {avg_h0:.4f}")

if __name__ == "__main__":
    in_file = sys.argv[1] if len(sys.argv) > 1 else "zcc_ir_optimized.json"
    out_file = sys.argv[2] if len(sys.argv) > 2 else "zcc_prime_features.json"
    build_bridge(in_file, out_file)
