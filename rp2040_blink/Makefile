# ZCC / Bare-Metal RP2040 Build Control

CC = arm-none-eabi-gcc
UF2CONV = python3 uf2conv.py
ZCC = ../zcc

# ARM Cortex-M0+ Constraints
CFLAGS = -mcpu=cortex-m0plus -mthumb -O0 -ffreestanding -nostdlib -Wall -Wextra
LDFLAGS = -T link.ld -nostdlib

all: ref

# -----------------------------------------------------
# GCC Reference Target
# -----------------------------------------------------
ref: hello_ref.uf2

hello_ref.elf: boot2_blob.s boot.s hello.c link.ld
	$(CC) $(CFLAGS) $(LDFLAGS) boot2_blob.s boot.s hello.c -o $@

hello_ref.bin: hello_ref.elf
	arm-none-eabi-objcopy -O binary $< $@

hello_ref.uf2: hello_ref.bin
	$(UF2CONV) $< -f RP2040 --base 0x10000000 -o $@

flash-ref: hello_ref.uf2
	cp hello_ref.uf2 /mnt/i/

# -----------------------------------------------------
# ZCC Backend Target (Milestone 1)
# -----------------------------------------------------
zcc: stress_zcc.uf2

stress_zcc.elf: boot2_blob.s boot.s stress_test.c link.ld
	gcc -E -P stress_test.c -o stress_test_pp.c
	$(ZCC) -target thumbv6m-none-eabi stress_test_pp.c -o stress_zcc.s
	$(CC) $(CFLAGS) $(LDFLAGS) boot2_blob.s boot.s stress_zcc.s -o $@

stress_zcc.bin: stress_zcc.elf
	arm-none-eabi-objcopy -O binary $< $@

stress_zcc.uf2: stress_zcc.bin
	$(UF2CONV) $< -f RP2040 --base 0x10000000 -o $@

flash-zcc: stress_zcc.uf2
	cp stress_zcc.uf2 /mnt/i/

# -----------------------------------------------------
clean:
	rm -f *.elf *.uf2 stress_zcc.s hello_zcc.s *.o
