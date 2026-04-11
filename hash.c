int printf(const char *fmt, ...);
void *malloc(long size);

typedef struct Entry {
    int key;
    int val;
    struct Entry *next;
} Entry;

typedef struct HashMap {
    Entry **buckets;
    int size;
} HashMap;

void *my_malloc(long n) { return malloc(n); }

HashMap *hashmap_new(int size) {
    int i;
    HashMap *m = (HashMap *)my_malloc(sizeof(HashMap));
    m->size = size;
    m->buckets = (Entry **)my_malloc(sizeof(Entry *) * size);
    for (i = 0; i < size; i++) m->buckets[i] = (Entry *)0;
    return m;
}

int hash(int key, int size) {
    int h = key % size;
    if (h < 0) h = -h;
    return h;
}

void hashmap_put(HashMap *m, int key, int val) {
    int h = hash(key, m->size);
    Entry *e = m->buckets[h];
    while (e) {
        if (e->key == key) { e->val = val; return; }
        e = e->next;
    }
    Entry *n = (Entry *)my_malloc(sizeof(Entry));
    n->key = key;
    n->val = val;
    n->next = m->buckets[h];
    m->buckets[h] = n;
}

int hashmap_get(HashMap *m, int key, int *found) {
    int h = hash(key, m->size);
    Entry *e = m->buckets[h];
    while (e) {
        if (e->key == key) { *found = 1; return e->val; }
        e = e->next;
    }
    *found = 0;
    return 0;
}

int main() {
    int found;
    HashMap *m = hashmap_new(16);
    hashmap_put(m, 1, 100);
    hashmap_put(m, 2, 200);
    hashmap_put(m, 3, 300);
    hashmap_put(m, 17, 999);
    hashmap_put(m, 2, 250);
    printf("get(1) = %d\n", hashmap_get(m, 1, &found));
    printf("get(2) = %d\n", hashmap_get(m, 2, &found));
    printf("get(3) = %d\n", hashmap_get(m, 3, &found));
    printf("get(17) = %d\n", hashmap_get(m, 17, &found));
    hashmap_get(m, 99, &found);
    printf("get(99) found=%d\n", found);
    return 0;
}
