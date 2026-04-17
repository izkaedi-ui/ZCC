/* ============================================
 * ZCC stb_image test — PNG-only, no SIMD
 * ============================================ */

/* Disable ALL formats except PNG */
#define STBI_NO_JPEG
#define STBI_NO_BMP
#define STBI_NO_PSD
#define STBI_NO_TGA
#define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM

/* Disable features ZCC can't handle */
#define STBI_NO_SIMD
#define STBI_NO_THREAD_LOCAL
#define STBI_NO_LINEAR
#define STBI_NO_STDIO       /* we'll handle file I/O ourselves */
#define STBI_FAILURE_USERMSG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stb_compat.h"

#ifndef __GNUC__
int STBI__ZFAST_BITS = 9;
#undef assert
void assert(int x) {}
void *STBI_REALLOC(void *p, size_t sz) { return realloc(p, sz); }
int STBI__COMBO(int a, int b) { return a * 8 + b; }
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char **argv) {
    int w, h, channels;
    unsigned char *data;
    FILE *f;
    long fsize;
    unsigned char *buf;
    unsigned long checksum;
    int i;

    if (argc < 2) {
        printf("usage: stb_test <image.png>\n");
        return 1;
    }

    /* Load file into memory ourselves (since STBI_NO_STDIO) */
    f = fopen(argv[1], "rb");
    if (!f) {
        printf("FAIL: cannot open %s\n", argv[1]);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (unsigned char *)malloc(fsize);
    if (!buf) {
        fclose(f);
        printf("FAIL: malloc\n");
        return 1;
    }
    fread(buf, 1, fsize, f);
    fclose(f);

    /* Decode from memory */
    data = stbi_load_from_memory(buf, fsize, &w, &h, &channels, 0);
    free(buf);

    if (!data) {
        printf("FAIL: decode error: %s\n", stbi_failure_reason());
        return 1;
    }

    printf("loaded: %dx%d channels=%d\n", w, h, channels);

    /* dump raw pixel buffer to file */
    f = fopen("pixels.raw", "wb");
    if (f) {
        fwrite(data, 1, w * h * channels, f);
        fclose(f);
    }

    /* simple checksum for quick comparison */
    checksum = 0;
    for (i = 0; i < w * h * channels; i++) {
        checksum = checksum * 31 + data[i];
    }
    printf("checksum=%lu bytes=%d\n", checksum, w * h * channels);

    stbi_image_free(data);
    return 0;
}
