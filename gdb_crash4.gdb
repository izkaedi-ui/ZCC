set pagination off
break *0x567293
run
printf "=== pProbe investigation ===\n"
printf "rbp = 0x%lx\n", $rbp
printf "rbp-32 (pProbe) storage addr = 0x%lx\n", $rbp-32
printf "[*pProbe] = 0x%lx\n", *((long*)($rbp-32))
x/4gx $rbp-32
printf "r15 = 0x%lx\n", $r15
printf "r14 = 0x%lx\n", $r14
printf "r13 = 0x%lx\n", $r13
printf "r12 = 0x%lx\n", $r12
printf "rbx = 0x%lx\n", $rbx
x/8gx $rbx
quit
