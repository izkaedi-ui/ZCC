import glob

DECLARATIONS = """
// Replaced system headers because ZCC does not support GNU system builtins
extern double sqrt(double);
extern float sqrtf(float);
extern float sinf(float);
extern float cosf(float);
extern float fminf(float, float);
extern float fmaxf(float, float);
extern float fabsf(float);
extern float floorf(float);
extern float fmodf(float, float);
extern float atan2f(float, float);
extern void *malloc(unsigned long size);
extern void free(void *ptr);
extern void *memset(void *s, int c, unsigned long n);
extern void *memcpy(void *dest, const void *src, unsigned long n);
extern int rand(void);
"""

def main():
    for f in glob.glob("exp*.c"):
        with open(f, "r") as file:
            content = file.read()
            
        content = content.replace("#include <math.h>", "")
        content = content.replace("#include <stdlib.h>", "")
        content = content.replace("#include <string.h>", "")
        
        # Put declarations after #include <stdio.h>
        content = content.replace("#include <stdio.h>", "#include <stdio.h>\n" + DECLARATIONS)
        
        with open(f, "w") as file:
            file.write(content)

if __name__ == "__main__":
    main()
