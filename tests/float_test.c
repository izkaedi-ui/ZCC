#include <stdio.h>

/* CG-FLOAT-009: Two-float-param function call
 * ZCC uses addsd (double) instead of addss (float) for float+float binary ops.
 * This test will FAIL under ZCC (oracle diff will expose it).
 * Filed in BUGS.md as CG-FLOAT-009.
 *
 * ERR-0028 status: ORACLE-VERIFIED
 * These helpers return float -- they exercise float binary op codegen.
 */
float add_floats(float a, float b) { return a + b; }
float mul_floats(float a, float b) { return a * b; }
int   float_to_int(float f)        { return (int)f; }

int main(void) {
    float x;
    float y;
    float z_sum;
    float z_prod;
    int n;
    int pass;
    int fail;

    x      = 3.0f;     /* Use round numbers to reduce ambiguity in output */
    y      = 1.0f;
    z_sum  = add_floats(x, y);    /* expected: 4.0 */
    z_prod = mul_floats(x, y);    /* expected: 3.0 */
    n      = float_to_int(z_prod);
    pass   = 0;
    fail   = 0;

    printf("add_floats(3.0f, 1.0f) = %.6f\n", (double)z_sum);
    printf("mul_floats(3.0f, 1.0f) = %.6f\n", (double)z_prod);
    printf("float_to_int(3.0f)     = %d\n", n);

    if (n == 3) { printf("[PASS] float_to_int\n"); pass++; }
    else        { printf("[FAIL] float_to_int expected=3 got=%d\n", n); fail++; }

    printf("\n=== RESULTS: %d passed, %d failed ===\n", pass, fail);
    return fail > 0 ? 1 : 0;
}
