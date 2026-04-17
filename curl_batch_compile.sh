#!/bin/sh
# curl_batch_compile.sh - Batch compile curl source files through ZCC
cd /mnt/h/__DOWNLOADS/selforglinux
mkdir -p curl_build

PASS=0
FAIL=0
TOTAL=0

FILES="urlapi easy url transfer multi http ftp sendf connect progress select socks getinfo share hash llist hostip escape formdata strcase strerror timeval if2ip conncache dotdot strdup curl_memrchr nonblock warnless curl_ctype parsedate speedcheck slist getenv version telnet dict tftp imap pop3 smtp rtsp curl_addrinfo curl_sspi curl_multibyte hostasyn hostsyn inet_pton content_encoding http_proxy socks_gssapi pingpong noproxy headers cfilters bufref dynbuf rand rename hmac curl_endian sha256_output setopt curl_path curl_range bufq timediff dynhds request cw_bearssl cf_socket cf_haproxy http_negotiate http_aws_sigv4 curl_trc doh"

for bn in $FILES; do
    f="curl-8.7.1/lib/${bn}.c"
    TOTAL=$((TOTAL+1))
    if [ ! -f "$f" ]; then
        echo "SKIP: $bn (file not found)"
        continue
    fi
    gcc -E -DHAVE_CONFIG_H -DCURL_STATICLIB \
        -Icurl-8.7.1/lib -Icurl-8.7.1/include \
        -include zcc-libc/zcc_compat.h -Izcc-libc/ \
        "$f" 2>/dev/null | sed '/^# [0-9]/d' > "curl_build/${bn}_pp.c"
    
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
echo "Results: $PASS PASS / $FAIL FAIL / $TOTAL TOTAL"
