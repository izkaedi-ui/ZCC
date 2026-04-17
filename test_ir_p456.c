int printf(const char *fmt, ...);

struct Point { int x; int y; };

int add_xy(struct Point *p) {
    return p->x + p->y;
}

int main() {
    /* ND_ADDR + ND_DEREF */
    int val = 42;
    int *ptr = &val;
    int deref = *ptr;

    /* ND_MEMBER */
    struct Point pt;
    pt.x = 10;
    pt.y = 20;
    int sum = add_xy(&pt);

    /* ND_TERNARY */
    int tern = (val > 10) ? 100 : 200;

    /* ND_COMMA_EXPR */
    int comma = (1, 2, 3);

    /* ND_CALL with args */
    printf("deref=%d sum=%d tern=%d comma=%d\n", deref, sum, tern, comma);
    return 0;
}
