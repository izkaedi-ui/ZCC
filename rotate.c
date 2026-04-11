int printf(const char *fmt, ...);
unsigned int rotl(unsigned int x, int n) {
    return (x << n) | (x >> (32 - n));
}
unsigned int rotr(unsigned int x, int n) {
    return (x >> n) | (x << (32 - n));
}
int main() {
    printf("%u\n", rotl(0x12345678, 4));
    printf("%u\n", rotr(0x12345678, 4));
    return 0;
}
