/* ZCC Dream Benchmark Workload — DO NOT EDIT */
int printf(const char *fmt, ...);
void *malloc(long size);
void free(void *ptr);

long fibonacci(int n) {
    if (n <= 1) return n;
    long a = 0, b = 1, t;
    int i;
    for (i = 2; i <= n; i++) {
        t = a + b;
        a = b;
        b = t;
    }
    return b;
}

int bubble_sort(int *arr, int n) {
    int i, j, tmp, swaps = 0;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
                swaps++;
            }
        }
    }
    return swaps;
}

long hash_string(const char *s) {
    long h = 5381;
    while (*s) {
        h = ((h << 5) + h) + *s;
        s++;
    }
    return h;
}

int matrix_mul(int n) {
    int A[8][8];
    int B[8][8];
    int C[8][8];
    int i, j, k;
    int sum = 0;
    for (i = 0; i < n && i < 8; i++)
        for (j = 0; j < n && j < 8; j++) {
            A[i][j] = i * n + j;
            B[i][j] = j * n + i;
        }
    for (i = 0; i < n && i < 8; i++)
        for (j = 0; j < n && j < 8; j++) {
            C[i][j] = 0;
            for (k = 0; k < n && k < 8; k++)
                C[i][j] += A[i][k] * B[k][j];
            sum += C[i][j];
        }
    return sum;
}

int main(void) {
    long f = fibonacci(40);
    int arr[10];
    int i;
    for (i = 0; i < 10; i++) arr[i] = 10 - i;
    int sw = bubble_sort(arr, 10);
    long h = hash_string("ZCC Oneirogenesis Dreams");
    int m = matrix_mul(8);
    printf("DREAM_BENCHMARK: fib=%ld swaps=%d hash=%ld matrix=%d\n", f, sw, h, m);
    return 0;
}
