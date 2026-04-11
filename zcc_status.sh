#!/bin/bash
# ══════════════════════════════════════════════════════════════════
# zcc_status.sh — ZCC Compiler Status Report Generator
#
# Produces ZCC_STATUS.md: a self-contained snapshot of the compiler's
# current state. Paste it into any new AI conversation for instant
# context. Covers: build health, IR backend status, pass statistics,
# function counts, peephole numbers, known bugs, file sizes.
#
# Usage: bash zcc_status.sh
# Output: ZCC_STATUS.md (in current directory)
# ══════════════════════════════════════════════════════════════════
set -e

OUT="ZCC_STATUS.md"
TMP_BUILD="/tmp/zcc_status_build"
mkdir -p "$TMP_BUILD"

RED='\033[0;31m'
GRN='\033[0;32m'
CYN='\033[0;36m'
RST='\033[0m'

step() { echo -e "${CYN}[status] $1${RST}"; }

# ── Gather data ──────────────────────────────────────────────────

step "Concatenating source"
cat part1.c part2.c part3.c ir.h ir_emit_dispatch.h ir_bridge.h part4.c part5.c ir.c ir_to_x86.c > zcc_pp.c 2>/dev/null || true

TOTAL_LINES=$(wc -l < zcc_pp.c 2>/dev/null || echo "?")
PART4_LINES=$(wc -l < part4.c 2>/dev/null || echo "?")
CP_LINES=$(wc -l < compiler_passes.c 2>/dev/null || echo "?")
CP_IR_LINES=$(wc -l < compiler_passes_ir.c 2>/dev/null || echo "?")

# Count functions
N_FUNCS=$(grep -c '^[a-zA-Z_].*):$' zcc2.s 2>/dev/null || echo "?")

# IR gate status
IR_GATE=$(grep 'return [01];' part4.c 2>/dev/null | grep -A0 'ir_whitelisted' | tail -1 || true)
if grep -q 'return 1;' part4.c 2>/dev/null && grep -q 'ir_whitelisted' part4.c 2>/dev/null; then
    # Check if return 1 is inside ir_whitelisted
    GATE_LINE=$(grep -n 'return [01];' part4.c | grep -B0 'codegen_func' | head -1 | cut -d: -f1)
    IR_RETURN=$(sed -n "${GATE_LINE}p" part4.c 2>/dev/null || echo "")
fi
# Simpler: just check the line before "void codegen_func"
IR_GATE_VAL=$(grep -B2 'void codegen_func' part4.c 2>/dev/null | grep 'return [01]' | grep -o '[01]' || echo "?")

# Blacklist entries
BLACKLIST=$(grep -A20 'ir_whitelisted' part4.c 2>/dev/null | grep '"' | grep -v '//' | sed 's/.*"\(.*\)".*/\1/' | tr '\n' ', ' | sed 's/,$//' || echo "none")

# Peephole count from last build
PEEPHOLE=$(grep 'elided' zcc2.s 2>/dev/null | head -1 || true)
if [ -z "$PEEPHOLE" ]; then
    PEEPHOLE_N=$(grep -c 'elided\|Peephole' build.log 2>/dev/null || echo "?")
else
    PEEPHOLE_N="$PEEPHOLE"
fi
# Try from build log
PEEPHOLE_COUNT=$(grep 'elided' build.log 2>/dev/null | tail -1 | grep -o '[0-9]* elided' || echo "")
if [ -z "$PEEPHOLE_COUNT" ]; then
    # Try stderr from last make
    PEEPHOLE_COUNT=$(grep 'Peephole.*OK' build.log 2>/dev/null | tail -1 | grep -o '([0-9]*)' | tr -d '()' || echo "?")
fi

# ── Test AST selfhost ────────────────────────────────────────────

step "Testing AST selfhost"
AST_SELFHOST="UNKNOWN"
if [ -f zcc2.s ] && [ -f zcc3.s ]; then
    if cmp -s zcc2.s zcc3.s; then
        AST_SELFHOST="VERIFIED (zcc2.s == zcc3.s)"
    else
        AST_SELFHOST="FAILED (zcc2.s != zcc3.s)"
    fi
elif [ -f Makefile ]; then
    # Try a quick build
    if make selfhost > "$TMP_BUILD/selfhost.log" 2>&1; then
        AST_SELFHOST="VERIFIED (fresh build)"
        PEEPHOLE_COUNT=$(grep 'elided' "$TMP_BUILD/selfhost.log" | tail -1 | grep -o '[0-9]*' | head -1 || echo "?")
    else
        AST_SELFHOST="FAILED"
    fi
fi

# ── Test IR selfhost ─────────────────────────────────────────────

step "Testing IR backend"
IR_SELFHOST="NOT TESTED"
IR_FUNC_COUNT="?"
IR_PASS_STATS=""

# Only test if gate is open or we can temporarily open it
if [ -f verify_ir_backend.sh ]; then
    step "Running verify_ir_backend.sh"
    if bash verify_ir_backend.sh > "$TMP_BUILD/ir_verify.log" 2>&1; then
        IR_SELFHOST="VERIFIED"
    else
        IR_SELFHOST="FAILED"
    fi

    # Extract stats from log
    IR_FUNC_COUNT=$(grep -c '\[ZCC-IR\] fn=' "$TMP_BUILD/ir_verify.log" 2>/dev/null || echo "?")
    BLACKLIST_HITS=$(grep -c 'ZCC-BLACKLIST' "$TMP_BUILD/ir_verify.log" 2>/dev/null || echo "0")
    
    # Pass statistics
    FOLDED=$(grep 'IR-Opts' "$TMP_BUILD/ir_verify.log" 2>/dev/null | tail -1 || echo "")
    DCE=$(grep 'DCE->SSA' "$TMP_BUILD/ir_verify.log" 2>/dev/null | tail -1 || echo "")
    MEM2REG=$(grep 'Mem2Reg' "$TMP_BUILD/ir_verify.log" 2>/dev/null | tail -1 || echo "")
    ESCAPE=$(grep 'EscapeAna' "$TMP_BUILD/ir_verify.log" 2>/dev/null | tail -1 || echo "")
    RLE=$(grep 'RLE' "$TMP_BUILD/ir_verify.log" 2>/dev/null | tail -1 || echo "")
fi

# ── Count CG-IR fixes ───────────────────────────────────────────

CG_FIXES=""
for tag in CG-IR-003 CG-IR-004 CG-IR-005 CG-IR-006 CG-IR-007 CG-IR-008 CG-IR-009 CG-IR-010 CG-IR-011 CG-IR-012 CG-IR-013; do
    if grep -q "$tag" compiler_passes.c part4.c 2>/dev/null; then
        CG_FIXES="$CG_FIXES $tag"
    fi
done

# ── Check for known issues ──────────────────────────────────────

KNOWN_ISSUES=""
# Missing symbols
if grep -q 'node_is_array\|node_is_func\|node_ptr_elem_size' compiler_passes.c 2>/dev/null; then
    if ! grep -q 'extern.*node_is_array' compiler_passes.c 2>/dev/null; then
        KNOWN_ISSUES="$KNOWN_ISSUES\n- node_is_array/node_is_func/node_ptr_elem_size: may cause link errors from IR assembly"
    fi
fi
# LICM disabled
if grep -q '// licm_pass' compiler_passes.c 2>/dev/null; then
    KNOWN_ISSUES="$KNOWN_ISSUES\n- LICM pass is commented out in run_all_passes"
fi

if [ -z "$KNOWN_ISSUES" ]; then
    KNOWN_ISSUES="None detected"
fi

# ── Generate timestamp ──────────────────────────────────────────

TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S %Z')

# ── Write ZCC_STATUS.md ─────────────────────────────────────────

step "Writing $OUT"

cat > "$OUT" << STATUSEOF
# ZCC Status Report
Generated: $TIMESTAMP

## Build Health

| Component | Status |
|-----------|--------|
| AST Selfhost | $AST_SELFHOST |
| IR Backend | $IR_SELFHOST |
| IR Functions | $IR_FUNC_COUNT compiled through IR |
| Blacklist Hits | $BLACKLIST_HITS functions fell back to AST |
| IR Gate | ir_whitelisted() returns $IR_GATE_VAL |
| Blacklist | $BLACKLIST |

## Source Sizes

| File | Lines |
|------|-------|
| zcc_pp.c (concatenated) | $TOTAL_LINES |
| part4.c (codegen) | $PART4_LINES |
| compiler_passes.c (IR passes) | $CP_LINES |
| compiler_passes_ir.c (IR helpers) | $CP_IR_LINES |

## CG-IR Bug Fixes Applied
$CG_FIXES

## Optimization Pass Stats (last IR run)
$([ -n "$FOLDED" ] && echo "- $FOLDED" || echo "- No IR-Opts data")
$([ -n "$RLE" ] && echo "- $RLE")
$([ -n "$DCE" ] && echo "- $DCE")
$([ -n "$ESCAPE" ] && echo "- $ESCAPE")
$([ -n "$MEM2REG" ] && echo "- $MEM2REG")

## Architecture

- Dual-emission: AST-direct (part4.c codegen_expr/codegen_stmt) + IR backend (compiler_passes.c)
- IR gate: ir_whitelisted() in part4.c controls which functions use IR
- Hybrid frame: AST owns prologue/epilogue, IR owns body (body_only=1, slot_base=-stack_size)
- Bootstrap: GCC -> zcc -> zcc2.s -> zcc2 -> zcc3.s -> cmp zcc2.s zcc3.s
- Build: make clean && make selfhost
- IR test: bash verify_ir_backend.sh
- Environment: Windows + WSL, PowerShell -> wsl -e sh -c
- Working dir: /mnt/h/__DOWNLOADS/selforglinux

## Key Code Locations

| What | File | Line(s) |
|------|------|---------|
| ir_whitelisted gate | part4.c | ~1890 |
| codegen_func | part4.c | ~1909 |
| IR body entry | compiler_passes.c | zcc_run_passes_emit_body_pgo ~5394 |
| run_all_passes | compiler_passes.c | ~4451 |
| Mem2Reg (single-block) | compiler_passes.c | scalar_promotion_pass ~1478 |
| Mem2Reg (multi-block) | compiler_passes.c | multi_block_mem2reg_one ~1681 |
| PHI edge copy | compiler_passes.c | ir_asm_emit_phi_edge_copy ~4826 |
| IRAsmCtx struct | compiler_passes.c | ~4775 |
| ir_asm_vreg_location | compiler_passes.c | ~4790 |

## Known Issues
$(echo -e "$KNOWN_ISSUES")

## Next Steps (suggested)
- Register allocation improvements (reduce spills)
- Re-enable LICM pass
- ASan run to confirm SARIF CWE-416/415 findings
- Per-function regression test suite (zcc_test_suite.sh)
STATUSEOF

echo ""
echo -e "${GRN}✓ $OUT written ($( wc -l < "$OUT") lines)${RST}"
echo -e "${CYN}Paste this file into any new conversation for instant context.${RST}"

# Cleanup
rm -rf "$TMP_BUILD"
