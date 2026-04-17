#!/bin/sh
# curl_compile_remaining.sh — Compile all curl source files we haven't compiled yet
cd /mnt/h/__DOWNLOADS/selforglinux

PASS=0
FAIL=0
SKIP=0
TOTAL=0

# Get list of ALL .c files in curl lib
for f in curl-8.7.1/lib/*.c; do
    bn=$(basename "$f" .c)
    TOTAL=$((TOTAL+1))
    
    # Skip if already compiled
    if [ -f "curl_build/${bn}.s" ]; then
        SKIP=$((SKIP+1))
        continue
    fi
    
    # Preprocess
    gcc -E -DHAVE_CONFIG_H -DCURL_STATICLIB \
        -Icurl-8.7.1/lib -Icurl-8.7.1/include \
        -include zcc-libc/zcc_compat.h -Izcc-libc/ \
        "$f" 2>/dev/null | sed '/^# [0-9]/d' > "curl_build/${bn}_pp.c"
    
    # Check if preprocessing produced output
    if [ ! -s "curl_build/${bn}_pp.c" ]; then
        echo "PP_EMPTY: $bn"
        FAIL=$((FAIL+1))
        continue
    fi
    
    # Compile
    if ./zcc "curl_build/${bn}_pp.c" -o "curl_build/${bn}.s" 2>"curl_build/${bn}.err"; then
        echo "PASS: $bn"
        PASS=$((PASS+1))
    else
        ERRCNT=$(grep -c 'error:' "curl_build/${bn}.err" 2>/dev/null || echo 0)
        FIRST_ERR=$(grep 'error:' "curl_build/${bn}.err" 2>/dev/null | head -1)
        echo "FAIL: $bn ($ERRCNT errors) — $FIRST_ERR"
        FAIL=$((FAIL+1))
    fi
done

echo "========================"
echo "NEW: $PASS pass / $FAIL fail / $SKIP already done / $TOTAL total"
