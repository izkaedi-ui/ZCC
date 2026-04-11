import sys
content = open('compiler_passes.c').read()
if 'print_ir_module(stderr, ctx->fn);' not in content:
    content = content.replace('ir_module_to_ssa(m);', 'if(is_main_func(m->funcs[i]->name) == 0 && strstr(m->funcs[i]->name, " log2_of\))