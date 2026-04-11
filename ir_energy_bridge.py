import json
import sys
import numpy as np
import leviathan_core

def parse_ir_and_scan(json_path):
    print(f"[*] Deserializing IR from {json_path}...")
    try:
        with open(json_path, 'r') as f:
            ir_data = json.load(f)
    except Exception as e:
        print(f"[-] Failed to read or parse JSON: {e}")
        return
    
    q_batch = []
    p_batch = []
    m_batch = []
    
    for fn in ir_data:
        print(f"[+] Analyzing function: {fn.get('function', 'unknown')}")
        for block in fn.get('blocks', []):
            for ins in block.get('instructions', []):
                op = ins.get('op', '')
                q = 0
                p = 0
                m = 1000 # Default gas limit equivalent
                
                # Map specific ZCC ops to threat profiles mimicking EVM red team
                if op == 'OP_ALLOCA':
                    q = 10; p = 5
                elif op == 'OP_LOAD':
                    q = 20; p = 10
                elif op == 'OP_STORE':
                    q = 50; p = 20
                elif op == 'OP_GEP':
                    q = 100; p = 50  # Alias confusion risk
                elif op == 'OP_CALL':
                    q = 80; p = 30
                else:
                    q = 5; p = 5

                q_batch.append(q)
                p_batch.append(p)
                m_batch.append(m)
                
    if not q_batch:
        print("[-] No valid operations found in IR.")
        return
        
    print(f"[*] Feed generated execution graph ({len(q_batch)} ops) to Leviathan Core...")
    energy, calc_time = leviathan_core.scan_mempool_batch(q_batch, p_batch, m_batch)
    
    print(f"\n[LEVIATHAN RESULTS]")
    print(f"Total Operations: {len(q_batch)}")
    print(f"Threat Energy:    {energy}")
    print(f"Latency:          {calc_time:.6f}s")
    
    # Use the non-linear threat spike threshold
    if energy > len(q_batch) * 800:
        print("\n[!!!] LEVIATHAN ALERT: CRITICAL VULNERABILITY (BUFFER OVERFLOW / ALIAS CONFUSION) DETECTED IN IR.")
    else:
        print("\n[+] IR Signature normal. No critical threats detected.")

if __name__ == '__main__':
    path = sys.argv[1] if len(sys.argv) > 1 else 'zcc_ir_dump.json'
    parse_ir_and_scan(path)
