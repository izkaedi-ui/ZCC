set pagination off
b parse_program
run test.c
b part3.c:2078
commands
print cc->pos
print prev_pos
print cc->tk
continue
end
continue
