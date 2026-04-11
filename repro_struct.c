struct Test {
    int a;
    union {
        int b;
        int c;
    };
};

int main() {
    struct Test t;
    t.a = 1;
    t.b = 2;
    return t.b;
}
