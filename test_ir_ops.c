int printf(const char *fmt, ...);

int main() {
    int a = 10, b = 3;
    int add = a + b;
    int sub = a - b;
    int mul = a * b;
    int div = a / b;
    int mod = a % b;
    int band = a & b;
    int bor = a | b;
    int bxor = a ^ b;
    int bnot = ~a;
    int shl = a << 1;
    int shr = a >> 1;
    int neg = -a;
    int eq = a == b;
    int ne = a != b;
    int lt = a < b;
    int le = a <= b;
    int gt = a > b;
    int ge = a >= b;
    printf("add=%d sub=%d mul=%d div=%d mod=%d\n", add, sub, mul, div, mod);
    printf("band=%d bor=%d bxor=%d bnot=%d shl=%d shr=%d neg=%d\n", band, bor, bxor, bnot, shl, shr, neg);
    printf("eq=%d ne=%d lt=%d le=%d gt=%d ge=%d\n", eq, ne, lt, le, gt, ge);
    return 0;
}
