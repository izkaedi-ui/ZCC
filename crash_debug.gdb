set pagination off
run zcc_pp.c -o /dev/null
bt full
info registers
x/20i $pc-40
x/32gx $rsp
