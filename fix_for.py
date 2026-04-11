import re

with open('compiler_passes.c', 'r') as f:
    code = f.read()

pattern = re.compile(
    r'zcc_lower_stmt\(ctx, node->body\);\s+Instr \*br_body = calloc\(1, sizeof\(Instr\)\);\s+br_body->id = ctx->next_instr_id\+\+; br_body->op = OP_BR; br_body->src\[0\] = latch; br_body->n_src = 1; br_body->exec_freq = 1\.0;\s+emit_instr\(ctx, br_body\);\s+fn->blocks\[body_blk\]->succs\[0\] = latch; fn->blocks\[body_blk\]->n_succs = 1;\s+fn->blocks\[latch\]->preds\[0\] = body_blk; fn->blocks\[latch\]->n_preds = 1;'
)

repl = """zcc_lower_stmt(ctx, node->body);
             { Block *_bb=fn->blocks[ctx->cur_block]; Instr *_bt=_bb?_bb->tail:NULL;
               if(!(_bt&&(_bt->op==OP_RET||_bt->op==OP_BR||_bt->op==OP_CONDBR))){
                 Instr *br_body = calloc(1, sizeof(Instr));
                 br_body->id = ctx->next_instr_id++; br_body->op = OP_BR; br_body->src[0] = latch; br_body->n_src = 1; br_body->exec_freq = 1.0;
                 emit_instr(ctx, br_body);
                 fn->blocks[ctx->cur_block]->succs[0] = latch; fn->blocks[ctx->cur_block]->n_succs = 1;
                 fn->blocks[latch]->preds[fn->blocks[latch]->n_preds++] = ctx->cur_block;
               }
             }"""

if pattern.search(code):
    new_code = pattern.sub(repl, code)
    with open('compiler_passes.c', 'w') as f:
        f.write(new_code)
    print("Fixed!")
else:
    print("Not found!")
