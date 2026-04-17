#include <stdio.h>
#include <math.h>
#include "kraken_v2_2_weights_double.h"

static double relu(double x) { return (x > 0.0) ? x : 0.0; }
static double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

static double classify_debug(const char *name, const double *f, const double *w1, const double *b1, const double *w2, const double *b2) {
    double h[32]; int i,j;
    for(i=0;i<32;i++){double s=b1[i]; for(j=0;j<32;j++) s+=f[j]*w1[i*32+j]; h[i]=relu(s);}
    double logit=b2[0]; for(i=0;i<32;i++) logit+=h[i]*w2[i];
    double prob=sigmoid(logit);
    printf("  %s: logit=%.4f prob=%.4f -> %s\n", name, logit, prob, (prob>=0.5)?"VULN":"SAFE");
    return prob;
}

int main(void){
    double clean[32] = {
        0.02, 0.01, 0.03, 0.02, 0.01, 0.02, 0.01, 0.03,
        0.02, 0.01, 0.02, 0.01, 0.03, 0.02, 0.01, 0.02,
        0.01, 0.03, 0.02, 0.01, 0.02, 0.01, 0.03, 0.02,
        0.01, 0.02, 0.01, 0.03, 0.02, 0.01, 0.02, 0.01
    };
    
    printf("CLEAN CONTRACT DEBUG:\n");
    classify_debug("Reentrancy  ", clean, reentrancy_w1, reentrancy_b1, reentrancy_w2, reentrancy_b2);
    classify_debug("Overflow    ", clean, overflow_w1, overflow_b1, overflow_w2, overflow_b2);
    classify_debug("TX Origin   ", clean, tx_origin_w1, tx_origin_b1, tx_origin_w2, tx_origin_b2);
    classify_debug("Delegatecall", clean, delegatecall_w1, delegatecall_b1, delegatecall_w2, delegatecall_b2);
    
    return 0;
}
