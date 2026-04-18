#include <stdio.h>
static double pos = 1.5;
static double neg = -1.5;
static double nzero = -0.0;
static double pzero = 0.0;
static float posf = 1.5f;
static float negf = -1.5f;
static double non_repr[3] = {0.1, 0.3, 0.7};
static double arr_neg[3] = {-0.0, -1.0, -1e-308};
int main(void) {
    printf("pos=%a neg=%a nzero=%a pzero=%a\n", pos, neg, nzero, pzero);
    printf("posf=%a negf=%a\n", (double)posf, (double)negf);
    printf("non_repr=%a %a %a\n", non_repr[0], non_repr[1], non_repr[2]);
    printf("arr_neg=%a %a %a\n", arr_neg[0], arr_neg[1], arr_neg[2]);
    return 0;
}
