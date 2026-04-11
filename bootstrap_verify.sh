#!/bin/bash
# ============================================================================
# 🔱 ZCC Bootstrap Verification
# ============================================================================
# Builds stage1→stage2→stage3 and verifies bitwise identity.
# Also captures assembly diffs at each stage for diagnostics.
#
# Usage: bootstrap_verify.sh <ZCC_DIR> <OUTPUT_DIR>
# ============================================================================

set -uo pipefail

ZCC_DIR="${1:-.}"
OUT_DIR="${2:-./bootstrap_out}"
mkdir -p "${OUT_DIR}"

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
CYN='\033[0;36m'
RST='\033[0m'

info()  { echo -e "  ${CYN}ℹ${RST}  $1"; }
pass()  { echo -e "  ${GRN}✓${RST}  $1"; }
fail()  { echo -e "  ${RED}✗${RST}  $1"; }
warn()  { echo -e "  ${YLW}⚠${RST}  $1"; }

BOOT_PASS=1

# ── Locate source files ──
ZCC_C="${ZCC_DIR}/zcc.c"
PASSES_C="${ZCC_DIR}/compiler_passes.c"

if [ ! -f "${ZCC_C}" ]; then
    fail "zcc.c not found in ${ZCC_DIR}"
    exit 1
fi

HAS_PASSES=0
PASSES_FLAG=""
if [ -f "${PASSES_C}" ]; then
    HAS_PASSES=1
    PASSES_FLAG="${PASSES_C}"
    info "Found compiler_passes.c — building with IR passes"
fi

# ── Check for preprocessed source ──
ZCC_PP="${ZCC_DIR}/zcc_pp.c"
SELF_HOST_SRC="${ZCC_C}"
if [ -f "${ZCC_PP}" ]; then
    SELF_HOST_SRC="${ZCC_PP}"
    info "Using preprocessed zcc_pp.c for self-hosting stages"
else
    warn "No zcc_pp.c found — self-host will use zcc.c directly"
    warn "(May fail if ZCC can't handle #include <stdint.h>)"
fi

# ════════════════════════════════════════════════════════════════
# Stage 0: Build with GCC (the trusted compiler)
# ════════════════════════════════════════════════════════════════
info "Stage 0: Building ZCC with GCC..."

GCC_CMD="gcc -O0 -g -std=c99 -o ${OUT_DIR}/zcc_stage1 ${ZCC_C}"
if [ ${HAS_PASSES} -eq 1 ]; then
    GCC_CMD="${GCC_CMD} ${PASSES_FLAG}"
fi
GCC_CMD="${GCC_CMD} -lm"

eval ${GCC_CMD} 2>"${OUT_DIR}/stage0_errors.log"
if [ $? -ne 0 ]; then
    fail "Stage 0 (GCC build) failed:"
    cat "${OUT_DIR}/stage0_errors.log"
    exit 1
fi
pass "Stage 1 binary built (GCC → zcc_stage1)"

# ── Stage 0: Generate reference assembly ──
info "Generating GCC reference assembly..."
gcc -O0 -S -std=c99 -o "${OUT_DIR}/gcc_ref.s" "${SELF_HOST_SRC}" 2>/dev/null || true

# ════════════════════════════════════════════════════════════════
# Stage 1: Self-compile → stage2
# ════════════════════════════════════════════════════════════════
info "Stage 1: zcc_stage1 compiling itself → zcc_stage2..."

# Capture assembly output
"${OUT_DIR}/zcc_stage1" "${SELF_HOST_SRC}" -S -o "${OUT_DIR}/stage2.s" 2>"${OUT_DIR}/stage1_errors.log"
S1_RC=$?

if [ ${S1_RC} -ne 0 ]; then
    fail "Stage 1 compilation failed (rc=${S1_RC}):"
    tail -20 "${OUT_DIR}/stage1_errors.log"
    BOOT_PASS=0
fi

# Assemble + link stage2
if [ -f "${OUT_DIR}/stage2.s" ]; then
    # If we have compiler_passes.c, compile it with GCC and link
    if [ ${HAS_PASSES} -eq 1 ]; then
        gcc -O0 -c -o "${OUT_DIR}/passes.o" "${PASSES_C}" 2>/dev/null
        gcc -o "${OUT_DIR}/zcc_stage2" "${OUT_DIR}/stage2.s" "${OUT_DIR}/passes.o" -lm 2>"${OUT_DIR}/stage2_link.log"
    else
        gcc -o "${OUT_DIR}/zcc_stage2" "${OUT_DIR}/stage2.s" -lm 2>"${OUT_DIR}/stage2_link.log"
    fi
    
    if [ $? -ne 0 ]; then
        fail "Stage 2 link failed:"
        cat "${OUT_DIR}/stage2_link.log"
        BOOT_PASS=0
    else
        pass "Stage 2 binary linked (zcc_stage1 → zcc_stage2)"
        
        # Stats
        S2_SIZE=$(wc -l < "${OUT_DIR}/stage2.s")
        info "stage2.s: ${S2_SIZE} lines"
    fi
else
    fail "No stage2.s produced"
    BOOT_PASS=0
fi

# ════════════════════════════════════════════════════════════════
# Stage 2: stage2 compiles itself → stage3
# ════════════════════════════════════════════════════════════════
if [ ${BOOT_PASS} -eq 1 ] && [ -x "${OUT_DIR}/zcc_stage2" ]; then
    info "Stage 2: zcc_stage2 compiling itself → zcc_stage3..."
    
    "${OUT_DIR}/zcc_stage2" "${SELF_HOST_SRC}" -S -o "${OUT_DIR}/stage3.s" 2>"${OUT_DIR}/stage2_errors.log"
    S2_RC=$?
    
    if [ ${S2_RC} -ne 0 ]; then
        fail "Stage 2 compilation failed (rc=${S2_RC}):"
        tail -20 "${OUT_DIR}/stage2_errors.log"
        BOOT_PASS=0
    fi
    
    if [ -f "${OUT_DIR}/stage3.s" ]; then
        # Link stage3
        if [ ${HAS_PASSES} -eq 1 ]; then
            gcc -o "${OUT_DIR}/zcc_stage3" "${OUT_DIR}/stage3.s" "${OUT_DIR}/passes.o" -lm 2>/dev/null
        else
            gcc -o "${OUT_DIR}/zcc_stage3" "${OUT_DIR}/stage3.s" -lm 2>/dev/null
        fi
        
        if [ $? -eq 0 ]; then
            pass "Stage 3 binary linked (zcc_stage2 → zcc_stage3)"
            S3_SIZE=$(wc -l < "${OUT_DIR}/stage3.s")
            info "stage3.s: ${S3_SIZE} lines"
        else
            fail "Stage 3 link failed"
            BOOT_PASS=0
        fi
    else
        fail "No stage3.s produced"
        BOOT_PASS=0
    fi
fi

# ════════════════════════════════════════════════════════════════
# Verification: stage2.s == stage3.s
# ════════════════════════════════════════════════════════════════
echo ""
info "═══ BOOTSTRAP VERIFICATION ═══"

if [ -f "${OUT_DIR}/stage2.s" ] && [ -f "${OUT_DIR}/stage3.s" ]; then
    if cmp -s "${OUT_DIR}/stage2.s" "${OUT_DIR}/stage3.s"; then
        pass "🔱 stage2.s == stage3.s  ←  BITWISE IDENTICAL"
        pass "BOOTSTRAP VERIFIED — ZCC is a correct self-hosting compiler"
        
        # Hash for the record
        SHA=$(sha256sum "${OUT_DIR}/stage2.s" | cut -d' ' -f1)
        info "SHA-256: ${SHA}"
        echo "${SHA}" > "${OUT_DIR}/verified_hash.txt"
    else
        fail "stage2.s ≠ stage3.s — BOOTSTRAP BROKEN"
        BOOT_PASS=0
        
        # Generate detailed diff
        diff "${OUT_DIR}/stage2.s" "${OUT_DIR}/stage3.s" > "${OUT_DIR}/stage2_vs_stage3.diff" 2>&1 || true
        DIFF_LINES=$(wc -l < "${OUT_DIR}/stage2_vs_stage3.diff")
        info "Diff: ${DIFF_LINES} lines divergent"
        
        # Show first 50 lines of diff
        info "First divergences:"
        head -50 "${OUT_DIR}/stage2_vs_stage3.diff" | while IFS= read -r line; do
            echo "    ${line}"
        done
        
        # Function-level diff analysis
        info "Analyzing divergent functions..."
        grep -n "^[a-zA-Z_]" "${OUT_DIR}/stage2.s" > "${OUT_DIR}/s2_funcs.txt" 2>/dev/null || true
        grep -n "^[a-zA-Z_]" "${OUT_DIR}/stage3.s" > "${OUT_DIR}/s3_funcs.txt" 2>/dev/null || true
        
        # Find which functions differ
        python3 -c "
import sys
s2 = open('${OUT_DIR}/stage2.s').readlines()
s3 = open('${OUT_DIR}/stage3.s').readlines()

# Extract function boundaries
def get_functions(lines):
    funcs = {}
    current = None
    start = 0
    for i, line in enumerate(lines):
        if line and not line[0].isspace() and ':' in line and not line.startswith('.'):
            if current:
                funcs[current] = ''.join(lines[start:i])
            current = line.split(':')[0].strip()
            start = i
    if current:
        funcs[current] = ''.join(lines[start:])
    return funcs

f2 = get_functions(s2)
f3 = get_functions(s3)

divergent = []
for name in sorted(set(f2.keys()) | set(f3.keys())):
    if name in f2 and name in f3:
        if f2[name] != f3[name]:
            divergent.append(name)
    elif name in f2:
        divergent.append(f'{name} (only in stage2)')
    else:
        divergent.append(f'{name} (only in stage3)')

if divergent:
    print(f'  {len(divergent)} functions differ:')
    for d in divergent[:20]:
        print(f'    → {d}')
    if len(divergent) > 20:
        print(f'    ... and {len(divergent)-20} more')
else:
    print('  No function-level differences found (whitespace/label only?)')
" 2>/dev/null || warn "Could not analyze function-level diffs"
    fi
else
    fail "Missing assembly files — cannot verify"
    BOOT_PASS=0
fi

# ════════════════════════════════════════════════════════════════
# Bonus: Stage2 vs GCC assembly comparison
# ════════════════════════════════════════════════════════════════
if [ -f "${OUT_DIR}/gcc_ref.s" ] && [ -f "${OUT_DIR}/stage2.s" ]; then
    echo ""
    info "═══ BONUS: ZCC vs GCC Assembly Stats ═══"
    
    GCC_LINES=$(wc -l < "${OUT_DIR}/gcc_ref.s")
    ZCC_LINES=$(wc -l < "${OUT_DIR}/stage2.s")
    
    info "GCC assembly:  ${GCC_LINES} lines"
    info "ZCC assembly:  ${ZCC_LINES} lines"
    
    if [ ${GCC_LINES} -gt 0 ]; then
        RATIO=$((ZCC_LINES * 100 / GCC_LINES))
        info "ZCC/GCC ratio: ${RATIO}%"
    fi
    
    # Instruction count comparison
    GCC_INSN=$(grep -cE '^\s+(mov|add|sub|imul|idiv|push|pop|call|ret|jmp|j[a-z]+|cmp|test|lea|xor|and|or|shl|shr|sar|neg|not|set)' "${OUT_DIR}/gcc_ref.s" 2>/dev/null || echo 0)
    ZCC_INSN=$(grep -cE '^\s+(mov|add|sub|imul|idiv|push|pop|call|ret|jmp|j[a-z]+|cmp|test|lea|xor|and|or|shl|shr|sar|neg|not|set)' "${OUT_DIR}/stage2.s" 2>/dev/null || echo 0)
    
    info "GCC instructions: ${GCC_INSN}"
    info "ZCC instructions: ${ZCC_INSN}"
fi

# ════════════════════════════════════════════════════════════════
# Functional verification: stage3 can compile a test program
# ════════════════════════════════════════════════════════════════
if [ -x "${OUT_DIR}/zcc_stage3" ]; then
    echo ""
    info "═══ FUNCTIONAL TEST: stage3 compiles hello world ═══"
    
    cat > "${OUT_DIR}/hello_test.c" << 'HELLO'
int putchar(int c);

int main(void) {
    char *msg = "ZCC STAGE3 OK\n";
    int i = 0;
    while (msg[i]) {
        putchar(msg[i]);
        i = i + 1;
    }
    return 0;
}
HELLO
    
    "${OUT_DIR}/zcc_stage3" "${OUT_DIR}/hello_test.c" -o "${OUT_DIR}/hello_s3" 2>/dev/null
    if [ $? -eq 0 ]; then
        HELLO_OUT=$("${OUT_DIR}/hello_s3" 2>/dev/null || echo "CRASH")
        if echo "${HELLO_OUT}" | grep -q "ZCC STAGE3 OK"; then
            pass "Stage 3 functional test: ${HELLO_OUT%%$'\n'*}"
        else
            fail "Stage 3 produced wrong output: ${HELLO_OUT}"
            BOOT_PASS=0
        fi
    else
        fail "Stage 3 cannot compile hello world"
        BOOT_PASS=0
    fi
fi

# ── Exit ──
echo ""
if [ ${BOOT_PASS} -eq 1 ]; then
    info "Bootstrap verification: ALL CLEAR"
    exit 0
else
    info "Bootstrap verification: FAILURES DETECTED"
    exit 1
fi
