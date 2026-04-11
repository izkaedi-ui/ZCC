#!/usr/bin/env bash
# =============================================================================
# verify_invariants.sh — ZCC Peephole Regression Harness
# =============================================================================
#
# WHAT THIS DOES
# --------------
# 1. BOOTSTRAP: gcc → zcc → zcc2 → zcc3 (full 3-stage chain)
# 2. SIDE-EFFECT TESTS: compiles and runs test_t2.c against zcc2
# 3. ASM HASH: sha256 of zcc2.s and zcc3.s — detects silent divergence
# 4. PUSH/POP RATIO: counts pushq/popq pairs eliminated by peephole
# 5. T3 RATIO: counts idivq replacements (sarq / andq substitutions)
# 6. SELF-HOST DIFF: cmp zcc2.s zcc3.s must produce zero output
#
# USAGE
# -----
#   chmod +x verify_invariants.sh
#   ./verify_invariants.sh                   # full run
#   ./verify_invariants.sh --quick           # skip T3 emission scan
#   ./verify_invariants.sh --bootstrap-only  # stop after stage3 build
#
# EXIT CODES
#   0  → ALL INVARIANTS HOLD
#   1  → SELF-HOST DIFF FAILED (zcc2.s ≠ zcc3.s)
#   2  → SIDE-EFFECT TESTS FAILED
#   3  → BUILD FAILED
#   4  → MISSING DEPENDENCY
#
# CONFIGURATION — override via environment
ZCC_SRC="${ZCC_SRC:-zcc.c}"
ZCC_PASSES="${ZCC_PASSES:-compiler_passes.c}"
TEST_SIDE_EFFECTS="${TEST_SIDE_EFFECTS:-test_t2.c}"
REPORT_FILE="${REPORT_FILE:-invariants_report.txt}"
# =============================================================================

set -euo pipefail

# ── Colours ─────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; CYAN='\033[0;36m'
YELLOW='\033[1;33m'; BOLD='\033[1m'; RESET='\033[0m'
ok()   { echo -e "  ${GREEN}✓${RESET}  $*"; }
fail() { echo -e "  ${RED}✗${RESET}  $*"; }
info() { echo -e "  ${CYAN}→${RESET}  $*"; }
warn() { echo -e "  ${YELLOW}!${RESET}  $*"; }
hdr()  { echo -e "\n${BOLD}${CYAN}═══ $* ═══${RESET}"; }

# ── Argument parsing ─────────────────────────────────────────────────────────
QUICK=0; BOOTSTRAP_ONLY=0
for arg in "$@"; do
    case "$arg" in
        --quick)           QUICK=1 ;;
        --bootstrap-only)  BOOTSTRAP_ONLY=1 ;;
        --help|-h)
            sed -n '4,30p' "$0"
            exit 0 ;;
    esac
done

# ── Dependency check ─────────────────────────────────────────────────────────
hdr "DEPENDENCY CHECK"
for cmd in gcc sha256sum cmp grep wc awk; do
    if command -v "$cmd" &>/dev/null; then ok "$cmd"; else fail "$cmd not found"; exit 4; fi
done

# ── Scratch area ─────────────────────────────────────────────────────────────
WORK=$(mktemp -d /tmp/zcc_invariants_XXXXXX)
# trap 'rm -rf "$WORK"' EXIT
info "Scratch dir: $WORK"

# ── Report accumulator ───────────────────────────────────────────────────────
declare -A METRICS=(
    [BOOTSTRAP_OK]=0
    [SELFHOST_DIFF]=0
    [SIDE_EFFECT_OK]=0
    [ZCC2_SHA256]="(not computed)"
    [ZCC3_SHA256]="(not computed)"
    [PUSHQ_TOTAL_ZCC2]=0
    [POPQ_TOTAL_ZCC2]=0
    [PUSH_POP_PAIRS]=0
    [IDIVQ_REMAINING]=0
    [T3_DIV_SARQ]=0
    [T3_MOD_ANDQ]=0
    [T3_ANNIHILATOR]=0
    [CONSTANT_FOLD_FOLDS]=0
)

# =============================================================================
# STAGE 1 — BOOTSTRAP CHAIN
# =============================================================================
hdr "STAGE 1 — BOOTSTRAP"

# 1a. GCC builds zcc (stage0 → stage1)
info "gcc → zcc (stage0)"
if [[ -f "$ZCC_PASSES" ]]; then
    gcc -O0 -o "$WORK/zcc" "$ZCC_SRC" "$ZCC_PASSES" -lm 2>"$WORK/gcc.log"
else
    gcc -O0 -o "$WORK/zcc" "$ZCC_SRC" 2>"$WORK/gcc.log"
fi || { fail "GCC build failed"; cat "$WORK/gcc.log"; exit 3; }
ok "stage0 built: $WORK/zcc"

# 1b. Preprocess for self-hosting (strip #includes ZCC can't parse)
info "Preprocessing $ZCC_SRC for self-host"
if [[ -f "zcc_ast_bridge_zcc.h" ]]; then
    sed '/^_Static_assert/d' "$ZCC_SRC" > "$WORK/zcc_pp.c"
    sed -e '/#include "zcc_ast_bridge.h"/r zcc_ast_bridge_zcc.h' -e '/#include "zcc_ast_bridge.h"/d' "$WORK/zcc_pp.c" > "$WORK/zcc_pp.c.tmp" && mv "$WORK/zcc_pp.c.tmp" "$WORK/zcc_pp.c"
    sed '/^#include[[:space:]]*<[^>]*>/d' "$WORK/zcc_pp.c" > "$WORK/zcc_pp.c.tmp" && mv "$WORK/zcc_pp.c.tmp" "$WORK/zcc_pp.c"
else
    warn "zcc_ast_bridge_zcc.h not found — using raw $ZCC_SRC"
    sed '/^_Static_assert/d' "$ZCC_SRC" > "$WORK/zcc_pp.c"
    sed '/^#include[[:space:]]*<[^>]*>/d' "$WORK/zcc_pp.c" > "$WORK/zcc_pp.c.tmp" && mv "$WORK/zcc_pp.c.tmp" "$WORK/zcc_pp.c"
fi

# 1c. zcc compiles itself → stage2, capturing assembly
info "zcc → zcc2.s (stage1 → stage2)"
"$WORK/zcc" "$WORK/zcc_pp.c" -o "$WORK/zcc2.s" 2>"$WORK/zcc2.log" || {
    fail "Stage2 asm failed"; cat "$WORK/zcc2.log"; exit 3; }
gcc -O0 -w -fno-asynchronous-unwind-tables -o "$WORK/zcc2" "$WORK/zcc2.s" "$ZCC_PASSES" -lm 2>>"$WORK/zcc2.log" || {
    fail "Stage2 link failed"; cat "$WORK/zcc2.log"; exit 3; }
ok "stage2 built"

# 1e. zcc2 compiles itself → stage3
info "zcc2 → zcc3.s (stage2 → stage3)"
"$WORK/zcc2" "$WORK/zcc_pp.c" -o "$WORK/zcc3.s" 2>"$WORK/zcc3.log" || {
    fail "Stage3 asm failed"; cat "$WORK/zcc3.log"; exit 3; }
gcc -O0 -w -fno-asynchronous-unwind-tables -o "$WORK/zcc3" "$WORK/zcc3.s" "$ZCC_PASSES" -lm 2>>"$WORK/zcc3.log" || {
    fail "Stage3 link failed"; cat "$WORK/zcc3.log"; exit 3; }
ok "stage3 built"

METRICS[BOOTSTRAP_OK]=1
ok "Bootstrap chain complete"

[[ $BOOTSTRAP_ONLY -eq 1 ]] && { echo "Stopped after bootstrap (--bootstrap-only)."; exit 0; }

# =============================================================================
# STAGE 2 — SELF-HOST INVARIANT (cmp zcc2.s zcc3.s)
# =============================================================================
hdr "STAGE 2 — SELF-HOST DIFF"

# Hash both assembly files
if [[ -f "$WORK/zcc2.s" && -s "$WORK/zcc2.s" ]]; then
    METRICS[ZCC2_SHA256]=$(sha256sum "$WORK/zcc2.s" | awk '{print $1}')
    METRICS[ZCC3_SHA256]=$(sha256sum "$WORK/zcc3.s" | awk '{print $1}')
    info "zcc2.s SHA256: ${METRICS[ZCC2_SHA256]}"
    info "zcc3.s SHA256: ${METRICS[ZCC3_SHA256]}"
fi

DIFF_OUTPUT=$(diff "$WORK/zcc2.s" "$WORK/zcc3.s" 2>/dev/null || true)
if [[ -z "$DIFF_OUTPUT" ]]; then
    ok "SELF-HOST OK — zcc2.s == zcc3.s (0 diffs)"
    METRICS[SELFHOST_DIFF]=0
else
    DIFF_LINES=$(echo "$DIFF_OUTPUT" | wc -l)
    fail "SELF-HOST FAILED — $DIFF_LINES divergent lines"
    echo "$DIFF_OUTPUT" | head -40
    echo "$DIFF_OUTPUT" > "$WORK/selfhost_diff.txt"
    info "Full diff saved to $WORK/selfhost_diff.txt"
    METRICS[SELFHOST_DIFF]=$DIFF_LINES
fi

# =============================================================================
# STAGE 3 — SIDE-EFFECT REGRESSION TESTS (test_t2.c)
# =============================================================================
hdr "STAGE 3 — SIDE-EFFECT REGRESSION (${TEST_SIDE_EFFECTS})"

if [[ ! -f "$TEST_SIDE_EFFECTS" ]]; then
    warn "$TEST_SIDE_EFFECTS not found — skipping side-effect tests"
    METRICS[SIDE_EFFECT_OK]=-1
else
    info "Compiling $TEST_SIDE_EFFECTS with zcc2"
    "$WORK/zcc2" "$TEST_SIDE_EFFECTS" -o "$WORK/test_t2.s" 2>"$WORK/test_compile.log" || {
        fail "test_t2.c compilation failed with zcc2"; cat "$WORK/test_compile.log"
        METRICS[SIDE_EFFECT_OK]=0
    }
    gcc -o "$WORK/test_t2_bin" "$WORK/test_t2.s" 2>>"$WORK/test_compile.log" || {
        fail "test_t2.c GCC link failed"; cat "$WORK/test_compile.log"
        METRICS[SIDE_EFFECT_OK]=0
    }

    if [[ -x "$WORK/test_t2_bin" ]]; then
        TEST_OUT=$("$WORK/test_t2_bin" 2>&1 || true)
        if echo "$TEST_OUT" | grep -q "TIER-2 TESTS OK"; then
            ok "TIER-2 TESTS OK"
            METRICS[SIDE_EFFECT_OK]=1
        else
            fail "Side-effect tests FAILED"
            echo "$TEST_OUT"
            METRICS[SIDE_EFFECT_OK]=0
        fi
    fi
fi

# =============================================================================
# STAGE 4 — PUSH/POP EFFICIENCY ANALYSIS
# =============================================================================
hdr "STAGE 4 — PUSH/POP EFFICIENCY RATIO"

if [[ -f "$WORK/zcc2.s" && -s "$WORK/zcc2.s" ]]; then
    PUSHQ=$(grep -c '^\s*pushq' "$WORK/zcc2.s" 2>/dev/null || echo 0)
    POPQ=$(grep  -c '^\s*popq'  "$WORK/zcc2.s" 2>/dev/null || echo 0)
    METRICS[PUSHQ_TOTAL_ZCC2]=$PUSHQ
    METRICS[POPQ_TOTAL_ZCC2]=$POPQ
    # Conservative: pairs = min(pushq, popq) in zcc2.s
    PAIRS=$(( PUSHQ < POPQ ? PUSHQ : POPQ ))
    METRICS[PUSH_POP_PAIRS]=$PAIRS

    info "pushq instructions in zcc2.s: $PUSHQ"
    info "popq  instructions in zcc2.s: $POPQ"

    # Estimate baseline (naive stack machine: every binary op = 1 pair)
    # Compare against a reference compiled with NO peephole (if available)
    if [[ -f "zcc2_baseline.s" ]]; then
        BASE_PUSH=$(grep -c '^\s*pushq' zcc2_baseline.s 2>/dev/null || echo 0)
        BASE_POP=$(grep  -c '^\s*popq'  zcc2_baseline.s 2>/dev/null || echo 0)
        BASE_PAIRS=$(( BASE_PUSH < BASE_POP ? BASE_PUSH : BASE_POP ))
        ELIDED=$(( BASE_PAIRS - PAIRS ))
        PCT=0
        [[ $BASE_PAIRS -gt 0 ]] && PCT=$(( ELIDED * 100 / BASE_PAIRS ))
        ok "Push/pop pairs eliminated: $ELIDED / $BASE_PAIRS (${PCT}% reduction)"
        info "(Baseline from zcc2_baseline.s — regenerate with: NO_PEEPHOLE=1 ./verify_invariants.sh)"
    else
        warn "No baseline file (zcc2_baseline.s) — run once without T1/T2/T3 to establish baseline"
        info "Current pairs in optimized binary: $PAIRS"
    fi
else
    warn "Assembly file empty or missing — push/pop ratio unavailable"
fi

# =============================================================================
# STAGE 5 — T3 TIER EMISSION ANALYSIS (idivq/sarq/andq scan)
# =============================================================================
if [[ $QUICK -eq 0 ]]; then
hdr "STAGE 5 — T3 TIER EMISSION ANALYSIS"

if [[ -f "$WORK/zcc2.s" && -s "$WORK/zcc2.s" ]]; then
    IDIVQ=$(grep -c '^\s*idivq'  "$WORK/zcc2.s" 2>/dev/null || echo 0)
    DIVQ=$(grep  -c '^\s*divq'   "$WORK/zcc2.s" 2>/dev/null || echo 0)
    SARQ=$(grep  -c '^\s*sarq'   "$WORK/zcc2.s" 2>/dev/null || echo 0)
    # Filter sarq $63 (sign-bit propagation in rounding fixup vs other uses)
    SARQ_DIV=$(grep -c '^\s*sarq\s*\$[0-9]' "$WORK/zcc2.s" 2>/dev/null || echo 0)
    ANDQ_MASK=$(grep -c '^\s*andq\s*\$[0-9]' "$WORK/zcc2.s" 2>/dev/null || echo 0)

    METRICS[IDIVQ_REMAINING]=$IDIVQ
    METRICS[T3_DIV_SARQ]=$SARQ_DIV
    METRICS[T3_MOD_ANDQ]=$ANDQ_MASK

    info "idivq (generic signed div) remaining: $IDIVQ"
    info "sarq  \$k (T3-A strength reductions):  $SARQ_DIV"
    info "andq  \$k (T3-B mod reductions):       $ANDQ_MASK"

    if [[ $SARQ_DIV -gt 0 ]]; then
        ok "T3-A ACTIVE — $SARQ_DIV division(s) reduced to arithmetic shift"
    else
        warn "T3-A INACTIVE — no sarq \$k emissions found (expected > 0 post-T3 patch)"
    fi

    if [[ $ANDQ_MASK -gt 0 ]]; then
        ok "T3-B ACTIVE — $ANDQ_MASK modulo(s) reduced to bitwise AND"
    else
        warn "T3-B INACTIVE — no andq mask emissions found (may be zero if no mod by power-of-2 in zcc.c)"
    fi

    # Count constant-fold eliminations: movq $FOLDED (preceded by no codegen for either operand)
    # Heuristic: movq $<signed-int>, %rax not preceded by any arithmetic
    CONST_MOVQ=$(grep -c '^\s*movq\s*\$-\?[0-9]' "$WORK/zcc2.s" 2>/dev/null || echo 0)
    METRICS[CONSTANT_FOLD_FOLDS]=$CONST_MOVQ
    info "movq \$IMM (constant-folded or immediate load): $CONST_MOVQ"
else
    warn "Assembly file unavailable — T3 analysis skipped"
fi
fi  # --quick

# =============================================================================
# STAGE 6 — FINAL REPORT
# =============================================================================
hdr "FINAL REPORT"

{
echo "=================================================="
echo " ZCC PEEPHOLE INVARIANT REPORT"
echo " Generated: $(date -u '+%Y-%m-%d %H:%M:%S UTC')"
echo "=================================================="
echo ""
echo "BOOTSTRAP"
echo "  Stage0 (gcc → zcc):    OK"
echo "  Stage1 (zcc → zcc2):   $([ ${METRICS[BOOTSTRAP_OK]} -eq 1 ] && echo OK || echo FAIL)"
echo "  Stage2 (zcc2 → zcc3):  $([ ${METRICS[BOOTSTRAP_OK]} -eq 1 ] && echo OK || echo FAIL)"
echo ""
echo "SELF-HOST DIFF"
echo "  zcc2.s SHA256: ${METRICS[ZCC2_SHA256]}"
echo "  zcc3.s SHA256: ${METRICS[ZCC3_SHA256]}"
if [[ ${METRICS[SELFHOST_DIFF]} -eq 0 ]]; then
    echo "  Diff lines: 0  ← INVARIANT HOLDS"
else
    echo "  Diff lines: ${METRICS[SELFHOST_DIFF]}  ← INVARIANT BROKEN"
fi
echo ""
echo "SIDE-EFFECT REGRESSION"
case ${METRICS[SIDE_EFFECT_OK]} in
    1)  echo "  TIER-2 TESTS OK" ;;
    0)  echo "  FAILED" ;;
    -1) echo "  SKIPPED (test_t2.c not found)" ;;
esac
echo ""
echo "PUSH/POP EFFICIENCY"
echo "  pushq instructions: ${METRICS[PUSHQ_TOTAL_ZCC2]}"
echo "  popq  instructions: ${METRICS[POPQ_TOTAL_ZCC2]}"
echo "  Residual pairs:     ${METRICS[PUSH_POP_PAIRS]}"
echo ""
if [[ $QUICK -eq 0 ]]; then
echo "T3 TIER EMISSION"
echo "  idivq remaining:    ${METRICS[IDIVQ_REMAINING]}"
echo "  T3-A sarq \$k:       ${METRICS[T3_DIV_SARQ]}"
echo "  T3-B andq mask:     ${METRICS[T3_MOD_ANDQ]}"
echo "  Constant folds:     ${METRICS[CONSTANT_FOLD_FOLDS]}"
echo ""
fi
echo "=================================================="
} | tee "$REPORT_FILE"

# ── Exit code ─────────────────────────────────────────────────────────────────
echo ""
OVERALL_OK=1

if [[ ${METRICS[SELFHOST_DIFF]} -ne 0 ]]; then
    fail "SELF-HOST INVARIANT BROKEN — zcc2.s ≠ zcc3.s"
    OVERALL_OK=0
fi

if [[ ${METRICS[SIDE_EFFECT_OK]} -eq 0 ]]; then
    fail "SIDE-EFFECT REGRESSION DETECTED"
    OVERALL_OK=0
fi

if [[ $OVERALL_OK -eq 1 ]]; then
    echo -e "\n${BOLD}${GREEN}╔═══════════════════════════╗${RESET}"
    echo -e "${BOLD}${GREEN}║   ALL INVARIANTS HOLD ✓   ║${RESET}"
    echo -e "${BOLD}${GREEN}╚═══════════════════════════╝${RESET}\n"
    exit 0
else
    echo -e "\n${BOLD}${RED}╔══════════════════════════════════╗${RESET}"
    echo -e "${BOLD}${RED}║   ✗ INVARIANT VIOLATION DETECTED ║${RESET}"
    echo -e "${BOLD}${RED}╚══════════════════════════════════╝${RESET}\n"
    # Specific exit codes for CI triage
    [[ ${METRICS[SELFHOST_DIFF]} -ne 0 ]] && exit 1
    [[ ${METRICS[SIDE_EFFECT_OK]} -eq 0 ]] && exit 2
fi
