#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux

# Find curl files using va_list and recompile them
for f in $(grep -rl 'va_start\|va_arg\|va_list' curl-8.7.1/lib/*.c 2>/dev/null); do
    base=$(basename "$f" .c)
    echo "Recompiling $base..."
    gcc -E -DHAVE_CONFIG_H -DCURL_STATICLIB \
        -Icurl-8.7.1/lib -Icurl-8.7.1/include \
        -Izcc-libc/ -include zcc-libc/zcc_compat.h \
        "$f" 2>/dev/null | grep -v '^# ' > "curl_build/${base}_pp.c"
    
    ./zcc "curl_build/${base}_pp.c" -o "curl_build/${base}.s" 2>&1 | tail -1
    
    if [ -f "curl_build/${base}.s" ]; then
        gcc -c "curl_build/${base}.s" -o "curl_build/${base}.o" 2>/dev/null
    fi
done

echo ""
echo "=== Done recompiling VA files ==="
