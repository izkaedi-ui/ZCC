with open('Makefile', 'r', encoding='utf-8') as f:
    code = f.read()

code = code.replace('-o zcc-tqpu', '-DZCC_EMIT_TQPU -o zcc-tqpu')

with open('Makefile', 'w', encoding='utf-8') as f:
    f.write(code)
