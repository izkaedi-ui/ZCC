int printf(const char *fmt, ...);

char *input;
int pos;

int parse_expr();

int parse_num() {
    int val = 0;
    while (input[pos] >= 48 && input[pos] <= 57) {
        val = val * 10 + (input[pos] - 48);
        pos++;
    }
    return val;
}

int parse_factor() {
    if (input[pos] == 40) {
        pos++;
        int val = parse_expr();
        pos++;
        return val;
    }
    return parse_num();
}

int parse_term() {
    int val = parse_factor();
    while (input[pos] == 42 || input[pos] == 47) {
        char op = input[pos++];
        int rhs = parse_factor();
        if (op == 42) val = val * rhs;
        else val = val / rhs;
    }
    return val;
}

int parse_expr() {
    int val = parse_term();
    while (input[pos] == 43 || input[pos] == 45) {
        char op = input[pos++];
        int rhs = parse_term();
        if (op == 43) val = val + rhs;
        else val = val - rhs;
    }
    return val;
}

int main() {
    char buf1[32];
    char buf2[32];
    char buf3[32];
    buf1[0]=49;buf1[1]=43;buf1[2]=50;buf1[3]=42;buf1[4]=51;buf1[5]=0;
    buf2[0]=40;buf2[1]=50;buf2[2]=43;buf2[3]=51;buf2[4]=41;buf2[5]=42;buf2[6]=52;buf2[7]=0;
    buf3[0]=49;buf3[1]=48;buf3[2]=43;buf3[3]=50;buf3[4]=42;buf3[5]=51;buf3[6]=45;buf3[7]=52;buf3[8]=0;
    input = buf1; pos = 0;
    printf("1+2*3 = %d\n", parse_expr());
    input = buf2; pos = 0;
    printf("(2+3)*4 = %d\n", parse_expr());
    input = buf3; pos = 0;
    printf("10+2*3-4 = %d\n", parse_expr());
    return 0;
}
