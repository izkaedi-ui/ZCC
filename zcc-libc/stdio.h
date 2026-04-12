typedef unsigned long size_t;
#define NULL 0
#define EOF (-1)
typedef struct _FILE FILE;
extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;
FILE* fopen(const char*, const char*);
int fclose(FILE*);
size_t fread(void*, size_t, size_t, FILE*);
size_t fwrite(const void*, size_t, size_t, FILE*);
int fprintf(FILE*, const char*, ...);
int printf(const char*, ...);
int sprintf(char*, const char*, ...);
int snprintf(char*, size_t, const char*, ...);
int vfprintf(FILE*, const char*, void*);
int fflush(FILE*);
int fseek(FILE*, long, int);
long ftell(FILE*);
int remove(const char*);
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
