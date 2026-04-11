#!/bin/bash
set -euo pipefail

# ============================================================
# CG-IR-005 FIX — PHI Edge Copy: All PHIs, Not Just First
# All-in-one: patch → rebuild → test → bootstrap verify
# Run from: /mnt/h/__DOWNLOADS/selforglinux
# ============================================================

DIR="$(cd "$(dirname "$0")" && pwd)"
cd /mnt/h/__DOWNLOADS/selforglinux 2>/dev/null || cd "$DIR"

SRC="compiler_passes.c"
BACKUP="${SRC}.bak.cgir005.$(date +%s)"

echo "═══════════════════════════════════════════════════"
echo "  CG-IR-005 FIX — PHI Edge Copy (All PHIs)        "
echo "═══════════════════════════════════════════════════"

# ── Step 0: Backup ──────────────────────────────────────
cp "$SRC" "$BACKUP"
echo "[0/6] Backed up → $BACKUP"

# ── Step 1: Find and patch ir_asm_emit_phi_edge_copy ────
echo "[1/6] Patching ir_asm_emit_phi_edge_copy..."

python3 - "$SRC" <<'PYEOF'
import sys, re

src_path = sys.argv[1]
with open(src_path, 'r') as f:
    code = f.read()

# Match the function — it's static void ir_asm_emit_phi_edge_copy(...)
# We need to find the entire function body and replace it.
# Strategy: find the function signature, then find the matching closing brace.

pattern = r'(static\s+void\s+ir_asm_emit_phi_edge_copy\s*\([^)]*\)\s*\{)'

m = re.search(pattern, code)
if not m:
    print("ERROR: Could not find ir_asm_emit_phi_edge_copy function signature")
    sys.exit(1)

func_start = m.start()
sig_end = m.end()

# Find matching closing brace
depth = 1
pos = sig_end
while pos < len(code) and depth > 0:
    if code[pos] == '{':
        depth += 1
    elif code[pos] == '}':
        depth -= 1
    pos += 1

func_end = pos  # position right after the closing '}'

old_func = code[func_start:func_end]
print(f"  Found function at offset {func_start}–{func_end} ({len(old_func)} chars)")

# Count how many PHIs the old version handled (sanity check)
if 'while' not in old_func and old_func.count('OP_PHI') <= 2:
    print("  Confirmed: old version only handles first PHI (no while loop)")
else:
    print("  WARNING: function may already be patched — check manually")
    # Continue anyway in case it's a partial fix

new_func = r'''static void ir_asm_emit_phi_edge_copy(IRAsmCtx *ctx, BlockID from_id, BlockID to_id)
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

print("  Patch applied successfully.")
PYEOF

if [ $? -ne 0 ]; then
    echo "PATCH FAILED — restoring backup"
    cp "$BACKUP" "$SRC"
    exit 1
fi

# ── Step 2: Verify patch is syntactically present ───────
echo "[2/6] Verifying patch..."
if grep -q 'CG-IR-005 FIX' "$SRC"; then
    echo "  ✅ CG-IR-005 comment marker found"
else
    echo "  ❌ Patch marker missing — restoring backup"
    cp "$BACKUP" "$SRC"
    exit 1
fi

if grep -q 'while.*phi_ins.*OP_PHI' "$SRC"; then
    echo "  ✅ while-loop over PHIs confirmed"
else
    echo "  ❌ while-loop not found — restoring backup"
    cp "$BACKUP" "$SRC"
    exit 1
fi

# ── Step 3: Rebuild with IR backend ─────────────────────
echo "[3/6] Building zcc3_ir (IR backend, optimized passes)..."

# Stage 1: gcc compiles zcc source → zcc1
if [ ! -f ./zcc2 ]; then
    echo "  zcc2 not found — running full bootstrap first"
    make clean && make selfhost
fi

# Stage IR: zcc2 compiles zcc_pp.c with IR backend → zcc3_ir
ZCC_IR_BACKEND=1 ./zcc2 zcc_pp.c -o zcc3_ir.s 2>&1 | tail -5
gcc -O0 -w -o zcc3_ir zcc3_ir.s compiler_passes.c -lm
echo "  ✅ zcc3_ir built"

# ── Step 4: Test — does the for-loop parse args now? ────
echo "[4/6] Testing arg parsing (the rc=1 bug)..."

echo 'int main(){return 0;}' > /tmp/cgir005_test.c
set +e
./zcc3_ir /tmp/cgir005_test.c -o /tmp/cgir005_test.s 2>/tmp/cgir005_err.log
RC=$?
set -e

if [ $RC -eq 0 ]; then
    echo "  ✅ PASS — zcc3_ir returned rc=0 (was rc=1 before fix)"
    echo "  ✅ OPTIMIZED BUILD UNLOCKED"
elif [ $RC -eq 139 ]; then
    echo "  ⚠️  rc=139 (segfault) — PHI fix didn't cause this, likely the unoptimized bug"
    echo "  Check: is this the noopt corruption (separate from CG-IR-005)?"
    cat /tmp/cgir005_err.log 2>/dev/null
elif [ $RC -eq 1 ]; then
    echo "  ❌ STILL rc=1 — PHI fix may be incomplete"
    echo "  Dumping for-loop PHIs from zcc3_ir.s for inspection:"
    grep -n 'phi_edge\|OP_PHI\|for\.head\|for\.body' zcc3_ir.s 2>/dev/null | head -20
    echo "  stderr:"
    cat /tmp/cgir005_err.log 2>/dev/null
else
    echo "  ⚠️  Unexpected rc=$RC"
    cat /tmp/cgir005_err.log 2>/dev/null
fi

# ── Step 5: Bootstrap verification (stage2 == stage3) ───
echo "[5/6] Bootstrap verification..."

if [ $RC -eq 0 ]; then
    echo "  Compiling zcc4_ir.s (self-compile through patched compiler)..."
    set +e
    ./zcc3_ir zcc_pp.c -o zcc4_ir.s 2>/tmp/cgir005_bootstrap_err.log
    RC2=$?
    set -e

    if [ $RC2 -eq 0 ]; then
        if diff -q zcc3_ir.s zcc4_ir.s > /dev/null 2>&1; then
            echo "  ✅ zcc3_ir.s == zcc4_ir.s — SELF-HOST VERIFIED"
        else
            DIFFLINES=$(diff zcc3_ir.s zcc4_ir.s | head -40)
            echo "  ❌ zcc3_ir.s != zcc4_ir.s — bootstrap mismatch"
            echo "  First differences:"
            echo "$DIFFLINES"
        fi
    else
        echo "  ❌ zcc4_ir build failed with rc=$RC2"
        cat /tmp/cgir005_bootstrap_err.log 2>/dev/null | tail -10
    fi
else
    echo "  ⏭️  Skipping bootstrap — basic test didn't pass"
fi

# ── Step 6: Summary ────────────────────────────────────
echo ""
echo "═══════════════════════════════════════════════════"
echo "  CG-IR-005 RESULTS"
echo "═══════════════════════════════════════════════════"
echo "  Backup:     $BACKUP"
echo "  Patch:      ir_asm_emit_phi_edge_copy → while-loop"
echo "  Basic test: rc=$RC (want 0)"
if [ $RC -eq 0 ]; then
    echo "  Bootstrap:  rc=$RC2"
    if diff -q zcc3_ir.s zcc4_ir.s > /dev/null 2>&1; then
        echo "  Self-host:  ✅ VERIFIED"
    else
        echo "  Self-host:  ❌ MISMATCH"
    fi
fi
echo "═══════════════════════════════════════════════════"
echo ""
echo "If rc=0 + self-host verified → CG-IR-005 is RESOLVED."
echo "If rc=1 still → dump PHI structure:"
echo "  grep -B2 -A5 'OP_PHI' zcc3_ir.s | head -60"
echo "If rc=139 → this is the separate unoptimized bug."
echo "  Next step: ASan build (use the asan_prep prompt on Sonnet)."
