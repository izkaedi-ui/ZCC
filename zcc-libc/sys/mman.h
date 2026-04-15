#define PROT_READ 1
#define PROT_WRITE 2
#define MAP_SHARED 1
#define MAP_PRIVATE 2
#define MAP_FAILED ((void*)-1)
void* mmap(void*, unsigned long, int, int, int, long);
int munmap(void*, unsigned long);
