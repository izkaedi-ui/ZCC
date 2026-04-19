/* tests/pp/missing_header_test.c
 * PP-INCLUDE-022 Gate 2: Anti-Hijack
 * This file MUST produce a "include file not found" error, NOT silently
 * inject stddef content. */
#include "definitely_nonexistent_header_abc123.h"

int should_not_compile(void) {
    return 0;
}
