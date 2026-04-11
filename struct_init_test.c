#include <stdio.h>
typedef unsigned char u8;
typedef int i32;

/* Exact replica of SQLite SelectDest */
struct SelectDest {
    u8 eDest;
    int iSDParm;
    int iSDParm2;
    int iSdst;
    int nSdst;
    char *zAffSdst;
    void *pOrderBy;
};

int main() {
    struct SelectDest dest = {9, 0, 0, 0, 0, 0, 0};
    printf("eDest=%d iSDParm=%d iSDParm2=%d iSdst=%d nSdst=%d\n",
           (int)dest.eDest, dest.iSDParm, dest.iSDParm2, dest.iSdst, dest.nSdst);
    
    if (dest.eDest == 9 && dest.iSdst == 0) {
        printf("PASS: eDest=9 iSdst=0 (was the root cause of SQLite SIGSEGV)\n");
        return 0;
    } else {
        printf("FAIL\n");
        return 1;
    }
}
