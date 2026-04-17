/* stb_compat.h — fills libc gaps for ZCC */
#ifndef STB_COMPAT_H
#define STB_COMPAT_H

/* stdio extras */
#ifndef SEEK_SET
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#endif

int fgetc(FILE *stream);
int ungetc(int c, FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
int getc(FILE *stream);

/* limits */
#ifndef SHRT_MAX
#define SHRT_MIN  (-32768)
#define SHRT_MAX  32767
#define USHRT_MAX 65535
#endif

#ifndef INT_MAX
#define INT_MIN   (-2147483647-1)
#define INT_MAX   2147483647
#define UINT_MAX  4294967295U
#endif

#ifndef LONG_MAX
#define LONG_MIN  (-9223372036854775807L-1)
#define LONG_MAX  9223372036854775807L
#define ULONG_MAX 18446744073709551615UL
#endif

/* assert */
#ifndef assert
#define assert(expr) ((void)0)
#endif

/* pow — needed by stb_image for gamma correction */
double pow(double base, double exp);
double ldexp(double x, int exp);

#endif /* STB_COMPAT_H */
