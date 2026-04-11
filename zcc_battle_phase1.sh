#!/bin/sh
# ═══════════════════════════════════════════════════════════════
# PHASE 1: Diagnose current state — DO NOT MODIFY ANY FILES
# ═══════════════════════════════════════════════════════════════
cd /mnt/h/__DOWNLOADS/selforglinux

echo "═══════════════════════════════════════════════"
echo "  PHASE 1: DIAGNOSIS"
echo "═══════════════════════════════════════════════"

# 1A. Verify baseline selfhost is clean
echo ""
echo "── 1A: Baseline selfhost ──"
make clean 2>/dev/null
timeout 120 make selfhost 2>/dev/null
RC=$?
if [ "$RC" = "0" ]; then
    echo "Baseline: SELF-HOST VERIFIED"
else
    echo "Baseline: FAILED (rc=$RC) — FIX THIS FIRST"
    exit 1
fi

# 1B. Check current whitelist
echo ""
echo "── 1B: Current IR whitelist ──"
awk '/ir_whitelisted/,/^}/' part4.c | head -30

# 1C. Check expression statement fix is present
echo ""
echo "── 1C: Expression statement fix ──"
awk '/void zcc_lower_stmt/,/^}/' compiler_passes.c | grep -B1 -A3 'default:' | head -15

# 1D. Identify the OP_LOAD lowering code
echo ""
echo "── 1D: OP_LOAD handler ──"
awk '/case OP_LOAD:/,/break;/' compiler_passes.c | head -25

# 1E. Show is_alpha from AST backend (correct — has movslq)
echo ""
echo "── 1E: is_alpha AST (correct reference) ──"
sed -n '/^is_alpha:/,/^[a-zA-Z_][a-zA-Z0-9_]*:/p' zcc2.s | head -15

# 1F. Find ALL movq-from-memory in the IR lowerer
echo ""
echo "── 1F: All movq dereferences in lowerer ──"
grep -n 'movq.*(%%\|movq.*(%' compiler_passes.c | grep -v 'movq.*%%r\|store\|STORE\|movq %%' | head -20

# 1G. How does the LOAD instruction carry width?
echo ""
echo "── 1G: LOAD width info ──"
grep -n 'ins->imm\|ins->type\|load.*width\|OP_LOAD' compiler_passes.c | head -20

# 1H. Check ir_asm_load_to_rax
echo ""
echo "── 1H: ir_asm_load_to_rax function ──"
awk '/ir_asm_load_to_rax/,/^}/' compiler_passes.c | head -25

# 1I. Golden tests
echo ""
echo "── 1I: Golden tests ──"
./zcc matrix_host.c -o /tmp/mx.s 2>/dev/null && gcc -o /tmp/mx /tmp/mx.s && /tmp/mx

echo ""
echo "═══════════════════════════════════════════════"
echo "  PHASE 1 COMPLETE — Review output above"
echo "  Key items:"
echo "    1D: Shows exactly how OP_LOAD emits movq"
echo "    1F: Shows all bare movq dereferences to patch"
echo "    1G: Shows how width info is carried (ins->imm?)"
echo "    1H: Shows if ir_asm_load_to_rax needs patching"
echo "═══════════════════════════════════════════════"
