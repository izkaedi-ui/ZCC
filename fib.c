int printf(const char *fmt, ...);

int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int i;
    printf("Fibonacci:
");
    for (i = 0; i < 10; i++) {
        printf("fib(%d) = %d
", i, fibonacci(i));
    }
    printf("Factorial:
");
    for (i = 1; i <= 10; i++) {
        printf("%d! = %d
", i, factorial(i));
    }
    return 0;
}
