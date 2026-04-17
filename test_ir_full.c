int printf(const char *fmt, ...);

int main() {
    /* Float ops */
    double a = 3.14;
    double b = 2.0;
    double fadd = a + b;
    double fsub = a - b;
    double fmul = a * b;
    double fdiv = a / b;

    /* Logic ops */
    int x = 5, y = 0;
    int land = x && y;
    int lor = x || y;

    /* Compound assign */
    int c = 10;
    c += 5;
    c -= 2;
    c *= 3;

    /* Pre/post inc/dec */
    int d = 100;
    ++d;
    d++;
    --d;
    d--;

    /* Ternary */
    int t = (a > b) ? 1 : 0;

    printf("fadd=%.2f fsub=%.2f fmul=%.2f fdiv=%.2f\n", fadd, fsub, fmul, fdiv);
    printf("land=%d lor=%d c=%d d=%d t=%d\n", land, lor, c, d, t);
    return 0;
}
