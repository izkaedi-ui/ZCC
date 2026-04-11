#!/bin/bash
gcc -O0 -o ref_out test.c >/dev/null 2>&1
./ref_out > ref.txt 2>/dev/null
./zcc test.c -o zcc_out.s >/dev/null 2>&1
gcc -O0 -o zcc_out zcc_out.s >/dev/null 2>&1
./zcc_out > zcc.txt 2>/dev/null
# If outputs are both created and they differ, creduce should keep this test
if [ -f ref.txt ] && [ -f zcc.txt ]; then
    if ! cmp -s ref.txt zcc.txt; then
        exit 0 # Interesting!
    fi
fi
exit 1 # Not interesting
