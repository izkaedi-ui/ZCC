typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef int i32;

struct Op {
    u8 opcode;
    i8 p4type;
    u16 p5;
    int p1;
    int p2;
    int p3;
};

int test_p1_offset(struct Op *op) {
    return op->p1;
}
int test_p3_offset(struct Op *op) {
    return op->p3;
}

int main(){
    struct Op o;
    int p1_off = (int)((unsigned long)&o.p1 - (unsigned long)&o);
    int p3_off = (int)((unsigned long)&o.p3 - (unsigned long)&o);
    return p1_off * 100 + p3_off;
}
