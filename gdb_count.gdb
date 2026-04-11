set pagination off
break *0x44460b
break *0x4446a7
break *0x4446fc
break *0x44480a
break *0x4449bc
break *0x4449f0
commands 1 2 3 4 5 6
  printf "=== Hit jump to page1_init_failed at %lx ===\n", $rip
  # Disassemble a bit to see context
  x/10i $rip - 15
  # Also show what's in rax/rdi just in case
  info registers rax rdi
end

run
quit
