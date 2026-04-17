cd failures/2
gcc -O0 -w -o gcc_bin test_2.c -lm
./gcc_bin > gcc.out
E1=\True
../../zcc test_2.c --ir -o zcc_no_peep.s
gcc -o zcc_no_peep zcc_no_peep.s -lm
./zcc_no_peep > zcc.out
E2=\True
echo GCC=\ NO_PEEP=EOF
bash test_fail.sh
