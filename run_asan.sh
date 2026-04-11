#!/usr/bin/env bash
# =============================================================================
# run_asan.sh — ZCC AddressSanitizer Build & Diagnostic Script
# =============================================================================
# CG-IR-016 / rc=139 investigation
# Machine: GIGABYTE AERO X16 | WSL2 Ubuntu | Ryzen AI 7 350 | RTX 5070
# Working dir: /mnt/h/__DOWNLOADS/selforglinux
#
# Usage:
#   ./run_asan.sh [--skip-trivial] [--post-fix]
#
# Outputs (all in ./asan_logs/):
#   asan_ast_report.txt       AST path — should be clean
#   asan_ir_noopt_report.txt  IR path noopt — the rc=139 crasher
#   asan_ir_opt_report.txt    IR path with passes enabled
#   asan_trivial.txt          minimal C program
#   asan_calls.txt            function-call test
#   asan_strcmp.txt           strcmp-path test (hypothesis: CG-IR-016 → bad rbx)
#   asan_summary.txt          extracted errors + stack traces
#   asan_post_fix.txt         after CG-IR-016 fix (--post-fix flag)
#   gdb_rbx.txt               GDB watchpoint trace on %rbx
#   asan_error_delta.txt      before/after error count comparison
# =============================================================================

set -o pipefail
WORKDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LOG_DIR="$WORKDIR/asan_logs"
ZCC_SRC="$WORKDIR/zcc.c"
CP_SRC="$WORKDIR/compiler_passes.c"
ZCC_PP="$WORKDIR/zcc_pp.c"
ZCC_ASAN="$WORKDIR/zcc_asan"
ZCC_ASAN_FIXED="$WORKDIR/zcc_asan_fixed"

SKIP_TRIVIAL=0
POST_FIX=0
for arg in "$@"; do
    case "$arg" in
        --skip-trivial) SKIP_TRIVIAL=1 ;;
        --post-fix)     POST_FIX=1 ;;
    esac
done

# ---------------------------------------------------------------------------
# Preflight
# ---------------------------------------------------------------------------
echo "============================================================"
echo "  ZCC ASan Build & Diagnostic — $(date)"
echo "============================================================"
echo "  WORKDIR : $WORKDIR"
echo "  LOG_DIR : $LOG_DIR"
echo ""

mkdir -p "$LOG_DIR"

for f in "$ZCC_SRC" "$CP_SRC" "$ZCC_PP"; do
    if [[ ! -f "$f" ]]; then
        echo "ERROR: required file not found: $f"
        exit 1
    fi
done

if ! command -v gcc &>/dev/null; then
    echo "ERROR: gcc not in PATH"
    exit 1
fi

# Check if ASan runtime is available
echo -n "  ASan available: "
echo 'int main(void){return 0;}' | gcc -fsanitize=address -x c - -o /tmp/_asan_probe 2>/dev/null \
    && echo "YES" || { echo "NO — install libasan: sudo apt-get install libasan6"; exit 1; }

# ---------------------------------------------------------------------------
# Step 1 — Build ASan-instrumented ZCC
# ---------------------------------------------------------------------------
echo ""
echo "[ STEP 1 ] Building zcc_asan..."
gcc -fsanitize=address -fno-omit-frame-pointer -g -O0 \
    -o "$ZCC_ASAN" "$ZCC_SRC" "$CP_SRC" -lm 2>&1 | tee "$LOG_DIR/build_asan.txt"
if [[ ! -f "$ZCC_ASAN" ]]; then
    echo "BUILD FAILED — see $LOG_DIR/build_asan.txt"
    exit 1
fi
echo "  Built: $ZCC_ASAN"

# ---------------------------------------------------------------------------
# Common ASan options
# ---------------------------------------------------------------------------
ASAN_BASE="detect_stack_use_after_return=1"
ASAN_BASE="${ASAN_BASE}:check_initialization_order=1"
ASAN_BASE="${ASAN_BASE}:strict_string_checks=1"
ASAN_HALT="${ASAN_BASE}:halt_on_error=0"   # continue past first error
ASAN_STOP="${ASAN_BASE}:halt_on_error=1"   # stop at first error

# ---------------------------------------------------------------------------
# Step 2 — AST path (reference — should produce zero ASan errors)
# ---------------------------------------------------------------------------
echo ""
echo "[ STEP 2 ] AST path (expect CLEAN)..."
ASAN_OPTIONS="$ASAN_STOP" \
    "$ZCC_ASAN" "$ZCC_PP" -o /dev/null 2>&1 | tee "$LOG_DIR/asan_ast_report.txt"
AST_ERRORS=$(grep -c "ERROR: AddressSanitizer" "$LOG_DIR/asan_ast_report.txt" 2>/dev/null || echo 0)
echo "  AST path ASan errors: $AST_ERRORS"

# ---------------------------------------------------------------------------
# Step 3 — IR path noopt (the rc=139 crasher)
# ---------------------------------------------------------------------------
echo ""
echo "[ STEP 3 ] IR path — no-opt (expect CRASH or ASan report)..."
ZCC_IR_BACKEND=1 ASAN_OPTIONS="$ASAN_HALT" \
    "$ZCC_ASAN" "$ZCC_PP" -o /dev/null 2>&1 | tee "$LOG_DIR/asan_ir_noopt_report.txt"
NOOPT_ERRORS=$(grep -c "ERROR: AddressSanitizer" "$LOG_DIR/asan_ir_noopt_report.txt" 2>/dev/null || echo 0)
NOOPT_RC=${PIPESTATUS[0]}
echo "  IR noopt exit code: $NOOPT_RC  |  ASan errors: $NOOPT_ERRORS"

# ---------------------------------------------------------------------------
# Step 4 — IR path with passes enabled
# ---------------------------------------------------------------------------
echo ""
echo "[ STEP 4 ] IR path — with passes (optimized)..."
ZCC_IR_BACKEND=1 ZCC_PASSES_ENABLED=1 ASAN_OPTIONS="$ASAN_HALT" \
    "$ZCC_ASAN" "$ZCC_PP" -o /dev/null 2>&1 | tee "$LOG_DIR/asan_ir_opt_report.txt"
OPT_ERRORS=$(grep -c "ERROR: AddressSanitizer" "$LOG_DIR/asan_ir_opt_report.txt" 2>/dev/null || echo 0)
echo "  IR opt ASan errors: $OPT_ERRORS"

# ---------------------------------------------------------------------------
# Step 5 — Targeted isolation tests
# ---------------------------------------------------------------------------
if [[ "$SKIP_TRIVIAL" -eq 0 ]]; then
    echo ""
    echo "[ STEP 5 ] Isolation tests..."

    # 5a — trivial program
    echo 'int main(void) { return 0; }' > /tmp/zcc_trivial.c
    echo -n "  trivial.c: "
    ZCC_IR_BACKEND=1 ASAN_OPTIONS="$ASAN_HALT" \
        "$ZCC_ASAN" /tmp/zcc_trivial.c -o /dev/null 2>&1 | tee "$LOG_DIR/asan_trivial.txt" | \
        grep -c "ERROR:" || echo 0
    echo "  (see $LOG_DIR/asan_trivial.txt)"

    # 5b — function calls
    cat > /tmp/zcc_calls.c << 'TESTEOF'
int add(int a, int b) { return a + b; }
int main(void) { return add(1, 2) - 3; }
TESTEOF
    echo -n "  calls.c: "
    ZCC_IR_BACKEND=1 ASAN_OPTIONS="$ASAN_HALT" \
        "$ZCC_ASAN" /tmp/zcc_calls.c -o /dev/null 2>&1 | tee "$LOG_DIR/asan_calls.txt" | \
        grep -c "ERROR:" || echo 0
    echo "  (see $LOG_DIR/asan_calls.txt)"

    # 5c — strcmp path (hypothesis: CG-IR-016 corrupts %rbx → bad strcmp ptr)
    cat > /tmp/zcc_strcmp.c << 'TESTEOF'
int my_strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (int)((unsigned char)*a) - (int)((unsigned char)*b);
}
int main(void) {
    return my_strcmp("hello", "hello");
}
TESTEOF
    echo -n "  strcmp_test.c: "
    ZCC_IR_BACKEND=1 ASAN_OPTIONS="$ASAN_HALT" \
        "$ZCC_ASAN" /tmp/zcc_strcmp.c -o /dev/null 2>&1 | tee "$LOG_DIR/asan_strcmp.txt" | \
        grep -c "ERROR:" || echo 0
    echo "  (see $LOG_DIR/asan_strcmp.txt)"

    # 5d — callee-saved register stress test (chain of calls to force rbx corruption)
    cat > /tmp/zcc_csave_stress.c << 'TESTEOF'
int leaf(int x) { return x + 1; }
int middle(int x) { return leaf(x) + leaf(x + 1); }
int upper(int x) { return middle(x) + middle(x + 2); }
int main(void) { return upper(0) - 8; }
TESTEOF
    echo -n "  csave_stress.c: "
    ZCC_IR_BACKEND=1 ASAN_OPTIONS="$ASAN_HALT" \
        "$ZCC_ASAN" /tmp/zcc_csave_stress.c -o /dev/null 2>&1 | tee "$LOG_DIR/asan_csave_stress.txt" | \
        grep -c "ERROR:" || echo 0
    echo "  (see $LOG_DIR/asan_csave_stress.txt)"
fi

# ---------------------------------------------------------------------------
# Step 6 — Extract summary from noopt report
# ---------------------------------------------------------------------------
echo ""
echo "[ STEP 6 ] Extracting ASan summary..."
{
    echo "=== ZCC ASan Summary — $(date) ==="
    echo ""
    echo "--- CRASH HEADER ---"
    grep -A 5 "ERROR: AddressSanitizer" "$LOG_DIR/asan_ir_noopt_report.txt" || \
        echo "(no AddressSanitizer ERROR found)"
    echo ""
    echo "--- ACCESS TYPE ---"
    grep "WRITE of size\|READ of size\|SEGV on unknown address" \
         "$LOG_DIR/asan_ir_noopt_report.txt" | head -10 || \
        echo "(no explicit read/write line — likely register corruption, not heap)"
    echo ""
    echo "--- STACK TRACE (top 20) ---"
    grep -A 30 "ERROR: AddressSanitizer" "$LOG_DIR/asan_ir_noopt_report.txt" | \
        grep "#[0-9]" | head -20 || echo "(no stack trace)"
    echo ""
    echo "--- ALLOCATION SITE ---"
    grep "allocated by\|freed by\|created by" \
         "$LOG_DIR/asan_ir_noopt_report.txt" | head -10 || echo "(none)"
    echo ""
    echo "--- DIAGNOSIS ---"
    if grep -q "heap-use-after-free" "$LOG_DIR/asan_ir_noopt_report.txt"; then
        echo "HEAP-USE-AFTER-FREE: likely PassResult struct freed prematurely or"
        echo "  LP64 ABI mismatch (ZCC uint32_t=8B vs GCC uint32_t=4B)."
    elif grep -q "heap-buffer-overflow" "$LOG_DIR/asan_ir_noopt_report.txt"; then
        echo "HEAP-BUFFER-OVERFLOW: ir_buf overflow — check ir_extra sizing (CG-IR-009)."
    elif grep -q "stack-buffer-overflow" "$LOG_DIR/asan_ir_noopt_report.txt"; then
        echo "STACK-BUFFER-OVERFLOW: spill slot collision (CG-IR-008 regression?)."
    elif grep -q "SEGV on unknown address" "$LOG_DIR/asan_ir_noopt_report.txt"; then
        echo "SEGV ON UNKNOWN ADDRESS: ASan cannot catch this directly."
        echo "  Likely CG-IR-016 register corruption: corrupted %rbx used as pointer."
        echo "  → Run GDB rbx watchpoint (Step 7) for exact site."
        echo "  → Apply fix_cgir016.py and retest with --post-fix."
    else
        echo "No specific ASan error pattern matched."
        echo "  If rc=139 persists without ASan ERROR: pure register corruption."
        echo "  → CG-IR-016 is the prime suspect."
    fi
} > "$LOG_DIR/asan_summary.txt"
echo "  Written: $LOG_DIR/asan_summary.txt"
cat "$LOG_DIR/asan_summary.txt"

# ---------------------------------------------------------------------------
# Step 7 — GDB rbx watchpoint (register corruption detection)
# ---------------------------------------------------------------------------
echo ""
echo "[ STEP 7 ] GDB %rbx watchpoint script..."
cat > "$LOG_DIR/gdb_rbx_watch.gdb" << 'GDBEOF'
set pagination off
set confirm off
set print thread-events off

# Break at the IR body emitter entry
break ir_asm_emit_function_body
commands 1
    silent
    printf "=== ir_asm_emit_function_body entered, rbx=%lx ===\n", $rbx
    watch $rbx
    continue
end

# Break at zcc_run_passes_emit_body_pgo entry
break zcc_run_passes_emit_body_pgo
commands 2
    silent
    printf "=== emit_body_pgo entered, rbx=%lx ===\n", $rbx
    continue
end

run zcc_pp.c -o /dev/null
if $_siginfo
    printf "\n=== SIGNAL RECEIVED ===\n"
    bt 20
end
quit
GDBEOF

if command -v gdb &>/dev/null; then
    echo "  Running GDB watchpoint on IR path..."
    ZCC_IR_BACKEND=1 gdb -batch \
        -x "$LOG_DIR/gdb_rbx_watch.gdb" \
        "$ZCC_ASAN" 2>&1 | tee "$LOG_DIR/gdb_rbx.txt" | head -60
    echo "  Full output: $LOG_DIR/gdb_rbx.txt"
else
    echo "  GDB not found — skipping (install: sudo apt-get install gdb)"
    echo "  Script saved: $LOG_DIR/gdb_rbx_watch.gdb"
    echo "  Run manually: ZCC_IR_BACKEND=1 gdb -batch -x $LOG_DIR/gdb_rbx_watch.gdb $ZCC_ASAN"
fi

# ---------------------------------------------------------------------------
# Step 8 — Post-fix validation (--post-fix)
# ---------------------------------------------------------------------------
if [[ "$POST_FIX" -eq 1 ]]; then
    echo ""
    echo "[ STEP 8 ] Post-CG-IR-016-fix validation..."
    gcc -fsanitize=address -fno-omit-frame-pointer -g -O0 \
        -o "$ZCC_ASAN_FIXED" "$ZCC_SRC" "$CP_SRC" -lm 2>&1 | \
        tee "$LOG_DIR/build_asan_fixed.txt"
    if [[ ! -f "$ZCC_ASAN_FIXED" ]]; then
        echo "  POST-FIX BUILD FAILED"
    else
        ZCC_IR_BACKEND=1 ASAN_OPTIONS="$ASAN_HALT" \
            "$ZCC_ASAN_FIXED" "$ZCC_PP" -o /dev/null 2>&1 | \
            tee "$LOG_DIR/asan_post_fix.txt"
        POST_ERRORS=$(grep -c "ERROR: AddressSanitizer" \
                      "$LOG_DIR/asan_post_fix.txt" 2>/dev/null || echo 0)
        {
            echo "=== ERROR COUNT COMPARISON ==="
            echo "Before fix (noopt): $NOOPT_ERRORS ASan error(s)"
            echo "After  fix        : $POST_ERRORS ASan error(s)"
            if [[ "$POST_ERRORS" -lt "$NOOPT_ERRORS" ]]; then
                echo "RESULT: IMPROVEMENT — error count reduced"
            elif [[ "$POST_ERRORS" -eq 0 ]]; then
                echo "RESULT: CLEAN — all memory errors resolved"
            else
                echo "RESULT: SAME or WORSE — investigate further"
            fi
        } | tee "$LOG_DIR/asan_error_delta.txt"
    fi
fi

# ---------------------------------------------------------------------------
# Final summary
# ---------------------------------------------------------------------------
echo ""
echo "============================================================"
echo "  FINAL SUMMARY"
echo "============================================================"
echo "  AST path errors          : $AST_ERRORS"
echo "  IR noopt errors/rc       : $NOOPT_ERRORS / $NOOPT_RC"
echo "  IR opt errors            : $OPT_ERRORS"
echo "  Logs directory           : $LOG_DIR"
echo ""
echo "  INTERPRETATION:"
echo "    • ASan detects MEMORY bugs (heap-overflow, UAF, stack-overflow)."
echo "    • ASan does NOT detect pure REGISTER corruption."
echo "    • If noopt rc=139 with no ASan ERROR: CG-IR-016 (bad %%rbx)"
echo "      is the likely cause — apply fix_cgir016.py, then rerun with"
echo "      --post-fix to confirm."
echo "    • If ASan reports heap-use-after-free near PassResult: LP64"
echo "      ABI mismatch (ZCC uint32_t=8 vs GCC uint32_t=4)."
echo "============================================================"
