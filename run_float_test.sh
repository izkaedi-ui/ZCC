#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux
make clean
make
./zcc float_test.c -o float_test.s
gcc float_test.s -o float_test -lm
./float_test
echo "rc=$?"
