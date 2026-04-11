typedef char u8;
typedef unsigned short u16;
typedef unsigned u32;
typedef long long i64;

struct Parse {
    void *db;
    char *zErrMsg;
    void *pVdbe;
    int rc;
    u8 colNamesSet;
    u8 checkSchema;
    u8 nested;
    u8 nTempReg;
    u8 isMultiWrite;
    u8 mayAbort;
    u8 hasCompound;
    u8 okConstFactor;
    u8 disableLookaside;
    u8 prepFlags;
    u8 withinRJSubrtn;
    int nRangeReg;
    int iRangeReg;
    int nErr;
    int nTab;
    int nMem;
};

int test_nMem_offset(struct Parse *p) {
    return p->nMem;
}

int main() {
    struct Parse p;
    int off_nMem = (int)((unsigned long)&p.nMem - (unsigned long)&p);
    return off_nMem;
}
