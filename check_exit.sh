#!/bin/bash
rm -f rc2.txt
ZCC_IR_BACKEND=1 ZCC_EMIT_IR=1 ./zcc2 sqlite3_zcc.c -o tmp.s > ir_dump2.txt 2>&1
echo $? > rc2.txt
