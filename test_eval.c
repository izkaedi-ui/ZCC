int printf(const char *fmt, ...);
int main() {
    int x1 = (int)(0x80000000u);
    int x2 = 575 - x1;
    int x3 = x2 < -205;
    int x_full = (int)((((575) - (((int)(0x80000000u))))) < (-205));
    printf("x1=%d x2=%d x3=%d full=%d\n", x1, x2, x3, x_full);
    return 0;
}
