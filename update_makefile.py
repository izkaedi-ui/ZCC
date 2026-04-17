with open('Makefile', 'a', encoding='utf-8') as f:
    f.write('\n# --- ZKAEDI PRIME QUANTUM EXTENSIONS ---\n')
    f.write('zprime-quantum: zcc.c $(PASSES) ir_to_zquantum.c ir_to_zprime.c\n')
    f.write('\t$(CC) -o zcc-zprime-quantum $^ -lm -Wl,-z,execstack\n')
    f.write('\t@echo "ZPrime Quantum-Inspired ready — codegen now runs superposition + collapse"\n\n')
    
    f.write('zprime-qec: zcc.c $(PASSES) ir_to_zqec.c ir_to_zquantum.c ir_to_zprime.c\n')
    f.write('\t$(CC) -o zcc-zprime-qec $^ -lm -Wl,-z,execstack\n')
    f.write('\t@echo "ZPrime QEC ready — codegen now detects & corrects errors in quantum lowering"\n\n')
    
    f.write('zprime-surface: zcc.c $(PASSES) ir_to_zsurface.c ir_to_zqec.c ir_to_zquantum.c ir_to_zprime.c\n')
    f.write('\t$(CC) -o zcc-zprime-surface $^ -lm -Wl,-z,execstack\n')
    f.write('\t@echo "ZPrime Surface Code ready — codegen now runs full 2D topological QEC"\n')
