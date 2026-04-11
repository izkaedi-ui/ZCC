curl -s -X POST http://localhost:8080/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{"model":"zkaedi-mini-q4_k_m.gguf","messages":[{"role":"user","content":"What is a reentrancy vulnerability in Solidity?"}],"max_tokens":200}'
