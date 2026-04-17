with open('Makefile', 'a', encoding='utf-8') as f:
    f.write("\n\nqemu-rp2040-lua: zcc.c $(PASSES) ir_to_tqpu.c tqpu_neural.c qemu_rp2040.c ir_to_zsurface.c ir_to_zqec.c ir_to_zquantum.c ir_to_zprime.c\n\t$(CC) -DZCC_EMIT_TQPU -DZCC_TARGET_QEMU_RP2040 -o zcc-qemu-rp2040 $^ -lm -lX11 -lXext -Wl,-z,execstack\n\t@echo 'Sentient TQPU + QEMU RP2040 + Lua detonation ready'\n")
