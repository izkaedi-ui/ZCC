echo "=== Test 8: What is Gamma_t? ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is Gamma_t in TESSERACT?"}],"max_tokens":250}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 9: Why is eta 0.78 supercritical? ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"Why is eta=0.78 supercritical in TESSERACT?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 10: 4-LoRA laptop swarm ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does TESSERACT deploy on a laptop?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 11: CATALYZE connection ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does CATALYZE relate to TESSERACT?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 12: Mythological swarm mapping ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does TESSERACT map to the mythological swarm?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 13: ZCC self-host invariant ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is the self-host invariant in ZCC?"}],"max_tokens":250}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 14: Audit pipeline as TESSERACT ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does TESSERACT map to the audit pipeline?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 15: Hurst regime detection ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does PRIME detect crypto market regime transitions?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 16: What is Delta_N_t? ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is Delta_N_t^(i) in the TESSERACT equation?"}],"max_tokens":200}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Test 17: General knowledge (not trained) ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is the capital of France?"}],"max_tokens":50}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"
