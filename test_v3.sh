echo "=== Test 1: CG-IR-012b (no system prompt) ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v3-q4_k_m.gguf","messages":[{"role":"user","content":"What is the CG-IR-012b bug in ZCC?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 2: PHI edge copy ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v3-q4_k_m.gguf","messages":[{"role":"user","content":"How does ZCC solve the PHI edge copy lost-copy problem?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 3: What does OP_CONDBR do? ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v3-q4_k_m.gguf","messages":[{"role":"user","content":"What does OP_CONDBR do in ZCC IR?"}],"max_tokens":200}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 4: What is ZCC? ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v3-q4_k_m.gguf","messages":[{"role":"user","content":"What is ZCC?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 5: Reentrancy (still works?) ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v3-q4_k_m.gguf","messages":[{"role":"user","content":"What is a reentrancy vulnerability in Solidity?"}],"max_tokens":200}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"
