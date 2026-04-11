import re

with open("compiler_passes.c", "r") as f:
    text = f.read()

# Fix ZND_ASSIGN
pattern_assign = r"""         case ZND_ASSIGN: \{
             if \(\!node->lhs\) return;
             RegID addr_r;
             if \(node->lhs->kind == ZND_VAR\) \{
                 addr_r = get_or_create_var\(ctx, node->lhs->name\);
                 if \(\!addr_r\) return;
             \} else \{
                 ctx->want_address = 1;
                 addr_r = zcc_lower_expr\(ctx, node->lhs\);
                 ctx->want_address = 0;
                 if \(\!addr_r\) return;
             \}
             RegID val_r = zcc_lower_expr\(ctx, node->rhs\);
             Instr \*st = calloc\(1, sizeof\(Instr\)\);
             st->id = ctx->next_instr_id\+\+;
             st->op = OP_STORE;
             st->dst = 0;
             st->src\[0\] = val_r; st->src\[1\] = addr_r;
             st->n_src = 2;
             st->exec_freq = 1\.0;
             emit_instr\(ctx, st\);
             return;
         \}"""
         
fix_assign = """         case ZND_ASSIGN: {
             if (!node->lhs) return;
             RegID addr_r;
             ctx->want_address = 1;
             addr_r = zcc_lower_expr(ctx, node->lhs);
             ctx->want_address = 0;
             if (!addr_r) return;
             RegID val_r = zcc_lower_expr(ctx, node->rhs);
             Instr *st = calloc(1, sizeof(Instr));
             st->id = ctx->next_instr_id++;
             st->op = OP_STORE;
             st->dst = 0;
             st->src[0] = val_r; st->src[1] = addr_r;
             st->n_src = 2;
             st->exec_freq = 1.0;
             st->imm = 8;
             if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0)
                 st->imm = node->lhs->member_size;
             emit_instr(ctx, st);
             return;
         }"""

text = re.sub(pattern_assign, fix_assign, text, count=1)

# Fix ZND_COMPOUND_ASSIGN
pattern_compound = r"""         case ZND_COMPOUND_ASSIGN: \{
             if \(\!node->lhs \|\| \!node->rhs\) return;
             RegID addr_r;
             if \(node->lhs->kind == ZND_VAR\) \{
                 addr_r = get_or_create_var\(ctx, node->lhs->name\);
                 if \(\!addr_r\) return;
             \} else \{
                 ctx->want_address = 1;
                 addr_r = zcc_lower_expr\(ctx, node->lhs\);
                 ctx->want_address = 0;
                 if \(\!addr_r\) return;
             \}
             RegID load_r = ctx->next_reg\+\+;
             Instr \*load_ins = calloc\(1, sizeof\(Instr\)\);
             load_ins->id = ctx->next_instr_id\+\+;
             load_ins->op = OP_LOAD;
             load_ins->dst = load_r;
             load_ins->src\[0\] = addr_r;
             load_ins->n_src = 1;
             load_ins->exec_freq = 1\.0;
             emit_instr\(ctx, load_ins\);
             RegID rhs_r = zcc_lower_expr\(ctx, node->rhs\);
             RegID sum_r = ctx->next_reg\+\+;
             Instr \*add_ins = calloc\(1, sizeof\(Instr\)\);
             add_ins->id = ctx->next_instr_id\+\+;
             add_ins->op = OP_ADD;
             add_ins->dst = sum_r;
             add_ins->src\[0\] = load_r;
             add_ins->src\[1\] = rhs_r;
             add_ins->n_src = 2;
             add_ins->exec_freq = 1\.0;
             emit_instr\(ctx, add_ins\);
             Instr \*st = calloc\(1, sizeof\(Instr\)\);
             st->id = ctx->next_instr_id\+\+;
             st->op = OP_STORE;
             st->dst = 0;
             st->src\[0\] = sum_r;
             st->src\[1\] = addr_r;
             st->n_src = 2;
             st->exec_freq = 1\.0;
             emit_instr\(ctx, st\);
             return;
         \}"""

fix_compound = """         case ZND_COMPOUND_ASSIGN: {
             if (!node->lhs || !node->rhs) return;
             RegID addr_r;
             ctx->want_address = 1;
             addr_r = zcc_lower_expr(ctx, node->lhs);
             ctx->want_address = 0;
             if (!addr_r) return;
             
             RegID load_r = ctx->next_reg++;
             Instr *load_ins = calloc(1, sizeof(Instr));
             load_ins->id = ctx->next_instr_id++;
             load_ins->op = OP_LOAD;
             load_ins->dst = load_r;
             load_ins->src[0] = addr_r;
             load_ins->n_src = 1;
             load_ins->imm = 8;
             if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0)
                 load_ins->imm = node->lhs->member_size;
             load_ins->exec_freq = 1.0;
             emit_instr(ctx, load_ins);
             
             RegID rhs_r = zcc_lower_expr(ctx, node->rhs);
             RegID sum_r = ctx->next_reg++;
             Instr *add_ins = calloc(1, sizeof(Instr));
             add_ins->id = ctx->next_instr_id++;
             add_ins->op = OP_ADD;
             add_ins->dst = sum_r;
             add_ins->src[0] = load_r;
             add_ins->src[1] = rhs_r;
             add_ins->n_src = 2;
             add_ins->exec_freq = 1.0;
             emit_instr(ctx, add_ins);
             
             Instr *st = calloc(1, sizeof(Instr));
             st->id = ctx->next_instr_id++;
             st->op = OP_STORE;
             st->dst = 0;
             st->src[0] = sum_r;
             st->src[1] = addr_r;
             st->n_src = 2;
             st->exec_freq = 1.0;
             st->imm = 8;
             if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0)
                 st->imm = node->lhs->member_size;
             emit_instr(ctx, st);
             return;
         }"""

text = re.sub(pattern_compound, fix_compound, text, count=1)

with open("compiler_passes.c", "w") as f:
    f.write(text)
