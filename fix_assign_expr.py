c = open("compiler_passes.c").read()
old = "  case ZND_COMPOUND_ASSIGN: {\n    z->lhs = zcc_node_from_expr(node_lhs(n));\n    z->rhs = zcc_node_from_expr(node_rhs(n));\n    z->compound_op = nd_to_znd(node_compound_op(n));"
new = "  case ZND_ASSIGN:\n    z->lhs = zcc_node_from_expr(node_lhs(n));\n    z->rhs = zcc_node_from_expr(node_rhs(n));\n    break;\n  case ZND_COMPOUND_ASSIGN: {\n    z->lhs = zcc_node_from_expr(node_lhs(n));\n    z->rhs = zcc_node_from_expr(node_rhs(n));\n    z->compound_op = nd_to_znd(node_compound_op(n));"
if old in c:
    c = c.replace(old, new, 1)
    open("compiler_passes.c", "w").write(c)
    print("FIXED: added ZND_ASSIGN to zcc_node_from_expr")
else:
    print("NOT FOUND")
