int printf(const char *fmt, ...);

char heap[4096];
int heap_pos;

void heap_init() { heap_pos = 0; }

void *alloc(int size) {
    char *ptr = heap + heap_pos;
    heap_pos = heap_pos + size;
    return (void *)ptr;
}

int main() {
    int i;
    heap_init();
    int *a = (int *)alloc(4 * 4);
    int *b = (int *)alloc(4 * 4);
    int *c = (int *)alloc(4 * 4);
    for (i = 0; i < 4; i++) a[i] = (i + 1) * 10;
    for (i = 0; i < 4; i++) b[i] = (i + 1) * 20;
    for (i = 0; i < 4; i++) c[i] = a[i] + b[i];
    printf("a: %d %d %d %d\n", a[0], a[1], a[2], a[3]);
    printf("b: %d %d %d %d\n", b[0], b[1], b[2], b[3]);
    printf("c: %d %d %d %d\n", c[0], c[1], c[2], c[3]);
    printf("heap used: %d bytes\n", heap_pos);
    return 0;
}
