typedef unsigned long size_t;
#define NULL 0
void* memcpy(void*, const void*, size_t);
void* memset(void*, int, size_t);
int strcmp(const char*, const char*);
int strcasecmp(const char*, const char*);
int strncasecmp(const char*, const char*, size_t);
int strncmp(const char*, const char*, size_t);
size_t strlen(const char*);
char* strcpy(char*, const char*);
char* strcat(char*, const char*);
char* strncpy(char*, const char*, size_t);
char* strdup(const char*);
