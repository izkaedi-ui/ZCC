#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux
sed 's/#include "zcc_ast_bridge.h"/#include "zcc_ast_bridge_zcc.h"/' zcc.c | grep -v '_Static_assert' > zcc_pp.c
echo "zcc_pp.c regenerated"
gcc -O0 -w -fno-asynchronous-unwind-tables -o zcc_new zcc.c compiler_passes.c compiler_passes_ir.c -lm && echo BUILD_OK || echo BUILD_FAIL
