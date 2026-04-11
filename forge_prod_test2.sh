#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

python3 << 'PYEOF'
import json, urllib.request

SP = "You are ZKAEDI-MINI v5, a TESSERACT syndicate node. TESSERACT = ZKAEDI PRIME v5 syndicate Hamiltonian, NOT Tesseract OCR. ZCC = self-hosting C compiler by ZKAEDI. PRIME = ZKAEDI Recursively Coupled Hamiltonian framework. CATALYZE = Hamiltonian optimizer. CG-IR-003 through CG-IR-014 = ZCC codegen bugs."

def ask(q):
    data = json.dumps({"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"system","content":SP},{"role":"user","content":q}],"max_tokens":300}).encode()
    req = urllib.request.Request("http://localhost:8080/v1/chat/completions", data=data, headers={"Content-Type":"application/json"})
    r = json.loads(urllib.request.urlopen(req).read())
    print(r["choices"][0]["message"]["content"])

tests = [
    ("TESSERACT equation", "What is the TESSERACT equation?"),
    ("CG-IR-012b", "What is CG-IR-012b in ZCC?"),
    ("PRIME parameters", "What do eta, gamma, beta control in PRIME?"),
    ("4-LoRA laptop swarm", "How does TESSERACT deploy on a laptop as a 4-LoRA swarm?"),
    ("Reentrancy", "What is reentrancy in Solidity?"),
]

print("=== ZKAEDI-MINI v5 PRODUCTION TEST (system prompt) ===\n")
for name, q in tests:
    print(f"--- {name} ---")
    try:
        ask(q)
    except Exception as e:
        print(f"ERROR: {e}")
    print()
print("=== ALL TESTS COMPLETE ===")
PYEOF
