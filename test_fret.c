#include <stdio.h>

double get_pi(void) { return 3.14; }
double add_doubles(double a, double b) { return a + b; }

int main() {
    double x = get_pi();
    printf("get_pi=%.2f\n", x);

    double y = add_doubles(1.5, 2.5);
    printf("add=%.2f\n", y);

    // This is what SQLite does internally
    double arr[1] = {3.14};
    double *p = arr;
    double val = *p;
    printf("deref=%.2f\n", val);
    return 0;
}
