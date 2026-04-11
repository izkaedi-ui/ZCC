cd /mnt/h/__DOWNLOADS/selforglinux || exit 1
gdb -batch -ex 'run zcc_pp.c -o zcc2.s' -ex 'frame 0' -ex 'set print pretty on' -ex 'print sizeof(*blk->head)' -ex 'print &blk->head->id' -ex 'print &blk->head->call_args' -ex 'print &blk->head->next' -ex 'x/140wx blk->head' ./zcc_debug > gdb_log6.txt 2>&1
