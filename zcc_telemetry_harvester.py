#!/usr/bin/env python3
import os
import json
import time
import subprocess
import glob
from datetime import datetime
import argparse

def parse_assembly(asm_path):
    metrics = {
        "movq": 0,
        "movl": 0,
        "push_pop_pairs": 0,
        "calls": 0,
        "total_instructions": 0
    }
    push_count = 0
    pop_count = 0
    
    if not os.path.exists(asm_path):
        return metrics

    with open(asm_path, "r", encoding="utf-8") as f:
        for line in f:
            tline = line.strip()
            # Ignore empty lines, labels, dots, comments
            if not tline or tline.startswith(('.', '#', '/')) or tline.endswith(':'):
                continue
            
            metrics["total_instructions"] += 1
            if tline.startswith("movq "):
                metrics["movq"] += 1
            elif tline.startswith("movl "):
                metrics["movl"] += 1
            elif tline.startswith("call "):
                metrics["calls"] += 1
            if "push" in tline:
                push_count += 1
            elif "pop" in tline:
                pop_count += 1
        
        metrics["push_pop_pairs"] = min(push_count, pop_count)
    return metrics

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", default="baseline", type=str)
    parser.add_argument("--output", default="zcc_baseline_post_bugs_001_004.jsonl", type=str)
    args = parser.parse_args()

    # Collect valid test files in the directory
    all_c = glob.glob("test_*.c") + glob.glob("seed*.c") + glob.glob("bisect*.c")
    files_to_test = [f for f in all_c if "zcc" not in f and "gcc" not in f and "pp" not in f]
    files_to_test = sorted(list(set(files_to_test)))
    
    out_file = args.output
    
    print(f"[ZCC Telemetry] Targeting baseline compilation for {len(files_to_test)} files...")
    
    # Use stage2 compiler if available, else default to zcc
    compiler_bin = "./zcc2" if os.path.exists("./zcc2") else "./zcc"
    if not os.path.exists(compiler_bin):
        print(f"Warning: {compiler_bin} not found. Cannot collect metrics.")
        return

    with open(out_file, "w", encoding="utf-8") as out:
        for test_file in files_to_test:
            asm_out = test_file.replace('.c', '.s')
            exe_out = test_file.replace('.c', '.exe')
            
            start_time = time.time()
            res = subprocess.run([compiler_bin, test_file, "-o", asm_out], capture_output=True)
            compilation_time_ms = int((time.time() - start_time) * 1000)
            
            exit_code = res.returncode
            bin_size = -1
            
            if exit_code == 0 and os.path.exists(asm_out):
                res2 = subprocess.run(["gcc", "-x", "assembler", asm_out, "-o", exe_out], capture_output=True)
                if res2.returncode == 0 and os.path.exists(exe_out):
                    bin_size = os.path.getsize(exe_out)
            
            inst_counts = parse_assembly(asm_out)
            
            record = {
                "test_file": test_file,
                "compilation_time_ms": compilation_time_ms,
                "binary_size_bytes": bin_size,
                "instruction_counts": inst_counts,
                "exit_code": exit_code,
                "self_host_generation": os.path.basename(compiler_bin),
                "bugs_fixed_count": 25,
                "timestamp": datetime.utcnow().isoformat() + "Z"
            }
            out.write(json.dumps(record) + "\n")
            
            print(f" -> Generated: {test_file} | Time: {compilation_time_ms}ms | Insn: {inst_counts['total_instructions']}")

    print(f"\n[ZCC Telemetry] Completed harvesting to {out_file}")

if __name__ == "__main__":
    main()
