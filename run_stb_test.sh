#!/bin/sh
set -e
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== GCC baseline ==="
gcc -O0 -o stb_test_gcc stb_test.c -lm
./stb_test_gcc test.png
cp pixels.raw pixels_gcc.raw
GCC_MD5=$(md5sum pixels_gcc.raw | cut -d' ' -f1)
echo "GCC MD5: $GCC_MD5"

echo ""
echo "=== ZCC compile ==="
./zcc stb_test.c -o stb_test.s 2>&1
echo "[OK] ZCC compiled stb_test.c"

echo ""
echo "=== Assemble + link ==="
gcc stb_test.s -o stb_test_zcc -lm
echo "[OK] Linked stb_test_zcc"

echo ""
echo "=== ZCC run ==="
./stb_test_zcc test.png
ZCC_MD5=$(md5sum pixels.raw | cut -d' ' -f1)
echo "ZCC MD5: $ZCC_MD5"

echo ""
echo "=== COMPARISON ==="
if [ "$GCC_MD5" = "$ZCC_MD5" ]; then
    echo "MATCH: pixel buffers are byte-for-byte identical"
    echo "GCC: $GCC_MD5"
    echo "ZCC: $ZCC_MD5"
else
    echo "MISMATCH!"
    echo "GCC: $GCC_MD5"
    echo "ZCC: $ZCC_MD5"
    echo "Diffing first divergence..."
    cmp -l pixels_gcc.raw pixels.raw | head -20
fi
