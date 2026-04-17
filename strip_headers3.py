import glob, re

def main():
    files = glob.glob("exp*.c")
    externs = """
#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
extern float expf(float);
extern int rand(void);
extern void *malloc(unsigned long size);
extern void free(void *ptr);
extern void *memset(void *s, int c, unsigned long n);
"""
    for f in files:
        with open(f, "r") as r:
            txt = r.read()
            
        txt = txt.replace("#include <stdio.h>", "#include <stdio.h>\n" + externs)
            
        with open(f, "w") as w:
            w.write(txt)

if __name__ == "__main__":
    main()
