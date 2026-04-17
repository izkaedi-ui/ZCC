import glob, re

def main():
    files = glob.glob("exp*.c")
    externs = """
extern float sinf(float);
extern float cosf(float);
extern float sqrtf(float);
extern float fabsf(float);
extern float floorf(float);
extern float fmaxf(float, float);
extern float fminf(float, float);
extern float expf(float);
extern float powf(float, float);
extern int rand(void);
extern void *malloc(unsigned long size);
extern void free(void *ptr);
extern void *memset(void *s, int c, unsigned long n);

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
"""
    for f in files:
        with open(f, "r") as r:
            txt = r.read()
            
        txt = re.sub(r'#include\s+<math\.h>', '', txt)
        txt = re.sub(r'#include\s+<stdlib\.h>', '', txt)
        txt = re.sub(r'#include\s+<string\.h>', '', txt)
        
        # Insert externs near the top
        if "extern float sinf" not in txt:
            txt = txt.replace("#include <stdio.h>", "#include <stdio.h>\n" + externs)
            
        with open(f, "w") as w:
            w.write(txt)

if __name__ == "__main__":
    main()
