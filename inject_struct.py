with open('part3.c', 'r') as f:
    c = f.read()
c = c.replace('static Type *parse_struct_or_union(Compiler * cc, int is_union) {\n    Type *stype;', 'static Type *parse_struct_or_union(Compiler * cc, int is_union) {\n    fprintf(stderr, "ZCC:DEBUG parse_struct_or_union enter\\n");\n    Type *stype;')
c = c.replace('stype = type_new(cc, is_union ? TY_UNION : TY_STRUCT);', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union A\\n");\n        stype = type_new(cc, is_union ? TY_UNION : TY_STRUCT);')
c = c.replace('expect(cc, TK_LBRACE);', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union B\\n");\n    expect(cc, TK_LBRACE);')
c = c.replace('ftype = parse_type(cc);', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union C\\n");\n            ftype = parse_type(cc);')
c = c.replace('field = (StructField *)cc_alloc(cc, sizeof(StructField));', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union D\\n");\n            field = (StructField *)cc_alloc(cc, sizeof(StructField));')
c = c.replace('expect(cc, TK_SEMI);', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union E\\n");\n            expect(cc, TK_SEMI);')
c = c.replace('stype->is_complete = 1;', 'fprintf(stderr, "ZCC:DEBUG parse_struct_or_union F\\n");\n        stype->is_complete = 1;')
with open('part3.c', 'w') as f:
    f.write(c)
print('Injected debug crumbs!')
