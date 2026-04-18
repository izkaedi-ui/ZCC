#include <stdio.h>
typedef struct { double a; double b; double c; } AllDouble;
static AllDouble arr[5] = {
    {1.0, 2.0, 3.0},
    {4.0, 5.0, 6.0},
    {7.0, 8.0, 9.0},
    {10.0, 11.0, 12.0},
    {13.0, 14.0, 15.0}
};

int main(void) {
    int i;
    for (i = 0; i < 5; i++)
        printf("%d: a=%.1f b=%.1f c=%.1f\n", i, arr[i].a, arr[i].b, arr[i].c);
    return 0;
}
