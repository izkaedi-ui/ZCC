with open('part3.c', 'r') as f:
    c = f.read()
c = c.replace('if (top_count >= 50000) {', 'fprintf(stderr, "ZCC:PARSE top_decl kind=%d line=%d\\n", cc->tk, cc->tk_line);\n        if (top_count >= 50000) {', 1)
c = c.replace('if (cc->tk == TK_LBRACE) {\n        Node *block;', 'if (cc->tk == TK_LBRACE) {\n        Node *block;\n        fprintf(stderr, "ZCC:PARSE compound line=%d\\n", cc->tk_line);', 1)
with open('part3.c', 'w') as f:
    f.write(c)
print('Injected!')
