with open('part3.c', 'r') as f:
    c = f.read()
c = c.replace('strncpy(field->name, fname, MAX_IDENT - 1);', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D1\\n");\n            strncpy(field->name, fname, MAX_IDENT - 1);')
c = c.replace('field->type = ftype;', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D2\\n");\n            field->type = ftype;')
c = c.replace('falign = type_align(ftype);', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D3\\n");\n            falign = type_align(ftype);')
c = c.replace('if (falign > max_align) max_align = falign;', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D4\\n");\n            if (falign > max_align) max_align = falign;')
c = c.replace('if (is_union) {', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D5\\n");\n            if (is_union) {')
c = c.replace('offset = (offset + falign - 1) & ~(falign - 1);', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D6\\n");\n                    offset = (offset + falign - 1) & ~(falign - 1);\n                    fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D7\\n");')
c = c.replace('field->offset = offset;', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D8\\n");\n                field->offset = offset;')
with open('part3.c', 'w') as f:
    f.write(c)
print('Injected more crumbs!')
