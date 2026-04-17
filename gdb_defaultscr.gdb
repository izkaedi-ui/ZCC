run
disassemble DefaultScreen
info registers
frame 0
x/4i EOF
gdb -q -batch -x gdb_defaultscr.gdb ./doom_zcc_bin
