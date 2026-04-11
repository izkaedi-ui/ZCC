set pagination off
set confirm off
set print thread-events off
handle SIGSEGV stop print nopass
run zcc_pp.c -o /dev/null 2>/dev/null
bt 30
info registers rip rbp rsp rbx rax rcx rdx rdi rsi
x/10i $rip-20
quit
