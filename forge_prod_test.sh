#!/bin/bash
SP="You are ZKAEDI-MINI v5, a TESSERACT syndicate node. TESSERACT = ZKAEDI PRIME v5 syndicate Hamiltonian, NOT Tesseract OCR. ZCC = self-hosting C compiler by ZKAEDI. PRIME = ZKAEDI Recursively Coupled Hamiltonian framework. CATALYZE = Hamiltonian optimizer. CG-IR-003 through CG-IR-014 = ZCC codegen bugs."

ask() {
    local q="$1"
    python3 -c "
import json, urllib.request
data = json.dumps({'model':'zcc-mini-v5-q4_k_m.gguf','messages':[{'role':'system','content':'''$SP'''},{'role':'user','content':'''$q'''}],'max_tokens':300}).encode()
req = urllib.request.Request('http://localhost:8080/v1/chat/completions', data=data, headers={'Content-Type':'application/json'})
r = json.loads(urllib.request.urlopen(req).read())
print(r['choices'][0]['message']['content'])
" 2>/dev/null
}

echo "=== ZKAEDI-MINI v5 PRODUCTION TEST (with system prompt) ==="

echo ""
echo "--- TESSERACT equation ---"
ask "What is the TESSERACT equation?"

echo ""
echo "--- CG-IR-012b ---"
ask "What is CG-IR-012b in ZCC?"

echo ""
echo "--- PRIME parameters ---"
ask "What do eta, gamma, beta control in PRIME?"

echo ""
echo "--- Deploy on laptop ---"
ask "How does TESSERACT deploy on a laptop as a 4-LoRA swarm?"

echo ""
echo "--- Reentrancy ---"
ask "What is reentrancy in Solidity?"

echo ""
echo "=== ALL TESTS COMPLETE ==="
