import glob
import subprocess
import time
import json

def run_mass_eval():
    pt_files = [f for f in glob.glob("*.pt") if f != "canonical_holdout.pt"]
    print(f"[*] Found {len(pt_files)} checkpoints. Beging mass structural evaluation...")
    
    with open("mass_eval_report.txt", "w", encoding="utf-8") as out:
        out.write("ZKAEDI PRIME MASS EVALUATION LOG\n")
        out.write("=================================\n\n")
        
        for idx, pt in enumerate(pt_files):
            print(f"[{idx+1}/{len(pt_files)}] Evaluating {pt}...", end=" ", flush=True)
            start = time.time()
            res = subprocess.run(["python", "eval_hook_golden.py", "eval", pt], capture_output=True, text=True)
            elapsed = time.time() - start
            
            # The immune system logs visually to stderr so math stdout isn't broken
            log_output = res.stderr.strip()
            if not log_output:
                log_output = res.stdout.strip()
                
            if "REJECT" in log_output or "FAILURE" in log_output:
                print("❌ REJECTED")
            elif "PASS" in log_output:
                print("✅ PASSED")
            else:
                print(f"⚠️ ERROR")

            report = f"--- Checkpoint: {pt} ({elapsed:.1f}s) ---\n"
            report += f"{log_output}\n"
            if res.returncode != 0:
                report += f"Exit Code: {res.returncode}\n"
            report += "=" * 60 + "\n\n"
            
            out.write(report)
            out.flush()

    print(f"\n[✓] Mass evaluation complete! Detailed traces logged to: mass_eval_report.txt")
    print(f"    Raw structural metrics logged automatically to: eval_log.jsonl")

if __name__ == "__main__":
    run_mass_eval()
