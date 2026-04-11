#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux

echo "=== IRType enum ==="
grep -n "IRType\|IR_TY_\|ir_type\b" compiler_passes.c | head -40

echo ""
echo "=== irtype_from_node/node_type accessors ==="
grep -n "irtype_from_node\|node_type_size\|node_type_unsigned" compiler_passes.c zcc.c | head -30

echo ""
echo "=== ir_asm_lower_insn full body (line numbers) ==="
awk '/^[a-z].*ir_asm_lower_insn/,/^\}/' compiler_passes.c | head -250

echo ""
echo "=== OP_ADD/SUB/MUL/NEG in zcc_node_from_expr + zcc_lower_expr (zcc.c) ==="
grep -n "OP_ADD\|OP_SUB\|OP_MUL\|OP_NEG\|OP_AND\|OP_OR\|OP_XOR\|OP_NOT\|OP_LT\|OP_GT\|OP_EQ\|OP_NE\|OP_LE\|OP_GE" zcc.c | grep -v "^[0-9]*:.*\/\/" | head -80

echo ""
echo "=== irtype_from_node call sites in zcc.c ==="
grep -n "irtype_from_node\|\.ir_type\s*=" zcc.c | head -40
