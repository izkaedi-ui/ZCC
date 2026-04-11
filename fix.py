with open('compiler_passes.c', 'r') as f:
    code = f.read()

target = '''            /* Body: loop statements, then branch to latch */
             ctx->cur_block = body_blk;
             zcc_lower_stmt(ctx, node->body);
             Instr *br_body = calloc(1, sizeof(Instr));
              br_body->id = ctx->next_instr_id++; br_body->op = OP_BR; br_body->src[0] = latch; br_body->n_src = 1; br_body->exec_freq = 1.0;
              emit_instr(ctx, br_body);
              fn->blocks[body_blk]->succs[0] = latch; fn->blocks[body_blk]->n_succs = 1;
              fn->blocks[latch]->preds[0] = body_blk; fn->blocks[latch]->n_preds = 1;'''

repl = '''            /* Body: loop statements, then branch to latch */
             ctx->cur_block = body_blk;
             zcc_lower_stmt(ctx, node->body);
             { Block *_bb=fn->blocks[ctx->cur_block]; Instr *_bt=_bb?_bb->tail:NULL;
               if(!(_bt&&(_bt->op==OP_RET||_bt->op==OP_BR||_bt->op==OP_CONDBR))){
                 Instr *br_body = calloc(1, sizeof(Instr));
                 br_body->id = ctx->next_instr_id++; br_body->op = OP_BR; br_body->src[0] = latch; br_body->n_src = 1; br_body->exec_freq = 1.0;
                 emit_instr(ctx, br_body);
                 fn->blocks[ctx->cur_block]->succs[0] = latch; fn->blocks[ctx->cur_block]->n_succs = 1;
                 fn->blocks[latch]->preds[fn->blocks[latch]->n_preds++] = ctx->cur_block;
               }
             }'''

if target in code:
    with open('compiler_passes.c', 'w') as f:
        f.write(code.replace(target, repl))
    print('fixed ZND_FOR')
else:
    print('not found ZND_FOR')
