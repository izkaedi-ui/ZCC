#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════
# 🔱 ZCC DEBUG PROTOCOL v1.0 — Silent Failure Isolation
# ═══════════════════════════════════════════════════════════════════════
#
# TARGET: zcc3_ir exits 1 with zero output when compiling zcc_pp.c
#
# HYPOTHESIS TREE:
#   H1: stderr/stdout pointer corruption (CG-IR-003 class — sign extension)
#   H2: File I/O miscompilation (fopen returns valid but fprintf is broken)
#   H3: Lexer/parser early crash (token buffer or AST allocation fails)
#   H4: IR bridge side-effect corrupts assembly output (IR alters codegen)
#
# METHODOLOGY: Compiler Forge Narrowing Principle
#   Phase 0 — Environment sanity
#   Phase 1 — Binary comparison (reference vs IR-compiled)
#   Phase 2 — Minimal input probing
#   Phase 3 — Assembly diffing (function-level)
#   Phase 4 — GDB breakpoint isolation
# ═══════════════════════════════════════════════════════════════════════

set -euo pipefail

WORKDIR="$(cd "$(dirname "$0")" && pwd)"
LOG="$WORKDIR/debug_protocol.log"
DIVIDER="═══════════════════════════════════════════════════════════"

log() { echo "[$(date +%H:%M:%S)] $*" | tee -a "$LOG"; }
section() { echo -e "\n$DIVIDER" | tee -a "$LOG"; log "PHASE $1: $2"; echo "$DIVIDER" | tee -a "$LOG"; }

> "$LOG"
log "🔱 ZCC DEBUG PROTOCOL — $(date)"
log "Working directory: $WORKDIR"

# ───────────────────────────────────────────────────────────────────────
section 0 "ENVIRONMENT SANITY"
# ───────────────────────────────────────────────────────────────────────

# Confirm binaries exist
for BIN in zcc zcc2 zcc3_ir; do
    if [ -f "$WORKDIR/$BIN" ]; then
        SIZE=$(stat -c%s "$WORKDIR/$BIN" 2>/dev/null || stat -f%z "$WORKDIR/$BIN" 2>/dev/null)
        MTIME=$(stat -c%y "$WORKDIR/$BIN" 2>/dev/null || stat -f "%Sm" "$WORKDIR/$BIN" 2>/dev/null)
        log "  ✓ $BIN exists — $SIZE bytes — modified $MTIME"
    else
        log "  ✗ $BIN MISSING — cannot proceed"
        exit 1
    fi
done

# Confirm reference compiler works
log ""
log "Testing reference compiler (zcc) on zcc_pp.c..."
if "$WORKDIR/zcc" "$WORKDIR/zcc_pp.c" -o /tmp/zcc_ref_test.s > /dev/null 2>&1; then
    REF_LINES=$(wc -l < /tmp/zcc_ref_test.s)
    log "  ✓ zcc compiles zcc_pp.c → $REF_LINES lines of assembly"
else
    log "  ✗ Reference compiler ALSO fails — problem is not IR-specific"
    exit 1
fi

# Confirm zcc2 (self-hosted, non-IR) works
log ""
log "Testing zcc2 on zcc_pp.c..."
if "$WORKDIR/zcc2" "$WORKDIR/zcc_pp.c" -o /tmp/zcc2_test.s > /dev/null 2>&1; then
    ZCC2_LINES=$(wc -l < /tmp/zcc2_test.s)
    log "  ✓ zcc2 compiles zcc_pp.c → $ZCC2_LINES lines of assembly"
else
    log "  ✗ zcc2 ALSO fails — problem predates IR bridge"
    exit 1
fi

# Test zcc3_ir failure
log ""
log "Testing zcc3_ir on zcc_pp.c (expected to fail)..."
set +e
"$WORKDIR/zcc3_ir" "$WORKDIR/zcc_pp.c" -o /tmp/zcc3_ir_test.s > /tmp/zcc3_stdout.txt 2> /tmp/zcc3_stderr.txt
EXIT_CODE=$?
set -e
STDOUT_SIZE=$(stat -c%s /tmp/zcc3_stdout.txt 2>/dev/null || echo 0)
STDERR_SIZE=$(stat -c%s /tmp/zcc3_stderr.txt 2>/dev/null || echo 0)
log "  Exit code: $EXIT_CODE"
log "  stdout: $STDOUT_SIZE bytes"
log "  stderr: $STDERR_SIZE bytes"
if [ -s /tmp/zcc3_stdout.txt ]; then
    log "  stdout content: $(head -5 /tmp/zcc3_stdout.txt)"
fi
if [ -s /tmp/zcc3_stderr.txt ]; then
    log "  stderr content: $(head -5 /tmp/zcc3_stderr.txt)"
fi

# ───────────────────────────────────────────────────────────────────────
section 1 "MINIMAL INPUT PROBING — Narrowing the failure"
# ───────────────────────────────────────────────────────────────────────

# Test 1: Can zcc3_ir compile ANYTHING?
cat > /tmp/zcc_probe_trivial.c << 'TRIVIAL'
int main() { return 0; }
TRIVIAL

log "Probe 1: Trivial program (return 0)..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_trivial.c -o /tmp/zcc_probe_trivial.s > /dev/null 2>&1
RC=$?
set -e
if [ $RC -eq 0 ]; then
    log "  ✓ PASS — zcc3_ir can compile trivial C"
else
    log "  ✗ FAIL (exit $RC) — zcc3_ir CANNOT compile anything → CRITICAL"
    log "  Diagnosis: Binary itself is fundamentally broken (likely corrupted init)"
    log "  Next: Skip to Phase 3 (assembly diff of init_compiler)"
fi

# Test 2: Can it handle printf?
cat > /tmp/zcc_probe_printf.c << 'PRINTF'
int printf(const char *fmt, ...);
int main() { printf("hello\n"); return 0; }
PRINTF

log "Probe 2: printf call..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_printf.c -o /tmp/zcc_probe_printf.s > /dev/null 2>&1
RC=$?
set -e
log "  $([ $RC -eq 0 ] && echo '✓ PASS' || echo "✗ FAIL (exit $RC)")"

# Test 3: Can it handle fprintf + FILE*?
cat > /tmp/zcc_probe_fprintf.c << 'FPRINTF'
struct _IO_FILE;
typedef struct _IO_FILE FILE;
extern FILE *stderr;
int fprintf(FILE *stream, const char *fmt, ...);
int main() { fprintf(stderr, "test\n"); return 0; }
FPRINTF

log "Probe 3: fprintf + stderr..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_fprintf.c -o /tmp/zcc_probe_fprintf.s > /dev/null 2>&1
RC=$?
set -e
log "  $([ $RC -eq 0 ] && echo '✓ PASS' || echo "✗ FAIL (exit $RC)")"

# Test 4: Can it handle structs?
cat > /tmp/zcc_probe_struct.c << 'STRUCT'
struct Token { int kind; int line; char *str; int len; };
int main() {
    struct Token t;
    t.kind = 1;
    t.line = 42;
    return t.kind;
}
STRUCT

log "Probe 4: Struct access..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_struct.c -o /tmp/zcc_probe_struct.s > /dev/null 2>&1
RC=$?
set -e
log "  $([ $RC -eq 0 ] && echo '✓ PASS' || echo "✗ FAIL (exit $RC)")"

# Test 5: String literals and arrays
cat > /tmp/zcc_probe_strings.c << 'STRINGS'
int strcmp(const char *a, const char *b);
int main() {
    char *keywords[3];
    keywords[0] = "if";
    keywords[1] = "while";
    keywords[2] = "return";
    return strcmp(keywords[0], "if");
}
STRINGS

log "Probe 5: String arrays (keyword table pattern)..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_strings.c -o /tmp/zcc_probe_strings.s > /dev/null 2>&1
RC=$?
set -e
log "  $([ $RC -eq 0 ] && echo '✓ PASS' || echo "✗ FAIL (exit $RC)")"

# Test 6: Switch/case (heavy in lexer)
cat > /tmp/zcc_probe_switch.c << 'SWITCH'
int classify(int c) {
    switch (c) {
    case 43: return 1;
    case 45: return 2;
    case 42: return 3;
    case 47: return 4;
    default: return 0;
    }
}
int main() { return classify(43); }
SWITCH

log "Probe 6: Switch/case..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_switch.c -o /tmp/zcc_probe_switch.s > /dev/null 2>&1
RC=$?
set -e
log "  $([ $RC -eq 0 ] && echo '✓ PASS' || echo "✗ FAIL (exit $RC)")"

# Test 7: File I/O (fopen/fclose — used in ZCC's main)
cat > /tmp/zcc_probe_fio.c << 'FIO'
struct _IO_FILE;
typedef struct _IO_FILE FILE;
FILE *fopen(const char *path, const char *mode);
int fclose(FILE *f);
int fprintf(FILE *f, const char *fmt, ...);
int main() {
    FILE *f;
    f = fopen("/tmp/zcc_fio_test.txt", "w");
    if (f) {
        fprintf(f, "hello\n");
        fclose(f);
    }
    return 0;
}
FIO

log "Probe 7: File I/O (fopen/fprintf/fclose)..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_fio.c -o /tmp/zcc_probe_fio.s > /dev/null 2>&1
RC=$?
set -e
log "  $([ $RC -eq 0 ] && echo '✓ PASS' || echo "✗ FAIL (exit $RC)")"

# Test 8: Enum (used heavily in token types)
cat > /tmp/zcc_probe_enum.c << 'ENUM'
enum TokenKind { TK_NUM, TK_IDENT, TK_IF, TK_WHILE, TK_RETURN, TK_EOF };
int main() {
    enum TokenKind k;
    k = TK_IF;
    if (k == TK_IF) return 1;
    return 0;
}
ENUM

log "Probe 8: Enum..."
set +e
"$WORKDIR/zcc3_ir" /tmp/zcc_probe_enum.c -o /tmp/zcc_probe_enum.s > /dev/null 2>&1
RC=$?
set -e
log "  $([ $RC -eq 0 ] && echo '✓ PASS' || echo "✗ FAIL (exit $RC)")"

# ───────────────────────────────────────────────────────────────────────
section 2 "BINARY SEARCH ON zcc_pp.c — Finding the breaking line"
# ───────────────────────────────────────────────────────────────────────

TOTAL_LINES=$(wc -l < "$WORKDIR/zcc_pp.c")
log "zcc_pp.c has $TOTAL_LINES lines"
log ""
log "Binary search: finding the line threshold where zcc3_ir breaks..."

LO=1
HI=$TOTAL_LINES
LAST_PASS=0

while [ $LO -lt $HI ]; do
    MID=$(( (LO + HI) / 2 ))
    head -n $MID "$WORKDIR/zcc_pp.c" > /tmp/zcc_bisect.c

    # Need to close any open function — append a safety return+close
    # This is imprecise but finds the approximate break region
    set +e
    "$WORKDIR/zcc3_ir" /tmp/zcc_bisect.c -o /tmp/zcc_bisect.s > /dev/null 2>&1
    RC=$?
    set -e

    if [ $RC -eq 0 ]; then
        LAST_PASS=$MID
        LO=$(( MID + 1 ))
        log "  Line $MID: ✓ PASS"
    else
        HI=$MID
        log "  Line $MID: ✗ FAIL"
    fi

    # Safety valve — if we've narrowed to < 50 lines, stop
    if [ $(( HI - LO )) -lt 50 ]; then
        break
    fi
done

log ""
log "Breaking region: lines $LAST_PASS → $HI (approx ±50 lines)"

# Show the breaking region
if [ $LAST_PASS -gt 0 ] && [ $HI -le $TOTAL_LINES ]; then
    SHOW_START=$(( LAST_PASS > 10 ? LAST_PASS - 10 : 1 ))
    SHOW_END=$(( HI + 10 < TOTAL_LINES ? HI + 10 : TOTAL_LINES ))
    log ""
    log "Context around break point (lines $SHOW_START-$SHOW_END):"
    sed -n "${SHOW_START},${SHOW_END}p" "$WORKDIR/zcc_pp.c" | head -60 >> "$LOG"
fi

# ───────────────────────────────────────────────────────────────────────
section 3 "ASSEMBLY DIFF — Reference vs IR-compiled"
# ───────────────────────────────────────────────────────────────────────

# Compare assembly output of zcc vs zcc3_ir on the trivial program
# (if zcc3_ir can compile it)
if [ -f /tmp/zcc_probe_trivial.s ]; then
    "$WORKDIR/zcc" /tmp/zcc_probe_trivial.c -o /tmp/zcc_ref_trivial.s > /dev/null 2>&1
    
    log "Assembly diff (trivial program: zcc vs zcc3_ir):"
    if diff /tmp/zcc_ref_trivial.s /tmp/zcc_probe_trivial.s > /tmp/zcc_trivial_diff.txt 2>&1; then
        log "  ✓ IDENTICAL — trivial program compiles identically"
    else
        DIFF_LINES=$(wc -l < /tmp/zcc_trivial_diff.txt)
        log "  ✗ DIVERGENT — $DIFF_LINES diff lines"
        log "  First 30 diff lines:"
        head -30 /tmp/zcc_trivial_diff.txt >> "$LOG"
    fi
fi

# Compare assembly for the compiler source itself (what each stage emits)
# zcc compiling zcc_pp.c vs zcc2 compiling zcc_pp.c should be identical (bootstrap invariant)
log ""
log "Verifying bootstrap invariant: zcc output vs zcc2 output..."
"$WORKDIR/zcc" "$WORKDIR/zcc_pp.c" -o /tmp/zcc_stage1_asm.s > /dev/null 2>&1
"$WORKDIR/zcc2" "$WORKDIR/zcc_pp.c" -o /tmp/zcc_stage2_asm.s > /dev/null 2>&1

if diff /tmp/zcc_stage1_asm.s /tmp/zcc_stage2_asm.s > /dev/null 2>&1; then
    log "  ✓ zcc == zcc2 assembly output (bootstrap holds)"
else
    log "  ✗ BOOTSTRAP BROKEN — zcc != zcc2 output (problem predates IR)"
    diff /tmp/zcc_stage1_asm.s /tmp/zcc_stage2_asm.s | head -30 >> "$LOG"
fi

# ───────────────────────────────────────────────────────────────────────
section 4 "GDB DEEP PROBE — Tracing the silent death"
# ───────────────────────────────────────────────────────────────────────

log "Running zcc3_ir under GDB with breakpoints..."

# GDB script for systematic probing
cat > /tmp/zcc_gdb_script.txt << 'GDB'
set pagination off
set confirm off

# Break at main entry
break main
run

# Check argc/argv are sane
printf "argc = %d\n", argc
printf "argv[0] = %s\n", argv[0]
printf "argv[1] = %s\n", argv[1]

# Continue and see where it dies
catch signal SIGSEGV
catch signal SIGABRT
continue

# If we get here, it exited normally — check where
bt
quit
GDB

set +e
gdb -batch -x /tmp/zcc_gdb_script.txt \
    --args "$WORKDIR/zcc3_ir" "$WORKDIR/zcc_pp.c" -o /tmp/zcc_gdb_test.s \
    > /tmp/zcc_gdb_output.txt 2>&1
set -e

log "GDB output:"
cat /tmp/zcc_gdb_output.txt >> "$LOG"
log ""
log "GDB output (last 40 lines):"
tail -40 /tmp/zcc_gdb_output.txt | tee -a "$LOG"

# ───────────────────────────────────────────────────────────────────────
section 5 "DIAGNOSIS SUMMARY"
# ───────────────────────────────────────────────────────────────────────

log ""
log "Full log written to: $LOG"
log ""
log "🔱 Next steps based on results:"
log "  1. If Phase 1 probes ALL pass → problem is scale-dependent (large input)"
log "  2. If Phase 1 trivial FAILS → zcc3_ir binary is fundamentally broken"
log "  3. Phase 2 bisect gives the approximate line → check what function/construct"
log "  4. Phase 3 diff reveals codegen divergence → match against CG-001..CG-010"
log "  5. Phase 4 GDB shows the actual crash site or exit path"
log ""
log "🔱 ZCC DEBUG PROTOCOL COMPLETE"
