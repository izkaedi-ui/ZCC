#!/usr/bin/env bash
# =============================================================================
# diagnose_stage3.sh — Stage 3 crash narrowing
# =============================================================================
# Tests five hypotheses about why zcc_ir2 still crashes.
# =============================================================================

WORKDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$WORKDIR" || exit 1

ZCC_PP="$WORKDIR/zcc_pp.c"
CP_SRC="$WORKDIR/compiler_passes.c"
OUT="$WORKDIR/diag3_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$OUT"

echo "================================================================"
echo "  diagnose_stage3.sh — $(date)"
echo "  Output: $OUT"
echo "================================================================"

# Ensure zcc_ir2 exists
if [[ ! -f zcc_ir2 ]]; then
    echo "Regenerating zcc_ir2.s and zcc_ir2..."
    ZCC_IR_BACKEND=1 ./zcc "$ZCC_PP" -o zcc_ir2.s 2>/dev/null
    gcc -O0 -w -o zcc_ir2 zcc_ir2.s "$CP_SRC" -lm 2>/dev/null
fi
echo "  zcc_ir2 size: $(stat -c%s zcc_ir2 2>/dev/null || echo '?') bytes"
echo ""

# ---------------------------------------------------------------------------
# TEST A: Does zcc_ir2 crash WITHOUT ZCC_IR_BACKEND?
# (Tests whether the IR-compiled binary itself works for AST-mode compilation)
# ---------------------------------------------------------------------------
echo "[ TEST A ] zcc_ir2 compiling zcc_pp.c WITHOUT ZCC_IR_BACKEND..."
echo 'int main(void){return 0;}' > "$OUT/trivial.c"

./zcc_ir2 "$OUT/trivial.c" -o "$OUT/trivial.s" 2>"$OUT/trivial_err.txt"
TA_TRIVIAL=$?

./zcc_ir2 "$ZCC_PP" -o "$OUT/ast_stage3.s" 2>"$OUT/ast_err.txt"
TA_FULL=$?

echo "  trivial.c  rc=$TA_TRIVIAL"
echo "  zcc_pp.c   rc=$TA_FULL"
if [[ $TA_FULL -eq 0 ]]; then
    echo "  → AST-mode works. Crash is SPECIFIC to ZCC_IR_BACKEND=1 in zcc_ir2."
else
    echo "  → AST-mode ALSO crashes. The IR binary itself is broken."
fi

# ---------------------------------------------------------------------------
# TEST B: GDB backtrace of the crashing case (without ASan)
# ---------------------------------------------------------------------------
echo ""
echo "[ TEST B ] GDB backtrace of ZCC_IR_BACKEND=1 ./zcc_ir2 zcc_pp.c crash..."

if command -v gdb &>/dev/null; then
    cat > "$OUT/gdb_stage3.gdb" << 'GDBEOF'
set pagination off
set confirm off
set print thread-events off
handle SIGSEGV stop print nopass
run zcc_pp.c -o /dev/null 2>/dev/null
bt 30
info registers rip rbp rsp rbx rax rcx rdx rdi rsi
x/10i $rip-20
quit
GDBEOF
    ZCC_IR_BACKEND=1 gdb -batch -x "$OUT/gdb_stage3.gdb" ./zcc_ir2 2>&1 \
        | grep -v "^Reading\|^No debug\|^BFD\|^warn\|^Using\|^This\|^For help\|^Type\|^Loaded\|^(gdb)\|^0x0\b" \
        | tee "$OUT/gdb_bt.txt"
else
    echo "  GDB not found"
fi

# ---------------------------------------------------------------------------
# TEST C: strace to find exact syscall at crash
# ---------------------------------------------------------------------------
echo ""
echo "[ TEST C ] strace (last 20 syscalls before crash)..."
if command -v strace &>/dev/null; then
    ZCC_IR_BACKEND=1 strace -e trace=all -o "$OUT/strace.txt" \
        ./zcc_ir2 "$ZCC_PP" -o /dev/null 2>/dev/null
    STC=$?
    echo "  Exit: $STC"
    echo "  Last 20 syscalls:"
    tail -20 "$OUT/strace.txt"
else
    echo "  strace not found"
fi

# ---------------------------------------------------------------------------
# TEST D: Check if OP_RET inline restores are present in zcc_ir2.s
#         for a known complex function
# ---------------------------------------------------------------------------
echo ""
echo "[ TEST D ] OP_RET inline restore presence check in zcc_ir2.s..."

python3 - << 'PYEOF' 2>&1 | tee "$OUT/op_ret_check.txt"
import re

text = open('zcc_ir2.s').read()

# Split into functions (preceded by pushq %rbp pattern)
# More robust: find lines that look like function bodies
# by looking for pushq %rbp immediately after a label

func_pattern = re.compile(
    r'\n([A-Za-z_][A-Za-z0-9_]*):\n'   # label
    r'    pushq %rbp\n'                  # function prologue
)

results = []
for m in func_pattern.finditer(text):
    name = m.group(1)
    start = m.start()
    # Find the next function to delimit body
    next_m = func_pattern.search(text, m.end())
    end = next_m.start() if next_m else len(text)
    body = text[start:end]

    uses_rbx = '%rbx' in body
    if not uses_rbx:
        continue

    has_entry_save  = 'movq %rbx, ' in body and re.search(r'movq %rbx, -\d+\(%rbp\)', body)
    has_ret_restore = bool(re.search(
        r'movq\s+-\d+\(%rbp\),\s*%rbx\n\s+movq\s+-\d+\(%rbp\),\s*%r12\n.*?jmp\s+\.Lfunc_end_',
        body, re.DOTALL
    ))
    has_jmp_end = '.Lfunc_end_' in body
    n_ret = len(re.findall(r'jmp\s+\.Lfunc_end_', body))

    results.append({
        'name': name,
        'entry_save': bool(has_entry_save),
        'ret_restore': has_ret_restore,
        'n_ret': n_ret,
    })

total = len(results)
with_entry  = sum(1 for r in results if r['entry_save'])
with_inline = sum(1 for r in results if r['ret_restore'])
missing     = [r for r in results if r['entry_save'] and not r['ret_restore'] and r['n_ret'] > 0]

print(f"Functions using %rbx: {total}")
print(f"  With entry save    : {with_entry}")
print(f"  With OP_RET restore: {with_inline}")
print(f"  Has save but NO OP_RET restore (has jmp .Lfunc_end): {len(missing)}")
for r in missing[:10]:
    print(f"    MISSING INLINE RESTORE: {r['name']} ({r['n_ret']} ret paths)")
PYEOF

# ---------------------------------------------------------------------------
# TEST E: Specific function body dump for top violation
# ---------------------------------------------------------------------------
echo ""
echo "[ TEST E ] Dump first function with missing inline restore..."

python3 - << 'PYEOF' 2>&1 | tee "$OUT/missing_func_body.txt"
import re

text = open('zcc_ir2.s').read()
func_pattern = re.compile(r'\n([A-Za-z_][A-Za-z0-9_]*):\n    pushq %rbp\n')

for m in func_pattern.finditer(text):
    name = m.group(1)
    next_m = func_pattern.search(text, m.end())
    end = next_m.start() if next_m else len(text)
    body = text[m.start():end]

    uses_rbx = '%rbx' in body
    has_entry_save = bool(re.search(r'movq %rbx, -\d+\(%rbp\)', body))
    has_ret_restore = bool(re.search(
        r'movq\s+-\d+\(%rbp\),\s*%rbx\n\s+movq\s+-\d+\(%rbp\),\s*%r12\n.*?jmp\s+\.Lfunc_end_',
        body, re.DOTALL
    ))
    has_jmp = '.Lfunc_end_' in body
    n_ret = len(re.findall(r'jmp\s+\.Lfunc_end_', body))

    if uses_rbx and has_entry_save and not has_ret_restore and n_ret > 0:
        print(f"=== FUNCTION: {name} (n_ret_paths={n_ret}) ===")
        print(body[:3000])
        break
else:
    print("No missing-restore functions found!")
    print("Either all functions have inline restores, OR")
    print("the OP_RET inline restore format doesn't match the search pattern.")
    # Alternative: just show first function with %rbx and OP_RET
    for m in func_pattern.finditer(text):
        name = m.group(1)
        next_m = func_pattern.search(text, m.end())
        end = next_m.start() if next_m else len(text)
        body = text[m.start():end]
        if '%rbx' in body and '.Lfunc_end_' in body:
            print(f"\n=== FIRST FUNCTION WITH %rbx + OP_RET (for reference): {name} ===")
            print(body[:2000])
            break
PYEOF

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
echo ""
echo "================================================================"
echo "  RESULTS SUMMARY"
echo "================================================================"
echo ""
echo "  AST-mode (no ZCC_IR_BACKEND): trivial=$TA_TRIVIAL  full=$TA_FULL"
echo ""
echo "  Key files:"
for f in gdb_bt.txt op_ret_check.txt missing_func_body.txt; do
    [[ -f "$OUT/$f" ]] && echo "    $OUT/$f ($(wc -l < "$OUT/$f") lines)"
done
echo ""
echo "  IF Test A (AST-mode) WORKS:"
echo "    → IR binary is fine; crash is in ZCC_IR_BACKEND recursive use"
echo "    → Look at gdb_bt.txt for call site"
echo "  IF Test A ALSO CRASHES:"
echo "    → Fundamental IR codegen bug in a core function"
echo "    → Look at op_ret_check.txt for missing inline restores"
