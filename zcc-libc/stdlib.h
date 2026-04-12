typedef unsigned long size_t;
#define NULL 0
void* malloc(size_t);
void free(void*);
void* calloc(size_t, size_t);
void* realloc(void*, size_t);
int atoi(const char*);
void exit(int);
#define RAND_MAX 32767
int rand(void);
