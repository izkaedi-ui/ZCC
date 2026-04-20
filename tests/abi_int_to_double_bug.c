#include <stdio.h>

/* Minimum reproducer for ZCC call-site implicit-conversion bug.
 * ZCC output:   p=0x1234 d=0.000000 x=504   (wrong)
 * GCC output:   p=0x1234 d=504.000000 x=42  (correct)
 *
 * ZCC's caller counts integer args sequentially (%rdi, %rsi, %rdx)
 * without checking callee parameter types. Integer literal 504
 * passed to a `double` parameter does not get converted to double
 * or placed in %xmm0 — it's treated as the next integer arg and
 * shifts subsequent integer args down by one register.
 *
 * Fix lives in part4.c call-emission code. Needs to walk callee's
 * parameter type list and, for each arg, select register class
 * (int vs float) based on PARAMETER type, emit conversion if
 * argument type differs, and use the correct register index for
 * that class.
 *
 * This is the root cause of the Lua 5.4.6 runtime failure at
 * luaL_checkversion_() — macro expands to
 *   luaL_checkversion_(L, 504, LUAL_NUMSIZES)
 * where 504 is int passed to lua_Number (double) parameter.
 */

void f(void *p, double d, unsigned long x) {
    printf("p=%p d=%f x=%lu\n", p, d, x);
}

int main(void) {
    f((void*)0x1234, 504, 42);
    return 0;
}
