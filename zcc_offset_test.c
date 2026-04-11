typedef unsigned Bool;
struct VdbeCursor {
    unsigned char eCurType;
    signed char iDb;
    unsigned char nullRow;
    unsigned char deferredMoveto;
    unsigned char isTable;
    Bool isEphemeral:1;
    Bool useRandomRowid:1;
    Bool isOrdered:1;
    Bool noReuse:1;
    Bool colCache:1;
    unsigned short seekHit;
};

int test_offset() {
    struct VdbeCursor c;
    return (int)((unsigned long)&c.seekHit - (unsigned long)&c);
}
int main() {
    return test_offset();
}
