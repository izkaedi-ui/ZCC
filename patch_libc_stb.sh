#!/bin/sh
cd /mnt/h/__DOWNLOADS/selforglinux/zcc-libc

# 1. Add SEEK_SET/SEEK_CUR/SEEK_END + missing stdio functions if not present
if ! grep -q SEEK_SET stdio.h 2>/dev/null; then
    cat >> stdio.h << 'EOF'

/* seek whence values */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
EOF
    echo "[PATCH] Added SEEK_SET/CUR/END to stdio.h"
fi

if ! grep -q fgetc stdio.h 2>/dev/null; then
    cat >> stdio.h << 'EOF'

/* character I/O */
int fgetc(FILE *stream);
int ungetc(int c, FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
int getc(FILE *stream);
EOF
    echo "[PATCH] Added fgetc/ungetc/feof/ferror to stdio.h"
fi

# 2. Add SHRT_MAX/SHRT_MIN to limits.h
if ! grep -q SHRT_MAX limits.h 2>/dev/null; then
    cat >> limits.h << 'EOF'

#define SHRT_MIN  (-32768)
#define SHRT_MAX  32767
#define USHRT_MAX 65535
EOF
    echo "[PATCH] Added SHRT_MAX/MIN to limits.h"
fi

# 3. Create assert.h if missing
if [ ! -f assert.h ]; then
    cat > assert.h << 'EOF'
#ifndef _ASSERT_H
#define _ASSERT_H

void abort(void);

#ifdef NDEBUG
#define assert(expr) ((void)0)
#else
#define assert(expr) ((void)((expr) || (__assert_fail(#expr, __FILE__, __LINE__), 0)))
#endif

static void __assert_fail(const char *expr, const char *file, int line) {
    /* minimal assert — just abort */
    abort();
}

#endif
EOF
    echo "[PATCH] Created assert.h"
fi

# 4. Ensure limits.h exists
if [ ! -f limits.h ]; then
    cat > limits.h << 'EOF'
#ifndef _LIMITS_H
#define _LIMITS_H

#define CHAR_BIT   8
#define SCHAR_MIN  (-128)
#define SCHAR_MAX  127
#define UCHAR_MAX  255
#define CHAR_MIN   (-128)
#define CHAR_MAX   127
#define SHRT_MIN   (-32768)
#define SHRT_MAX   32767
#define USHRT_MAX  65535
#define INT_MIN    (-2147483647-1)
#define INT_MAX    2147483647
#define UINT_MAX   4294967295U
#define LONG_MIN   (-9223372036854775807L-1)
#define LONG_MAX   9223372036854775807L
#define ULONG_MAX  18446744073709551615UL
#define LLONG_MIN  (-9223372036854775807LL-1)
#define LLONG_MAX  9223372036854775807LL
#define ULLONG_MAX 18446744073709551615ULL

#endif
EOF
    echo "[PATCH] Created limits.h"
fi

echo ""
echo "=== Libc patches complete ==="
ls -la stdio.h limits.h assert.h 2>/dev/null
