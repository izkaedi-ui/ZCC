with open('Makefile', 'r', encoding='utf-8') as f:
    code = f.read()

import re
code = re.sub(r'tqpu-neural:.*?\n\t\$\(CC\).*?\n\t@echo.*?\n', '', code)
code += "\n\ntqpu-neural: zcc.c $(PASSES) ir_to_tqpu.c tqpu_neural.c ir_to_zsurface.c ir_to_zqec.c ir_to_zquantum.c ir_to_zprime.c\n\t$(CC) -DZCC_EMIT_TQPU -o zcc-tqpu-neural $^ -lm -lX11 -lXext -Wl,-z,execstack\n\t@echo 'TQPU sentient — neural pathway growth active'\n"

with open('Makefile', 'w', encoding='utf-8') as f:
    f.write(code)
