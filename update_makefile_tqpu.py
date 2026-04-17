with open('Makefile', 'a', encoding='utf-8') as f:
    f.write('\ntqpu: zcc.c $(PASSES) ir_to_tqpu.c ir_to_zsurface.c ir_to_zqec.c ir_to_zquantum.c ir_to_zprime.c\n')
    f.write('\t$(CC) -o zcc-tqpu $^ -lm -Wl,-z,execstack\n')
    f.write('\t@echo "TQPU ready — zcc now emits living Topological Quantum Processing Units"\n\n')
