#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== ZCC IR FORGE ? LIVE SCAN ==="
echo ""

# Compile with IR, capture all function IR dumps
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o /dev/null 2>ir_full.log

# Extract function names and block counts
grep '\[ZCC-IR\] fn=' ir_full.log | while read line; do
    FNAME=$(echo "$line" | sed 's/.*fn=\([^ ]*\).*/\1/')
    BLOCKS=$(echo "$line" | sed 's/.*emitted.*) \([0-9]*\) blocks/\1/')
    echo "--- $FNAME ($BLOCKS blocks) ---"
done | head -20

echo ""
echo "Top 5 largest functions by block count:"
grep '\[ZCC-IR\] fn=' ir_full.log | \
    sed 's/.*fn=\([^ ]*\).*) \([0-9]*\) blocks/\2 \1/' | \
    sort -rn | head -5 | while read BLOCKS FNAME; do

    echo ""
    echo "=== Analyzing: $FNAME ($BLOCKS blocks) ==="

    # Ask ZKAEDI-MINI about this function
    PROMPT="You are analyzing the ZCC compiler function $FNAME which has $BLOCKS basic blocks in its control flow graph after optimization (DCE, Mem2Reg, escape analysis, constant folding). This function is part of a self-hosting C compiler. In 3 sentences: what does this function likely do based on its name, and what optimization opportunities might exist for a function with $BLOCKS blocks?"

    RESPONSE=$(curl -s -X POST http://localhost:8080/v1/chat/completions \
      -H "Content-Type: application/json" \
      -d "{\"model\":\"zkaedi-mini-q4_k_m.gguf\",\"messages\":[{\"role\":\"system\",\"content\":\"You are ZKAEDI-MINI, a compiler optimization and security specialist.\"},{\"role\":\"user\",\"content\":\"$PROMPT\"}],\"max_tokens\":150}" \
      | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])" 2>/dev/null)

    echo "$RESPONSE"
done

echo ""
echo "=== FORGE COMPLETE ==="
