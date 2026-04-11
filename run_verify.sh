#!/bin/bash
echo "Building ZCC stage 2 and IR stage 3..."
make clean
make selfhost
make ir-verify
if [ -f zcc_ir_stage3.s ]; then
    echo "SUCCESS: IR Stage 3 completed!"
    md5sum zcc2.s zcc_ir_stage3.s
else
    echo "FAILED: zcc_ir_stage3.s not created."
fi
