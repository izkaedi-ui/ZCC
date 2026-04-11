#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

SYSTEM_PROMPT=$(cat zkaedi_mini_system.txt | tr '\n' ' ' | sed "s/'/\\\\'/g")
MODEL="zcc-mini-v5-q4_k_m.gguf"
PORT=8080

ask_mini() {
    local question="$1"
    local max_tokens="${2:-300}"
    
    curl -s -X POST "http://localhost:$PORT/v1/chat/completions" \
      -H "Content-Type: application/json" \
      -d "{\"model\":\"$MODEL\",\"messages\":[{\"role\":\"system\",\"content\":\"$SYSTEM_PROMPT\"},{\"role\":\"user\",\"content\":\"$question\"}],\"max_tokens\":$max_tokens}" \
      | python3 -c "import sys,json; r=json.load(sys.stdin); print(r['choices'][0]['message']['content'])" 2>/dev/null
}

if [ "$1" = "--test" ]; then
    echo "=== ZKAEDI-MINI v5 PRODUCTION TEST ==="
    echo ""
    echo "--- TESSERACT equation ---"
    ask_mini "What is the TESSERACT equation?"
    echo ""
    echo "--- CG-IR-012b ---"
    ask_mini "What is CG-IR-012b in ZCC?"
    echo ""
    echo "--- PRIME parameters ---"
    ask_mini "What do eta, gamma, beta control in PRIME?"
    echo ""
    echo "--- Reentrancy ---"
    ask_mini "What is reentrancy in Solidity?"
    echo ""
    echo "=== ALL TESTS COMPLETE ==="
elif [ "$1" = "--audit" ]; then
    echo "=== SOLIDITY AUDIT MODE ==="
    ask_mini "Audit this Solidity contract for vulnerabilities: $2" 500
elif [ "$1" = "--ir" ]; then
    echo "=== ZCC IR ANALYSIS MODE ==="
    ask_mini "Analyze this ZCC IR for optimization opportunities: $2" 400
elif [ -n "$1" ]; then
    ask_mini "$*"
else
    echo "Usage:"
    echo "  forge_production.sh --test              Run production tests"
    echo "  forge_production.sh --audit '<code>'    Audit Solidity code"
    echo "  forge_production.sh --ir '<ir_dump>'    Analyze ZCC IR"
    echo "  forge_production.sh '<question>'        Ask anything"
fi
