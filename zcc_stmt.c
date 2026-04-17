void codegen_stmt(Compiler *cc, Node *node);
void codegen_expr(Compiler *cc, Node *node);
void codegen_addr(Compiler *cc, Node *node);
void codegen_store(Compiler *cc, Type *type);
void codegen_load(Compiler *cc, Type *type);
Node *node_new(Compiler *cc, int kind, int line);
Node *node_num(Compiler *cc, long long val, int line);
Node *node_flit(Compiler *cc, double val, int line);
Type *type_new(Compiler *cc, int kind);
Type *type_ptr(Compiler *cc, Type *base);
Type *type_array(Compiler *cc, Type *base, int len);
int type_size(Type *t);
int type_align(Type *t);
int is_integer(Type *t);
int is_pointer(Type *t);
int is_float_type(Type *t);
static int is_type_token(Compiler *cc);
int peek_token(Compiler *cc);
void error(Compiler *cc, char *msg);
void error_at(Compiler *cc, int line, char *msg);

/* ZKAEDI FORCE RENDER CACHE INVALIDATION */

/* ================================================================ */
/* ARENA ALLOCATOR                                                   */
/* ================================================================ */

static unsigned long long next_alloc_id = 1;

/* Cap total arena blocks to avoid OOM from unbounded allocation (e.g. corrupt AST / infinite loop). Use literal 64 so compiler without preprocessor still works. */
void *cc_alloc(Compiler *cc, int size) {
    ArenaBlock *a;
    void *p;
    static int block_count = 0;
    size = (size + 7) & ~7;  /* align to 8 */
    a = &cc->arena;
    while (a) {
        if (a->pos + size <= a->cap) {
            p = a->data + a->pos;
            a->pos = a->pos + size;
            /* zero the memory — use temp ptr to avoid complex cast expression */
            {
                char *cp;
                int i;
                cp = (char *)p;
                for (i = 0; i < size; i++) {
                    cp[i] = 0;
                }
            }
            /* magic header: second qword = alloc_id for tracing */
            if (size >= 16) {
                *((unsigned long long *)((char *)p + 8)) = next_alloc_id++;
            }
            return p;
        }
        if (!a->next) {
            if (block_count >= 64) {
                printf("zcc: too many arena blocks (%d) — possible infinite loop or corrupt AST (last alloc size=%d)\n", block_count, size);
                if (cc) {
                    printf("zcc: stuck near token kind=%d line=%d text=%.127s\n", cc->tk, cc->tk_line, cc->tk_text);
                }
                exit(1);
            }
            if (block_count >= 60 && cc) {
                printf("zcc: near block limit block_count=%d token=%d line=%d size=%d text=%.127s\n", block_count, cc->tk, cc->tk_line, size, cc->tk_text);
            }
            block_count++;
            {
                int newcap;
                ArenaBlock *nb;
                newcap = ARENA_SIZE;
                if (size > newcap) newcap = size * 2;
                nb = (ArenaBlock *)malloc(sizeof(ArenaBlock));
                nb->data = (char *)malloc(newcap);
                nb->pos = 0;
                nb->cap = newcap;
                nb->next = 0;
                a->next = nb;
            }
        }
        a = a->next;
    }
    printf( "zcc: arena alloc failed\n");
    exit(1);
    return 0;
}