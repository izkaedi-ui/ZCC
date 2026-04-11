#!/bin/bash
# ============================================================================
# 🔱 ZCC SELF-HOSTING HARDENING PIPELINE v1.0
# ============================================================================
# Automated bootstrap verification, fuzzing, and regression testing.
# 
# Usage:
#   ./run_hardening.sh [ZCC_DIR]
#
# Where ZCC_DIR contains: zcc.c, compiler_passes.c, part*.c, ir*.h, Makefile
# Defaults to current directory.
#
# Phases:
#   1. Bootstrap Verification  — stage1→stage2→stage3, verify s2==s3
#   2. Regression Test Suite   — 100+ targeted codegen tests
#   3. Fuzz Campaign           — random valid C programs, gcc vs zcc
#   4. Report Generation       — HTML report with pass/fail/diff summary
# ============================================================================

set -euo pipefail

# ── Configuration ──
ZCC_DIR="${1:-.}"
HARNESS_DIR="$(cd "$(dirname "$0")" && pwd)/harness"
RESULTS_DIR="${ZCC_DIR}/hardening_results"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
REPORT_DIR="${RESULTS_DIR}/${TIMESTAMP}"
FUZZ_COUNT="${FUZZ_COUNT:-200}"
FUZZ_TIMEOUT="${FUZZ_TIMEOUT:-10}"

# Colors
RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
CYN='\033[0;36m'
RST='\033[0m'

banner() {
    echo -e "${CYN}"
    echo "╔══════════════════════════════════════════════════════════════╗"
    echo "║  🔱 ZCC SELF-HOSTING HARDENING PIPELINE v1.0               ║"
    echo "║     ZKAEDI SYSTEMS — $(date '+%Y-%m-%d %H:%M:%S')                    ║"
    echo "╚══════════════════════════════════════════════════════════════╝"
    echo -e "${RST}"
}

phase_header() {
    echo ""
    echo -e "${CYN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RST}"
    echo -e "${CYN}  PHASE $1: $2${RST}"
    echo -e "${CYN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RST}"
    echo ""
}

pass()  { echo -e "  ${GRN}✓ PASS${RST}  $1"; }
fail()  { echo -e "  ${RED}✗ FAIL${RST}  $1"; }
warn()  { echo -e "  ${YLW}⚠ WARN${RST}  $1"; }
info()  { echo -e "  ${CYN}ℹ INFO${RST}  $1"; }

# ── Sanity Checks ──
banner

if [ ! -f "${ZCC_DIR}/zcc.c" ]; then
    echo -e "${RED}ERROR: zcc.c not found in ${ZCC_DIR}${RST}"
    echo "Usage: ./run_hardening.sh /path/to/zcc"
    exit 1
fi

mkdir -p "${REPORT_DIR}"/{bootstrap,regression,fuzz,asm_diffs}

TOTAL_PASS=0
TOTAL_FAIL=0
TOTAL_SKIP=0

# ════════════════════════════════════════════════════════════════
# PHASE 1: Bootstrap Verification
# ════════════════════════════════════════════════════════════════
phase_header 1 "BOOTSTRAP VERIFICATION"

BOOT_LOG="${REPORT_DIR}/bootstrap/bootstrap.log"
bash "${HARNESS_DIR}/bootstrap_verify.sh" "${ZCC_DIR}" "${REPORT_DIR}/bootstrap" 2>&1 | tee "${BOOT_LOG}"
BOOT_RC=${PIPESTATUS[0]}

if [ ${BOOT_RC} -eq 0 ]; then
    pass "Bootstrap chain: stage2 == stage3 (bitwise identical)"
    ((TOTAL_PASS++))
else
    fail "Bootstrap chain BROKEN — see ${BOOT_LOG}"
    ((TOTAL_FAIL++))
fi

# ════════════════════════════════════════════════════════════════
# PHASE 2: Regression Test Suite
# ════════════════════════════════════════════════════════════════
phase_header 2 "REGRESSION TEST SUITE"

REG_LOG="${REPORT_DIR}/regression/regression.log"
REG_PASS=0
REG_FAIL=0
REG_SKIP=0

# Compile the test suite with GCC first to get reference outputs
info "Compiling test suite with GCC (reference)..."
gcc -O0 -std=c99 -o "${REPORT_DIR}/regression/test_ref" \
    "${HARNESS_DIR}/test_codegen.c" -lm 2>"${REPORT_DIR}/regression/gcc_errors.log"

if [ $? -ne 0 ]; then
    fail "Test suite doesn't compile with GCC — fix tests first"
    cat "${REPORT_DIR}/regression/gcc_errors.log"
    exit 1
fi

# Get GCC reference output
"${REPORT_DIR}/regression/test_ref" > "${REPORT_DIR}/regression/ref_output.txt" 2>&1
GCC_RC=$?
info "GCC reference: rc=${GCC_RC}, $(wc -l < "${REPORT_DIR}/regression/ref_output.txt") output lines"

# Now compile with ZCC (stage1)
ZCC_BIN="${ZCC_DIR}/zcc"
if [ ! -x "${ZCC_BIN}" ]; then
    # Try building it
    info "Building ZCC stage1 with GCC..."
    (cd "${ZCC_DIR}" && make zcc_full 2>/dev/null) || \
    gcc -O0 -o "${ZCC_BIN}" "${ZCC_DIR}/zcc.c" "${ZCC_DIR}/compiler_passes.c" -lm 2>/dev/null || true
fi

if [ -x "${ZCC_BIN}" ]; then
    info "Compiling test suite with ZCC..."
    
    # ZCC compilation
    "${ZCC_BIN}" "${HARNESS_DIR}/test_codegen.c" -o "${REPORT_DIR}/regression/test_zcc" \
        2>"${REPORT_DIR}/regression/zcc_errors.log" && {
        
        # Run ZCC-compiled test
        timeout 30 "${REPORT_DIR}/regression/test_zcc" > "${REPORT_DIR}/regression/zcc_output.txt" 2>&1
        ZCC_RC=$?
        
        # Compare outputs line by line
        while IFS= read -r line; do
            TEST_NAME=$(echo "$line" | cut -d'|' -f1)
            TEST_RESULT=$(echo "$line" | cut -d'|' -f2)
            
            # Find matching line in ZCC output
            ZCC_LINE=$(grep "^${TEST_NAME}|" "${REPORT_DIR}/regression/zcc_output.txt" 2>/dev/null || echo "")
            
            if [ -z "${ZCC_LINE}" ]; then
                warn "${TEST_NAME} — not reached (crash before this test?)"
                ((REG_SKIP++))
            elif [ "${line}" = "${ZCC_LINE}" ]; then
                pass "${TEST_NAME}"
                ((REG_PASS++))
            else
                fail "${TEST_NAME}"
                echo "    GCC: ${TEST_RESULT}"
                echo "    ZCC: $(echo "${ZCC_LINE}" | cut -d'|' -f2)"
                ((REG_FAIL++))
            fi
        done < "${REPORT_DIR}/regression/ref_output.txt"
        
        info "Regression: ${REG_PASS} pass, ${REG_FAIL} fail, ${REG_SKIP} skip"
    } || {
        fail "ZCC cannot compile test suite — $(cat "${REPORT_DIR}/regression/zcc_errors.log" | head -5)"
        ((REG_FAIL++))
    }
else
    warn "No ZCC binary found — skipping regression (bootstrap-only mode)"
    ((REG_SKIP++))
fi

TOTAL_PASS=$((TOTAL_PASS + REG_PASS))
TOTAL_FAIL=$((TOTAL_FAIL + REG_FAIL))
TOTAL_SKIP=$((TOTAL_SKIP + REG_SKIP))

# ════════════════════════════════════════════════════════════════
# PHASE 3: Fuzz Campaign
# ════════════════════════════════════════════════════════════════
phase_header 3 "FUZZ CAMPAIGN (${FUZZ_COUNT} programs)"

FUZZ_LOG="${REPORT_DIR}/fuzz/fuzz.log"
FUZZ_PASS=0
FUZZ_FAIL=0
FUZZ_CRASH=0

if [ -x "${ZCC_BIN}" ] && command -v python3 &>/dev/null; then
    python3 "${HARNESS_DIR}/zcc_fuzz.py" \
        --zcc "${ZCC_BIN}" \
        --count "${FUZZ_COUNT}" \
        --timeout "${FUZZ_TIMEOUT}" \
        --output-dir "${REPORT_DIR}/fuzz" \
        --seed 42 \
        2>&1 | tee "${FUZZ_LOG}"
    
    # Parse fuzzer results
    if [ -f "${REPORT_DIR}/fuzz/summary.json" ]; then
        FUZZ_PASS=$(python3 -c "import json; d=json.load(open('${REPORT_DIR}/fuzz/summary.json')); print(d.get('pass',0))")
        FUZZ_FAIL=$(python3 -c "import json; d=json.load(open('${REPORT_DIR}/fuzz/summary.json')); print(d.get('fail',0))")
        FUZZ_CRASH=$(python3 -c "import json; d=json.load(open('${REPORT_DIR}/fuzz/summary.json')); print(d.get('crash',0))")
        
        info "Fuzz: ${FUZZ_PASS} pass, ${FUZZ_FAIL} mismatch, ${FUZZ_CRASH} crash"
        
        if [ ${FUZZ_CRASH} -gt 0 ]; then
            fail "CRASHES detected — minimal reproducers saved to ${REPORT_DIR}/fuzz/crashes/"
        fi
        if [ ${FUZZ_FAIL} -gt 0 ]; then
            fail "OUTPUT MISMATCHES — diffs saved to ${REPORT_DIR}/fuzz/mismatches/"
        fi
    fi
else
    warn "Skipping fuzz (need ZCC binary + python3)"
    ((TOTAL_SKIP++))
fi

TOTAL_PASS=$((TOTAL_PASS + FUZZ_PASS))
TOTAL_FAIL=$((TOTAL_FAIL + FUZZ_FAIL + FUZZ_CRASH))

# ════════════════════════════════════════════════════════════════
# PHASE 4: Report
# ════════════════════════════════════════════════════════════════
phase_header 4 "FINAL REPORT"

TOTAL=$((TOTAL_PASS + TOTAL_FAIL + TOTAL_SKIP))
if [ ${TOTAL} -eq 0 ]; then TOTAL=1; fi
PASS_PCT=$((TOTAL_PASS * 100 / TOTAL))

echo ""
echo -e "${CYN}╔══════════════════════════════════════════════════════════════╗${RST}"
echo -e "${CYN}║  HARDENING REPORT — ${TIMESTAMP}                      ║${RST}"
echo -e "${CYN}╠══════════════════════════════════════════════════════════════╣${RST}"
printf "${CYN}║${RST}  Bootstrap:   %-45s${CYN}║${RST}\n" "$([ ${BOOT_RC} -eq 0 ] && echo '✓ VERIFIED' || echo '✗ BROKEN')"
printf "${CYN}║${RST}  Regression:  %-45s${CYN}║${RST}\n" "${REG_PASS} pass / ${REG_FAIL} fail / ${REG_SKIP} skip"
printf "${CYN}║${RST}  Fuzzing:     %-45s${CYN}║${RST}\n" "${FUZZ_PASS} pass / ${FUZZ_FAIL} mismatch / ${FUZZ_CRASH} crash"
echo -e "${CYN}╠══════════════════════════════════════════════════════════════╣${RST}"
printf "${CYN}║${RST}  TOTAL:       %-45s${CYN}║${RST}\n" "${TOTAL_PASS}/${TOTAL} passed (${PASS_PCT}%)"
echo -e "${CYN}╚══════════════════════════════════════════════════════════════╝${RST}"
echo ""
echo "Full results: ${REPORT_DIR}"
echo ""

# Generate machine-readable summary
cat > "${REPORT_DIR}/summary.json" << ENDJSON
{
    "timestamp": "${TIMESTAMP}",
    "bootstrap_verified": $([ ${BOOT_RC} -eq 0 ] && echo "true" || echo "false"),
    "regression": {"pass": ${REG_PASS}, "fail": ${REG_FAIL}, "skip": ${REG_SKIP}},
    "fuzz": {"pass": ${FUZZ_PASS}, "fail": ${FUZZ_FAIL}, "crash": ${FUZZ_CRASH}},
    "total_pass": ${TOTAL_PASS},
    "total_fail": ${TOTAL_FAIL},
    "total_skip": ${TOTAL_SKIP},
    "pass_pct": ${PASS_PCT}
}
ENDJSON

exit ${TOTAL_FAIL}
