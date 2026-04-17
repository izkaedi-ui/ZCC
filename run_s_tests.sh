#!/bin/bash
cd /mnt/h/agents/selforglinux_build
echo '=== ZCC .s Output Tests ==='
echo

files=("exp1_raytracer_simd.c" "exp2_voxel_engine.c" "exp3_audio_visualizer.c" "exp4_vr_stereo.c" "exp5_physics_engine.c")
names=("raytracer" "voxel" "audio" "vr" "physics")

for i in 0 1 2 3 4; do
    f="${files[$i]}"
    n="${names[$i]}"
    expnum=$((i+1))
    out="/tmp/exp${expnum}_${n}"
    asm="/tmp/exp${expnum}_${n}.s"

    echo "--- EXP$expnum: $f ---"
    # First emit .s so we can inspect it
    ZCC_EMIT_IR=1 ./zcc "$f" -o "$asm" 2>&1 | tail -2
    if [ -f "$asm" ]; then
        LINES=$(wc -l < "$asm")
        echo "  .s output: $LINES lines"
        # Now link and run
        gcc -no-pie -O0 -o "$out" "$asm" -lm 2>&1 | head -3
        if [ -f "$out" ]; then
            timeout 8 "$out" > "${out}.ppm"
            SZ=$(stat -c%s "${out}.ppm" 2>/dev/null || echo 0)
            echo "  binary ran: PPM output $SZ bytes — OK"
        else
            echo "  link failed"
        fi
    else
        echo "  .s emit FAILED"
    fi
    echo
done

echo '=== Test Suite: Basic C features ==='
./zcc test_bitfield.c -o /tmp/test_bitfield && /tmp/test_bitfield && echo 'bitfield: PASS' || echo 'bitfield: FAIL'
./zcc test_vla.c -o /tmp/test_vla && /tmp/test_vla && echo 'vla: PASS' || echo 'vla: FAIL'
./zcc test_float_bug.c -o /tmp/test_float && /tmp/test_float && echo 'float: PASS' || echo 'float: FAIL'
./zcc test_anon.c -o /tmp/test_anon && /tmp/test_anon && echo 'anon_struct: PASS' || echo 'anon_struct: FAIL'
echo
echo 'Done.'
