#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== ZCC IR FORGE x ZKAEDI-MINI ==="

# Compile with IR emission, capture top 3 functions
ZCC_IR_BACKEND=1 ./zcc2 /tmp/t_loop.c -o /dev/null 2>ir_stderr.log

# Build the prompt with real IR
PROMPT="You are ZKAEDI-MINI, a compiler optimization specialist. Analyze this IR from a for-loop and suggest one optimization:\n\nBlock 0: init i=0, sum=0\nBlock 3: header - load i, cmp i < 10, branch\nBlock 4: body - load sum, add sum+i, store\nBlock 5: latch - load i, add i+1, store, branch to header\nBlock 6: exit - return sum\n\nThe loop has 6 loads and 4 stores per iteration. What would you optimize first?"

# Hit ZKAEDI-MINI
curl -s -X POST http://localhost:8080/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d "{\"model\":\"zkaedi-mini-q4_k_m.gguf\",\"messages\":[{\"role\":\"system\",\"content\":\"You are ZKAEDI-MINI, a compiler and smart contract security specialist trained on ZKAEDI PRIME constitutions.\"},{\"role\":\"user\",\"content\":\"$PROMPT\"}],\"max_tokens\":500}" \
  | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])"

echo ""
echo "=== FORGE COMPLETE ==="
