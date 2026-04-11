#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== ZCC IR FORGE v2 ? LIVE SCAN ==="

# Check what the IR log actually looks like
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o /dev/null 2>ir_full.log

echo "IR log lines: $(wc -l < ir_full.log)"
echo "Sample IR lines:"
grep -i 'fn=\|func\|emitted\|ZCC-IR' ir_full.log | head -10

echo ""
echo "--- Sending top functions to ZKAEDI-MINI ---"

# Extract function names from whatever format the log uses
grep 'ZCC-IR' ir_full.log | head -5 | while IFS= read -r line; do
    FNAME=$(echo "$line" | grep -o 'fn=[^ ]*' | cut -d= -f2)
    if [ -z "$FNAME" ]; then continue; fi

    echo ""
    echo "=== $FNAME ==="

    PROMPT="Analyze the ZCC compiler function named $FNAME. This is a self-hosting C compiler. Based on the function name, what does it likely do, and what compiler optimization would benefit it most? Answer in 3 sentences."

    curl -s -X POST http://localhost:8080/v1/chat/completions \
      -H "Content-Type: application/json" \
      -d "{\"model\":\"zkaedi-mini-q4_k_m.gguf\",\"messages\":[{\"role\":\"system\",\"content\":\"You are ZKAEDI-MINI, a compiler specialist.\"},{\"role\":\"user\",\"content\":\"$PROMPT\"}],\"max_tokens\":150}" \
      | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])" 2>/dev/null

done

echo ""
echo "=== FORGE COMPLETE ==="
