double exp(double x);
#include <stdio.h>
#include <math.h>
#include "kraken_v2_2_weights_double.h"

static double relu(double x) { return (x > 0.0) ? x : 0.0; }
static double sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }

static int classify_binary(const double *f, const double *w1, const double *b1, const double *w2, const double *b2) {
    double h[32]; int i,j;
    for(i=0;i<32;i++){double s=b1[i]; for(j=0;j<32;j++) s+=f[j]*w1[i*32+j]; h[i]=relu(s);}
    double l=b2[0]; for(i=0;i<32;i++) l+=h[i]*w2[i];
    return (sigmoid(l)>=0.5)?1:0;
}

typedef struct{int reentrancy,overflow,tx_origin,delegatecall,flash_loan,selfdestruct,timestamp,unchecked_call;}KrakenResult;

KrakenResult kraken_infer(const double f[32]){
    KrakenResult r;
    r.reentrancy=classify_binary(f,reentrancy_w1,reentrancy_b1,reentrancy_w2,reentrancy_b2);
    r.overflow=classify_binary(f,overflow_w1,overflow_b1,overflow_w2,overflow_b2);
    r.tx_origin=classify_binary(f,tx_origin_w1,tx_origin_b1,tx_origin_w2,tx_origin_b2);
    r.delegatecall=classify_binary(f,delegatecall_w1,delegatecall_b1,delegatecall_w2,delegatecall_b2);
    r.flash_loan=classify_binary(f,flash_loan_w1,flash_loan_b1,flash_loan_w2,flash_loan_b2);
    r.selfdestruct=classify_binary(f,selfdestruct_w1,selfdestruct_b1,selfdestruct_w2,selfdestruct_b2);
    r.timestamp=classify_binary(f,timestamp_w1,timestamp_b1,timestamp_w2,timestamp_b2);
    r.unchecked_call=classify_binary(f,unchecked_call_w1,unchecked_call_b1,unchecked_call_w2,unchecked_call_b2);
    return r;
}

int main(void){
    double a[32]={4.8266,1.9703,1.7372,4.7714,0.2801,-2.3537,4.2806,2.7146,0.7466,-0.1666,-2.4130,1.6129,-0.2767,-4.2102,2.0987,-0.0561,2.6111,2.1119,0.1095,-4.1588,-3.3719,-4.2868,4.5083,-3.2871,1.8991,-4.6554,-2.7753,0.2341,0.2130,3.2073,2.0964,3.5508};
    KrakenResult r=kraken_infer(a);
    printf("KRAKEN v2.2 DOUBLE WORKAROUND\n");
    printf("Reentrancy:   %s\n",r.reentrancy?"VULNERABLE":"SAFE");
    printf("Overflow:     %s\n",r.overflow?"VULNERABLE":"SAFE");
    printf("TX Origin:    %s\n",r.tx_origin?"VULNERABLE":"SAFE");
    printf("Delegatecall: %s\n",r.delegatecall?"VULNERABLE":"SAFE");
    printf("Flash Loan:   %s\n",r.flash_loan?"VULNERABLE":"SAFE");
    printf("Selfdestruct: %s\n",r.selfdestruct?"VULNERABLE":"SAFE");
    printf("Timestamp:    %s\n",r.timestamp?"VULNERABLE":"SAFE");
    printf("Unchecked:    %s\n",r.unchecked_call?"VULNERABLE":"SAFE");
    test_clean(); return 0;
}

void test_clean(void) {
    double clean[32] = {
        0.02, 0.01, 0.03, 0.02, 0.01, 0.02, 0.01, 0.03,
        0.02, 0.01, 0.02, 0.01, 0.03, 0.02, 0.01, 0.02,
        0.01, 0.03, 0.02, 0.01, 0.02, 0.01, 0.03, 0.02,
        0.01, 0.02, 0.01, 0.03, 0.02, 0.01, 0.02, 0.01
    };
    printf("\nCLEAN CONTRACT TEST:\n");
    KrakenResult r = kraken_infer(clean);
    printf("Reentrancy:   %s\n", r.reentrancy ? "VULNERABLE" : "SAFE");
    printf("Overflow:     %s\n", r.overflow ? "VULNERABLE" : "SAFE");
    printf("TX Origin:    %s\n", r.tx_origin ? "VULNERABLE" : "SAFE");
    printf("Delegatecall: %s\n", r.delegatecall ? "VULNERABLE" : "SAFE");
    printf("Flash Loan:   %s\n", r.flash_loan ? "VULNERABLE" : "SAFE");
    printf("Selfdestruct: %s\n", r.selfdestruct ? "VULNERABLE" : "SAFE");
    printf("Timestamp:    %s\n", r.timestamp ? "VULNERABLE" : "SAFE");
    printf("Unchecked:    %s\n", r.unchecked_call ? "VULNERABLE" : "SAFE");
}
