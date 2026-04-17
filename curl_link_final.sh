#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux/curl_build

# Step 1: Rename 'errno' symbol in all ZCC .o files to '__zcc_errno'
echo "=== Renaming errno references ==="
for f in *.o; do
    case "$f" in
        curl_test.o|curl_stubs.o|curl_sysvals.o|curl_errno_wrap.o) continue ;;
    esac
    if nm "$f" 2>/dev/null | grep -q ' U errno$'; then
        objcopy --redefine-sym errno=__zcc_errno "$f"
    fi
done
echo "Done"

# Step 2: Errno wrapper
gcc -c -w -x c - -o curl_errno_wrap.o <<'EOF'
int __zcc_errno = 0;
EOF

# Step 3: Clean sysvals (no errno)
cat > /tmp/curl_sysvals_clean.c <<'SEOF'
#undef EAGAIN
#undef EINPROGRESS
#undef EINVAL
#undef ERANGE
#undef EACCES
#undef EADDRINUSE
#undef EADDRNOTAVAIL
#undef EAFNOSUPPORT
int EAGAIN        = 11;
int EINPROGRESS   = 115;
int EINVAL        = 22;
int ERANGE        = 34;
int EACCES        = 13;
int EADDRINUSE    = 98;
int EADDRNOTAVAIL = 99;
int EAFNOSUPPORT  = 97;
int EAI_MEMORY    = -10;
int EAI_NODATA    = -5;
int SOCK_STREAM   = 1;
int PF_INET       = 2;
int AF_UNSPEC     = 0;
int IPPROTO_TCP   = 6;
int IPPROTO_IP    = 0;
int IPPROTO_UDP   = 17;
int SOL_SOCKET    = 1;
int SO_KEEPALIVE  = 9;
int MSG_NOSIGNAL  = 16384;
int O_NONBLOCK    = 2048;
int F_GETFL       = 3;
int F_SETFL       = 4;
int CLOCK_MONOTONIC     = 1;
int CLOCK_MONOTONIC_RAW = 4;
unsigned int UINT_MAX   = 4294967295U;
unsigned long ULONG_MAX = 18446744073709551615UL;
unsigned short USHRT_MAX = 65535;
int FD_SETSIZE          = 1024;
void FD_SET(int fd, void *set) {
    unsigned long *bits = (unsigned long *)set;
    bits[fd / (8 * sizeof(unsigned long))] |= (1UL << (fd % (8 * sizeof(unsigned long))));
}
int __builtin_constant_p() { return 0; }
SEOF
gcc -c -w /tmp/curl_sysvals_clean.c -o curl_sysvals.o

# Step 4: Build object list
OBJS=""
for f in *.o; do
    case "$f" in
        curl_test.o|curl_stubs.o|curl_sysvals.o|curl_errno_wrap.o) continue ;;
    esac
    OBJS="$OBJS $f"
done

echo ""
echo "=== Linking $(echo $OBJS | wc -w) ZCC objects ==="

gcc -no-pie -o curl_zcc_test \
    curl_test.o curl_stubs.o curl_sysvals.o curl_errno_wrap.o \
    $OBJS \
    -lm 2>&1

if [ $? -eq 0 ] && [ -f curl_zcc_test ]; then
    echo ""
    echo "=========================================="
    echo "           LINK SUCCESS"
    echo "=========================================="
    ls -la curl_zcc_test
    file curl_zcc_test
    echo ""
    echo "=== Running test ==="
    ./curl_zcc_test
else
    echo "LINK FAILED"
fi
