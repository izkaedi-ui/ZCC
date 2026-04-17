#define MACRO(a, b) a + b
#define NESTED MACRO(1, 2)
#define EMPTY() 1
int main() {
    int x = MACRO(3, 4);
    int y = NESTED;
    int z = EMPTY();
    return 0;
}
