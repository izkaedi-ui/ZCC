import sys
with open('part4.c', 'r', encoding='utf-8') as f:
    code = f.read()

code = code.replace(
'''      if (strcmp(reg, "rax") == 0) reg = "r0";
      else if (strcmp(reg, "r11") == 0) reg = "r1";
      fprintf(cc->out, "    push {%s}\\n", reg);''',
'''      if (strcmp(reg, "rax") == 0) reg = "r0";
      else if (strcmp(reg, "r11") == 0) reg = "r1";
      else if (strcmp(reg, "rdx") == 0) reg = "r2";
      fprintf(cc->out, "    push {%s}\\n", reg);'''
)
code = code.replace(
'''      if (strcmp(reg, "rax") == 0) reg = "r0";
      else if (strcmp(reg, "r11") == 0) reg = "r1";
      fprintf(cc->out, "    pop {%s}\\n", reg);''',
'''      if (strcmp(reg, "rax") == 0) reg = "r0";
      else if (strcmp(reg, "r11") == 0) reg = "r1";
      else if (strcmp(reg, "rdx") == 0) reg = "r2";
      fprintf(cc->out, "    pop {%s}\\n", reg);'''
)

with open('part4.c', 'w', encoding='utf-8') as f:
    f.write(code)
