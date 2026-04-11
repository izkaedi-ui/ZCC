#!/bin/bash
# ============================================================================
# 🔱 ZCC Bootstrap Verification v1.1
# ============================================================================
# Fixed: ZCC has no -S flag. Compiles to binary directly.
# Verification: compare binaries OR intermediate .s files if ZCC produces them.
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
fi

# ── Detect ZCC's intermediate .s file pattern ──
# ZCC may produce .s files as intermediates. We'll look for them.
find_asm() {
    # Check common patterns: output.s, zcc_pp.s, or any new .s in the dir
    local dir="$1"
    local base
    base=$(basename "${SELF_HOST_SRC}" .c)
    
    # Check in order of likelihood
    for candidate in "${dir}/${base}.s" "${dir}/output.s" "${ZCC_DIR}/${base}.s" "${ZCC_DIR}/output.s"; do
        if [ -f "${candidate}" ]; then
            echo "${candidate}"
            return 0
        fi
    done
    
    return 1
}

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

# ════════════════════════════════════════════════════════════════
# Stage 1: Self-compile → stage2
# ════════════════════════════════════════════════════════════════
info "Stage 1: zcc_stage1 compiling itself → zcc_stage2..."

# Snapshot .s files before compilation so we can detect new ones
BEFORE_S=$(find "${ZCC_DIR}" -maxdepth 1 -name "*.s" -newer "${OUT_DIR}/zcc_stage1" 2>/dev/null || true)

"${OUT_DIR}/zcc_stage1" "${SELF_HOST_SRC}" -o "${OUT_DIR}/zcc_stage2" 2>"${OUT_DIR}/stage1_errors.log"
S1_RC=$?

if [ ${S1_RC} -ne 0 ]; then
    fail "Stage 1 compilation failed (rc=${S1_RC}):"
    tail -20 "${OUT_DIR}/stage1_errors.log"
    BOOT_PASS=0
else
    pass "Stage 2 binary built (zcc_stage1 → zcc_stage2)"
    
    # Try to find the intermediate .s file ZCC produced
    ASM_S2=$(find_asm "${OUT_DIR}" 2>/dev/null || find_asm "${ZCC_DIR}" 2>/dev/null || true)
    if [ -n "${ASM_S2}" ]; then
        cp "${ASM_S2}" "${OUT_DIR}/stage2.s" 2>/dev/null
        S2_LINES=$(wc -l < "${OUT_DIR}/stage2.s")
        info "Found stage2 assembly: ${ASM_S2} (${S2_LINES} lines)"
    else
        # No .s found — we'll compare binaries instead
        info "No intermediate .s found — will compare binaries directly"
    fi
fi

# ════════════════════════════════════════════════════════════════
# Stage 2: stage2 compiles itself → stage3
# ════════════════════════════════════════════════════════════════
if [ ${BOOT_PASS} -eq 1 ] && [ -x "${OUT_DIR}/zcc_stage2" ]; then
    info "Stage 2: zcc_stage2 compiling itself → zcc_stage3..."
    
    "${OUT_DIR}/zcc_stage2" "${SELF_HOST_SRC}" -o "${OUT_DIR}/zcc_stage3" 2>"${OUT_DIR}/stage2_errors.log"
    S2_RC=$?
    
    if [ ${S2_RC} -ne 0 ]; then
        fail "Stage 2 compilation failed (rc=${S2_RC}):"
        tail -20 "${OUT_DIR}/stage2_errors.log"
        BOOT_PASS=0
    else
        pass "Stage 3 binary built (zcc_stage2 → zcc_stage3)"
        
        # Try to find stage3 .s
        ASM_S3=$(find_asm "${OUT_DIR}" 2>/dev/null || find_asm "${ZCC_DIR}" 2>/dev/null || true)
        if [ -n "${ASM_S3}" ]; then
            cp "${ASM_S3}" "${OUT_DIR}/stage3.s" 2>/dev/null
            S3_LINES=$(wc -l < "${OUT_DIR}/stage3.s")
            info "Found stage3 assembly: ${ASM_S3} (${S3_LINES} lines)"
        fi
    fi
fi

# ════════════════════════════════════════════════════════════════
# Verification
# ════════════════════════════════════════════════════════════════
echo ""
info "═══ BOOTSTRAP VERIFICATION ═══"

VERIFIED=0

# Method 1: Compare .s files if we have them
if [ -f "${OUT_DIR}/stage2.s" ] && [ -f "${OUT_DIR}/stage3.s" ]; then
    info "Comparing assembly: stage2.s vs stage3.s"
    if cmp -s "${OUT_DIR}/stage2.s" "${OUT_DIR}/stage3.s"; then
        pass "🔱 stage2.s == stage3.s  ←  BITWISE IDENTICAL"
        VERIFIED=1
    else
        fail "stage2.s ≠ stage3.s"
        diff "${OUT_DIR}/stage2.s" "${OUT_DIR}/stage3.s" > "${OUT_DIR}/stage2_vs_stage3.diff" 2>&1 || true
        DIFF_LINES=$(wc -l < "${OUT_DIR}/stage2_vs_stage3.diff")
        info "Diff: ${DIFF_LINES} lines divergent"
        head -30 "${OUT_DIR}/stage2_vs_stage3.diff"
        BOOT_PASS=0
    fi
fi

# Method 2: Compare binaries
if [ -f "${OUT_DIR}/zcc_stage2" ] && [ -f "${OUT_DIR}/zcc_stage3" ]; then
    info "Comparing binaries: zcc_stage2 vs zcc_stage3"
    if cmp -s "${OUT_DIR}/zcc_stage2" "${OUT_DIR}/zcc_stage3"; then
        pass "🔱 zcc_stage2 == zcc_stage3  ←  BINARIES IDENTICAL"
        VERIFIED=1
        
        SHA=$(sha256sum "${OUT_DIR}/zcc_stage2" | cut -d' ' -f1)
        info "SHA-256: ${SHA}"
        echo "${SHA}" > "${OUT_DIR}/verified_hash.txt"
    else
        if [ ${VERIFIED} -eq 0 ]; then
            # Binaries differ — but this might be due to timestamps/ASLR
            # Compare .text sections via objdump
            info "Binaries differ — comparing .text sections..."
            
            objdump -d "${OUT_DIR}/zcc_stage2" > "${OUT_DIR}/stage2_disasm.txt" 2>/dev/null
            objdump -d "${OUT_DIR}/zcc_stage3" > "${OUT_DIR}/stage3_disasm.txt" 2>/dev/null
            
            if [ -f "${OUT_DIR}/stage2_disasm.txt" ] && [ -f "${OUT_DIR}/stage3_disasm.txt" ]; then
                if diff -q "${OUT_DIR}/stage2_disasm.txt" "${OUT_DIR}/stage3_disasm.txt" >/dev/null 2>&1; then
                    pass "🔱 Disassembly identical (binary diff is metadata only)"
                    VERIFIED=1
                else
                    fail "Disassembly differs — codegen is NOT stable"
                    diff "${OUT_DIR}/stage2_disasm.txt" "${OUT_DIR}/stage3_disasm.txt" \
                        > "${OUT_DIR}/disasm.diff" 2>&1 || true
                    DIFF_LINES=$(wc -l < "${OUT_DIR}/disasm.diff")
                    info "Disasm diff: ${DIFF_LINES} lines"
                    head -30 "${OUT_DIR}/disasm.diff"
                    BOOT_PASS=0
                fi
            fi
        else
            info "Binary diff is expected (timestamps) — .s match confirmed above"
        fi
    fi
fi

if [ ${VERIFIED} -eq 1 ]; then
    pass "BOOTSTRAP VERIFIED — ZCC is a correct self-hosting compiler"
else
    if [ ${BOOT_PASS} -eq 1 ]; then
        warn "Could not verify — no .s or binary comparison succeeded"
        BOOT_PASS=0
    fi
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
    if [ $? -eq 0 ] && [ -x "${OUT_DIR}/hello_s3" ]; then
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

# ════════════════════════════════════════════════════════════════
# Stats
# ════════════════════════════════════════════════════════════════
echo ""
info "═══ BINARY STATS ═══"
if [ -f "${OUT_DIR}/zcc_stage2" ]; then
    S2_SIZE=$(stat -c%s "${OUT_DIR}/zcc_stage2" 2>/dev/null || stat -f%z "${OUT_DIR}/zcc_stage2" 2>/dev/null || echo "?")
    info "Stage 2 binary: ${S2_SIZE} bytes"
fi
if [ -f "${OUT_DIR}/zcc_stage3" ]; then
    S3_SIZE=$(stat -c%s "${OUT_DIR}/zcc_stage3" 2>/dev/null || stat -f%z "${OUT_DIR}/zcc_stage3" 2>/dev/null || echo "?")
    info "Stage 3 binary: ${S3_SIZE} bytes"
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
