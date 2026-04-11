b yy_reduce
run
watch *(long*)(0x7fffffffd6d8 + 8)
continue
bt
info registers
x/4gx 0x7fffffffd6d8
