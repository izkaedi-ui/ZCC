set args zcc.c -o zcc3_ir.s
break ZCC_IR_FLUSH
run
info registers rdi
x/a $rdi
bt
continue
