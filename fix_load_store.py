import re

with open("compiler_passes.c", "r") as f:
    text = f.read()

pattern_load = r"""        case OP_LOAD: \{
            if \(s_debug_main_emit\)
                fprintf\(stderr, "\[PGO-DEBUG\] block %u OP_LOAD dst=%u src0=%u\\n", \(unsigned\)cur_block, ins->dst, ins->src\[0\]\);
            int sz = \(ins->imm == 4\) \? 4 : 8;
            ir_asm_load_to_rax\(ctx, ins->src\[0\]\);
            if \(sz == 4\) \{
                fprintf\(f, "    movl \(%%rax\), %%eax\\n"\);
                fprintf\(f, "    movslq %%eax, %%rax\\n"\);
            \} else \{
                fprintf\(f, "    movq \(%%rax\), %%rax\\n"\);
            \}
            ir_asm_store_rax_to\(ctx, ins->dst\);
            break;
        \}"""

fix_load = """        case OP_LOAD: {
            if (s_debug_main_emit)
                fprintf(stderr, "[PGO-DEBUG] block %u OP_LOAD dst=%u src0=%u\\n", (unsigned)cur_block, ins->dst, ins->src[0]);
            ir_asm_load_to_rax(ctx, ins->src[0]);
            switch ((int)ins->imm) {
                case 1:  fprintf(f, "    movzbq (%%rax), %%rax\\n"); break;
                case 2:  fprintf(f, "    movzwq (%%rax), %%rax\\n"); break;
                case 4:  fprintf(f, "    movslq (%%rax), %%rax\\n"); break;
                default: fprintf(f, "    movq (%%rax), %%rax\\n");   break;
            }
            ir_asm_store_rax_to(ctx, ins->dst);
            break;
        }"""
        
text = re.sub(pattern_load, fix_load, text, count=1)

pattern_store = r"""        case OP_STORE:
            if \(ins->n_src >= 2\) \{
                int sz = \(ins->imm == 4\) \? 4 : 8;
                ir_asm_load_to_rax\(ctx, ins->src\[1\]\);
                ir_asm_load_to_rcx\(ctx, ins->src\[0\]\);
                if \(sz == 4\)
                    fprintf\(f, "    movl %%ecx, \(%%rax\)\\n"\);
                else
                    fprintf\(f, "    movq %%rcx, \(%%rax\)\\n"\);
            \}
            break;"""
            
fix_store = """        case OP_STORE:
            if (ins->n_src >= 2) {
                ir_asm_load_to_rax(ctx, ins->src[1]);
                ir_asm_load_to_rcx(ctx, ins->src[0]);
                switch ((int)ins->imm) {
                    case 1:  fprintf(f, "    movb %%cl, (%%rax)\\n");  break;
                    case 2:  fprintf(f, "    movw %%cx, (%%rax)\\n");  break;
                    case 4:  fprintf(f, "    movl %%ecx, (%%rax)\\n"); break;
                    default: fprintf(f, "    movq %%rcx, (%%rax)\\n"); break;
                }
            }
            break;"""

text = re.sub(pattern_store, fix_store, text, count=1)

# Now apply is_array fix to ZND_VAR in zcc_lower_expr
pattern_var = r"""        case ZND_VAR: \{
            if \(node->is_global\) \{
                RegID ptr_r = ctx->next_reg\+\+;
                Instr \*lea_ins = make_instr_global\(ctx->next_instr_id\+\+, ptr_r, node->name, node->line_no\);
                emit_instr\(ctx, lea_ins\);
                if \(ctx->want_address\) return ptr_r;
                RegID val_r = ctx->next_reg\+\+;
                Instr \*load_ins = make_instr\(ctx->next_instr_id\+\+, OP_LOAD, val_r, \(RegID\[\]\)\{ptr_r\}, 1\);
                load_ins->line_no = node->line_no;
                emit_instr\(ctx, load_ins\);
                return val_r;
            \} else \{
                RegID alloca_r = get_or_create_var\(ctx, node->name\);
                if \(!alloca_r\) return 0;
                if \(ctx->want_address\) return alloca_r;
                RegID load_r = ctx->next_reg\+\+;
                Instr \*load_ins = make_instr\(ctx->next_instr_id\+\+, OP_LOAD, load_r, \(RegID\[\]\)\{alloca_r\}, 1\);
                load_ins->line_no = node->line_no;
                emit_instr\(ctx, load_ins\);
                return load_r;
            \}
        \}"""
        
fix_var = """        case ZND_VAR: {
            if (node->is_global) {
                RegID ptr_r = ctx->next_reg++;
                Instr *lea_ins = make_instr_global(ctx->next_instr_id++, ptr_r, node->name, node->line_no);
                emit_instr(ctx, lea_ins);
                if (ctx->want_address) return ptr_r;
                if (node->is_array) return ptr_r;
                RegID val_r = ctx->next_reg++;
                Instr *load_ins = make_instr(ctx->next_instr_id++, OP_LOAD, val_r, (RegID[]){ptr_r}, 1);
                load_ins->imm = 8;
                if (node->ty && node->ty->kind == TY_VECTOR) load_ins->imm = 8;
                else if (node->member_size > 0) load_ins->imm = node->member_size;
                load_ins->line_no = node->line_no;
                emit_instr(ctx, load_ins);
                return val_r;
            } else {
                RegID alloca_r = get_or_create_var(ctx, node->name);
                if (!alloca_r) return 0;
                if (ctx->want_address) return alloca_r;
                if (node->is_array) return alloca_r;
                RegID load_r = ctx->next_reg++;
                Instr *load_ins = make_instr(ctx->next_instr_id++, OP_LOAD, load_r, (RegID[]){alloca_r}, 1);
                load_ins->imm = 8;
                if (node->ty && node->ty->kind == TY_VECTOR) load_ins->imm = 8;
                else if (node->member_size > 0) load_ins->imm = node->member_size;
                load_ins->line_no = node->line_no;
                emit_instr(ctx, load_ins);
                return load_r;
            }
        }"""
        
text = re.sub(pattern_var, fix_var, text, count=1)


with open("compiler_passes.c", "w") as f:
    f.write(text)
