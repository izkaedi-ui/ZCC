int printf(const char *fmt, ...);
void *malloc(long size);

int main() {
    char prog[64];
    char mem[256];
    int i, pc, dp;

    prog[0]=43;prog[1]=43;prog[2]=43;prog[3]=43;prog[4]=43;
    prog[5]=43;prog[6]=43;prog[7]=43;prog[8]=91;prog[9]=45;
    prog[10]=62;prog[11]=43;prog[12]=43;prog[13]=43;prog[14]=43;
    prog[15]=43;prog[16]=43;prog[17]=43;prog[18]=43;prog[19]=60;
    prog[20]=93;prog[21]=62;prog[22]=46;prog[23]=0;

    for (i = 0; i < 256; i++) mem[i] = 0;
    pc = 0; dp = 0;

    while (prog[pc]) {
        char c = prog[pc];
        if (c == 43) { mem[dp]++; }
        else if (c == 45) { mem[dp]--; }
        else if (c == 62) { dp++; }
        else if (c == 60) { dp--; }
        else if (c == 46) { printf("%c", (int)mem[dp]); }
        else if (c == 91) {
            if (!mem[dp]) {
                int depth = 1;
                while (depth > 0) {
                    pc++;
                    if (prog[pc] == 91) depth++;
                    else if (prog[pc] == 93) depth--;
                }
            }
        }
        else if (c == 93) {
            if (mem[dp]) {
                int depth = 1;
                while (depth > 0) {
                    pc--;
                    if (prog[pc] == 93) depth++;
                    else if (prog[pc] == 91) depth--;
                }
            }
        }
        pc++;
    }
    printf("\n");
    return 0;
}
