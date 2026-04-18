#include <stdio.h>

static double single = 42.5;
static double arr[3] = {1.5, 2.5, 3.5};
static int iarr[3] = {10, 20, 30};
static int isingle = 99;

typedef struct { double x; } OneField;
static OneField one_struct = {7.5};

typedef struct { int a; int b; } IntStruct;
static IntStruct istruct = {11, 22};

int main(void) {
    printf("single=%f arr[0]=%f arr[1]=%f arr[2]=%f\n", single, arr[0], arr[1], arr[2]);
    printf("one_struct.x=%f\n", one_struct.x);
    printf("isingle=%d iarr[0]=%d iarr[1]=%d iarr[2]=%d\n", isingle, iarr[0], iarr[1], iarr[2]);
    printf("istruct.a=%d istruct.b=%d\n", istruct.a, istruct.b);
    return 0;
}
