int printf(const char *fmt, ...);

int my_strlen(char *s) {
    int n = 0;
    while (s[n]) n++;
    return n;
}

void my_reverse(char *s) {
    int i, j;
    char tmp;
    i = 0;
    j = my_strlen(s) - 1;
    while (i < j) {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}

void my_strcpy(char *dst, char *src) {
    int i = 0;
    while (src[i]) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = 0;
}

int main() {
    char buf[32];
    my_strcpy(buf, "ZKAEDI");
    printf("original: %s
", buf);
    printf("length: %d
", my_strlen(buf));
    my_reverse(buf);
    printf("reversed: %s
", buf);
    return 0;
}
