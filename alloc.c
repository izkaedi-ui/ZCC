int printf(const char *fmt, ...);

char heap[4096];
int heap_pos;

void *bump_alloc(int size) {
    void *ptr = (void *)(heap + heap_pos);
    heap_pos = heap_pos + size;
    return ptr;
}

typedef struct Block {
    int size;
    int free;
    struct Block *next;
} Block;

Block *freelist;

void allocator_init() {
    freelist = (Block *)heap;
    freelist->size = 4096 - sizeof(Block);
    freelist->free = 1;
    freelist->next = (Block *)0;
}

void *my_alloc(int size) {
    Block *b = freelist;
    while (b) {
        if (b->free && b->size >= size) {
            b->free = 0;
            return (void *)(b + 1);
        }
        b = b->next;
    }
    return (void *)0;
}

void my_free(void *ptr) {
    Block *b = (Block *)ptr - 1;
    b->free = 1;
}

int main() {
    int i;
    int *a;
    int *b;
    int *c;
    allocator_init();
    a = (int *)my_alloc(sizeof(int) * 4);
    b = (int *)my_alloc(sizeof(int) * 4);
    c = (int *)my_alloc(sizeof(int) * 4);
    for (i = 0; i < 4; i++) a[i] = i * 10;
    for (i = 0; i < 4; i++) b[i] = i * 20;
    for (i = 0; i < 4; i++) c[i] = i * 30;
    printf("a: %d %d %d %d\n", a[0], a[1], a[2], a[3]);
    printf("b: %d %d %d %d\n", b[0], b[1], b[2], b[3]);
    printf("c: %d %d %d %d\n", c[0], c[1], c[2], c[3]);
    my_free(b);
    int *d = (int *)my_alloc(sizeof(int) * 4);
    d[0] = 99;
    printf("d[0] after free+realloc: %d\n", d[0]);
    return 0;
}
