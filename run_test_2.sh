cp failures/2/test_2.c my_test.c
gcc -O0 -w -o gcc_my_test my_test.c -lm
./gcc_my_test > gcc.out
GCC_EXIT=$?
./zcc my_test.c --ir --peephole --peephole-deterministic -o zcc_peep.s
gcc -o zcc_peep zcc_peep.s -lm
./zcc_peep > zcc.out
PEEP_EXIT=$?
./zcc my_test.c --ir -o zcc_no_peep.s
gcc -o zcc_no_peep zcc_no_peep.s -lm
./zcc_no_peep > zcc_no_peep.out
NO_PEEP_EXIT=$?
echo GCC=$GCC_EXIT PEEP=$PEEP_EXIT NO_PEEP=$NO_PEEP_EXIT
