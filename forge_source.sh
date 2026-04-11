#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== ZKAEDI-MINI analyzing ZCC source ==="

FUNCS="codegen_expr codegen_stmt scalar_promotion_pass ir_asm_emit_phi_edge_copy cc_alloc"

for FNAME in $FUNCS; do
    echo ""
    echo "=== $FNAME ==="

    BODY=$(grep -A30 "^.*$FNAME(" zcc_pp.c 2>/dev/null | head -30 | tr '"' "'" | tr '\n' ' ' | cut -c1-800)

    if [ -z "$BODY" ]; then
        echo "(not found)"
        continue
    fi

    curl -s -X POST http://localhost:8080/v1/chat/completions \
      -H "Content-Type: application/json" \
      -d "{\"model\":\"zkaedi-mini-q4_k_m.gguf\",\"messages\":[{\"role\":\"system\",\"content\":\"You are ZKAEDI-MINI, a C compiler security and optimization specialist.\"},{\"role\":\"user\",\"content\":\"Analyze this C function for bugs or optimizations: $BODY\"}],\"max_tokens\":200}" \
      | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])" 2>/dev/null

done

echo ""
echo "=== FORGE COMPLETE ==="
