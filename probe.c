int cc_alloc(int n) { return n; }
void* some_small_func(int x) { return (void*)(long)cc_alloc(x); }
int main() { some_small_func(8); return 0; }
