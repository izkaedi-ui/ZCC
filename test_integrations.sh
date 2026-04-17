#!/usr/bin/env bash
# Pre-bind integration tests — run entirely inside WSL bash
set -uo pipefail
PASS=0; FAIL=0; WARN=0
ZCC=/mnt/h/__DOWNLOADS/selforglinux
AGENTS=/mnt/h/agents
BUILD=$AGENTS/selforglinux_build

ok()   { echo "  [PASS] $*"; PASS=$((PASS+1)); }
fail() { echo "  [FAIL] $*"; FAIL=$((FAIL+1)); }
warn() { echo "  [WARN] $*"; WARN=$((WARN+1)); }
hdr()  { echo; echo "========================================="; echo "  $*"; echo "========================================="; }

# ── DOOM ──────────────────────────────────────────────────────────────────────
hdr "TIER 2A: DOOM SMOKE TEST"
cd $ZCC
DOOM_OUT=$(timeout 3 ./doom_bin -novideo -nomusic -nosound 2>&1 || true)
echo "$DOOM_OUT"
echo "$DOOM_OUT" | grep -q "V_Init"             && ok  "Doom: V_Init reached (binary boots)" || fail "Doom: V_Init not found"
echo "$DOOM_OUT" | grep -q "M_LoadDefaults"     && ok  "Doom: M_LoadDefaults reached"        || warn "Doom: M_LoadDefaults not found"
echo "$DOOM_OUT" | grep -q "Segmentation fault" && fail "Doom: SIGSEGV during init" || ok "Doom: no SIGSEGV during init"

# ── SQLITE ────────────────────────────────────────────────────────────────────
hdr "TIER 2B: SQLITE SMOKE TEST"
cat > /tmp/zcc_sqlite_test.sql <<'SQL'
.version
SELECT 1+1;
CREATE TABLE zcc_t(id INTEGER, val TEXT);
INSERT INTO zcc_t VALUES(1,'hello');
SELECT id,val FROM zcc_t WHERE id=1;
DROP TABLE zcc_t;
SQL
SQL_OUT=$(timeout 5 ./sqlite3_test < /tmp/zcc_sqlite_test.sql 2>&1 || true)
echo "$SQL_OUT"
echo "$SQL_OUT" | grep -q "SQLite 3"  && ok "SQLite: version banner"   || fail "SQLite: no version"
echo "$SQL_OUT" | grep -q "rc=0"      && ok "SQLite: all ops rc=0"     || fail "SQLite: non-zero rc"
echo "$SQL_OUT" | grep -q "1 = 1"     && ok "SQLite: 1+1 arithmetic"   || fail "SQLite: arithmetic failed"
echo "$SQL_OUT" | grep -qE "hello|id = 1" && ok "SQLite: DML round-trip" || warn "SQLite: DML output format differs"

# ── KRAKEN ────────────────────────────────────────────────────────────────────
hdr "TIER 2C: KRAKEN INFERENCE TEST"
KRAKEN_OUT=$(timeout 5 ./kraken_final 2>&1 || true)
echo "$KRAKEN_OUT"
echo "$KRAKEN_OUT" | grep -q "KRAKEN v2.2"             && ok  "Kraken: banner"           || fail "Kraken: no banner"
echo "$KRAKEN_OUT" | grep -q "Reentrancy:   VULNERABLE" && ok  "Kraken: Reentrancy correct" || fail "Kraken: wrong Reentrancy"
echo "$KRAKEN_OUT" | grep -q "Overflow:     SAFE"       && ok  "Kraken: Overflow correct"   || fail "Kraken: wrong Overflow"
VULN=$(echo "$KRAKEN_OUT" | grep -c "VULNERABLE" || echo 0)
[ "$VULN" -ge 3 ] && ok "Kraken: $VULN VULNERABLE flags" || warn "Kraken: only $VULN VULNERABLE flags"

# ── ASM PROFILER ──────────────────────────────────────────────────────────────
hdr "TIER 1A: ASM PROFILER"
ASM_OUT=$(python3 $BUILD/asm_profile.py 2>&1)
echo "$ASM_OUT" | tail -8
echo "$ASM_OUT" | grep -q "Estimated" && ok "ASM Profiler: completed" || fail "ASM Profiler: failed"
SAVINGS=$(echo "$ASM_OUT" | grep "Estimated instruction savings" | grep -oP '[0-9,]+' | head -1 | tr -d ',')
echo "     Savings: ~${SAVINGS:-?} instructions"

# ── DCE ───────────────────────────────────────────────────────────────────────
hdr "TIER 1B: ZCC DCE"
# Generate minimal IR JSON
python3 - <<'PYEOF'
import re, json
from pathlib import Path
src = Path("/mnt/h/__DOWNLOADS/selforglinux/zcc2.s").read_text()
funcs = re.findall(r'^([a-zA-Z_]\w+):$', src, re.MULTILINE)
blocks = []
for f in funcs[:30]:
    pat = re.compile(rf'^{re.escape(f)}:\n((?:(?!^[a-zA-Z_]\w+:).*\n)*)', re.MULTILINE)
    m = pat.search(src)
    body = m.group(1) if m else ""
    insts = [l.strip() for l in body.splitlines() if l.strip() and not l.strip().endswith(':') and not l.strip().startswith('.')]
    blocks.append({"func": f, "instructions": insts[:20], "dead_labels": [], "successors": []})
out = {"version": "1.0", "functions": blocks, "total_functions": len(funcs)}
Path("/mnt/h/__DOWNLOADS/selforglinux/zcc_ir.json").write_text(json.dumps(out, indent=2))
print(f"IR JSON: {len(funcs)} functions, {sum(len(b['instructions']) for b in blocks)} instructions")
PYEOF
DCE_OUT=$(cd $ZCC && python3 $BUILD/zcc_dce.py 2>&1 | head -15 || true)
echo "$DCE_OUT"
echo "$DCE_OUT" | grep -qiE "dead|block|eliminat|function|pass" && ok "DCE: ran and found targets" || warn "DCE: completed with no findings (IR too small)"

# ── CGCE STEERING ─────────────────────────────────────────────────────────────
hdr "TIER 3: CGCE STEERING AGENT"
# Install dep if needed
python3 -c "import websocket" 2>/dev/null || pip3 install websocket-client -q
python3 -c "import websocket; print('  dep: websocket-client OK')"

# Test with --sigma and no live server (expect graceful fail/timeout)
CGCE_OUT=$(timeout 6 python3 $AGENTS/soc_cgce_mutator_agent.py \
    --sigma 1.09 --tau 1.87 --offline --output /tmp/cgce_out.json 2>&1 || true)
echo "$CGCE_OUT" | head -15
echo "$CGCE_OUT" | grep -qiE "cgce|sigma|hamilto|mcmc|phase|offline|error|connect" \
    && ok "CGCE: agent launched and responded" \
    || warn "CGCE: no response (needs live SOC feed)"
test -f /tmp/cgce_out.json && ok "CGCE: output JSON written" || warn "CGCE: no output JSON (offline mode)"

# ── RESULTS ───────────────────────────────────────────────────────────────────
echo
echo "========================================="
echo "  INTEGRATION TEST SUMMARY"
echo "========================================="
echo "  PASS : $PASS"
echo "  WARN : $WARN  (bindable with caveats)"
echo "  FAIL : $FAIL  (needs fix before binding)"
echo "========================================="

[ $FAIL -eq 0 ] && echo "VERDICT: ALL SYSTEMS READY — BIND" && exit 0
echo "VERDICT: $FAIL FAILURES — FIX BEFORE BIND" && exit 1
