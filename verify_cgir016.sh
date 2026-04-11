#!/usr/bin/env bash
# =============================================================================
# verify_cgir016.sh — CG-IR-016 Verification Suite  (v2)
# =============================================================================
# v2 fixes vs v1:
#  - Restore grep: 'movq .*, %rbx$' not 'movq .*,%rbx$' (space before %rbx)
#  - safe_count() replaces "grep -c || echo 0" (avoids double-0 output)
#  - Alignment check: only flags subq > 8 (call-site subq $8 is legal)
#  - Violation check: accepts pushq %rbx (AST path) as a valid save
#  - Reports fix_csave_formula.py sentinel separately (formula fix)
# =============================================================================

set -o pipefail
WORKDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$WORKDIR" || exit 1

ZCC_SRC="$WORKDIR/zcc.c"
CP_SRC="$WORKDIR/compiler_passes.c"
ZCC_PP="$WORKDIR/zcc_pp.c"
FIX_SCRIPT="$WORKDIR/fix_cgir016.py"
FIX_FMLA="$WORKDIR/fix_csave_formula.py"
ZCC="$WORKDIR/zcc"
ZCC2="$WORKDIR/zcc2"

APPLY_FIX=1
RUN_REGRESSION=1
for arg in "$@"; do
    case "$arg" in --no-apply) APPLY_FIX=0 ;; --no-regression) RUN_REGRESSION=0 ;; esac
done

PASS=0; FAIL=0
pass() { echo "  ✅ PASS: $1"; PASS=$((PASS+1)); }
fail() { echo "  ❌ FAIL: $1"; FAIL=$((FAIL+1)); }
info() { echo "  ℹ  INFO: $1"; }

# grep -c exits 1 on 0 matches, which would abort with set -e;  this is safe
safe_count() { grep -c "$1" "$2" 2>/dev/null || true; }

echo "=================================================================="
echo "  verify_cgir016.sh v2 — $(date)"
echo "=================================================================="

# --- Step 0: apply fixes ---------------------------------------------------
if [[ "$APPLY_FIX" -eq 1 ]]; then
  echo ""
  echo "[ STEP 0 ] Applying patches..."
  if [[ -f "$FIX_SCRIPT" ]]; then
    python3 "$FIX_SCRIPT" "$CP_SRC" && pass "fix_cgir016.py OK" || fail "fix_cgir016.py failed"
  else
    fail "fix_cgir016.py not found"
  fi
  if [[ -f "$FIX_FMLA" ]]; then
    python3 "$FIX_FMLA" "$CP_SRC" && pass "fix_csave_formula.py OK" || fail "fix_csave_formula.py failed"
  else
    info "fix_csave_formula.py not found — skipping"
  fi
  if grep -q "csave_offset = -(stack_size + 8 \* ((int)max_reg + n_alloca + 8))" "$CP_SRC" 2>/dev/null; then
    pass "csave_base formula: correct"
  else
    fail "csave_base formula: WRONG — -(stack_size+ir_extra-8) contaminates alloca region"
    grep -n "csave_offset" "$CP_SRC" | head -3
  fi
else
  info "Skipping fix application"
fi

# --- Step 1: build ---------------------------------------------------------
echo ""
echo "[ STEP 1 ] GCC build..."
gcc -O0 -w -o "$ZCC" "$ZCC_SRC" "$CP_SRC" -lm 2>&1
[[ -f "$ZCC" ]] && pass "Build OK" || { fail "Build FAILED"; exit 1; }

# --- Step 2: AST self-host -------------------------------------------------
echo ""
echo "[ STEP 2 ] AST self-host..."
"$ZCC" "$ZCC_PP" -o zcc2.s 2>/dev/null
gcc -O0 -w -o "$ZCC2" zcc2.s "$CP_SRC" -lm 2>/dev/null
"$ZCC2" "$ZCC_PP" -o zcc3.s 2>/dev/null
if cmp -s zcc2.s zcc3.s 2>/dev/null; then
  pass "zcc2.s == zcc3.s ✓"
else
  fail "zcc2.s != zcc3.s"
  [[ -f zcc2.s && -f zcc3.s ]] && diff zcc2.s zcc3.s | head -20
fi

# --- Step 3: IR self-host --------------------------------------------------
echo ""
echo "[ STEP 3 ] IR self-host..."
ZCC_IR_BACKEND=1 "$ZCC" "$ZCC_PP" -o zcc_ir2.s 2>/dev/null; IR2=$?
gcc -O0 -w -o zcc_ir2 zcc_ir2.s "$CP_SRC" -lm 2>/dev/null
ZCC_IR_BACKEND=1 ./zcc_ir2 "$ZCC_PP" -o zcc_ir3.s 2>/dev/null; IR3=$?
if [[ $IR2 -ne 0 ]]; then fail "IR stage2 rc=$IR2"
elif [[ $IR3 -ne 0 ]]; then fail "IR stage3 rc=$IR3 (rc=139=SIGSEGV)"; info "Run run_asan.sh for diagnosis"
elif cmp -s zcc_ir2.s zcc_ir3.s 2>/dev/null; then pass "zcc_ir2.s == zcc_ir3.s ✓"
else
  fail "zcc_ir2.s != zcc_ir3.s"
  diff zcc_ir2.s zcc_ir3.s | head -20
fi

# --- Step 4: register audit ------------------------------------------------
echo ""
echo "[ STEP 4 ] Callee-save register audit..."
if [[ -f zcc_ir2.s ]]; then
  RBX_USES=$(safe_count '%rbx'          zcc_ir2.s)
  RBX_IR_SAVE=$(safe_count 'movq %rbx, ' zcc_ir2.s)
  # restores: "movq N(%rbp), %rbx" — note the SPACE before %rbx
  RBX_IR_REST=$(safe_count ', %rbx$'     zcc_ir2.s)
  RBX_AST_SAVE=$(safe_count 'pushq %rbx' zcc_ir2.s)
  RBX_AST_REST=$(safe_count 'popq %rbx'  zcc_ir2.s)
  R12=$(safe_count 'movq %r12, ' zcc_ir2.s)
  R13=$(safe_count 'movq %r13, ' zcc_ir2.s)
  R14=$(safe_count 'movq %r14, ' zcc_ir2.s)
  R15=$(safe_count 'movq %r15, ' zcc_ir2.s)
  echo ""
  echo "  %rbx total uses    : $RBX_USES"
  echo "  IR  saves  (movq)  : $RBX_IR_SAVE     restores: $RBX_IR_REST"
  echo "  AST saves (pushq)  : $RBX_AST_SAVE    restores: $RBX_AST_REST"
  echo "  r12/r13/r14/r15    : $R12 / $R13 / $R14 / $R15 saves"
  TOTAL_SAVES=$((RBX_IR_SAVE + RBX_AST_SAVE))
  [[ $TOTAL_SAVES -gt 0 ]]          && pass "rbx protected ($RBX_IR_SAVE IR + $RBX_AST_SAVE AST saves)" || fail "No %rbx saves found"
  [[ $RBX_IR_SAVE -eq $RBX_IR_REST ]] && pass "IR rbx save/restore balanced: $RBX_IR_SAVE" || fail "IR rbx imbalanced: $RBX_IR_SAVE saves vs $RBX_IR_REST restores"
  echo ""
  echo "  Per-function violation scan (rbx used without any save)..."
  VIO=$(python3 - <<'PYEOF' 2>/dev/null
import re, sys
text = open('zcc_ir2.s').read()
parts = re.split(r'^([A-Za-z_][A-Za-z0-9_]*):\n', text, flags=re.MULTILINE)
v = []
for i in range(1, len(parts), 2):
    name = parts[i]; body = parts[i+1] if i+1<len(parts) else ''
    if '%rbx' in body and 'movq %rbx, ' not in body and 'pushq %rbx' not in body:
        v.append(name)
if v:
    for n in v[:10]: print(f'  VIOLATION: {n}')
    if len(v)>10: print(f'  ...and {len(v)-10} more')
    print(len(v))
else:
    print(0)
PYEOF
)
  VIO_COUNT=$(echo "$VIO" | tail -1)
  echo "$VIO" | head -n -1
  [[ "${VIO_COUNT:-1}" -eq 0 ]] && pass "No rbx violations" || fail "$VIO_COUNT violation(s)"
fi

# --- Step 5: IR frame alignment (only >8 byte subq) ----------------------
echo ""
echo "[ STEP 5 ] IR frame alignment (subq > 8)..."
if [[ -f zcc_ir2.s ]]; then
  BAD=$(python3 - <<'PYEOF' 2>/dev/null
import re
hits = re.findall(r'subq \$(\d+), %rsp', open('zcc_ir2.s').read())
bad = [int(h) for h in hits if int(h) > 8 and int(h) % 16 != 0]
print('\n'.join(f'  MISALIGNED: subq ${b}, %rsp' for b in bad[:5]))
print(len(bad))
PYEOF
)
  CNT=$(echo "$BAD" | tail -1)
  echo "$BAD" | head -n -1
  [[ "${CNT:-0}" -eq 0 ]] && pass "All IR frame extensions are 16-byte aligned" || fail "$CNT misaligned extensions"
fi

# --- Step 6: sentinel checks -----------------------------------------------
echo ""
echo "[ STEP 6 ] Sentinel checks..."
grep -q "CG-IR-016-CSAVE-V2"      "$CP_SRC" && pass "CG-IR-016-CSAVE-V2 applied"    || fail "V2 patch missing"
grep -q "CG-IR-016-CSAVE-V2-FMLA" "$CP_SRC" && pass "CG-IR-016-CSAVE-V2-FMLA applied" || fail "Formula fix missing — run fix_csave_formula.py"
grep -q "n_csave_slots"            "$CP_SRC" && pass "Explicit n_csave_slots present" || fail "n_csave_slots not found"

# --- Step 7: regression ----------------------------------------------------
if [[ "$RUN_REGRESSION" -eq 1 && -f "$WORKDIR/run_tests.sh" ]]; then
  echo ""
  echo "[ STEP 7 ] Regression suite..."
  bash "$WORKDIR/run_tests.sh" 2>&1 | tail -5
  [[ ${PIPESTATUS[0]} -eq 0 ]] && pass "Regression PASS" || fail "Regression FAIL"
fi

# --- Final -----------------------------------------------------------------
echo ""
echo "=================================================================="
echo "  PASSED: $PASS  |  FAILED: $FAIL"
if [[ "$FAIL" -eq 0 ]]; then
  echo "  🔱 ALL CHECKS PASSED — CG-IR-016 VERIFIED"
else
  echo "  ⚠  $FAIL FAILED — do not ship"
  echo "     IR self-host still crashing? → run_asan.sh"
fi
echo "=================================================================="
exit "$FAIL"
