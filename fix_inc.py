import re

with open("compiler_passes.c", "r") as f:
    text = f.read()

# We need to replace the entire block from 'case ZND_POST_INC:' to the end of ZND_PRE_INC etc.
# Actually, let's just write a clean function and replace exactly where we need.

fixes = """        case ZND_PRE_INC: {
            if (!node->lhs) return 0;
            ctx->want_address = 1;
            RegID addr_r = zcc_lower_expr(ctx, node->lhs);
            ctx->want_address = 0;
            if (!addr_r) return 0;
            Instr *load_ins = calloc(1, sizeof(Instr));
            load_ins->id = ctx->next_instr_id++;
            load_ins->op = OP_LOAD;
            load_ins->dst = r;
            load_ins->src[0] = addr_r;
            load_ins->n_src = 1;
            load_ins->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) load_ins->imm = node->lhs->member_size;
            load_ins->exec_freq = 1.0;
            load_ins->line_no = node->line_no;
            emit_instr(ctx, load_ins);
            RegID r_new = ctx->next_reg++;
            Instr *add_ins = calloc(1, sizeof(Instr));
            add_ins->id = ctx->next_instr_id++;
            add_ins->op = OP_ADD;
            add_ins->dst = r_new;
            add_ins->src[0] = r;
            add_ins->src[1] = 0;
            add_ins->n_src = 2;
            add_ins->exec_freq = 1.0;
            add_ins->line_no = node->line_no;
            RegID one_r = ctx->next_reg++;
            Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, 1, node->line_no);
            emit_instr(ctx, c1);
            add_ins->src[1] = one_r;
            emit_instr(ctx, add_ins);
            Instr *st = calloc(1, sizeof(Instr));
            st->id = ctx->next_instr_id++;
            st->op = OP_STORE;
            st->dst = 0;
            st->src[0] = r_new;
            st->src[1] = addr_r;
            st->n_src = 2;
            st->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) st->imm = node->lhs->member_size;
            st->exec_freq = 1.0;
            st->line_no = node->line_no;
            emit_instr(ctx, st);
            return r_new;
        }
        case ZND_PRE_DEC: {
            if (!node->lhs) return 0;
            ctx->want_address = 1;
            RegID addr_r = zcc_lower_expr(ctx, node->lhs);
            ctx->want_address = 0;
            if (!addr_r) return 0;
            Instr *load_ins = calloc(1, sizeof(Instr));
            load_ins->id = ctx->next_instr_id++;
            load_ins->op = OP_LOAD;
            load_ins->dst = r;
            load_ins->src[0] = addr_r;
            load_ins->n_src = 1;
            load_ins->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) load_ins->imm = node->lhs->member_size;
            load_ins->exec_freq = 1.0;
            load_ins->line_no = node->line_no;
            emit_instr(ctx, load_ins);
            RegID r_new = ctx->next_reg++;
            Instr *sub_ins = calloc(1, sizeof(Instr));
            sub_ins->id = ctx->next_instr_id++;
            sub_ins->op = OP_SUB;
            sub_ins->dst = r_new;
            sub_ins->src[0] = r;
            sub_ins->src[1] = 0;
            sub_ins->n_src = 2;
            sub_ins->exec_freq = 1.0;
            sub_ins->line_no = node->line_no;
            RegID one_r = ctx->next_reg++;
            Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, 1, node->line_no);
            emit_instr(ctx, c1);
            sub_ins->src[1] = one_r;
            emit_instr(ctx, sub_ins);
            Instr *st = calloc(1, sizeof(Instr));
            st->id = ctx->next_instr_id++;
            st->op = OP_STORE;
            st->dst = 0;
            st->src[0] = r_new;
            st->src[1] = addr_r;
            st->n_src = 2;
            st->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) st->imm = node->lhs->member_size;
            st->exec_freq = 1.0;
            st->line_no = node->line_no;
            emit_instr(ctx, st);
            return r_new;
        }
        case ZND_POST_INC: {
            if (!node->lhs) return 0;
            ctx->want_address = 1;
            RegID addr_r = zcc_lower_expr(ctx, node->lhs);
            ctx->want_address = 0;
            if (!addr_r) return 0;
            Instr *load_ins = calloc(1, sizeof(Instr));
            load_ins->id = ctx->next_instr_id++;
            load_ins->op = OP_LOAD;
            load_ins->dst = r;
            load_ins->src[0] = addr_r;
            load_ins->n_src = 1;
            load_ins->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) load_ins->imm = node->lhs->member_size;
            load_ins->exec_freq = 1.0;
            load_ins->line_no = node->line_no;
            emit_instr(ctx, load_ins);
            RegID r_new = ctx->next_reg++;
            Instr *add_ins = calloc(1, sizeof(Instr));
            add_ins->id = ctx->next_instr_id++;
            add_ins->op = OP_ADD;
            add_ins->dst = r_new;
            add_ins->src[0] = r;
            add_ins->src[1] = 0;
            add_ins->n_src = 2;
            add_ins->exec_freq = 1.0;
            add_ins->line_no = node->line_no;
            RegID one_r = ctx->next_reg++;
            Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, 1, node->line_no);
            emit_instr(ctx, c1);
            add_ins->src[1] = one_r;
            emit_instr(ctx, add_ins);
            Instr *st = calloc(1, sizeof(Instr));
            st->id = ctx->next_instr_id++;
            st->op = OP_STORE;
            st->dst = 0;
            st->src[0] = r_new;
            st->src[1] = addr_r;
            st->n_src = 2;
            st->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) st->imm = node->lhs->member_size;
            st->exec_freq = 1.0;
            st->line_no = node->line_no;
            emit_instr(ctx, st);
            return r;
        }
        case ZND_POST_DEC: {
            if (!node->lhs) return 0;
            ctx->want_address = 1;
            RegID addr_r = zcc_lower_expr(ctx, node->lhs);
            ctx->want_address = 0;
            if (!addr_r) return 0;
            Instr *load_ins = calloc(1, sizeof(Instr));
            load_ins->id = ctx->next_instr_id++;
            load_ins->op = OP_LOAD;
            load_ins->dst = r;
            load_ins->src[0] = addr_r;
            load_ins->n_src = 1;
            load_ins->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) load_ins->imm = node->lhs->member_size;
            load_ins->exec_freq = 1.0;
            load_ins->line_no = node->line_no;
            emit_instr(ctx, load_ins);
            RegID r_new = ctx->next_reg++;
            Instr *sub_ins = calloc(1, sizeof(Instr));
            sub_ins->id = ctx->next_instr_id++;
            sub_ins->op = OP_SUB;
            sub_ins->dst = r_new;
            sub_ins->src[0] = r;
            sub_ins->src[1] = 0;
            sub_ins->n_src = 2;
            sub_ins->exec_freq = 1.0;
            sub_ins->line_no = node->line_no;
            RegID one_r = ctx->next_reg++;
            Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, 1, node->line_no);
            emit_instr(ctx, c1);
            sub_ins->src[1] = one_r;
            emit_instr(ctx, sub_ins);
            Instr *st = calloc(1, sizeof(Instr));
            st->id = ctx->next_instr_id++;
            st->op = OP_STORE;
            st->dst = 0;
            st->src[0] = r_new;
            st->src[1] = addr_r;
            st->n_src = 2;
            st->imm = 8;
            if (node->lhs && node->lhs->kind == ZND_DEREF && node->lhs->member_size > 0) st->imm = node->lhs->member_size;
            st->exec_freq = 1.0;
            st->line_no = node->line_no;
            emit_instr(ctx, st);
            return r;
        }"""

pattern = r"        case ZND_PRE_INC: \{.*?return r;     /\* return old value \(before decrement\) \*/\n        \}"
text = re.sub(pattern, fixes, text, flags=re.DOTALL)

with open("compiler_passes.c", "w") as f:
    f.write(text)
