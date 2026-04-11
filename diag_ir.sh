ZCC_IR_BACKEND=1 ./zcc2 /tmp/t_loop.c -o /tmp/t_loop_ir.s 2>diag_ir.log
grep -A200 "PRE-OPT IR" diag_ir.log | head -n 80
