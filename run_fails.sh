#!/bin/bash
for f in curl_build/*_pp.c; do
    ./zcc -c "$f" -o /tmp/discard.o > /dev/null 2>&1 || echo "FAILED $f"
done
