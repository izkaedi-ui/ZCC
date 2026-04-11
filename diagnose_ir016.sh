#!/usr/bin/env bash
# =============================================================================
# diagnose_ir016.sh — CG-IR-016 post-fix crash diagnostic
# =============================================================================
# Pinpoints why zcc_ir2 still segfaults after CG-IR-016 formula fix.
# Answers four questions:
#   Q1. Is "next_offset_from_rbp" a real function or a goto label?
#   Q2. Does it have the expected movq %rbx save?
#   Q3. What is the ACTUAL crash site (ASan / GDB)?
#   Q4. Are there any other functions with %rbx but no save?
# =============================================================================

WORKDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$WORKDIR" || exit 1

ZCC_PP="$WORKDIR/zcc_pp.c"
CP_SRC="$WORKDIR/compiler_passes.c"
ZCC_SRC="$WORKDIR/zcc.c"
OUT="$WORKDIR/diag_$(date +%Y%m%d_%H%M%S)"
mkdir -p "$OUT"

echo "================================================================"
echo "  diagnose_ir016.sh — $(date)"
echo "  Output dir: $OUT"
echo "================================================================"

# ---------------------------------------------------------------------------
# Q1 + Q2: Inspect next_offset_from_rbp in zcc_ir2.s
# ---------------------------------------------------------------------------
echo ""
echo "[ Q1 + Q2 ] next_offset_from_rbp in zcc_ir2.s..."

if [[ ! -f zcc_ir2.s ]]; then
    echo "  zcc_ir2.s not found — regenerating..."
    ZCC_IR_BACKEND=1 ./zcc "$ZCC_PP" -o zcc_ir2.s 2>/dev/null
fi

python3 - << 'PYEOF' 2>&1 | tee "$OUT/next_offset_from_rbp.txt"
import re, sys

text = open('zcc_ir2.s').read()
label = 'next_offset_from_rbp'

# Find the label
idx = text.find('\n' + label + ':\n')
if idx < 0:
    # Also try as a function-like label at line start
    m = re.search(r'^' + re.escape(label) + r':\n', text, re.MULTILINE)
    if m:
        idx = m.start() - 1 if m.start() > 0 else 0
    else:
        print(f"LABEL '{label}' NOT FOUND in zcc_ir2.s")
        print("This function may not exist or may be inlined/absent.")
        sys.exit(0)

# Show context before label (last 500 chars = ~8 instructions)
pre_start = max(0, idx - 500)
pre = text[pre_start:idx]
print("=== CONTEXT BEFORE label (what precedes it) ===")
print(pre[-400:])
print()

# Classify: function or goto label?
is_func = bool(re.search(r'pushq\s+%rbp', pre[-300:]) is None and
               (re.search(r'\bret\b|\bjmp\b', pre[-100:]) or
                re.search(r'\.globl\s+' + re.escape(label), pre)))
# Simpler heuristic: if preceded by ret/jmp it's a function entry
has_ret_before = bool(re.search(r'\bret\b', pre[-200:]))
has_globl = ('.globl ' + label) in text
has_pushq_before = bool(re.search(r'pushq\s+%rbp', pre[-50:]))  # immediately before

print("=== CLASSIFICATION ===")
if has_globl:
    print(f"  .globl {label} found → FUNCTION (exported)")
elif has_pushq_before:
    print(f"  Preceded immediately by pushq %rbp → ZCC typically emits label AFTER pushq")
    print(f"  (ZCC prologue: pushq / movq / subq / LABEL → that's unusual)")
else:
    print(f"  .globl NOT found")
    if has_ret_before:
        print(f"  Preceded by ret → likely FUNCTION ENTRY (new function starts here)")
    else:
        print(f"  NOT preceded by ret → likely a GOTO LABEL inside another function")
        print(f"  → This is a FALSE POSITIVE in the violation scanner!")
print()

# Find next top-level label to delimit the body
post_idx = idx + len('\n' + label + ':\n')
m_next = re.search(r'\n[A-Za-z_][A-Za-z0-9_]*:\n', text[post_idx:])
end_idx = post_idx + m_next.start() if m_next else min(post_idx + 1500, len(text))

body = text[post_idx:end_idx]
print(f"=== BODY of '{label}' (up to next label) ===")
print(body[:1500])
print()

# Check for saves
has_ir_save    = 'movq %rbx, ' in body
has_ast_save   = 'pushq %rbx' in body
has_ir_rest    = re.search(r'movq\s+-?\d+\(%rbp\),\s*%rbx', body) is not None
has_subq_frame = re.search(r'subq\s+\$\d+,\s*%rsp', body) is not None

print("=== SAVE/RESTORE ANALYSIS ===")
print(f"  IR save  (movq %rbx, N(%rbp)) : {'YES' if has_ir_save else 'NO'}")
print(f"  AST save (pushq %rbx)         : {'YES' if has_ast_save else 'NO'}")
print(f"  IR rest  (movq N(%rbp), %rbx) : {'YES' if has_ir_rest else 'NO'}")
print(f"  IR frame ext (subq $N, %rsp)  : {'YES' if has_subq_frame else 'NO'}")
if not has_ir_save and not has_ast_save:
    if not has_subq_frame:
        print()
        print("  VERDICT: No frame extension → goto label inside another function.")
        print("           Violation is a FALSE POSITIVE.")
    else:
        print()
        print("  VERDICT: Has frame extension but NO save → REAL BUG.")
        print("           next_offset_from_rbp is an IR function missing callee saves.")
PYEOF

# ---------------------------------------------------------------------------
# Q2b: Better violation scan — skip goto labels, only flag real functions
# ---------------------------------------------------------------------------
echo ""
echo "[ Q2b ] Re-scan with improved function detection..."

python3 - << 'PYEOF' 2>&1 | tee "$OUT/violation_scan_v2.txt"
import re

text = open('zcc_ir2.s').read()

# Split on ALL top-level labels (not .L-prefixed local labels)
parts = re.split(r'\n([A-Za-z_][A-Za-z0-9_]*):\n', text)

violations = []
false_positives = []

for i in range(1, len(parts), 2):
    name = parts[i]
    body = parts[i+1] if i+1 < len(parts) else ''
    pre  = parts[i-1] if i > 0 else ''

    if '%rbx' not in body:
        continue

    # Is this a real function entry or a goto label?
    # Real function: preceded by ret / jmp / jne / je / (beginning of file)
    # OR: has .globl name somewhere before
    has_globl   = ('.globl ' + name) in text
    pre_tail    = pre[-200:] if len(pre) >= 200 else pre
    # A function entry is typically preceded by a terminator instruction
    preceded_by_term = bool(re.search(
        r'\b(ret|jmp|je|jne|jl|jg|jle|jge|jb|ja|jbe|jae)\b\s*$', pre_tail.strip()
    ))
    is_func = has_globl or preceded_by_term

    if not is_func:
        false_positives.append(name)
        continue

    # Real function — check for saves
    ir_save  = 'movq %rbx, ' in body
    ast_save = 'pushq %rbx' in body
    if not ir_save and not ast_save:
        violations.append(name)

print(f"Likely goto labels (false positives): {len(false_positives)}")
for f in false_positives[:10]:
    print(f"  fp: {f}")

print()
print(f"REAL violations (real functions, no save): {len(violations)}")
for v in violations[:20]:
    print(f"  VIOLATION: {v}")
PYEOF

# ---------------------------------------------------------------------------
# Q3a: ASan build and crash report
# ---------------------------------------------------------------------------
echo ""
echo "[ Q3 ] ASan crash diagnosis..."

if [[ ! -f zcc_asan ]]; then
    echo "  Building ASan binary..."
    gcc -fsanitize=address -fno-omit-frame-pointer -g -O0 \
        -o zcc_asan "$ZCC_SRC" "$CP_SRC" -lm 2>&1
fi

if [[ -f zcc_asan ]]; then
    echo "  Running ASan on IR path (halt_on_error=0, expect rc=139 or ASan report)..."
    ZCC_IR_BACKEND=1 \
    ASAN_OPTIONS="detect_stack_use_after_return=1:halt_on_error=0:print_stacktrace=1:log_path=$OUT/asan_ir" \
        ./zcc_asan "$ZCC_PP" -o /dev/null 2>&1 | tail -30
    echo "  ASan logs: $OUT/asan_ir.*"

    # Summarize any asan reports
    for f in "$OUT"/asan_ir.*; do
        [[ -f "$f" ]] || continue
        echo ""
        echo "  === $f ==="
        grep -A 30 "ERROR:\|SEGV\|stack-overflow" "$f" | head -40
    done
fi

# ---------------------------------------------------------------------------
# Q3b: GDB minimal crash site — just get a backtrace
# ---------------------------------------------------------------------------
echo ""
echo "[ Q3b ] GDB crash backtrace (IR path)..."

if command -v gdb &>/dev/null && [[ -f zcc_asan ]]; then
    cat > "$OUT/gdb_bt.gdb" << 'GDBEOF'
set pagination off
set confirm off
set print thread-events off
handle SIGSEGV stop print
run zcc_pp.c -o /dev/null
bt 25
info registers rbx rip rsp rbp
quit
GDBEOF
    ZCC_IR_BACKEND=1 gdb -batch -x "$OUT/gdb_bt.gdb" ./zcc_asan 2>&1 \
        | grep -v "^Reading\|^No debugging\|^BFD\|warning:" \
        | tee "$OUT/gdb_bt.txt" | head -50
fi

# ---------------------------------------------------------------------------
# Q4: Check the csave_base formula in current compiled binary
# ---------------------------------------------------------------------------
echo ""
echo "[ Q4 ] Verify csave_base offsets are sane in zcc_ir2.s..."

python3 - << 'PYEOF' 2>&1 | tee "$OUT/csave_offset_check.txt"
import re

text = open('zcc_ir2.s').read()

# Find all csave patterns: movq %rbx, N(%rbp) — these should all be at the same
# relative position within each function's frame.
# Also look for collisions: where the rbx save offset matches a spill slot.

# Pattern: movq %rbx, -N(%rbp) — extract the offset N
saves = re.findall(r'movq %rbx, (-\d+)\(%rbp\)', text)
saves_pos = [int(s) for s in saves]

if saves_pos:
    print(f"rbx save offsets — count: {len(saves_pos)}")
    print(f"  min: {min(saves_pos)}, max: {max(saves_pos)}")
    # Distribution
    from collections import Counter
    dist = Counter(saves_pos)
    print(f"  unique offsets: {len(dist)}")
    # Most common
    for off, cnt in sorted(dist.items(), key=lambda x: -x[1])[:5]:
        print(f"    offset {off}: {cnt} functions")
else:
    print("No 'movq %rbx, N(%rbp)' patterns found!")

# Check restores
rests = re.findall(r'movq (-\d+)\(%rbp\), %rbx', text)
rests_pos = [int(s) for s in rests]
if rests_pos:
    print(f"\nrbx restore offsets — count: {len(rests_pos)}")
    print(f"  unique: {len(set(rests_pos))}")
    # Count mismatches: restores with no matching save at same offset
    save_set = set(saves_pos)
    rest_set = set(rests_pos)
    unmatched = rest_set - save_set
    if unmatched:
        print(f"  WARNING: {len(unmatched)} restore offsets with no matching save: {sorted(unmatched)[:5]}")
    else:
        print(f"  All restore offsets have matching saves ✓")
else:
    print("\nNo rbx restore patterns found!")

# Check for suspicious offsets: saves should NOT be at slot_base-8 multiples
# (which would indicate overlap with spill slots)
# But we can't easily check without knowing stack_size per function.
print("\nOffset range sanity: saves in range [MIN, MAX]")
if saves_pos:
    print(f"  Smallest (most negative): {min(saves_pos)} — must be within allocated frame")
    print(f"  Largest (least negative): {max(saves_pos)}")
PYEOF

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
echo ""
echo "================================================================"
echo "  DIAGNOSTIC SUMMARY"
echo "  Files in: $OUT/"
echo "================================================================"
ls -la "$OUT/"
echo ""
echo "Next steps based on Q1:"
echo "  If 'goto label' → fix scanner, find REAL crash via Q3 ASan/GDB output"
echo "  If 'real function, no save' → add to IR blacklist OR debug ir_ast=NULL path"
echo "  Paste $OUT/gdb_bt.txt + $OUT/asan_ir.* for final diagnosis"
