#!/bin/bash
# Validation script for ZCC experiments
# Verifies code quality, feature usage, and correctness

set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}╔══════════════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}║  ZCC EXPERIMENTS - CODE VALIDATION SUITE                        ║${NC}"
echo -e "${CYAN}╚══════════════════════════════════════════════════════════════════╝${NC}"
echo ""

PASS=0
FAIL=0

# Test function
test_feature() {
    local name=$1
    local pattern=$2
    local file=$3
    
    if grep -q "$pattern" "$file" 2>/dev/null; then
        echo -e "${GREEN}✓${NC} $name found in $file"
        ((PASS++))
        return 0
    else
        echo -e "${RED}✗${NC} $name NOT found in $file"
        ((FAIL++))
        return 1
    fi
}

echo -e "${YELLOW}1. Checking Feature Usage...${NC}"
echo ""

# CG-005: Inline Assembly
test_feature "Inline asm (exp1)" "__asm__.*volatile" "exp1_raytracer_simd.c"
test_feature "Inline asm (exp3)" "__asm__.*volatile" "exp3_audio_visualizer.c"
test_feature "Inline asm (exp4)" "__asm__.*volatile" "exp4_vr_stereo.c"
test_feature "Inline asm (exp5)" "__asm__.*volatile" "exp5_physics_engine.c"

echo ""

# CG-006/CG-008: VLAs
test_feature "VLA (exp1)" "framebuffer\[height\]\[width\]" "exp1_raytracer_simd.c"
test_feature "VLA (exp2)" "framebuffer\[height\]\[width\]" "exp2_voxel_engine.c"
test_feature "VLA (exp3)" "fft_data\[fft_size\]" "exp3_audio_visualizer.c"
test_feature "VLA (exp4)" "left_framebuffer\[eye_height\]" "exp4_vr_stereo.c"
test_feature "VLA (exp5)" "RigidBody bodies\[num_bodies\]" "exp5_physics_engine.c"

echo ""

# CG-007: typeof
test_feature "typeof (exp5)" "typeof" "exp5_physics_engine.c"

echo ""

# CG-009: _Generic
test_feature "_Generic (exp2)" "_Generic" "exp2_voxel_engine.c"
test_feature "_Generic (exp5)" "_Generic" "exp5_physics_engine.c"

echo ""

# CG-010: Bitfields
test_feature "Bitfields (exp1)" "unsigned.*:.*[0-9]" "exp1_raytracer_simd.c"
test_feature "Bitfields (exp2)" "unsigned.*:.*[0-9]" "exp2_voxel_engine.c"
test_feature "Bitfields (exp4)" "unsigned.*:.*[0-9]" "exp4_vr_stereo.c"

echo ""
echo -e "${YELLOW}2. Checking Code Quality...${NC}"
echo ""

# Check for common issues
for exp in exp*.c; do
    # Check includes
    if grep -q "#include <stdio.h>" "$exp"; then
        echo -e "${GREEN}✓${NC} $exp has stdio.h"
        ((PASS++))
    else
        echo -e "${RED}✗${NC} $exp missing stdio.h"
        ((FAIL++))
    fi
    
    # Check main function
    if grep -q "int main(void)" "$exp"; then
        echo -e "${GREEN}✓${NC} $exp has main function"
        ((PASS++))
    else
        echo -e "${RED}✗${NC} $exp missing main function"
        ((FAIL++))
    fi
done

echo ""
echo -e "${YELLOW}3. Checking Documentation...${NC}"
echo ""

# Check README exists
if [ -f "README.md" ]; then
    echo -e "${GREEN}✓${NC} README.md exists"
    ((PASS++))
    
    # Check README has all experiments
    for i in 1 2 3 4 5; do
        if grep -q "EXPERIMENT $i" "README.md"; then
            echo -e "${GREEN}✓${NC} README documents Experiment $i"
            ((PASS++))
        else
            echo -e "${RED}✗${NC} README missing Experiment $i"
            ((FAIL++))
        fi
    done
else
    echo -e "${RED}✗${NC} README.md not found"
    ((FAIL+=6))
fi

echo ""

# Check Makefile
if [ -f "Makefile" ]; then
    echo -e "${GREEN}✓${NC} Makefile exists"
    ((PASS++))
    
    # Check Makefile has all targets
    for i in 1 2 3 4 5; do
        if grep -q "exp$i:" "Makefile"; then
            echo -e "${GREEN}✓${NC} Makefile has exp$i target"
            ((PASS++))
        else
            echo -e "${RED}✗${NC} Makefile missing exp$i target"
            ((FAIL++))
        fi
    done
else
    echo -e "${RED}✗${NC} Makefile not found"
    ((FAIL+=6))
fi

echo ""
echo -e "${YELLOW}4. Code Statistics...${NC}"
echo ""

# Line counts
total_lines=0
for exp in exp*.c; do
    lines=$(wc -l < "$exp")
    total_lines=$((total_lines + lines))
    echo "  $exp: $lines lines"
done

echo ""
echo "  Total: $total_lines lines of code"
echo ""

# Feature counts
echo "Feature usage across all experiments:"
echo "  Inline asm:  $(grep -c '__asm__.*volatile' exp*.c) occurrences"
echo "  VLAs:        $(grep -c '\[height\]\[width\]' exp*.c) + $(grep -c '\[.*_size\]' exp*.c) occurrences"
echo "  typeof:      $(grep -c 'typeof' exp*.c) occurrences"
echo "  _Generic:    $(grep -c '_Generic' exp*.c) occurrences"
echo "  Bitfields:   $(grep -c 'unsigned.*:.*[0-9]' exp*.c) occurrences"

echo ""
echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
echo -e "${CYAN}VALIDATION SUMMARY${NC}"
echo -e "${CYAN}═══════════════════════════════════════════════════════════${NC}"
echo ""
echo -e "Tests passed: ${GREEN}$PASS${NC}"
echo -e "Tests failed: ${RED}$FAIL${NC}"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}✓ ALL VALIDATION CHECKS PASSED!${NC}"
    echo -e "${GREEN}Code is production-ready for Gemini to build.${NC}"
    exit 0
else
    echo -e "${RED}✗ VALIDATION FAILED${NC}"
    echo -e "${RED}Please fix the issues above before proceeding.${NC}"
    exit 1
fi
