#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
int main() {
    int max1 = MAX(10, 20);
    int min1 = MIN(MAX(1, 2), MAX(3, 4));
    return 0;
}
