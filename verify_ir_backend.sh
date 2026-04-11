#!/bin/bash
# ZCC IR Backend Verification — Tests semantic equivalence between
# AST-direct codegen (part4.c) and the IR lowering backend.
#
# Success = the compiler built by the IR backend produces identical
# assembly to the compiler built by the AST backend.
#
# Usage: bash verify_ir_backend.sh
set -e

RED='\033[0;31m'
GRN='\033[0;32m'
CYN='\033[0;36m'
RST='\033[0m'

step() { echo -e "${CYN}=== $1 ===${RST}"; }
pass() { echo -e "${GRN}[PASS] $1${RST}"; }
fail() { echo -e "${RED}[FAIL] $1${RST}"; exit 1; }

# ---------- Prepare concatenated source ----------
step "Preparing zcc_pp.c (concatenated source)"
cat part1.c part2.c part3.c ir.h ir_emit_dispatch.h ir_bridge.h part4.c part5.c ir.c ir_to_x86.c > zcc_pp.c
echo "  $(wc -l < zcc_pp.c) lines"

# ---------- Stage 1: Bootstrap zcc (host GCC) ----------
step "Stage 1: GCC bootstrap → zcc_host"
gcc -O0 -w -o zcc_host zcc_pp.c
pass "zcc_host built"

# ---------- Stage 2: AST backend self-host ----------
step "Stage 2: zcc_host compiles itself (AST backend) → zcc2_ast"
./zcc_host zcc_pp.c -o zcc2_ast.s
gcc -O0 -w -o zcc2_ast zcc2_ast.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || \
gcc -O0 -w -o zcc2_ast zcc2_ast.s
pass "zcc2_ast built ($(wc -l < zcc2_ast.s) asm lines)"

# ---------- Stage 3: AST backend reference ----------
step "Stage 3: zcc2_ast compiles source (AST backend) → reference.s"
./zcc2_ast zcc_pp.c -o reference.s
pass "reference.s generated ($(wc -l < reference.s) asm lines)"

# ---------- Stage 4: IR backend compilation ----------
step "Stage 4: zcc2_ast compiles source (IR backend) → zcc3_ir.s"
ZCC_IR_BACKEND=1 ./zcc2_ast zcc_pp.c -o zcc3_ir.s
pass "zcc3_ir.s generated ($(wc -l < zcc3_ir.s) asm lines)"

# ---------- Stage 5: Build the IR-backend-compiled compiler ----------
step "Stage 5: Link zcc3_ir.s → zcc3_ir binary"
gcc -O0 -w -o zcc3_ir zcc3_ir.s compiler_passes.c compiler_passes_ir.c -lm 2>/dev/null || \
gcc -O0 -w -o zcc3_ir zcc3_ir.s
pass "zcc3_ir linked"

# ---------- Stage 6: Does the IR-built compiler produce correct output? ----------
step "Stage 6: zcc3_ir compiles source (AST backend) → check.s"
./zcc3_ir zcc_pp.c -o check.s
pass "check.s generated ($(wc -l < check.s) asm lines)"

# ---------- Stage 7: The Verdict ----------
step "Stage 7: SEMANTIC EQUIVALENCE CHECK"
echo "  reference.s = AST-backend-compiled compiler's output"
echo "  check.s     = IR-backend-compiled compiler's output"
echo ""
if cmp -s reference.s check.s; then
    pass "reference.s == check.s — IR BACKEND SEMANTICALLY EQUIVALENT"
    echo ""
    echo -e "${GRN}╔══════════════════════════════════════════════════════════════╗${RST}"
    echo -e "${GRN}║  IR BACKEND VERIFICATION: PASSED                           ║${RST}"
    echo -e "${GRN}║  The IR lowering backend produces a compiler that behaves  ║${RST}"
    echo -e "${GRN}║  identically to the AST direct backend.                    ║${RST}"
    echo -e "${GRN}╚══════════════════════════════════════════════════════════════╝${RST}"
else
    fail "reference.s != check.s — MISMATCH DETECTED"
    echo ""
    echo "First difference:"
    diff reference.s check.s | head -30
    echo ""
    echo "To find the divergent function:"
    echo "  diff reference.s check.s | grep 'cc_func\\|^[<>].*\\.globl'"
fi

# ---------- Bonus: Assembly size comparison ----------
step "Bonus: Assembly size comparison"
AST_LINES=$(wc -l < reference.s)
IR_LINES=$(wc -l < zcc3_ir.s)
echo "  AST backend output:  $AST_LINES lines (reference.s)"
echo "  IR backend output:   $IR_LINES lines (zcc3_ir.s)"
if [ "$IR_LINES" -lt "$AST_LINES" ]; then
    DIFF=$((AST_LINES - IR_LINES))
    echo -e "  ${GRN}IR backend is $DIFF lines smaller${RST}"
elif [ "$IR_LINES" -gt "$AST_LINES" ]; then
    DIFF=$((IR_LINES - AST_LINES))
    echo -e "  ${CYN}IR backend is $DIFF lines larger (r10/r11 save/restore overhead)${RST}"
else
    echo "  Identical line counts"
fi
