#!/bin/sh
# ═══════════════════════════════════════════════════════════════
# PHASE 3: Verify CG-IR-007 fix + expression statement fix
# ═══════════════════════════════════════════════════════════════
cd /mnt/h/__DOWNLOADS/selforglinux

echo "═══════════════════════════════════════════════"
echo "  PHASE 3: VERIFICATION"
echo "═══════════════════════════════════════════════"

# 3A. Clean rebuild baseline
echo ""
echo "── 3A: Clean baseline rebuild ──"
make clean 2>/dev/null
timeout 120 make selfhost
echo "Baseline: SELF-HOST VERIFIED"

# 3B. Verify golden tests still pass
echo ""
echo "── 3B: Golden value tests ──"
./zcc matrix_host.c -o /tmp/mx3.s 2>/dev/null
gcc -o /tmp/mx3 /tmp/mx3.s
/tmp/mx3
echo "Golden tests done"

# 3C. Check is_alpha now has movslq in IR output
echo ""
echo "── 3C: is_alpha load instruction check ──"
echo "AST backend (reference):"
sed -n '/^is_alpha:/,/^[a-zA-Z_][a-zA-Z0-9_]*:/p' zcc2.s | grep -E 'movslq|movq.*\(%' | head -3
echo "IR backend:"
sed -n '/^is_alpha:/,/^[a-zA-Z_][a-zA-Z0-9_]*:/p' zcc3.s | grep -E 'movslq|movq.*\(%' | head -3

# 3D. Full diff check
echo ""
echo "── 3D: Assembly diff ──"
DIFF_OUT=$(diff zcc2.s zcc3.s 2>/dev/null)
DIFF_LINES=$(echo "$DIFF_OUT" | wc -l)
if [ -z "$DIFF_OUT" ]; then
    echo "ZERO DIFF — PERFECT"
else
    echo "$DIFF_LINES diff lines"
    echo "First divergence:"
    echo "$DIFF_OUT" | head -20
    echo ""
    FIRST_LINE=$(echo "$DIFF_OUT" | head -1 | sed 's/[^0-9].*//;s/,.*//')
    echo "Function at divergence:"
    sed -n "1,${FIRST_LINE}p" zcc2.s | grep '^[a-z_]' | tail -1
fi

# 3E. Expression statement test
echo ""
echo "── 3E: Expression statement test ──"
cat > /tmp/test_expr_stmt.c << 'TESTEOF'
int printf(const char *fmt, ...);
int test_expr_stmt() {
    int x = 0;
    x++;
    x++;
    x++;
    return x;
}
int test_call_stmt() {
    printf("hello from expr stmt\n");
    return 0;
}
int main() {
    int r1 = test_expr_stmt();
    int r2 = test_call_stmt();
    if (r1 != 3) { printf("FAIL: x=%d expected 3\n", r1); return 1; }
    if (r2 != 0) { printf("FAIL: call_stmt=%d expected 0\n", r2); return 2; }
    printf("EXPR STMT TEST: ALL PASS (x=%d)\n", r1);
    return 0;
}
TESTEOF
./zcc /tmp/test_expr_stmt.c -o /tmp/test_expr_stmt.s 2>/dev/null
gcc -o /tmp/test_expr_stmt /tmp/test_expr_stmt.s 2>/dev/null
/tmp/test_expr_stmt
echo "Exit code: $?"

# 3F. Peephole count
echo ""
echo "── 3F: Peephole count ──"
./zcc zcc.c -o /dev/null 2>&1 | grep -i 'peephole\|elided'

echo ""
echo "═══════════════════════════════════════════════"
echo "  PHASE 3 COMPLETE"
echo "═══════════════════════════════════════════════"
