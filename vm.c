int printf(const char *fmt, ...);

int stack[256];
int sp;

void push(int v) { stack[sp++] = v; }
int pop() { return stack[--sp]; }

int main() {
    int prog[32];
    int pc, a, b;
    sp = 0; pc = 0;

    prog[0]  = 1; prog[1]  = 10;
    prog[2]  = 1; prog[3]  = 20;
    prog[4]  = 2;
    prog[5]  = 1; prog[6]  = 5;
    prog[7]  = 3;
    prog[8]  = 1; prog[9]  = 3;
    prog[10] = 4;
    prog[11] = 5;
    prog[12] = 0;

    while (prog[pc] != 0) {
        int op = prog[pc++];
        if (op == 1) { push(prog[pc++]); }
        else if (op == 2) { b = pop(); a = pop(); push(a + b); }
        else if (op == 3) { b = pop(); a = pop(); push(a * b); }
        else if (op == 4) { b = pop(); a = pop(); push(a - b); }
        else if (op == 5) { printf("result: %d\n", pop()); }
    }
    return 0;
}
