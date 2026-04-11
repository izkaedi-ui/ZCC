echo "=== Test 1: TESSERACT equation ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is the TESSERACT equation?"}],"max_tokens":400}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 2: PRIME parameters ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What do eta, gamma, beta, and sigma control in PRIME?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 3: OP_CONDBR ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What does OP_CONDBR do in ZCC IR?"}],"max_tokens":200}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 4: Geometric median ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is Byzantine robustness in TESSERACT?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 5: Jacobian jumps ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is punctuated equilibrium in TESSERACT?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 6: Identity ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"Who are you and what do you know about?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 7: Reentrancy (retained?) ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is a reentrancy vulnerability in Solidity?"}],"max_tokens":200}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"
