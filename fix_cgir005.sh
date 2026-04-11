#!/bin/bash
set -euo pipefail
cd /mnt/h/__DOWNLOADS/selforglinux

SRC="compiler_passes.c"
BACKUP="${SRC}.bak.cgir005.$(date +%s)"

echo "=== CG-IR-005 FIX — PHI Edge Copy (All PHIs) ==="

cp "$SRC" "$BACKUP"
echo "[0/6] Backed up -> $BACKUP"

echo "[1/6] Patching ir_asm_emit_phi_edge_copy..."

python3 - "$SRC" <<'PYEOF'
import sys, re
src_path = sys.argv[1]
with open(src_path, 'r') as f:
    code = f.read()
pattern = r'(static\s+void\s+ir_asm_emit_phi_edge_copy\s*\([^)]*\)\s*\{)'
m = re.search(pattern, code)
if not m:
    print("ERROR: Could not find ir_asm_emit_phi_edge_copy")
    sys.exit(1)
func_start = m.start()
sig_end = m.end()
depth = 1
pos = sig_end
while pos < len(code) and depth > 0:
    if code[pos] == '{': depth += 1
    elif code[pos] == '}': depth -= 1
    pos += 1
func_end = pos
old_func = code[func_start:func_end]
print(f"  Found function at offset {func_start}-{func_end} ({len(old_func)} chars)")
new_func = '''static void ir_asm_emit_phi_edge_copy(IRAsmCtx *ctx, BlockID from_id, BlockID to_id)
{
    Block *to_blk;
    Instr *phi_ins;
    int i;

    to_blk = &ctx->fn->blocks[to_id];
    phi_ins = to_blk->head;

    /* CG-IR-005 FIX: iterate ALL leading PHIs, not just the first.
       PHIs always cluster at block head in SSA form. */
    while (phi_ins && phi_ins->op == OP_PHI) {
        for (i = 0; i < phi_ins->n_phi_srcs; i++) {
            if (phi_ins->phi_srcs[i].block == from_id) {
                ir_asm_load_to_rax(ctx, phi_ins->phi_srcs[i].reg);
                ir_asm_store_rax_to(ctx, phi_ins->dst);
                break;
            }
        }
        phi_ins = phi_ins->next;
    }
}'''
code = code[:func_start] + new_func + code[func_end:]
with open(src_path, 'w') as f:
    f.write(code)
print("  Patch applied.")
PYEOF

if [ $? -ne 0 ]; then
    echo "PATCH FAILED - restoring backup"
    cp "$BACKUP" "$SRC"
    exit 1
fi

echo "[2/6] Verifying patch..."
grep -q 'CG-IR-005 FIX' "$SRC" && echo "  OK: marker found" || { echo "  FAIL"; cp "$BACKUP" "$SRC"; exit 1; }
grep -q 'while.*phi_ins.*OP_PHI' "$SRC" && echo "  OK: while-loop confirmed" || { echo "  FAIL"; cp "$BACKUP" "$SRC"; exit 1; }

echo "[3/6] Building zcc3_ir..."
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir.s 2>&1 | tail -5
gcc -O0 -w -o zcc3_ir zcc3_ir.s compiler_passes.c -lm
echo "  zcc3_ir built"

echo "[4/6] Testing arg parsing..."
echo 'int main(){return 0;}' > /tmp/cgir005_test.c
set +e
./zcc3_ir /tmp/cgir005_test.c -o /tmp/cgir005_test.s 2>/tmp/cgir005_err.log
RC=$?
set -e

if [ $RC -eq 0 ]; then
    echo "  PASS rc=0 — OPTIMIZED BUILD UNLOCKED"
elif [ $RC -eq 1 ]; then
    echo "  FAIL rc=1 — still broken"
    cat /tmp/cgir005_err.log 2>/dev/null
elif [ $RC -eq 139 ]; then
    echo "  rc=139 — separate unoptimized bug, not CG-IR-005"
else
    echo "  rc=$RC unexpected"
fi

echo "[5/6] Bootstrap verification..."
if [ $RC -eq 0 ]; then
    set +e
    ./zcc3_ir zcc_pp.c -o zcc4_ir.s 2>/dev/null
    RC2=$?
    set -e
    if [ $RC2 -eq 0 ] && diff -q zcc3_ir.s zcc4_ir.s >/dev/null 2>&1; then
        echo "  zcc3_ir.s == zcc4_ir.s — SELF-HOST VERIFIED"
    elif [ $RC2 -eq 0 ]; then
        echo "  MISMATCH — first diffs:"
        diff zcc3_ir.s zcc4_ir.s | head -30
    else
        echo "  zcc4_ir build failed rc=$RC2"
    fi
else
    echo "  Skipped — basic test failed"
fi

echo ""
echo "=== RESULTS ==="
echo "  Backup:  $BACKUP"
echo "  Test:    rc=$RC"
[ $RC -eq 0 ] && echo "  Next:    ASan build for the rc=139 unoptimized bug"
[ $RC -eq 1 ] && echo "  Next:    dump PHIs — grep -B2 -A5 'OP_PHI' zcc3_ir.s | head -60"
