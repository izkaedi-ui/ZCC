#!/bin/bash
cd /mnt/h/agents/selforglinux_build/cve_corpus
ZCC="../zcc.exe"
VULN=0; SAFE=0; TP=0; FP=0
for f in juliet_subset/CWE416_Use_After_Free/*.c; do
    bn=$(basename "$f" .c)
    gcc -E -nostdinc -DINCLUDEMAIN=1 -I. "$f" > tmp_pp.c 2>/dev/null || continue
    out=$($ZCC --security-416 tmp_pp.c -o /tmp/null.s 2>/dev/null)
    violations=$(echo "$out" | grep -c 'use-after-free in')
    if echo "$bn" | grep -q '_bad'; then
        VULN=$((VULN+1))
        [ "$violations" -gt 0 ] && TP=$((TP+1))
    else
        SAFE=$((SAFE+1))
        [ "$violations" -gt 0 ] && FP=$((FP+1)) && echo "FP: $bn (violations=$violations)"
    fi
done
echo "security-416 live test on ${VULN} bad + ${SAFE} good Juliet files:"
echo "  TP=$TP FP=$FP (from $VULN vuln, $SAFE safe)"
echo "  Prec=$(python3 -c "print(f'{$TP/($TP+$FP+0.0001):.2f}')" 2>/dev/null || echo 'calc error')"
echo "  Rec=$(python3 -c "print(f'{$TP/$VULN:.2f}')" 2>/dev/null || echo 'calc error')"
rm -f tmp_pp.c
