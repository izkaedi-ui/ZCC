#!/bin/bash
cd /mnt/h/__DOWNLOADS/selforglinux/lua-5.4.6/src
gcc -no-pie lua.s -o lua -lm
./lua -e "print(1+1)"
echo EXIT:$?
