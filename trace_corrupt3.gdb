b sqlite3CorruptError
commands 1
silent
info registers rdi
frame 1
p pOld->nCell
p pOld->aiOvfl[0]
p pOld->nOverflow
c
end
r
