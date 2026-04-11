int printf(char *fmt, ...);
int log2_of(long long val) {
    int n;
    n = 0;
    while(val > 1) {
        val = val >> 1;
        n = n + 1;
    }
    return n;
}
int main() {
    int t = 1024;
    printf("%d\n", log2_of(t));
    return 0;
}
