import sys

with open('part4.c', 'r') as f:
    lines = f.readlines()

new_lines = []
for line in lines:
    if 'long long offset =' in line and 'elem->' in line:
        pass # old lines, we'll replace
    new_lines.append(line)
    if 'else if (elem->kind == ND_VAR)' in line:
        new_lines.insert(-1, '''                } else if (elem->kind == ND_ADDR && elem->lhs && elem->lhs->kind == ND_DEREF && elem->lhs->lhs && elem->lhs->lhs->kind == ND_ADD && elem->lhs->lhs->lhs && elem->lhs->lhs->lhs->kind == ND_VAR && elem->lhs->lhs->rhs && elem->lhs->lhs->rhs->kind == ND_NUM) {
                    long long raw_val = elem->lhs->lhs->rhs->int_val;
                    long long tsize = 1;
                    if (elem->lhs->lhs->lhs->type && elem->lhs->lhs->lhs->type->ptr_to) {
                        tsize = type_size(elem->lhs->lhs->lhs->type->ptr_to);
                    }
                    long long offset = raw_val * tsize;
                    fprintf(stderr, "DEBUG_ZCC_ELEM: raw=%lld tsize=%lld offset=%lld\\n", raw_val, tsize, offset);
                    if (elem_size == 4) fprintf(cc->out, "    .long %s + %lld\\n", elem->lhs->lhs->lhs->name, offset);
                    else fprintf(cc->out, "    .quad %s + %lld\\n", elem->lhs->lhs->lhs->name, offset);
''')
    if '} else if (gvar->initializer->kind == ND_VAR) {' in line:
        new_lines.insert(-1, '''    } else if (gvar->initializer->kind == ND_ADDR && gvar->initializer->lhs && gvar->initializer->lhs->kind == ND_DEREF && gvar->initializer->lhs->lhs && gvar->initializer->lhs->lhs->kind == ND_ADD && gvar->initializer->lhs->lhs->lhs && gvar->initializer->lhs->lhs->lhs->kind == ND_VAR && gvar->initializer->lhs->lhs->rhs && gvar->initializer->lhs->lhs->rhs->kind == ND_NUM) {
      long long raw_val = gvar->initializer->lhs->lhs->rhs->int_val;
      long long tsize = 1;
      if (gvar->initializer->lhs->lhs->lhs->type && gvar->initializer->lhs->lhs->lhs->type->ptr_to) {
        tsize = type_size(gvar->initializer->lhs->lhs->lhs->type->ptr_to);
      }
      long long offset = raw_val * tsize;
      fprintf(stderr, "DEBUG_ZCC_GVAR: raw=%lld tsize=%lld offset=%lld\\n", raw_val, tsize, offset);
      if (size == 4) fprintf(cc->out, "    .long %s + %lld\\n", gvar->initializer->lhs->lhs->lhs->name, offset);
      else fprintf(cc->out, "    .quad %s + %lld\\n", gvar->initializer->lhs->lhs->lhs->name, offset);
''')

with open('part4.c', 'w') as f:
    f.writelines(new_lines)
print("done!")