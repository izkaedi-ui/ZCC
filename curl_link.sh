#!/bin/sh
# curl_link_v2.sh — Full reassembly + link analysis
cd /mnt/h/__DOWNLOADS/selforglinux/curl_build

echo "=== Reassemble ALL .s files ==="
PASS=0
FAIL=0
for f in *.s; do
    base=$(echo "$f" | sed 's/\.s$//')
    if gcc -c "$f" -o "${base}.o" 2>/dev/null; then
        PASS=$((PASS+1))
    else
        echo "ASM_FAIL: $f"
        FAIL=$((FAIL+1))
    fi
done
echo "Assembled: $PASS ok, $FAIL fail"

echo ""
echo "=== Symbol analysis ==="
# All symbols defined across all .o files
nm *.o 2>/dev/null | grep ' [TDdBb] ' | awk '{print $3}' | sort -u > /tmp/curl_def.txt
# All undefined references
nm *.o 2>/dev/null | grep ' U ' | awk '{print $2}' | sort -u > /tmp/curl_und.txt
# Missing = undefined and NOT defined anywhere
comm -23 /tmp/curl_und.txt /tmp/curl_def.txt > /tmp/curl_ext.txt

echo "Defined: $(wc -l < /tmp/curl_def.txt)"
echo "Undefined refs: $(wc -l < /tmp/curl_und.txt)"
echo "External (truly missing): $(wc -l < /tmp/curl_ext.txt)"

echo ""
echo "--- Curl_ internal still missing ---"
grep '^Curl_' /tmp/curl_ext.txt | wc -l
grep '^Curl_' /tmp/curl_ext.txt | head -20

echo ""
echo "--- Public curl_ API missing ---"
grep '^curl_' /tmp/curl_ext.txt

echo ""
echo "--- curlx_ missing ---"
grep '^curlx_' /tmp/curl_ext.txt

echo ""
echo "--- System/libc missing ---"
grep -v '^[Cc]url' /tmp/curl_ext.txt
