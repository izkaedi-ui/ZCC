set pagination off
break *0x567293
run
printf "=== CRASH STATE ===\n"
printf "rax (dereference target) = 0x%lx\n", $rax
printf "rbp = 0x%lx\n", $rbp
printf "rbp-32 = 0x%lx\n", *((long*)($rbp-32))
printf "pLoop->aiRowLogEst base = 0x%lx\n", *((long*)($rbp-32))+16
printf "value at +16 = 0x%lx\n", *((long*)(*((long*)($rbp-32))+16))
quit
