set pagination off
break *0x567281
run
printf "=== AT CRASH-SITE mov $0x0,rax ===\n"
printf "rax before zero = 0x%lx\n", $rax
printf "r11 = 0x%lx\n", $r11
printf "rbp = 0x%lx\n", $rbp
x/8gx $rbp-320
stepi
printf "rax after zero = 0x%lx\n", $rax
cont
quit
