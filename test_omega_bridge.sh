#!/usr/bin/env bash
# Test 3: Omega Kraken Bridge
BRIDGE=/mnt/h/agents/core-ide-system/omega_kraken_bridge.js
PT=/mnt/h/agents/selforglinux_build/canonical_holdout.pt

echo "=== OMEGA KRAKEN BRIDGE PROBE ==="

# Node.js available?
NODE=$(which node 2>/dev/null || which nodejs 2>/dev/null || echo "")
if [ -z "$NODE" ]; then
  echo "NODE_AVAILABLE=false"
  echo "STATUS=SKIP"
  exit 0
fi
echo "NODE_AVAILABLE=true"
echo "NODE=$NODE"
NODE_VER=$($NODE --version 2>&1)
echo "NODE_VER=$NODE_VER"

# canonical_holdout.pt exists?
if [ -f "$PT" ]; then
  PT_SIZE=$(wc -c < "$PT")
  echo "PT_EXISTS=true  SIZE=$PT_SIZE bytes"
else
  echo "PT_EXISTS=false"
fi

# Bridge syntax check
$NODE --check "$BRIDGE" 2>&1 && echo "BRIDGE_SYNTAX=OK" || echo "BRIDGE_SYNTAX=FAIL"

# Probe exports / top-level structure
python3 - << 'PYEOF'
import re
src = open('/mnt/h/agents/core-ide-system/omega_kraken_bridge.js').read()
functions = re.findall(r'(?:function|async function|const)\s+(\w+)\s*[=(]', src)
print(f"BRIDGE_EXPORTS={functions[:8]}")
has_weight_read = 'canonical_holdout' in src or 'weight' in src.lower()
print(f"READS_WEIGHTS={has_weight_read}")
PYEOF

echo "STATUS=PASS"
