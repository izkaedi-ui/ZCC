set pagination off
break *0x567293
run
printf "rax=0x%lx\n", $rax
printf "r11=0x%lx\n", $r11
printf "rbp=0x%lx\n", $rbp
x/4gx $rax-16
x/4gx $rax
info frame
quit
