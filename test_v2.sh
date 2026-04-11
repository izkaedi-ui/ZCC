echo "=== Test 1: CG-IR-012b ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v2-q4_k_m.gguf","messages":[{"role":"user","content":"What is the CG-IR-012b bug in ZCC and why was it the most destructive?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 2: PHI edge copy ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v2-q4_k_m.gguf","messages":[{"role":"user","content":"How does ZCC solve the PHI edge copy lost-copy problem?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 3: OP_CONDBR ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v2-q4_k_m.gguf","messages":[{"role":"user","content":"What does the ZCC IR opcode OP_CONDBR do?"}],"max_tokens":200}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"
