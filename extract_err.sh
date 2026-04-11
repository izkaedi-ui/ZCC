#!/bin/bash
./zcc2 sqlite3_zcc.c -o /tmp/sqlite3.s 2> err.log
grep "unknown struct member" err.log | grep -o "'[^']*'" | sort | uniq -c | sort -nr | head -20
