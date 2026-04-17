typedef struct S { void *ptr; } S;
void* func(S *s) { return s->ptr; }
