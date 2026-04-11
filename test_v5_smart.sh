echo "=== Q1: Full equation with all three terms ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"Write out the full TESSERACT syndicate Hamiltonian equation and explain each of the three terms."}],"max_tokens":500}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q2: Anisotropic sharpening (avoids Gamma_t collision) ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What is anisotropic sharpening in the TESSERACT equation and how does it differ from the original scalar gamma?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q3: Supercritical regime (avoids bare eta) ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"Why does the TESSERACT syndicate operate in the supercritical regime above the Wilson-Fisher fixed point?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q4: Laptop swarm (avoids deploy collision) ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does TESSERACT map from 64 syndicate nodes down to a 4-LoRA swarm on a single GPU?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q5: Compiler as Term 2 ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does the TESSERACT equation map to a self-hosting compiler bootstrap chain?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q6: Training as Term 3 ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How does TESSERACT explain why fine-tuning v1 through v4 of ZKAEDI-MINI showed jump discontinuities instead of smooth improvement?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q7: Sensitivity-driven escalation ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"In TESSERACT, when should the syndicate escalate to human review instead of auto-resolving?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q8: Hadamard product role ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"What role does the Hadamard product play in Term 2 of the TESSERACT equation?"}],"max_tokens":250}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q9: How CATALYZE discovered Term 2 empirically ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"How did the CATALYZE optimizer empirically discover what TESSERACT later formalized as Term 2?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== Q10: What makes geom_med better than voting ==="
curl -s -X POST http://localhost:8080/v1/chat/completions -H "Content-Type: application/json" -d '{"model":"zcc-mini-v5-q4_k_m.gguf","messages":[{"role":"user","content":"In TESSERACT, why is geometric median fusion superior to simple majority voting or arithmetic mean?"}],"max_tokens":300}' | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"
