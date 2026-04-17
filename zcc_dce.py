#!/usr/bin/env python3
import json
import sys
import os

def error_exit(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)

def main():
    if len(sys.argv) >= 3:
        input_file = sys.argv[1]
        output_file = sys.argv[2]
    else:
        input_file = "zcc_ir.json"
        output_file = "zcc_ir_optimized.json"
        
    report_file = "dce_report.json"

    if not os.path.exists(input_file):
        error_exit(f"Error: Could not find input file '{input_file}'")
        
    try:
        with open(input_file, 'r') as f:
            ir_data = json.load(f)
    except Exception as e:
        error_exit(f"Error parsing JSON: {e}")

    # Determine schema variant
    # We normalized it to have "func", "instructions"
    
    total_fns = len(ir_data)
    total_inst_before = 0
    total_inst_after = 0
    total_eliminated = 0
    
    per_function_stats = []
    elim_by_opcode = {}

    side_effecting_ops = {
        'IR_RET', 'IR_BR', 'IR_BR_IF', 'IR_STORE', 
        'IR_CALL', 'IR_ARG', 'IR_LABEL'
    }
    
    # Track safety auditing
    safety_audit = {
        "calls_preserved": True,
        "stores_preserved": True,
        "branches_preserved": True,
        "returns_preserved": True,
        "labels_preserved": True,
        "args_preserved": True
    }
    
    def get_uses(inst):
        uses = set()
        for field in ['src1', 'src2', 'src3']:
            if field in inst and isinstance(inst[field], str) and inst[field].startswith('%'):
                uses.add(inst[field])
                
        # For STORE, the 'dst' field holds the memory address being written to
        if inst.get('op') == 'IR_STORE':
            if 'dst' in inst and isinstance(inst['dst'], str) and inst['dst'].startswith('%'):
                uses.add(inst['dst'])
                
        return uses

    def get_def(inst):
        if inst.get('op') == 'IR_STORE':
            return None
        if 'dst' in inst and isinstance(inst['dst'], str) and inst['dst'].startswith('%'):
            return inst['dst']
        return None

    optimized_ir = []

    for fn_idx, func in enumerate(ir_data):
        fn_name = func.get("func", func.get("name", f"unknown_fn_{fn_idx}"))
        insts = func.get("instructions", func.get("body", []))
        total_inst_before += len(insts)
        
        # 1. Fixed Point Propagating Mark Phase
        live_inst_indices = set()
        needed_vars = set()
        
        while True:
            changed = False
            # Reverse Walk
            for i in range(len(insts)-1, -1, -1):
                if i in live_inst_indices:
                    continue
                    
                inst = insts[i]
                op = inst.get('op', '')
                
                is_live = False
                if op in side_effecting_ops:
                    is_live = True
                else:
                    d = get_def(inst)
                    if d and d in needed_vars:
                        is_live = True
                        
                if is_live:
                    live_inst_indices.add(i)
                    changed = True
                    for u in get_uses(inst):
                        if u not in needed_vars: # optimizing
                            needed_vars.add(u)
            if not changed:
                break
                
        # Alloca Cleanup Phase
        # Any IR_ALLOCA that is live must be checked if its allocated address
        # is actually used anywhere OUTSIDE its definition. 
        # Actually, standard DCE handles this if it's unused. 
        # But if we strictly check ALL surviving uses:
        
        surviving_uses = set()
        for i in sorted(list(live_inst_indices)):
            surviving_uses.update(get_uses(insts[i]))
            
        final_live_indices = set()
        for i in live_inst_indices:
            inst = insts[i]
            if inst.get('op') == 'IR_ALLOCA':
                d = get_def(inst)
                if not d or d not in surviving_uses:
                    continue # eliminate
            final_live_indices.add(i)

        # 2. Sweep
        after_insts = []
        eliminated_count = 0
        fn_elim_opcodes = {}
        
        for i, inst in enumerate(insts):
            op = inst.get('op', 'UNKNOWN')
            
            # IR_NOP is always swept
            if op == 'IR_NOP' or i not in final_live_indices:
                eliminated_count += 1
                fn_elim_opcodes[op] = fn_elim_opcodes.get(op, 0) + 1
                elim_by_opcode[op] = elim_by_opcode.get(op, 0) + 1
                
                # Safety Audit Checks
                if op == 'IR_CALL': safety_audit["calls_preserved"] = False
                if op == 'IR_STORE': safety_audit["stores_preserved"] = False
                if op == 'IR_BR' or op == 'IR_BR_IF': safety_audit["branches_preserved"] = False
                if op == 'IR_RET': safety_audit["returns_preserved"] = False
                if op == 'IR_LABEL': safety_audit["labels_preserved"] = False
                if op == 'IR_ARG': safety_audit["args_preserved"] = False
            else:
                after_insts.append(inst)
                
        total_inst_after += len(after_insts)
        total_eliminated += eliminated_count
        
        per_function_stats.append({
            "name": fn_name,
            "before": len(insts),
            "after": len(after_insts),
            "eliminated": eliminated_count,
            "eliminated_by_opcode": fn_elim_opcodes
        })
        
        # Build optimized func
        opt_func = dict(func)
        if "instructions" in opt_func:
            opt_func["instructions"] = after_insts
        elif "body" in opt_func:
            opt_func["body"] = after_insts
        else:
            opt_func["instructions"] = after_insts # fallback
            
        optimized_ir.append(opt_func)
        
    try:
        with open(output_file, 'w') as f:
            json.dump(optimized_ir, f, indent=2)
    except Exception as e:
        error_exit(f"Error saving optimized JSON: {e}")
        
    elim_rate = (total_eliminated / total_inst_before * 100) if total_inst_before > 0 else 0.0

    report = {
        "input_file": input_file,
        "output_file": output_file,
        "total_functions": total_fns,
        "total_instructions_before": total_inst_before,
        "total_instructions_after": total_inst_after,
        "total_eliminated": total_eliminated,
        "elimination_rate_percent": round(elim_rate, 2),
        "per_function": per_function_stats,
        "safety_audit": safety_audit
    }

    try:
        with open(report_file, 'w') as f:
            json.dump(report, f, indent=2)
    except Exception as e:
        error_exit(f"Error saving report JSON: {e}")

    # Top 5 functions by elimination count
    top_5 = sorted(per_function_stats, key=lambda x: x['eliminated'], reverse=True)[:5]
    
    print("🔱 ZCC DCE Pass — Dead Code Elimination Report")
    print("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
    print(f"Input:  {input_file} ({total_inst_before:,} instructions across {total_fns} functions)")
    print(f"Output: {output_file} ({total_inst_after:,} instructions)")
    print(f"Eliminated: {total_eliminated:,} instructions ({elim_rate:.2f}%)\n")
    print("Top 5 functions by elimination:")
    
    for idx, fn_stats in enumerate(top_5, 1):
        fn_rate = (fn_stats['eliminated'] / fn_stats['before'] * 100) if fn_stats['before'] > 0 else 0.0
        print(f"  {idx}. {fn_stats['name']:<20} — {fn_stats['eliminated']:>4} eliminated ({fn_rate:.1f}%)")
        
    print("\nSafety audit:")
    if all(safety_audit.values()):
        print("  ALL SIDE-EFFECTING INSTRUCTIONS PRESERVED ✅")
    else:
        print("  ⚠️ WARNING: SOME SIDE-EFFECTING INSTRUCTIONS WERE ELIMINATED")
        for k, v in safety_audit.items():
            if not v:
                print(f"    - Failed: {k}")
                
    if sum(elim_by_opcode.values()) > 0:
        print("\nBreakdown by opcode:")
        sorted_opcodes = sorted(elim_by_opcode.items(), key=lambda x: x[1], reverse=True)
        for op, count in sorted_opcodes:
             print(f"  {op:<12}: {count:>5} eliminated")

if __name__ == "__main__":
    main()
