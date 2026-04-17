#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux
rm -f curl_ir_raw.txt
for f in curl_build/*_pp.c; do
  ZCC_EMIT_IR=1 ./zcc "$f" 2>/dev/null >> curl_ir_raw.txt
done
echo DONE
grep -c '^; func' curl_ir_raw.txt
wc -l curl_ir_raw.txt
