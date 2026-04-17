// doom_shims.c
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
int doom_open(const char *path, int flags, ...) { 
    printf("ZCC_DEBUG: doom_open called with path: %s\n", path);
    return open(path, flags, 0666); 
}
int doom_access(const char *path, int mode) {
    if (path && (unsigned long long)path > 0x1000) {
        printf("ZCC_DEBUG: access called on path: %s\n", path);
    } else {
        printf("ZCC_DEBUG: access called on invalid pointer: %p\n", path);
    }
    return access(path, mode);
}
void zcc_close(int fd) { close(fd); }
void *alloca(size_t size) { return malloc(size); }

extern int XDefaultScreen(void*);
extern unsigned long XRootWindow(void*, int);

int DefaultScreen(void* d) {
    return XDefaultScreen(d);
}

unsigned long RootWindow(void* d, int s) {
    return XRootWindow(d, s);
}

void* DefaultVisual(void* d, int s) {
    extern void* XDefaultVisual(void*, int);
    return XDefaultVisual(d, s);
}
