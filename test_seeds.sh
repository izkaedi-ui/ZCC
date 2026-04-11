#!/bin/bash
# 🔱 ZCC Seed Regression Test
# Tests the 6 previously failing seeds + 4 known passing seeds
# Usage: bash test_seeds.sh [path_to_zcc]

ZCC=${1:-./zcc}
PASS=0
FAIL=0
CRASH=0

SEEDS=(42 43 44 45 46 47 48 49 50 51)
KNOWN_PASS=(42 43 46 49)

run_seed() {
    local seed=$1
    local c_file="fuzz_run6/mismatches/mismatch_seed${seed}.c"

    # If not in mismatches, generate it
    if [ ! -f "$c_file" ]; then
        c_file="/tmp/zcc_test_seed${seed}.c"
        python3 -c "
import sys, os
sys.path.insert(0, '.')
# inline minimal generator if zcc_fuzz.py supports --dump-seed
" 2>/dev/null
        # Fall back: run fuzzer for this seed only
        python3 zcc_fuzz.py --zcc "$ZCC" --count 1 --seed "$seed" \
            --output-dir /tmp/zcc_seed_tmp_${seed} 2>/dev/null
        c_file="/tmp/zcc_seed_tmp_${seed}/mismatches/mismatch_seed${seed}.c"
        if [ ! -f "$c_file" ]; then
            echo "  [seed=$seed] SKIP (no source found)"
            return
        fi
    fi

    # GCC reference
    gcc_out=$(gcc -O0 -o /tmp/ref_seed${seed} "$c_file" 2>/dev/null && \
              /tmp/ref_seed${seed} 2>/dev/null)

    # ZCC compile
    "$ZCC" "$c_file" -o /tmp/zcc_seed${seed}.s 2>/dev/null
    if [ $? -ne 0 ]; then
        echo "  [seed=$seed] ✗ ZCC COMPILE FAIL"
        ((FAIL++))
        return
    fi

    gcc -O0 -o /tmp/zcc_bin_seed${seed} /tmp/zcc_seed${seed}.s 2>/dev/null
    zcc_out=$(/tmp/zcc_bin_seed${seed} 2>/dev/null)
    exit_code=$?

    if [ $exit_code -eq 136 ] || [ $exit_code -eq 139 ]; then
        echo "  [seed=$seed] ✗ CRASH (exit=$exit_code) — GCC=$gcc_out"
        ((CRASH++))
        ((FAIL++))
        return
    fi

    if [ "$gcc_out" = "$zcc_out" ]; then
        echo "  [seed=$seed] ✓ PASS  ($gcc_out)"
        ((PASS++))
    else
        echo "  [seed=$seed] ✗ MISMATCH — GCC=$gcc_out  ZCC=$zcc_out"
        ((FAIL++))
    fi
}

echo "🔱 ZCC Seed Regression — $(date)"
echo "   ZCC: $ZCC"
echo ""

for seed in "${SEEDS[@]}"; do
    run_seed $seed
done

echo ""
echo "═══ RESULTS ═══"
echo "  Pass:   $PASS / ${#SEEDS[@]}"
echo "  Fail:   $FAIL"
echo "  Crash:  $CRASH"
echo ""

if [ $FAIL -eq 0 ]; then
    echo "🔱 ALL SEEDS PASS — ready for selfhost verify"
    exit 0
else
    echo "✗ $FAIL seed(s) still failing"
    exit 1
fi
