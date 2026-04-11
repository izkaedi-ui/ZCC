#!/bin/bash
cat > /tmp/t_loop.c << 'CEOF'
int main() {
    int i;
    int sum;
    sum = 0;
    for (i = 0; i < 10; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}
CEOF

echo "=== AST PATH ==="
./zcc2 /tmp/t_loop.c -o /tmp/t_loop_ast.s 2>/dev/null
gcc -O0 -w -o /tmp/t_loop_ast /tmp/t_loop_ast.s
timeout 2 /tmp/t_loop_ast; echo "AST rc=$?"

echo "=== IR PATH ==="
ZCC_IR_BACKEND=1 ./zcc2 /tmp/t_loop.c -o /tmp/t_loop_ir.s 2>/dev/null
gcc -O0 -w -o /tmp/t_loop_ir /tmp/t_loop_ir.s
timeout 2 /tmp/t_loop_ir; echo "IR rc=$?"

echo "=== IR MAIN ASM ==="
grep -A50 'main:' /tmp/t_loop_ir.s | head -60

echo "=== AST MAIN ASM ==="
grep -A30 'main:' /tmp/t_loop_ast.s | head -35
