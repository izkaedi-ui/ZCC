#include <stdio.h>

/* constant_fold_target: all operations on compile-time constants */
int fold_test(void) {
    int a = 7;
    int b = 13;
    int c = a * b + 3;    /* should fold to 94 at IR level */
    int d = c / 2;        /* should fold to 47 */
    return d;             /* DCE eliminates a, b, c */
}

/* licm_target: loop-invariant computation inside hot loop */
int licm_test(int n) {
    int sum = 0;
    int i = 0;
    while (i < n) {
        int base = 42 * 17;  /* LICM should hoist this */
        sum = sum + base + i;
        i = i + 1;
    }
    return sum;
}

/* dce_target: dead assignments that should be eliminated */
int dce_test(int x) {
    int dead1 = x * 3;    /* never used -> DCE kills */
    int dead2 = x + 7;    /* never used -> DCE kills */
    int live = x * 2;
    return live;
}

/* regpressure_target: 8+ simultaneous live values */
int pressure_test(int a, int b, int c, int d) {
    int e = a + b;
    int f = c + d;
    int g = e * f;
    int h = a - d;
    int i = b * c;
    int j = g + h;        /* 6+ values live here */
    int k = i - j;
    int m = e + f + g + h + i + j + k;  /* all live */
    return m;
}

/* escape_target: local allocation that doesn't escape */
int escape_test(void) {
    int arr[4];           /* should be promoted to registers */
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    return arr[0] + arr[1] + arr[2] + arr[3];  /* 100 */
}

int main(void) {
    int fail = 0;
    if (fold_test() != 47)      { printf("FAIL fold\n"); fail = 1; }
    if (licm_test(100) != 76350) { printf("FAIL licm\n"); fail = 1; }
    if (dce_test(5) != 10)      { printf("FAIL dce\n"); fail = 1; }
    if (pressure_test(1,2,3,4) != 40) { printf("FAIL pressure\n"); fail = 1; }
    if (escape_test() != 100)   { printf("FAIL escape\n"); fail = 1; }
    if (!fail) printf("ALL PASS\n");
    return fail;
}
