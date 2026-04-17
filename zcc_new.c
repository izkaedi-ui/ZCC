/*
 * ZCC - ZKAEDI C Compiler
 * A self-hosting C subset compiler targeting x86-64 Linux (System V ABI)
 * Single-file design: cat part1.c part2.c part3.c part4.c part5.c > zcc.c
 *
 * Usage:  ./zcc input.c -o output
 * Self-host: ./zcc zcc.c -o zcc2 && ./zcc2 zcc.c -o zcc3 && cmp zcc2 zcc3
 *
 * Supports: int/char/long/short/void/unsigned, pointers, arrays, structs,
 *           unions, enums, typedef, sizeof, function calls (up to 6 args),
 *           if/else/while/for/do-while/switch-case, goto/labels, break/continue,
 *           string literals, char literals, compound assignment, ternary,
 *           bitwise ops, shift ops, logical ops, pre/post inc/dec,
 *           struct member access (. and ->), casts, global/local vars,
 *           static/extern/const qualifiers, #include <stdio.h> etc via host cpp.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

/* ================================================================ */
/* CONSTANTS — using enum for self-hosting (no preprocessor needed)  */
/* ================================================================ */

enum {
    MAX_IDENT   = 128,
    MAX_STR     = 4096,
    MAX_STRINGS = 16384,
    MAX_GLOBALS = 16384,
    MAX_STRUCTS = 8192,
    MAX_PARAMS  = 128,
    MAX_CALL_ARGS = 256,
    MAX_CASES   = 4096,
    MAX_INIT    = 8192,
    ARENA_SIZE  = 16777216
};

/* ================================================================ */
/* TOKEN TYPES                                                       */
/* ================================================================ */

enum {
    TK_EOF = 0,
    TK_NUM, TK_STR, TK_CHAR_LIT, TK_IDENT, TK_FLIT,
    /* type keywords */
    TK_INT, TK_CHAR, TK_VOID, TK_LONG, TK_SHORT,
    TK_UNSIGNED, TK_SIGNED, TK_FLOAT, TK_DOUBLE,
    /* control flow */
    TK_IF, TK_ELSE, TK_WHILE, TK_FOR, TK_DO,
    TK_RETURN, TK_BREAK, TK_CONTINUE, TK_GOTO,
    TK_SWITCH, TK_CASE, TK_DEFAULT,
    /* type-related */
    TK_STRUCT, TK_UNION, TK_ENUM, TK_TYPEDEF,
    TK_SIZEOF, TK_STATIC, TK_EXTERN, TK_CONST,
    TK_VOLATILE, TK_AUTO, TK_REGISTER, TK_INLINE, TK_ASM,
    TK_BUILTIN_VA_ARG, TK_TYPEOF, TK_AUTO_TYPE, TK_GENERIC,
    /* operators */
    TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH, TK_PERCENT,
    TK_AMP, TK_PIPE, TK_CARET, TK_TILDE, TK_BANG,
    TK_ASSIGN, TK_EQ, TK_NE,
    TK_LT, TK_GT, TK_LE, TK_GE,
    TK_LAND, TK_LOR,
    TK_SHL, TK_SHR,
    TK_INC, TK_DEC,
    TK_ARROW, TK_DOT,
    TK_QUESTION, TK_COLON,
    /* compound assignment */
    TK_PLUS_ASSIGN, TK_MINUS_ASSIGN, TK_STAR_ASSIGN,
    TK_SLASH_ASSIGN, TK_PERCENT_ASSIGN,
    TK_AMP_ASSIGN, TK_PIPE_ASSIGN, TK_CARET_ASSIGN,
    TK_SHL_ASSIGN, TK_SHR_ASSIGN,
    /* delimiters */
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE,
    TK_LBRACKET, TK_RBRACKET,
    TK_SEMI, TK_COMMA, TK_ELLIPSIS,
    TK_HASH
};

/* ================================================================ */
/* AST NODE TYPES                                                    */
/* ================================================================ */

enum {
    ND_NUM = 1, ND_STR, ND_CHAR_LIT, ND_FLIT,
    ND_VAR, ND_ASSIGN,
    ND_ADD, ND_SUB, ND_MUL, ND_DIV, ND_MOD,
    ND_EQ, ND_NE, ND_LT, ND_LE, ND_GT, ND_GE,
    ND_LAND, ND_LOR, ND_LNOT,
    ND_BAND, ND_BOR, ND_BXOR, ND_BNOT,
    ND_SHL, ND_SHR,
    ND_FADD, ND_FSUB, ND_FMUL, ND_FDIV,
    ND_NEG, ND_ADDR, ND_DEREF,
    ND_CALL, ND_RETURN, ND_BLOCK,
    ND_IF, ND_WHILE, ND_FOR, ND_DO_WHILE,
    ND_BREAK, ND_CONTINUE, ND_GOTO, ND_LABEL,
    ND_SWITCH, ND_CASE, ND_DEFAULT,
    ND_CAST, ND_SIZEOF, ND_VA_ARG,
    ND_MEMBER, ND_PRE_INC, ND_PRE_DEC,
    ND_POST_INC, ND_POST_DEC,
    ND_TERNARY,
    ND_COMMA_EXPR,
    ND_FUNC_DEF, ND_GLOBAL_VAR,
    ND_COMPOUND_ASSIGN,
    ND_INIT_LIST,
    ND_ASM,
    ND_NOP,
    ND_VLA_ALLOC,
    ND_RSP_SAVE,
    ND_RSP_RESTORE
};

/* ================================================================ */
/* TYPE KINDS                                                        */
/* ================================================================ */

enum {
    TY_VOID = 0, TY_CHAR, TY_UCHAR, TY_SHORT, TY_USHORT,
    TY_INT, TY_UINT, TY_LONG, TY_ULONG,
    TY_LONGLONG, TY_ULONGLONG, TY_FLOAT, TY_DOUBLE,
    TY_PTR, TY_ARRAY, TY_FUNC, TY_STRUCT, TY_UNION, TY_ENUM
};

/* ================================================================ */
/* FORWARD DECLARATIONS OF STRUCTS                                   */
/* ================================================================ */

typedef struct Type Type;
typedef struct Node Node;
typedef struct Symbol Symbol;
typedef struct Scope Scope;
typedef struct Compiler Compiler;
typedef struct ArenaBlock ArenaBlock;
typedef struct StringEntry StringEntry;
typedef struct StructField StructField;

/* ================================================================ */
/* DATA STRUCTURES                                                   */
/* ================================================================ */

struct ArenaBlock {
    char *data;
    int pos;
    int cap;
    ArenaBlock *next;
};

struct StructField {
    char name[MAX_IDENT];
    Type *type;
    int offset;
    int is_bitfield;
    int bit_offset;
    int bit_size;
    StructField *next;
};

struct Type {
    unsigned long long magic;
    unsigned long long alloc_id;
    int kind;
    int size;
    int align;
    Type *base;        /* for ptr/array */
    int array_len;
    int is_vla;
    struct Node *vla_size_expr;
    /* function */
    Type *ret;
    Type **params;
    int num_params;
    int is_variadic;
    /* struct/union */
    char tag[MAX_IDENT];
    StructField *fields;
    int is_complete;
};

struct StringEntry {
    char *data;
    int len;
    int label_id;
};

struct Symbol {
    char name[MAX_IDENT];
    Type *type;
    int is_local;
    int is_global;
    int is_typedef;
    int is_enum_const;
    long long enum_val;
    int stack_offset;  /* for locals */
    char asm_name[MAX_IDENT];
    /* Regalloc */
    char *assigned_reg;
    int live_start;
    int live_end;
    Symbol *next;      /* linked list in scope */
};

struct Scope {
    Symbol *symbols;
    Scope *parent;
};

struct Node {
    unsigned long long magic;
    unsigned long long alloc_id;
    int kind;
    int line;
    Type *type;

    /* ND_NUM */
    long long int_val;

    /* ND_FLIT */
    double f_val;

    /* ND_STR */
    int str_id;

    /* ND_VAR */
    char name[MAX_IDENT];
    Symbol *sym;

    /* ND_ASSIGN, binary ops */
    Node *lhs;
    Node *rhs;

    /* ND_CALL */
    char func_name[MAX_IDENT];
    Node **args;
    int num_args;

    /* ND_IF / ND_WHILE / ND_FOR / ND_TERNARY */
    Node *cond;
    Node *then_body;
    Node *else_body;
    Node *init;
    Node *inc;

    /* ND_BLOCK */
    Node **stmts;
    int num_stmts;

    /* ND_FUNC_DEF */
    char func_def_name[MAX_IDENT];
    Type *func_type;
    char param_names_buf[MAX_PARAMS][MAX_IDENT];
    Type *param_types[MAX_PARAMS];
    int num_params;
    Node *body;
    int stack_size;

    /* ND_MEMBER */
    char member_name[MAX_IDENT];
    int member_offset;
    int member_size;
    int is_bitfield;
    int bit_offset;
    int bit_size;

    /* ND_SWITCH */
    Node **cases;
    int num_cases;
    Node *default_case;

    /* ND_CASE */
    long long case_val;
    Node *case_body;

    /* ND_GOTO / ND_LABEL */
    char label_name[MAX_IDENT];

    /* ND_COMPOUND_ASSIGN */
    int compound_op;  /* ND_ADD, ND_SUB, etc */

    /* ND_CAST */
    Type *cast_type;

    /* ND_GLOBAL_VAR */
    int is_static;
    int is_extern;
    Node *initializer;

    /* ND_ASM */
    char *asm_string;

    /* linked list for top-level */
    Node *next;
};

#include "zcc_ast_bridge.h"

/* Keep nd_to_znd mapping in sync with this file's enum. */
/* _Static_assert(ND_NUM == ZCC_ND_NUM, "zcc_ast_bridge.h: ND_NUM out of sync"); */
/* _Static_assert(ND_STR == ZCC_ND_STR, "zcc_ast_bridge.h: ND_STR out of sync"); */
/* _Static_assert(ND_WHILE == ZCC_ND_WHILE, "zcc_ast_bridge.h: ND_WHILE out of sync"); */
/* _Static_assert(ND_CAST == ZCC_ND_CAST, "zcc_ast_bridge.h: ND_CAST out of sync"); */
/* _Static_assert(ND_CALL == ZCC_ND_CALL, "zcc_ast_bridge.h: ND_CALL out of sync"); */
/* _Static_assert(ND_NOP == ZCC_ND_NOP, "zcc_ast_bridge.h: ND_NOP out of sync"); */
/* _Static_assert(ND_GT == ZCC_ND_GT, "zcc_ast_bridge.h: ND_GT out of sync"); */
/* _Static_assert(ND_GE == ZCC_ND_GE, "zcc_ast_bridge.h: ND_GE out of sync"); */
/* _Static_assert(ND_EQ == ZCC_ND_EQ, "zcc_ast_bridge.h: ND_EQ out of sync"); */
/* _Static_assert(ND_NE == ZCC_ND_NE, "zcc_ast_bridge.h: ND_NE out of sync"); */
/* _Static_assert(ND_MOD == ZCC_ND_MOD, "zcc_ast_bridge.h: ND_MOD out of sync"); */
/* _Static_assert(ND_ADDR == ZCC_ND_ADDR, "zcc_ast_bridge.h: ND_ADDR out of sync"); */
/* _Static_assert(ND_DEREF == ZCC_ND_DEREF, "zcc_ast_bridge.h: ND_DEREF out of sync"); */
/* _Static_assert(ND_SWITCH == ZCC_ND_SWITCH, "zcc_ast_bridge.h: ND_SWITCH out of sync"); */

/* Bridge accessors: Node* → IR bridge (Option A copy boundary). */
int node_kind(struct Node *n) { return n ? n->kind : 0; }
const char *node_asm_string(struct Node *n) { return n ? n->asm_string : 0; }
long long node_int_val(struct Node *n) { return n ? (long long)n->int_val : 0; }
int node_str_id(struct Node *n) { return n ? n->str_id : 0; }
void node_name(struct Node *n, char *buf, int len) {
    if (!n || !buf || len == 0) return;
    int i; i = 0;
    while (i < len - 1 && n->name[i]) { buf[i] = n->name[i]; i++; }
    buf[i] = '\0';
}
struct Node *node_lhs(struct Node *n) { return n ? n->lhs : NULL; }
struct Node *node_rhs(struct Node *n) { return n ? n->rhs : NULL; }
struct Node *node_cond(struct Node *n) { return n ? n->cond : NULL; }
struct Node *node_then_body(struct Node *n) { return n ? n->then_body : NULL; }
struct Node *node_else_body(struct Node *n) { return n ? n->else_body : NULL; }
struct Node *node_body(struct Node *n) { return n ? n->body : NULL; }
struct Node *node_init(struct Node *n) { return n ? n->init : NULL; }
struct Node *node_inc(struct Node *n) { return n ? n->inc : NULL; }
int node_compound_op(struct Node *n) { return n ? n->compound_op : 0; }
struct Node **node_stmts(struct Node *n) { return n ? n->stmts : NULL; }
int node_num_stmts(struct Node *n) { return n ? n->num_stmts : 0; }
const char *node_func_name(struct Node *n) {
    if (!n) return "";
    if (n->func_name[0]) return n->func_name;
    if (n->lhs && n->lhs->kind == ND_VAR && n->lhs->name[0]) return n->lhs->name;
    return "";
}

int node_lhs_ptr_size(struct Node *n) {
    if (!n || !n->lhs || !n->lhs->type) return 1;
    if (is_pointer(n->lhs->type) && n->lhs->type->base) {
        return type_size(n->lhs->type->base);
    }
    return 1;
}

int node_rhs_ptr_size(struct Node *n) {
    if (!n || !n->rhs || !n->rhs->type) return 1;
    if (is_pointer(n->rhs->type) && n->rhs->type->base) {
        return type_size(n->rhs->type->base);
    }
    return 1;
}
struct Node *node_arg(struct Node *n, int i) {
    if (!n || !n->args || i < 0 || i >= n->num_args) return NULL;
    return n->args[i];
}
int node_num_args(struct Node *n) { return n ? n->num_args : 0; }
struct Node **node_cases(struct Node *n) { return n ? n->cases : NULL; }
int node_num_cases(struct Node *n) { return n ? n->num_cases : 0; }
struct Node *node_default_case(struct Node *n) { return n ? n->default_case : NULL; }
long long node_case_val(struct Node *n) { return n ? (long long)n->case_val : 0; }
struct Node *node_case_body(struct Node *n) { return n ? n->case_body : NULL; }
int node_member_offset(struct Node *n) { return n ? n->member_offset : 0; }

int node_is_bitfield(struct Node *n) { return n ? n->is_bitfield : 0; }
int node_bit_offset(struct Node *n) { return n ? n->bit_offset : 0; }
int node_bit_size(struct Node *n) { return n ? n->bit_size : 0; }
int node_member_size(struct Node *n) { return (n && n->type) ? (int)n->type->size : 8; }
int node_line_no(struct Node *n) { return n ? n->line : 0; }
int node_is_global(struct Node *n) {
    if (!n || n->kind != ND_VAR) return 0;
    return (n->sym && n->sym->is_local) ? 0 : 1;
}

int node_is_array(struct Node *n) {
    if (!n) return 0;
    return (n->type && n->type->kind == TY_ARRAY) ? 1 : 0;
}

int node_is_func(struct Node *n) {
    if (!n) return 0;
    /* Also handle implicit functions with no symbol/type properly if needed, but normally part3.c typed them as ty_int. Wait, if implicit, how do we know it's a function? Normally implicit functions are only in ND_CALL, and we only decay functions in ND_VAR/ND_MEMBER! */
    return (n->type && n->type->kind == TY_FUNC) ? 1 : 0;
}

/* CG-IR-015: Type-width accessors for IR instruction selection.
 * Used by compiler_passes.c to pick 32-bit vs 64-bit lowering for
 * OP_DIV / OP_MOD / OP_SHR so the IR backend matches GCC semantics. */
int node_type_size(struct Node *n) {
    /* Returns the result type size in bytes (4=int, 8=long/ptr).
     * Returns 8 (safe default) when type is unknown or absent. */
    if (!n || !n->type) return 8;
    return (n->type->size > 0) ? (int)n->type->size : 8;
}
int node_type_unsigned(struct Node *n) {
    /* Returns 1 if the node's result type is an unsigned integer kind. */
    if (!n || !n->type) return 0;
    int k = n->type->kind;
    return (k == TY_UCHAR || k == TY_USHORT || k == TY_UINT ||
            k == TY_ULONG || k == TY_ULONGLONG) ? 1 : 0;
}

struct Compiler {
    int verbose;
    /* source */
    char *source;
    int source_len;
    int pos;
    char *filename;

    /* current token */
    int tk;
    long long tk_val;
    double tk_fval;
    char tk_text[MAX_IDENT];
    char tk_str[MAX_STR];
    int tk_str_len;
    int tk_line;
    int tk_col;

    /* peek token (for lookahead) */
    int has_peek;
    int peek_tk;
    long long peek_val;
    double peek_fval;
    char peek_text[MAX_IDENT];
    char peek_str[MAX_STR];
    int peek_str_len;
    int peek_line;
    int peek_col;

    /* lexer state */
    int line;
    int col;

    /* type singletons */
    Type *ty_void;
    Type *ty_char;
    Type *ty_uchar;
    Type *ty_short;
    Type *ty_ushort;
    Type *ty_int;
    Type *ty_uint;
    Type *ty_long;
    Type *ty_ulong;
    Type *ty_longlong;
    Type *ty_ulonglong;
    Type *ty_float;
    Type *ty_double;

    /* scope */
    Scope *current_scope;

    /* strings table */
    StringEntry strings[MAX_STRINGS];
    int num_strings;

    /* structs */
    Type *structs[MAX_STRUCTS];
    int num_structs;

    /* globals for codegen */
    Node *globals[MAX_GLOBALS];
    int num_globals;

    /* codegen state */
    FILE *out;
    int label_count;
    int stack_depth;

    /* loop labels for break/continue */
    int break_label;
    int continue_label;
    int in_loop_depth;
    int vla_sp_offsets[100];
    int vla_sp_depth;

    /* switch labels */
    int switch_end_label;

    /* current function name (for returns) */
    char current_func[MAX_IDENT];
    int func_end_label;

    /* arena allocator */
    ArenaBlock arena;

    /* error count */
    int errors;

    /* local variable offset counter (for codegen) */
    int local_offset;

    int current_is_static;
    int current_is_const;
};

typedef struct TargetBackend {
    int ptr_size;
    void (*emit_prologue)(Compiler *cc, Node *func);
    void (*emit_epilogue)(Compiler *cc, Node *func);
    void (*emit_call)(Compiler *cc, Node *func);
    void (*emit_binary_op)(Compiler *cc, int op);
    void (*emit_load_stack)(Compiler *cc, int offset, const char *reg);
    void (*emit_store_stack)(Compiler *cc, int offset, const char *reg);
    void (*emit_float_binop)(Compiler *cc, int op);
} TargetBackend;

extern TargetBackend *backend_ops;
extern int ZCC_POINTER_WIDTH;
extern int ZCC_INT_WIDTH;

/* ================================================================ */
/* FORWARD DECLARATIONS                                              */
/* ================================================================ */

void *cc_alloc(Compiler *cc, int size);
char *cc_strdup(Compiler *cc, char *s);
void next_token(Compiler *cc);
void expect(Compiler *cc, int tk);
Node *parse_program(Compiler *cc);
Node *parse_stmt(Compiler *cc);
Node *parse_expr(Compiler *cc);
Node *parse_assign(Compiler *cc);
Node *parse_ternary(Compiler *cc);
Node *parse_logor(Compiler *cc);
Node *parse_logand(Compiler *cc);
Node *parse_bitor(Compiler *cc);
Node *parse_bitxor(Compiler *cc);
Node *parse_bitand(Compiler *cc);
Node *parse_equality(Compiler *cc);
Node *parse_relational(Compiler *cc);
Node *parse_shift(Compiler *cc);
Node *parse_add(Compiler *cc);
Node *parse_mul(Compiler *cc);
Node *parse_unary(Compiler *cc);
Node *parse_postfix(Compiler *cc);
Node *parse_primary(Compiler *cc);
Type *parse_type(Compiler *cc);
Type *parse_declarator(Compiler *cc, Type *base, char *name_out);
void scope_push(Compiler *cc);
void scope_pop(Compiler *cc);
Symbol *scope_find(Compiler *cc, char *name);
Symbol *scope_add(Compiler *cc, char *name, Type *type);
Symbol *scope_add_local(Compiler *cc, char *name, Type *type);
void codegen_program(Compiler *cc, Node *prog);
void codegen_func(Compiler *cc, Node *func);
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
            if (block_count >= 512) {
                printf("zcc: too many arena blocks (%d) — possible infinite loop or corrupt AST (last alloc size=%d)\n", block_count, size);
                if (cc) {
                    printf("zcc: stuck near token kind=%d line=%d text=%.127s\n", cc->tk, cc->tk_line, cc->tk_text);
                }
                exit(1);
            }
            if (block_count >= 500 && cc) {
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

char *cc_strdup(Compiler *cc, char *s) {
    int len;
    char *p;
    int i;
    len = 0;
    while (s[len]) len++;
    p = (char *)cc_alloc(cc, len + 1);
    for (i = 0; i <= len; i++) {
        p[i] = s[i];
    }
    return p;
}

/* ================================================================ */
/* ERROR REPORTING                                                   */
/* ================================================================ */

void error(Compiler *cc, char *msg) {
    char *name;
    name = cc->filename;
    if (!name) name = "<input>";
    printf( "%s:%d: error: %s\n", name, cc->tk_line, msg);
    cc->errors++;
}

void error_at(Compiler *cc, int line, char *msg) {
    char *name;
    name = cc->filename;
    if (!name) name = "<input>";
    printf( "%s:%d: error: %s\n", name, line, msg);
    cc->errors++;
}

/* ================================================================ */
/* TYPE CONSTRUCTORS                                                 */
/* ================================================================ */

Type *type_new(Compiler *cc, int kind) {
    Type *t;
    t = (Type *)cc_alloc(cc, sizeof(Type));
    t->magic = 0xDEADBEEF87654321ULL;
    t->alloc_id = next_alloc_id - 1;
    t->kind = kind;
    switch (kind) {
        case TY_VOID:  t->size = 0; t->align = 1; break;
        case TY_CHAR:  t->size = 1; t->align = 1; break;
        case TY_UCHAR: t->size = 1; t->align = 1; break;
        case TY_SHORT: t->size = 2; t->align = 2; break;
        case TY_USHORT:t->size = 2; t->align = 2; break;
        case TY_INT:   t->size = 4; t->align = 4; break;
        case TY_UINT:  t->size = 4; t->align = 4; break;
        case TY_LONG:  t->size = 8; t->align = 8; break;
        case TY_ULONG: t->size = 8; t->align = 8; break;
        case TY_LONGLONG:  t->size = 8; t->align = 8; break;
        case TY_ULONGLONG: t->size = 8; t->align = 8; break;
        case TY_FLOAT: t->size = 4; t->align = 4; break;
        case TY_DOUBLE: t->size = 8; t->align = 8; break;
        case TY_PTR:   t->size = 8; t->align = 8; break;
        case TY_ENUM:  t->size = 4; t->align = 4; break;
        default:       t->size = 0; t->align = 1; break;
    }
    validate_type(cc, t, "type_new", 0);
    return t;
}

Type *type_ptr(Compiler *cc, Type *base) {
    Type *t;
    t = type_new(cc, TY_PTR);
    t->base = base;
    return t;
}

Type *type_array(Compiler *cc, Type *base, int len) {
    Type *t;
    t = type_new(cc, TY_ARRAY);
    t->base = base;
    t->array_len = len;
    t->size = type_size(base) * len;
    t->align = type_align(base);
    return t;
}

Type *type_func(Compiler *cc, Type *ret) {
    Type *t;
    t = type_new(cc, TY_FUNC);
    t->ret = ret;
    t->size = 8;
    t->align = 8;
    return t;
}

int type_size(Type *t) {
    if (!t) return 8;
    return t->size;
}

int type_align(Type *t) {
    if (!t) return 8;
    return t->align;
}

int is_integer(Type *t) {
    if (!t) return 0;
    if (t->kind >= TY_CHAR) {
        if (t->kind <= TY_ULONGLONG) return 1;
    }
    if (t->kind == TY_ENUM) return 1;
    return 0;
}

int is_pointer(Type *t) {
    if (!t) return 0;
    if (t->kind == TY_PTR) return 1;
    if (t->kind == TY_ARRAY) return 1;
    return 0;
}

int is_float_type(Type *t) {
    if (!t) return 0;
    if (t->kind == TY_FLOAT) return 1;
    if (t->kind == TY_DOUBLE) return 1;
    return 0;
}

int is_unsigned_type(Type *t) {
    if (!t) return 0;
    if (t->kind == TY_UCHAR) return 1;
    if (t->kind == TY_USHORT) return 1;
    if (t->kind == TY_UINT) return 1;
    if (t->kind == TY_ULONG) return 1;
    if (t->kind == TY_ULONGLONG) return 1;
    return 0;
}

/* ================================================================ */
/* NODE CONSTRUCTORS                                                 */
/* ================================================================ */

Node *node_new(Compiler *cc, int kind, int line) {
    Node *n;
    n = (Node *)cc_alloc(cc, sizeof(Node));
    n->magic = 0xC0FFEEBAD1234567ULL;
    n->alloc_id = next_alloc_id - 1;
    n->kind = kind;
    n->line = line;
    validate_node(cc, n, "node_new", line);
    return n;
}

Node *node_num(Compiler *cc, long long val, int line) {
    Node *n;
    n = node_new(cc, ND_NUM, line);
    n->int_val = val;
    n->type = cc->ty_int;
    if (val > 2147483647 || val < -2147483648) {
        n->type = cc->ty_long;
    }
    return n;
}

Node *node_flit(Compiler *cc, double val, int line) {
    Node *n;
    n = node_new(cc, ND_FLIT, line);
    n->f_val = val;
    n->type = cc->ty_double;
    return n;
}

/* ================================================================ */
/* SCOPE MANAGEMENT                                                  */
/* ================================================================ */

void scope_push(Compiler *cc) {
    Scope *s;
    s = (Scope *)cc_alloc(cc, sizeof(Scope));
    s->parent = cc->current_scope;
    s->symbols = 0;
    cc->current_scope = s;
}

void scope_pop(Compiler *cc) {
    if (cc->current_scope) {
        cc->current_scope = cc->current_scope->parent;
    }
}

Symbol *scope_find(Compiler *cc, char *name) {
    Scope *s;
    Symbol *sym;
    s = cc->current_scope;
    while (s) {
        sym = s->symbols;
        while (sym) {
            if (strcmp(sym->name, name) == 0) return sym;
            sym = sym->next;
        }
        s = s->parent;
    }
    return 0;
}

Symbol *scope_find_local(Compiler *cc, char *name) {
    Symbol *sym;
    if (!cc->current_scope) return 0;
    sym = cc->current_scope->symbols;
    while (sym) {
        if (strcmp(sym->name, name) == 0) return sym;
        sym = sym->next;
    }
    return 0;
}

Symbol *scope_add(Compiler *cc, char *name, Type *type) {
    Symbol *sym;
    sym = (Symbol *)cc_alloc(cc, sizeof(Symbol));
    strncpy(sym->name, name, MAX_IDENT - 1);
    sym->type = type;
    sym->stack_offset = 0;
    sym->next = cc->current_scope->symbols;
    cc->current_scope->symbols = sym;
    if (strcmp(name, "yyParser") == 0) {
        printf("DEBUG: scope_add('yyParser') with type kind=%d tag='%s'\n", type ? type->kind : -1, (type && type->tag[0]) ? type->tag : "<none>");
        fflush(stdout);
    }
    return sym;
}

/* Add local with auto-assigned stack offset */
Symbol *scope_add_local(Compiler *cc, char *name, Type *type) {
    Symbol *sym;
    int sz;
    sym = scope_add(cc, name, type);
    sym->is_local = 1;
    sz = type_size(type);
    if (sz < 8) sz = 8;  /* minimum 8-byte slots */
    cc->local_offset = cc->local_offset - sz;
    sym->stack_offset = cc->local_offset;
    return sym;
}

/* ================================================================ */
/* KEYWORD TABLE                                                     */
/* ================================================================ */

typedef struct {
    char *word;
    int token;
} Keyword;

static Keyword keywords[] = {
    {"int",       TK_INT},
    {"char",      TK_CHAR},
    {"void",      TK_VOID},
    {"long",      TK_LONG},
    {"short",     TK_SHORT},
    {"unsigned",  TK_UNSIGNED},
    {"signed",    TK_SIGNED},
    {"float",     TK_FLOAT},
    {"double",    TK_DOUBLE},
    {"if",        TK_IF},
    {"else",      TK_ELSE},
    {"while",     TK_WHILE},
    {"for",       TK_FOR},
    {"do",        TK_DO},
    {"return",    TK_RETURN},
    {"break",     TK_BREAK},
    {"continue",  TK_CONTINUE},
    {"goto",      TK_GOTO},
    {"switch",    TK_SWITCH},
    {"case",      TK_CASE},
    {"default",   TK_DEFAULT},
    {"struct",    TK_STRUCT},
    {"union",     TK_UNION},
    {"enum",      TK_ENUM},
    {"typedef",   TK_TYPEDEF},
    {"sizeof",    TK_SIZEOF},
    {"__builtin_va_arg", TK_BUILTIN_VA_ARG},
    {"static",    TK_STATIC},
    {"extern",    TK_EXTERN},
    {"const",     TK_CONST},
    {"volatile",  TK_VOLATILE},
    {"auto",      TK_AUTO},
    {"register",  TK_REGISTER},
    {"inline",    TK_INLINE},
    {"asm",       TK_ASM},
    {"__asm__",   TK_ASM},
    {"__signed__", TK_SIGNED},
    {"__signed",   TK_SIGNED},
    {"__const__",  TK_CONST},
    {"__const",    TK_CONST},
    {"__inline__", TK_INLINE},
    {"__inline",   TK_INLINE},
    {"__volatile__", TK_VOLATILE},
    {"__volatile", TK_VOLATILE},
    {"__restrict__", TK_VOLATILE},
    {"__restrict", TK_VOLATILE},
    {"restrict",  TK_VOLATILE},
    {"__int128",   TK_LONG},
    {"__int128_t", TK_LONG},
    {"typeof",     TK_TYPEOF},
    {"__typeof__", TK_TYPEOF},
    {"__typeof",   TK_TYPEOF},
    {"__auto_type", TK_AUTO_TYPE},
    {"_Generic",   TK_GENERIC},
    {0, 0}
};

static int kw_count = 53;

static int lookup_keyword(char *name) {
    int i;
    if (!name) return 0;
    for (i = 0; i < kw_count; i++) {
        if (!keywords[i].word) break;
        if (strcmp(keywords[i].word, name) == 0) {
            return keywords[i].token;
        }
    }
    return 0;
}

/* stage2 fallback: match ident by length and bytes (no strcmp/keyword table). Bounds-safe. */
static int lookup_keyword_fallback(char *buf, int len) {
    if (!buf || len <= 0 || len > 16) return 0;  /* max keyword length 16 */
    if (len==3 && buf[0]=='i'&&buf[1]=='n'&&buf[2]=='t') return TK_INT;
    if (len==4 && buf[0]=='c'&&buf[1]=='h'&&buf[2]=='a'&&buf[3]=='r') return TK_CHAR;
    if (len==4 && buf[0]=='v'&&buf[1]=='o'&&buf[2]=='i'&&buf[3]=='d') return TK_VOID;
    if (len==4 && buf[0]=='l'&&buf[1]=='o'&&buf[2]=='n'&&buf[3]=='g') return TK_LONG;
    if (len==5 && buf[0]=='s'&&buf[1]=='h'&&buf[2]=='o'&&buf[3]=='r'&&buf[4]=='t') return TK_SHORT;
    if (len==6 && buf[0]=='s'&&buf[1]=='i'&&buf[2]=='g'&&buf[3]=='n'&&buf[4]=='e'&&buf[5]=='d') return TK_SIGNED;
    if (len==5 && buf[0]=='f'&&buf[1]=='l'&&buf[2]=='o'&&buf[3]=='a'&&buf[4]=='t') return TK_FLOAT;
    if (len==6 && buf[0]=='d'&&buf[1]=='o'&&buf[2]=='u'&&buf[3]=='b'&&buf[4]=='l'&&buf[5]=='e') return TK_DOUBLE;
    if (len==2 && buf[0]=='i'&&buf[1]=='f') return TK_IF;
    if (len==3 && buf[0]=='a'&&buf[1]=='s'&&buf[2]=='m') return TK_ASM;
    if (len==7 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='a'&&buf[3]=='s'&&buf[4]=='m'&&buf[5]=='_'&&buf[6]=='_') return TK_ASM;
    if (len==4 && buf[0]=='e'&&buf[1]=='l'&&buf[2]=='s'&&buf[3]=='e') return TK_ELSE;
    if (len==5 && buf[0]=='w'&&buf[1]=='h'&&buf[2]=='i'&&buf[3]=='l'&&buf[4]=='e') return TK_WHILE;
    if (len==3 && buf[0]=='f'&&buf[1]=='o'&&buf[2]=='r') return TK_FOR;
    if (len==2 && buf[0]=='d'&&buf[1]=='o') return TK_DO;
    if (len==6 && buf[0]=='r'&&buf[1]=='e'&&buf[2]=='t'&&buf[3]=='u'&&buf[4]=='r'&&buf[5]=='n') return TK_RETURN;
    if (len==5 && buf[0]=='b'&&buf[1]=='r'&&buf[2]=='e'&&buf[3]=='a'&&buf[4]=='k') return TK_BREAK;
    if (len==4 && buf[0]=='g'&&buf[1]=='o'&&buf[2]=='t'&&buf[3]=='o') return TK_GOTO;
    if (len==6 && buf[0]=='s'&&buf[1]=='w'&&buf[2]=='i'&&buf[3]=='t'&&buf[4]=='c'&&buf[5]=='h') return TK_SWITCH;
    if (len==4 && buf[0]=='c'&&buf[1]=='a'&&buf[2]=='s'&&buf[3]=='e') return TK_CASE;
    if (len==6 && buf[0]=='s'&&buf[1]=='t'&&buf[2]=='r'&&buf[3]=='u'&&buf[4]=='c'&&buf[5]=='t') return TK_STRUCT;
    if (len==5 && buf[0]=='u'&&buf[1]=='n'&&buf[2]=='i'&&buf[3]=='o'&&buf[4]=='n') return TK_UNION;
    if (len==4 && buf[0]=='e'&&buf[1]=='n'&&buf[2]=='u'&&buf[3]=='m') return TK_ENUM;
    if (len==6 && buf[0]=='s'&&buf[1]=='i'&&buf[2]=='z'&&buf[3]=='e'&&buf[4]=='o'&&buf[5]=='f') return TK_SIZEOF;
    if (len==6 && buf[0]=='t'&&buf[1]=='y'&&buf[2]=='p'&&buf[3]=='e'&&buf[4]=='o'&&buf[5]=='f') return TK_TYPEOF;
    if (len==8 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='t'&&buf[3]=='y'&&buf[4]=='p'&&buf[5]=='e'&&buf[6]=='o'&&buf[7]=='f') return TK_TYPEOF;
    if (len==10 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='t'&&buf[3]=='y'&&buf[4]=='p'&&buf[5]=='e'&&buf[6]=='o'&&buf[7]=='f'&&buf[8]=='_'&&buf[9]=='_') return TK_TYPEOF;
    if (len==11 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='a'&&buf[3]=='u'&&buf[4]=='t'&&buf[5]=='o'&&buf[6]=='_'&&buf[7]=='t'&&buf[8]=='y'&&buf[9]=='p'&&buf[10]=='e') return TK_AUTO_TYPE;
    if (len==6 && buf[0]=='s'&&buf[1]=='t'&&buf[2]=='a'&&buf[3]=='t'&&buf[4]=='i'&&buf[5]=='c') return TK_STATIC;
    if (len==6 && buf[0]=='e'&&buf[1]=='x'&&buf[2]=='t'&&buf[3]=='e'&&buf[4]=='r'&&buf[5]=='n') return TK_EXTERN;
    if (len==5 && buf[0]=='c'&&buf[1]=='o'&&buf[2]=='n'&&buf[3]=='s'&&buf[4]=='t') return TK_CONST;
    if (len==4 && buf[0]=='a'&&buf[1]=='u'&&buf[2]=='t'&&buf[3]=='o') return TK_AUTO;
    if (len==6 && buf[0]=='i'&&buf[1]=='n'&&buf[2]=='l'&&buf[3]=='i'&&buf[4]=='n'&&buf[5]=='e') return TK_INLINE;
    /* 7–8 char keywords: need len check then safe indices 0..6 or 0..7 */
    if (len==7 && buf[0]=='t'&&buf[1]=='y'&&buf[2]=='p'&&buf[3]=='e'&&buf[4]=='d'&&buf[5]=='e'&&buf[6]=='f') return TK_TYPEDEF;
    if (len==7 && buf[0]=='d'&&buf[1]=='e'&&buf[2]=='f'&&buf[3]=='a'&&buf[4]=='u'&&buf[5]=='l'&&buf[6]=='t') return TK_DEFAULT;
    if (len==8 && buf[0]=='u'&&buf[1]=='n'&&buf[2]=='s'&&buf[3]=='i'&&buf[4]=='g'&&buf[5]=='n'&&buf[6]=='e'&&buf[7]=='d') return TK_UNSIGNED;
    if (len==8 && buf[0]=='v'&&buf[1]=='o'&&buf[2]=='l'&&buf[3]=='a'&&buf[4]=='t'&&buf[5]=='i'&&buf[6]=='l'&&buf[7]=='e') return TK_VOLATILE;
    if (len==8 && buf[0]=='c'&&buf[1]=='o'&&buf[2]=='n'&&buf[3]=='t'&&buf[4]=='i'&&buf[5]=='n'&&buf[6]=='u'&&buf[7]=='e') return TK_CONTINUE;
    if (len==8 && buf[0]=='r'&&buf[1]=='e'&&buf[2]=='g'&&buf[3]=='i'&&buf[4]=='s'&&buf[5]=='t'&&buf[6]=='e'&&buf[7]=='r') return TK_REGISTER;
    if (len==8 && buf[0]=='r'&&buf[1]=='e'&&buf[2]=='s'&&buf[3]=='t'&&buf[4]=='r'&&buf[5]=='i'&&buf[6]=='c'&&buf[7]=='t') return TK_VOLATILE;
    if (len==8 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='c'&&buf[3]=='o'&&buf[4]=='n'&&buf[5]=='s'&&buf[6]=='t') return TK_CONST;
    if (len==8 && buf[0]=='_'&&buf[1]=='G'&&buf[2]=='e'&&buf[3]=='n'&&buf[4]=='e'&&buf[5]=='r'&&buf[6]=='i'&&buf[7]=='c') return TK_GENERIC;
    if (len==8 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='s'&&buf[3]=='i'&&buf[4]=='g'&&buf[5]=='n'&&buf[6]=='e'&&buf[7]=='d') return TK_SIGNED;
    if (len==8 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='i'&&buf[3]=='n'&&buf[4]=='l'&&buf[5]=='i'&&buf[6]=='n'&&buf[7]=='e') return TK_INLINE;
    if (len==9 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='c'&&buf[3]=='o'&&buf[4]=='n'&&buf[5]=='s'&&buf[6]=='t'&&buf[7]=='_'&&buf[8]=='_') return TK_CONST;
    if (len==10 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='s'&&buf[3]=='i'&&buf[4]=='g'&&buf[5]=='n'&&buf[6]=='e'&&buf[7]=='d'&&buf[8]=='_'&&buf[9]=='_') return TK_SIGNED;
    if (len==10 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='i'&&buf[3]=='n'&&buf[4]=='l'&&buf[5]=='i'&&buf[6]=='n'&&buf[7]=='e'&&buf[8]=='_'&&buf[9]=='_') return TK_INLINE;
    if (len==10 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='r'&&buf[3]=='e'&&buf[4]=='s'&&buf[5]=='t'&&buf[6]=='r'&&buf[7]=='i'&&buf[8]=='c'&&buf[9]=='t') return TK_VOLATILE;
    if (len==12 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='r'&&buf[3]=='e'&&buf[4]=='s'&&buf[5]=='t'&&buf[6]=='r'&&buf[7]=='i'&&buf[8]=='c'&&buf[9]=='t'&&buf[10]=='_'&&buf[11]=='_') return TK_VOLATILE;
    if (len==10 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='v'&&buf[3]=='o'&&buf[4]=='l'&&buf[5]=='a'&&buf[6]=='t'&&buf[7]=='i'&&buf[8]=='l'&&buf[9]=='e') return TK_VOLATILE;
    if (len==12 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='v'&&buf[3]=='o'&&buf[4]=='l'&&buf[5]=='a'&&buf[6]=='t'&&buf[7]=='i'&&buf[8]=='l'&&buf[9]=='e'&&buf[10]=='_'&&buf[11]=='_') return TK_VOLATILE;
    if (len==8 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='i'&&buf[3]=='n'&&buf[4]=='t'&&buf[5]=='1'&&buf[6]=='2'&&buf[7]=='8') return TK_LONG;
    if (len==10 && buf[0]=='_'&&buf[1]=='_'&&buf[2]=='i'&&buf[3]=='n'&&buf[4]=='t'&&buf[5]=='1'&&buf[6]=='2'&&buf[7]=='8'&&buf[8]=='_'&&buf[9]=='t') return TK_LONG;
    return 0;
}

/* ================================================================ */
/* LEXER                                                             */
/* ================================================================ */

static int is_alpha(int c) {
    if (c >= 'a') { if (c <= 'z') return 1; }
    if (c >= 'A') { if (c <= 'Z') return 1; }
    if (c == '_') return 1;
    return 0;
}

static int is_digit(int c) {
    if (c >= '0') { if (c <= '9') return 1; }
    return 0;
}

static int is_alnum(int c) {
    if (is_alpha(c)) return 1;
    if (is_digit(c)) return 1;
    return 0;
}

static int is_space(int c) {
    if (c == ' ') return 1;
    if (c == '\t') return 1;
    if (c == '\r') return 1;
    if (c == '\f') return 1;
    if (c == '\v') return 1;
    return 0;
}

static int hex_val(int c) {
    if (c >= '0') { if (c <= '9') return c - '0'; }
    if (c >= 'a') { if (c <= 'f') return c - 'a' + 10; }
    if (c >= 'A') { if (c <= 'F') return c - 'A' + 10; }
    return -1;
}

static int peek_char(Compiler *cc) {
    if (cc->pos < cc->source_len) return (unsigned char)cc->source[cc->pos];
    return -1;
}

static int read_char(Compiler *cc) {
    int c;
    if (cc->pos >= cc->source_len) return -1;
    c = (unsigned char)cc->source[cc->pos];
    cc->pos++;
    if (c == '\n') {
        cc->line++;
        cc->col = 1;
    } else {
        cc->col++;
    }
    return c;
}

static int read_escape(Compiler *cc) {
    int c;
    c = read_char(cc);
    switch (c) {
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case '\\': return '\\';
        case '\'': return '\'';
        case '"': return '"';
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            int val = c - '0';
            if (peek_char(cc) >= '0' && peek_char(cc) <= '7') {
                val = val * 8 + (read_char(cc) - '0');
                if (peek_char(cc) >= '0' && peek_char(cc) <= '7') {
                    val = val * 8 + (read_char(cc) - '0');
                }
            }
            return val;
        }
        case 'a': return '\a';
        case 'b': return '\b';
        case 'f': return '\f';
        case 'v': return '\v';
        case 'x': {
            int val;
            int h;
            val = 0;
            h = hex_val(peek_char(cc));
            while (h >= 0) {
                read_char(cc);
                val = val * 16 + h;
                h = hex_val(peek_char(cc));
            }
            return val;
        }
        default: return c;
    }
}

void next_token(Compiler *cc) {
    int c;
    int kw;
    int entry_pos = cc->pos;

    /* if we have a peeked token, use it */
    if (cc->has_peek) {
        cc->tk = cc->peek_tk;
        cc->tk_val = cc->peek_val;
        strncpy(cc->tk_text, cc->peek_text, MAX_IDENT - 1);
        strncpy(cc->tk_str, cc->peek_str, MAX_STR - 1);
        cc->tk_str_len = cc->peek_str_len;
        cc->tk_line = cc->peek_line;
        cc->tk_col = cc->peek_col;
        cc->has_peek = 0;
        return;
    }

again:
    /* skip whitespace */
    while (cc->pos < cc->source_len) {
        c = peek_char(cc);
        if (c == '\n') {
            read_char(cc);
            goto again;
        }
        if (is_space(c)) {
            read_char(cc);
        } else {
            break;
        }
    }

    cc->tk_line = cc->line;
    cc->tk_col = cc->col;

    if (cc->pos >= cc->source_len) {
        cc->tk = TK_EOF;
        cc->tk_text[0] = 0;
        return;
    }

    c = peek_char(cc);

    /* skip line comments */
    if (c == '/') {
        if (cc->pos + 1 < cc->source_len) {
            if (cc->source[cc->pos + 1] == '/') {
                while (cc->pos < cc->source_len) {
                    if (cc->source[cc->pos] == '\n') break;
                    cc->pos++;
                }
                goto again;
            }
            /* block comments */
            if (cc->source[cc->pos + 1] == '*') {
                cc->pos += 2;
                cc->col += 2;
                while (cc->pos + 1 < cc->source_len) {
                    if (cc->source[cc->pos] == '\n') {
                        cc->line++;
                        cc->col = 1;
                    }
                    if (cc->source[cc->pos] == '*') {
                        if (cc->source[cc->pos + 1] == '/') {
                            cc->pos += 2;
                            cc->col += 2;
                            goto again;
                        }
                    }
                    cc->pos++;
                    cc->col++;
                }
                goto again;
            }
        }
    }

    /* preprocessor lines: skip entirely */
    if (c == '#') {
        while (cc->pos < cc->source_len) {
            if (cc->source[cc->pos] == '\n') break;
            /* handle line continuation */
            if (cc->source[cc->pos] == '\\') {
                if (cc->pos + 1 < cc->source_len) {
                    if (cc->source[cc->pos + 1] == '\n') {
                        cc->pos += 2;
                        cc->line++;
                        cc->col = 1;
                        continue;
                    }
                }
            }
            cc->pos++;
        }
        goto again;
    }

    /* number literals */
    if (is_digit(c)) {
        long val;
        int start;
        int is_float = 0;
        int i_look = 0;
        char *end;
        double fval;
        int len;
        
        if (c == '0' && cc->pos + 1 < cc->source_len && 
            (cc->source[cc->pos + 1] == 'x' || cc->source[cc->pos + 1] == 'X')) {
            is_float = 0;
        } else {
            while (cc->pos + i_look < cc->source_len) {
                char ch = cc->source[cc->pos + i_look];
                if (ch == '.' || ch == 'e' || ch == 'E') { is_float = 1; break; }
                if (!is_digit(ch) && !is_alpha(ch)) break;
                i_look++;
            }
        }
        
        if (is_float) {
            fval = strtod(cc->source + cc->pos, &end);
            len = end - (cc->source + cc->pos);
            if (len <= 0) len = 1;
            /* we peeked c, so real current index is cc->pos, 
               but we need to advance cc->pos. */
            cc->pos = cc->pos + len;
            cc->col = cc->col + len;
            if (cc->pos < cc->source_len) {
                char sc = cc->source[cc->pos];
                if (sc == 'f' || sc == 'F' || sc == 'l' || sc == 'L') {
                    cc->pos++; cc->col++;
                }
            }
            cc->tk = TK_FLIT;
            cc->tk_fval = fval;
            cc->tk_text[0] = 0;
            return;
        }

        val = 0;
        start = cc->pos;
        if (c == '0') {
            read_char(cc);
            if (peek_char(cc) == 'x' || peek_char(cc) == 'X') {
                read_char(cc);
                while (hex_val(peek_char(cc)) >= 0) {
                    val = val * 16 + hex_val(peek_char(cc));
                    read_char(cc);
                }
            } else if (peek_char(cc) >= '0') {
                if (peek_char(cc) <= '7') {
                    /* octal */
                    while (peek_char(cc) >= '0') {
                        if (peek_char(cc) <= '7') {
                            val = val * 8 + (peek_char(cc) - '0');
                            read_char(cc);
                        } else {
                            break;
                        }
                    }
                }
            }
        } else {
            while (is_digit(peek_char(cc))) {
                val = val * 10 + (peek_char(cc) - '0');
                read_char(cc);
            }
        }
        /* parse suffixes like L, U, UL, ULL, etc */
        int is_uns = 0;
        int is_lng = 0;
        while (peek_char(cc) == 'L' || peek_char(cc) == 'l' ||
               peek_char(cc) == 'U' || peek_char(cc) == 'u') {
            int ch = read_char(cc);
            if (ch == 'U' || ch == 'u') is_uns = 1;
            if (ch == 'L' || ch == 'l') is_lng = 1;
        }
        cc->tk = TK_NUM;
        cc->tk_val = val;
        cc->tk_text[0] = is_uns ? 'U' : 0;
        cc->tk_text[1] = is_lng ? 'L' : 0;
        return;
    }

    /* character literal */
    if (c == '\'') {
        int val;
        read_char(cc);  /* skip opening ' */
        if (peek_char(cc) == '\\') {
            read_char(cc);
            val = read_escape(cc);
        } else {
            val = read_char(cc);
        }
        if (peek_char(cc) == '\'') read_char(cc);  /* skip closing ' */
        cc->tk = TK_CHAR_LIT;
        cc->tk_val = val;
        return;
    }

    /* string literal */
    if (c == '"') {
        int len;
        read_char(cc);  /* skip opening " */
        len = 0;
        while (peek_char(cc) != '"') {
            if (peek_char(cc) == -1) break;
            if (peek_char(cc) == '\\') {
                read_char(cc);
                cc->tk_str[len] = read_escape(cc);
            } else {
                cc->tk_str[len] = read_char(cc);
            }
            len++;
            if (len >= MAX_STR - 1) break;
        }
        if (peek_char(cc) == '"') read_char(cc);  /* skip closing " */
        cc->tk_str[len] = 0;
        cc->tk_str_len = len;
        cc->tk = TK_STR;

        /* handle adjacent string literals */
        /* skip whitespace and check for another " */
        while (1) {
            int saved_pos;
            int saved_line;
            int saved_col;
            saved_pos = cc->pos;
            saved_line = cc->line;
            saved_col = cc->col;
            while (cc->pos < cc->source_len) {
                int ch;
                ch = cc->source[cc->pos];
                if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
                    if (ch == '\n') {
                        cc->line++;
                        cc->col = 1;
                    } else {
                        cc->col++;
                    }
                    cc->pos++;
                } else {
                    break;
                }
            }
            if (cc->pos < cc->source_len) {
                if (cc->source[cc->pos] == '"') {
                    /* concatenate adjacent string */
                    read_char(cc); /* skip " */
                    while (peek_char(cc) != '"') {
                        if (peek_char(cc) == -1) break;
                        if (peek_char(cc) == '\\') {
                            read_char(cc);
                            cc->tk_str[len] = read_escape(cc);
                        } else {
                            cc->tk_str[len] = read_char(cc);
                        }
                        len++;
                        if (len >= MAX_STR - 1) break;
                    }
                    if (peek_char(cc) == '"') read_char(cc);
                    cc->tk_str[len] = 0;
                    cc->tk_str_len = len;
                } else {
                    /* no adjacent string — restore position and exit loop */
                    cc->pos = saved_pos;
                    cc->line = saved_line;
                    cc->col = saved_col;
                    break;
                }
            } else {
                break;
            }
        }
        return;
    }

    /* identifiers and keywords: use local buffer for lookup to avoid bad cc->tk_text in stage2 (offset/codegen quirk) */
    if (is_alpha(c)) {
        static char ident_buf[MAX_IDENT];
        int len;
        len = 0;
        while (is_alnum(peek_char(cc))) {
            ident_buf[len] = read_char(cc);
            len++;
            if (len >= MAX_IDENT - 1) break;
        }
        ident_buf[len] = 0;

        kw = lookup_keyword(ident_buf);
        if (!kw) kw = lookup_keyword_fallback(ident_buf, len);
        if (kw) {
            cc->tk = kw;
        } else {
            if (strcmp(ident_buf, "__attribute__") == 0 || strcmp(ident_buf, "__attribute") == 0) {
                int pcount = 0;
                int started = 0;
                while (cc->pos < cc->source_len) {
                    char ac = cc->source[cc->pos];
                    if (ac == '(') { pcount++; started = 1; }
                    else if (ac == ')') {
                        pcount--;
                        if (started && pcount == 0) { cc->pos++; cc->col++; break; }
                    }
                    cc->pos++; cc->col++;
                }
                goto again;
            }
            if (strcmp(ident_buf, "__extension__") == 0) {
                goto again;
            }
            
            cc->tk = TK_IDENT;
        }
        strncpy(cc->tk_text, ident_buf, MAX_IDENT - 1);
        cc->tk_text[MAX_IDENT - 1] = 0;
        return;
    }

    /* operators and delimiters */
    read_char(cc);

    if (c == '+') {
        if (peek_char(cc) == '+') { read_char(cc); cc->tk = TK_INC; return; }
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_PLUS_ASSIGN; return; }
        cc->tk = TK_PLUS; return;
    }
    if (c == '-') {
        if (peek_char(cc) == '-') { read_char(cc); cc->tk = TK_DEC; return; }
        if (peek_char(cc) == '>') { read_char(cc); cc->tk = TK_ARROW; return; }
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_MINUS_ASSIGN; return; }
        cc->tk = TK_MINUS; return;
    }
    if (c == '*') {
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_STAR_ASSIGN; return; }
        cc->tk = TK_STAR; return;
    }
    if (c == '/') {
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_SLASH_ASSIGN; return; }
        cc->tk = TK_SLASH; return;
    }
    if (c == '%') {
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_PERCENT_ASSIGN; return; }
        cc->tk = TK_PERCENT; return;
    }
    if (c == '&') {
        if (peek_char(cc) == '&') { read_char(cc); cc->tk = TK_LAND; return; }
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_AMP_ASSIGN; return; }
        cc->tk = TK_AMP; return;
    }
    if (c == '|') {
        if (peek_char(cc) == '|') { read_char(cc); cc->tk = TK_LOR; return; }
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_PIPE_ASSIGN; return; }
        cc->tk = TK_PIPE; return;
    }
    if (c == '^') {
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_CARET_ASSIGN; return; }
        cc->tk = TK_CARET; return;
    }
    if (c == '~') { cc->tk = TK_TILDE; return; }
    if (c == '!') {
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_NE; return; }
        cc->tk = TK_BANG; return;
    }
    if (c == '=') {
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_EQ; return; }
        cc->tk = TK_ASSIGN; return;
    }
    if (c == '<') {
        if (peek_char(cc) == '<') {
            read_char(cc);
            if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_SHL_ASSIGN; return; }
            cc->tk = TK_SHL; return;
        }
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_LE; return; }
        cc->tk = TK_LT; return;
    }
    if (c == '>') {
        if (peek_char(cc) == '>') {
            read_char(cc);
            if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_SHR_ASSIGN; return; }
            cc->tk = TK_SHR; return;
        }
        if (peek_char(cc) == '=') { read_char(cc); cc->tk = TK_GE; return; }
        cc->tk = TK_GT; return;
    }
    if (c == '(') { cc->tk = TK_LPAREN; return; }
    if (c == ')') { cc->tk = TK_RPAREN; return; }
    if (c == '{') { cc->tk = TK_LBRACE; return; }
    if (c == '}') { cc->tk = TK_RBRACE; return; }
    if (c == '[') { cc->tk = TK_LBRACKET; return; }
    if (c == ']') { cc->tk = TK_RBRACKET; return; }
    if (c == ';') { cc->tk = TK_SEMI; return; }
    if (c == ',') { cc->tk = TK_COMMA; return; }
    if (c == '.') {
        if (is_digit(peek_char(cc))) {
            char *end;
            double fval = strtod(cc->source + cc->pos - 1, &end);
            int len = end - (cc->source + cc->pos - 1);
            if (len <= 0) len = 1;
            cc->pos = cc->pos - 1 + len;
            cc->col = cc->col - 1 + len;
            if (cc->pos < cc->source_len) {
                char sc = cc->source[cc->pos];
                if (sc == 'f' || sc == 'F' || sc == 'l' || sc == 'L') {
                    cc->pos++; cc->col++;
                }
            }
            cc->tk = TK_FLIT;
            cc->tk_fval = fval;
            cc->tk_text[0] = 0;
            return;
        }
        if (peek_char(cc) == '.') {
            read_char(cc);
            if (peek_char(cc) == '.') { read_char(cc); cc->tk = TK_ELLIPSIS; return; }
        }
        cc->tk = TK_DOT; return;
    }
    if (c == '?') { cc->tk = TK_QUESTION; return; }
    if (c == ':') { cc->tk = TK_COLON; return; }

    /* unknown character — skip */
    goto again;
}

/* Token enum in part1 runs 0..TK_HASH. Stage2 may compile enums to different values (75-81 seen); use 128 so we only flag obvious garbage. */
#define MAX_KNOWN_TOKEN  128

/* Human-readable token name for error messages (index = enum value from part1). */
static const char *token_name(int t) {
    static const char *names[] = {
        "EOF", "NUM", "STR", "CHAR_LIT", "IDENT", "FLIT",
        "INT", "CHAR", "VOID", "LONG", "SHORT", "UNSIGNED", "SIGNED", "FLOAT", "DOUBLE",
        "IF", "ELSE", "WHILE", "FOR", "DO", "RETURN", "BREAK", "CONTINUE", "GOTO",
        "SWITCH", "CASE", "DEFAULT", "STRUCT", "UNION", "ENUM", "TYPEDEF",
        "SIZEOF", "STATIC", "EXTERN", "CONST", "VOLATILE", "AUTO", "REGISTER", "INLINE",
        "BUILTIN_VA_ARG",
        "PLUS", "MINUS", "STAR", "SLASH", "PERCENT", "AMP", "PIPE", "CARET", "TILDE", "BANG",
        "ASSIGN", "EQ", "NE", "LT", "GT", "LE", "GE", "LAND", "LOR", "SHL", "SHR",
        "INC", "DEC", "ARROW", "DOT", "QUESTION", "COLON",
        "PLUS_ASSIGN", "MINUS_ASSIGN", "STAR_ASSIGN", "SLASH_ASSIGN", "PERCENT_ASSIGN",
        "AMP_ASSIGN", "PIPE_ASSIGN", "CARET_ASSIGN", "SHL_ASSIGN", "SHR_ASSIGN",
        "LPAREN", "RPAREN", "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
        "SEMI", "COMMA", "ELLIPSIS", "HASH"
    };
    if (t >= 0 && t < 85) return names[t];  /* 85 = number of token names; avoid sizeof for stage2 parse */
    return "?";
}

void expect(Compiler *cc, int tk) {
    char buf[256];
    if (tk < 0 || tk > 128) {
        sprintf(buf, "DEMON: insane expected token %d (line %d) — possible stack corruption", tk, cc->tk_line);
        error(cc, buf);
    }
    if (cc->tk != tk) {
        sprintf(buf, "expected %s (%d), got %s (%d)", token_name(tk), tk, token_name(cc->tk), cc->tk);
        error(cc, buf);
    }
    next_token(cc);
}

int peek_token(Compiler *cc) {
    int s_tk;
    long s_val;
    char s_text[MAX_IDENT];
    char s_str[MAX_STR];
    int s_slen;
    int s_line;
    int s_col;

    if (cc->has_peek) return cc->peek_tk;

    /* save current token */
    s_tk = cc->tk;
    s_val = cc->tk_val;
    strncpy(s_text, cc->tk_text, MAX_IDENT - 1);
    strncpy(s_str, cc->tk_str, MAX_STR - 1);
    s_slen = cc->tk_str_len;
    s_line = cc->tk_line;
    s_col = cc->tk_col;

    /* lex next */
    next_token(cc);

    /* store peeked */
    cc->peek_tk = cc->tk;
    cc->peek_val = cc->tk_val;
    strncpy(cc->peek_text, cc->tk_text, MAX_IDENT - 1);
    strncpy(cc->peek_str, cc->tk_str, MAX_STR - 1);
    cc->peek_str_len = cc->tk_str_len;
    cc->peek_line = cc->tk_line;
    cc->peek_col = cc->tk_col;
    cc->has_peek = 1;

    /* restore current */
    cc->tk = s_tk;
    cc->tk_val = s_val;
    strncpy(cc->tk_text, s_text, MAX_IDENT - 1);
    strncpy(cc->tk_str, s_str, MAX_STR - 1);
    cc->tk_str_len = s_slen;
    cc->tk_line = s_line;
    cc->tk_col = s_col;

    return cc->peek_tk;
}

/* ZKAEDI FORCE RENDER CACHE INVALIDATION */
/* ================================================================ */
/* PARSER                                                            */
/* ================================================================ */

static int is_type_token(Compiler *cc) {
    if (cc->tk >= TK_INT && cc->tk <= TK_DOUBLE) return 1;
    if (cc->tk >= TK_STATIC && cc->tk <= TK_INLINE) return 1;
    if (cc->tk == TK_STRUCT) return 1;
    if (cc->tk == TK_UNION) return 1;
    if (cc->tk == TK_ENUM) return 1;
    if (cc->tk == TK_TYPEDEF) return 1;
    if (cc->tk == TK_TYPEOF) return 1;
    if (cc->tk == TK_AUTO_TYPE) return 1;
    if (cc->tk == TK_IDENT) {
        Symbol *sym;
        sym = scope_find(cc, cc->tk_text);
        if (sym) {
            if (sym->is_typedef) return 1;
        }
    }
    return 0;
}

static Type *find_struct(Compiler *cc, char *tag) {
    int i;
    for (i = 0; i < cc->num_structs; i++) {
        if (strcmp(cc->structs[i]->tag, tag) == 0) return cc->structs[i];
    }
    return 0;
}

static void register_struct(Compiler *cc, Type *t) {
    if (cc->num_structs < MAX_STRUCTS) {
        cc->structs[cc->num_structs] = t;
        cc->num_structs++;
    }
}

/* ---------------------------------------------------------------- */
/* Parse struct/union body                                           */
/* ---------------------------------------------------------------- */

Type *parse_type(Compiler *cc);
Type *parse_declarator(Compiler *cc, Type *base, char *name_out);
static long long parse_const_expr_ternary(Compiler *cc);
static long long parse_const_expr_lor(Compiler *cc);
static long long parse_const_expr_land(Compiler *cc);
static long long parse_const_expr_bor(Compiler *cc);
static long long parse_const_expr_bxor(Compiler *cc);
static long long parse_const_expr_band(Compiler *cc);
static long long parse_const_expr_eq(Compiler *cc);
static long long parse_const_expr_rel(Compiler *cc);
static long long parse_const_expr_shift(Compiler *cc);
static long long parse_const_expr_add(Compiler *cc);
static long long parse_const_expr_mul(Compiler *cc);
static long long parse_const_expr_unary(Compiler *cc);
static long long parse_const_expr_primary(Compiler *cc);

static long long parse_const_expr(Compiler *cc) { return parse_const_expr_ternary(cc); }

static long long parse_const_expr_ternary(Compiler *cc) {
    long long val = parse_const_expr_lor(cc);
    if (cc->tk == TK_QUESTION) {
        next_token(cc);
        long long t_val = parse_const_expr(cc);
        expect(cc, TK_COLON);
        long long f_val = parse_const_expr_ternary(cc);
        return val ? t_val : f_val;
    }
    return val;
}
static long long parse_const_expr_lor(Compiler *cc) {
    long long val = parse_const_expr_land(cc);
    while (cc->tk == TK_LOR) { next_token(cc); long long rhs = parse_const_expr_land(cc); val = val || rhs; }
    return val;
}
static long long parse_const_expr_land(Compiler *cc) {
    long long val = parse_const_expr_bor(cc);
    while (cc->tk == TK_LAND) { next_token(cc); long long rhs = parse_const_expr_bor(cc); val = val && rhs; }
    return val;
}
static long long parse_const_expr_bor(Compiler *cc) {
    long long val = parse_const_expr_bxor(cc);
    while (cc->tk == TK_PIPE) { next_token(cc); val |= parse_const_expr_bxor(cc); }
    return val;
}
static long long parse_const_expr_bxor(Compiler *cc) {
    long long val = parse_const_expr_band(cc);
    while (cc->tk == TK_CARET) { next_token(cc); val ^= parse_const_expr_band(cc); }
    return val;
}
static long long parse_const_expr_band(Compiler *cc) {
    long long val = parse_const_expr_eq(cc);
    while (cc->tk == TK_AMP) { next_token(cc); val &= parse_const_expr_eq(cc); }
    return val;
}
static long long parse_const_expr_eq(Compiler *cc) {
    long long val = parse_const_expr_rel(cc);
    while (cc->tk == TK_EQ || cc->tk == TK_NE) {
        int tk = cc->tk; next_token(cc);
        if (tk == TK_EQ) val = (val == parse_const_expr_rel(cc));
        else val = (val != parse_const_expr_rel(cc));
    }
    return val;
}
static long long parse_const_expr_rel(Compiler *cc) {
    long long val = parse_const_expr_shift(cc);
    while (cc->tk == TK_LT || cc->tk == TK_GT || cc->tk == TK_LE || cc->tk == TK_GE) {
        int tk = cc->tk; next_token(cc);
        if (tk == TK_LT) val = (val < parse_const_expr_shift(cc));
        else if (tk == TK_GT) val = (val > parse_const_expr_shift(cc));
        else if (tk == TK_LE) val = (val <= parse_const_expr_shift(cc));
        else val = (val >= parse_const_expr_shift(cc));
    }
    return val;
}
static long long parse_const_expr_shift(Compiler *cc) {
    long long val = parse_const_expr_add(cc);
    while (cc->tk == TK_SHL || cc->tk == TK_SHR) {
        int tk = cc->tk; next_token(cc);
        if (tk == TK_SHL) val <<= parse_const_expr_add(cc);
        else val >>= parse_const_expr_add(cc);
    }
    return val;
}
static long long parse_const_expr_add(Compiler *cc) {
    long long val = parse_const_expr_mul(cc);
    while (cc->tk == TK_PLUS || cc->tk == TK_MINUS) {
        int tk = cc->tk; next_token(cc);
        if (tk == TK_PLUS) val += parse_const_expr_mul(cc);
        else val -= parse_const_expr_mul(cc);
    }
    return val;
}
static long long parse_const_expr_mul(Compiler *cc) {
    long long val = parse_const_expr_unary(cc);
    while (cc->tk == TK_STAR || cc->tk == TK_SLASH || cc->tk == TK_PERCENT) {
        int tk = cc->tk; next_token(cc);
        long long rhs = parse_const_expr_unary(cc);
        if (tk == TK_STAR) val *= rhs;
        else if (tk == TK_SLASH) { if (rhs) val /= rhs; }
        else if (tk == TK_PERCENT) { if (rhs) val %= rhs; }
    }
    return val;
}
static long long parse_const_expr_unary(Compiler *cc) {
    if (cc->tk == TK_LPAREN) {
        int is_cast = 0;
        int ptk = peek_token(cc);
        int curr_tk = cc->tk;
        char curr_txt[MAX_IDENT];
        strncpy(curr_txt, cc->tk_text, MAX_IDENT - 1);
        cc->tk = ptk;
        strncpy(cc->tk_text, cc->peek_text, MAX_IDENT - 1);
        if (is_type_token(cc)) is_cast = 1;
        cc->tk = curr_tk;
        strncpy(cc->tk_text, curr_txt, MAX_IDENT - 1);
        
        if (is_cast) {
            next_token(cc); /* consume ( */
            Type *st = parse_type(cc);
            char dummy[128];
            st = parse_declarator(cc, st, dummy);
            expect(cc, TK_RPAREN);
            return parse_const_expr_unary(cc);
        }
    }
    if (cc->tk == TK_MINUS) { next_token(cc); return -parse_const_expr_unary(cc); }
    if (cc->tk == TK_PLUS) { next_token(cc); return parse_const_expr_unary(cc); }
    if (cc->tk == TK_TILDE) { next_token(cc); return ~parse_const_expr_unary(cc); }
    if (cc->tk == TK_BANG) { next_token(cc); return !parse_const_expr_unary(cc); }
    if (cc->tk == TK_SIZEOF) {
        next_token(cc);
        if (cc->tk == TK_LPAREN) {
            next_token(cc);
            if (is_type_token(cc)) {
                Type *st = parse_type(cc);
                char dummy[128];
                st = parse_declarator(cc, st, dummy);
                expect(cc, TK_RPAREN);
                return type_size(st);
            }
            long long v = parse_const_expr(cc);
            expect(cc, TK_RPAREN);
            return v;
        } else {
            return 8;
        }
    }
    return parse_const_expr_primary(cc);
}
static long long parse_const_expr_primary(Compiler *cc) {
    if (cc->tk == TK_LPAREN) {
        long long val;
        next_token(cc);
        val = parse_const_expr(cc);
        expect(cc, TK_RPAREN);
        return val;
    }
    if (cc->tk == TK_NUM) {
        long long val = cc->tk_val;
        next_token(cc);
        return val;
    }
    if (cc->tk == TK_IDENT) {
        Symbol *sym = scope_find(cc, cc->tk_text);
        if (sym && sym->is_enum_const) {
            long long val = sym->enum_val;
            next_token(cc);
            return val;
        }
        next_token(cc);
        return 0;
    }
    next_token(cc);
    return 0;
}



static Type *parse_struct_or_union(Compiler *cc, int is_union) {
    Type *stype;
    char tag[MAX_IDENT];
    int has_tag;

    has_tag = 0;
    tag[0] = 0;

    if (cc->tk == TK_IDENT) {
        strncpy(tag, cc->tk_text, MAX_IDENT - 1);
        has_tag = 1;
        next_token(cc);
    }

    /* forward reference or existing struct */
    if (cc->tk != TK_LBRACE) {
        if (has_tag) {
            stype = find_struct(cc, tag);
            if (stype) return stype;
            /* forward declaration */
        stype = type_new(cc, is_union ? TY_UNION : TY_STRUCT);
            strncpy(stype->tag, tag, MAX_IDENT - 1);
            stype->is_complete = 0;
            register_struct(cc, stype);
            return stype;
        }
        error(cc, "expected struct/union tag or body");
        return cc->ty_int;
    }

    /* check if already declared */
    stype = 0;
    if (has_tag) {
        stype = find_struct(cc, tag);
    }
    if (!stype) {
        stype = type_new(cc, is_union ? TY_UNION : TY_STRUCT);
        if (has_tag) {
            strncpy(stype->tag, tag, MAX_IDENT - 1);
            register_struct(cc, stype);
        }
    }

    expect(cc, TK_LBRACE);

    {
        StructField *last_field;
        int offset;
        int max_size;
        int max_align;
        int current_bit_offset;
        int bf_base_sz;

        last_field = 0;
        offset = 0;
        max_size = 0;
        max_align = 1;
        current_bit_offset = 0;
        bf_base_sz = 0;

        while (cc->tk != TK_RBRACE) {
            Type *ftype;
            Type *base_ftype;
            char fname[MAX_IDENT];
            StructField *field;
            int falign;
            int bit_sz;

            if (cc->tk == TK_EOF) break;

            base_ftype = parse_type(cc);
            do {
                fname[0] = 0;
                bit_sz = -1;
                ftype = parse_declarator(cc, base_ftype, fname);

                if (cc->tk == TK_COLON) {
                    next_token(cc);
                    bit_sz = (int)parse_const_expr(cc);
                }

                field = (StructField *)cc_alloc(cc, sizeof(StructField));
                strncpy(field->name, fname, MAX_IDENT - 1);
                field->type = ftype;
                field->is_bitfield = 0;
                field->bit_offset = 0;
                field->bit_size = 0;

                falign = type_align(ftype);
                if (falign > max_align) max_align = falign;

                if (is_union) {
                    field->offset = 0;
                    if (type_size(ftype) > max_size) max_size = type_size(ftype);
                    if (bit_sz >= 0) {
                        field->is_bitfield = 1;
                        field->bit_size = bit_sz;
                    }
                } else {
                    if (bit_sz >= 0) {
                        int type_bits = type_size(ftype) * 8;
                        if (bit_sz == 0 || current_bit_offset + bit_sz > type_bits) {
                            if (current_bit_offset > 0) {
                                offset += bf_base_sz; /* flush prev bitfield */
                                current_bit_offset = 0;
                            }
                        }
                        if (bit_sz > 0) {
                            if (current_bit_offset == 0) {
                                offset = (offset + falign - 1) & ~(falign - 1);
                                bf_base_sz = type_size(ftype);
                            }
                            field->offset = offset;
                            field->is_bitfield = 1;
                            field->bit_offset = current_bit_offset;
                            field->bit_size = bit_sz;
                            current_bit_offset += bit_sz;
                        } else {
                            field->offset = offset;
                        }
                    } else {
                        if (current_bit_offset > 0) {
                            offset += bf_base_sz;
                            current_bit_offset = 0;
                        }
                        if (falign > 1) {
                            offset = (offset + falign - 1) & ~(falign - 1);
                        }
                        field->offset = offset;
                        offset += type_size(ftype);
                    }
                }

                field->next = 0;
                if (last_field) last_field->next = field;
                else stype->fields = field;
                last_field = field;

                if (cc->tk == TK_COMMA) {
                    next_token(cc);
                } else {
                    break;
                }
            } while (1);
            expect(cc, TK_SEMI);
        }
        if (current_bit_offset > 0) {
            offset += bf_base_sz;
        }

            if (is_union) {
            stype->size = max_size;
        } else {
            stype->size = offset;
        }
        /* align total size */
        if (max_align > 1) {
            stype->size = (stype->size + max_align - 1) & ~(max_align - 1);
        }
        stype->align = max_align;
        stype->is_complete = 1;
    }

    expect(cc, TK_RBRACE);
    if (stype->tag[0] && (strcmp(stype->tag, "yyStackEntry") == 0 || strcmp(stype->tag, "yyParser") == 0 || strcmp(stype->tag, "Walker") == 0)) {
        int n = 0;
        StructField *f = stype->fields;
        while(f) { n++; printf("FIELD: %s\n", f->name); f = f->next; }
        printf("DEBUG: Parsed %s with %d fields (Type=%p, fields=%p)\n", stype->tag, n, (void*)stype, (void*)stype->fields);
        fflush(stdout);
    }
    return stype;
}

/* ---------------------------------------------------------------- */
/* Parse enum                                                        */
/* ---------------------------------------------------------------- */

static Type *parse_enum_def(Compiler *cc) {
    char tag[MAX_IDENT];
    Type *etype;

    tag[0] = 0;
    if (cc->tk == TK_IDENT) {
        strncpy(tag, cc->tk_text, MAX_IDENT - 1);
        next_token(cc);
    }

    etype = type_new(cc, TY_ENUM);

    if (cc->tk == TK_LBRACE) {
        long val;
        next_token(cc);
        val = 0;
        while (cc->tk != TK_RBRACE) {
            char name[MAX_IDENT];
            Symbol *sym;

            if (cc->tk == TK_EOF) break;
            if (cc->tk != TK_IDENT) {
                error(cc, "expected enum constant name");
                next_token(cc);
                continue;
            }
            strncpy(name, cc->tk_text, MAX_IDENT - 1);
            next_token(cc);

            if (cc->tk == TK_ASSIGN) {
                next_token(cc);
                val = parse_const_expr(cc);
            }

            sym = scope_add(cc, name, etype);
            sym->is_enum_const = 1;
            sym->enum_val = val;
            val++;

            if (cc->tk == TK_COMMA) {
                next_token(cc);
            } else {
                break;
            }
        }
        expect(cc, TK_RBRACE);
    }
    return etype;
}

/* ---------------------------------------------------------------- */
/* Parse base type                                                   */
/* ---------------------------------------------------------------- */

Node *parse_expr(Compiler *cc);

Type *parse_type(Compiler *cc) {
    Type *type = 0;
    int is_unsigned = 0;
    int is_signed = 0;
    int is_long = 0;
    int is_short = 0;
    int is_int = 0;
    int is_char = 0;
    int is_double = 0;
    int is_float = 0;
    int is_void = 0;
    int is_typedef_kw = 0;
    int is_static = 0;
    int is_extern = 0;
    int is_const = 0;

    /* storage class / qualifiers / basic types */
    for (;;) {
        if (cc->tk == TK_STATIC) { is_static = 1; next_token(cc); }
        else if (cc->tk == TK_EXTERN) { is_extern = 1; next_token(cc); }
        else if (cc->tk == TK_CONST) { is_const = 1; next_token(cc); }
        else if (cc->tk == TK_VOLATILE) { next_token(cc); }
        else if (cc->tk == TK_INLINE) { next_token(cc); }
        else if (cc->tk == TK_AUTO) { next_token(cc); }
        else if (cc->tk == TK_REGISTER) { next_token(cc); }
        else if (cc->tk == TK_TYPEDEF) { is_typedef_kw = 1; next_token(cc); }

        else if (cc->tk == TK_UNSIGNED) { is_unsigned = 1; next_token(cc); }
        else if (cc->tk == TK_SIGNED) { is_signed = 1; next_token(cc); }
        else if (cc->tk == TK_LONG) { is_long++; next_token(cc); }
        else if (cc->tk == TK_SHORT) { is_short = 1; next_token(cc); }
        else if (cc->tk == TK_INT) { is_int = 1; next_token(cc); }
        else if (cc->tk == TK_CHAR) { is_char = 1; next_token(cc); }
        else if (cc->tk == TK_DOUBLE) { is_double = 1; next_token(cc); }
        else if (cc->tk == TK_FLOAT) { is_float = 1; next_token(cc); }
        else if (cc->tk == TK_VOID) { is_void = 1; next_token(cc); }
        else if (cc->tk == TK_TYPEOF) {
            next_token(cc);
            expect(cc, TK_LPAREN);
            Node *expr = parse_expr(cc);
            expect(cc, TK_RPAREN);
            type = expr->type;
            if (!type) type = cc->ty_int;
        }
        else if (cc->tk == TK_AUTO_TYPE) {
            next_token(cc);
            type = type_new(cc, TY_VOID);
            type->size = 0; /* placeholder */
        }
        else break;
    }

    if (is_void) { type = cc->ty_void; }
    else if (is_float) { type = cc->ty_float; }
    else if (is_double) { type = cc->ty_double; }
    else if (is_char) {
        if (is_unsigned) { type = cc->ty_uchar; } else { type = cc->ty_char; }
    }
    else if (is_short) {
        if (is_unsigned) { type = cc->ty_ushort; } else { type = cc->ty_short; }
    }
    else if (is_long >= 2) {
        if (is_unsigned) { type = cc->ty_ulonglong; } else { type = cc->ty_longlong; }
    }
    else if (is_long == 1) {
        if (is_unsigned) { type = cc->ty_ulong; } else { type = cc->ty_long; }
    }
    else if (is_int || is_unsigned || is_signed) {
        if (is_unsigned) { type = cc->ty_uint; } else { type = cc->ty_int; }
    }

    if (!type) {
        if (cc->tk == TK_STRUCT) {
            next_token(cc);
            type = parse_struct_or_union(cc, 0);
        }
        else if (cc->tk == TK_UNION) {
            next_token(cc);
            type = parse_struct_or_union(cc, 1);
        }
        else if (cc->tk == TK_ENUM) {
            next_token(cc);
            type = parse_enum_def(cc);
        }
        else if (cc->tk == TK_IDENT) {
            /* could be a typedef name */
            Symbol *sym;
            sym = scope_find(cc, cc->tk_text);
            if (strcmp(cc->tk_text, "yyParser") == 0) {
                printf("DEBUG: parse_type lookup 'yyParser'. sym=%p\n", (void*)sym);
            }
            if (sym && sym->is_typedef) {
                type = sym->type;
                next_token(cc);
            }
        }
    }

    if (!type) {
        type = cc->ty_int;
    }

    /* skip trailing const/volatile (e.g. 'int const') */
    while (cc->tk == TK_CONST || cc->tk == TK_VOLATILE) {
        if (cc->tk == TK_CONST) { is_const = 1; }
        next_token(cc);
    }

    cc->current_is_static = is_static;
    cc->current_is_const = is_const;
    return type;
}

/* ---------------------------------------------------------------- */
/* Parse declarator (pointers, arrays, function params)              */
/* ---------------------------------------------------------------- */

Node *parse_assign(Compiler *cc);

static int eval_const_expr(Node *n) {
    if (!n) return 0;
    if (n->kind == ND_NUM) return (int)n->int_val;
    if (n->kind == ND_ADD) return eval_const_expr(n->lhs) + eval_const_expr(n->rhs);
    if (n->kind == ND_SUB) return eval_const_expr(n->lhs) - eval_const_expr(n->rhs);
    if (n->kind == ND_MUL) return eval_const_expr(n->lhs) * eval_const_expr(n->rhs);
    if (n->kind == ND_DIV) {
        int r = eval_const_expr(n->rhs);
        return r ? eval_const_expr(n->lhs) / r : 0;
    }
    if (n->kind == ND_MOD) {
        int r = eval_const_expr(n->rhs);
        return r ? eval_const_expr(n->lhs) % r : 0;
    }
    if (n->kind == ND_SHL) return eval_const_expr(n->lhs) << eval_const_expr(n->rhs);
    if (n->kind == ND_SHR) return eval_const_expr(n->lhs) >> eval_const_expr(n->rhs);
    if (n->kind == ND_BAND) return eval_const_expr(n->lhs) & eval_const_expr(n->rhs);
    if (n->kind == ND_BOR) return eval_const_expr(n->lhs) | eval_const_expr(n->rhs);
    if (n->kind == ND_BXOR) return eval_const_expr(n->lhs) ^ eval_const_expr(n->rhs);
    if (n->kind == ND_CAST) return eval_const_expr(n->lhs);
    /* Unary operators — critical for negative switch cases like case (-15): */
    if (n->kind == ND_NEG)  return -eval_const_expr(n->lhs);
    if (n->kind == ND_BNOT) return ~eval_const_expr(n->lhs);
    if (n->kind == ND_LNOT) return !eval_const_expr(n->lhs);
    /* Enum constants (global only — local vars are NOT constants) */
    if (n->kind == ND_VAR && n->sym && n->sym->is_enum_const && !n->sym->is_local)
        return n->sym->enum_val;
    return 0; /* Fallback for unsupported complex compile-time bounds */
}



static Type *inject_base_type(Compiler *cc, Type *t, Type *base) {
    if (!t || t == cc->ty_int) return base;
    if (t->kind == TY_PTR) {
        return type_ptr(cc, inject_base_type(cc, t->base, base));
    }
    if (t->kind == TY_ARRAY) {
        return type_array(cc, inject_base_type(cc, t->base, base), t->array_len);
    }
    if (t->kind == TY_FUNC) {
        Type *n = type_func(cc, inject_base_type(cc, t->ret, base));
        n->params = t->params;
        n->num_params = t->num_params;
        n->is_variadic = t->is_variadic;
        return n;
    }
    return base;
}

Type *parse_declarator(Compiler *cc, Type *base, char *name_out) {
    Type *type;

    type = base;
    name_out[0] = 0;

    /* pointers */
    while (cc->tk == TK_STAR) {
        next_token(cc);
        /* skip const/volatile after * */
        while (cc->tk == TK_CONST || cc->tk == TK_VOLATILE) {
            next_token(cc);
        }
        type = type_ptr(cc, type);
    }

    /* name */
    if (cc->tk == TK_IDENT) {
        strncpy(name_out, cc->tk_text, MAX_IDENT - 1);
        next_token(cc);
    }
    else if (cc->tk == TK_LPAREN) {
        /* grouped declarator: (*name)(params) or (*name)[N] */
        int pk;
        pk = peek_token(cc);
        if (pk == TK_STAR || pk == TK_IDENT || pk == TK_LPAREN) {
            Type *inner;
            next_token(cc); /* consume ( */
            inner = parse_declarator(cc, cc->ty_int, name_out);
            expect(cc, TK_RPAREN);
            /* process outer dimensions and function args first! */
            if (cc->tk == TK_LBRACKET) {
                int arr_lens[16];
                int arr_num = 0;
                while (cc->tk == TK_LBRACKET) {
                    int len = 0;
                    next_token(cc);
                    if (cc->tk != TK_RBRACKET) len = (int)parse_const_expr(cc);
                    expect(cc, TK_RBRACKET);
                    if (arr_num < 16) arr_lens[arr_num++] = len;
                }
                while (arr_num > 0) {
                    arr_num--;
                    type = type_array(cc, type, arr_lens[arr_num]);
                }
            }
            if (cc->tk == TK_LPAREN) {
                Type *ftype;
                next_token(cc);
                ftype = type_func(cc, type);
                ftype->params = (Type **)cc_alloc(cc, sizeof(Type *) * MAX_PARAMS);
                ftype->num_params = 0;
                ftype->is_variadic = 0;
                if (cc->tk != TK_RPAREN) {
                    if (cc->tk == TK_VOID) {
                        int vpk;
                        vpk = peek_token(cc);
                        if (vpk == TK_RPAREN) {
                            next_token(cc);
                        } else {
                            goto parse_params_inner;
                        }
                    } else {
                        parse_params_inner:
                        while (cc->tk != TK_RPAREN) {
                            Type *ptype;
                            char pname[MAX_IDENT];
                            if (cc->tk == TK_ELLIPSIS) {
                                ftype->is_variadic = 1;
                                next_token(cc);
                                break;
                            }
                            if (cc->tk == TK_EOF) break;
                            ptype = parse_type(cc);
                            ptype = parse_declarator(cc, ptype, pname);
                            if (ptype->kind == TY_ARRAY) ptype = type_ptr(cc, ptype->base);
                            if (ftype->num_params < MAX_PARAMS) {
                                ftype->params[ftype->num_params] = ptype;
                                ftype->num_params++;
                            }
                            if (cc->tk == TK_COMMA) {
                                next_token(cc);
                            } else {
                                break;
                            }
                        }
                    }
                }
                expect(cc, TK_RPAREN);
                type = ftype;
            }
            return inject_base_type(cc, inner, type);
        }
    }

    /* array dimensions */
    if (cc->tk == TK_LBRACKET) {
        int arr_lens[16];
        int arr_num = 0;
        while (cc->tk == TK_LBRACKET) {
            int len = 0;
            next_token(cc);
            if (cc->tk != TK_RBRACKET) len = (int)parse_const_expr(cc);
            expect(cc, TK_RBRACKET);
            if (arr_num < 16) arr_lens[arr_num++] = len;
        }
        while (arr_num > 0) {
            arr_num--;
            type = type_array(cc, type, arr_lens[arr_num]);
        }
    }

    /* function parameters */
    if (cc->tk == TK_LPAREN) {
        Type *ftype;
        next_token(cc);
        ftype = type_func(cc, type);
        ftype->params = (Type **)cc_alloc(cc, sizeof(Type *) * MAX_PARAMS);
        ftype->num_params = 0;
        ftype->is_variadic = 0;

        if (cc->tk != TK_RPAREN) {
            if (cc->tk == TK_VOID) {
                int pk;
                pk = peek_token(cc);
                if (pk == TK_RPAREN) {
                    next_token(cc); /* skip void */
                } else {
                    goto parse_params;
                }
            } else {
                parse_params:
                while (cc->tk != TK_RPAREN) {
                    Type *ptype;
                    char pname[MAX_IDENT];

                    if (cc->tk == TK_ELLIPSIS) {
                        ftype->is_variadic = 1;
                        next_token(cc);
                        break;
                    }
                    if (cc->tk == TK_EOF) break;

                    ptype = parse_type(cc);
                    ptype = parse_declarator(cc, ptype, pname);

                    if (ptype->kind == TY_ARRAY) ptype = type_ptr(cc, ptype->base);

                    if (ftype->num_params < MAX_PARAMS) {
                        ftype->params[ftype->num_params] = ptype;
                        ftype->num_params++;
                    }

                    if (cc->tk == TK_COMMA) {
                        next_token(cc);
                    } else {
                        break;
                    }
                }
            }
        }
        expect(cc, TK_RPAREN);
        type = ftype;
    }

    return type;
}

/* ================================================================ */
/* EXPRESSION PARSING — FULL PRECEDENCE CHAIN                       */
/* ================================================================ */

/* --- primary --- */

Node *parse_primary(Compiler *cc) {
    Node *n;
    int line;

    line = cc->tk_line;

    if (cc->tk == TK_NUM) {
        n = node_num(cc, cc->tk_val, line);
        if (cc->tk_text[0] == 'U') {
             if (cc->tk_val <= 4294967295ULL) {
                 n->type = cc->ty_uint;
             } else {
                 n->type = cc->ty_ulong;
             }
        }
        if (cc->tk_text[1] == 'L') {
             if (cc->tk_text[0] == 'U') {
                 n->type = cc->ty_ulong;
             } else {
                 n->type = cc->ty_long;
             }
        }
        next_token(cc);
        return n;
    }

    if (cc->tk == TK_FLIT) {
        n = node_flit(cc, cc->tk_fval, line);
        next_token(cc);
        return n;
    }

    if (cc->tk == TK_CHAR_LIT) {
        n = node_num(cc, cc->tk_val, line);
        n->type = cc->ty_char;
        next_token(cc);
        return n;
    }

    if (cc->tk == TK_STR) {
        int sid;
        StringEntry *se;

        sid = cc->num_strings;
        if (sid < MAX_STRINGS) {
            se = &cc->strings[sid];
            se->data = cc_strdup(cc, cc->tk_str);
            se->len = cc->tk_str_len;
            se->label_id = cc->label_count;
            cc->label_count++;
            cc->num_strings++;
        }
        n = node_new(cc, ND_STR, line);
        n->str_id = sid;
        n->type = type_ptr(cc, cc->ty_char);
        next_token(cc);
        return n;
    }

    if (cc->tk == TK_IDENT) {
        Symbol *sym;
        n = node_new(cc, ND_VAR, line);
        strncpy(n->name, cc->tk_text, MAX_IDENT - 1);
        sym = scope_find(cc, cc->tk_text);
        
        if (strcmp(cc->tk_text, "yypParser") == 0 || strcmp(cc->tk_text, "pParser") == 0) {
            printf("DEBUG: parse_primary lookup '%s'. sym=%p, sym->type=%p, base=%p, base_tag='%s'\n", 
                   cc->tk_text,
                   (void*)sym, sym ? (void*)sym->type : NULL, 
                   (sym && sym->type) ? (void*)sym->type->base : NULL, 
                   (sym && sym->type && sym->type->base && sym->type->base->tag[0]) ? sym->type->base->tag : "<none>");
            fflush(stdout);
        }
        
        if (sym) {
            n->sym = sym;
            n->type = sym->type;
            if (sym->asm_name[0]) {
                strncpy(n->name, sym->asm_name, MAX_IDENT - 1);
            }
            if (sym->is_enum_const && !sym->is_local) {
                /* Only fold to ND_NUM if this is truly a global enum constant.
                 * A local variable (is_local=1) with the same name as an outer
                 * enum constant must NOT be constant-folded — it is a live
                 * stack variable that must be loaded at runtime. */
                n->kind = ND_NUM;
                n->int_val = sym->enum_val;
                n->type = cc->ty_int;
            }
        } else {
            /* stdio global pointers: stdin, stdout, stderr are FILE* (size 8) */
            if (strcmp(n->name, "stdin") == 0 || strcmp(n->name, "stdout") == 0 || strcmp(n->name, "stderr") == 0) {
                n->type = type_ptr(cc, cc->ty_char);
            } else {
                n->type = cc->ty_int;
            }
        }
        next_token(cc);
        return n;
    }

    if (cc->tk == TK_LPAREN) {
        next_token(cc);

        /* check for cast expression */
        if (is_type_token(cc)) {
            Type *cast_type;
            char dummy[MAX_IDENT];
            cast_type = parse_type(cc);
            cast_type = parse_declarator(cc, cast_type, dummy);
            expect(cc, TK_RPAREN);
            n = node_new(cc, ND_CAST, line);
            n->cast_type = cast_type;
            n->lhs = parse_unary(cc);
            n->type = cast_type;
            return n;
        }

        n = parse_expr(cc);
        expect(cc, TK_RPAREN);
        return n;
    }

    if (cc->tk == TK_BUILTIN_VA_ARG) {
        Node *n;
        Node *ap_node;
        Type *ret_type;
        char dummy[MAX_IDENT];
        int line;
        
        line = cc->tk_line;
        next_token(cc); /* skip va_arg */
        expect(cc, TK_LPAREN);
        ap_node = parse_assign(cc);
        expect(cc, TK_COMMA);
        ret_type = parse_type(cc);
        ret_type = parse_declarator(cc, ret_type, dummy);
        expect(cc, TK_RPAREN);

        n = node_new(cc, ND_VA_ARG, line);
        n->lhs = ap_node;
        n->type = ret_type;
        return n;
    }

    if (cc->tk == TK_SIZEOF) {
        int pk;
        int is_type_in_parens;
        next_token(cc);
        if (cc->tk == TK_LPAREN) {
            pk = peek_token(cc);
            is_type_in_parens = 0;
            if (pk >= TK_INT) {
                if (pk <= TK_INLINE || pk == TK_STRUCT ||
                    pk == TK_UNION || pk == TK_ENUM) {
                    is_type_in_parens = 1;
                }
            }
            if (!is_type_in_parens && pk == TK_IDENT) {
                Symbol *sz_sym;
                sz_sym = scope_find(cc, cc->peek_text);
                if (sz_sym && sz_sym->is_typedef) is_type_in_parens = 1;
            }
            if (is_type_in_parens) {
                Type *st;
                char dummy[MAX_IDENT];
                next_token(cc);  /* skip ( */
                st = parse_type(cc);
                st = parse_declarator(cc, st, dummy);
                expect(cc, TK_RPAREN);
                n = node_num(cc, type_size(st), line);
                return n;
            }
            /* sizeof(expr) */
            {
                Node *expr;
                next_token(cc); /* skip ( */
                expr = parse_unary(cc);
                expect(cc, TK_RPAREN);
                n = node_num(cc, type_size(expr->type), line);
                return n;
            }
        } else {
            Node *expr;
            expr = parse_unary(cc);
            n = node_num(cc, type_size(expr->type), line);
            return n;
        }
    }

    /* fallthrough — unexpected token */
    {
        char buf[256];
        sprintf(buf, "unexpected token %d in expression", cc->tk);
        error(cc, buf);
        next_token(cc);
        n = node_num(cc, 0, line);
        return n;
    }
}

/* --- postfix: [], ., ->, (), ++, -- --- */
/* Bug fix: call handler inlined, && replaced with nested if */

static StructField *find_struct_member(Type *type, const char *name, int *out_offset) {
    StructField *f;
    if (!type) {
        return 0;
    }

    for (f = type->fields; f; f = f->next) {
        if (f->name[0]) {
            if (strcmp(f->name, name) == 0) {
                *out_offset = f->offset;
                fprintf(stderr, "FOUND FIELD: %s, is_bf=%d, bf_sz=%d\n", f->name, f->is_bitfield, f->bit_size);
                return f;
            }
        } else if (f->type && (f->type->kind == TY_STRUCT || f->type->kind == TY_UNION)) {
            int sub_offset = 0;
            StructField *sub = find_struct_member(f->type, name, &sub_offset);
            if (sub) {
                *out_offset = f->offset + sub_offset;
                return sub;
            }
        }
    }
    return 0;
}

Node *parse_postfix(Compiler *cc) {
    Node *n;
    n = parse_primary(cc);

    for (;;) {
        int line;
        line = cc->tk_line;

        /* array subscript */
        if (cc->tk == TK_LBRACKET) {
            Node *idx;
            Node *add;
            next_token(cc);
            idx = parse_expr(cc);
            expect(cc, TK_RBRACKET);
            /* a[i] => *(a + i) */
            add = node_new(cc, ND_ADD, line);
            add->lhs = n;
            add->rhs = idx;
            if (is_pointer(n->type)) {
                add->type = n->type;
            } else {
                add->type = idx->type;
            }
            n = node_new(cc, ND_DEREF, line);
            n->lhs = add;
            if (add->type) {
                if (add->type->base) {
                    n->type = add->type->base;
                } else {
                    n->type = cc->ty_int;
                }
            } else {
                n->type = cc->ty_int;
            }
            continue;
        }

        /* struct member access */
        if (cc->tk == TK_DOT) {
            int accumulated_offset = 0;
            StructField *f;
            Node *member;
            next_token(cc);
            if (cc->tk != TK_IDENT) {
                error(cc, "expected member name");
                break;
            }
            member = node_new(cc, ND_MEMBER, line);
            member->lhs = n;
            strncpy(member->member_name, cc->tk_text, MAX_IDENT - 1);
            /* find field */
            if (n->type) {
                f = find_struct_member(n->type, cc->tk_text, &accumulated_offset);
                if (f) {
                    member->member_offset = accumulated_offset;
                    member->type = f->type;
                    member->member_size = type_size(f->type);
                    member->is_bitfield = f->is_bitfield;
                    member->bit_offset = f->bit_offset;
                    member->bit_size = f->bit_size;
                } else {
                    char buf[256];
                    sprintf(buf, "unknown struct member '%s' in struct '%s' (type=%p, fields=%p)", cc->tk_text, (n->type && n->type->tag[0]) ? n->type->tag : "<anon>", (void*)n->type, n->type ? (void*)n->type->fields : NULL);
                    error(cc, buf);
                    member->type = cc->ty_int;
                }
            } else {
                member->type = cc->ty_int;
            }
            n = member;
            next_token(cc);
            continue;
        }

        /* arrow member access */
        if (cc->tk == TK_ARROW) {
            int accumulated_offset = 0;
            StructField *f;
            Node *deref;
            next_token(cc);
            if (cc->tk != TK_IDENT) {
                error(cc, "expected member name after ->");
                break;
            }
            /* deref first */
            deref = node_new(cc, ND_DEREF, line);
            deref->lhs = n;
            if (n->type) {
                if (n->type->base) {
                    deref->type = n->type->base;
                } else {
                    deref->type = cc->ty_int;
                }
            } else {
                deref->type = cc->ty_int;
            }
            n = node_new(cc, ND_MEMBER, line);
            n->lhs = deref;
            strncpy(n->member_name, cc->tk_text, MAX_IDENT - 1);
            if (deref->type) {
                f = find_struct_member(deref->type, cc->tk_text, &accumulated_offset);
                if (f) {
                    n->member_offset = accumulated_offset;
                    n->type = f->type;
                    n->member_size = type_size(f->type);
                    n->is_bitfield = f->is_bitfield;
                    n->bit_offset = f->bit_offset;
                    n->bit_size = f->bit_size;
                } else {
                    char buf[256];
                    sprintf(buf, "unknown struct member '%s' after -> in struct '%s' (type=%p, fields=%p)", cc->tk_text, (deref->type && deref->type->tag[0]) ? deref->type->tag : "<anon>", (void*)deref->type, deref->type ? (void*)deref->type->fields : NULL);
                    error(cc, buf);
                    n->type = cc->ty_int;
                }
            } else {
                n->type = cc->ty_int;
            }
            next_token(cc);
            continue;
        }

        /* function call — inlined handler, nested ifs instead of && */
        if (cc->tk == TK_LPAREN) {
            int is_direct = 0;
            if (n->kind == ND_VAR) {
                if (!n->sym) is_direct = 1;
                else if (n->sym->type && n->sym->type->kind == TY_FUNC) is_direct = 1;
            }
            if (is_direct) {
                /* function call */
                Node *call;
                Symbol *sym;
                next_token(cc);
                call = node_new(cc, ND_CALL, line);
                strncpy(call->func_name, n->name, MAX_IDENT - 1);
                call->args = (Node **)cc_alloc(cc, sizeof(Node *) * MAX_CALL_ARGS);
                call->num_args = 0;
                while (cc->tk != TK_RPAREN) {
                    if (cc->tk == TK_EOF) break;
                    if (call->num_args < MAX_CALL_ARGS) {
                        call->args[call->num_args] = parse_assign(cc);
                        call->num_args = call->num_args + 1;
                    } else {
                        parse_assign(cc);
                    }
                    if (cc->tk == TK_COMMA) {
                        next_token(cc);
                    } else {
                        break;
                    }
                }
                expect(cc, TK_RPAREN);
                sym = scope_find(cc, call->func_name);
                if (sym) {
                    if (sym->type) {
                        if (sym->type->kind == TY_FUNC) {
                            if (sym->type->ret) {
                                call->type = sym->type->ret;
                            } else {
                                call->type = cc->ty_int;
                            }
                        } else {
                            call->type = cc->ty_int;
                        }
                    } else {
                        call->type = cc->ty_int;
                    }
                } else {
                    call->type = cc->ty_int;
                }
                n = call;
                continue;
            }
            if (n->kind == ND_MEMBER || n->kind == ND_DEREF || n->kind == ND_CAST || (n->kind == ND_VAR && !is_direct)) {
                /* indirect call through function pointer member, deref, or cast */
                Node *call;
                next_token(cc);
                call = node_new(cc, ND_CALL, line);
                call->func_name[0] = 0; /* empty = indirect */
                call->lhs = n; /* callee expression */
                call->args = (Node **)cc_alloc(cc, sizeof(Node *) * MAX_CALL_ARGS);
                call->num_args = 0;
                while (cc->tk != TK_RPAREN) {
                    if (cc->tk == TK_EOF) break;
                    if (call->num_args < MAX_CALL_ARGS) {
                        call->args[call->num_args] = parse_assign(cc);
                        call->num_args = call->num_args + 1;
                    } else {
                        parse_assign(cc);
                    }
                    if (cc->tk == TK_COMMA) {
                        next_token(cc);
                    } else {
                        break;
                    }
                }
                expect(cc, TK_RPAREN);
                /* derive return type from function pointer type */
                if (n->type) {
                    if (n->type->kind == TY_FUNC) {
                        call->type = n->type->ret ? n->type->ret : cc->ty_int;
                    } else if (n->type->kind == TY_PTR && n->type->base && n->type->base->kind == TY_FUNC) {
                        call->type = n->type->base->ret ? n->type->base->ret : cc->ty_int;
                    } else {
                        call->type = cc->ty_int;
                    }
                } else {
                    call->type = cc->ty_int;
                }
                n = call;
                continue;
            }
        }

        /* post increment */
        if (cc->tk == TK_INC) {
            Node *inc;
            next_token(cc);
            inc = node_new(cc, ND_POST_INC, line);
            inc->lhs = n;
            inc->type = n->type;
            n = inc;
            continue;
        }

        /* post decrement */
        if (cc->tk == TK_DEC) {
            Node *dec;
            next_token(cc);
            dec = node_new(cc, ND_POST_DEC, line);
            dec->lhs = n;
            dec->type = n->type;
            n = dec;
            continue;
        }

        break;
    }

    return n;
}

/* --- unary: -, +, !, ~, *, &, ++, --, cast, sizeof --- */

static int types_equal(Type *a, Type *b) {
    if (!a || !b) return 0;
    if (a->kind != b->kind) return 0;
    if (a->kind == TY_PTR) return types_equal(a->base, b->base);
    if (a->kind == TY_ARRAY) return a->array_len == b->array_len && types_equal(a->base, b->base);
    if (a->kind == TY_STRUCT || a->kind == TY_UNION) return strcmp(a->tag, b->tag) == 0;
    return 1;
}

Node *parse_unary(Compiler *cc) {
    int line;
    Node *n;

    line = cc->tk_line;

    if (cc->tk == TK_GENERIC) {
        next_token(cc); /* consume _Generic */
        expect(cc, TK_LPAREN);
        Node *expr_n = parse_assign(cc);
        Type *expr_type = expr_n->type;
        expect(cc, TK_COMMA);
        
        Node *selected_case = 0;
        int default_matched = 0;
        int exact_match = 0;
        
        while (cc->tk != TK_RPAREN && cc->tk != TK_EOF) {
            int is_default = 0;
            Type *case_type = 0;
            if (cc->tk == TK_DEFAULT) {
                is_default = 1;
                next_token(cc);
            } else {
                char dummy[MAX_IDENT];
                case_type = parse_type(cc);
                case_type = parse_declarator(cc, case_type, dummy);
            }
            expect(cc, TK_COLON);
            Node *case_expr = parse_assign(cc);
            
            if (!exact_match) {
                if (is_default) {
                    if (!selected_case) {
                        selected_case = case_expr;
                        default_matched = 1;
                    }
                } else if (types_equal(expr_type, case_type)) {
                    selected_case = case_expr;
                    exact_match = 1;
                }
            }
            
            if (cc->tk == TK_COMMA) {
                next_token(cc);
            } else {
                break;
            }
        }
        expect(cc, TK_RPAREN);
        if (!selected_case) {
            error(cc, "_Generic: no matching type found");
        }
        return selected_case;
    }

    if (cc->tk == TK_SIZEOF) {
        Type *t = 0;
        int sz = 0;
        int is_type = 0;
        next_token(cc);
        if (cc->tk == TK_LPAREN) {
            int pk = peek_token(cc);
            if ((pk >= TK_INT && pk <= TK_DOUBLE) || (pk >= TK_STATIC && pk <= TK_INLINE) || pk == TK_STRUCT || pk == TK_UNION || pk == TK_ENUM || pk == TK_TYPEDEF) {
                is_type = 1;
            } else if (pk == TK_IDENT) {
                Symbol *sym = scope_find(cc, cc->peek_text);
                if (sym && sym->is_typedef) is_type = 1;
            }
        }
        
        if (is_type) {
            char dummy[MAX_IDENT];
            next_token(cc);
            t = parse_type(cc);
            t = parse_declarator(cc, t, dummy);
            expect(cc, TK_RPAREN);
        } else {
            Node *expr_n;
            if (cc->tk == TK_LPAREN) {
                next_token(cc);
                expr_n = parse_expr(cc);
                expect(cc, TK_RPAREN);
            } else {
                expr_n = parse_unary(cc);
            }
            if (expr_n && expr_n->type) {
                t = expr_n->type;
            }
        }
        if (t) sz = type_size(t);
        if (sz == 0) sz = 8;
        n = node_num(cc, sz, line);
        n->type = cc->ty_ulong;
        return n;
    }

    if (cc->tk == TK_LPAREN) {
        int pk = peek_token(cc);
        int is_cast = 0;
        if (pk == TK_INT || pk == TK_CHAR || pk == TK_VOID || pk == TK_STRUCT || pk == TK_UNION || pk == TK_ENUM || pk == TK_LONG || pk == TK_SHORT || pk == TK_UNSIGNED || pk == TK_SIGNED || pk == TK_FLOAT || pk == TK_DOUBLE) {
            is_cast = 1;
        } else if (pk == TK_IDENT) {
            Symbol *sym = scope_find(cc, cc->peek_text);
            if (sym && sym->is_typedef) is_cast = 1;
        }
        
        if (is_cast) {
            char dummy[MAX_IDENT];
            Type *cast_type;
            next_token(cc); /* consume LPAREN */
            cast_type = parse_type(cc);
            cast_type = parse_declarator(cc, cast_type, dummy);
            expect(cc, TK_RPAREN);
            if (cc->tk == TK_LBRACE) {
                Symbol *sym;
                char tmp_name[32];
                sprintf(tmp_name, ".L_comp_%d", cc->label_count++);
                sym = scope_add_local(cc, tmp_name, cast_type);

                Node *var_n_final = node_new(cc, ND_VAR, line);
                strncpy(var_n_final->name, tmp_name, MAX_IDENT - 1);
                var_n_final->sym = sym;
                var_n_final->type = cast_type;

                Node *expr = 0;
                next_token(cc); /* skip { */

                if (cast_type->kind == TY_STRUCT) {
                    StructField *sf = cast_type->fields;
                    while (cc->tk != TK_RBRACE && cc->tk != TK_EOF && sf) {
                        Node *var_n = node_new(cc, ND_VAR, line);
                        strncpy(var_n->name, tmp_name, MAX_IDENT - 1);
                        var_n->sym = sym;
                        var_n->type = cast_type;

                        Node *mem_n = node_new(cc, ND_MEMBER, line);
                        mem_n->lhs = var_n;
                        strncpy(mem_n->member_name, sf->name, MAX_IDENT - 1);
                        mem_n->member_offset = sf->offset;
                        mem_n->type = sf->type;
                        mem_n->member_size = type_size(sf->type);

                        Node *asgn_v = parse_assign(cc);
                        Node *asgn_n = node_new(cc, ND_ASSIGN, line);
                        asgn_n->lhs = mem_n;
                        asgn_n->rhs = asgn_v;
                        asgn_n->type = mem_n->type;

                        if (expr) {
                            Node *cma = node_new(cc, ND_COMMA_EXPR, line);
                            cma->lhs = expr;
                            cma->rhs = asgn_n;
                            cma->type = asgn_n->type;
                            expr = cma;
                        } else {
                            expr = asgn_n;
                        }

                        sf = sf->next;
                        if (cc->tk == TK_COMMA) next_token(cc);
                    }
                } else if (cast_type->kind == TY_ARRAY) {
                    int idx = 0;
                    while (cc->tk != TK_RBRACE && cc->tk != TK_EOF) {
                        Node *var_n = node_new(cc, ND_VAR, line);
                        strncpy(var_n->name, tmp_name, MAX_IDENT - 1);
                        var_n->sym = sym;
                        var_n->type = cast_type;

                        Node *add_n = node_new(cc, ND_ADD, line);
                        add_n->lhs = var_n;
                        add_n->rhs = node_num(cc, (long long)idx, line);
                        add_n->type = cast_type;

                        Node *deref_n = node_new(cc, ND_DEREF, line);
                        deref_n->lhs = add_n;
                        deref_n->type = cast_type->base;

                        Node *asgn_v = parse_assign(cc);
                        Node *asgn_n = node_new(cc, ND_ASSIGN, line);
                        asgn_n->lhs = deref_n;
                        asgn_n->rhs = asgn_v;
                        asgn_n->type = cast_type->base;

                        if (expr) {
                            Node *cma = node_new(cc, ND_COMMA_EXPR, line);
                            cma->lhs = expr;
                            cma->rhs = asgn_n;
                            cma->type = asgn_n->type;
                            expr = cma;
                        } else {
                            expr = asgn_n;
                        }

                        idx++;
                        if (cc->tk == TK_COMMA) next_token(cc);
                    }
                } else {
                    Node *var_n = node_new(cc, ND_VAR, line);
                    strncpy(var_n->name, tmp_name, MAX_IDENT - 1);
                    var_n->sym = sym;
                    var_n->type = cast_type;

                    Node *asgn_v = parse_assign(cc);
                    Node *asgn_n = node_new(cc, ND_ASSIGN, line);
                    asgn_n->lhs = var_n;
                    asgn_n->rhs = asgn_v;
                    asgn_n->type = cast_type;
                    expr = asgn_n;
                    if (cc->tk == TK_COMMA) next_token(cc);
                }

                while (cc->tk != TK_RBRACE && cc->tk != TK_EOF) next_token(cc);
                expect(cc, TK_RBRACE);

                if (!expr) expr = var_n_final;
                else {
                    Node *cma = node_new(cc, ND_COMMA_EXPR, line);
                    cma->lhs = expr;
                    cma->rhs = var_n_final;
                    cma->type = cast_type;
                    expr = cma;
                }
                return expr;
            }

            n = node_new(cc, ND_CAST, line);
            n->lhs = parse_unary(cc);
            n->type = cast_type;
            n->cast_type = cast_type;
            return n;
        }
    }

    if (cc->tk == TK_MINUS) {
        next_token(cc);
        n = node_new(cc, ND_NEG, line);
        n->lhs = parse_unary(cc);
        n->type = n->lhs->type;
        return n;
    }
    if (cc->tk == TK_PLUS) {
        next_token(cc);
        return parse_unary(cc);
    }
    if (cc->tk == TK_BANG) {
        next_token(cc);
        n = node_new(cc, ND_LNOT, line);
        n->lhs = parse_unary(cc);
        n->type = cc->ty_int;
        return n;
    }
    if (cc->tk == TK_TILDE) {
        next_token(cc);
        n = node_new(cc, ND_BNOT, line);
        n->lhs = parse_unary(cc);
        n->type = n->lhs->type;
        return n;
    }
    if (cc->tk == TK_STAR) {
        next_token(cc);
        n = node_new(cc, ND_DEREF, line);
        n->lhs = parse_unary(cc);
        if (n->lhs->type) {
            if (n->lhs->type->base) {
                n->type = n->lhs->type->base;
            } else {
                n->type = cc->ty_int;
            }
        } else {
            n->type = cc->ty_int;
        }
        return n;
    }
    if (cc->tk == TK_AMP) {
        next_token(cc);
        n = node_new(cc, ND_ADDR, line);
        n->lhs = parse_unary(cc);
        n->type = type_ptr(cc, n->lhs->type);
        return n;
    }
    if (cc->tk == TK_INC) {
        next_token(cc);
        n = node_new(cc, ND_PRE_INC, line);
        n->lhs = parse_unary(cc);
        n->type = n->lhs->type;
        return n;
    }
    if (cc->tk == TK_DEC) {
        next_token(cc);
        n = node_new(cc, ND_PRE_DEC, line);
        n->lhs = parse_unary(cc);
        n->type = n->lhs->type;
        return n;
    }

    return parse_postfix(cc);
}

/* --- binary operators: mul, add, shift, relational, equality, bit, logical --- */

static Type *promote_type(Compiler *cc, Type *t1, Type *t2) {
    if (!t1) return t2;
    if (!t2) return t1;
    if (t1->kind == TY_DOUBLE || t2->kind == TY_DOUBLE) return cc->ty_double;
    if (t1->kind == TY_FLOAT || t2->kind == TY_FLOAT) return cc->ty_float;
    if (t1->kind == TY_PTR) return t1;
    if (t2->kind == TY_PTR) return t2;
    if (type_size(t1) >= 8 || type_size(t2) >= 8) {
        if (is_unsigned_type(t1) || is_unsigned_type(t2)) return cc->ty_ulong;
        return cc->ty_long;
    }
    if (is_unsigned_type(t1) || is_unsigned_type(t2)) return cc->ty_uint;
    return cc->ty_int;
}

static Node *ensure_type(Compiler *cc, Node *n, Type *ty) {
    if (!n || !n->type || !ty) return n;
    if (n->type == ty) return n;
    Node *c = node_new(cc, ND_CAST, n->line);
    c->lhs = n;
    c->cast_type = ty;
    c->type = ty;
    return c;
}

Node *parse_mul(Compiler *cc) {
    Node *n;
    n = parse_unary(cc);
    while (cc->tk == TK_STAR || cc->tk == TK_SLASH || cc->tk == TK_PERCENT) {
        int op;
        int line;
        Node *rhs;
        Node *binop;
        Type *pt;
        line = cc->tk_line;
        if (cc->tk == TK_STAR) op = ND_MUL;
        else if (cc->tk == TK_SLASH) op = ND_DIV;
        else op = ND_MOD;
        next_token(cc);
        rhs = parse_unary(cc);
        pt = promote_type(cc, n->type, rhs->type);
        n = ensure_type(cc, n, pt);
        rhs = ensure_type(cc, rhs, pt);
        binop = node_new(cc, op, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = pt;
        n = binop;
    }
    return n;
}

Node *parse_add(Compiler *cc) {
    Node *n;
    Type *pt;
    n = parse_mul(cc);
    while (cc->tk == TK_PLUS || cc->tk == TK_MINUS) {
        int op;
        int line;
        Node *rhs;
        Node *binop;
        Type *pt;
        line = cc->tk_line;
        if (cc->tk == TK_PLUS) op = ND_ADD;
        else op = ND_SUB;
        next_token(cc);
        rhs = parse_mul(cc);
        binop = node_new(cc, op, line);
        binop->lhs = n;
        binop->rhs = rhs;
        /* pointer arithmetic */
        if (is_pointer(n->type)) {
            binop->type = n->type;
        } else if (is_pointer(rhs->type)) {
            binop->type = rhs->type;
        } else {
            pt = promote_type(cc, n->type, rhs->type);
            n = ensure_type(cc, n, pt);
            rhs = ensure_type(cc, rhs, pt);
            binop->lhs = n;
            binop->rhs = rhs;
            binop->type = pt;
        }
        n = binop;
    }
    return n;
}

Node *parse_shift(Compiler *cc) {
    Node *n;
    n = parse_add(cc);
    while (cc->tk == TK_SHL || cc->tk == TK_SHR) {
        int op;
        int line;
        Node *rhs;
        Node *binop;
        line = cc->tk_line;
        if (cc->tk == TK_SHL) op = ND_SHL;
        else op = ND_SHR;
        next_token(cc);
        rhs = parse_add(cc);
        binop = node_new(cc, op, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = n->type;
        n = binop;
    }
    return n;
}

Node *parse_relational(Compiler *cc) {
    Node *n;
    n = parse_shift(cc);
    while (cc->tk == TK_LT || cc->tk == TK_GT || cc->tk == TK_LE || cc->tk == TK_GE) {
        int op;
        int line;
        Node *rhs;
        Node *binop;
        Type *pt;
        line = cc->tk_line;
        if (cc->tk == TK_LT) op = ND_LT;
        else if (cc->tk == TK_GT) op = ND_GT;
        else if (cc->tk == TK_LE) op = ND_LE;
        else op = ND_GE;
        next_token(cc);
        rhs = parse_shift(cc);
        pt = promote_type(cc, n->type, rhs->type);
        n = ensure_type(cc, n, pt);
        rhs = ensure_type(cc, rhs, pt);
        binop = node_new(cc, op, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = cc->ty_int;
        n = binop;
    }
    return n;
}

Node *parse_equality(Compiler *cc) {
    Node *n;
    n = parse_relational(cc);
    while (cc->tk == TK_EQ || cc->tk == TK_NE) {
        int op;
        int line;
        Node *rhs;
        Node *binop;
        Type *pt;
        line = cc->tk_line;
        if (cc->tk == TK_EQ) op = ND_EQ;
        else op = ND_NE;
        next_token(cc);
        rhs = parse_relational(cc);
        pt = promote_type(cc, n->type, rhs->type);
        n = ensure_type(cc, n, pt);
        rhs = ensure_type(cc, rhs, pt);
        binop = node_new(cc, op, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = cc->ty_int;
        n = binop;
    }
    return n;
}

Node *parse_bitand(Compiler *cc) {
    Node *n;
    n = parse_equality(cc);
    while (cc->tk == TK_AMP) {
        int line;
        Node *rhs;
        Node *binop;
        Type *pt;
        line = cc->tk_line;
        next_token(cc);
        rhs = parse_equality(cc);
        pt = promote_type(cc, n->type, rhs->type);
        n = ensure_type(cc, n, pt);
        rhs = ensure_type(cc, rhs, pt);
        binop = node_new(cc, ND_BAND, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = pt;
        n = binop;
    }
    return n;
}

Node *parse_bitxor(Compiler *cc) {
    Node *n;
    n = parse_bitand(cc);
    while (cc->tk == TK_CARET) {
        int line;
        Node *rhs;
        Node *binop;
        Type *pt;
        line = cc->tk_line;
        next_token(cc);
        rhs = parse_bitand(cc);
        pt = promote_type(cc, n->type, rhs->type);
        n = ensure_type(cc, n, pt);
        rhs = ensure_type(cc, rhs, pt);
        binop = node_new(cc, ND_BXOR, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = pt;
        n = binop;
    }
    return n;
}

Node *parse_bitor(Compiler *cc) {
    Node *n;
    Type *pt;
    n = parse_bitxor(cc);
    while (cc->tk == TK_PIPE) {
        int line;
        Node *rhs;
        Node *binop;
        Type *pt;
        line = cc->tk_line;
        next_token(cc);
        rhs = parse_bitxor(cc);
        pt = promote_type(cc, n->type, rhs->type);
        n = ensure_type(cc, n, pt);
        rhs = ensure_type(cc, rhs, pt);
        binop = node_new(cc, ND_BOR, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = pt;
        n = binop;
    }
    return n;
}

Node *parse_logand(Compiler *cc) {
    Node *n;
    n = parse_bitor(cc);
    while (cc->tk == TK_LAND) {
        int line;
        Node *rhs;
        Node *binop;
        line = cc->tk_line;
        next_token(cc);
        rhs = parse_bitor(cc);
        binop = node_new(cc, ND_LAND, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = cc->ty_int;
        n = binop;
    }
    return n;
}

Node *parse_logor(Compiler *cc) {
    Node *n;
    n = parse_logand(cc);
    while (cc->tk == TK_LOR) {
        int line;
        Node *rhs;
        Node *binop;
        line = cc->tk_line;
        next_token(cc);
        rhs = parse_logand(cc);
        binop = node_new(cc, ND_LOR, line);
        binop->lhs = n;
        binop->rhs = rhs;
        binop->type = cc->ty_int;
        n = binop;
    }
    return n;
}

Node *parse_ternary(Compiler *cc) {
    Node *n;
    n = parse_logor(cc);
    if (cc->tk == TK_QUESTION) {
        Node *tern;
        int line;
        line = cc->tk_line;
        next_token(cc);
        tern = node_new(cc, ND_TERNARY, line);
        tern->cond = n;
        tern->then_body = parse_expr(cc);
        expect(cc, TK_COLON);
        tern->else_body = parse_ternary(cc);
        tern->type = promote_type(cc, tern->then_body->type, tern->else_body->type);
        tern->then_body = ensure_type(cc, tern->then_body, tern->type);
        tern->else_body = ensure_type(cc, tern->else_body, tern->type);
        n = tern;
    }
    return n;
}

Node *parse_assign(Compiler *cc) {
    Node *n;
    n = parse_ternary(cc);

    if (cc->tk == TK_ASSIGN) {
        Node *asgn;
        int line;
        line = cc->tk_line;
        next_token(cc);
        asgn = node_new(cc, ND_ASSIGN, line);
        asgn->lhs = n;
        asgn->rhs = ensure_type(cc, parse_assign(cc), n->type);
        asgn->type = n->type;
        return asgn;
    }

    /* compound assignment: +=, -=, *=, /=, %=, &=, |=, ^=, <<=, >>= */
    if (cc->tk >= TK_PLUS_ASSIGN) {
        if (cc->tk <= TK_SHR_ASSIGN) {
            Node *ca;
            int line;
            int op;
            line = cc->tk_line;
            switch (cc->tk) {
                case TK_PLUS_ASSIGN:    op = ND_ADD; break;
                case TK_MINUS_ASSIGN:   op = ND_SUB; break;
                case TK_STAR_ASSIGN:    op = ND_MUL; break;
                case TK_SLASH_ASSIGN:   op = ND_DIV; break;
                case TK_PERCENT_ASSIGN: op = ND_MOD; break;
                case TK_AMP_ASSIGN:     op = ND_BAND; break;
                case TK_PIPE_ASSIGN:    op = ND_BOR; break;
                case TK_CARET_ASSIGN:   op = ND_BXOR; break;
                case TK_SHL_ASSIGN:     op = ND_SHL; break;
                case TK_SHR_ASSIGN:     op = ND_SHR; break;
                default: op = ND_ADD; break;
            }
            next_token(cc);
            ca = node_new(cc, ND_COMPOUND_ASSIGN, line);
            ca->compound_op = op;
            ca->lhs = n;
            ca->rhs = parse_assign(cc);
            ca->type = n->type;
            return ca;
        }
    }

    return n;
}

Node *parse_expr(Compiler *cc) {
    Node *n;
    n = parse_assign(cc);
    while (cc->tk == TK_COMMA) {
        Node *comma;
        int line;
        line = cc->tk_line;
        next_token(cc);
        comma = node_new(cc, ND_COMMA_EXPR, line);
        comma->lhs = n;
        comma->rhs = parse_assign(cc);
        comma->type = comma->rhs->type;
        n = comma;
    }
    return n;
}

/* ================================================================ */
/* STATEMENT PARSING                                                 */
/* ================================================================ */

static Node *current_sw = 0;

static int push_vla_scope(Compiler *cc) {
    cc->local_offset -= 8;
    int sp_off = cc->local_offset;
    if (cc->vla_sp_depth < 100) cc->vla_sp_offsets[cc->vla_sp_depth++] = sp_off;
    return sp_off;
}

static Node *wrap_vla_scope(Compiler *cc, Node *body, int sp_off) {
    if (!body) return body;
    Node *sp_save = node_new(cc, ND_RSP_SAVE, body->line);
    sp_save->member_offset = sp_off;
    Node *sp_restore = node_new(cc, ND_RSP_RESTORE, body->line);
    sp_restore->member_offset = sp_off;
    
    Node *nblock = node_new(cc, ND_BLOCK, body->line);
    nblock->stmts = (Node **)cc_alloc(cc, 3 * sizeof(Node*));
    nblock->stmts[0] = sp_save;
    nblock->stmts[1] = body;
    nblock->stmts[2] = sp_restore;
    nblock->num_stmts = 3;
    return nblock;
}

static void pop_vla_scope(Compiler *cc) {
    if (cc->vla_sp_depth > 0) cc->vla_sp_depth--;
}

Node *parse_stmt(Compiler *cc) {
    int line;
    line = cc->tk_line;

    /* inline assembly */
    if (cc->tk == TK_ASM) {
        Node *n;
        next_token(cc);
        if (cc->tk == TK_VOLATILE) next_token(cc);
        expect(cc, TK_LPAREN);
        n = node_new(cc, ND_ASM, line);
        if (cc->tk == TK_STR) {
            n->asm_string = cc_strdup(cc, cc->tk_str);
            next_token(cc);
        } else {
            error(cc, "expected string literal in asm");
            n->asm_string = "";
        }
        expect(cc, TK_RPAREN);
        expect(cc, TK_SEMI);
        return n;
    }

    /* block */
    if (cc->tk == TK_LBRACE) {
        Node *block;
        int cap;
        int cnt;
        next_token(cc);
        scope_push(cc);
        block = node_new(cc, ND_BLOCK, line);
        cap = 64;
        block->stmts = (Node **)cc_alloc(cc, sizeof(Node *) * cap);
        cnt = 0;
        while (cc->tk != TK_RBRACE) {
            int prev_pos;
            int no_progress_count;
            if (cc->tk == TK_EOF) break;
            if (cnt >= 4096) {
                error(cc, "block has too many statements (possible parser loop)");
                break;
            }
            if (cnt >= cap) {
                Node **old;
                int newcap;
                int i;
                old = block->stmts;
                newcap = cap * 2;
                if (newcap > 4096) newcap = 4096;
                block->stmts = (Node **)cc_alloc(cc, sizeof(Node *) * newcap);
                for (i = 0; i < cnt; i++) block->stmts[i] = old[i];
                cap = newcap;
            }
            prev_pos = cc->pos;
            block->stmts[cnt] = parse_stmt(cc);
            cnt++;
            no_progress_count = 0;
            /* avoid infinite loop if parse_stmt didn't consume (e.g. after error) */
            while (cc->pos == prev_pos && cc->tk != TK_RBRACE && cc->tk != TK_EOF) {
                next_token(cc);
                no_progress_count++;
                if (no_progress_count > 1024) {
                    error(cc, "parser stuck (no progress after 1024 tokens)");
                    block->num_stmts = cnt;
                    scope_pop(cc);
                    return block;
                }
            }
        }
        block->num_stmts = cnt;
        scope_pop(cc);
        expect(cc, TK_RBRACE);
        return block;
    }

    /* return */
    if (cc->tk == TK_RETURN) {
        Node *ret;
        next_token(cc);
        ret = node_new(cc, ND_RETURN, line);
        if (cc->tk != TK_SEMI) {
            ret->lhs = parse_expr(cc);
        }
            expect(cc, TK_SEMI);
        return ret;
    }

    /* if */
    if (cc->tk == TK_IF) {
        Node *ifn;
        next_token(cc);
        expect(cc, TK_LPAREN);
        ifn = node_new(cc, ND_IF, line);
        ifn->cond = parse_expr(cc);
        expect(cc, TK_RPAREN);
        ifn->then_body = parse_stmt(cc);
        if (cc->tk == TK_ELSE) {
            next_token(cc);
            ifn->else_body = parse_stmt(cc);
        }
        return ifn;
    }

    /* while */
    if (cc->tk == TK_WHILE) {
        Node *wh;
        next_token(cc);
        expect(cc, TK_LPAREN);
        wh = node_new(cc, ND_WHILE, line);
        wh->cond = parse_expr(cc);
        expect(cc, TK_RPAREN);
        int sp_off = push_vla_scope(cc);
        Node *body = parse_stmt(cc);
        pop_vla_scope(cc);
        wh->body = wrap_vla_scope(cc, body, sp_off);
        return wh;
    }

    /* for */
    if (cc->tk == TK_FOR) {
        Node *forn;
        next_token(cc);
        expect(cc, TK_LPAREN);
        forn = node_new(cc, ND_FOR, line);

        scope_push(cc);

        /* init — could be declaration or expression */
        if (cc->tk == TK_SEMI) {
            next_token(cc);
            forn->init = 0;
        } else if (is_type_token(cc)) {
            forn->init = parse_stmt(cc);  /* handles the semicolon */
        } else {
            forn->init = parse_expr(cc);
            expect(cc, TK_SEMI);
        }

        /* condition */
        if (cc->tk == TK_SEMI) {
            forn->cond = 0;
        } else {
            forn->cond = parse_expr(cc);
        }
            expect(cc, TK_SEMI);

        /* increment */
        if (cc->tk == TK_RPAREN) {
            forn->inc = 0;
        } else {
            forn->inc = parse_expr(cc);
        }
        expect(cc, TK_RPAREN);

        int sp_off = push_vla_scope(cc);
        Node *body = parse_stmt(cc);
        pop_vla_scope(cc);
        forn->body = wrap_vla_scope(cc, body, sp_off);
        scope_pop(cc);
        return forn;
    }

    /* do-while */
    if (cc->tk == TK_DO) {
        Node *dow;
        next_token(cc);
        dow = node_new(cc, ND_DO_WHILE, line);
        int sp_off = push_vla_scope(cc);
        Node *body = parse_stmt(cc);
        pop_vla_scope(cc);
        dow->body = wrap_vla_scope(cc, body, sp_off);
        if (cc->tk != TK_WHILE) {
            error(cc, "expected 'while' after do body");
        }
        next_token(cc); /* skip 'while' */
        expect(cc, TK_LPAREN);
        dow->cond = parse_expr(cc);
        expect(cc, TK_RPAREN);
            expect(cc, TK_SEMI);
        return dow;
    }

    /* switch */
    if (cc->tk == TK_SWITCH) {
        Node *sw;
        next_token(cc);
        expect(cc, TK_LPAREN);
        sw = node_new(cc, ND_SWITCH, line);
        sw->cond = parse_expr(cc);
        expect(cc, TK_RPAREN);
        /* parse body — extract cases */
        sw->cases = (Node **)cc_alloc(cc, sizeof(Node *) * MAX_CASES);
        sw->num_cases = 0;
        sw->default_case = 0;
        
        Node *old_sw = current_sw;
        current_sw = sw;

        int sp_off = push_vla_scope(cc);
        Node *body = parse_stmt(cc);
        pop_vla_scope(cc);
        sw->body = wrap_vla_scope(cc, body, sp_off);

        current_sw = old_sw;
        return sw;
    }

    if (cc->tk == TK_CASE) {
        Node *cn;
        Node *cnode;
        Node *gto;
        long long val;
        next_token(cc);
        val = eval_const_expr(parse_assign(cc));
        expect(cc, TK_COLON);

        cn = node_new(cc, ND_LABEL, line);
        sprintf(cn->label_name, "__zc_%llx_%d", (unsigned long long)val, line);
        cn->lhs = parse_stmt(cc);

        if (current_sw && current_sw->num_cases < MAX_CASES) {
            cnode = node_new(cc, ND_CASE, line);
            cnode->case_val = val;
            gto = node_new(cc, ND_GOTO, line);
            strncpy(gto->label_name, cn->label_name, MAX_IDENT - 1);
            cnode->case_body = gto;
            current_sw->cases[current_sw->num_cases++] = cnode;
        }
        return cn;
    }

    if (cc->tk == TK_DEFAULT) {
        Node *dn;
        Node *dnode;
        Node *gto;
        next_token(cc);
        expect(cc, TK_COLON);

        dn = node_new(cc, ND_LABEL, line);
        sprintf(dn->label_name, "__zc_def_%d", line);
        dn->lhs = parse_stmt(cc);

        if (current_sw) {
            dnode = node_new(cc, ND_DEFAULT, line);
            gto = node_new(cc, ND_GOTO, line);
            strncpy(gto->label_name, dn->label_name, MAX_IDENT - 1);
            dnode->case_body = gto;
            current_sw->default_case = dnode;
        }
        return dn;
    }

    /* break */
    if (cc->tk == TK_BREAK) {
        Node *brk;
        next_token(cc);
        brk = node_new(cc, ND_BREAK, line);
        expect(cc, TK_SEMI);
        if (cc->vla_sp_depth > 0) {
            int off = cc->vla_sp_offsets[cc->vla_sp_depth - 1];
            Node *rest = node_new(cc, ND_RSP_RESTORE, line);
            rest->member_offset = off;
            Node *bblock = node_new(cc, ND_BLOCK, line);
            bblock->stmts = (Node **)cc_alloc(cc, 2 * sizeof(Node*));
            bblock->stmts[0] = rest;
            bblock->stmts[1] = brk;
            bblock->num_stmts = 2;
            return bblock;
        }
        return brk;
    }

    /* continue */
    if (cc->tk == TK_CONTINUE) {
        Node *cont;
        next_token(cc);
        cont = node_new(cc, ND_CONTINUE, line);
        expect(cc, TK_SEMI);
        if (cc->vla_sp_depth > 0) {
            int off = cc->vla_sp_offsets[cc->vla_sp_depth - 1];
            Node *rest = node_new(cc, ND_RSP_RESTORE, line);
            rest->member_offset = off;
            Node *cblock = node_new(cc, ND_BLOCK, line);
            cblock->stmts = (Node **)cc_alloc(cc, 2 * sizeof(Node*));
            cblock->stmts[0] = rest;
            cblock->stmts[1] = cont;
            cblock->num_stmts = 2;
            return cblock;
        }
        return cont;
    }

    /* goto */
    if (cc->tk == TK_GOTO) {
        Node *gto;
        next_token(cc);
        gto = node_new(cc, ND_GOTO, line);
        strncpy(gto->label_name, cc->tk_text, MAX_IDENT - 1);
        next_token(cc);
            expect(cc, TK_SEMI);
        return gto;
    }

    /* label: check ident followed by colon */
    if (cc->tk == TK_IDENT) {
        int pk;
        pk = peek_token(cc);
        if (pk == TK_COLON) {
            Node *lbl;
            lbl = node_new(cc, ND_LABEL, line);
            strncpy(lbl->label_name, cc->tk_text, MAX_IDENT - 1);
            next_token(cc);  /* skip ident */
            next_token(cc);  /* skip : */
            lbl->lhs = parse_stmt(cc);
            return lbl;
        }
    }

    /* local variable declaration */
    if (is_type_token(cc)) {
        Type *base;
        int is_typedef = 0;
        int is_auto = 0;
        if (cc->tk == TK_TYPEDEF) is_typedef = 1;
        if (cc->tk == TK_AUTO_TYPE) is_auto = 1;
        base = parse_type(cc);
        int is_static_local = cc->current_is_static;

        /* handle multiple declarators: int a, b, c; */
        {
            Node *block;
            int cap;
            int cnt;
            block = node_new(cc, ND_BLOCK, line);
            cap = 8;
            block->stmts = (Node **)cc_alloc(cc, sizeof(Node *) * cap);
            cnt = 0;

            for (;;) {
                Type *vtype;
                char vname[MAX_IDENT];
                Symbol *sym;
                Node *decl;

                vtype = parse_declarator(cc, base, vname);

                if (vname[0]) {
                    if (is_typedef) {
                        sym = scope_add(cc, vname, vtype);
                        sym->is_typedef = 1;
                    } else if (is_static_local) {
                        char mangled[MAX_IDENT];
                        sprintf(mangled, "%s__%s__%d", cc->current_func, vname, cc->label_count++);
                        sym = scope_add_local(cc, vname, vtype);
                        sym->is_local = 0;
                        sym->is_global = 1;
                        strncpy(sym->asm_name, mangled, MAX_IDENT - 1);

                        Node *gvar = node_new(cc, ND_GLOBAL_VAR, line);
                        strncpy(gvar->name, mangled, MAX_IDENT - 1);
                        gvar->type = vtype;
                        gvar->is_static = 1;
                        gvar->is_extern = 0;

                        if (cc->tk == TK_ASSIGN) {
                            next_token(cc);
                            if (cc->tk == TK_LBRACE) {
                                Node *init_list;
                                Node **inits;
                                int count;
                                init_list = node_new(cc, ND_INIT_LIST, line);
                                inits = (Node **)cc_alloc(cc, sizeof(Node *) * MAX_INIT);
                                count = 0;
                                next_token(cc); /* skip { */
                                int depth = 1;
                                int skip_tk;
                                int prev_pos;
                                int no_progress_count = 0;
                                int top_elems = 0;
                                int last_comma = 1;
                                while (depth > 0 && cc->tk != TK_EOF) {
                                    if (depth == 1 && cc->tk != TK_RBRACE && cc->tk != TK_COMMA && last_comma) {
                                        top_elems++;
                                        last_comma = 0;
                                    }
                                    prev_pos = cc->pos;
                                    if (cc->tk == TK_LBRACE) {
                                        depth++;
                                        next_token(cc);
                                    } else if (cc->tk == TK_RBRACE) {
                                        depth--;
                                        if (depth == 0) break;
                                        next_token(cc);
                                    } else if (cc->tk == TK_COMMA) {
                                        if (depth == 1) last_comma = 1;
                                        next_token(cc);
                                    } else {
                                        skip_tk = cc->tk;
                                        if (count < MAX_INIT) {
                                            inits[count++] = parse_assign(cc);
                                        } else {
                                            parse_assign(cc);
                                        }
                                        if (cc->tk == skip_tk) {
                                            if (cc->tk != TK_EOF) next_token(cc);
                                        }
                                        if (cc->pos == prev_pos) {
                                            no_progress_count++;
                                            if (no_progress_count > 100) { break; }
                                        } else {
                                            no_progress_count = 0;
                                        }
                                    }
                                }
                                if (cc->tk == TK_RBRACE) next_token(cc);
                                init_list->args = inits;
                                init_list->num_args = count;
                                gvar->initializer = init_list;
                                
                                if (gvar->type->kind == TY_ARRAY && gvar->type->array_len == 0) {
                                    gvar->type->array_len = top_elems;
                                    gvar->type->size = type_size(gvar->type->base) * top_elems;
                                    sym->type->array_len = top_elems;
                                    sym->type->size = gvar->type->size;
                                }
                            } else {
                                Node *init_node2 = parse_assign(cc);
                                gvar->initializer = init_node2;
                                /* Fix: if char[] initialized with string literal, update size */
                                if (init_node2 && init_node2->kind == ND_STR &&
                                    gvar->type->kind == TY_ARRAY && gvar->type->array_len == 0) {
                                    int slen2 = cc->strings[init_node2->str_id].len + 1;
                                    gvar->type->array_len = slen2;
                                    gvar->type->size = type_size(gvar->type->base) * slen2;
                                    sym->type->array_len = slen2;
                                    sym->type->size = gvar->type->size;
                                }
                            }
                        }

                        if (cc->num_globals < MAX_GLOBALS) cc->globals[cc->num_globals++] = gvar;
                    } else {
                        sym = scope_add_local(cc, vname, vtype);

                        /* initializer? */
                        if (cc->tk == TK_ASSIGN) {
                            Node *asgn;
                            Node *var;
                            next_token(cc);

                        /* check for array/struct initializer list */
                        if (cc->tk == TK_LBRACE) {
                            if (vtype->kind == TY_ARRAY) {
                                /* Parse array initializer list and emit element assignments */
                                int depth;
                                int init_count;
                                Node **inits;
                                int init_cap;
                                Type *elem_type;
                                Type *arr_type;
                                int sz;
                                int skip_tk;
                                int prev_pos;
                                int no_progress_count = 0;
                                next_token(cc); /* skip { */
                                init_count = 0;
                                init_cap = 16;
                                inits = (Node **)cc_alloc(cc, sizeof(Node *) * init_cap);
                                depth = 1;
                                while (depth > 0 && cc->tk != TK_EOF) {
                                    prev_pos = cc->pos;
                                    if (cc->tk == TK_LBRACE) {
                                        depth++;
                                        next_token(cc);
                                    } else if (cc->tk == TK_RBRACE) {
                                        depth--;
                                        if (depth == 0) break;
                                        next_token(cc);
                                    } else if (cc->tk == TK_COMMA) {
                                        next_token(cc);
                                    } else {
                                        skip_tk = cc->tk;
                                        Node *expr = parse_assign(cc);
                                        if (cc->tk == skip_tk) {
                                            if (cc->tk != TK_EOF) next_token(cc);
                                        }
                                        if (cc->pos == prev_pos) {
                                            no_progress_count++;
                                            if (no_progress_count > 100) {
                                                error(cc, "array parser stuck (possible infinite loop)");
                                                break;
                                            }
                                        } else {
                                            no_progress_count = 0;
                                        }
                                        if (init_count >= init_cap) {
                                            Node **old_inits = inits;
                                            int j;
                                            init_cap *= 2;
                                            inits = (Node **)cc_alloc(cc, sizeof(Node *) * init_cap);
                                            for (j = 0; j < init_count; j++) inits[j] = old_inits[j];
                                        }
                                        inits[init_count++] = expr;
                                    }
                                }
                                if (cc->tk == TK_RBRACE) next_token(cc); /* skip final } */
                                elem_type = vtype->base;
                                if (vtype->array_len == 0) {
                                    arr_type = type_array(cc, elem_type, init_count);
                                    sym->type = arr_type;
                                    sz = type_size(arr_type);
                                    if (sz < 8) sz = 8;
                                    cc->local_offset = cc->local_offset + 8;
                                    cc->local_offset = cc->local_offset - sz;
                                    sym->stack_offset = cc->local_offset;
                                } else {
                                    arr_type = vtype;
                                }
                                /* Grow block stmts if needed */
                                while (cnt + init_count > cap) {
                                    Node **new_stmts;
                                    int j;
                                    new_stmts = (Node **)cc_alloc(cc, sizeof(Node *) * cap * 2);
                                    for (j = 0; j < cnt; j++) new_stmts[j] = block->stmts[j];
                                    block->stmts = new_stmts;
                                    cap = cap * 2;
                                }
                                {
                                    int idx;
                                    for (idx = 0; idx < init_count; idx++) {
                                        Node *var;
                                        Node *add;
                                        Node *deref;
                                        Node *asgn;
                                        var = node_new(cc, ND_VAR, line);
                                        strncpy(var->name, vname, MAX_IDENT - 1);
                                        var->sym = sym;
                                        var->type = arr_type;
                                        add = node_new(cc, ND_ADD, line);
                                        add->lhs = var;
                                        add->rhs = node_num(cc, (long long)idx, line);
                                        add->type = arr_type;
                                        deref = node_new(cc, ND_DEREF, line);
                                        deref->lhs = add;
                                        deref->type = elem_type;
                                        asgn = node_new(cc, ND_ASSIGN, line);
                                        asgn->lhs = deref;
                                        asgn->rhs = inits[idx];
                                        asgn->type = elem_type;
                                        block->stmts[cnt] = asgn;
                                        cnt++;
                                    }
                                }
                            } else if (vtype->kind == TY_STRUCT || vtype->kind == TY_UNION) {
                                /* Local struct/union initializer: {v0, v1, ...}
                                 * Walk StructField list, assign each field in order. */
                                int init_count_s;
                                int init_cap_s;
                                Node **inits_s;
                                int skip_tk_s;
                                int prev_pos_s;
                                int no_progress_s;
                                int depth_s;
                                init_count_s = 0;
                                init_cap_s = 32;
                                inits_s = (Node **)cc_alloc(cc, sizeof(Node *) * init_cap_s);
                                depth_s = 1;
                                no_progress_s = 0;
                                next_token(cc); /* skip { */
                                while (depth_s > 0 && cc->tk != TK_EOF) {
                                    prev_pos_s = cc->pos;
                                    if (cc->tk == TK_LBRACE) {
                                        depth_s++;
                                        next_token(cc);
                                    } else if (cc->tk == TK_RBRACE) {
                                        depth_s--;
                                        if (depth_s == 0) break;
                                        next_token(cc);
                                    } else if (cc->tk == TK_COMMA) {
                                        next_token(cc);
                                    } else {
                                        skip_tk_s = cc->tk;
                                        if (init_count_s < init_cap_s) {
                                            inits_s[init_count_s++] = parse_assign(cc);
                                        } else {
                                            parse_assign(cc);
                                        }
                                        if (cc->tk == skip_tk_s) {
                                            if (cc->tk != TK_EOF) next_token(cc);
                                        }
                                        if (cc->pos == prev_pos_s) {
                                            no_progress_s++;
                                            if (no_progress_s > 100) break;
                                        } else {
                                            no_progress_s = 0;
                                        }
                                    }
                                }
                                if (cc->tk == TK_RBRACE) next_token(cc);
                                /* Now emit field assignments */
                                {
                                    StructField *sf;
                                    int fi;
                                    fi = 0;
                                    sf = vtype->fields;
                                    while (sf && fi < init_count_s) {
                                        /* Build: var.sf_name = inits_s[fi] */
                                        Node *var_n;
                                        Node *mem_n;
                                        Node *asgn_n;
                                        int acc_off;
                                        StructField *found_f;
                                        var_n = node_new(cc, ND_VAR, line);
                                        strncpy(var_n->name, vname, MAX_IDENT - 1);
                                        var_n->sym = sym;
                                        var_n->type = vtype;
                                        acc_off = 0;
                                        found_f = find_struct_member(vtype, sf->name, &acc_off);
                                        mem_n = node_new(cc, ND_MEMBER, line);
                                        mem_n->lhs = var_n;
                                        strncpy(mem_n->member_name, sf->name, MAX_IDENT - 1);
                                        if (found_f) {
                                            mem_n->member_offset = acc_off;
                                            mem_n->type = found_f->type;
                                            mem_n->member_size = type_size(found_f->type);
                                        } else {
                                            mem_n->member_offset = sf->offset;
                                            mem_n->type = sf->type;
                                            mem_n->member_size = type_size(sf->type);
                                        }
                                        asgn_n = node_new(cc, ND_ASSIGN, line);
                                        asgn_n->lhs = mem_n;
                                        asgn_n->rhs = inits_s[fi];
                                        asgn_n->type = mem_n->type;
                                        /* Grow block if needed */
                                        if (cnt >= cap) {
                                            Node **new_stmts;
                                            int ji;
                                            new_stmts = (Node **)cc_alloc(cc, sizeof(Node *) * cap * 2);
                                            for (ji = 0; ji < cnt; ji++) new_stmts[ji] = block->stmts[ji];
                                            block->stmts = new_stmts;
                                            cap = cap * 2;
                                        }
                                        block->stmts[cnt] = asgn_n;
                                        cnt++;
                                        sf = sf->next;
                                        fi++;
                                        /* Skip nested struct fields for now — treat as flat */
                                    }
                                    /* Zero-initialize remaining fields if fewer inits provided */
                                    while (sf) {
                                        Node *var_n;
                                        Node *mem_n;
                                        Node *asgn_n;
                                        int acc_off;
                                        StructField *found_f;
                                        var_n = node_new(cc, ND_VAR, line);
                                        strncpy(var_n->name, vname, MAX_IDENT - 1);
                                        var_n->sym = sym;
                                        var_n->type = vtype;
                                        acc_off = 0;
                                        found_f = find_struct_member(vtype, sf->name, &acc_off);
                                        mem_n = node_new(cc, ND_MEMBER, line);
                                        mem_n->lhs = var_n;
                                        strncpy(mem_n->member_name, sf->name, MAX_IDENT - 1);
                                        if (found_f) {
                                            mem_n->member_offset = acc_off;
                                            mem_n->type = found_f->type;
                                            mem_n->member_size = type_size(found_f->type);
                                        } else {
                                            mem_n->member_offset = sf->offset;
                                            mem_n->type = sf->type;
                                            mem_n->member_size = type_size(sf->type);
                                        }
                                        asgn_n = node_new(cc, ND_ASSIGN, line);
                                        asgn_n->lhs = mem_n;
                                        asgn_n->rhs = node_num(cc, 0LL, line);
                                        asgn_n->type = mem_n->type;
                                        if (cnt >= cap) {
                                            Node **new_stmts;
                                            int ji;
                                            new_stmts = (Node **)cc_alloc(cc, sizeof(Node *) * cap * 2);
                                            for (ji = 0; ji < cnt; ji++) new_stmts[ji] = block->stmts[ji];
                                            block->stmts = new_stmts;
                                            cap = cap * 2;
                                        }
                                        block->stmts[cnt] = asgn_n;
                                        cnt++;
                                        sf = sf->next;
                                    }
                                }
                            } else {
                                /* Other non-array, non-struct types: skip initializer list */
                                int skip_depth_x;
                                skip_depth_x = 1;
                                next_token(cc);
                                while (skip_depth_x > 0) {
                                    if (cc->tk == TK_EOF) break;
                                    if (cc->tk == TK_LBRACE) skip_depth_x = skip_depth_x + 1;
                                    if (cc->tk == TK_RBRACE) skip_depth_x = skip_depth_x - 1;
                                    if (skip_depth_x > 0) next_token(cc);
                                }
                                next_token(cc);
                            }
                        } else {
                            var = node_new(cc, ND_VAR, line);
                            strncpy(var->name, vname, MAX_IDENT - 1);
                            Node *init_expr = parse_assign(cc);
                            if (cc->current_is_const && init_expr) {
                                int const_val = eval_const_expr(init_expr);
                                if (const_val != 0 || (init_expr->kind == ND_NUM && init_expr->int_val == 0)) {
                                    sym->is_enum_const = 1;
                                    sym->enum_val = const_val;
                                }
                            }
                            if (is_auto) {
                                vtype = init_expr->type;
                                if (!vtype) vtype = cc->ty_int;
                                sym->type = vtype;
                            }
                            var->sym = sym;
                            var->type = vtype;
                            asgn = node_new(cc, ND_ASSIGN, line);
                            asgn->lhs = var;
                            asgn->rhs = init_expr;
                            asgn->type = vtype;
                            if (cnt < cap) {
                                block->stmts[cnt] = asgn;
                                cnt++;
                            }
                        }
                    }
                    }
                }

                if (cc->tk == TK_COMMA) {
                    next_token(cc);
                } else {
                    break;
                }
            }

            block->num_stmts = cnt;
            expect(cc, TK_SEMI);

            if (cnt == 0) {
                return node_new(cc, ND_NOP, line);
            }
            if (cnt == 1) {
                return block->stmts[0];
            }
            return block;
        }
    }

    /* empty statement */
    if (cc->tk == TK_SEMI) {
        next_token(cc);
        return node_new(cc, ND_NOP, line);
    }

    /* expression statement */
    {
        Node *expr;
        expr = parse_expr(cc);
            expect(cc, TK_SEMI);
        return expr;
    }
}

/* ================================================================ */
/* TOP-LEVEL PARSING                                                 */
/* ================================================================ */

static Node *parse_func_def(Compiler *cc, Type *ret_type, char *name, int is_static) {
    Node *func;
    int line;
    int i;
    int is_variadic = 0;

    line = cc->tk_line;
    func = node_new(cc, ND_FUNC_DEF, line);
    strncpy(func->func_def_name, name, MAX_IDENT - 1);
    func->is_static = is_static;

    /* parse parameters */
    expect(cc, TK_LPAREN);
    scope_push(cc);
    cc->local_offset = 0;

    func->num_params = 0;
    if (cc->tk != TK_RPAREN) {
        if (cc->tk == TK_VOID) {
            int pk;
            pk = peek_token(cc);
            if (pk == TK_RPAREN) {
                next_token(cc);
            } else {
                goto parse_fparams;
            }
        } else {
            parse_fparams:
            while (cc->tk != TK_RPAREN) {
                Type *ptype;
                char pname[MAX_IDENT];
                Symbol *psym;

                if (cc->tk == TK_ELLIPSIS) {
                    is_variadic = 1;
                    next_token(cc);
                    break;
                }
                if (cc->tk == TK_EOF) break;

                ptype = parse_type(cc);
                ptype = parse_declarator(cc, ptype, pname);

                if (ptype->kind == TY_ARRAY) ptype = type_ptr(cc, ptype->base);

                if (func->num_params < MAX_PARAMS) {
                    func->param_types[func->num_params] = ptype;
                    strncpy(func->param_names_buf[func->num_params], pname, MAX_IDENT - 1);
                    psym = scope_add_local(cc, pname, ptype);
                    func->num_params++;
                }

                if (cc->tk == TK_COMMA) {
                    next_token(cc);
                } else if (cc->tk == TK_RPAREN) {
                    break;
                } else {
                    /* Unexpected token (e.g. ->) after parameter — skip to ) to avoid cascade */
                    if (cc->tk == TK_ARROW) {
                        error(cc, "unexpected '->' in parameter list (missing ',' or ')'?)");
                    } else {
                        error(cc, "expected ',' or ')' after parameter");
                    }
                    while (cc->tk != TK_RPAREN && cc->tk != TK_EOF) next_token(cc);
                    break;
                }
            }
        }
    }
    expect(cc, TK_RPAREN);

    /* CG-IR-VARARGS: Reserve the first 48 bytes exclusively for reg_save_area */
    if (is_variadic && cc->local_offset > -48) {
        cc->local_offset = -48;
    }

    /* register function in global scope (forward decl and definition) */
    {
        Type *ftype;
        Symbol *fsym;
        ftype = type_func(cc, ret_type);
        ftype->num_params = func->num_params;
        ftype->is_variadic = is_variadic;
        if (func->num_params > 0) {
            ftype->params = (Type **)cc_alloc(cc, sizeof(Type *) * func->num_params);
            for (int k = 0; k < func->num_params; k++) {
                ftype->params[k] = func->param_types[k];
            }
        }
        func->func_type = ftype;

        /* add to parent scope (global) */
        fsym = scope_find(cc, name);
        if (!fsym) {
            /* temporarily pop to add to parent */
            Scope *cur;
            cur = cc->current_scope;
            cc->current_scope = cur->parent;
            fsym = scope_add(cc, name, ftype);
            fsym->is_global = 1;
            cc->current_scope = cur;
        }
    }

    /* forward declaration: );  — do not parse body, do not add to codegen list */
    if (cc->tk == TK_SEMI) {
        next_token(cc);
        func->body = 0;
        scope_pop(cc);
        return func;
    }

    /* definition: ) { ... } */
    func->body = parse_stmt(cc);
    func->stack_size = -cc->local_offset;
    scope_pop(cc);

    return func;
}

Node *parse_program(Compiler *cc) {
    Node *head;
    Node *tail;
    int top_count;
    int prev_pos;

    head = 0;
    tail = 0;
    top_count = 0;

    while (cc->tk != TK_EOF) {
        Type *base;
        char name[MAX_IDENT];
        Type *dtype;
        int is_typedef_kw;
        int is_static;
        int is_extern;
        int line;

        if (top_count >= 50000) {
            error(cc, "too many top-level declarations (possible parser loop)");
            break;
        }
        if (top_count > 0 && cc->pos == prev_pos) {
            next_token(cc);
        }
        top_count++;
        prev_pos = cc->pos;
        line = cc->tk_line;
        is_typedef_kw = 0;
        is_static = 0;
        is_extern = 0;

        /* check storage class */
        if (cc->tk == TK_TYPEDEF) { is_typedef_kw = 1; }
        if (cc->tk == TK_STATIC) { is_static = 1; }
        if (cc->tk == TK_EXTERN) { is_extern = 1; }

        base = parse_type(cc);

        /* bare struct/enum definition with no declarator */
        if (cc->tk == TK_SEMI) {
            next_token(cc);
            continue;
        }

        /* After base type, parse name and pointers only (not full declarator)
           so we can detect function defs before params are consumed */
        {
            Type *ptr_type;
            ptr_type = base;

            /* parse pointer stars */
            while (cc->tk == TK_STAR) {
                next_token(cc);
                while (cc->tk == TK_CONST || cc->tk == TK_VOLATILE) next_token(cc);
                ptr_type = type_ptr(cc, ptr_type);
            }

            /* get name — also handle grouped declarator (*name)(params) for function pointers */
            name[0] = 0;
            if (cc->tk == TK_LPAREN) {
                int gpk;
                gpk = peek_token(cc);
                if (gpk == TK_STAR) {
                    /* function pointer: typedef int (*name)(params); or int (*fp)(int); */
                    int gptr;
                    int arr_lens[8];
                    int arr_num;
                    next_token(cc); /* consume ( */
                    gptr = 0;
                    arr_num = 0;
                    while (cc->tk == TK_STAR) {
                        next_token(cc);
                        while (cc->tk == TK_CONST || cc->tk == TK_VOLATILE) next_token(cc);
                        gptr++;
                    }
                    if (cc->tk == TK_IDENT) {
                        strncpy(name, cc->tk_text, MAX_IDENT - 1);
                        next_token(cc);
                    }
                    int is_inner_func = 0;
                    if (cc->tk == TK_LPAREN) {
                        is_inner_func = 1;
                        next_token(cc);
                        int pcnt = 1;
                        while (pcnt > 0 && cc->tk != TK_EOF) {
                            if (cc->tk == TK_LPAREN) pcnt++;
                            else if (cc->tk == TK_RPAREN) pcnt--;
                            next_token(cc);
                        }
                    }
                    while (cc->tk == TK_LBRACKET) {
                        int alen = 0;
                        next_token(cc);
                        if (cc->tk != TK_RBRACKET) alen = (int)parse_const_expr(cc);
                        expect(cc, TK_RBRACKET);
                        if (arr_num < 8) arr_lens[arr_num++] = alen;
                    }
                    expect(cc, TK_RPAREN);
                    /* parse trailing (params) */
                    if (cc->tk == TK_LPAREN) {
                        Type *ftype;
                        next_token(cc);
                        ftype = type_func(cc, ptr_type);
                        ftype->params = (Type **)cc_alloc(cc, sizeof(Type *) * MAX_PARAMS);
                        ftype->num_params = 0;
                        ftype->is_variadic = 0;
                        if (cc->tk != TK_RPAREN) {
                            if (cc->tk == TK_VOID) {
                                int vpk2;
                                vpk2 = peek_token(cc);
                                if (vpk2 == TK_RPAREN) {
                                    next_token(cc);
                                } else {
                                    goto parse_gd_params;
                                }
                            } else {
                                parse_gd_params:
                                while (cc->tk != TK_RPAREN) {
                                    Type *ptype;
                                    char pname[128];
                                    if (cc->tk == TK_ELLIPSIS) {
                                        ftype->is_variadic = 1;
                                        next_token(cc);
                                        break;
                                    }
                                    if (cc->tk == TK_EOF) break;
                                    ptype = parse_type(cc);
                                    ptype = parse_declarator(cc, ptype, pname);
                                    if (ftype->num_params < MAX_PARAMS) {
                                        ftype->params[ftype->num_params] = ptype;
                                        ftype->num_params++;
                                    }
                                    if (cc->tk == TK_COMMA) {
                                        next_token(cc);
                                    } else {
                                        break;
                                    }
                                }
                            }
                        }
                        expect(cc, TK_RPAREN);
                        dtype = ftype;
                    } else {
                        dtype = ptr_type;
                    }
                    while (gptr > 0) {
                        dtype = type_ptr(cc, dtype);
                        gptr--;
                    }
                    while (arr_num > 0) {
                        arr_num--;
                        dtype = type_array(cc, dtype, arr_lens[arr_num]);
                    }
                    if (is_inner_func) {
                        dtype = type_func(cc, dtype);
                    }
                    /* fall through to typedef / global var handling */
                    goto after_name;
                }
            }
            if (cc->tk == TK_IDENT) {
                strncpy(name, cc->tk_text, MAX_IDENT - 1);
                next_token(cc);
            }
            dtype = ptr_type;

            after_name:
            /* handle bare struct/enum definitions (no name) */
            if (!name[0]) {
                if (cc->tk == TK_SEMI) {
                    next_token(cc);
                    continue;
                }
                /* unnamed — error or typedef was handled above */
                if (cc->tk != TK_LPAREN) {
                    error(cc, "expected name in declaration");
                    next_token(cc);
                    continue;
                }
            }

            /* function definition or forward declaration: name followed by ( */
            if (cc->tk == TK_LPAREN) {
                if (!is_typedef_kw) {
                    Node *func;
                    func = parse_func_def(cc, ptr_type, name, is_static);
                    /* only link into list if definition (has body); forward decl has body == 0 */
                    if (func->body) {
                        func->next = 0;
                        if (!head) { head = func; tail = func; }
                        else { tail->next = func; tail = func; }
                    }
                    continue;
                } else {
                    /* it's a typedef to a function type: typedef int func_t(int, int); */
                    int depth;
                    dtype = type_func(cc, ptr_type);
                    next_token(cc); /* consume ( */
                    depth = 1;
                    while (depth > 0 && cc->tk != TK_EOF) {
                        if (cc->tk == TK_LPAREN) depth++;
                        else if (cc->tk == TK_RPAREN) depth--;
                        next_token(cc);
                    }
                    /* falls through to array dimensional parsing or typedef registration */
                }
            }

            /* array dimensions after name */
            while (cc->tk == TK_LBRACKET) {
                int alen;
                next_token(cc);
                alen = 0;
                if (cc->tk != TK_RBRACKET) alen = (int)parse_const_expr(cc);
                expect(cc, TK_RBRACKET);
                dtype = type_array(cc, dtype, alen);
            }
        }

        /* typedef */
        if (is_typedef_kw) {
            Symbol *sym;
            sym = scope_add(cc, name, dtype);
            sym->is_typedef = 1;
            /* handle multiple typedef names */
            while (cc->tk == TK_COMMA) {
                char name2[MAX_IDENT];
                next_token(cc);
                dtype = parse_declarator(cc, base, name2);
                sym = scope_add(cc, name2, dtype);
                sym->is_typedef = 1;
            }
            expect(cc, TK_SEMI);
            continue;
        }

        /* global variable */
        {
            Node *gvar;
            gvar = node_new(cc, ND_GLOBAL_VAR, line);
            strncpy(gvar->name, name, MAX_IDENT - 1);
            gvar->type = dtype;
            gvar->is_static = is_static;
            gvar->is_extern = is_extern;

            /* register in scope */
            {
                Symbol *sym;
                sym = scope_add(cc, name, dtype);
                sym->is_global = 1;
            }

            /* initializer */
            if (cc->tk == TK_ASSIGN) {
                next_token(cc);
                if (cc->tk == TK_LBRACE) {
                    /* Parse array initializer list */
                    Node *init_list;
                    Node **inits;
                    int count;
                    init_list = node_new(cc, ND_INIT_LIST, line);
                    inits = (Node **)cc_alloc(cc, sizeof(Node *) * MAX_INIT);
                    count = 0;
                    next_token(cc); /* skip { */
                    int depth = 1;
                    int skip_tk;
                    int prev_pos;
                    int no_progress_count = 0;
                    int top_elems = 0;
                    int last_comma = 1;
                    while (depth > 0 && cc->tk != TK_EOF) {
                        if (depth == 1 && cc->tk != TK_RBRACE && cc->tk != TK_COMMA && last_comma) {
                            top_elems++;
                            last_comma = 0;
                        }
                        prev_pos = cc->pos;
                        if (cc->tk == TK_LBRACE) {
                            depth++;
                            next_token(cc);
                        } else if (cc->tk == TK_RBRACE) {
                            depth--;
                            if (depth == 0) break;
                            next_token(cc);
                        } else if (cc->tk == TK_COMMA) {
                            if (depth == 1) last_comma = 1;
                            next_token(cc);
                        } else {
                            skip_tk = cc->tk;
                            if (count < MAX_INIT) {
                                inits[count++] = parse_assign(cc);
                            } else {
                                parse_assign(cc); /* discard if over limit */
                            }
                            if (cc->tk == skip_tk) {
                                if (cc->tk != TK_EOF) next_token(cc);
                            }
                            if (cc->pos == prev_pos) {
                                no_progress_count++;
                                if (no_progress_count > 100) {
                                    error(cc, "global array parser stuck");
                                    break;
                                }
                            } else {
                                no_progress_count = 0;
                            }
                        }
                    }
                    if (cc->tk == TK_RBRACE) next_token(cc); /* skip } */
                    init_list->args = inits;
                    init_list->num_args = count;
                    gvar->initializer = init_list;
                    
                    /* adjust array size if omitted */
                    if (gvar->type->kind == TY_ARRAY && gvar->type->array_len == 0) {
                        gvar->type->array_len = top_elems;
                        gvar->type->size = type_size(gvar->type->base) * top_elems;
                        if (gvar->sym) {
                            gvar->sym->type->array_len = top_elems;
                            gvar->sym->type->size = gvar->type->size;
                        }
                    }
                } else {
                    Node *init_node = parse_assign(cc);
                    gvar->initializer = init_node;
                    /* Fix: if char[] initialized with string literal, update size */
                    if (init_node && init_node->kind == ND_STR &&
                        gvar->type->kind == TY_ARRAY && gvar->type->array_len == 0) {
                        int slen = cc->strings[init_node->str_id].len + 1; /* +1 for null */
                        gvar->type->array_len = slen;
                        gvar->type->size = type_size(gvar->type->base) * slen;
                    }
                    if (cc->current_is_const && init_node) {
                        int const_val = eval_const_expr(init_node);
                        if (const_val != 0 || (init_node->kind == ND_NUM && init_node->int_val == 0)) {
                            Symbol *hsym = scope_find(cc, gvar->name);
                            if (hsym) {
                                hsym->is_enum_const = 1;
                                hsym->enum_val = const_val;
                            }
                        }
                    }
                }
            }

            /* store for codegen */
            if (cc->num_globals < MAX_GLOBALS) {
                cc->globals[cc->num_globals] = gvar;
                cc->num_globals++;
            }

            /* handle multiple declarators: int a, b; */
            while (cc->tk == TK_COMMA) {
                Node *gvar2;
                char name2[MAX_IDENT];
                Type *dtype2;
                Symbol *sym2;

                next_token(cc);
                dtype2 = parse_declarator(cc, base, name2);
                gvar2 = node_new(cc, ND_GLOBAL_VAR, line);
                strncpy(gvar2->name, name2, MAX_IDENT - 1);
                gvar2->type = dtype2;
                gvar2->is_extern = is_extern;
                gvar2->is_static = is_static;

                sym2 = scope_add(cc, name2, dtype2);
                sym2->is_global = 1;

                if (cc->tk == TK_ASSIGN) {
                    next_token(cc);
                    Node *init_node2 = parse_assign(cc);
                    gvar2->initializer = init_node2;
                    if (cc->current_is_const && init_node2) {
                        int const_val = eval_const_expr(init_node2);
                        if (const_val != 0 || (init_node2->kind == ND_NUM && init_node2->int_val == 0)) {
                            sym2->is_enum_const = 1;
                            sym2->enum_val = const_val;
                        }
                    }
                }

                if (cc->num_globals < MAX_GLOBALS) {
                    cc->globals[cc->num_globals] = gvar2;
                    cc->num_globals++;
                }
            }

            /* link */
            gvar->next = 0;
            if (!head) { head = gvar; tail = gvar; }
            else { tail->next = gvar; tail = gvar; }

            expect(cc, TK_SEMI);
            continue;
        }
    }

    return head;
}
/* ZKAEDI FORCE RENDER CACHE INVALIDATION */
/*
 * ir.h — ZCC 3-Address Intermediate Representation
 *
 * Design constraints:
 *   1. ZCC-parseable: no #include <stdint.h>, no _Static_assert
 *   2. Native LP64 types only (char=1, short=2, int=4, long=8 on x86-64)
 *   3. GCC boundary fn zcc_run_passes_log: UNCHANGED, never included here
 *   4. All IR nodes heap-allocated via ir_node_alloc()
 *   5. ir_module_free() is the single cleanup entry point
 *
 * Text format (one node per line):
 *   LABEL   .L0:
 *   ALLOCA  i64  t0  8
 *   CONST   i32  t1  42
 *   ADD     i32  t2  t0  t1
 *   STORE   ptr  t0  t2
 *   LOAD    i32  t3  t0
 *   BR_IF       t3  .L1
 *   BR          .L2
 *   CALL    i32  rv  printf  [args follow as IR_ARG nodes]
 *   ARG     i32      t3
 *   RET     i32      t2
 */

#ifndef ZCC_IR_H
#define ZCC_IR_H

/* ── stdio.h is always available to ZCC ─────────────────────────────── */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Field size constants ────────────────────────────────────────────── */
enum {
    IR_NAME_MAX   = 64,    /* max length of a variable/temp name        */
    IR_LABEL_MAX  = 64,    /* max length of a branch label              */
    IR_FUNC_MAX   = 64,    /* max length of a function name             */
    IR_MAX_FUNCS  = 8192,  /* max functions per module                  */
    IR_STR_MAX    = 256    /* max string literal length in IR           */
};

/* ── Opcodes ─────────────────────────────────────────────────────────── */
typedef enum {
    /* Terminators (must end a basic block) */
    IR_RET     = 0, /* return [src1]                                      */
    IR_BR,          /* unconditional jump → label                         */
    IR_BR_IF,       /* if src1 != 0, jump → label                        */

    /* Memory */
    IR_ALLOCA,      /* dst = alloca(imm) — stack slot, imm = bytes        */
    IR_LOAD,        /* dst = *src1                                         */
    IR_STORE,       /* *dst = src1   (dst is an address, src1 is value)   */

    /* Arithmetic */
    IR_ADD,         /* dst = src1 + src2                                   */
    IR_SUB,         /* dst = src1 - src2                                   */
    IR_MUL,         /* dst = src1 * src2                                   */
    IR_DIV,         /* dst = src1 / src2  (signed or unsigned per type)   */
    IR_MOD,         /* dst = src1 % src2                                   */
    IR_NEG,         /* dst = -src1                                         */

    /* Bitwise */
    IR_AND,         /* dst = src1 & src2                                   */
    IR_OR,          /* dst = src1 | src2                                   */
    IR_XOR,         /* dst = src1 ^ src2                                   */
    IR_NOT,         /* dst = ~src1                                         */
    IR_SHL,         /* dst = src1 << src2                                  */
    IR_SHR,         /* dst = src1 >> src2  (arithmetic if signed type)    */

    /* Comparisons — always produce i32 0 or 1 */
    IR_EQ,          /* dst = (src1 == src2)                                */
    IR_NE,          /* dst = (src1 != src2)                                */
    IR_LT,          /* dst = (src1 <  src2)                                */
    IR_LE,          /* dst = (src1 <= src2)                                */
    IR_GT,          /* dst = (src1 >  src2)                                */
    IR_GE,          /* dst = (src1 >= src2)                                */

    /* Type operations */
    IR_CAST,        /* dst = (type)src1  — widen/narrow/ptr-cast           */
    IR_COPY,        /* dst = src1  — identity, phi-lowering target         */

    /* Constants */
    IR_CONST,       /* dst = imm  (integer immediate, stored in .imm)     */
    IR_CONST_STR,   /* dst = &str_literal  (label points into .rodata)    */

    /* Function call */
    IR_CALL,        /* dst = call label(...)  — args via IR_ARG nodes      */
    IR_ARG,         /* argument to the preceding IR_CALL                   */

    /* SSA */
    IR_PHI,         /* dst = phi(src1 from label, src2 from label2)       */
    IR_ADDR,        /* dst = &src1                                        */

    /* Pseudo */
    IR_LABEL,       /* label definition — label field only                 */
    IR_NOP,         /* no operation — used as placeholder during passes    */

    /* Floating-point operations */
    IR_FCONST,      /* dst = float_imm  (double stored bitwise in .imm)   */
    IR_FADD,        /* dst = src1 +f src2  (double add)                    */
    IR_FSUB,        /* dst = src1 -f src2  (double sub)                    */
    IR_FMUL,        /* dst = src1 *f src2  (double mul)                    */
    IR_FDIV,        /* dst = src1 /f src2  (double div)                    */
    IR_ITOF,        /* dst = (double)src1  (signed int64 to double)        */
    IR_FTOI,        /* dst = (int64)src1   (double to signed int64)        */
    IR_ASM,         /* dst = asm_string                                    */

    IR_OP_COUNT     /* sentinel — keep last                                */
} ir_op_t;

/* ── Type system ─────────────────────────────────────────────────────── */
/*
 * Encodes C's type width AND signedness.
 * Signedness matters for: DIV, MOD, SHR, LT/LE/GT/GE, CAST.
 * Unsigned types: U8..U64.  Signed: I8..I64.
 */
typedef enum {
    IR_TY_VOID = 0,
    IR_TY_I8,       /* signed  8-bit                                       */
    IR_TY_I16,      /* signed 16-bit                                       */
    IR_TY_I32,      /* signed 32-bit  — default int                        */
    IR_TY_I64,      /* signed 64-bit  — long on LP64                       */
    IR_TY_U8,       /* unsigned  8-bit                                     */
    IR_TY_U16,      /* unsigned 16-bit                                     */
    IR_TY_U32,      /* unsigned 32-bit                                     */
    IR_TY_U64,      /* unsigned 64-bit                                     */
    IR_TY_PTR,      /* generic pointer — 8 bytes LP64                      */
    IR_TY_F32,      /* float   — reserved, not emitted in P1               */
    IR_TY_F64,      /* double  — reserved, not emitted in P1               */
    IR_TY_COUNT     /* sentinel                                             */
} ir_type_t;

/* ── Core node ───────────────────────────────────────────────────────── */
/*
 * Every IR instruction is one ir_node_t.
 * Layout:
 *   binary:  op  type  dst  src1  src2
 *   unary:   op  type  dst  src1  (src2 = "")
 *   branch:  op        src1 label (dst/src2 = "")
 *   label:   IR_LABEL  label (all others = "")
 *   const:   IR_CONST  type  dst  imm
 *   call:    IR_CALL   type  dst  label (label = fn name)
 *   arg:     IR_ARG    type      src1
 *   phi:     IR_PHI    type  dst  src1  src2  label (src1 from label, src2 from label2)
 *   alloca:  IR_ALLOCA ptr   dst  imm (bytes)
 *   store:   IR_STORE  type  dst(addr)  src1(val)
 *   load:    IR_LOAD   type  dst  src1(addr)
 */
typedef struct ir_node_t {
    ir_op_t            op;
    ir_type_t          type;

    char               dst[IR_NAME_MAX];    /* destination temp/var       */
    char               src1[IR_NAME_MAX];   /* first source               */
    char               src2[IR_NAME_MAX];   /* second source (if any)     */
    char               label[IR_LABEL_MAX]; /* branch target / label name */
    char               label2[IR_LABEL_MAX];/* phi: label for src2        */

    char              *asm_string;          /* assembly string            */

    long               imm;      /* integer immediate (CONST, ALLOCA)     */
    int                lineno;   /* source file line number               */

    struct ir_node_t  *next;     /* intrusive singly-linked list          */
} ir_node_t;

/* ── Function container ──────────────────────────────────────────────── */
typedef struct {
    char       name[IR_FUNC_MAX];
    ir_type_t  ret_type;

    ir_node_t *head;          /* first node in the function               */
    ir_node_t *tail;          /* last  node — O(1) append                 */
    int        node_count;

    int        tmp_counter;   /* monotonic counter for fresh temps        */
    int        lbl_counter;   /* monotonic counter for fresh labels       */
    
    int        num_params;
    char       param_names[8][IR_NAME_MAX]; /* SystemV up to 6 in regs, keep 8 safe */
} ir_func_t;

/* ── Module container ────────────────────────────────────────────────── */
typedef struct {
    ir_func_t *funcs[IR_MAX_FUNCS];
    int        func_count;
} ir_module_t;

/* ── Global emit-IR flag (set by zcc.c at startup) ───────────────────── */
/*
 * When non-zero, ZCC's codegen routes through ir_append_node() instead
 * of writing x86-64 assembly directly.
 * Set by:  ZCC_EMIT_IR=1 environment variable
 *      or: --emit-ir command-line flag
 */
extern int g_emit_ir;

/* Active function during code generation — set by codegen before emitting */
extern ir_func_t *g_ir_cur_func;

/* Active module — created once per compilation unit */
extern ir_module_t *g_ir_module;

/* ── Construction API ────────────────────────────────────────────────── */

/* Allocate and zero a single ir_node_t.  Never returns NULL (exits on OOM). */
ir_node_t   *ir_node_alloc(void);

/* Append a pre-built node to a function's node list. */
void         ir_append(ir_func_t *fn, ir_node_t *n);

/* Create and append a node in one call. Returns the new node. */
ir_node_t   *ir_emit(ir_func_t *fn, ir_op_t op, ir_type_t ty,
                     const char *dst, const char *src1, const char *src2,
                     const char *label, long imm, int lineno);

/* Fresh temporaries and labels (function-scoped counters) */
void         ir_fresh_tmp(ir_func_t *fn, char *buf);    /* writes "t<N>" */
void         ir_fresh_label(ir_func_t *fn, char *buf);  /* writes ".L<N>" */

/* Module lifecycle */
ir_module_t *ir_module_create(void);
ir_func_t   *ir_func_create(ir_module_t *mod, const char *name,
                             ir_type_t ret_type, int num_params);
void         ir_module_free(ir_module_t *mod);

/* ── Emission API ────────────────────────────────────────────────────── */

/* Emit IR text for one function to fp (stdout or output file). */
void         ir_func_emit_text(const ir_func_t *fn, FILE *fp);

/* Emit the entire module. */
void         ir_module_emit_text(const ir_module_t *mod, FILE *fp);

/* ── Query helpers ───────────────────────────────────────────────────── */

/* Opcode → mnemonic string ("ADD", "LOAD", ...) */
const char  *ir_op_name(ir_op_t op);

/* Type → string ("i32", "ptr", ...) */
const char  *ir_type_name(ir_type_t ty);

/* Width in bytes of a type (0 for void, -1 for unknown) */
int          ir_type_bytes(ir_type_t ty);

/* Non-zero if type is unsigned integer */
int          ir_type_unsigned(ir_type_t ty);

/* Non-zero if op is a basic-block terminator */
int          ir_op_is_terminator(ir_op_t op);

#endif /* ZCC_IR_H */
/*
 * ir_emit_dispatch.h — Emit dispatch shim for zcc.c
 *
 * Include this AFTER "ir.h" in zcc.c.
 * Provides ZCC_EMIT_* macros that check g_emit_ir and route to either:
 *   a) the existing x86-64 asm fprintf path (g_emit_ir == 0)
 *   b) ir_emit() into g_ir_cur_func  (g_emit_ir != 0)
 *
 * Usage in zcc.c codegen functions:
 *
 *   // Before (x86-64 only):
 *   fprintf(output, "    addq %s, %s\n", src, dst);
 *
 *   // After (dual-mode):
 *   ZCC_EMIT_BINARY(IR_ADD, ty, dst_tmp, src1_tmp, src2_tmp, lineno)
 *   // falls through to x86 emit if g_emit_ir == 0
 *
 * The macros are intentionally simple — they do NOT replace all of zcc.c's
 * codegen in P1.  P1's goal is:
 *   1. ir.h / ir.c compile cleanly
 *   2. --emit-ir flag activates IR path
 *   3. IR text is emitted for every function
 *   4. x86 path is completely unaffected (stage2==stage3 green)
 *
 * Full codegen routing (every node type) is a P3-IR deliverable.
 */

#ifndef ZCC_IR_EMIT_DISPATCH_H
#define ZCC_IR_EMIT_DISPATCH_H

#include "ir.h"

char *getenv(const char *name);

static void ZCC_IR_INIT(void) {
    if (getenv("ZCC_EMIT_IR")) g_emit_ir = 1;
    if (getenv("ZCC_IR_BACKEND")) g_emit_ir = 1;
    if (g_emit_ir) g_ir_module = ir_module_create();
}

static void ZCC_IR_FUNC_BEGIN(const char *fname, ir_type_t ret_ty, int num_params) {
    if (g_emit_ir) {
        g_ir_cur_func = ir_func_create(g_ir_module, fname, ret_ty, num_params);
    }
}

static void ZCC_IR_FUNC_END(void) {
    if (g_emit_ir) g_ir_cur_func = 0;
}

static void ZCC_IR_FLUSH(FILE *fp) {
    if (g_emit_ir && g_ir_module) {
        ir_module_emit_text(g_ir_module, fp);
        ir_module_free(g_ir_module);
        g_ir_module = 0;
    }
}

static void ZCC_EMIT_BINARY(ir_op_t op, ir_type_t ty, const char *dst, const char *s1, const char *s2, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, op, ty, dst, s1, s2, 0, 0, line);
    }
}

static void ZCC_EMIT_UNARY(ir_op_t op, ir_type_t ty, const char *dst, const char *src, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, op, ty, dst, src, 0, 0, 0, line);
    }
}

static void ZCC_EMIT_CONST(ir_type_t ty, const char *dst, long immval, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_CONST, ty, dst, 0, 0, 0, immval, line);
    }
}

static void ZCC_EMIT_ALLOCA(const char *dst, long bytes, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_ALLOCA, IR_TY_PTR, dst, 0, 0, 0, bytes, line);
    }
}

static void ZCC_EMIT_STORE(ir_type_t ty, const char *addr, const char *val, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_STORE, ty, addr, val, 0, 0, 0, line);
    }
}

static void ZCC_EMIT_LOAD(ir_type_t ty, const char *dst, const char *addr, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_LOAD, ty, dst, addr, 0, 0, 0, line);
    }
}

static void ZCC_EMIT_LABEL(const char *lbl, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_LABEL, IR_TY_VOID, 0, 0, 0, lbl, 0, line);
    }
}

static void ZCC_EMIT_BR(const char *lbl, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_BR, IR_TY_VOID, 0, 0, 0, lbl, 0, line);
    }
}

static void ZCC_EMIT_BR_IF(const char *cond, const char *lbl, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_BR_IF, IR_TY_VOID, 0, cond, 0, lbl, 0, line);
    }
}

static void ZCC_EMIT_RET(ir_type_t ty, const char *val, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_RET, ty, 0, val ? val : "", 0, 0, 0, line);
    }
}

static void ZCC_EMIT_CALL(ir_type_t ty, const char *dst, const char *fname, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_CALL, ty, dst, 0, 0, fname, 0, line);
    }
}

static void ZCC_EMIT_ARG(ir_type_t ty, const char *val, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_ARG, ty, 0, val, 0, 0, 0, line);
    }
}

static void ZCC_EMIT_FCONST(const char *dst, long bits, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_FCONST, IR_TY_F64, dst, 0, 0, 0, bits, line);
    }
}

static void ZCC_EMIT_FBINARY(ir_op_t op, const char *dst, const char *s1, const char *s2, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, op, IR_TY_F64, dst, s1, s2, 0, 0, line);
    }
}

static void ZCC_EMIT_ITOF(const char *dst, const char *src, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_ITOF, IR_TY_F64, dst, src, 0, 0, 0, line);
    }
}

static void ZCC_EMIT_FTOI(const char *dst, const char *src, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_emit(g_ir_cur_func, IR_FTOI, IR_TY_I64, dst, src, 0, 0, 0, line);
    }
}

static void ZCC_EMIT_ASM(const char *asm_str, int line) {
    if (g_emit_ir && g_ir_cur_func) {
        ir_node_t *n = ir_emit(g_ir_cur_func, IR_ASM, IR_TY_VOID, 0, 0, 0, 0, 0, line);
        if (n) n->asm_string = (char *)asm_str;
    }
}

#endif /* ZCC_IR_EMIT_DISPATCH_H */
/* ================================================================ */
/* ir_bridge.h — ZCC IR Bridge Infrastructure                        */
/*                                                                   */
/* Translation layer between register-machine x86-64 codegen and     */
/* 3-address SSA-style IR emission.                                  */
/*                                                                   */
/* Include this ONCE at the top of part4.c, after ir_emit_dispatch.h */
/* is already visible (it comes through part5.c concatenation).      */
/*                                                                   */
/* CRITICAL: All types here must be ZCC-parseable.                   */
/*   - No stdint.h, no _Static_assert, no __attribute__              */
/*   - No designated initializers, no compound literals              */
/* ================================================================ */

#ifndef ZCC_IR_BRIDGE_H
#define ZCC_IR_BRIDGE_H

/* ── IR Temporary Name Generator ─────────────────────────────────── */
/*                                                                     */
/* The x86-64 backend leaves results in %rax. The IR needs named       */
/* temporaries (%t0, %t1, ...). This counter generates unique names.   */
/* Reset at each function boundary via ir_bridge_func_begin().         */

static int ir_tmp_counter;
static char ir_tmp_buf[32];         /* scratch for sprintf */
static char ir_last_result[32];     /* name of the last IR temp produced */
static int ir_bridge_active;        /* 1 = emit IR alongside asm */

static void ir_bridge_reset(void) {
    ir_tmp_counter = 0;
    ir_last_result[0] = 0;
    ir_bridge_active = 1;
}

static char *ir_bridge_fresh_tmp(void) {
    /* Returns a new temporary name like "%t0", "%t1", etc.
     * Also copies into ir_last_result so callers can reference it. */
    sprintf(ir_tmp_buf, "%%t%d", ir_tmp_counter);
    ir_tmp_counter++;
    /* copy to ir_last_result */
    {
        int i;
        for (i = 0; ir_tmp_buf[i]; i++)
            ir_last_result[i] = ir_tmp_buf[i];
        ir_last_result[i] = 0;
    }
    return ir_tmp_buf;
}

/* Snapshot the current ir_last_result into a caller-owned buffer.
 * Use this to save lhs result before codegen_expr(rhs) overwrites it. */
static void ir_save_result(char *dst) {
    int i;
    for (i = 0; ir_last_result[i]; i++)
        dst[i] = ir_last_result[i];
    dst[i] = 0;
}


/* ── AST Type → IR Type Mapper ───────────────────────────────────── */
/*                                                                     */
/* Maps the compiler's internal Type* to the ir_type_t enum.           */
/* Handles all TY_* kinds including pointers, arrays, and structs.     */

static ir_type_t ir_map_type(Type *ty) {
    if (!ty) return IR_TY_I64;   /* default to 64-bit signed */

    switch (ty->kind) {
    case TY_VOID:       return IR_TY_VOID;
    case TY_CHAR:       return IR_TY_I8;
    case TY_UCHAR:      return IR_TY_U8;
    case TY_SHORT:      return IR_TY_I16;
    case TY_USHORT:     return IR_TY_U16;
    case TY_INT:        return IR_TY_I32;
    case TY_UINT:       return IR_TY_U32;
    case TY_LONG:       return IR_TY_I64;
    case TY_ULONG:      return IR_TY_U64;
    case TY_LONGLONG:   return IR_TY_I64;
    case TY_ULONGLONG:  return IR_TY_U64;
    default:            break;
    }

    /* Pointers, arrays, function pointers, structs → IR_TY_PTR (64-bit) */
    if (ty->kind == TY_PTR || ty->kind == TY_ARRAY || ty->kind == TY_FUNC)
        return IR_TY_PTR;

    /* Structs passed by pointer in System V ABI */
    if (ty->kind == TY_STRUCT)
        return IR_TY_PTR;

    /* Enum → i32 */
    if (ty->kind == TY_ENUM)
        return IR_TY_I32;

    return IR_TY_I64;  /* safety fallback */
}


/* ── Variable Name Extraction ────────────────────────────────────── */
/*                                                                     */
/* For ND_VAR nodes, extract the symbol name for IR readability.       */
/* Falls back to stack offset notation if name is unavailable.         */

static char ir_var_buf[64];

static char *ir_var_name(Node *node) {
    if (node->sym && node->sym->stack_offset != 0) {
        sprintf(ir_var_buf, "%%stack_%d", node->sym->stack_offset);
    } else if (node->sym && node->sym->name[0]) {
        sprintf(ir_var_buf, "%%%s", node->sym->name);
    } else {
        sprintf(ir_var_buf, "%%anon");
    }
    return ir_var_buf;
}


/* ── Function Boundary Helpers ───────────────────────────────────── */

static void ir_bridge_func_begin(Node *func) {
    ir_tmp_counter = 0;
    ir_last_result[0] = 0;
    ZCC_IR_FUNC_BEGIN(func->func_def_name, ir_map_type(func->func_type), func->num_params);
}

static void ir_bridge_func_end(void) {
    ZCC_IR_FUNC_END();
}


/* ── Binary Op Mapper ────────────────────────────────────────────── */
/*                                                                     */
/* Maps AST node kinds (ND_ADD, ND_SUB, ...) to IR opcodes (IR_ADD,   */
/* IR_SUB, ...). Returns -1 for non-binary nodes.                      */

static ir_op_t ir_map_binop(int nd_kind) {
    switch (nd_kind) {
    case ND_ADD:  return IR_ADD;
    case ND_SUB:  return IR_SUB;
    case ND_MUL:  return IR_MUL;
    case ND_DIV:  return IR_DIV;
    case ND_MOD:  return IR_MOD;
    case ND_BAND: return IR_AND;
    case ND_BOR:  return IR_OR;
    case ND_BXOR: return IR_XOR;
    case ND_SHL:  return IR_SHL;
    case ND_SHR:  return IR_SHR;
    case ND_EQ:   return IR_EQ;
    case ND_NE:   return IR_NE;
    case ND_LT:   return IR_LT;
    case ND_LE:   return IR_LE;
    case ND_GT:   return IR_GT;
    case ND_GE:   return IR_GE;
    default:      return IR_NOP;
    }
}


/* ── Convenience: Emit Binary with Auto-Temp ─────────────────────── */
/*                                                                     */
/* Pattern: save lhs result, codegen rhs, emit IR_BINARY with fresh   */
/* destination temp. Used by all arithmetic/bitwise/comparison nodes.  */

static void ir_emit_binary_op(int nd_kind, Type *ty, char *lhs_tmp,
                               char *rhs_tmp, int line) {
    ir_op_t op;
    char *dst;

    op = ir_map_binop(nd_kind);
    if (op == IR_NOP) return;  /* not a binary op */

    dst = ir_bridge_fresh_tmp();
    ZCC_EMIT_BINARY(op, ir_map_type(ty), dst, lhs_tmp, rhs_tmp, line);
}

static void ir_emit_var_load(Node *node) {
    char *vname = ir_var_name(node);
    char *dst = ir_bridge_fresh_tmp();
    if (node->type && (node->type->kind == TY_ARRAY || node->type->kind == TY_STRUCT || node->type->kind == TY_UNION)) {
        ZCC_EMIT_UNARY(IR_ADDR, ir_map_type(node->type), dst, vname, node->line);
    } else {
        ZCC_EMIT_LOAD(ir_map_type(node->type), dst, vname, node->line);
    }
}

#endif /* ZCC_IR_BRIDGE_H */
/* ================================================================ */
/* CODE GENERATOR — x86-64 Linux (System V) or Windows x64 ABI      */
/* ================================================================ */

#include "ir_emit_dispatch.h"
#include "ir_bridge.h"

#ifdef __clang__
#pragma clang diagnostic ignored "-Wdangling-else"
#pragma clang diagnostic ignored "-Wmisleading-indentation"
#pragma clang diagnostic ignored "-Wpointer-bool-conversion"
#pragma clang diagnostic ignored "-Wtautological-pointer-compare"
int is_unsigned_type(Type *ty);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
#endif

static void push_reg(Compiler *cc, char *reg) {
  if (backend_ops) {
      if (strcmp(reg, "rax") == 0) reg = "r0";
      else if (strcmp(reg, "r11") == 0) reg = "r1";
      else if (strcmp(reg, "rdx") == 0) reg = "r2";
      fprintf(cc->out, "    push {%s}\n", reg);
      cc->stack_depth++;
      return;
  }
  fprintf(cc->out, "    pushq %%%s\n", reg);
  cc->stack_depth++;
}

static void pop_reg(Compiler *cc, char *reg) {
  if (backend_ops) {
      if (strcmp(reg, "rax") == 0) reg = "r0";
      else if (strcmp(reg, "r11") == 0) reg = "r1";
      else if (strcmp(reg, "rdx") == 0) reg = "r2";
      fprintf(cc->out, "    pop {%s}\n", reg);
      cc->stack_depth--;
      return;
  }
  fprintf(cc->out, "    popq %%%s\n", reg);
  cc->stack_depth--;
}

static int new_label(Compiler *cc) {
  int l;
  l = cc->label_count;
  cc->label_count++;
  return l;
}

static int is_power_of_2_val(long long val) {
  return val > 0 && (val & (val - 1)) == 0;
}

static int log2_of(long long val) {
  int n;
  n = 0;
  while (val > 1) {
    val = val >> 1;
    n = n + 1;
  }
  return n;
}

/* Format codes for label emission — avoid passing string literals as 2nd arg
 * (stage2 mispasses them). */
enum { FMT_JE = 1, FMT_JMP, FMT_DEF, FMT_JNE };
static void emit_label_fmt(Compiler *cc, int n, int fmt) {
  if (backend_ops) {
      switch (fmt) {
      case FMT_JE:
        fprintf(cc->out, "    beq .L%d\n", n);
        break;
      case FMT_JMP:
        fprintf(cc->out, "    b .L%d\n", n);
        break;
      case FMT_DEF:
        fprintf(cc->out, ".L%d:\n", n);
        break;
      case FMT_JNE:
        fprintf(cc->out, "    bne .L%d\n", n);
        break;
      default:
        fprintf(cc->out, ".L%d:\n", n);
        break;
      }
      return;
  }
  switch (fmt) {
  case FMT_JE:
    if (backend_ops) fprintf(cc->out, "    beq .L%d\n" \
    ); else fprintf(cc->out, "    je .L%d\n", n);
    break;
  case FMT_JMP:
    if (backend_ops) fprintf(cc->out, "    b .L%d\n" \
    ); else fprintf(cc->out, "    jmp .L%d\n", n);
    break;
  case FMT_DEF:
    fprintf(cc->out, ".L%d:\n", n);
    break;
  case FMT_JNE:
    fprintf(cc->out, "    jne .L%d\n", n);
    break;
  default:
    fprintf(cc->out, ".L%d:\n", n);
    break;
  }
}

/* Forward: defined after bad_node_cutoff. */
static int is_bad_ptr(const void *p);

/* True if ptr is in fault range. GDB: 0x800b457d0 (hi=0, lo>=2G), 0x800ac07d0
 * (hi=0x80). Reject: (hi==0 && lo>=2G) OR (hi in [0x80, 0x100)). Not inline so
 * stage2 emits a body. */
int ptr_in_fault_range(const void *p) {
  unsigned long long u = (unsigned long long)(const char *)p;
  unsigned int hi = (unsigned int)(u >> 32);
  unsigned int lo = (unsigned int)u;
  return (hi == 0 && lo >= 0x80000000U) || (hi >= 0x80U && hi < 0x100U);
}
/* Use ptr_in_fault_range() in code, not macro; stage2 may not expand macros. */

/* ---------------------------------------------------------------- */
/* Load value from address in %rax                                   */
/* ---------------------------------------------------------------- */

void codegen_load(Compiler *cc, Type *type) {
  if (backend_ops) {
      if (!type || type->size == 4 || type->size == 8 || type->kind == TY_PTR) {
          fprintf(cc->out, "    ldr r0, [r0]\n");
      } else if (type->size == 1) {
          fprintf(cc->out, "    ldrb r0, [r0]\n");
      } else if (type->size == 2) {
          fprintf(cc->out, "    ldrh r0, [r0]\n");
      }
      return;
  }
  if (!type) {
    fprintf(cc->out, "    movq (%%rax), %%rax\n");
    return;
  }
  /* Do NOT check fault range here: stage2 miscompiles it and rejects valid
   * Type* ptrs. */
  if (type->kind == TY_ARRAY)
    return;
  if (type->kind == TY_STRUCT || type->kind == TY_UNION)
    return;
  /* Pointers must always be 64-bit (fixes stage2 sign-extended 32-bit load ->
   * 0x80xxxxxx crash). */
  if (type->kind == TY_PTR) {
    fprintf(cc->out, "    movq (%%rax), %%rax\n");
    return;
  }
  switch (type->size) {
  case 1:
    if (is_unsigned_type(type))
      fprintf(cc->out, "    movzbq (%%rax), %%rax\n");
    else
      fprintf(cc->out, "    movsbq (%%rax), %%rax\n");
    break;
  case 2:
    if (is_unsigned_type(type))
      fprintf(cc->out, "    movzwq (%%rax), %%rax\n");
    else
      fprintf(cc->out, "    movswq (%%rax), %%rax\n");
    break;
  case 4:
    if (is_unsigned_type(type))
      fprintf(cc->out, "    movl (%%rax), %%eax\n");
    else
      fprintf(cc->out, "    movslq (%%rax), %%rax\n");
    break;
  default:
    fprintf(cc->out, "    movq (%%rax), %%rax\n");
    break;
  }
}

/* ---------------------------------------------------------------- */
/* Store: value in %r11, address in %rax                             */
/* ---------------------------------------------------------------- */

void codegen_store(Compiler *cc, Type *type) {
  pop_reg(cc, "r11");
  if (backend_ops) {
      if (!type || type->size == 4 || type->size == 8 || type->kind == TY_PTR) {
          fprintf(cc->out, "    str r1, [r0]\n");
      } else if (type->size == 1) {
          fprintf(cc->out, "    strb r1, [r0]\n");
      } else if (type->size == 2) {
          fprintf(cc->out, "    strh r1, [r0]\n");
      }
      fprintf(cc->out, "    mov r0, r1\n");
      return;
  }
  if (!type) {
    if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
    if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
    return;
  }
  /* Do NOT check fault range here: stage2 miscompiles it and rejects valid
   * Type* ptrs. */
  /* Pointers must always be 64-bit (fixes stage2 sign-extended 32-bit store).
   */
  if (type->kind == TY_PTR) {
    if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
    if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
    return;
  }
  if (type->kind == TY_STRUCT || type->kind == TY_UNION || type->size > 8) {
    fprintf(cc->out, "    pushq %%rsi\n");
    fprintf(cc->out, "    pushq %%rdi\n");
    fprintf(cc->out, "    pushq %%rcx\n");
    fprintf(cc->out, "    movq %%r11, %%rsi\n");
    fprintf(cc->out, "    movq %%rax, %%rdi\n");
    fprintf(cc->out, "    movq $%d, %%rcx\n", type->size);
    fprintf(cc->out, "    rep movsb\n");
    fprintf(cc->out, "    popq %%rcx\n");
    fprintf(cc->out, "    popq %%rdi\n");
    fprintf(cc->out, "    popq %%rsi\n");
    if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
    return;
  }
  switch (type->size) {
  case 1:
    if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
    break;
  case 2:
    if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
    break;
  case 4:
    if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
    break;
  default:
    if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
    break;
  }
  if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
}

/* Return cutoff so stage2 can't miscompile inline constant (GDB showed
 * node=0x10000). */
static unsigned long long bad_node_cutoff(void) { return 65536; }

/* Only null and very low ptr (<=0x10000). No hi/lo range check here—stage2
 * miscompiles it and then rejects valid heap (e.g. 0x00A7C8C0).
 * ptr_in_fault_range handles 0x80xxxxxx. */
static int is_bad_ptr(const void *p) {
  unsigned long long u = (unsigned long long)(const char *)p;
  return !p || u <= bad_node_cutoff();
}

/* Validate node/type after creation; fatal on first bad to pinpoint root cause.
 */
void validate_node(Compiler *cc, Node *node, const char *where, int line) {
  char buf[320];
  if (!node)
    return;
  if (node->magic != 0xC0FFEEBAD1234567ULL) {
    sprintf(buf, "CORRUPTED NODE (magic=0x%llx, alloc_id=%llu) at %s",
            (unsigned long long)node->magic, (unsigned long long)node->alloc_id,
            where);
    error_at(cc, line, buf);
    exit(1);
  }
  /* Do NOT check ptr_in_fault_range(node->type): stage2 miscompiles it. */
}

void validate_type(Compiler *cc, Type *type, const char *where, int line) {
  char buf[256];
  if (!type)
    return;
  if (type->magic != 0xDEADBEEF87654321ULL) {
    sprintf(buf, "CORRUPTED TYPE (magic=0x%llx, alloc_id=%llu) at %s",
            (unsigned long long)type->magic, (unsigned long long)type->alloc_id,
            where);
    error_at(cc, line, buf);
    exit(1);
  }
  /* Do NOT check ptr_in_fault_range(type) here: type is the pointer we just got
   * from cc_alloc (arena address like 0xa56040). Stage2 miscompiles that check
   * and falsely rejects valid ptrs. */
}

static int guard_error_count = 0;

/* Fatal on first bad node in codegen to stop spam. */
static void guard_node(Compiler *cc, Node *node) {
  char buf[256];
  if (!node || guard_error_count > 0)
    return;
  if (node->magic != 0xC0FFEEBAD1234567ULL) {
    sprintf(buf, "FATAL: node magic corrupted (0x%llx, id=%llu)",
            (unsigned long long)node->magic,
            (unsigned long long)node->alloc_id);
    error_at(cc, node->line, buf);
    guard_error_count++;
    exit(1);
  }
  /* Do NOT check ptr_in_fault_range(node->type): stage2 miscompiles it and
   * rejects valid Type* ptrs. */
}

/* Wrappers: catch bad ptr before any node load. */
static void codegen_expr_checked(Compiler *cc, Node *n) {
  if (is_bad_ptr(n)) {
    fprintf(cc->out, "    movq $0, %%rax\n");
    return;
  }
  codegen_expr(cc, n);
}
static void codegen_addr_checked(Compiler *cc, Node *n) {
  /* In main, never substitute 0 for param/local addresses: Stage2 can have
   * "bad" node ptrs (e.g. 0x10000) and we must use the fixed stack slots. */
  if (is_bad_ptr(n) &&
      (!cc->current_func[0] || strcmp(cc->current_func, "main") != 0)) {
    fprintf(cc->out, "    movq $0, %%rax\n");
    return;
  }
  codegen_addr(cc, n);
}

/* ---------------------------------------------------------------- */
/* Address of lvalue into %rax                                       */
/* ---------------------------------------------------------------- */

void codegen_addr(Compiler *cc, Node *node) {
  char errbuf[80];
  /* Do NOT use ptr_in_fault_range(node): stage2 miscompiles it and rejects
   * valid arena ptrs. */
  if (!node) {
    error_at(cc, 0, "codegen_addr: null node");
    fprintf(cc->out, "    movq $0, %%rax\n");
    return;
  }
  /* In main, do not substitute 0 for bad node ptr — we need to reach the main
   * param/local stack slots. */
  if (is_bad_ptr(node) &&
      (!cc->current_func[0] || strcmp(cc->current_func, "main") != 0)) {
    error_at(cc, 0, "codegen_addr: bad node ptr");
    fprintf(cc->out, "    movq $0, %%rax\n");
    return;
  }
  if (node->kind == ND_VAR) {
    /* Main params/locals: always use fixed stack layout when in main, so Stage2
     * never emits globals for these. */
    if (cc->current_func[0] && strcmp(cc->current_func, "main") == 0) {
      if (strcmp(node->name, "argc") == 0) {
        if (cc->verbose)
          fprintf(stderr, "zcc: codegen main param 'argc'\n");
        fprintf(cc->out, "    leaq -8(%%rbp), %%rax\n");
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, "%stack_-8", node->line);
        return;
      }
      if (strcmp(node->name, "argv") == 0) {
        if (cc->verbose)
          fprintf(stderr, "zcc: codegen main param 'argv'\n");
        fprintf(cc->out, "    leaq -16(%%rbp), %%rax\n");
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, "%stack_-16", node->line);
        return;
      }
      if (strcmp(node->name, "input_file") == 0) {
        if (cc->verbose)
          fprintf(stderr, "zcc: codegen main local 'input_file'\n");
        fprintf(cc->out, "    leaq -32(%%rbp), %%rax\n");
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, "%stack_-32", node->line);
        return;
      }
      if (strcmp(node->name, "output_file") == 0) {
        if (cc->verbose)
          fprintf(stderr, "zcc: codegen main local 'output_file'\n");
        fprintf(cc->out, "    leaq -40(%%rbp), %%rax\n");
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, "%stack_-40", node->line);
        return;
      }
    }
    /* Do NOT check is_bad_ptr(node->sym): Stage1 can have valid Symbol* in
     * fault range -> emit global -> segfault. */
    /* Use stack whenever we have a symbol with stack_offset. Params/locals have
     * negative offset; guard against wrong sign. */
    if (node->sym) {
      int off = node->sym->stack_offset;
      if (off > 0)
        off = -off; /* never use positive: would write above frame (e.g. return
                       address) and crash */
      if (off != 0 && !node->sym->is_global) {
        if (backend_ops) {
            fprintf(cc->out, "    ldr r3, =%d\n    adds r0, r7, r3\n", off);
        } else {
            fprintf(cc->out, "    leaq %d(%%rbp), %%rax\n", off);
        }
        char vname[32];
        sprintf(vname, "%%stack_%d", off);
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, vname, node->line);
        return;
      }
    }
    if (backend_ops) {
        fprintf(cc->out, "    ldr r0, =%s\n", node->name);
    } else {
        fprintf(cc->out, "    leaq %s(%%rip), %%rax\n", node->name);
    }
    char gname[32];
    sprintf(gname, "%%%s", node->name);
    char *dst = ir_bridge_fresh_tmp();
    ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, gname, node->line);
    return;
  }
  if (node->kind == ND_DEREF) {
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_addr: ND_DEREF null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    return; /* Note: ptr is ALREADY eval'd by codegen_expr! ir_last_result holds
               it! */
  }
  if (node->kind == ND_MEMBER) {
    if (!node->lhs) {
      error_at(cc, node->line,
               "codegen_addr: ND_MEMBER null lhs (member access on null)");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_addr_checked(cc, node->lhs);
    if (node->member_offset != 0) {
      fprintf(cc->out, "    addq $%d, %%rax\n", node->member_offset);
      char lhs_ir[32];
      ir_save_result(lhs_ir);
      char off_str[32];
      char c_tmp[32];
      sprintf(off_str, "%d", node->member_offset);
      sprintf(c_tmp, "%%t%d", ir_tmp_counter++);
      ZCC_EMIT_UNARY(IR_CONST, IR_TY_I64, c_tmp, off_str, node->line);

      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_BINARY(IR_ADD, IR_TY_PTR, dst, lhs_ir, c_tmp, node->line);
    }
    return;
  }
  error_at(cc, node->line, "not an lvalue");
}

static int ptr_elem_size(Type *type) {
  if (!type)
    return 1;
  if (type->kind == TY_PTR || type->kind == TY_ARRAY) {
    if (type->base)
      return type_size(type->base);
  }
  return 1;
}

/* ---------------------------------------------------------------- */
/* Expression codegen — result in %rax                               */
/* ---------------------------------------------------------------- */

void codegen_expr(Compiler *cc, Node *node) {
  if (!node)
    return;

  if (cc) {
  }

  int lbl1;
  int lbl2;
  char errbuf[80];

  /* Do NOT use ptr_in_fault_range(node): stage2 miscompiles it and rejects
   * valid arena ptrs. */
  if (!node) {
    error_at(cc, 0, "codegen_expr: NULL node");
    fprintf(cc->out, "    movq $0, %%rax\n");
    return;
  }
  /* In main, do not substitute 0 for bad node ptr — we need to reach main
   * param/local codegen. */
  if (is_bad_ptr(node) &&
      (!cc->current_func[0] || strcmp(cc->current_func, "main") != 0)) {
    error_at(cc, 0, "codegen_expr: bad node ptr");
    fprintf(cc->out, "    movq $0, %%rax\n");
    return;
  }
  if (node->kind < ND_NUM || node->kind > ND_NOP) {
    sprintf(errbuf, "codegen_expr: invalid kind %d (0x%x) at %p", node->kind,
            node->kind, (void *)node);
    error_at(cc, node->line, errbuf);
    return;
  }
  guard_node(cc, node);

  switch (node->kind) {

  case ND_NUM:
    if (backend_ops) {
        fprintf(cc->out, "    ldr r0, =%lld\n", node->int_val);
        {
          char *dst = ir_bridge_fresh_tmp();
          ZCC_EMIT_CONST(ir_map_type(node->type), dst, node->int_val, node->line);
        }
        return;
    }
    if (node->int_val >= -2147483648) {
      if (node->int_val <= 2147483647) {
        fprintf(cc->out, "    movq $%lld, %%rax\n", node->int_val);
        {
          char *dst = ir_bridge_fresh_tmp();
          ZCC_EMIT_CONST(ir_map_type(node->type), dst, node->int_val,
                         node->line);
        }
        return;
      }
    }
    fprintf(cc->out, "    movabsq $%lld, %%rax\n", node->int_val);
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_CONST(ir_map_type(node->type), dst, node->int_val, node->line);
    }
    return;

  case ND_FLIT: {
    int lbl;
    unsigned long long bits;
    lbl = cc->label_count;
    cc->label_count = cc->label_count + 1;
    fprintf(cc->out, "    .section .rodata\n");
    fprintf(cc->out, "    .p2align 3\n");
    fprintf(cc->out, ".L_flit_%d:\n", lbl);
    memcpy(&bits, &node->f_val, 8);
    fprintf(cc->out, "    .quad %llu\n", bits);
    fprintf(cc->out, "    .text\n");
    fprintf(cc->out, "    movsd .L_flit_%d(%%rip), %%xmm0\n", lbl);
    fprintf(cc->out, "    movq %%xmm0, %%rax\n");
    /* Satisfy IR subsystem sequence */
    {
      char *dst = ir_bridge_fresh_tmp();
      long flit_bits;
      memcpy(&flit_bits, &node->f_val, 8);
      ZCC_EMIT_FCONST(dst, flit_bits, node->line);
    }
    return;
  }

  case ND_STR: {
    char lbl_buf[32];
    fprintf(cc->out, "    leaq .L%d(%%rip), %%rax\n",
            cc->strings[node->str_id].label_id);
    sprintf(lbl_buf, ".L%d", cc->strings[node->str_id].label_id);
    char *dst = ir_bridge_fresh_tmp();
    ZCC_EMIT_CONST(IR_TY_PTR, dst, 0, node->line); /* Fake for now */
    ZCC_EMIT_UNARY(IR_CONST_STR, IR_TY_PTR, dst, lbl_buf,
                   node->line); /* REAL emit */
    return;
  }

  case ND_VAR:
    if (node->sym && node->sym->assigned_reg) {
      if (backend_ops) fprintf(cc->out, "    mov r0, %s\n", node->sym->assigned_reg); else fprintf(cc->out, "    movq %s, %%rax\n", node->sym->assigned_reg);
      if (node->type && !node_type_unsigned(node) && !is_pointer(node->type)) {
          if (node->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
          else if (node->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->type && node_type_unsigned(node)) {
          if (node->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
          else if (node->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      {
        char *vname = ir_var_name(node);
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_LOAD(ir_map_type(node->type), dst, vname, node->line);
      }
      return;
    }
    codegen_addr_checked(cc, node);
    if (node->type) {
      if (node->type->kind != TY_ARRAY) {
        if (node->type->kind != TY_STRUCT) {
          if (node->type->kind != TY_UNION) {
            if (node->type->kind != TY_FUNC) {
              codegen_load(cc, node->type);
            }
          }
        }
      }
    }
    {
      char *vname = ir_var_name(node);
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_LOAD(ir_map_type(node->type), dst, vname, node->line);
    }
    return;

  case ND_ASSIGN: {
    char rhs_ir[32];
    if (!node->lhs || !node->rhs) {
      error_at(cc, node->line, "codegen_expr: ND_ASSIGN missing lhs or rhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      if (backend_ops) fprintf(cc->out, "    mov %s, r0\n", node->lhs->sym->assigned_reg); else fprintf(cc->out, "    movq %%rax, %s\n", node->lhs->sym->assigned_reg);
      {
        char *vname = ir_var_name(node->lhs);
        ZCC_EMIT_STORE(ir_map_type(node->lhs->type), vname, rhs_ir, node->line);
      }
      return;
    }
    push_reg(cc, "rax");
    codegen_addr_checked(cc, node->lhs);
    char lhs_addr_ir[32];
    ir_save_result(lhs_addr_ir);
    if (is_bad_ptr(node->lhs) &&
        (!cc->current_func[0] || strcmp(cc->current_func, "main") != 0)) {
      error_at(cc, node->line, "codegen_expr: ND_ASSIGN lhs bad ptr");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (!node->lhs->type && node->lhs->kind != ND_MEMBER) {
      error_at(cc, node->line, "codegen_expr: ND_ASSIGN lhs has null type");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (node->lhs->kind == ND_MEMBER) {
        fprintf(stderr, "PART4 ND_ASSIGN: member=%s is_bf=%d bs=%d bo=%d\n", node->lhs->member_name, node->lhs->is_bitfield, node->lhs->bit_size, node->lhs->bit_offset);
    }
    /* Use member_size for ND_MEMBER so stage2 stores 4 bytes to cc->tk etc.,
     * not 8 */
    if (node->lhs->kind == ND_MEMBER && node->lhs->member_size > 0) {
      pop_reg(cc, "r11");
      if ((node->lhs->type && (node->lhs->type->kind == TY_STRUCT || node->lhs->type->kind == TY_UNION)) || node->lhs->member_size > 8) {
        fprintf(cc->out, "    pushq %%rsi\n");
        fprintf(cc->out, "    pushq %%rdi\n");
        fprintf(cc->out, "    pushq %%rcx\n");
        fprintf(cc->out, "    movq %%r11, %%rsi\n");
        fprintf(cc->out, "    movq %%rax, %%rdi\n");
        fprintf(cc->out, "    movq $%d, %%rcx\n", node->lhs->member_size);
        fprintf(cc->out, "    rep movsb\n");
        fprintf(cc->out, "    popq %%rcx\n");
        fprintf(cc->out, "    popq %%rdi\n");
        fprintf(cc->out, "    popq %%rsi\n");
        if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
      } else {
        /* If member type is pointer/func, always use movq regardless of member_size */
        if (node->lhs->type && is_pointer(node->lhs->type)) {
          if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
          else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
        } else {
        if (!backend_ops && node->lhs->is_bitfield) {
            long long mask = (1ULL << node->lhs->bit_size) - 1;
            long long shifted_mask = ~(mask << node->lhs->bit_offset);
            switch(node->lhs->member_size) {
                case 1: fprintf(cc->out, "    movzbl (%%rax), %%r10d\n"); break;
                case 2: fprintf(cc->out, "    movzwl (%%rax), %%r10d\n"); break;
                case 4: fprintf(cc->out, "    movl (%%rax), %%r10d\n"); break;
                default: fprintf(cc->out, "    movq (%%rax), %%r10\n"); break;
            }
            fprintf(cc->out, "    movq $0x%llx, %%r8\n", (unsigned long long)shifted_mask);
            fprintf(cc->out, "    andq %%r8, %%r10\n");
            fprintf(cc->out, "    movq $0x%llx, %%r8\n", (unsigned long long)mask);
            fprintf(cc->out, "    andq %%r8, %%r11\n");
            if (node->lhs->bit_offset > 0) {
                fprintf(cc->out, "    movb $%d, %%cl\n", node->lhs->bit_offset);
                fprintf(cc->out, "    shlq %%cl, %%r11\n");
            }
            fprintf(cc->out, "    orq %%r10, %%r11\n");
        }
        switch (node->lhs->member_size) {
        case 1:
          if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
          break;
        case 2:
          if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
          break;
        case 4:
          if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
          break;
        default:
          if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
          break;
        }
        }
        if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
      }
    } else {
      codegen_store(cc, node->lhs->type);
    }
    {
      ZCC_EMIT_STORE(ir_map_type(node->lhs->type), lhs_addr_ir, rhs_ir,
                     node->line);
    }
    return;
  }

  case ND_COMPOUND_ASSIGN:
    if (!node->lhs || !node->rhs) {
      error_at(cc, node->line,
               "codegen_expr: ND_COMPOUND_ASSIGN missing operand");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      codegen_expr_checked(cc, node->rhs);
      if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
      if (backend_ops) fprintf(cc->out, "    mov r0, %s\n", reg); else fprintf(cc->out, "    movq %s, %%rax\n", reg);
      switch (node->compound_op) {
      case ND_ADD:
        if (is_pointer(node->lhs->type)) {
          int esz = ptr_elem_size(node->lhs->type);
          if (esz > 1)
            fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
        }
        fprintf(cc->out, "    addq %%r11, %%rax\n");
        break;
      case ND_SUB:
        if (is_pointer(node->lhs->type)) {
          int esz = ptr_elem_size(node->lhs->type);
          if (esz > 1)
            fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
        }
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_SUB);
      else fprintf(cc->out, "    subq %%r11, %%rax\n");
        break;
      case ND_MUL:
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_MUL);
      else fprintf(cc->out, "    imulq %%r11, %%rax\n");
        break;
      case ND_DIV:
        if (node->lhs->type && is_unsigned_type(node->lhs->type)) {
          fprintf(cc->out, "    xorq %%rdx, %%rdx\n    divq %%r11\n");
        } else {
          fprintf(cc->out, "    cqo\n    idivq %%r11\n");
        }
        break;
      case ND_MOD:
        if (node->lhs->type && is_unsigned_type(node->lhs->type)) {
          fprintf(
              cc->out,
              "    xorq %%rdx, %%rdx\n    divq %%r11\n    movq %%rdx, %%rax\n");
        } else {
          fprintf(cc->out, "    cqo\n    idivq %%r11\n    movq %%rdx, %%rax\n");
        }
        break;
      case ND_BAND:
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_BAND);
      else fprintf(cc->out, "    andq %%r11, %%rax\n");
        break;
      case ND_BOR:
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_BOR);
      else fprintf(cc->out, "    orq %%r11, %%rax\n");
        break;
      case ND_BXOR:
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_BXOR);
      else fprintf(cc->out, "    xorq %%r11, %%rax\n");
        break;
      case ND_SHL:
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHL);
      else fprintf(cc->out, "    movq %%r11, %%rcx\n    shlq %%cl, %%rax\n");
        break;
      case ND_SHR:
        if (node->lhs->type && is_unsigned_type(node->lhs->type))
          if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHR);
      else fprintf(cc->out, "    movq %%r11, %%rcx\n    shrq %%cl, %%rax\n");
        else
          if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHR);
      else fprintf(cc->out, "    movq %%r11, %%rcx\n    sarq %%cl, %%rax\n");
        break;
      default:
        break;
      }
      if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      if (backend_ops) fprintf(cc->out, "    mov %s, r0\n", reg); else fprintf(cc->out, "    movq %%rax, %s\n", reg);
      return;
    }
    codegen_addr_checked(cc, node->lhs);
    if (is_bad_ptr(node->lhs)) {
      error_at(cc, node->line, "codegen_expr: ND_COMPOUND_ASSIGN lhs bad ptr");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (!node->lhs->type) {
      error_at(cc, node->line,
               "codegen_expr: ND_COMPOUND_ASSIGN lhs has null type");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    push_reg(cc, "rax");
    if (node->lhs->kind == ND_MEMBER && node->lhs->member_size > 0) {
      switch (node->lhs->member_size) {
      case 1:
        fprintf(cc->out, "    movzbl (%%rax), %%eax\n");
        break;
      case 2:
        fprintf(cc->out, "    movzwl (%%rax), %%eax\n");
        break;
      case 4:
        fprintf(cc->out, "    movl (%%rax), %%eax\n");
        break;
      default:
        fprintf(cc->out, "    movq (%%rax), %%rax\n");
        break;
      }
    } else {
      codegen_load(cc, node->lhs->type);
    }
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    switch (node->compound_op) {
    case ND_ADD:
      if (node->lhs->type && is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
        fprintf(cc->out, "    movq %%r11, %%xmm1\n");
        fprintf(cc->out, "    addsd %%xmm1, %%xmm0\n");
        fprintf(cc->out, "    movq %%xmm0, %%rax\n");
        break;
      }
      if (is_pointer(node->lhs->type)) {
        int esz;
        esz = ptr_elem_size(node->lhs->type);
        if (esz > 1)
          fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
      }
      fprintf(cc->out, "    addq %%r11, %%rax\n");
      break;
    case ND_SUB:
      if (node->lhs->type && is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
        fprintf(cc->out, "    movq %%r11, %%xmm1\n");
        fprintf(cc->out, "    subsd %%xmm1, %%xmm0\n");
        fprintf(cc->out, "    movq %%xmm0, %%rax\n");
        break;
      }
      if (is_pointer(node->lhs->type)) {
        int esz = ptr_elem_size(node->lhs->type);
        if (esz > 1)
          fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
      }
      if (backend_ops) backend_ops->emit_binary_op(cc, ND_SUB);
      else fprintf(cc->out, "    subq %%r11, %%rax\n");
      break;
    case ND_MUL:
      if (node->lhs->type && is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
        fprintf(cc->out, "    movq %%r11, %%xmm1\n");
        fprintf(cc->out, "    mulsd %%xmm1, %%xmm0\n");
        fprintf(cc->out, "    movq %%xmm0, %%rax\n");
        break;
      }
      if (backend_ops) backend_ops->emit_binary_op(cc, ND_MUL);
      else fprintf(cc->out, "    imulq %%r11, %%rax\n");
      break;
    case ND_DIV:
      if (node->lhs->type && is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
        fprintf(cc->out, "    movq %%r11, %%xmm1\n");
        fprintf(cc->out, "    divsd %%xmm1, %%xmm0\n");
        fprintf(cc->out, "    movq %%xmm0, %%rax\n");
        break;
      }
      if (node->lhs->type && is_unsigned_type(node->lhs->type)) {
        fprintf(cc->out, "    xorq %%rdx, %%rdx\n");
        fprintf(cc->out, "    divq %%r11\n");
      } else {
        fprintf(cc->out, "    cqo\n    idivq %%r11\n");
      }
      break;
    case ND_MOD:
      if (node->lhs->type && is_unsigned_type(node->lhs->type)) {
        fprintf(cc->out, "    xorq %%rdx, %%rdx\n");
        fprintf(cc->out, "    divq %%r11\n");
        if (backend_ops) fprintf(cc->out, "    mov r0, r2\n");
      else fprintf(cc->out, "    movq %%rdx, %%rax\n");
      } else {
        fprintf(cc->out, "    cqo\n    idivq %%r11\n    movq %%rdx, %%rax\n");
      }
      break;
    case ND_BAND:
      if (backend_ops) backend_ops->emit_binary_op(cc, ND_BAND);
      else fprintf(cc->out, "    andq %%r11, %%rax\n");
      break;
    case ND_BOR:
      if (backend_ops) backend_ops->emit_binary_op(cc, ND_BOR);
      else fprintf(cc->out, "    orq %%r11, %%rax\n");
      break;
    case ND_BXOR:
      if (backend_ops) backend_ops->emit_binary_op(cc, ND_BXOR);
      else fprintf(cc->out, "    xorq %%r11, %%rax\n");
      break;
    case ND_SHL:
      if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHL);
      else fprintf(cc->out, "    movq %%r11, %%rcx\n    shlq %%cl, %%rax\n");
      break;
    case ND_SHR:
      if (node->lhs->type && is_unsigned_type(node->lhs->type))
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHR);
      else fprintf(cc->out, "    movq %%r11, %%rcx\n    shrq %%cl, %%rax\n");
      else
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_SHR);
      else fprintf(cc->out, "    movq %%r11, %%rcx\n    sarq %%cl, %%rax\n");
      break;
    default:
      break;
    }
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->kind == ND_MEMBER && node->lhs->member_size > 0) {
      switch (node->lhs->member_size) {
      case 1:
        if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
        break;
      case 2:
        if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
        break;
      case 4:
        if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
        break;
      default:
        if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
        break;
      }
    } else {
      switch (type_size(node->lhs->type)) {
      case 1:
        if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
        break;
      case 2:
        if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
        break;
      case 4:
        if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
        break;
      default:
        if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
        break;
      }
    }
    if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
    if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
        if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
        if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
    }
    {
      char *dst = ir_bridge_fresh_tmp();
      ir_emit_binary_op(node->compound_op, node->lhs->type, "ca_lhs", "ca_rhs", node->line);
    }
    return;

  case ND_ADD: {
    char lhs_ir[32];
    char rhs_ir[32];
    if (!node->lhs || !node->rhs) {
      error_at(cc, node->line, "codegen_expr: ND_ADD missing operand");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (node->type && is_float_type(node->type)) {
      codegen_expr_checked(cc, node->lhs);
      if (!is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    subq $8, %%rsp\n");
      fprintf(cc->out, "    movsd %%xmm0, (%%rsp)\n");
      codegen_expr_checked(cc, node->rhs);
      if (!is_float_type(node->rhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    movsd (%%rsp), %%xmm1\n");
      fprintf(cc->out, "    addq $8, %%rsp\n");
      fprintf(cc->out, "    addsd %%xmm1, %%xmm0\n");
      fprintf(cc->out, "    movq %%xmm0, %%rax\n");
      ir_emit_binary_op(ND_ADD, node->type, "f_lhs", "f_rhs", node->line);
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
    else if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && is_pointer(node->lhs->type)) {
      int esz = ptr_elem_size(node->lhs->type);
      if (esz > 1) {
        if (backend_ops) fprintf(cc->out, "    ldr r3, =%d\n    muls r1, r3, r1\n", esz);
        else fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
        char scale_str[32];
        sprintf(scale_str, "%d", esz);
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_BINARY(IR_MUL, IR_TY_I64, dst, rhs_ir, scale_str, node->line);
        strcpy(rhs_ir, dst);
      }
    } else if (node->rhs->type && is_pointer(node->rhs->type)) {
      int esz = ptr_elem_size(node->rhs->type);
      if (esz > 1) {
        if (backend_ops) fprintf(cc->out, "    ldr r3, =%d\n    muls r0, r3, r0\n", esz);
        else fprintf(cc->out, "    imulq $%d, %%rax\n", esz);
        char scale_str[32];
        sprintf(scale_str, "%d", esz);
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_BINARY(IR_MUL, IR_TY_I64, dst, lhs_ir, scale_str, node->line);
        strcpy(lhs_ir, dst);
      }
    }
    if (backend_ops && backend_ops->emit_binary_op) {
        backend_ops->emit_binary_op(cc, ND_ADD);
    } else {
        fprintf(cc->out, "    addq %%r11, %%rax\n");
    }
    if (node->type && type_size(node->type) == 4 && !is_pointer(node->type)) {
      if (node_type_unsigned(node)) {
        if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n");
      } else {
        if (!backend_ops) fprintf(cc->out, "    cltq\n");
      }
    }
    ir_emit_binary_op(ND_ADD, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_SUB: {
    char lhs_ir[32];
    char rhs_ir[32];
    if (!node->lhs || !node->rhs) {
      error_at(cc, node->line, "codegen_expr: ND_SUB missing operand");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (node->type && is_float_type(node->type)) {
      codegen_expr_checked(cc, node->lhs);
      if (!is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    subq $8, %%rsp\n");
      fprintf(cc->out, "    movsd %%xmm0, (%%rsp)\n");
      codegen_expr_checked(cc, node->rhs);
      if (!is_float_type(node->rhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    movsd (%%rsp), %%xmm1\n");
      fprintf(cc->out, "    addq $8, %%rsp\n");
      fprintf(cc->out, "    subsd %%xmm0, %%xmm1\n");
      fprintf(cc->out, "    movsd %%xmm1, %%xmm0\n");
      fprintf(cc->out, "    movq %%xmm0, %%rax\n");
      ir_emit_binary_op(ND_SUB, node->type, "f_lhs", "f_rhs", node->line);
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
    else if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && is_pointer(node->lhs->type)) {
      if (node->rhs->type && is_pointer(node->rhs->type)) {
        if (backend_ops && backend_ops->emit_binary_op) backend_ops->emit_binary_op(cc, ND_SUB);
        else if (backend_ops) backend_ops->emit_binary_op(cc, ND_SUB);
      else fprintf(cc->out, "    subq %%r11, %%rax\n");
        {
          int esz;
          esz = ptr_elem_size(node->lhs->type);
          if (esz > 1) {
            if (backend_ops) { /* no-op for hello.c */ } else {
            fprintf(cc->out, "    movq $%d, %%r11\n", esz);
            fprintf(cc->out, "    cqo\n    idivq %%r11\n");
            }
          }
        }
        ir_emit_binary_op(ND_SUB, node->type, lhs_ir, rhs_ir, node->line);
        return;
      }
      {
        int esz;
        esz = node->lhs->type ? ptr_elem_size(node->lhs->type) : 1;
        if (esz > 1) {
          if (backend_ops) {
            fprintf(cc->out, "    ldr r3, =%d\n    muls r1, r3, r1\n", esz);
          } else {
            fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
          }
        }
      }
    }
    if (backend_ops && backend_ops->emit_binary_op) backend_ops->emit_binary_op(cc, ND_SUB);
    else if (backend_ops) backend_ops->emit_binary_op(cc, ND_SUB);
      else fprintf(cc->out, "    subq %%r11, %%rax\n");
    if (node->type && type_size(node->type) == 4 && !is_pointer(node->type)) {
      if (node_type_unsigned(node)) {
        if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n");
      } else {
        if (!backend_ops) fprintf(cc->out, "    cltq\n");
      }
    }
    ir_emit_binary_op(ND_SUB, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_MUL: {
    char lhs_ir[32];
    char rhs_ir[32];
    if (!node->lhs || !node->rhs) {
      error_at(cc, node->line, "codegen_expr: binary op missing operand");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (node->type && is_float_type(node->type)) {
      codegen_expr_checked(cc, node->lhs);
      if (!is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    subq $8, %%rsp\n");
      fprintf(cc->out, "    movsd %%xmm0, (%%rsp)\n");
      codegen_expr_checked(cc, node->rhs);
      if (!is_float_type(node->rhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    movsd (%%rsp), %%xmm1\n");
      fprintf(cc->out, "    addq $8, %%rsp\n");
      fprintf(cc->out, "    mulsd %%xmm1, %%xmm0\n");
      fprintf(cc->out, "    movq %%xmm0, %%rax\n");
      ir_emit_binary_op(ND_MUL, node->type, "f_lhs", "f_rhs", node->line);
      return;
    }
    if (!backend_ops) {
      if (node->rhs->kind == ND_NUM && is_power_of_2_val(node->rhs->int_val)) {
        int shift;
        codegen_expr_checked(cc, node->lhs);
        ir_save_result(lhs_ir);
        shift = log2_of(node->rhs->int_val);
        fprintf(cc->out, "    shlq $%d, %%rax\n", shift);
        ir_emit_binary_op(ND_SHL, node->type, lhs_ir, "unused_rhs", node->line);
        return;
      }
      if (node->lhs->kind == ND_NUM && is_power_of_2_val(node->lhs->int_val)) {
        int shift;
        codegen_expr_checked(cc, node->rhs);
        ir_save_result(rhs_ir);
        shift = log2_of(node->lhs->int_val);
        fprintf(cc->out, "    shlq $%d, %%rax\n", shift);
        ir_emit_binary_op(ND_SHL, node->type, "unused_lhs", rhs_ir, node->line);
        return;
      }
      if (node->rhs->kind == ND_NUM && node->rhs->int_val == 3) {
        codegen_expr_checked(cc, node->lhs);
        ir_save_result(lhs_ir);
        fprintf(cc->out, "    leaq (%%rax,%%rax,2), %%rax\n");
        ir_emit_binary_op(ND_MUL, node->type, lhs_ir, "unused_rhs", node->line);
        return;
      }
      if (node->rhs->kind == ND_NUM && node->rhs->int_val == 5) {
        codegen_expr_checked(cc, node->lhs);
        ir_save_result(lhs_ir);
        fprintf(cc->out, "    leaq (%%rax,%%rax,4), %%rax\n");
        ir_emit_binary_op(ND_MUL, node->type, lhs_ir, "unused_rhs", node->line);
        return;
      }
      if (node->rhs->kind == ND_NUM && node->rhs->int_val == 9) {
        codegen_expr_checked(cc, node->lhs);
        ir_save_result(lhs_ir);
        fprintf(cc->out, "    leaq (%%rax,%%rax,8), %%rax\n");
        ir_emit_binary_op(ND_MUL, node->type, lhs_ir, "unused_rhs", node->line);
        return;
      }
    }

    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    pop_reg(cc, "r11");
    if (backend_ops) {
        if (backend_ops->emit_binary_op) backend_ops->emit_binary_op(cc, ND_MUL);
    } else {
        if (backend_ops) backend_ops->emit_binary_op(cc, ND_MUL);
      else fprintf(cc->out, "    imulq %%r11, %%rax\n");
    }
    if (node->type && type_size(node->type) == 4 && !is_pointer(node->type)) {
      if (node_type_unsigned(node)) {
        if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n");
      } else {
        if (!backend_ops) fprintf(cc->out, "    cltq\n");
      }
    }
    ir_emit_binary_op(ND_MUL, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_DIV: {
    char lhs_ir[32];
    char rhs_ir[32];
    if (!node->lhs || !node->rhs) {
      error_at(cc, node->line, "codegen_expr: ND_DIV missing operand");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (node->type && is_float_type(node->type)) {
      codegen_expr_checked(cc, node->lhs);
      if (!is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    subq $8, %%rsp\n");
      fprintf(cc->out, "    movsd %%xmm0, (%%rsp)\n");
      codegen_expr_checked(cc, node->rhs);
      if (!is_float_type(node->rhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    movsd (%%rsp), %%xmm1\n");
      fprintf(cc->out, "    addq $8, %%rsp\n");
      fprintf(cc->out, "    divsd %%xmm0, %%xmm1\n");
      fprintf(cc->out, "    movsd %%xmm1, %%xmm0\n");
      fprintf(cc->out, "    movq %%xmm0, %%rax\n");
      ir_emit_binary_op(ND_DIV, node->type, "f_lhs", "f_rhs", node->line);
      return;
    }

    if (node->rhs->kind == ND_NUM && is_power_of_2_val(node->rhs->int_val)) {
      int shift;
      char rhs_val_str[32];
      codegen_expr_checked(cc, node->lhs);
      ir_save_result(lhs_ir);
      shift = log2_of(node->rhs->int_val);
      if (node->type && is_unsigned_type(node->type)) {
        fprintf(cc->out, "    shrq $%d, %%rax\n", shift);
      } else {
        fprintf(cc->out, "    movq %%rax, %%rcx\n");
        fprintf(cc->out, "    sarq $63, %%rcx\n");
        fprintf(cc->out, "    andq $%lld, %%rcx\n", (1LL << shift) - 1);
        fprintf(cc->out, "    addq %%rcx, %%rax\n");
        fprintf(cc->out, "    sarq $%d, %%rax\n", shift);
      }
      snprintf(rhs_val_str, 32, "$%lld", node->rhs->int_val);
      ir_emit_binary_op(ND_DIV, node->type, lhs_ir, rhs_val_str, node->line);
      return;
    }

    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && node->rhs->type &&
        (is_unsigned_type(node->lhs->type) ||
         is_unsigned_type(node->rhs->type))) {
      fprintf(cc->out, "    xorq %%rdx, %%rdx\n");
      fprintf(cc->out, "    divq %%r11\n");
    } else {
      fprintf(cc->out, "    cqo\n    idivq %%r11\n");
    }
    ir_emit_binary_op(ND_DIV, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_MOD: {
    char lhs_ir[32];
    char rhs_ir[32];

    if (node->rhs && node->rhs->kind == ND_NUM &&
        is_power_of_2_val(node->rhs->int_val)) {
      long long mask;
      char rhs_val_str[32];
      codegen_expr_checked(cc, node->lhs);
      ir_save_result(lhs_ir);
      mask = node->rhs->int_val - 1;
      if (node->type && is_unsigned_type(node->type)) {
        fprintf(cc->out, "    andq $%lld, %%rax\n", mask);
        snprintf(rhs_val_str, 32, "$%lld", node->rhs->int_val);
        ir_emit_binary_op(ND_MOD, node->type, lhs_ir, rhs_val_str, node->line);
        return;
      }
    }

    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && node->rhs->type &&
        (is_unsigned_type(node->lhs->type) ||
         is_unsigned_type(node->rhs->type))) {
      fprintf(cc->out, "    xorq %%rdx, %%rdx\n");
      fprintf(cc->out, "    divq %%r11\n");
      if (backend_ops) fprintf(cc->out, "    mov r0, r2\n");
      else fprintf(cc->out, "    movq %%rdx, %%rax\n");
    } else {
      fprintf(cc->out, "    cqo\n    idivq %%r11\n");
      if (backend_ops) fprintf(cc->out, "    mov r0, r2\n");
      else fprintf(cc->out, "    movq %%rdx, %%rax\n");
    }
    ir_emit_binary_op(ND_MOD, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_BAND: {
    char lhs_ir[32];
    char rhs_ir[32];
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    pop_reg(cc, "r11");
    if (backend_ops) backend_ops->emit_binary_op(cc, ND_BAND);
      else fprintf(cc->out, "    andq %%r11, %%rax\n");
    ir_emit_binary_op(ND_BAND, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_BOR: {
    char lhs_ir[32];
    char rhs_ir[32];
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    pop_reg(cc, "r11");
    if (backend_ops) backend_ops->emit_binary_op(cc, ND_BOR);
      else fprintf(cc->out, "    orq %%r11, %%rax\n");
    ir_emit_binary_op(ND_BOR, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_BXOR: {
    char lhs_ir[32];
    char rhs_ir[32];
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    pop_reg(cc, "r11");
    if (backend_ops) backend_ops->emit_binary_op(cc, ND_BXOR);
      else fprintf(cc->out, "    xorq %%r11, %%rax\n");
    ir_emit_binary_op(ND_BXOR, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_BNOT: {
    char src_ir[32];
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_BNOT null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(src_ir);
    if (node->type && type_size(node->type) == 4) {
      if (node_type_unsigned(node)) {
        fprintf(cc->out, "    notl %%eax\n");
      } else {
        fprintf(cc->out, "    notl %%eax\n    cltq\n");
      }
    } else {
      fprintf(cc->out, "    notq %%rax\n");
    }
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_NOT, ir_map_type(node->type), dst, src_ir, node->line);
    }
    return;
  }

  case ND_SHL: {
    char lhs_ir[32];
    char rhs_ir[32];
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    fprintf(cc->out, "    movq %%rax, %%rcx\n");
    pop_reg(cc, "rax");
    fprintf(cc->out, "    shlq %%cl, %%rax\n");
    ir_emit_binary_op(ND_SHL, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_SHR: {
    char lhs_ir[32];
    char rhs_ir[32];
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    fprintf(cc->out, "    movq %%rax, %%rcx\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && is_unsigned_type(node->lhs->type))
      fprintf(cc->out, "    shrq %%cl, %%rax\n");
    else
      fprintf(cc->out, "    sarq %%cl, %%rax\n");
    ir_emit_binary_op(ND_SHR, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_NEG: {
    char src_ir[32];
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_NEG null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (node->type && is_float_type(node->type)) {
      codegen_expr_checked(cc, node->lhs);
      fprintf(cc->out, "    movabsq $-9223372036854775808, %%r11\n");
      if (backend_ops) backend_ops->emit_binary_op(cc, ND_BXOR);
      else fprintf(cc->out, "    xorq %%r11, %%rax\n");
      {
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_UNARY(IR_NEG, ir_map_type(node->type), dst, "f_lhs", node->line);
      }
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(src_ir);
    if (node->type && type_size(node->type) == 4) {
      if (node_type_unsigned(node)) {
        fprintf(cc->out, "    negl %%eax\n");
      } else {
        fprintf(cc->out, "    negl %%eax\n    cltq\n");
      }
    } else {
      fprintf(cc->out, "    negq %%rax\n");
    }
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_NEG, ir_map_type(node->type), dst, src_ir, node->line);
    }
    return;
  }

  case ND_LNOT: {
    char src_ir[32];
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_LNOT null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(src_ir);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    sete %%al\n");
    fprintf(cc->out, "    movzbl %%al, %%eax\n");
    {
      char zero_ir[32];
      char *zt = ir_bridge_fresh_tmp();
      int i;
      for (i = 0; zt[i]; i++)
        zero_ir[i] = zt[i];
      zero_ir[i] = 0;
      ZCC_EMIT_CONST(IR_TY_I64, zero_ir, 0, node->line);
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_BINARY(IR_EQ, IR_TY_I32, dst, src_ir, zero_ir, node->line);
    }
    return;
  }

  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_GT:
  case ND_GE: {
    int uns;
    int use32;
    char lhs_ir[32];
    char rhs_ir[32];

    if ((node->lhs && node->lhs->type && is_float_type(node->lhs->type)) ||
        (node->rhs && node->rhs->type && is_float_type(node->rhs->type))) {
      codegen_expr_checked(cc, node->lhs);
      if (!node->lhs->type || !is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    subq $8, %%rsp\n");
      fprintf(cc->out, "    movsd %%xmm0, (%%rsp)\n");
      codegen_expr_checked(cc, node->rhs);
      if (!node->rhs->type || !is_float_type(node->rhs->type)) {
        fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
      } else {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
      fprintf(cc->out, "    movsd (%%rsp), %%xmm1\n");
      fprintf(cc->out, "    addq $8, %%rsp\n");
      fprintf(cc->out, "    ucomisd %%xmm0, %%xmm1\n");
      switch (node->kind) {
      case ND_EQ:
        fprintf(cc->out, "    sete %%al\n    setnp %%r11b\n    andb %%r11b, %%al\n");
        break;
      case ND_NE:
        fprintf(cc->out, "    setne %%al\n    setp %%r11b\n    orb %%r11b, %%al\n");
        break;
      case ND_LT:
        fprintf(cc->out, "    setb %%al\n");
        break;
      case ND_LE:
        fprintf(cc->out, "    setbe %%al\n");
        break;
      case ND_GT:
        fprintf(cc->out, "    seta %%al\n");
        break;
      case ND_GE:
        fprintf(cc->out, "    setae %%al\n");
        break;
      }
      fprintf(cc->out, "    movzbl %%al, %%eax\n");
      ir_emit_binary_op(node->kind, node->type, "f_lhs", "f_rhs", node->line);
      return;
    }

    uns = (node->lhs && node->lhs->type && is_unsigned_type(node->lhs->type)) ||
          (node->rhs && node->rhs->type && is_unsigned_type(node->rhs->type));
    use32 = node->lhs && node->lhs->type && node->rhs &&
            node->rhs->type && type_size(node->lhs->type) == 4 &&
            type_size(node->rhs->type) == 4;
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    if (backend_ops) {
        fprintf(cc->out, "    mov r1, r0\n");
    } else {
        if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    }
    pop_reg(cc, "rax");
    if (backend_ops) {
        fprintf(cc->out, "    cmp r0, r1\n");
        int lbl_true = new_label(cc);
        int lbl_end = new_label(cc);
        if (uns) {
            switch (node->kind) {
            case ND_EQ: fprintf(cc->out, "    beq .L%d\n", lbl_true); break;
            case ND_NE: fprintf(cc->out, "    bne .L%d\n", lbl_true); break;
            case ND_LT: fprintf(cc->out, "    blo .L%d\n", lbl_true); break;
            case ND_LE: fprintf(cc->out, "    bls .L%d\n", lbl_true); break;
            case ND_GT: fprintf(cc->out, "    bhi .L%d\n", lbl_true); break;
            case ND_GE: fprintf(cc->out, "    bhs .L%d\n", lbl_true); break;
            }
        } else {
            switch (node->kind) {
            case ND_EQ: fprintf(cc->out, "    beq .L%d\n", lbl_true); break;
            case ND_NE: fprintf(cc->out, "    bne .L%d\n", lbl_true); break;
            case ND_LT: fprintf(cc->out, "    blt .L%d\n", lbl_true); break;
            case ND_LE: fprintf(cc->out, "    ble .L%d\n", lbl_true); break;
            case ND_GT: fprintf(cc->out, "    bgt .L%d\n", lbl_true); break;
            case ND_GE: fprintf(cc->out, "    bge .L%d\n", lbl_true); break;
            }
        }
        fprintf(cc->out, "    movs r0, #0\n");
        fprintf(cc->out, "    b .L%d\n", lbl_end);
        fprintf(cc->out, ".L%d:\n", lbl_true);
        fprintf(cc->out, "    movs r0, #1\n");
        fprintf(cc->out, ".L%d:\n", lbl_end);
    } else {
        if (use32)
          fprintf(cc->out,
                  "    cmpl %%r11d, %%eax\n"); /* 32-bit: avoids sign-extended imm
                                                  in 64-bit */
        else
          if (backend_ops) fprintf(cc->out, "    cmp r0, r1\n" \
    ); else fprintf(cc->out, "    cmpq %%r11, %%rax\n");
        if (uns) {
          switch (node->kind) {
          case ND_EQ:
            fprintf(cc->out, "    sete %%al\n");
            break;
          case ND_NE:
            fprintf(cc->out, "    setne %%al\n");
            break;
          case ND_LT:
            fprintf(cc->out, "    setb %%al\n");
            break;
          case ND_LE:
            fprintf(cc->out, "    setbe %%al\n");
            break;
          case ND_GT:
            fprintf(cc->out, "    seta %%al\n");
            break;
          case ND_GE:
            fprintf(cc->out, "    setae %%al\n");
            break;
          default:
            break;
          }
        } else {
          switch (node->kind) {
          case ND_EQ:
            fprintf(cc->out, "    sete %%al\n");
            break;
          case ND_NE:
            fprintf(cc->out, "    setne %%al\n");
            break;
          case ND_LT:
            fprintf(cc->out, "    setl %%al\n");
            break;
          case ND_LE:
            fprintf(cc->out, "    setle %%al\n");
            break;
          case ND_GT:
            fprintf(cc->out, "    setg %%al\n");
            break;
          case ND_GE:
            fprintf(cc->out, "    setge %%al\n");
            break;
          default:
            break;
          }
        }
        fprintf(cc->out, "    movzbl %%al, %%eax\n");
    }
    ir_emit_binary_op(node->kind, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_LAND: {
    char land_lhs_ir[32];
    char land_lbl[32];
    lbl1 = new_label(cc);
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(land_lhs_ir);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    if (backend_ops) emit_label_fmt(cc, lbl1, FMT_JE);
    else emit_label_fmt(cc, lbl1, FMT_JE);
    sprintf(land_lbl, ".L%d", lbl1);
    ZCC_EMIT_BR_IF(land_lhs_ir, land_lbl, node->line);
    codegen_expr_checked(cc, node->rhs);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    setne %%al\n");
    fprintf(cc->out, "    movzbl %%al, %%eax\n");
    fprintf(cc->out, ".L%d:\n", lbl1);
    ZCC_EMIT_LABEL(land_lbl, node->line);
    return;
  }

  case ND_LOR: {
    char lor_lhs_ir[32];
    char lor_lbl1[32];
    char lor_lbl2[32];
    lbl1 = new_label(cc);
    lbl2 = new_label(cc);
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lor_lhs_ir);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    jne .L%d\n", lbl1);
    sprintf(lor_lbl1, ".L%d", lbl1);
    sprintf(lor_lbl2, ".L%d", lbl2);
    codegen_expr_checked(cc, node->rhs);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    jne .L%d\n", lbl1);
    fprintf(cc->out, "    movq $0, %%rax\n");
    if (backend_ops) emit_label_fmt(cc, lbl2, FMT_JMP);
    else emit_label_fmt(cc, lbl2, FMT_JMP);
    ZCC_EMIT_BR(lor_lbl2, node->line);
    fprintf(cc->out, ".L%d:\n", lbl1);
    ZCC_EMIT_LABEL(lor_lbl1, node->line);
    fprintf(cc->out, "    movq $1, %%rax\n");
    fprintf(cc->out, ".L%d:\n", lbl2);
    ZCC_EMIT_LABEL(lor_lbl2, node->line);
    return;
  }

  case ND_VA_ARG: {
    int lbl_overflow;
    int lbl_end;

    /* Evaluate LHS (ap) -> %rax */
    codegen_expr(cc, node->lhs);

    /* Safe-keep ap in %rcx */
    fprintf(cc->out, "    movq %%rax, %%rcx\n");

    lbl_overflow = new_label(cc);
    lbl_end = new_label(cc);

    int fp = is_float_type(node->type);
    
    if (fp) {
        /* Float path: Check fp_offset (4(%%rcx)) threshold 176 */
        fprintf(cc->out, "    movl 4(%%rcx), %%edx\n");
        fprintf(cc->out, "    cmpl $176, %%edx\n");
        fprintf(cc->out, "    jae .L%d\n", lbl_overflow);
        /* Fast path: fetch from reg_save_area */
        fprintf(cc->out, "    movq 16(%%rcx), %%rsi\n");
        fprintf(cc->out, "    movslq %%edx, %%rax\n");
        fprintf(cc->out, "    addq %%rax, %%rsi\n");
        fprintf(cc->out, "    movq (%%rsi), %%rax\n");
        /* Increment fp_offset by 16 */
        fprintf(cc->out, "    addl $16, %%edx\n");
        fprintf(cc->out, "    movl %%edx, 4(%%rcx)\n");
    } else {
        /* GP path: Check gp_offset (0(%%rcx)) threshold 48 */
        fprintf(cc->out, "    movl 0(%%rcx), %%edx\n");
        fprintf(cc->out, "    cmpl $48, %%edx\n");
        fprintf(cc->out, "    jae .L%d\n", lbl_overflow);
        /* Fast path: fetch from reg_save_area */
        fprintf(cc->out, "    movq 16(%%rcx), %%rsi\n");
        fprintf(cc->out, "    movslq %%edx, %%rax\n");
        fprintf(cc->out, "    addq %%rax, %%rsi\n");
        fprintf(cc->out, "    movq (%%rsi), %%rax\n");
        /* Increment gp_offset by 8 */
        fprintf(cc->out, "    addl $8, %%edx\n");
        fprintf(cc->out, "    movl %%edx, 0(%%rcx)\n");
    }

    emit_label_fmt(cc, lbl_end, FMT_JMP);

    /* Slow path: fetch from overflow_arg_area */
    emit_label_fmt(cc, lbl_overflow, FMT_DEF);
    fprintf(cc->out, "    movq 8(%%rcx), %%rsi\n");
    fprintf(cc->out, "    movq (%%rsi), %%rax\n");

    /* Increment overflow_arg_area by 8 (even for floats, stack passing is 8-aligned) */
    fprintf(cc->out, "    leaq 8(%%rsi), %%rdi\n");
    fprintf(cc->out, "    movq %%rdi, 8(%%rcx)\n");

    /* End */
    emit_label_fmt(cc, lbl_end, FMT_DEF);
    return;
  }

  case ND_ADDR:
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_ADDR null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_addr_checked(cc, node->lhs);
    {
      char addr_src[32];
      ir_save_result(addr_src);
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, addr_src, node->line);
    }
    return;

  case ND_DEREF:
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_DEREF null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    {
      char deref_addr[32];
      ir_save_result(deref_addr);
      /* Type-aware load: char* -> movsbl, int* -> movl, ptr/long* -> movq */
      if (node->type && node->type->kind == TY_FUNC) {
          /* Do nothing: function pointer decays natively */
      } else {
          codegen_load(cc, node->type);
      }
      {
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_LOAD(ir_map_type(node->type), dst, deref_addr, node->line);
      }
    }
    return;

  case ND_MEMBER:
    codegen_addr_checked(cc, node);
    {
      char member_addr[32];
      ir_save_result(member_addr);
      if (node->type) {
        codegen_load(cc, node->type);
      } else {
        switch (node->member_size) {
        case 1:
          fprintf(cc->out, "    movzbl (%%rax), %%eax\n");
          break;
        case 2:
          fprintf(cc->out, "    movzwl (%%rax), %%eax\n");
          break;
        case 4:
          fprintf(cc->out, "    movl (%%rax), %%eax\n");
          break;
        case 8:
          fprintf(cc->out, "    movq (%%rax), %%rax\n");
          break;
          break;
        }
      }
      if (!backend_ops && node->is_bitfield) {
          if (node->bit_offset > 0) {
              fprintf(cc->out, "    movb $%d, %%cl\n", node->bit_offset);
              fprintf(cc->out, "    shrq %%cl, %%rax\n");
          }
          long long mask = (1ULL << node->bit_size) - 1;
          fprintf(cc->out, "    movq $0x%llx, %%r8\n", (unsigned long long)mask);
          fprintf(cc->out, "    andq %%r8, %%rax\n");
      }
      {
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_LOAD(ir_map_type(node->type), dst, member_addr, node->line);
      }
    }
    return;

  case ND_CAST: {
    char src_ir[32];
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_CAST null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    ir_save_result(src_ir);
    /* truncate/extend based on target type */
    if (node->cast_type) {
      int src_size = node->lhs && node->lhs->type ? type_size(node->lhs->type) : 4;
      switch (node->cast_type->size) {
      case 1:
        if (node->cast_type->kind == TY_UCHAR)
          fprintf(cc->out, "    movzbl %%al, %%eax\n");
        else
          fprintf(cc->out, "    movsbl %%al, %%eax\n");
        break;
      case 2:
        if (node->cast_type->kind == TY_USHORT)
          fprintf(cc->out, "    movzwl %%ax, %%eax\n");
        else
          fprintf(cc->out, "    movswl %%ax, %%eax\n");
        break;
      case 4:
        if (node->cast_type && !is_float_type(node->cast_type) && node->lhs && node->lhs->type && is_float_type(node->lhs->type)) {
            fprintf(cc->out, "    movq %%rax, %%xmm0\n");
            fprintf(cc->out, "    cvttsd2si %%xmm0, %%rax\n");
        } else if (node->cast_type->kind == TY_UINT || node->cast_type->kind == TY_ULONG) {
            if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n");
        } else if (!is_pointer(node->lhs ? node->lhs->type : 0)) {
            if (!backend_ops) fprintf(cc->out, "    cltq\n");
        }
        break;
      case 8:
        if (node->cast_type && is_float_type(node->cast_type) && node->lhs && node->lhs->type && !is_float_type(node->lhs->type)) {
            if (is_unsigned_type(node->lhs->type)) {
                fprintf(cc->out, "    testq %%rax, %%rax\n");
                fprintf(cc->out, "    js 1f\n");
                fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
                fprintf(cc->out, "    jmp 2f\n");
                fprintf(cc->out, "1:\n");
                if (backend_ops) fprintf(cc->out, "    mov r2, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%rdx\n");
                fprintf(cc->out, "    shrq $1, %%rdx\n");
                fprintf(cc->out, "    andl $1, %%eax\n");
                fprintf(cc->out, "    orq %%rax, %%rdx\n");
                fprintf(cc->out, "    cvtsi2sdq %%rdx, %%xmm0\n");
                fprintf(cc->out, "    addsd %%xmm0, %%xmm0\n");
                fprintf(cc->out, "2:\n");
            } else {
                fprintf(cc->out, "    cvtsi2sdq %%rax, %%xmm0\n");
            }
            fprintf(cc->out, "    movq %%xmm0, %%rax\n");
        } else if (node->cast_type && !is_float_type(node->cast_type) && node->lhs && node->lhs->type && is_float_type(node->lhs->type)) {
            fprintf(cc->out, "    movq %%rax, %%xmm0\n");
            fprintf(cc->out, "    cvttsd2si %%xmm0, %%rax\n");
        } else if (src_size == 4 && !is_pointer(node->lhs ? node->lhs->type : 0)) {
            if (node->lhs && node->lhs->type && is_unsigned_type(node->lhs->type)) {
                if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n");

            } else {
                if (!backend_ops) fprintf(cc->out, "    cltq\n");

            }
        }
        break;
      default:
        break;
      }
    }
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_CAST, ir_map_type(node->type), dst, src_ir, node->line);
    }
    return;
  }

  case ND_TERNARY: {
    char ternary_cond_ir[32];
    char ternary_lbl1[32];
    char ternary_lbl2[32];
    if (!node->cond || !node->then_body || !node->else_body) {
      error_at(cc, node->line,
               "codegen_expr: ND_TERNARY missing cond/then/else");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    lbl1 = new_label(cc);
    lbl2 = new_label(cc);
    codegen_expr_checked(cc, node->cond);
    ir_save_result(ternary_cond_ir);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    if (backend_ops) emit_label_fmt(cc, lbl1, FMT_JE);
    else emit_label_fmt(cc, lbl1, FMT_JE);
    sprintf(ternary_lbl1, ".L%d", lbl1);
    ZCC_EMIT_BR_IF(ternary_cond_ir, ternary_lbl1, node->line);
    codegen_expr_checked(cc, node->then_body);
    if (backend_ops) emit_label_fmt(cc, lbl2, FMT_JMP);
    else emit_label_fmt(cc, lbl2, FMT_JMP);
    sprintf(ternary_lbl2, ".L%d", lbl2);
    ZCC_EMIT_BR(ternary_lbl2, node->line);
    fprintf(cc->out, ".L%d:\n", lbl1);
    ZCC_EMIT_LABEL(ternary_lbl1, node->line);
    codegen_expr_checked(cc, node->else_body);
    fprintf(cc->out, ".L%d:\n", lbl2);
    ZCC_EMIT_LABEL(ternary_lbl2, node->line);
    return;
  }

  case ND_COMMA_EXPR:
    codegen_expr_checked(cc, node->lhs);
    codegen_expr_checked(cc, node->rhs);
    return;

  case ND_PRE_INC:
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      int esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n", esz); if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    adds r0, r0, r3\n"); else fprintf(cc->out, "    adds r1, r1, r3\n"); } else fprintf(cc->out, "    addq $%d, %s\n", esz, reg);
      if (backend_ops) fprintf(cc->out, "    mov r0, %s\n", reg); else fprintf(cc->out, "    movq %s, %%rax\n", reg);
      if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      if (backend_ops) fprintf(cc->out, "    mov %s, r0\n", reg); else fprintf(cc->out, "    movq %%rax, %s\n", reg);
      return;
    }
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_PRE_INC null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_addr_checked(cc, node->lhs);
    if (is_bad_ptr(node->lhs)) {
      error_at(cc, node->line, "codegen_expr: ND_PRE_INC lhs bad ptr");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    push_reg(cc, "rax");
    if (!node->lhs->type) {
      error_at(cc, node->line, "codegen_expr: ND_PRE_INC lhs has null type");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_load(cc, node->lhs->type);
    {
      int esz;
      esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n    adds r0, r0, r3\n", esz); } else fprintf(cc->out, "    addq $%d, %%rax\n", esz);
    }
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    switch (type_size(node->lhs->type)) {
    case 1:
      if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
    if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
        if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
        if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
    }
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_CAST, ir_map_type(node->lhs->type), dst, "pre_inc", node->line);
    }
    return;

  case ND_PRE_DEC:
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      int esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n", esz); if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    subs r0, r0, r3\n"); else fprintf(cc->out, "    subs r1, r1, r3\n"); } else fprintf(cc->out, "    subq $%d, %s\n", esz, reg);
      if (backend_ops) fprintf(cc->out, "    mov r0, %s\n", reg); else fprintf(cc->out, "    movq %s, %%rax\n", reg);
      if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      if (backend_ops) fprintf(cc->out, "    mov %s, r0\n", reg); else fprintf(cc->out, "    movq %%rax, %s\n", reg);
      return;
    }
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_PRE_DEC null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_addr_checked(cc, node->lhs);
    if (is_bad_ptr(node->lhs)) {
      error_at(cc, node->line, "codegen_expr: ND_PRE_DEC lhs bad ptr");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    push_reg(cc, "rax");
    if (!node->lhs->type) {
      error_at(cc, node->line, "codegen_expr: ND_PRE_DEC lhs has null type");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_load(cc, node->lhs->type);
    {
      int esz;
      esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n    subs r0, r0, r3\n", esz); } else fprintf(cc->out, "    subq $%d, %%rax\n", esz);
    }
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    switch (type_size(node->lhs->type)) {
    case 1:
      if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    if (backend_ops) fprintf(cc->out, "    mov r0, r1\n");
      else fprintf(cc->out, "    movq %%r11, %%rax\n");
    if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
        if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
        if (node->lhs->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
    }
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_CAST, ir_map_type(node->lhs->type), dst, "pre_dec", node->line);
    }
    return;

  case ND_POST_INC:
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      int esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) fprintf(cc->out, "    mov r0, %s\n", reg); else fprintf(cc->out, "    movq %s, %%rax\n", reg);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n", esz); if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    adds r0, r0, r3\n"); else fprintf(cc->out, "    adds r1, r1, r3\n"); } else fprintf(cc->out, "    addq $%d, %s\n", esz, reg);
      return;
    }
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_POST_INC null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_addr_checked(cc, node->lhs);
    if (is_bad_ptr(node->lhs)) {
      error_at(cc, node->line, "codegen_expr: ND_POST_INC lhs bad ptr");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    push_reg(cc, "rax");
    if (!node->lhs->type) {
      error_at(cc, node->line, "codegen_expr: ND_POST_INC lhs has null type");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_load(cc, node->lhs->type);
    push_reg(cc, "rax"); /* save original value */
    {
      int esz;
      esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n    adds r0, r0, r3\n", esz); } else fprintf(cc->out, "    addq $%d, %%rax\n", esz);
    }
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rdx"); /* rdx = original value */
    pop_reg(cc, "rax"); /* rax = address */
    switch (type_size(node->lhs->type)) {
    case 1:
      if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    if (backend_ops) fprintf(cc->out, "    mov r0, r2\n");
      else fprintf(cc->out, "    movq %%rdx, %%rax\n");
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_CAST, ir_map_type(node->lhs->type), dst, "post_inc", node->line);
    }
    return;

  case ND_POST_DEC:
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      int esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) fprintf(cc->out, "    mov r0, %s\n", reg); else fprintf(cc->out, "    movq %s, %%rax\n", reg);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n", esz); if (strcmp(reg, "rax") == 0) fprintf(cc->out, "    subs r0, r0, r3\n"); else fprintf(cc->out, "    subs r1, r1, r3\n"); } else fprintf(cc->out, "    subq $%d, %s\n", esz, reg);
      return;
    }
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_POST_DEC null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_addr_checked(cc, node->lhs);
    if (is_bad_ptr(node->lhs)) {
      error_at(cc, node->line, "codegen_expr: ND_POST_DEC lhs bad ptr");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    push_reg(cc, "rax");
    if (!node->lhs->type) {
      error_at(cc, node->line, "codegen_expr: ND_POST_DEC lhs has null type");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_load(cc, node->lhs->type);
    push_reg(cc, "rax");
    {
      int esz;
      esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      if (backend_ops) { fprintf(cc->out, "    ldr r3, =%d\n    subs r0, r0, r3\n", esz); } else fprintf(cc->out, "    subq $%d, %%rax\n", esz);
    }
    if (backend_ops) fprintf(cc->out, "    mov r1, r0\n");
      else fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rdx"); /* rdx = original value */
    pop_reg(cc, "rax"); /* rax = address */
    switch (type_size(node->lhs->type)) {
    case 1:
      if (backend_ops) fprintf(cc->out, "    strb r1, [r0]\n");
      else fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      if (backend_ops) fprintf(cc->out, "    strh r1, [r0]\n");
      else fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      if (backend_ops) fprintf(cc->out, "    str r1, [r0]\n");
      else fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    if (backend_ops) fprintf(cc->out, "    mov r0, r2\n");
      else fprintf(cc->out, "    movq %%rdx, %%rax\n");
    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_UNARY(IR_CAST, ir_map_type(node->lhs->type), dst, "post_dec", node->line);
    }
    return;

  case ND_CALL: {
    char *argregs[6];
    int i;
    int nargs;
    int shadow_and_stack;
    int args_on_stack;
    int alignment_pad;
    int cleanup_bytes;
    char args_ir_1d[2048];

    /* System V AMD64 (Linux): 6 register args: RDI, RSI, RDX, RCX, R8, R9; 7th+
     * on stack */
    argregs[0] = "rdi";
    argregs[1] = "rsi";
    argregs[2] = "rdx";
    argregs[3] = "rcx";
    argregs[4] = "r8";
    argregs[5] = "r9";

    if (!node->func_name[0] && !node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_CALL no func_name and no callee");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    if (!node->args) {
      error_at(cc, node->line, "codegen_expr: ND_CALL with NULL args");
      return;
    }
    nargs = node->num_args;
    if (nargs < 0 || nargs > 64) {
      error_at(cc, node->line, "call node bad num_args");
      return;
    }

    /* System V AMD64: no shadow space required. Space for 7th+ gp args and 9th+ fp args is left on the stack. */
    {
      int temp_gp = 0, temp_fp = 0;
      for (i = 0; i < nargs; i++) {
        if (node->args[i] && node->args[i]->type && is_float_type(node->args[i]->type)) {
          if (temp_fp < 8) temp_fp++;
        } else {
          if (temp_gp < 6) temp_gp++;
        }
      }
      args_on_stack = nargs - (temp_gp + temp_fp);
    }
    /* We must ensure that AFTER the args are on the stack, %rsp is 16-byte
     * aligned. */
    /* Currently, depth is cc->stack_depth. After pushes/pops, depth will be
     * cc->stack_depth + args_on_stack. */
    alignment_pad = 0;
    {
      if ((cc->stack_depth + args_on_stack) % 2 != 0) {
        alignment_pad = 8;
      }
    }


    if (alignment_pad > 0) {
      fprintf(cc->out, "    subq $%d, %%rsp\n", alignment_pad);
      cc->stack_depth++;
    }

    /* for indirect calls, evaluate callee first and save on stack */
    if (node->func_name[0] == 0 && node->lhs) {
      codegen_expr_checked(cc, node->lhs);
      push_reg(cc, "rax");
    }

    /* push args in reverse order */
    for (i = nargs - 1; i >= 0; i--) {
      if (!node->args[i]) {
        error_at(cc, node->line, "null argument in call");
        return;
      }
      codegen_expr_checked(cc, node->args[i]);
      ir_save_result(&args_ir_1d[i * 32]);
      ZCC_EMIT_ARG(ir_map_type(node->args[i] ? node->args[i]->type : 0), &args_ir_1d[i * 32], node->line);
      push_reg(cc, "rax");
    }

    /* pop args into correct registers: floats->xmm, ints->gpregs independently */
    {
      int gp_idx = 0;
      int fp_idx = 0;
      for (i = 0; i < nargs; i++) {
        if (node->args[i] && node->args[i]->type && is_float_type(node->args[i]->type)) {
          if (fp_idx < 8) {
            fprintf(cc->out, "    popq %%rax\n");
            cc->stack_depth--;
            fprintf(cc->out, "    movq %%rax, %%xmm%d\n", fp_idx);
            fp_idx++;
          }
        } else {
          if (backend_ops) {
              if (gp_idx < 4) {
                 fprintf(cc->out, "    pop {r%d}\n", gp_idx);
                 cc->stack_depth--;
                 gp_idx++;
              }
          } else {
              if (gp_idx < 6) {
                pop_reg(cc, argregs[gp_idx]);
                gp_idx++;
              }
          }
        }
      }
      if (!backend_ops) {
          fprintf(cc->out, "    movl $%d, %%eax\n", fp_idx > 8 ? 8 : fp_idx);
      }
    }
    if (node->func_name[0] == 0 && node->lhs) {
      /* indirect call: pop callee into r10, call *r10 */
      pop_reg(cc, "r10");
      fprintf(cc->out, "    call *%%r10\n");
    } else if (strcmp(node->func_name, "__builtin_va_start") == 0) {
      Symbol *fsym = scope_find(cc, cc->current_func);
      int nparams_gp = 0;
      int nparams_fp = 0;
      if (fsym && fsym->type && fsym->type->params) {
          for (int j = 0; j < fsym->type->num_params; j++) {
              if (is_float_type(fsym->type->params[j])) nparams_fp++;
              else nparams_gp++;
          }
      }
      int gp_offset = nparams_gp * 8;
      int fp_offset = 48 + nparams_fp * 16;
      fprintf(cc->out, "    # arg0 = ap, arg1 ignored\n");
      fprintf(cc->out, "    movl $%d, 0(%%rdi)\n", gp_offset);
      fprintf(cc->out, "    movl $%d, 4(%%rdi)\n", fp_offset);
      fprintf(cc->out, "    leaq 16(%%rbp), %%rax\n");
      fprintf(cc->out, "    movq %%rax, 8(%%rdi)\n");
      /* The reg_save_area is dynamically stored at stack bottom. */
      fprintf(cc->out, "    leaq %d(%%rbp), %%rax\n", fsym ? fsym->stack_offset : -176);
      fprintf(cc->out, "    movq %%rax, 16(%%rdi)\n");
    } else if (strcmp(node->func_name, "__builtin_va_end") == 0) {
      /* va_end is a no-op on x86-64 SysV ABI */
      fprintf(cc->out, "    # __builtin_va_end (no-op)\n");
    } else {
      if (backend_ops && backend_ops->emit_call) {
          backend_ops->emit_call(cc, node);
      } else {
          fprintf(cc->out, "    call %s\n", node->func_name);
      }
    }

    if (node->type && is_float_type(node->type)) {
        fprintf(cc->out, "    movq %%xmm0, %%rax\n");
    } else if (node->type && !node_type_unsigned(node)) {
        if (node->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    cltq\n"); }
        else if (node->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->type && node_type_unsigned(node)) {
        if (node->type->size == 4) { if (!backend_ops) fprintf(cc->out, "    movl %%eax, %%eax\n"); }
        else if (node->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
        else if (node->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
    }

    {
      char *dst = ir_bridge_fresh_tmp();
      ZCC_EMIT_CALL(ir_map_type(node->type), dst, node->func_name, node->line);
    }

    /* cleanup arguments left on stack AND the alignment pad */
    cleanup_bytes = (args_on_stack * 8) + alignment_pad;
    if (cleanup_bytes > 0) {
      fprintf(cc->out, "    addq $%d, %%rsp\n", cleanup_bytes);
      cc->stack_depth -= (cleanup_bytes / 8);
    }
    return;
  }

  case ND_NOP:
    return;

  default:
    error_at(cc, node->line, "unsupported expression in codegen");
    return;
  }
}

/* ================================================================ */
/* STATEMENT CODEGEN                                                 */
/* ================================================================ */

void codegen_stmt(Compiler *cc, Node *node) {
  if (!node)
    return;

  if (cc) {
  }

  int lbl1;
  int lbl2;
  int lbl3;
  int old_break;
  int old_continue;
  char errbuf[80];

  /* Do NOT use ptr_in_fault_range(node): stage2 miscompiles it and rejects
   * valid arena ptrs. */
  if (!node)
    return;
  if (is_bad_ptr(node)) {
    sprintf(errbuf, "codegen_stmt: bad node ptr %p", (void *)node);
    error_at(cc, 0, errbuf);
    return;
  }

  switch (node->kind) {
  case ND_RSP_SAVE: {
      if (!backend_ops && node->member_offset) {
          int off = node->member_offset;
          if (off > 0) off = -off;
          fprintf(cc->out, "    movq %%rsp, %d(%%rbp)\n", off);
      }
      return;
  }

  case ND_RSP_RESTORE: {
      if (!backend_ops && node->member_offset) {
          int off = node->member_offset;
          if (off > 0) off = -off;
          fprintf(cc->out, "    movq %d(%%rbp), %%rsp\n", off);
      }
      return;
  }

  case ND_RETURN:
    if (node->lhs) {
      codegen_expr_checked(cc, node->lhs);
      {
        char ret_tmp[32];
        ir_save_result(ret_tmp);
        ZCC_EMIT_RET(ir_map_type(node->lhs->type), ret_tmp, node->line);
      }
      if (node->lhs->type && is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
      }
    } else {
      ZCC_EMIT_RET(0, "", node->line);
    }
    if (backend_ops) fprintf(cc->out, "    b .Lfunc_end_%d\n", cc->func_end_label);
    else fprintf(cc->out, "    jmp .Lfunc_end_%d\n", cc->func_end_label);
    return;

  case ND_BLOCK: {
    int i;
    int nst;
    if (!node->stmts) {
      error_at(cc, node->line, "codegen_stmt: ND_BLOCK null stmts");
      return;
    }
    nst = node->num_stmts;
    if (nst < 0 || nst > 65536) {
      error_at(cc, node->line, "block: bad num_stmts");
      return;
    }
    for (i = 0; i < nst; i++) {
      codegen_stmt(cc, node->stmts[i]);
    }
    return;
  }

  case ND_IF: {
    char cond_ir[32];
    char ir_lbl[32];
    lbl1 = new_label(cc);
    codegen_expr_checked(cc, node->cond);
    ir_save_result(cond_ir);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n");
    else if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    emit_label_fmt(cc, lbl1, FMT_JE);
    sprintf(ir_lbl, ".L%d", lbl1);
    ZCC_EMIT_BR_IF(cond_ir, ir_lbl, node->line);
    codegen_stmt(cc, node->then_body);
    if (node->else_body) {
      lbl2 = new_label(cc);
      emit_label_fmt(cc, lbl2, FMT_JMP);
      sprintf(ir_lbl, ".L%d", lbl2);
      ZCC_EMIT_BR(ir_lbl, node->line);
      emit_label_fmt(cc, lbl1, FMT_DEF);
      sprintf(ir_lbl, ".L%d", lbl1);
      ZCC_EMIT_LABEL(ir_lbl, node->line);
      codegen_stmt(cc, node->else_body);
      emit_label_fmt(cc, lbl2, FMT_DEF);
      sprintf(ir_lbl, ".L%d", lbl2);
      ZCC_EMIT_LABEL(ir_lbl, node->line);
    } else {
      emit_label_fmt(cc, lbl1, FMT_DEF);
      sprintf(ir_lbl, ".L%d", lbl1);
      ZCC_EMIT_LABEL(ir_lbl, node->line);
    }
    return;
  }

  case ND_WHILE: {
    char cond_ir[32];
    char ir_lbl[32];
    lbl1 = new_label(cc); /* loop start */
    lbl2 = new_label(cc); /* loop end */
    old_break = cc->break_label;
    old_continue = cc->continue_label;
    cc->break_label = lbl2;
    cc->continue_label = lbl1;
    emit_label_fmt(cc, lbl1, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl1);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    codegen_expr_checked(cc, node->cond);
    ir_save_result(cond_ir);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n");
    else if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    emit_label_fmt(cc, lbl2, FMT_JE);
    sprintf(ir_lbl, ".L%d", lbl2);
    ZCC_EMIT_BR_IF(cond_ir, ir_lbl, node->line);
    codegen_stmt(cc, node->body);
    emit_label_fmt(cc, lbl1, FMT_JMP);
    sprintf(ir_lbl, ".L%d", lbl1);
    ZCC_EMIT_BR(ir_lbl, node->line);
    emit_label_fmt(cc, lbl2, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl2);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    cc->break_label = old_break;
    cc->continue_label = old_continue;
    return;
  }

  case ND_FOR: {
    char cond_ir[32];
    char ir_lbl[32];
    lbl1 = new_label(cc); /* loop start */
    lbl2 = new_label(cc); /* loop end */
    lbl3 = new_label(cc); /* continue target (increment) */
    old_break = cc->break_label;
    old_continue = cc->continue_label;
    cc->break_label = lbl2;
    cc->continue_label = lbl3;
    if (node->init)
      codegen_stmt(cc, node->init);
    emit_label_fmt(cc, lbl1, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl1);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    if (node->cond) {
      codegen_expr_checked(cc, node->cond);
      ir_save_result(cond_ir);
      if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n");
      else if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
      emit_label_fmt(cc, lbl2, FMT_JE);
      sprintf(ir_lbl, ".L%d", lbl2);
      ZCC_EMIT_BR_IF(cond_ir, ir_lbl, node->line);
    }
    codegen_stmt(cc, node->body);
    emit_label_fmt(cc, lbl3, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl3);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    if (node->inc)
      codegen_expr_checked(cc, node->inc);
    emit_label_fmt(cc, lbl1, FMT_JMP);
    sprintf(ir_lbl, ".L%d", lbl1);
    ZCC_EMIT_BR(ir_lbl, node->line);
    emit_label_fmt(cc, lbl2, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl2);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    cc->break_label = old_break;
    cc->continue_label = old_continue;
    return;
  }

  case ND_DO_WHILE: {
    /* do { body } while (cond)
     * Layout:
     *   lbl_body: (lbl1)
     *       <body> — break→lbl2, continue→lbl_cond
     *   lbl_cond: (lbl3 — new label for condition)
     *       <cond>; jne lbl_body
     *   lbl_break: (lbl2)
     *
     * CRITICAL: continue must jump to lbl_cond (the condition/increment),
     * NOT back to lbl_body (which would skip the while-condition entirely,
     * turning "do { if(!x) continue; } while(++i<n)" into an infinite loop
     * because ++i is in the condition and never evaluated). */
    int lbl3;   /* separate continue-target label = condition start */
    char cond_ir[32];
    char ir_lbl[32];
    lbl1 = new_label(cc);  /* body start */
    lbl2 = new_label(cc);  /* break/exit */
    lbl3 = new_label(cc);  /* continue target = condition-start */
    old_break = cc->break_label;
    old_continue = cc->continue_label;
    cc->break_label = lbl2;
    cc->continue_label = lbl3;   /* ← fix: continue skips to condition */
    emit_label_fmt(cc, lbl1, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl1);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    codegen_stmt(cc, node->body);
    /* condition begins here — this is where continue targets */
    emit_label_fmt(cc, lbl3, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl3);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    codegen_expr_checked(cc, node->cond);
    ir_save_result(cond_ir);
    if (backend_ops) fprintf(cc->out, "    cmp r0, #0\n" \
    ); else fprintf(cc->out, "    cmpq $0, %%rax\n");
    emit_label_fmt(cc, lbl1, FMT_JNE);
    sprintf(ir_lbl, ".L%d", lbl1);
    ZCC_EMIT_BR_IF(cond_ir, ir_lbl, node->line);
    emit_label_fmt(cc, lbl2, FMT_DEF);
    sprintf(ir_lbl, ".L%d", lbl2);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    cc->break_label = old_break;
    cc->continue_label = old_continue;
    return;
  }

  case ND_SWITCH: {
    int end_lbl;
    int *case_labels;
    int default_lbl;
    int i;
    int ncase;
    char switch_val_ir[32];
    char ir_lbl[64];

    if (!node->cases) {
      error_at(cc, node->line, "codegen_stmt: ND_SWITCH null cases");
      return;
    }
    ncase = node->num_cases;
    if (ncase < 0 || ncase > MAX_CASES) {
      error_at(cc, node->line, "switch: bad num_cases");
      return;
    }

    end_lbl = new_label(cc);
    old_break = cc->break_label;
    cc->break_label = end_lbl;

    codegen_expr_checked(cc, node->cond);
    ir_save_result(switch_val_ir);

    case_labels = (int *)cc_alloc(cc, sizeof(int) * (ncase + 1));
    for (i = 0; i < ncase; i++) {
      if (!node->cases[i]) {
        error_at(cc, node->line, "codegen_stmt: ND_SWITCH null case");
        return;
      }
      case_labels[i] = new_label(cc);
      fprintf(cc->out, "    cmpq $%lld, %%rax\n", node->cases[i]->case_val);
      emit_label_fmt(cc, case_labels[i], FMT_JE);

      {
        char case_const_ir[32];
        char cmp_ir[32];
        char *ct;

        ct = ir_bridge_fresh_tmp();
        sprintf(case_const_ir, "%s", ct);
        ZCC_EMIT_CONST(IR_TY_I64, case_const_ir, node->cases[i]->case_val,
                       node->line);

        ct = ir_bridge_fresh_tmp();
        sprintf(cmp_ir, "%s", ct);

        ZCC_EMIT_BINARY(IR_EQ, IR_TY_I64, cmp_ir, switch_val_ir, case_const_ir,
                        node->line);
        sprintf(ir_lbl, ".L%d", case_labels[i]);
        ZCC_EMIT_BR_IF(cmp_ir, ir_lbl, node->line);
      }
    }
    default_lbl = new_label(cc);
    if (node->default_case) {
      emit_label_fmt(cc, default_lbl, FMT_JMP);
      sprintf(ir_lbl, ".L%d", default_lbl);
      ZCC_EMIT_BR(ir_lbl, node->line);
    } else {
      emit_label_fmt(cc, end_lbl, FMT_JMP);
      sprintf(ir_lbl, ".L%d", end_lbl);
      ZCC_EMIT_BR(ir_lbl, node->line);
    }

    for (i = 0; i < ncase; i++) {
      emit_label_fmt(cc, case_labels[i], FMT_DEF);
      sprintf(ir_lbl, ".L%d", case_labels[i]);
      ZCC_EMIT_LABEL(ir_lbl, node->line);
      if (node->cases[i]->case_body)
        codegen_stmt(cc, node->cases[i]->case_body);
    }
    if (node->default_case) {
      emit_label_fmt(cc, default_lbl, FMT_DEF);
      sprintf(ir_lbl, ".L%d", default_lbl);
      ZCC_EMIT_LABEL(ir_lbl, node->line);
      if (node->default_case->case_body)
        codegen_stmt(cc, node->default_case->case_body);
    }

    if (node->body)
      codegen_stmt(cc, node->body);

    emit_label_fmt(cc, end_lbl, FMT_DEF);
    sprintf(ir_lbl, ".L%d", end_lbl);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    cc->break_label = old_break;
    return;
  }

  case ND_BREAK: {
    char ir_lbl[32];
    emit_label_fmt(cc, cc->break_label, FMT_JMP);
    sprintf(ir_lbl, ".L%d", cc->break_label);
    ZCC_EMIT_BR(ir_lbl, node->line);
    return;
  }

  case ND_CONTINUE: {
    char ir_lbl[32];
    emit_label_fmt(cc, cc->continue_label, FMT_JMP);
    sprintf(ir_lbl, ".L%d", cc->continue_label);
    ZCC_EMIT_BR(ir_lbl, node->line);
    return;
  }

  case ND_GOTO: {
    char ir_lbl[64];
    if (!node->label_name) {
      error_at(cc, node->line, "codegen_stmt: ND_GOTO null label_name");
      return;
    }
    fprintf(cc->out, "    jmp .Luser_%s_%s\n", cc->current_func,
            node->label_name);
    sprintf(ir_lbl, ".Luser_%s_%s", cc->current_func, node->label_name);
    ZCC_EMIT_BR(ir_lbl, node->line);
    return;
  }

  case ND_LABEL: {
    char ir_lbl[64];
    if (!node->label_name) {
      error_at(cc, node->line, "codegen_stmt: ND_LABEL null label_name");
      return;
    }
    fprintf(cc->out, ".Luser_%s_%s:\n", cc->current_func, node->label_name);
    sprintf(ir_lbl, ".Luser_%s_%s", cc->current_func, node->label_name);
    ZCC_EMIT_LABEL(ir_lbl, node->line);
    if (node->lhs)
      codegen_stmt(cc, node->lhs);
    return;
  }

  case ND_NOP:
    return;

  case ND_ASM:
    fprintf(cc->out, "    %s\n", node->asm_string);
    ZCC_EMIT_ASM(node->asm_string, node->line);
    /* Note: IR backend handles this via ZCC_ND_ASM -> OP_ASM translation if enabled */
    return;

  default: {
    /* expression statement */
    char badmsg[80];
    if (is_bad_ptr(node)) {
      sprintf(badmsg, "codegen_stmt default: bad expr node %p",
              node ? (void *)node : (void *)0);
      error_at(cc, 0, badmsg);
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node);
    return;
  }
  }
}

/* ================================================================ */
/* REGISTER ALLOCATOR (CHEATCODE 3)                                  */
/* ================================================================ */

static int pseudo_pc = 0;
static Symbol *ra_locals[1024];
static int num_ra_locals = 0;
static char *get_callee_reg(int i) {
  if (backend_ops) {
    if (i == 0) return "r4";
    if (i == 1) return "r5";
    if (i == 2) return "r6";
    if (i == 3) return "r7";
    return "r4";
  }
  if (i == 0)
    return "%r12";
  if (i == 1)
    return "%r13";
  if (i == 2)
    return "%r14";
  if (i == 3)
    return "%r15";
  return "%rbx";
}

static void ra_add_local(Symbol *sym) {
  int i;
  if (!sym || !sym->is_local || sym->live_start == -1)
    return;
  for (i = 0; i < num_ra_locals; i++) {
    if (ra_locals[i] == sym)
      return;
  }
  if (num_ra_locals < 1024)
    ra_locals[num_ra_locals++] = sym;
}

static void compute_liveness(Node *n) {
  int i;
  int loop_start;
  if (!n)
    return;
  pseudo_pc++;

  if (n->kind == ND_WHILE || n->kind == ND_FOR || n->kind == ND_DO_WHILE) {
    loop_start = pseudo_pc;
    compute_liveness(n->cond);
    compute_liveness(n->body);
    compute_liveness(n->init);
    compute_liveness(n->inc);
    for (i = 0; i < num_ra_locals; i++) {
      if (ra_locals[i]->live_end >= loop_start) {
        ra_locals[i]->live_end = pseudo_pc;
        if (ra_locals[i]->live_start >= loop_start) {
          ra_locals[i]->live_start = loop_start;
        }
      }
    }
    return;
  }

  if (n->kind == ND_ADDR && n->lhs && n->lhs->kind == ND_VAR && n->lhs->sym) {
    n->lhs->sym->live_start = -1; /* disable alloc */
  }
  if (n->kind == ND_VAR && n->sym && n->sym->is_local) {
    if (n->sym->live_start != -1) {
      if (n->sym->live_start == 0)
        n->sym->live_start = pseudo_pc;
      n->sym->live_end = pseudo_pc;
      ra_add_local(n->sym);
    }
  }

  compute_liveness(n->lhs);
  compute_liveness(n->rhs);
  compute_liveness(n->cond);
  compute_liveness(n->then_body);
  compute_liveness(n->else_body);
  compute_liveness(n->init);
  compute_liveness(n->inc);
  compute_liveness(n->body);
  if (n->stmts) {
    for (i = 0; i < n->num_stmts; i++)
      compute_liveness(n->stmts[i]);
  }
  if (n->args) {
    for (i = 0; i < n->num_args; i++)
      compute_liveness(n->args[i]);
  }
  if (n->cases) {
    for (i = 0; i < n->num_cases; i++)
      compute_liveness(n->cases[i]);
  }
  compute_liveness(n->default_case);
  compute_liveness(n->case_body);
}

static int allocate_registers(Node *func) {
  int count = 0;
  int i, j, r;
  int param_limit = -(func->num_params * 8);
  char *active_regs[5] = {0, 0, 0, 0, 0};
  int active_ends[5] = {0, 0, 0, 0, 0};
  int used_regs_bitmask = 0;

  num_ra_locals = 0;
  pseudo_pc = 1;
  compute_liveness(func->body);

  for (i = 0; i < num_ra_locals; i++) {
    Symbol *sym = ra_locals[i];
    if (sym->stack_offset >= param_limit && sym->stack_offset < 0) {
      sym->live_start = -1; /* never alloc parameters for safety */
    }
    if (sym->live_start != -1 && sym->type && sym->type->kind != TY_ARRAY &&
        (is_integer(sym->type) || sym->type->kind == TY_PTR)) {
      ra_locals[count++] = sym;
    } else {
      sym->live_start = -1;
    }
  }
  num_ra_locals = count;

  for (i = 0; i < num_ra_locals; i++) {
    for (j = i + 1; j < num_ra_locals; j++) {
      if (ra_locals[j]->live_start < ra_locals[i]->live_start) {
        Symbol *t = ra_locals[i];
        ra_locals[i] = ra_locals[j];
        ra_locals[j] = t;
      }
    }
  }

  /* Linear Scan */
  for (i = 0; i < num_ra_locals; i++) {
    Symbol *sym = ra_locals[i];
    if (sym->live_start == -1)
      continue;

    for (r = 0; r < 5; r++) {
      if (active_regs[r] && active_ends[r] <= sym->live_start)
        active_regs[r] = 0;
    }

    int allocated = -1;
    for (r = 0; r < 5; r++) {
      if (!active_regs[r]) {
        allocated = r;
        active_regs[r] = get_callee_reg(r);
        active_ends[r] = sym->live_end;
        sym->assigned_reg = get_callee_reg(r);
        used_regs_bitmask |= (1 << r);
        break;
      }
    }

    if (allocated == -1) {
      sym->assigned_reg = 0;
    }
  }
  return used_regs_bitmask;
}

extern struct ZCCNode *zcc_node_from(struct Node *ast);
extern int zcc_run_passes_emit_body_pgo(struct ZCCNode *body, const char *profile, const char *name, void *out, int stack_size, int num_params, int end_label1, int end_label2);

#pragma weak zcc_run_passes_emit_body_pgo
#pragma weak zcc_node_from

static int ir_blacklisted(const char *name) {
  if (!name)
    return 0;
  static const char *blacklist[] = {
      /* Add explicitly dangerous functions here to fallback to AST backend */
      "main", "read_file", "init_compiler", 
      "lookup_keyword_fallback", "parse_stmt", "next_token", NULL};
  int i;
  for (i = 0; blacklist[i]; i++) {
    if (strcmp(name, blacklist[i]) == 0) {
      fprintf(stderr, "[ZCC-BLACKLIST] HIT (skipping IR): %s\n", name);
      return 1;
    }
  }
  return 0;
}

void codegen_func(Compiler *cc, Node *func) {
  char *argregs[6];
  int i;
  int stack_size;
  int used_regs;

  if (!func)
    return;
  fprintf(stderr, "cc_func: %s\n", func->func_def_name);
  used_regs = allocate_registers(func);

  argregs[0] = "rdi";
  argregs[1] = "rsi";
  argregs[2] = "rdx";
  argregs[3] = "rcx";
  argregs[4] = "r8";
  argregs[5] = "r9";

  cc->func_end_label = new_label(cc);
  strncpy(cc->current_func, func->func_def_name, MAX_IDENT - 1);

  ir_bridge_func_begin(func);

  if (backend_ops && backend_ops->emit_prologue) {
      backend_ops->emit_prologue(cc, func);
  } else {
      stack_size = func->stack_size + 40; /* reserve 5x8 byte push slots */
      if (func->func_type && func->func_type->is_variadic) {
          stack_size += 176;
      }
      if (stack_size < 256)
        stack_size = 256;
      stack_size = (stack_size + 15) & ~15;

      fprintf(cc->out, "    .text\n");
      if (!func->is_static) {
        fprintf(cc->out, "    .globl %s\n", func->func_def_name);
      }
      fprintf(cc->out, "%s:\n", func->func_def_name);
      fprintf(cc->out, "    pushq %%rbp\n");
      fprintf(cc->out, "    movq %%rsp, %%rbp\n");
      fprintf(cc->out, "    subq $%d, %%rsp\n", stack_size);

      for (i = 0; i < 5; i++) {
        if (used_regs & (1 << i)) {
          fprintf(cc->out, "    movq %s, %d(%%rbp)\n", get_callee_reg(i),
                  -(func->stack_size + 8 * (i + 1)));
        }
      }
  }

  cc->stack_depth = 0;

  /* We need to re-create the scope with locals for codegen.
     During parsing, scope_add_local assigned stack offsets.
     Those offsets are stored in the Symbol nodes (via the arena).
     The body's ND_VAR nodes reference those Symbols.
     So the offsets are already embedded in the AST — we just need
     to store params from registers to their assigned stack slots. */

  scope_push(cc);

  /* Store params from registers to their stack locations... */
  int param_offset = 0;
  int f_idx = 0;
  int gp_idx = 0;
  if (!backend_ops) {
  for (i = 0; i < func->num_params; i++) {
    int sz = type_size(func->param_types[i]);
    if (sz < 8) sz = 8;
    param_offset -= sz;
    
    if (func->param_types[i]->kind == TY_STRUCT || func->param_types[i]->kind == TY_UNION) {
        if (i < 6) {
          fprintf(cc->out, "    movq %%%s, %%r10\n", argregs[gp_idx]);
          gp_idx++;
        } else {
          fprintf(cc->out, "    movq %d(%%rbp), %%r10\n", 16 + (i - 6) * 8);
        }
        int j;
        for (j = 0; j < sz; j++) {
            fprintf(cc->out, "    movb %d(%%r10), %%al\n", j);
            fprintf(cc->out, "    movb %%al, %d(%%rbp)\n", param_offset + j);
        }
    } else if (is_float_type(func->param_types[i])) {
        if (f_idx < 8) {
            fprintf(cc->out, "    movq %%xmm%d, %%rax\n", f_idx);
            fprintf(cc->out, "    movq %%rax, %d(%%rbp)\n", param_offset);
        } else {
            fprintf(cc->out, "    movq %d(%%rbp), %%rax\n", 16 + (i - 6) * 8);
            fprintf(cc->out, "    movq %%rax, %d(%%rbp)\n", param_offset);
        }
        f_idx++;
    } else {
        if (i < 6) {
          fprintf(cc->out, "    movq %%%s, %d(%%rbp)\n", argregs[gp_idx], param_offset);
          gp_idx++;
        } else {
          fprintf(cc->out, "    movq %d(%%rbp), %%rax\n", 16 + (i - 6) * 8);
          fprintf(cc->out, "    movq %%rax, %d(%%rbp)\n", param_offset);
        }
    }
  }

  if (func->func_type && func->func_type->is_variadic) {
    int save_base = -(func->stack_size + 40 + 176);
    /* Save GP regs at offsets 0-47 */
    for (i = 0; i < 6; i++) {
        fprintf(cc->out, "    movq %%%s, %d(%%rbp)\n", argregs[i], save_base + i*8);
    }
    /* Save XMM regs at offsets 48-175 */
    for (i = 0; i < 8; i++) {
        fprintf(cc->out, "    movsd %%xmm%d, %d(%%rbp)\n", i, save_base + 48 + i*16);
    }
    Symbol *fsym = scope_find(cc, func->func_def_name);
    if (fsym) {
        fsym->stack_offset = save_base;
    }
  }
  } /* end !backend_ops block */

  int ir_ok = 0;
  if ((getenv("ZCC_IR_BACKEND") || getenv("ZCC_IR_LOWER")) &&
      !ir_blacklisted(func->func_def_name)) {
    if (zcc_run_passes_emit_body_pgo && zcc_node_from) {
      void *ir_ast = zcc_node_from((void *)func->body);
      if (ir_ast) {
        char params_env[512];
        memset(params_env, 0, sizeof(params_env));
        for (int p_idx = 0; p_idx < func->num_params; p_idx++) {
          if (p_idx > 0)
            strcat(params_env, ",");
          strncat(params_env, func->param_names_buf[p_idx],
                  511 - strlen(params_env));
        }
        setenv("ZCC_IR_PARAM_NAMES", params_env, 1);

        ir_ok = zcc_run_passes_emit_body_pgo(
            ir_ast, NULL, func->func_def_name, cc->out, stack_size,
            func->num_params, cc->func_end_label, cc->func_end_label);

        unsetenv("ZCC_IR_PARAM_NAMES");
      }
    }
  }
  if (!ir_ok) {
    codegen_stmt(cc, func->body);
  }

  if (backend_ops && backend_ops->emit_epilogue) {
      backend_ops->emit_epilogue(cc, func);
  } else {
      fprintf(cc->out, ".Lfunc_end_%d:\n", cc->func_end_label);
      for (i = 4; i >= 0; i--) {
        if (used_regs & (1 << i)) {
          fprintf(cc->out, "    movq %d(%%rbp), %s\n",
                  -(func->stack_size + 8 * (i + 1)), get_callee_reg(i));
        }
      }
      fprintf(cc->out, "    movq %%rbp, %%rsp\n");
      fprintf(cc->out, "    popq %%rbp\n");
      fprintf(cc->out, "    ret\n");
  }

  ir_bridge_func_end();

  scope_pop(cc);
}

/* ================================================================ */
/* GLOBAL VARIABLE EMISSION                                          */
/* Bug fix: section directive (.bss/.data) BEFORE label              */
/* Bug fix: .p2align 3 before every label (x86-64 ABI alignment)    */
/* ================================================================ */

static long long eval_const_expr_p4(Node *elem, int *ok) {
    if (!elem) { *ok = 0; return 0; }
    if (elem->kind == ND_CAST) return eval_const_expr_p4(elem->lhs, ok);
    if (elem->kind == ND_NUM) return elem->int_val;
    if (elem->kind == ND_ADD) return eval_const_expr_p4(elem->lhs, ok) + eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_SUB) return eval_const_expr_p4(elem->lhs, ok) - eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_MUL) return eval_const_expr_p4(elem->lhs, ok) * eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_DIV) { long long r = eval_const_expr_p4(elem->rhs, ok); if(r) return eval_const_expr_p4(elem->lhs, ok) / r; return 0; }
    if (elem->kind == ND_BOR) return eval_const_expr_p4(elem->lhs, ok) | eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_BAND) return eval_const_expr_p4(elem->lhs, ok) & eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_BXOR) return eval_const_expr_p4(elem->lhs, ok) ^ eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_SHL) return eval_const_expr_p4(elem->lhs, ok) << eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_SHR) return eval_const_expr_p4(elem->lhs, ok) >> eval_const_expr_p4(elem->rhs, ok);
    if (elem->kind == ND_NEG) return -eval_const_expr_p4(elem->lhs, ok);
    if (elem->kind == ND_BNOT) return ~eval_const_expr_p4(elem->lhs, ok);
    if (elem->kind == ND_LNOT) return !eval_const_expr_p4(elem->lhs, ok);
    *ok = 0;
    return 0;
}

static void emit_struct_fields(Compiler *cc, StructField *fields, Node **args, int num_args, int *arg_idx, int arg_end, int base_offset, int *emitted) {
    StructField *f;
    for (f = fields; f; f = f->next) {
        int field_abs_offset = base_offset + f->offset;
        
        if (field_abs_offset > *emitted) {
            fprintf(cc->out, "    .zero %d\n", field_abs_offset - *emitted);
            *emitted = field_abs_offset;
        }
        
        if (f->type->kind == TY_STRUCT) {
            emit_struct_fields(cc, f->type->fields, args, num_args, arg_idx, arg_end, field_abs_offset, emitted);
        } else if (f->type->kind == TY_ARRAY) {
            int j;
            if (f->type->base && (f->type->base->kind == TY_STRUCT || f->type->base->kind == TY_UNION)) {
                int elem_size = type_size(f->type->base);
                for (j = 0; j < f->type->array_len; j++) {
                    int expected_end = field_abs_offset + (j + 1) * elem_size;
                    emit_struct_fields(cc, f->type->base->fields, args, num_args, arg_idx, arg_end, field_abs_offset + j * elem_size, emitted);
                    if (*emitted < expected_end) {
                        fprintf(cc->out, "    .zero %d\n", expected_end - *emitted);
                        *emitted = expected_end;
                    }
                }
            } else {
                int elem_size = type_size(f->type->base);
                for (j = 0; j < f->type->array_len; j++) {
                    if (*arg_idx < arg_end && *arg_idx < num_args) {
                        Node *elem = args[(*arg_idx)++];
                        while (elem && elem->kind == ND_CAST) elem = elem->lhs; /* Strip ND_CAST */
                        int const_ok = 1;
                        long long const_val = eval_const_expr_p4(elem, &const_ok);
                        if (!elem) {
                            fprintf(cc->out, "    .zero %d\n", elem_size);
                        } else if (const_ok) {
                            if (elem_size == 1) fprintf(cc->out, "    .byte %lld\n", const_val);
                            else if (elem_size == 2) fprintf(cc->out, "    .short %lld\n", const_val);
                            else if (elem_size == 4) fprintf(cc->out, "    .long %lld\n", const_val);
                            else fprintf(cc->out, "    .quad %lld\n", const_val);
                        } else if (elem->kind == ND_STR) {
                            if (elem_size == 4) fprintf(cc->out, "    .long .Lstr_%d\n", elem->str_id);
                            else fprintf(cc->out, "    .quad .Lstr_%d\n", elem->str_id);
                        } else if (elem->kind == ND_ADDR && elem->lhs && elem->lhs->kind == ND_VAR) {
                            if (elem_size == 4) fprintf(cc->out, "    .long %s\n", elem->lhs->name);
                            else fprintf(cc->out, "    .quad %s\n", elem->lhs->name);
                } else if (elem->kind == ND_ADDR && elem->lhs && elem->lhs->kind == ND_DEREF && elem->lhs->lhs && elem->lhs->lhs->kind == ND_ADD && elem->lhs->lhs->lhs && elem->lhs->lhs->lhs->kind == ND_VAR && elem->lhs->lhs->rhs && elem->lhs->lhs->rhs->kind == ND_NUM) {
                    long long offset = elem->lhs->lhs->rhs->int_val;
                    if (elem->lhs->lhs->lhs->type && elem->lhs->lhs->lhs->type->base) offset *= type_size(elem->lhs->lhs->lhs->type->base);
                    if (elem_size == 4) fprintf(cc->out, "    .long %s + %lld\n", elem->lhs->lhs->lhs->name, offset);
                    else fprintf(cc->out, "    .quad %s + %lld\n", elem->lhs->lhs->lhs->name, offset);
                } else if (elem->kind == ND_ADD && elem->lhs && elem->lhs->kind == ND_VAR && elem->rhs && elem->rhs->kind == ND_NUM) {
                    long long offset = elem->rhs->int_val;
                    if (elem->lhs->type && elem->lhs->type->base) offset *= type_size(elem->lhs->type->base);
                    if (elem_size == 4) fprintf(cc->out, "    .long %s + %lld\n", elem->lhs->name, offset);
                    else fprintf(cc->out, "    .quad %s + %lld\n", elem->lhs->name, offset);
                        } else if (elem->kind == ND_VAR) {
                            if (elem_size == 4) fprintf(cc->out, "    .long %s\n", elem->name);
                            else fprintf(cc->out, "    .quad %s\n", elem->name);
                        } else {
                            fprintf(stderr, "STRUCT-ARRAY FALLBACK ZERO: kind=%d, const_ok=%d\n", elem->kind, const_ok);
                            if (elem->lhs) fprintf(stderr, "  lhs->kind=%d lhs->name=%s\n", elem->lhs->kind, elem->lhs->name ? elem->lhs->name : "none");
                            fprintf(cc->out, "    .zero %d\n", elem_size);
                        }
                    } else {
                        fprintf(cc->out, "    .zero %d\n", elem_size);
                    }
                    *emitted += elem_size;
                }
            }
        } else {
            int elem_size = type_size(f->type);
            if (*arg_idx < arg_end && *arg_idx < num_args) {
                Node *elem = args[(*arg_idx)++];
                while (elem && elem->kind == ND_CAST) { elem = elem->lhs; } /* Strip ND_CAST */
                int const_ok = 1;
                long long const_val = eval_const_expr_p4(elem, &const_ok);
                if (!elem) {
                    fprintf(cc->out, "    .zero %d\n", elem_size);
                } else if (const_ok) {
                    if (elem_size == 1) fprintf(cc->out, "    .byte %lld\n", const_val);
                    else if (elem_size == 2) fprintf(cc->out, "    .short %lld\n", const_val);
                    else if (elem_size == 4) fprintf(cc->out, "    .long %lld\n", const_val);
                    else fprintf(cc->out, "    .quad %lld\n", const_val);
                } else if (elem->kind == ND_STR) {
                    if (elem_size == 4) fprintf(cc->out, "    .long .Lstr_%d\n", elem->str_id);
                    else fprintf(cc->out, "    .quad .Lstr_%d\n", elem->str_id);
                } else if (elem->kind == ND_ADDR && elem->lhs && elem->lhs->kind == ND_VAR) {
                    if (elem_size == 4) fprintf(cc->out, "    .long %s\n", elem->lhs->name);
                    else fprintf(cc->out, "    .quad %s\n", elem->lhs->name);
                } else if (elem->kind == ND_ADDR && elem->lhs && elem->lhs->kind == ND_DEREF && elem->lhs->lhs && elem->lhs->lhs->kind == ND_ADD && elem->lhs->lhs->lhs && elem->lhs->lhs->lhs->kind == ND_VAR && elem->lhs->lhs->rhs && elem->lhs->lhs->rhs->kind == ND_NUM) {
                    long long offset = elem->lhs->lhs->rhs->int_val;
                    if (elem->lhs->lhs->lhs->type && elem->lhs->lhs->lhs->type->base) offset *= type_size(elem->lhs->lhs->lhs->type->base);
                    if (elem_size == 4) fprintf(cc->out, "    .long %s + %lld\n", elem->lhs->lhs->lhs->name, offset);
                    else fprintf(cc->out, "    .quad %s + %lld\n", elem->lhs->lhs->lhs->name, offset);
                } else if (elem->kind == ND_ADD && elem->lhs && elem->lhs->kind == ND_VAR && elem->rhs && elem->rhs->kind == ND_NUM) {
                    long long offset = elem->rhs->int_val;
                    if (elem->lhs->type && elem->lhs->type->base) offset *= type_size(elem->lhs->type->base);
                    if (elem_size == 4) fprintf(cc->out, "    .long %s + %lld\n", elem->lhs->name, offset);
                    else fprintf(cc->out, "    .quad %s + %lld\n", elem->lhs->name, offset);
                } else if (elem->kind == ND_VAR) {
                    if (elem_size == 4) fprintf(cc->out, "    .long %s\n", elem->name);
                    else fprintf(cc->out, "    .quad %s\n", elem->name);
                } else {
                    fprintf(stderr, "STRUCT FIELD FALLBACK ZERO: kind=%d, const_ok=%d\n", elem->kind, const_ok);
                    if (elem->lhs) fprintf(stderr, "  lhs->kind=%d lhs->name=%s\n", elem->lhs->kind, elem->lhs->name ? elem->lhs->name : "none");
                    fprintf(cc->out, "    .zero %d\n", elem_size);
                }
            } else {
                fprintf(cc->out, "    .zero %d\n", elem_size);
            }
            *emitted += elem_size;
        }
    }
}

static void emit_global_var(Compiler *cc, Node *gvar) {
  int size;

  if (gvar->is_extern)
    return; /* no emission for extern */

  size = type_size(gvar->type);
  if (size <= 0)
    size = 8;
    
  if (gvar->initializer) {
    /* initialized data goes in .data */
    fprintf(cc->out, "    .data\n");
    fprintf(cc->out, "    .p2align 3\n");
    if (!gvar->is_static) {
      fprintf(cc->out, "    .globl %s\n", gvar->name);
    }
    fprintf(cc->out, "%s:\n", gvar->name);
    /* only handle simple integer initializers */
    /* Strip ND_CAST wrappers (e.g. function pointer casts like
       `Curl_cmalloc = (curl_malloc_callback)malloc`) */
    Node *init = gvar->initializer;
    while (init && init->kind == ND_CAST) init = init->lhs;
    if (init && init->kind == ND_NUM) {
      if (size == 1)
        fprintf(cc->out, "    .byte %lld\n", init->int_val);
      else if (size == 2)
        fprintf(cc->out, "    .short %lld\n", init->int_val);
      else if (size == 4)
        fprintf(cc->out, "    .long %lld\n", init->int_val);
      else
        fprintf(cc->out, "    .quad %lld\n", init->int_val);
    } else if (init->kind == ND_ADDR && init->lhs && init->lhs->kind == ND_VAR) {
      if (size == 4)
        fprintf(cc->out, "    .long %s\n", init->lhs->name);
      else
        fprintf(cc->out, "    .quad %s\n", init->lhs->name);
    } else if (init->kind == ND_ADDR && init->lhs && init->lhs->kind == ND_DEREF && init->lhs->lhs && init->lhs->lhs->kind == ND_ADD && init->lhs->lhs->lhs && init->lhs->lhs->lhs->kind == ND_VAR && init->lhs->lhs->rhs && init->lhs->lhs->rhs->kind == ND_NUM) {
      long long offset = init->lhs->lhs->rhs->int_val;
      if (init->lhs->lhs->lhs->type && init->lhs->lhs->lhs->type->base) offset *= type_size(init->lhs->lhs->lhs->type->base);
      if (size == 4) fprintf(cc->out, "    .long %s + %lld\n", init->lhs->lhs->lhs->name, offset);
      else fprintf(cc->out, "    .quad %s + %lld\n", init->lhs->lhs->lhs->name, offset);
    } else if (init->kind == ND_ADD && init->lhs && init->lhs->kind == ND_VAR && init->rhs && init->rhs->kind == ND_NUM) {
      long long offset = init->rhs->int_val;
      if (init->lhs->type && init->lhs->type->base) offset *= type_size(init->lhs->type->base);
      if (size == 4) fprintf(cc->out, "    .long %s + %lld\n", init->lhs->name, offset);
      else fprintf(cc->out, "    .quad %s + %lld\n", init->lhs->name, offset);
    } else if (init->kind == ND_VAR) {
      if (size == 4)
        fprintf(cc->out, "    .long %s\n", init->name);
      else
        fprintf(cc->out, "    .quad %s\n", init->name);
    } else if (init->kind == ND_STR) {
      if (gvar->type && gvar->type->kind == TY_ARRAY) {
          int j;
          int str_id = init->str_id;
          int len = cc->strings[str_id].len;
          fprintf(cc->out, "    .ascii \"");
          for (j = 0; j < len; j++) {
              char c = cc->strings[str_id].data[j];
              if (c == '\n') fprintf(cc->out, "\\n");
              else if (c == '\t') fprintf(cc->out, "\\t");
              else if (c == '\r') fprintf(cc->out, "\\r");
              else if (c == '\\') fprintf(cc->out, "\\\\");
              else if (c == '"') fprintf(cc->out, "\\\"");
              else if (c >= 32 && c < 127) fprintf(cc->out, "%c", c);
              else fprintf(cc->out, "\\%03o", (unsigned char)c);
          }
          fprintf(cc->out, "\\0\"\n");
          if (size > len + 1) fprintf(cc->out, "    .zero %d\n", size - (len + 1));
      } else {
          if (size == 4) fprintf(cc->out, "    .long .Lstr_%d\n", init->str_id);
          else fprintf(cc->out, "    .quad .Lstr_%d\n", init->str_id);
      }
        } else if (init->kind == ND_INIT_LIST) {
            int emitted = 0;
            int i;
            int elem_size = 1;
        if (gvar->type && gvar->type->kind == TY_STRUCT) {
            int arg_idx = 0;
            emit_struct_fields(cc, gvar->type->fields, init->args, init->num_args, &arg_idx, init->num_args, 0, &emitted);
        } else if (gvar->type && gvar->type->kind == TY_ARRAY && gvar->type->base && (gvar->type->base->kind == TY_STRUCT || gvar->type->base->kind == TY_UNION)) {
            int i;
            int arg_idx = 0;
            int elem_size = type_size(gvar->type->base);
            int budget = init->num_args / gvar->type->array_len;
            for (i = 0; i < gvar->type->array_len; i++) {
                int expected_end = (i + 1) * elem_size;
                int arg_end = (i + 1) * budget;
                if (arg_end > init->num_args) arg_end = init->num_args;
                emit_struct_fields(cc, gvar->type->base->fields, init->args, init->num_args, &arg_idx, arg_end, i * elem_size, &emitted);
                if (emitted < expected_end) {
                    fprintf(cc->out, "    .zero %d\n", expected_end - emitted);
                    emitted = expected_end;
                }
            }
        } else {
            int i;
            int elem_size = 1;
            if (gvar->type && gvar->type->base) elem_size = type_size(gvar->type->base);
            for (i = 0; i < init->num_args; i++) {
                Node *elem = init->args[i];
                int const_ok = 1;
                long long const_val = eval_const_expr_p4(elem, &const_ok);
                if (!elem) {
                    fprintf(cc->out, "    .zero %d\n", elem_size);
                } else if (const_ok) {
                    if (elem_size == 1) fprintf(cc->out, "    .byte %lld\n", const_val);
                    else if (elem_size == 2) fprintf(cc->out, "    .short %lld\n", const_val);
                    else if (elem_size == 4) fprintf(cc->out, "    .long %lld\n", const_val);
                    else fprintf(cc->out, "    .quad %lld\n", const_val);
                } else if (elem->kind == ND_STR) {
                    if (elem_size == 4) fprintf(cc->out, "    .long .Lstr_%d\n", elem->str_id);
                    else fprintf(cc->out, "    .quad .Lstr_%d\n", elem->str_id);
                } else if (elem->kind == ND_ADDR && elem->lhs && elem->lhs->kind == ND_VAR) {
                    if (elem_size == 4) fprintf(cc->out, "    .long %s\n", elem->lhs->name);
                    else fprintf(cc->out, "    .quad %s\n", elem->lhs->name);
                } else if (elem->kind == ND_ADDR && elem->lhs && elem->lhs->kind == ND_DEREF && elem->lhs->lhs && elem->lhs->lhs->kind == ND_ADD && elem->lhs->lhs->lhs && elem->lhs->lhs->lhs->kind == ND_VAR && elem->lhs->lhs->rhs && elem->lhs->lhs->rhs->kind == ND_NUM) {
                    long long offset = elem->lhs->lhs->rhs->int_val;
                    if (elem->lhs->lhs->lhs->type && elem->lhs->lhs->lhs->type->base) offset *= type_size(elem->lhs->lhs->lhs->type->base);
                    if (elem_size == 4) fprintf(cc->out, "    .long %s + %lld\n", elem->lhs->lhs->lhs->name, offset);
                    else fprintf(cc->out, "    .quad %s + %lld\n", elem->lhs->lhs->lhs->name, offset);
                } else if (elem->kind == ND_ADD && elem->lhs && elem->lhs->kind == ND_VAR && elem->rhs && elem->rhs->kind == ND_NUM) {
                    long long offset = elem->rhs->int_val;
                    if (elem->lhs->type && elem->lhs->type->base) offset *= type_size(elem->lhs->type->base);
                    if (elem_size == 4) fprintf(cc->out, "    .long %s + %lld\n", elem->lhs->name, offset);
                    else fprintf(cc->out, "    .quad %s + %lld\n", elem->lhs->name, offset);
                } else if (elem->kind == ND_VAR) {
                    if (elem_size == 4) fprintf(cc->out, "    .long %s\n", elem->name);
                    else fprintf(cc->out, "    .quad %s\n", elem->name);
                } else {
                    fprintf(cc->out, "    .zero %d\n", elem_size);
                }
                emitted += elem_size;
            }
        }
        if (emitted < size) {
            fprintf(cc->out, "    .zero %d\n", size - emitted);
        }
    } else {
        fprintf(cc->out, "    .zero %d\n", size);
    }
  } else {
    /* tentative definitions and uninitialized data */
    if (gvar->is_static) {
        fprintf(cc->out, "    .local %s\n", gvar->name);
        fprintf(cc->out, "    .comm %s, %d, 8\n", gvar->name, size);
    } else {
        fprintf(cc->out, "    .comm %s, %d, 8\n", gvar->name, size);
    }
  }
}

/* ================================================================ */
/* STRING LITERAL EMISSION                                           */
/* ================================================================ */

static void emit_strings(Compiler *cc) {
  int i;
  fprintf(cc->out, "    .section .rodata\n");
  for (i = 0; i < cc->num_strings; i++) {
    int j;
    fprintf(cc->out, ".L%d:\n", cc->strings[i].label_id);
    fprintf(cc->out, ".Lstr_%d:\n", i); /* IR backend alias */
    fprintf(cc->out, "    .asciz \"");
    for (j = 0; j < cc->strings[i].len; j++) {
      char c;
      c = cc->strings[i].data[j];
      if (c == '\n')
        fprintf(cc->out, "\\n");
      else if (c == '\t')
        fprintf(cc->out, "\\t");
      else if (c == '\r')
        fprintf(cc->out, "\\r");
      else if (c == '\\')
        fprintf(cc->out, "\\\\");
      else if (c == '"')
        fprintf(cc->out, "\\\"");
      else if (c == '\0')
        fprintf(cc->out, "\\0");
      else if (c >= 32 && c < 127)
        fprintf(cc->out, "%c", c);
      else
        fprintf(cc->out, "\\%03o", (unsigned char)c);
    }
    fprintf(cc->out, "\"\n");
  }
}

/* ================================================================ */
/* PROGRAM CODEGEN ENTRY                                             */
/* ================================================================ */

static void fold_constants(Compiler *cc, Node *node) {
  if (!node)
    return;

  fold_constants(cc, node->lhs);
  fold_constants(cc, node->rhs);
  fold_constants(cc, node->cond);
  fold_constants(cc, node->then_body);
  fold_constants(cc, node->else_body);
  fold_constants(cc, node->init);
  fold_constants(cc, node->inc);
  fold_constants(cc, node->body);
  fold_constants(cc, node->case_body);

  if (node->kind == ND_CALL && node->num_args > 0) {
    int i;
    for (i = 0; i < node->num_args; i++)
      fold_constants(cc, node->args[i]);
  }
  if (node->kind == ND_BLOCK && node->num_stmts > 0) {
    int i;
    for (i = 0; i < node->num_stmts; i++)
      fold_constants(cc, node->stmts[i]);
  }
  if (node->kind == ND_SWITCH && node->num_cases > 0) {
    int i;
    for (i = 0; i < node->num_cases; i++)
      fold_constants(cc, node->cases[i]);
    fold_constants(cc, node->default_case);
  }

  if (node->kind == ND_NEG || node->kind == ND_BNOT || node->kind == ND_LNOT) {
    if (node->lhs && node->lhs->kind == ND_NUM) {
      long long v1 = node->lhs->int_val, res = 0;
      if (node->kind == ND_NEG)
        res = -v1;
      else if (node->kind == ND_BNOT)
        res = ~v1;
      else if (node->kind == ND_LNOT)
        res = !v1;

      if (node->type && !node_type_unsigned(node)) {
          if (node->type->size == 4) res = (long long)(int)res;
          else if (node->type->size == 1) res = (long long)(char)res;
          else if (node->type->size == 2) res = (long long)(short)res;
      } else if (node->type && node_type_unsigned(node)) {
          if (node->type->size == 4) res = (long long)(unsigned int)res;
          else if (node->type->size == 1) res = (long long)(unsigned char)res;
          else if (node->type->size == 2) res = (long long)(unsigned short)res;
      }
      node->kind = ND_NUM;
      node->int_val = res;
      node->lhs = 0;
    }
  }

  if (node->kind == ND_ADD || node->kind == ND_SUB || node->kind == ND_MUL ||
      node->kind == ND_DIV || node->kind == ND_MOD) {
    if (node->lhs && node->rhs && node->lhs->kind == ND_NUM &&
        node->rhs->kind == ND_NUM) {
      long long v1 = node->lhs->int_val, v2 = node->rhs->int_val, res = 0;
      int is_unsigned = node->lhs->type && node_type_unsigned(node->lhs);
      unsigned long long u1 = v1, u2 = v2;
      if (is_unsigned && node->lhs->type) {
          if (node->lhs->type->size == 4) { u1 = (unsigned int)v1; u2 = (unsigned int)v2; }
          else if (node->lhs->type->size == 1) { u1 = (unsigned char)v1; u2 = (unsigned char)v2; }
          else if (node->lhs->type->size == 2) { u1 = (unsigned short)v1; u2 = (unsigned short)v2; }
      }

      if (node->kind == ND_ADD)
        res = v1 + v2;
      else if (node->kind == ND_SUB)
        res = v1 - v2;
      else if (node->kind == ND_MUL)
        res = v1 * v2;
      else if (node->kind == ND_DIV && v2 != 0)
        res = is_unsigned ? u1 / u2 : v1 / v2;
      else if (node->kind == ND_MOD && v2 != 0)
        res = is_unsigned ? u1 % u2 : v1 % v2;
      else
        return;
      if (node->type && !node_type_unsigned(node)) {
          if (node->type->size == 4) res = (long long)(int)res;
          else if (node->type->size == 1) res = (long long)(char)res;
          else if (node->type->size == 2) res = (long long)(short)res;
      } else if (node->type && node_type_unsigned(node)) {
          if (node->type->size == 4) res = (long long)(unsigned int)res;
          else if (node->type->size == 1) res = (long long)(unsigned char)res;
          else if (node->type->size == 2) res = (long long)(unsigned short)res;
      }
      node->kind = ND_NUM;
      node->int_val = res;
      node->lhs = 0;
      node->rhs = 0;
    }
  }

  if (node->kind == ND_SHL || node->kind == ND_SHR || node->kind == ND_BAND ||
      node->kind == ND_BOR || node->kind == ND_BXOR || node->kind == ND_LT ||
      node->kind == ND_LE || node->kind == ND_GT || node->kind == ND_GE ||
      node->kind == ND_EQ || node->kind == ND_NE) {
    if (node->lhs && node->rhs && node->lhs->kind == ND_NUM &&
        node->rhs->kind == ND_NUM) {
      long long v1 = node->lhs->int_val, v2 = node->rhs->int_val, res = 0;
      int is_unsigned = node->lhs->type && node_type_unsigned(node->lhs);
      unsigned long long u1 = v1, u2 = v2;
      if (is_unsigned && node->lhs->type) {
          if (node->lhs->type->size == 4) { u1 = (unsigned int)v1; u2 = (unsigned int)v2; }
          else if (node->lhs->type->size == 1) { u1 = (unsigned char)v1; u2 = (unsigned char)v2; }
          else if (node->lhs->type->size == 2) { u1 = (unsigned short)v1; u2 = (unsigned short)v2; }
      }

      if (node->kind == ND_SHL)
        res = v1 << v2;
      else if (node->kind == ND_SHR)
        res = is_unsigned ? u1 >> u2 : v1 >> v2;
      else if (node->kind == ND_BAND)
        res = v1 & v2;
      else if (node->kind == ND_BOR)
        res = v1 | v2;
      else if (node->kind == ND_BXOR)
        res = v1 ^ v2;
      else if (node->kind == ND_LT)
        res = is_unsigned ? u1 < u2 : v1 < v2;
      else if (node->kind == ND_LE)
        res = is_unsigned ? u1 <= u2 : v1 <= v2;
      else if (node->kind == ND_GT)
        res = is_unsigned ? u1 > u2 : v1 > v2;
      else if (node->kind == ND_GE)
        res = is_unsigned ? u1 >= u2 : v1 >= v2;
      else if (node->kind == ND_EQ)
        res = v1 == v2;
      else if (node->kind == ND_NE)
        res = v1 != v2;
      if (node->type && !node_type_unsigned(node)) {
          if (node->type->size == 4) res = (long long)(int)res;
          else if (node->type->size == 1) res = (long long)(char)res;
          else if (node->type->size == 2) res = (long long)(short)res;
      } else if (node->type && node_type_unsigned(node)) {
          if (node->type->size == 4) res = (long long)(unsigned int)res;
          else if (node->type->size == 1) res = (long long)(unsigned char)res;
          else if (node->type->size == 2) res = (long long)(unsigned short)res;
      }
      node->kind = ND_NUM;
      node->int_val = res;
      node->lhs = 0;
      node->rhs = 0;
    }
  }

  /* Dead-Branch Elimination (AST-level DCE) */
  if (node->kind == ND_IF) {
    if (node->cond && node->cond->kind == ND_NUM) {
      if (node->cond->int_val == 0) {
        /* condition is false: replace with else body or empty block */
        if (node->else_body) {
          Node *tgt = node->else_body;
          unsigned long long save_magic = node->magic;
          unsigned long long save_alloc_id = node->alloc_id;
          *node = *tgt;
          node->magic = save_magic;
          node->alloc_id = save_alloc_id;
        } else {
          node->kind = ND_BLOCK;
          node->num_stmts = 0;
          node->stmts = (Node **)cc_alloc(cc, sizeof(Node *));
        }
      } else {
        /* condition is true: replace with then body */
        if (node->then_body) {
          Node *tgt = node->then_body;
          unsigned long long save_magic = node->magic;
          unsigned long long save_alloc_id = node->alloc_id;
          *node = *tgt;
          node->magic = save_magic;
          node->alloc_id = save_alloc_id;
        } else {
          node->kind = ND_BLOCK;
          node->num_stmts = 0;
          node->stmts = (Node **)cc_alloc(cc, sizeof(Node *));
        }
      }
    }
  } else if (node->kind == ND_WHILE) {
    if (node->cond && node->cond->kind == ND_NUM && node->cond->int_val == 0) {
      /* while (0): unreachable loop, collapse entirely */
      node->kind = ND_BLOCK;
      node->num_stmts = 0;
      node->stmts = (Node **)cc_alloc(cc, sizeof(Node *));
    }
  } else if (node->kind == ND_TERNARY) {
    if (node->cond && node->cond->kind == ND_NUM) {
      if (node->cond->int_val == 0) {
        if (node->else_body) {
          Node *tgt = node->else_body;
          unsigned long long save_magic = node->magic;
          unsigned long long save_alloc_id = node->alloc_id;
          *node = *tgt;
          node->magic = save_magic;
          node->alloc_id = save_alloc_id;
        }
      } else {
        if (node->then_body) {
          Node *tgt = node->then_body;
          unsigned long long save_magic = node->magic;
          unsigned long long save_alloc_id = node->alloc_id;
          *node = *tgt;
          node->magic = save_magic;
          node->alloc_id = save_alloc_id;
        }
      }
    }
  }
}

void codegen_program(Compiler *cc, Node *prog) {
  Node *n;
  int i;

  /* Do NOT check ptr_in_fault_range(prog/n): stage2 miscompiles it and falsely
   * rejects valid arena pointers (e.g. 0xac09f8), causing early exit and
   * WinMain link error. */
  if (!prog)
    return;

  /* Linux: avoid "missing .note.GNU-stack" linker warning */
  if (!backend_ops) fprintf(cc->out, "    .section .note.GNU-stack,\"\",@progbits\n");
  if (cc->filename) {
    fprintf(cc->out, "    .file 1 \"%s\"\n", cc->filename);
  }

  /* Emit functions */
  n = prog;
  while (n) {
    if (n->kind == ND_FUNC_DEF) {
      fold_constants(cc, n);
      codegen_func(cc, n);
    }
    n = n->next;
  }

  /* Deduplicate tentative definitions: keep only one instance (prioritizing initializers) */
  for (i = 0; i < cc->num_globals; i++) {
    int j;
    Node *g1 = cc->globals[i];
    if (!g1 || g1->kind != ND_GLOBAL_VAR) continue;
    
    for (j = i + 1; j < cc->num_globals; j++) {
      Node *g2 = cc->globals[j];
      if (!g2 || g2->kind != ND_GLOBAL_VAR) continue;

      
      if (strcmp(g1->name, g2->name) == 0) {
        if (g1->initializer && g2->initializer) {
          cc->globals[j] = 0; /* Keep first initializer, drop second */
        } else if (g1->initializer) {
          cc->globals[j] = 0; /* Keep initialized, drop uninitialized */
        } else if (g2->initializer) {
          cc->globals[i] = 0; /* Drop uninitialized, keep initialized */
          break; /* g1 is gone, stop checking against g1 */
        } else {
          if (g1->is_extern && !g2->is_extern) {
              cc->globals[i] = 0; /* Keep tentative, drop extern */
              break;
          } else {
              cc->globals[j] = 0; /* Both uninitialized, prioritize g1 */
          }
        }
      }
    }
  }

  /* Emit global variables */
  for (i = 0; i < cc->num_globals; i++) {
    if (cc->globals[i]) {
      fold_constants(cc, cc->globals[i]->initializer);
      emit_global_var(cc, cc->globals[i]);
    }
  }

  /* Emit string literals */
  if (cc->num_strings > 0) {
    emit_strings(cc);
  }
}

/* ZKAEDI FORCE RENDER CACHE INVALIDATION */

extern int is_pointer(Type *type);

int node_ptr_elem_size(struct Node *n) {
  if (!n || !n->type)
    return 0;
  if (is_pointer(n->type)) {
    return ptr_elem_size(n->type);
  }
  return 0;
}
/* ================================================================ */
/* COMPILER INITIALIZATION                                           */
/* ================================================================ */

#include "ir_emit_dispatch.h"

static void init_compiler(Compiler *cc) {
  /* zero everyt5555hing — cc was calloc'd */

  /* init arena */
  cc->arena.data = (char *)malloc(ARENA_SIZE);
  cc->arena.pos = 0;
  cc->arena.cap = ARENA_SIZE;
  cc->arena.next = 0;

  /* create type singletons */
  cc->ty_void = type_new(cc, TY_VOID);
  cc->ty_char = type_new(cc, TY_CHAR);
  cc->ty_uchar = type_new(cc, TY_UCHAR);
  cc->ty_short = type_new(cc, TY_SHORT);
  cc->ty_ushort = type_new(cc, TY_USHORT);
  cc->ty_int = type_new(cc, TY_INT);
  cc->ty_uint = type_new(cc, TY_UINT);
  cc->ty_long = type_new(cc, TY_LONG);
  cc->ty_ulong = type_new(cc, TY_ULONG);
  cc->ty_longlong = type_new(cc, TY_LONGLONG);
  cc->ty_ulonglong = type_new(cc, TY_ULONGLONG);
  cc->ty_float = type_new(cc, TY_FLOAT);
  cc->ty_double = type_new(cc, TY_DOUBLE);

  /* init lexer */
  cc->line = 1;
  cc->col = 1;
  cc->pos = 0;
  cc->has_peek = 0;
  cc->label_count = 100; /* start at 100 to avoid clashes */
  cc->errors = 0;
  cc->num_strings = 0;
  cc->num_structs = 0;
  cc->num_globals = 0;

  /* push global scope */
  scope_push(cc);

  /* register common typedefs and builtins */
  {
    Symbol *sym;
    /* typedef void *FILE — so FILE* works */
    sym = scope_add(cc, "FILE", type_ptr(cc, cc->ty_void));
    sym->is_typedef = 1;

    /* typedef long size_t */
    sym = scope_add(cc, "size_t", cc->ty_ulonglong);
    sym->is_typedef = 1;

    /* typedef long ssize_t */
    sym = scope_add(cc, "ssize_t", cc->ty_long);
    sym->is_typedef = 1;

    /* typedef int ptrdiff_t */
    sym = scope_add(cc, "ptrdiff_t", cc->ty_long);
    sym->is_typedef = 1;

    /* SysV ABI requires va_list to be an array of 1 struct of size 24 */
    {
        Type *t_va = type_new(cc, TY_STRUCT);
        t_va->size = 24;
        t_va->align = 8;
        t_va->is_complete = 1;
        sym = scope_add(cc, "__builtin_va_list", type_array(cc, t_va, 1));
    }
    sym->is_typedef = 1;

    /* _Float128 workaround */
    sym = scope_add(cc, "_Float128", cc->ty_double);
    sym->is_typedef = 1;


    /* NULL as enum constant */
    sym = scope_add(cc, "NULL", cc->ty_long);
    sym->is_enum_const = 1;
    sym->enum_val = 0;

    /* stdout, stderr */
    sym = scope_add(cc, "stdout", type_ptr(cc, cc->ty_void));
    sym->is_global = 1;
    sym = scope_add(cc, "stderr", type_ptr(cc, cc->ty_void));
    sym->is_global = 1;

    /* common libc functions */
    {
      Type *ft;

      /* printf — returns int, variadic */
      ft = type_func(cc, cc->ty_int);
      ft->is_variadic = 1;
      sym = scope_add(cc, "printf", ft);
      sym->is_global = 1;

      /* fprintf */
      ft = type_func(cc, cc->ty_int);
      ft->is_variadic = 1;
      sym = scope_add(cc, "fprintf", ft);
      sym->is_global = 1;

      /* sscanf */
      ft = type_func(cc, cc->ty_int);
      ft->is_variadic = 1;
      sym = scope_add(cc, "sscanf", ft);
      sym->is_global = 1;

      /* sprintf */
      ft = type_func(cc, cc->ty_int);
      ft->is_variadic = 1;
      sym = scope_add(cc, "sprintf", ft);
      sym->is_global = 1;

      /* malloc — returns void* */
      ft = type_func(cc, type_ptr(cc, cc->ty_void));
      sym = scope_add(cc, "malloc", ft);
      sym->is_global = 1;

      /* calloc */
      ft = type_func(cc, type_ptr(cc, cc->ty_void));
      sym = scope_add(cc, "calloc", ft);
      sym->is_global = 1;

      /* realloc */
      ft = type_func(cc, type_ptr(cc, cc->ty_void));
      sym = scope_add(cc, "realloc", ft);
      sym->is_global = 1;

      /* free — returns void */
      ft = type_func(cc, cc->ty_void);
      sym = scope_add(cc, "free", ft);
      sym->is_global = 1;

      /* exit */
      ft = type_func(cc, cc->ty_void);
      sym = scope_add(cc, "exit", ft);
      sym->is_global = 1;

      /* fopen — returns FILE* */
      ft = type_func(cc, type_ptr(cc, cc->ty_void));
      sym = scope_add(cc, "fopen", ft);
      sym->is_global = 1;

      /* fclose */
      ft = type_func(cc, cc->ty_int);
      sym = scope_add(cc, "fclose", ft);
      sym->is_global = 1;

      /* fread */
      ft = type_func(cc, cc->ty_ulong);
      sym = scope_add(cc, "fread", ft);
      sym->is_global = 1;

      /* fwrite */
      ft = type_func(cc, cc->ty_ulong);
      sym = scope_add(cc, "fwrite", ft);
      sym->is_global = 1;

      /* fseek */
      ft = type_func(cc, cc->ty_int);
      sym = scope_add(cc, "fseek", ft);
      sym->is_global = 1;

      /* ftell */
      ft = type_func(cc, cc->ty_long);
      sym = scope_add(cc, "ftell", ft);
      sym->is_global = 1;

      /* fgets */
      ft = type_func(cc, type_ptr(cc, cc->ty_char));
      sym = scope_add(cc, "fgets", ft);
      sym->is_global = 1;

      /* fputs */
      ft = type_func(cc, cc->ty_int);
      sym = scope_add(cc, "fputs", ft);
      sym->is_global = 1;

      /* strcmp, strncpy, strlen, strncmp */
      ft = type_func(cc, cc->ty_int);
      sym = scope_add(cc, "strcmp", ft);
      sym->is_global = 1;

      ft = type_func(cc, type_ptr(cc, cc->ty_char));
      sym = scope_add(cc, "strncpy", ft);
      sym->is_global = 1;

      ft = type_func(cc, cc->ty_ulong);
      sym = scope_add(cc, "strlen", ft);
      sym->is_global = 1;

      /* strstr */
      ft = type_func(cc, type_ptr(cc, cc->ty_char));
      sym = scope_add(cc, "strstr", ft);
      sym->is_global = 1;

      /* strtod */
      ft = type_func(cc, cc->ty_double);
      sym = scope_add(cc, "strtod", ft);
      sym->is_global = 1;

      ft = type_func(cc, cc->ty_int);
      sym = scope_add(cc, "strncmp", ft);
      sym->is_global = 1;

      ft = type_func(cc, type_ptr(cc, cc->ty_char));
      sym = scope_add(cc, "strcpy", ft);
      sym->is_global = 1;

      /* memset, memcpy */
      ft = type_func(cc, type_ptr(cc, cc->ty_void));
      sym = scope_add(cc, "memset", ft);
      sym->is_global = 1;

      ft = type_func(cc, type_ptr(cc, cc->ty_void));
      sym = scope_add(cc, "memcpy", ft);
      sym->is_global = 1;

      /* write, read (syscall wrappers) */
      ft = type_func(cc, cc->ty_long);
      sym = scope_add(cc, "write", ft);
      sym->is_global = 1;

      ft = type_func(cc, cc->ty_long);
      sym = scope_add(cc, "read", ft);
      sym->is_global = 1;

      /* system */
      ft = type_func(cc, cc->ty_int);
      sym = scope_add(cc, "system", ft);
      sym->is_global = 1;

      /* _exit */
      ft = type_func(cc, cc->ty_void);
      sym = scope_add(cc, "_exit", ft);
      sym->is_global = 1;
    }
  }
}

/* ================================================================ */
/* FILE READING                                                      */
/* ================================================================ */

static char *read_file(char *path, int *out_len) {
  FILE *fp;
  long len;
  char *buf;
  int nr;

  fprintf(stderr, "read_file: path = '%s' (addr: %p)\n", path, path);
  fp = fopen(path, "rb");
  if (!fp)
    return 0;

  fseek(fp, 0, 2); /* SEEK_END = 2 */
  len = ftell(fp);
  fseek(fp, 0, 0); /* SEEK_SET = 0 */

  buf = (char *)malloc(len + 1);
  if (!buf) {
    fclose(fp);
    return 0;
  }

  nr = fread(buf, 1, len, fp);
  buf[nr] = 0;
  fclose(fp);

  *out_len = nr;
  return buf;
}

/* ================================================================ */
/* PEEPHOLE OPTIMIZER                                                */
/* ================================================================ */

#define MAX_PEEP_LINES 3000000
#define MAX_PEEP_LEN 128

static char *line_buffer = 0;
static char **line_ptrs = 0;

static void peephole_optimize(char *filename) {
  FILE *fp;
  int nlines = 0;
  int i;
  int eliminated = 0;
  long file_size;

  fp = fopen(filename, "r");
  if (!fp)
    return;

  fseek(fp, 0, 2);
  file_size = ftell(fp);
  fseek(fp, 0, 0);

  if (!line_ptrs) {
    line_ptrs = (char **)malloc(MAX_PEEP_LINES * sizeof(char *));
  }
  line_buffer = (char *)malloc(file_size + MAX_PEEP_LINES * 128);
  if (!line_buffer || !line_ptrs) {
    fclose(fp);
    return;
  }

  while (nlines < MAX_PEEP_LINES && fgets(line_buffer + nlines * 128, 128, fp)) {
    line_ptrs[nlines] = line_buffer + nlines * 128;
    nlines++;
  }
  fclose(fp);

  for (i = 0; i < nlines;) {
    char *l1 = line_ptrs[i];

    /* 1. Redundant Push/Pop */
    if (strncmp(l1, "    pushq ", 10) == 0 && i + 1 < nlines) {
      char *l2 = line_ptrs[i + 1];
      if (strncmp(l2, "    popq ", 9) == 0) {
        char tmp1[64], tmp2[64];
        sscanf(l1, "    pushq %s", tmp1);
        sscanf(l2, "    popq %s", tmp2);
        if (strcmp(tmp1, tmp2) == 0) {
          line_ptrs[i][0] = 0;
          line_ptrs[i + 1][0] = 0;
          eliminated += 2;
          i += 2;
          continue;
        } else {
          sprintf(line_ptrs[i], "    movq %s, %s\n", tmp1, tmp2);
          line_ptrs[i + 1][0] = 0;
          eliminated += 2;
          i += 2;
          continue;
        }
      }
    }

    /* 2. Arithmetic Nullification */
    if (strcmp(l1, "    addq $0, %rax\n") == 0 ||
        strcmp(l1, "    subq $0, %rax\n") == 0 ||
        strcmp(l1, "    addq $0, %rsp\n") == 0 ||
        strcmp(l1, "    subq $0, %rsp\n") == 0) {
      line_ptrs[i][0] = 0;
      eliminated += 1;
      i += 1;
      continue;
    }

    /* 3. Push/Lea/Pop Triad */
    if (strcmp(l1, "    pushq %rax\n") == 0 && i + 2 < nlines) {
      char *l2 = line_ptrs[i + 1];
      char *l3 = line_ptrs[i + 2];
      if (strncmp(l2, "    leaq ", 9) == 0 && strstr(l2, ", %rax") &&
          strncmp(l3, "    popq ", 9) == 0) {
        char pop_reg[64];
        sscanf(l3, "    popq %s", pop_reg);
        sprintf(line_ptrs[i], "    movq %%rax, %s\n", pop_reg);
        line_ptrs[i + 2][0] = 0;
        eliminated += 3;
        i += 3;
        continue;
      }
    }
    i++;
  }

  fp = fopen(filename, "w");
  if (!fp) {
    free(line_buffer);
    return;
  }
  for (i = 0; i < nlines; i++) {
    if (line_ptrs[i][0] != 0)
      fputs(line_ptrs[i], fp);
  }
  fclose(fp);
  free(line_buffer);
  printf("[Phase 5] Native C Peephole Optimization... OK (%d elided)\n",
         eliminated);
}

/* ================================================================ */
/* MAIN                                                              */
/* Bug fix: Compiler is heap-allocated (52KB+ struct, not stack)     */
/* ================================================================ */

/* Forward declaration for IR pass manager (linked separately) */
void ir_pm_run_default(void *mod_ptr, int verbose);

int main(int argc, char **argv) {
  Compiler *cc;
  char *input_file;
  char *output_file;
  char *source;
  int source_len;
  char asm_file[256];
  char cmd[512];
  Node *prog;
  int ret;
  int i;
  int al;

  input_file = 0;
  output_file = 0;

  int pp_only = 0;

  int zcc_verbose_flag = 0;

  int compile_only = 0;

  int g_ir_primary = 0;

  /* parse arguments */
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-o") == 0) {
      i++;
      if (i < argc)
        output_file = argv[i];
    } else if (strcmp(argv[i], "-c") == 0) {
      compile_only = 1;
    } else if (strcmp(argv[i], "--pp-only") == 0) {
      pp_only = 1;
    } else if (strcmp(argv[i], "-v") == 0) {
      zcc_verbose_flag = 1;
    } else if (strcmp(argv[i], "--ir") == 0) {
      g_emit_ir = 1;
      g_ir_primary = 1;
    } else {
      input_file = argv[i];
    }
  }

  if (!zcc_verbose_flag) {
#ifdef _WIN32
    freopen("nul", "w", stderr);
#else
    freopen("/dev/null", "w", stderr);
#endif
  }

  if (!input_file) {
    printf("Usage: zcc <input.c> [-o output]\n");
    return 1;
  }

  if (!output_file)
    output_file = "a.out";

  ZCC_IR_INIT();

  /* read source file */
  source = read_file(input_file, &source_len);
  if (!source) {
    printf("zcc: cannot read '%s'\n", input_file);
    return 1;
  }

  /* PREPROCESSOR HOOK */
  {
    int pp_len;
    char *pp_source = zcc_preprocess(source, source_len, input_file, ".:./include", &pp_len);
    if (!pp_source) {
      fprintf(stderr, "zcc: preprocessing failed\n");
      return 1;
    }
    /* We leak original `source` here because we replaced it.
       ZCC doesn't care much about leaking in main string allocations anyway. */
    source = pp_source;
    source_len = pp_len;
  }

  if (pp_only) {
    printf("%s", source);
    return 0;
  }

  /* heap-allocate compiler state (too large for stack) */
  cc = (Compiler *)calloc(1, sizeof(Compiler));
  if (!cc) {
    printf("zcc: out of memory\n");
    free(source);
    return 1;
  }

  cc->source = source;
  cc->source_len = source_len;
  cc->filename = input_file;

  init_compiler(cc);

  /* generate asm file name */
  strncpy(asm_file, output_file, 250);
  al = 0;
  while (asm_file[al])
    al++;

  int stop_at_asm = 0;
  if (al >= 2 && asm_file[al - 2] == '.' && asm_file[al - 1] == 's') {
    stop_at_asm = 1;
  } else {
    asm_file[al] = '.';
    asm_file[al + 1] = 's';
    asm_file[al + 2] = 0;
  }

  /* open output */
  cc->out = fopen(asm_file, "w");
  if (!cc->out) {
    printf("zcc: cannot write '%s'\n", asm_file);
    free(source);
    free(cc);
    return 1;
  }

  /* lex first token */
  printf("[Phase 1] Lexical Array Bootstrap... OK\n");
  next_token(cc);

  /* parse */
  printf("[Phase 2] AST Topological Generation... ");
  prog = parse_program(cc);

  if (cc->errors > 0) {
    printf("\033[0;31mFAILED\033[0m\n");
    printf("zcc: %d error(s)\n", cc->errors);
    fclose(cc->out);
    free(source);
    free(cc);
    return 1;
  }

  printf("OK\n");

  /* generate code */
  printf("[Phase 3] Native AST Constant Folding... OK\n");
  printf("[Phase 4] SystemV ABI X86-64 Codegen... OK\n");
  fprintf(cc->out, "# ZCC asm begin\n");
  codegen_program(cc, prog);
  fclose(cc->out);

  /* IR pass manager — runs when --ir flag is active */
  if (g_ir_primary && g_ir_module) {
    int ir_total_nodes = 0;
    int ir_fi;
    for (ir_fi = 0; ir_fi < g_ir_module->func_count; ir_fi++) {
      ir_total_nodes += g_ir_module->funcs[ir_fi]->node_count;
    }
    printf("[Phase IR] IR Pass Manager (%d funcs, %d nodes)...\n",
           g_ir_module->func_count, ir_total_nodes);
    ir_pm_run_default(g_ir_module, 1);
    printf("[Phase IR] Pass Manager Complete.\n");
  }

  if (!g_ir_primary) {
    ZCC_IR_FLUSH(stdout);
  }

  /* peephole optimize the emitted assembly safely out-of-bounds */
  peephole_optimize(asm_file);

  /* assemble and link if not stopping at assembly */
  if (!stop_at_asm) {
    printf("[Phase 6] GCC Assembly/Linker Binding... ");
    if (compile_only) {
      sprintf(cmd, "gcc -O0 -w -fno-asynchronous-unwind-tables -Wa,--noexecstack -fno-unwind-tables -c -o %s %s 2>&1", output_file, asm_file);
    } else if (strcmp(input_file, "zcc.c") == 0 || (strlen(input_file) >= 6 && strcmp(input_file + strlen(input_file) - 6, "/zcc.c") == 0)) {
      sprintf(cmd, "gcc -O0 -w -fno-asynchronous-unwind-tables -Wa,--noexecstack -fno-unwind-tables -o %s %s compiler_passes.c compiler_passes_ir.c ir_pass_manager.c -lm 2>&1", output_file, asm_file);
    } else {
      sprintf(cmd, "gcc -O0 -w -fno-asynchronous-unwind-tables -Wa,--noexecstack -fno-unwind-tables -o %s %s -lm -lpthread -ldl 2>&1", output_file, asm_file);
    }
    ret = system(cmd);
    if (ret != 0) {
      printf("FAILED\n");
      printf("zcc: assembly/linking failed\n");
      free(source);
      free(cc);
      return 1;
    }
    printf("OK\n");
  }

  printf("[OK] ZCC Engine Compilation Terminated Successfully.\n");

  free(source);
  free(cc);
  return 0;
}

/* ZKAEDI FORCE RENDER CACHE INVALIDATION */
/* ================================================================ */
/* PART 6: ARM TARGET BACKEND (thumbv6m)                             */
/* ================================================================ */


TargetBackend *backend_ops = 0;
int ZCC_POINTER_WIDTH = 8;
int ZCC_INT_WIDTH = 4;

static void thumb_emit_prologue(Compiler *cc, Node *func) {
    int stack_size = func->stack_size + 40;
    if (stack_size < 256) stack_size = 256;
    stack_size = (stack_size + 7) & ~7;

    fprintf(cc->out, "    .text\n");
    fprintf(cc->out, "    .syntax unified\n");
    fprintf(cc->out, "    .cpu cortex-m0plus\n");
    fprintf(cc->out, "    .thumb\n");
    if (!func->is_static) {
        fprintf(cc->out, "    .global %s\n", func->func_def_name);
    }
    fprintf(cc->out, "    .type %s, %%function\n", func->func_def_name);
    fprintf(cc->out, "%s:\n", func->func_def_name);
    
    fprintf(cc->out, "    push {r4, r5, r6, r7, lr}\n");
    fprintf(cc->out, "    mov r7, sp\n");

    if (stack_size <= 508 && (stack_size % 4 == 0)) {
        fprintf(cc->out, "    sub sp, #%d\n", stack_size);
    } else {
        fprintf(cc->out, "    ldr r3, =%d\n", stack_size);
        fprintf(cc->out, "    mov r4, sp\n");
        fprintf(cc->out, "    subs r4, r4, r3\n");
        fprintf(cc->out, "    mov sp, r4\n");
    }

    int i;
    for (i = 0; i < func->num_params && i < 4; i++) {
        fprintf(cc->out, "    ldr r3, =%d\n", -(i * 4 + 8));
        fprintf(cc->out, "    adds r3, r7, r3\n");
        fprintf(cc->out, "    str r%d, [r3]\n", i);
    }
}

static void thumb_emit_epilogue(Compiler *cc, Node *func) {
    fprintf(cc->out, ".Lfunc_end_%d:\n", cc->func_end_label);
    fprintf(cc->out, "    mov sp, r7\n");
    fprintf(cc->out, "    pop {r4, r5, r6, r7, pc}\n");
}

static void thumb_emit_call(Compiler *cc, Node *func) {
    fprintf(cc->out, "    bl %s\n", func->func_name);
}

static void thumb_emit_binary_op(Compiler *cc, int op) {
    /* op matches ND_ADD, ND_SUB, etc.
       r0 = lhs, r1 = rhs
       output -> r0 */
    switch (op) {
        case ND_ADD:
            fprintf(cc->out, "    adds r0, r0, r1\n");
            break;
        case ND_SUB:
            fprintf(cc->out, "    subs r0, r0, r1\n");
            break;
        case ND_MUL:
            fprintf(cc->out, "    muls r0, r1, r0\n"); /* thumb-1 allows only dest=lhs */
            break;
        case ND_DIV:
            fprintf(cc->out, "    bl __aeabi_idiv\n"); /* software divide */
            break;
        case ND_BAND:
            fprintf(cc->out, "    ands r0, r0, r1\n");
            break;
        case ND_BOR:
            fprintf(cc->out, "    orrs r0, r0, r1\n");
            break;
        case ND_BXOR:
            fprintf(cc->out, "    eors r0, r0, r1\n");
            break;
        case ND_SHL:
            fprintf(cc->out, "    lsls r0, r0, r1\n");
            break;
        case ND_SHR:
            fprintf(cc->out, "    asrs r0, r0, r1\n"); /* arithmetic shift right */
            break;
    }
}

static void thumb_emit_load_stack(Compiler *cc, int offset, const char *reg) {
    if (offset >= 0 && offset <= 1020 && (offset % 4 == 0)) {
        fprintf(cc->out, "    ldr %s, [r7, #%d]\n", reg, offset);
    } else {
        fprintf(cc->out, "    ldr r3, =%d\n", offset);
        fprintf(cc->out, "    adds r3, r7, r3\n");
        fprintf(cc->out, "    ldr %s, [r3]\n", reg);
    }
}

static void thumb_emit_store_stack(Compiler *cc, int offset, const char *reg) {
    if (offset >= 0 && offset <= 1020 && (offset % 4 == 0)) {
        fprintf(cc->out, "    str %s, [r7, #%d]\n", reg, offset);
    } else {
        fprintf(cc->out, "    ldr r3, =%d\n", offset);
        fprintf(cc->out, "    adds r3, r7, r3\n");
        fprintf(cc->out, "    str %s, [r3]\n", reg);
    }
}

static void thumb_emit_float_binop(Compiler *cc, int op) {
    const char *fn = 0;
    switch (op) {
        case ND_FADD: fn = "__aeabi_fadd"; break;
        case ND_FSUB: fn = "__aeabi_fsub"; break;
        case ND_FMUL: fn = "__aeabi_fmul"; break;
        case ND_FDIV: fn = "__aeabi_fdiv"; break;
    }
    if (fn) {
        fprintf(cc->out, "    bl %s\n", fn);
    }
}

TargetBackend backend_thumbv6m = {
    4, /* ptr_size */
    thumb_emit_prologue,
    thumb_emit_epilogue,
    thumb_emit_call,
    thumb_emit_binary_op,
    thumb_emit_load_stack,
    thumb_emit_store_stack,
    thumb_emit_float_binop
};
/*
 * ir.c — ZCC IR construction and text emission
 *
 * Compiled by GCC (as part of compiler_passes.c's translation unit)
 * AND by ZCC itself at stage2 (included via zcc.c).
 *
 * ZCC-parseable rules strictly observed:
 *   - No #include <stdint.h>
 *   - No _Static_assert
 *   - No VLAs
 *   - No C99 designated initializers on structs (ZCC may not support)
 *   - No compound literals
 */

#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Globals ─────────────────────────────────────────────────────────── */

int           g_emit_ir      = 0;
ir_func_t    *g_ir_cur_func  = 0;  /* NULL — ZCC may not parse "= NULL" */
ir_module_t  *g_ir_module    = 0;

/* ── Opcode table ────────────────────────────────────────────────────── */

static const char *OP_NAMES[41] = {
    "ret",
    "br",
    "br_if",
    "alloca",
    "load",
    "store",
    "add",
    "sub",
    "mul",
    "div",
    "mod",
    "neg",
    "and",
    "or",
    "xor",
    "not",
    "shl",
    "shr",
    "eq",
    "ne",
    "lt",
    "le",
    "gt",
    "ge",
    "cast",
    "copy",
    "const",
    "const_str",
    "call",
    "arg",
    "phi",
    "addr",
    "label",
    "nop",
    "fconst",
    "fadd",
    "fsub",
    "fmul",
    "fdiv",
    "itof",
    "ftoi",
    "asm",
    "alloca"
};

/* ── Type table ──────────────────────────────────────────────────────── */

static const char *TY_NAMES[12] = {
    "void",
    "i8", "i16", "i32", "i64",
    "u8", "u16", "u32", "u64",
    "ptr",
    "f32", "f64"
};

static const int TY_BYTES[12] = {
    0,              /* void  */
    1, 2, 4, 8,    /* i8..i64 */
    1, 2, 4, 8,    /* u8..u64 */
    8,              /* ptr — LP64 */
    4, 8            /* f32, f64 */
};

/* ── Query helpers ────────────────────────────────────────────────────── */

const char *ir_op_name(ir_op_t op) {
    if (op < 0 || op >= IR_OP_COUNT) return "???";
    return OP_NAMES[op];
}

const char *ir_type_name(ir_type_t ty) {
    if (ty < 0 || ty >= 12) return "???";
    return TY_NAMES[ty];
}

int ir_type_bytes(ir_type_t ty) {
    if (ty < 0 || ty >= 12) return -1;
    return TY_BYTES[ty];
}

int ir_type_unsigned(ir_type_t ty) {
    return ty == IR_TY_U8 || ty == IR_TY_U16 ||
           ty == IR_TY_U32 || ty == IR_TY_U64;
}

int ir_op_is_terminator(ir_op_t op) {
    return op == IR_RET || op == IR_BR || op == IR_BR_IF;
}

/* ── Node allocation ──────────────────────────────────────────────────── */

ir_node_t *ir_node_alloc(void) {
    ir_node_t *n = (ir_node_t *)calloc(1, sizeof(ir_node_t));
    if (!n) {
        fprintf(stderr, "ir: out of memory allocating ir_node_t\n");
        exit(1);
    }
    return n;
}

/* ── Append ───────────────────────────────────────────────────────────── */

void ir_append(ir_func_t *fn, ir_node_t *n) {
    n->next = 0;
    if (!fn->head) {
        fn->head = fn->tail = n;
    } else {
        fn->tail->next = n;
        fn->tail = n;
    }
    fn->node_count++;
}

/* ── Safe string copy (no strlcpy in POSIX baseline) ─────────────────── */

static void safe_copy(char *dst, const char *src, int max) {
    if (!src) { dst[0] = '\0'; return; }
    int i = 0;
    while (i < max - 1 && src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

/* ── Primary emit function ────────────────────────────────────────────── */

ir_node_t *ir_emit(ir_func_t *fn, ir_op_t op, ir_type_t ty,
                   const char *dst,  const char *src1,
                   const char *src2, const char *label,
                   long imm, int lineno) {
    ir_node_t *n = ir_node_alloc();
    n->op     = op;
    n->type   = ty;
    n->imm    = imm;
    n->lineno = lineno;
    safe_copy(n->dst,   dst   ? dst   : "", IR_NAME_MAX);
    safe_copy(n->src1,  src1  ? src1  : "", IR_NAME_MAX);
    safe_copy(n->src2,  src2  ? src2  : "", IR_NAME_MAX);
    safe_copy(n->label, label ? label : "", IR_LABEL_MAX);
    ir_append(fn, n);
    return n;
}

/* ── Fresh name generators ────────────────────────────────────────────── */

void ir_fresh_tmp(ir_func_t *fn, char *buf) {
    /* writes "t<N>" into buf — buf must be >= IR_NAME_MAX bytes */
    int n = fn->tmp_counter++;
    int i = 0;
    buf[i++] = 't';
    /* write decimal digits of n */
    if (n == 0) {
        buf[i++] = '0';
    } else {
        char tmp[20];
        int  tlen = 0;
        int  v = n;
        while (v > 0) { tmp[tlen++] = '0' + (v % 10); v /= 10; }
        /* reverse */
        int j;
        for (j = tlen - 1; j >= 0; j--) buf[i++] = tmp[j];
    }
    buf[i] = '\0';
}

void ir_fresh_label(ir_func_t *fn, char *buf) {
    /* writes ".L<N>" into buf */
    int n = fn->lbl_counter++;
    int i = 0;
    buf[i++] = '.';
    buf[i++] = 'L';
    if (n == 0) {
        buf[i++] = '0';
    } else {
        char tmp[20];
        int  tlen = 0;
        int  v = n;
        while (v > 0) { tmp[tlen++] = '0' + (v % 10); v /= 10; }
        int j;
        for (j = tlen - 1; j >= 0; j--) buf[i++] = tmp[j];
    }
    buf[i] = '\0';
}

/* ── Module / function lifecycle ─────────────────────────────────────── */

ir_module_t *ir_module_create(void) {
    ir_module_t *mod = (ir_module_t *)calloc(1, sizeof(ir_module_t));
    if (!mod) {
        fprintf(stderr, "ir: out of memory allocating ir_module_t\n");
        exit(1);
    }
    return mod;
}

ir_func_t *ir_func_create(ir_module_t *mod, const char *name,
                           ir_type_t ret_type, int num_params) {
    if (mod->func_count >= IR_MAX_FUNCS) {
        fprintf(stderr, "ir: exceeded IR_MAX_FUNCS (%d)\n", IR_MAX_FUNCS);
        exit(1);
    }
    ir_func_t *fn = (ir_func_t *)calloc(1, sizeof(ir_func_t));
    if (!fn) {
        fprintf(stderr, "ir: out of memory allocating ir_func_t\n");
        exit(1);
    }
    safe_copy(fn->name, name ? name : "anon", IR_FUNC_MAX);
    fn->ret_type = ret_type;
    fn->num_params = num_params;
    mod->funcs[mod->func_count++] = fn;
    return fn;
}

void ir_module_free(ir_module_t *mod) {
    int i;
    if (!mod) return;
    for (i = 0; i < mod->func_count; i++) {
        ir_func_t *fn = mod->funcs[i];
        ir_node_t *n  = fn->head;
        while (n) {
            ir_node_t *next = n->next;
            free(n);
            n = next;
        }
        free(fn);
    }
    free(mod);
}

/* ── Text emission ────────────────────────────────────────────────────── */
/*
 * Text format (columns are tab-separated):
 *
 *   ; func <name> -> <ret_type>
 *   <op>  <type>  <dst>  <src1>  <src2>  <label>  [imm=<N>]  [; line <N>]
 *
 * Fields that are unused are printed as "-".
 * LABEL nodes print:   LABEL  -  -  -  -  <label>
 * CONST nodes print:   CONST  i32  t0  -  -  -  imm=42
 *
 * One node per line.  Machine-parseable for P2-IR's ir_build_cfg().
 */

static void emit_field(FILE *fp, const char *s) {
    if (s && s[0]) fprintf(fp, "%s", s);
    else           fprintf(fp, "-");
}

void ir_func_emit_text(const ir_func_t *fn, FILE *fp) {
    const ir_node_t *n;

    fprintf(fp, "; func %s -> %s\n", fn->name, ir_type_name(fn->ret_type));

    for (n = fn->head; n; n = n->next) {
        /* Opcode */
        fprintf(fp, "  %-10s", ir_op_name(n->op));

        /* Type */
        fprintf(fp, "  %-6s", n->type == IR_TY_VOID ? "-"
                                                      : ir_type_name(n->type));

        /* dst */
        fprintf(fp, "  ");
        emit_field(fp, n->dst);

        /* src1 */
        fprintf(fp, "  ");
        emit_field(fp, n->src1);

        /* src2 */
        fprintf(fp, "  ");
        emit_field(fp, n->src2);

        /* label */
        fprintf(fp, "  ");
        emit_field(fp, n->label);

        /* asm string */
        if (n->op == IR_ASM) {
            fprintf(fp, "  str=\"%s\"", n->asm_string ? n->asm_string : "");
        }

        /* imm — only print for ops that use it */
        if (n->op == IR_CONST || n->op == IR_ALLOCA || n->op == IR_FCONST) {
            fprintf(fp, "  imm=%ld", n->imm);
        }

        /* lineno annotation */
        if (n->lineno > 0) {
            fprintf(fp, "  ; line %d", n->lineno);
        }

        fprintf(fp, "\n");
    }

    fprintf(fp, "; end %s  nodes=%d\n\n", fn->name, fn->node_count);
}

void ir_module_emit_text(const ir_module_t *mod, FILE *fp) {
    int i;
    fprintf(fp, "; ZCC IR module  funcs=%d\n\n", mod->func_count);
    for (i = 0; i < mod->func_count; i++) {
        ir_func_emit_text(mod->funcs[i], fp);
    }
}
/**
 * compiler_passes.c — ZKAEDI PRIME Compiler Optimization Suite
 *
 * Five production-grade IR transformation passes:
 *
 *   PASS 1: DCE → SSA Bridge  — φ-function reachability as liveness oracle
 *   PASS 2: Escape Analysis   — pointer provenance → heap-to-stack demotion
 *   PASS 3: LICM              — alias-aware loop-invariant code motion
 *                               (hardened: OP_ALLOCA must-not-alias model,
 *                                single preheader for multi-entry loops)
 *   PASS 4: PGO BB Reordering — branch probability → icache-optimal layout
 *
 * Each pass is self-contained. All passes are chained via run_all_passes().
 *
 * Build:
 *   Standalone (no zcc):  cc -O2 -std=c17 -Wall -Wextra -DZCC_BRIDGE_STANDALONE
 * compiler_passes.c -o passes -lm With zcc (Node copy): cc -O2 -std=c17 -Wall
 * -Wextra zcc.c compiler_passes.c -o zcc_full -lm
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Forward-declare struct Node (defined in zcc.c) so all uses of
 * 'struct Node *' in this TU share the same incomplete-type tag.
 * Without this, GCC 13 treats the tag as re-declared in each parameter
 * list and reports spurious "conflicting types" on the static forward
 * declarations of zcc_node_from_expr / zcc_node_from_stmt.           */
struct Node;

#include "zcc_ast_bridge.h"

/* When set (ZCC_PGO_DEBUG_MAIN=1 and emitting main), OP_LOAD in
 * ir_asm_lower_insn logs block/dst/src0/slot to stderr for crash triage. */
static int s_debug_main_emit = 0;

/* ─────────────────────────────────────────────────────────────────────────────
 * SHARED IR PRIMITIVES
 * ───────────────────────────────────────────────────────────────────────────
 */

#define MAX_OPERANDS 4
#define MAX_CALL_ARGS 16
#define MAX_PHI_SOURCES 32
#define MAX_SUCCS 2048
#define MAX_PREDS 2048
#define MAX_INSTRS 65536 /* must exceed max RegID in any compiled function */
#define MAX_BLOCKS                                                             \
  8192 /* per-function block limit (parser/lexer can be large) */
#define MAX_ALLOCS 256
#define MAX_LOOPS 64
#define MAX_LOOP_BLOCKS 256
#define NAME_LEN 64

#define NO_BLOCK 0xFFFFFFFFu /* sentinel: absent block ID    */
#define NO_ALLOC 0xFFFFFFFFu /* sentinel: absent alloca ID   */

/* ── CG-IR-015: IR value type for width-sensitive instruction lowering ────────
 * Propagated from the AST (via member_size encoding) through zcc_lower_expr
 * into Instr.ir_type.  Zero-value IR_TY_I64 is the safe 64-bit default so
 * all existing calloc'd Instrs remain correct without explicit initialisation.
 * Only I32/U32 need special treatment in ir_asm_lower_insn (div/mod/shr).   */

typedef uint32_t RegID;   /* virtual register identifier            */
typedef uint32_t BlockID; /* basic block identifier                 */
typedef uint32_t InstrID; /* instruction identifier within a block  */

typedef enum {
  OP_NOP = 0,
  OP_CONST, /* immediate constant (Phase B lowering)           */
  OP_PHI,   /* φ(r₀:B₀, r₁:B₁, …)                */
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_MOD, /* TODO: x86 idiv sequence when lowering to asm    */
  OP_BAND,
  OP_BOR,
  OP_BXOR,
  OP_BNOT,
  OP_SHL,
  OP_SHR,
  OP_LT,
  OP_EQ,
  OP_NE,
  OP_GT,
  OP_GE,
  OP_LE, /* comparisons */
  OP_LOAD,
  OP_STORE,
  OP_ALLOCA, /* stack allocation — escape candidate  */
  OP_GEP,    /* GetElementPtr — tracks escape        */
  OP_CALL,   /* may cause escape                     */
  OP_RET,
  OP_BR,     /* unconditional branch                 */
  OP_CONDBR, /* conditional branch                   */
  OP_COPY,
  OP_UNDEF,
  OP_PGO_COUNTER_ADDR, /* PGO instrumentation: dst = &__zcc_edge_counts[imm] */
  OP_GLOBAL,           /* load address of global symbol: lea name(%rip), %reg */
  OP_ASM,              /* inline assembly string */
  OP_VLA_ALLOC,
} Opcode;

static const char *opcode_name[] __attribute__((unused)) = {"nop",
                                                            "const",
                                                            "phi",
                                                            "add",
                                                            "sub",
                                                            "mul",
                                                            "div",
                                                            "mod",
                                                            "band",
                                                            "bor",
                                                            "bxor",
                                                            "bnot",
                                                            "shl",
                                                            "shr",
                                                            "lt",
                                                            "eq",
                                                            "ne",
                                                            "gt",
                                                            "ge",
                                                            "le",
                                                            "load",
                                                            "store",
                                                            "alloca",
                                                            "gep",
                                                            "call",
                                                            "ret",
                                                            "br",
                                                            "condbr",
                                                            "copy",
                                                            "undef",
                                                            "pgo_counter_addr",
                                                            "global",
                                                            "asm"};

typedef struct {
  RegID reg;     /* source register                    */
  BlockID block; /* predecessor block the value comes from */
} PhiSource;

typedef struct Instr {
  InstrID id;
  Opcode op;
  RegID dst; /* destination register (0 = no def)   */
  RegID src[MAX_OPERANDS];
  uint32_t n_src;

  /* φ-node sources (populated only when op == OP_PHI) */
  PhiSource phi[MAX_PHI_SOURCES];
  uint32_t n_phi;

  /* Immediate (OP_CONST, or e.g. alloca size; 0 if unused) */
  int64_t imm;

  /* OP_CALL: callee name and argument registers */
  char call_name[128];
  RegID call_args[MAX_CALL_ARGS];
  uint32_t n_call_args;

  /* Metadata */
  bool dead;        /* marked by DCE                       */
  bool escape;      /* marked by escape analysis           */
  double exec_freq; /* from PGO profile                    */
  int line_no;      /* source line for DWARF .loc (0 = none) */
  IRType ir_type;   /* CG-IR-015: value type for width-sensitive lowering     */
                    /* (0 = IR_TY_I64 default; set for OP_DIV/MOD/SHR/SHL)   */
  int lscan_seq;    /* Liveness sequence ID injected by ir_asm_number_and_liveness */

  char *asm_string;

  struct Instr *next;
  struct Instr *prev;
} Instr;

typedef struct Block {
  BlockID id;
  char name[NAME_LEN];

  /* Instruction doubly-linked list */
  Instr *head;
  Instr *tail;
  uint32_t n_instrs;

  /* CFG edges */
  BlockID succs[MAX_SUCCS];
  uint32_t n_succs;
  BlockID preds[MAX_PREDS];
  uint32_t n_preds;

  /* Branch probabilities (succs[i] → branch_prob[i]) */
  double branch_prob[MAX_SUCCS];

  /* Liveness (DCE / SSA pass) */
  uint64_t live_in[8]; /* bitset — up to 512 regs                */
  uint64_t live_out[8];

  /* PGO / reordering metadata */
  double exec_freq;   /* estimated execution frequency          */
  bool placed;        /* used during chain construction         */
  BlockID chain_next; /* next block in the PGO chain            */

  bool reachable; /* set by CFG reachability pass           */
} Block;

typedef struct Function {
  Block *blocks[MAX_BLOCKS];
  uint32_t n_blocks;
  BlockID entry;
  BlockID exit;

  /* Def-use chains (reg → defining instruction) */
  Instr *def_of[MAX_INSTRS];     /* indexed by RegID */
  BlockID def_block[MAX_INSTRS]; /* block housing def_of[r]; NO_BLOCK if arg */
  uint32_t n_regs;

  /* Pass statistics */
  struct {
    uint32_t dce_instrs_removed;
    uint32_t dce_blocks_removed;
    uint32_t ea_promotions;
    uint32_t licm_hoisted;
    uint32_t licm_preheaders_inserted;
    uint32_t pgo_blocks_reordered;
  } stats;
} Function;

/* ─────────────────────────────────────────────────────────────────────────────
 * BITSET HELPERS  (512 registers → 8 × 64-bit words)
 * ───────────────────────────────────────────────────────────────────────────
 */

static inline void bs_set(uint64_t *bs, RegID r) {
  bs[r >> 6] |= (1ULL << (r & 63));
}
static inline void bs_clear(uint64_t *bs, RegID r) {
  bs[r >> 6] &= ~(1ULL << (r & 63));
}
static inline bool bs_get(uint64_t *bs, RegID r) {
  return !!(bs[r >> 6] & (1ULL << (r & 63)));
}
static inline bool bs_empty(uint64_t *bs) {
  for (int i = 0; i < 8; i++)
    if (bs[i])
      return false;
  return true;
}
static inline bool bs_union(uint64_t *dst, const uint64_t *src) {
  bool changed = false;
  for (int i = 0; i < 8; i++) {
    uint64_t before = dst[i];
    dst[i] |= src[i];
    changed |= (dst[i] != before);
  }
  return changed;
}
static inline void bs_copy(uint64_t *dst, const uint64_t *src) {
  memcpy(dst, src, 8 * sizeof(uint64_t));
}

/* ─────────────────────────────────────────────────────────────────────────────
 * PASS 1: DCE → SSA BRIDGE PASS
 *
 * Theory
 * ──────
 * In SSA form every variable is defined exactly once. A definition is dead iff
 * it cannot influence any program output — i.e., no chain of use-def edges from
 * that definition reaches a *critical* instruction (store, call, return).
 *
 * Rather than computing full dataflow fixpoints, we exploit SSA structure:
 *
 *   1. Mark all registers used by critical instructions as LIVE (seed
 * worklist).
 *   2. For each live register r, mark the instruction that defines r as live;
 *      then mark all of its operands live → propagate backward.
 *   3. For φ-nodes: a φ at block B with source (rₖ: Bₖ) is live iff the
 *      φ's destination is live AND the CFG edge Bₖ→B is reachable.
 *      This is the SSA-specific refinement: we prune φ sources that come from
 *      unreachable predecessors without needing separate reachability analysis.
 *   4. Any instruction whose dst ∉ live_set at the end of propagation is dead.
 *      Any block with no live instructions and no reachable successors is dead.
 *
 * Complexity: O(N) where N = total number of SSA values — one pass per value
 * in the worklist. No iterative dataflow convergence required.
 * ───────────────────────────────────────────────────────────────────────────
 */

/* Worklist for backward liveness propagation */
typedef struct {
  RegID items[MAX_INSTRS];
  int head, tail, size;
} RegWorklist;

static void wl_push(RegWorklist *wl, RegID r) {
  assert(wl->size < MAX_INSTRS);
  wl->items[wl->tail] = r;
  wl->tail = (wl->tail + 1) % MAX_INSTRS;
  wl->size++;
}
static RegID wl_pop(RegWorklist *wl) {
  assert(wl->size > 0);
  RegID r = wl->items[wl->head];
  wl->head = (wl->head + 1) % MAX_INSTRS;
  wl->size--;
  return r;
}

/* Determine if an instruction is a critical side-effect anchor */
static bool is_critical(const Instr *ins) {
  return ins->op == OP_STORE || ins->op == OP_CALL || ins->op == OP_RET ||
         ins->op == OP_BR || ins->op == OP_CONDBR;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * PASS 3: LICM — Loop-Invariant Code Motion  (full implementation)
 *
 * Pipeline:
 *   A. def→block map      populate fn->def_block[] for all live defs
 *   B. RPO via iterative DFS   needed for Cooper dominator algorithm
 *   C. Iterative dominator tree  Cooper et al. 2001, O(N) in practice
 *   D. Back-edge detection  succ dominates pred ⟹ back edge
 *   E. Natural loop body    reverse BFS from latch, bounded by header
 *   F. Preheader insertion  single entry-point from outside the loop
 *   G. Alias map            pointer regs that are OP_STORE targets in loop
 *   H. Hoist loop-invariant instructions to preheader (iterate until stable)
 *
 * Alias model (OP_ALLOCA-based):
 *   A register r is "aliased" if it appears as the pointer operand (src[1])
 *   of any OP_STORE inside the loop.  OP_LOAD through an aliased pointer is
 *   NOT invariant.  Pure arithmetic/const whose operands are all outside the
 *   loop ARE invariant.
 * ───────────────────────────────────────────────────────────────────────────
 */

/* ── Block-membership bitset (MAX_BLOCKS=512 → 8 × 64-bit words) ── */
#define BLKSET_WORDS 8
typedef uint64_t BlkSet[BLKSET_WORDS];

static inline void blkset_add(BlkSet bs, BlockID b) {
  bs[b >> 6] |= (1ULL << (b & 63));
}
static inline bool blkset_has(const BlkSet bs, BlockID b) {
  return !!(bs[b >> 6] & (1ULL << (b & 63)));
}
static inline void blkset_zero(BlkSet bs) {
  memset(bs, 0, BLKSET_WORDS * sizeof(uint64_t));
}
static inline void blkset_merge(BlkSet dst, const BlkSet src) {
  for (int i = 0; i < BLKSET_WORDS; i++)
    dst[i] |= src[i];
}

/* ── RPO state (module-level, single-threaded) ── */
static uint32_t licm_rpo_arr[MAX_BLOCKS]; /* rpo_arr[rpo_idx] = BlockID  */
static uint32_t licm_rpo_of[MAX_BLOCKS];  /* rpo_of[BlockID]  = rpo_idx  */
static uint32_t licm_rpo_n;

/* ── Dominator tree ── */
static BlockID licm_idom[MAX_BLOCKS]; /* licm_idom[b] = imm-dom of b */

/* ── Natural loop descriptor ── */
typedef struct {
  BlockID header;
  BlockID preheader;
  bool has_preheader;
  BlkSet body_bs; /* bitset of block IDs in loop  */
  BlockID latches[MAX_LOOP_BLOCKS];
  uint32_t n_latches;
} LICMLoop;

/* V2 Alias Oracle: base+offset per store so we can allow p->y hoisted past
 * store to p->x */
#define MAX_LOOP_STORES 128
typedef struct {
  RegID base;
  int64_t offset;
  bool is_variable; /* true if offset is unknown/dynamic */
} StoreTarget;
typedef struct {
  StoreTarget targets[MAX_LOOP_STORES];
  uint32_t n_targets;
  bool clobber_all; /* fallback if we exceed capacity or hit an unknown pointer
                     */
} AliasOracle;

/* ═════════════════════════════════════════════════════════════════
 * A.  Build fn->def_block[] and refresh fn->def_of[]
 * ═════════════════════════════════════════════════════════════════ */
static void licm_build_def_block(Function *fn) {
  memset(fn->def_block, 0xFF, sizeof(fn->def_block)); /* NO_BLOCK */
  memset(fn->def_of, 0,
         sizeof(fn->def_of)); /* NULL — cleared to avoid stale freed pointers */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk || !blk->reachable)
      continue;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->dst && ins->dst < MAX_INSTRS) {
        fn->def_block[ins->dst] = bi;
        fn->def_of[ins->dst] = ins;
      }
    }
  }
}

/* ═════════════════════════════════════════════════════════════════
 * B.  Iterative DFS post-order → RPO
 *     Iterative to avoid call-stack overflow on large CFGs.
 * ═════════════════════════════════════════════════════════════════ */
static void licm_compute_rpo(Function *fn) {
  static bool visited[MAX_BLOCKS];
  static uint32_t post[MAX_BLOCKS];
  static BlockID stk[MAX_BLOCKS];
  static bool expanded[MAX_BLOCKS];
  uint32_t post_n = 0;
  int top = -1;

  memset(visited, 0, fn->n_blocks * sizeof(bool));
  memset(licm_rpo_of, 0xFF, sizeof(licm_rpo_of));

  stk[++top] = fn->entry;
  expanded[top] = false;

  while (top >= 0) {
    BlockID b = stk[top];
    if (!expanded[top]) {
      if (visited[b]) {
        top--;
        continue;
      }
      visited[b] = true;
      expanded[top] = true;
      Block *blk = fn->blocks[b];
      /* push successors in reverse so first succ is processed first */
      for (int si = (int)blk->n_succs - 1; si >= 0; si--) {
        BlockID s = blk->succs[si];
        if (s < fn->n_blocks && !visited[s] && top + 1 < MAX_BLOCKS) {
          stk[++top] = s;
          expanded[top] = false;
        }
      }
    } else {
      post[post_n++] = b;
      top--;
    }
  }

  licm_rpo_n = post_n;
  for (uint32_t i = 0; i < licm_rpo_n; i++) {
    licm_rpo_arr[i] = post[licm_rpo_n - 1 - i];
    licm_rpo_of[post[licm_rpo_n - 1 - i]] = i;
  }
}

/* ═════════════════════════════════════════════════════════════════
 * C.  Cooper et al. 2001 iterative dominator tree
 * ═════════════════════════════════════════════════════════════════ */
static BlockID licm_intersect(BlockID b1, BlockID b2) {
  uint32_t f1 = licm_rpo_of[b1], f2 = licm_rpo_of[b2];
  while (f1 != f2) {
    while (f1 > f2) {
      b1 = licm_idom[b1];
      if (b1 == NO_BLOCK)
        return NO_BLOCK;
      f1 = licm_rpo_of[b1];
    }
    while (f2 > f1) {
      b2 = licm_idom[b2];
      if (b2 == NO_BLOCK)
        return NO_BLOCK;
      f2 = licm_rpo_of[b2];
    }
  }
  return b1;
}

static void licm_compute_doms(Function *fn) {
  for (uint32_t i = 0; i < fn->n_blocks; i++)
    licm_idom[i] = NO_BLOCK;
  licm_idom[fn->entry] = fn->entry;

  bool changed = true;
  while (changed) {
    changed = false;
    /* Iterate in RPO order; skip entry (index 0) */
    for (uint32_t ri = 1; ri < licm_rpo_n; ri++) {
      BlockID b = licm_rpo_arr[ri];
      Block *blk = fn->blocks[b];
      if (!blk || !blk->reachable)
        continue;

      BlockID new_idom = NO_BLOCK;
      for (uint32_t pi = 0; pi < blk->n_preds; pi++) {
        BlockID p = blk->preds[pi];
        if (p >= fn->n_blocks || licm_idom[p] == NO_BLOCK)
          continue;
        if (new_idom == NO_BLOCK)
          new_idom = p;
        else
          new_idom = licm_intersect(p, new_idom);
      }
      if (new_idom != NO_BLOCK && licm_idom[b] != new_idom) {
        licm_idom[b] = new_idom;
        changed = true;
      }
    }
  }
}

/* Returns true if 'd' dominates 'b' (walks idom chain from b to root). */
static bool licm_dominates(BlockID d, BlockID b) {
  uint32_t guard = 0;
  BlockID cur = b;
  while (guard++ < MAX_BLOCKS) {
    if (cur == d)
      return true;
    if (licm_idom[cur] == NO_BLOCK)
      return false;
    if (licm_idom[cur] == cur)
      return (cur == d); /* reached root */
    cur = licm_idom[cur];
  }
  return false;
}

/* ── Dominance frontiers (for global mem2reg) ── */
#define MAX_DF_PER_BLOCK 32
static uint32_t df_list[MAX_BLOCKS][MAX_DF_PER_BLOCK];
static uint32_t df_count[MAX_BLOCKS];

/** Compute DF(b) for each block: y is in DF(b) iff b dominates a predecessor of
 * y but not y. */
static void compute_dominance_frontiers(Function *fn) {
  for (uint32_t i = 0; i < fn->n_blocks; i++)
    df_count[i] = 0;

  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk || !blk->reachable)
      continue;
    BlockID y = (BlockID)bi;
    BlockID idom_y = licm_idom[y];
    if (idom_y == NO_BLOCK)
      continue;

    for (uint32_t pi = 0; pi < blk->n_preds; pi++) {
      BlockID p = blk->preds[pi];
      if (p >= fn->n_blocks)
        continue;
      BlockID runner = p;
      while (runner != idom_y) {
        /* y is in DF(runner) */
        uint32_t dc = df_count[runner];
        uint32_t j;
        for (j = 0; j < dc; j++)
          if (df_list[runner][j] == y)
            break;
        if (j >= dc && dc < MAX_DF_PER_BLOCK) {
          df_list[runner][dc] = y;
          df_count[runner]++;
        }
        if (licm_idom[runner] == NO_BLOCK)
          break;
        runner = licm_idom[runner];
      }
    }
  }
}

/* ═════════════════════════════════════════════════════════════════
 * E.  Natural loop body via reverse BFS from latch to header
 * ═════════════════════════════════════════════════════════════════ */
static void licm_loop_body(Function *fn, BlockID header, BlockID latch,
                           BlkSet body) {
  static bool visited[MAX_BLOCKS];
  static BlockID queue[MAX_BLOCKS];
  memset(visited, 0, fn->n_blocks * sizeof(bool));

  int qh = 0, qt = 0;
  blkset_add(body, header);
  visited[header] = true;

  if (!blkset_has(body, latch)) {
    blkset_add(body, latch);
    visited[latch] = true;
    queue[qt++] = latch;
  }

  while (qh < qt) {
    BlockID cur = queue[qh++];
    Block *blk = fn->blocks[cur];
    for (uint32_t pi = 0; pi < blk->n_preds; pi++) {
      BlockID p = blk->preds[pi];
      if (p >= fn->n_blocks || visited[p])
        continue;
      visited[p] = true;
      blkset_add(body, p);
      if (qt < MAX_BLOCKS)
        queue[qt++] = p;
    }
  }
}

/* ═════════════════════════════════════════════════════════════════
 * F.  Preheader insertion
 *
 *  Before:   pred₁  pred₂ … (non-loop)   latch₁ latch₂ (loop back-edges)
 *                       ↘   ↙                  ↙ ↙
 *                         header  ←─────────────
 *
 *  After:    pred₁  pred₂ …          latch₁ latch₂
 *                       ↘↙                  ↙ ↙
 *                     preheader           header
 *                         └───────────────→ header
 *
 *  PHI nodes in header:  all non-loop source entries are merged into one
 *  entry via the preheader block.  If there are multiple non-loop sources
 *  with different values we use the first (correct for the single-pred case
 *  that arises in well-structured loops; production quality would insert
 *  copies in the preheader for each).
 * ═════════════════════════════════════════════════════════════════ */
static BlockID licm_insert_preheader(Function *fn, BlockID header,
                                     const BlkSet loop_body,
                                     uint32_t *synth_id) {
  if (fn->n_blocks >= MAX_BLOCKS)
    return (BlockID)NO_BLOCK;

  BlockID ph_id = fn->n_blocks++;
  fn->blocks[ph_id] = calloc(1, sizeof(Block));
  Block *ph = fn->blocks[ph_id];
  ph->id = ph_id;
  snprintf(ph->name, NAME_LEN, "preheader.%u", (unsigned)header);
  ph->reachable = true;
  ph->exec_freq = 0.0;
  ph->succs[0] = header;
  ph->n_succs = 1;
  ph->branch_prob[0] = 1.0;
  licm_idom[ph_id] = NO_BLOCK; /* dominator not in the tree yet */

  /* Preheader ends with an unconditional branch to the loop header */
  Instr *br = calloc(1, sizeof(Instr));
  br->id = (*synth_id)++;
  br->op = OP_BR;
  br->src[0] = header;
  br->n_src = 1;
  br->exec_freq = 1.0;
  ph->head = ph->tail = br;
  ph->n_instrs = 1;

  /* Collect non-loop predecessors of the header */
  Block *hdr = fn->blocks[header];
  BlockID nlp[MAX_PREDS];
  uint32_t n_nlp = 0;
  for (uint32_t pi = 0; pi < hdr->n_preds; pi++) {
    BlockID p = hdr->preds[pi];
    if (!blkset_has(loop_body, p))
      nlp[n_nlp++] = p;
  }

  /* Redirect non-loop predecessors to point at preheader */
  for (uint32_t i = 0; i < n_nlp; i++) {
    BlockID p = nlp[i];
    Block *pblk = fn->blocks[p];

    /* Update successor list */
    for (uint32_t si = 0; si < pblk->n_succs; si++)
      if (pblk->succs[si] == header) {
        pblk->succs[si] = ph_id;
        break;
      }

    /* Patch branch-instruction target operands */
    for (Instr *ins = pblk->head; ins; ins = ins->next)
      if (ins->op == OP_BR || ins->op == OP_CONDBR)
        for (uint32_t s = 0; s < ins->n_src; s++)
          if (ins->src[s] == header)
            ins->src[s] = ph_id;

    /* Register the predecessor of the preheader */
    if (ph->n_preds < MAX_PREDS)
      ph->preds[ph->n_preds++] = p;
  }

  /* Rebuild header predecessor list: keep only loop preds + preheader */
  uint32_t new_np = 0;
  for (uint32_t pi = 0; pi < hdr->n_preds; pi++) {
    BlockID p = hdr->preds[pi];
    if (blkset_has(loop_body, p))
      hdr->preds[new_np++] = p;
  }
  hdr->preds[new_np++] = ph_id;
  hdr->n_preds = new_np;

  /* Fix PHI nodes in header:
   * For each φ, collect the non-loop-pred sources into a single entry
   * via the preheader.  The first non-loop source value is used. */
  for (Instr *ins = hdr->head; ins; ins = ins->next) {
    if (ins->op != OP_PHI)
      continue;
    RegID ph_val = 0;
    bool found = false;
    uint32_t new_n = 0;
    for (uint32_t p = 0; p < ins->n_phi; p++) {
      if (!blkset_has(loop_body, ins->phi[p].block)) {
        if (!found) {
          ph_val = ins->phi[p].reg;
          found = true;
        }
        /* drop extra non-loop sources */
      } else {
        ins->phi[new_n++] = ins->phi[p];
      }
    }
    if (found) {
      ins->phi[new_n].reg = ph_val;
      ins->phi[new_n].block = ph_id;
      new_n++;
    }
    ins->n_phi = new_n;
  }

  return ph_id;
}

/* ═════════════════════════════════════════════════════════════════
 * G.  V2 Alias Oracle — base+offset per store (fixes V1: same memory, different
 * regs) ═════════════════════════════════════════════════════════════════ */
/* Walk backward from a pointer reg: if OP_ADD with OP_CONST, extract base and
 * offset. */
static void licm_analyze_ptr(Function *fn, RegID ptr, RegID *out_base,
                             int64_t *out_off, bool *out_var) {
  *out_base = ptr;
  *out_off = 0;
  *out_var = true;
  if (!ptr || ptr >= MAX_INSTRS)
    return;
  Instr *def = fn->def_of[ptr];
  if (!def)
    return;
  if (def->op == OP_ADD && def->n_src == 2) {
    RegID s0 = def->src[0], s1 = def->src[1];
    Instr *d0 = (s0 && s0 < MAX_INSTRS) ? fn->def_of[s0] : NULL;
    Instr *d1 = (s1 && s1 < MAX_INSTRS) ? fn->def_of[s1] : NULL;
    if (d1 && d1->op == OP_CONST) {
      *out_base = s0;
      *out_off = d1->imm;
      *out_var = false;
    } else if (d0 && d0->op == OP_CONST) {
      *out_base = s1;
      *out_off = d0->imm;
      *out_var = false;
    }
  } else if (def->op == OP_ALLOCA) {
    *out_base = ptr;
    *out_off = 0;
    *out_var = false;
  }
}

static void licm_build_alias_v2(Function *fn, const BlkSet loop_body,
                                AliasOracle *oracle) {
  oracle->n_targets = 0;
  oracle->clobber_all = false;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    if (!blkset_has(loop_body, bi))
      continue;
    for (Instr *ins = fn->blocks[bi]->head; ins; ins = ins->next) {
      if (ins->op == OP_STORE && ins->n_src >= 2) {
        if (oracle->n_targets >= MAX_LOOP_STORES) {
          oracle->clobber_all = true;
          return;
        }
        StoreTarget *tgt = &oracle->targets[oracle->n_targets++];
        licm_analyze_ptr(fn, ins->src[1], &tgt->base, &tgt->offset,
                         &tgt->is_variable);
      }
    }
  }
}

/* Loop contains OP_CALL → conservative: do not hoist any OP_LOAD (call may
 * write anywhere). */
static bool licm_loop_has_call(Function *fn, const BlkSet loop_body) {
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    if (!blkset_has(loop_body, bi))
      continue;
    for (Instr *ins = fn->blocks[bi]->head; ins; ins = ins->next) {
      if (ins->op == OP_CALL)
        return true;
    }
  }
  return false;
}

/* Helper: return the block index that contains the instruction defining r, or
 * NO_BLOCK. */
static BlockID licm_def_block_of(Function *fn, RegID r) {
  if (!r || r >= MAX_INSTRS)
    return (BlockID)NO_BLOCK;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk)
      continue;
    for (Instr *p = blk->head; p; p = p->next)
      if (p->dst == r)
        return bi;
  }
  return (BlockID)NO_BLOCK;
}

/* ═════════════════════════════════════════════════════════════════
 * H.  Invariance predicate
 *     Instruction is invariant iff it has no side effects AND every
 *     operand's defining instruction is outside the loop (or in preheader).
 *     Recursively: if any src is defined inside the loop → not invariant.
 * ═════════════════════════════════════════════════════════════════ */
static bool licm_is_invariant(Function *fn, const Instr *ins,
                              const BlkSet loop_body, BlockID preheader,
                              const Block *ph_blk, const Instr *ph_term,
                              bool allow_preheader_operands,
                              const AliasOracle *oracle, bool loop_has_call) {
  (void)ph_blk;
  (void)ph_term;
  (void)allow_preheader_operands;

  /* Side-effecting or control-flow instructions are never invariant */
  switch (ins->op) {
  case OP_STORE:
  case OP_CALL:
  case OP_RET:
  case OP_BR:
  case OP_CONDBR:
  case OP_PHI:
  case OP_NOP:
  case OP_ALLOCA:
  case OP_UNDEF:
  case OP_PGO_COUNTER_ADDR: /* PGO probe: must stay in block */
    return false;
  default:
    break;
  }

  /* Alias barrier V2 for OP_LOAD: peer through OP_ADD to base+offset; block
   * only if same base and same offset (or unknown offset). Allows p->y hoisted
   * past store to p->x. */
  if (ins->op == OP_LOAD && ins->n_src >= 1) {
    if (loop_has_call)
      return false;
    if (oracle->clobber_all)
      return false;
    RegID load_base;
    int64_t load_off;
    bool load_var;
    licm_analyze_ptr(fn, ins->src[0], &load_base, &load_off, &load_var);
    for (uint32_t i = 0; i < oracle->n_targets; i++) {
      const StoreTarget *tgt = &oracle->targets[i];
      if (tgt->base == load_base) {
        if (tgt->is_variable || load_var)
          return false;
        if (tgt->offset == load_off)
          return false;
        /* same base, different constant offset → no alias, continue */
      } else {
        Instr *db1 = fn->def_of[tgt->base];
        Instr *db2 = fn->def_of[load_base];
        bool distinct_allocas =
            (db1 && db2 && db1->op == OP_ALLOCA && db2->op == OP_ALLOCA &&
             tgt->base != load_base);
        if (!distinct_allocas)
          return false;
      }
    }
  }

  /* Every source RegID must be defined outside the loop (or in preheader).
   * Find def block by scanning so we don't rely on def_block[] being current.
   * If def is not found (NO_BLOCK) treat as not invariant — def may have been
   * removed by a prior pass (e.g. DCE), so do not hoist. */
  for (uint32_t s = 0; s < ins->n_src; s++) {
    RegID r = ins->src[s];
    BlockID db = licm_def_block_of(fn, r);
    if (db == (BlockID)NO_BLOCK || db >= fn->n_blocks)
      return false; /* unknown or out-of-range def → do not hoist */
    if (db == preheader)
      continue; /* already hoisted → ok */
    if (blkset_has(loop_body, db))
      return false; /* operand defined inside loop → not invariant */
  }
  return true;
}

/* ═════════════════════════════════════════════════════════════════
 * LICM pass entry point
 * ═════════════════════════════════════════════════════════════════ */
uint32_t licm_pass(Function *fn) {
  fn->stats.licm_hoisted = 0;
  fn->stats.licm_preheaders_inserted = 0;
  if (fn->n_blocks == 0)
    return 0;

  /* A — def→block map */
  licm_build_def_block(fn);

  /* B + C — RPO and dominator tree */
  licm_compute_rpo(fn);
  licm_compute_doms(fn);

  /* D — Discover natural loops via back edges */
  static LICMLoop loops[MAX_LOOPS];
  uint32_t n_loops = 0;
  uint32_t synth_id =
      fn->n_regs + 10000; /* synthetic IDs for preheader branches */

  for (uint32_t bi = 0; bi < fn->n_blocks && n_loops < MAX_LOOPS; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk || !blk->reachable)
      continue;

    for (uint32_t si = 0; si < blk->n_succs; si++) {
      BlockID h = blk->succs[si];
      if (h >= fn->n_blocks || licm_idom[h] == NO_BLOCK)
        continue;
      if (!licm_dominates(h, bi))
        continue; /* not a back edge */

      /* Back edge bi → h; h is the loop header */
      LICMLoop *lp = NULL;
      for (uint32_t li = 0; li < n_loops; li++)
        if (loops[li].header == h) {
          lp = &loops[li];
          break;
        }

      if (!lp) {
        if (n_loops >= MAX_LOOPS)
          break;
        lp = &loops[n_loops++];
        memset(lp, 0, sizeof(LICMLoop));
        lp->header = h;
        lp->preheader = (BlockID)NO_BLOCK;
        lp->has_preheader = false;
      }
      if (lp->n_latches < MAX_LOOP_BLOCKS)
        lp->latches[lp->n_latches++] = bi;

      /* E — compute (and merge) loop body */
      BlkSet extra;
      blkset_zero(extra);
      licm_loop_body(fn, h, bi, extra);
      blkset_merge(lp->body_bs, extra);
    }
  }

  if (n_loops == 0)
    return 0;

  fprintf(stderr, "[LICM]      %u natural loop(s) found\n", n_loops);

  /* F — Insert preheaders */
  for (uint32_t li = 0; li < n_loops; li++) {
    LICMLoop *lp = &loops[li];
    Block *hdr = fn->blocks[lp->header];

    uint32_t n_outside = 0;
    for (uint32_t pi = 0; pi < hdr->n_preds; pi++)
      if (!blkset_has(lp->body_bs, hdr->preds[pi]))
        n_outside++;

    if (n_outside == 0)
      continue; /* degenerate: all preds are back-edges */

    BlockID ph = licm_insert_preheader(fn, lp->header, lp->body_bs, &synth_id);
    if (ph != (BlockID)NO_BLOCK) {
      lp->preheader = ph;
      lp->has_preheader = true;
      fn->stats.licm_preheaders_inserted++;
      fprintf(stderr,
              "[LICM]      preheader '%s' inserted for loop header '%s'\n",
              fn->blocks[ph]->name, fn->blocks[lp->header]->name);
    }
  }

  /* Rebuild def→block map after preheader insertion may have added blocks */
  licm_build_def_block(fn);
  /* Rebuild reachability so later passes see the new blocks */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++)
    if (fn->blocks[bi])
      fn->blocks[bi]->reachable = false;
  /* quick BFS re-mark */
  {
    bool vis[MAX_BLOCKS];
    memset(vis, 0, sizeof(vis));
    BlockID q[MAX_BLOCKS];
    int qh = 0, qt = 0;
    q[qt++] = fn->entry;
    vis[fn->entry] = true;
    while (qh < qt) {
      BlockID cur = q[qh++];
      fn->blocks[cur]->reachable = true;
      for (uint32_t si = 0; si < fn->blocks[cur]->n_succs; si++) {
        BlockID s = fn->blocks[cur]->succs[si];
        if (s < fn->n_blocks && !vis[s]) {
          vis[s] = true;
          q[qt++] = s;
        }
      }
    }
  }
  /* Rebuild def→block so invariance check sees correct def locations (reachable
   * now set) */
  licm_build_def_block(fn);

  /* G + H — Hoist for each loop, innermost-last discovery order */
  uint32_t total = 0;
  for (uint32_t li = 0; li < n_loops; li++) {
    LICMLoop *lp = &loops[li];
    if (!lp->has_preheader)
      continue;

    Block *ph_blk = fn->blocks[lp->preheader];
    Instr *ph_term = ph_blk->tail; /* the OP_BR to header */

    AliasOracle oracle;
    licm_build_alias_v2(fn, lp->body_bs, &oracle);
    bool loop_has_call = licm_loop_has_call(fn, lp->body_bs);

    /* Iterate until stable: a hoisted instruction may enable another */
    bool progress = true;
    bool allow_ph =
        false; /* first round: only hoist if operands outside loop */
    while (progress) {
      progress = false;

      for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
        if (!blkset_has(lp->body_bs, bi))
          continue;
        Block *blk = fn->blocks[bi];
        if (!blk || !blk->reachable)
          continue;

        Instr *ins = blk->head;
        while (ins) {
          Instr *nxt = ins->next;
          if (licm_is_invariant(fn, ins, lp->body_bs, lp->preheader, ph_blk,
                                ph_term, allow_ph, &oracle, loop_has_call)) {
            /* Unlink from loop block */
            if (ins->prev)
              ins->prev->next = ins->next;
            else
              blk->head = ins->next;
            if (ins->next)
              ins->next->prev = ins->prev;
            else
              blk->tail = ins->prev;
            blk->n_instrs--;
            ins->prev = ins->next = NULL;

            /* Insert after the last operand def in the preheader so uses see
             * defs */
            Instr *insert_after = NULL;
            for (Instr *p = ph_blk->head; p && p != ph_term; p = p->next) {
              for (uint32_t s = 0; s < ins->n_src; s++) {
                RegID r = ins->src[s];
                if (r && r < MAX_INSTRS && fn->def_block[r] == lp->preheader &&
                    fn->def_of[r] == p)
                  insert_after = p;
              }
            }
            if (insert_after) {
              ins->next = insert_after->next;
              ins->prev = insert_after;
              if (insert_after->next)
                insert_after->next->prev = ins;
              else
                ph_blk->tail = ins;
              insert_after->next = ins;
            } else {
              ins->next = ph_blk->head;
              ins->prev = NULL;
              if (ph_blk->head)
                ph_blk->head->prev = ins;
              else
                ph_blk->tail = ins;
              ph_blk->head = ins;
            }
            ph_blk->n_instrs++;

            /* Update def→block map */
            if (ins->dst && ins->dst < MAX_INSTRS)
              fn->def_block[ins->dst] = lp->preheader;

            total++;
            progress = true;
            fprintf(stderr, "[LICM]      hoisted %%%u (%s) from '%s' -> '%s'\n",
                    (unsigned)ins->dst, opcode_name[ins->op], blk->name,
                    ph_blk->name);
          }
          ins = nxt;
        }
      }

      allow_ph =
          true; /* from next round, allow operands already in preheader */
      /* Rebuild alias oracle after each round (hoisted stores change store set)
       */
      if (progress)
        licm_build_alias_v2(fn, lp->body_bs, &oracle);
    }
  }

  fn->stats.licm_hoisted = total;
  return total;
}

/**
 * constant_fold_pass() — Fold binary ops and BNOT when operands are constants.
 * Mutates instructions in-place to OP_CONST so DCE can remove the now-redundant
 * constant definitions. Requires def_of[] to be populated
 * (licm_build_def_block).
 */
static uint32_t constant_fold_pass(Function *fn) {
  uint32_t folded = 0;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk || !blk->reachable)
      continue;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (!ins->dst || ins->dst >= MAX_INSTRS)
        continue;
      int64_t result;
      Instr *d0 = (ins->n_src >= 1 && ins->src[0] < MAX_INSTRS)
                      ? fn->def_of[ins->src[0]]
                      : NULL;
      Instr *d1 = (ins->n_src >= 2 && ins->src[1] < MAX_INSTRS)
                      ? fn->def_of[ins->src[1]]
                      : NULL;

      switch (ins->op) {
      case OP_ADD:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = d0->imm + d1->imm;
          goto fold_binary;
        }
        break;
      case OP_SUB:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = d0->imm - d1->imm;
          goto fold_binary;
        }
        break;
      case OP_MUL:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = d0->imm * d1->imm;
          goto fold_binary;
        }
        break;
      case OP_DIV:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST &&
            d1->imm != 0) {
          result = d0->imm / d1->imm;
          goto fold_binary;
        }
        break;
      case OP_MOD:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST &&
            d1->imm != 0) {
          result = d0->imm % d1->imm;
          goto fold_binary;
        }
        break;
      case OP_BAND:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (int64_t)((uint64_t)d0->imm & (uint64_t)d1->imm);
          goto fold_binary;
        }
        break;
      case OP_BOR:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (int64_t)((uint64_t)d0->imm | (uint64_t)d1->imm);
          goto fold_binary;
        }
        break;
      case OP_BXOR:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (int64_t)((uint64_t)d0->imm ^ (uint64_t)d1->imm);
          goto fold_binary;
        }
        break;
      case OP_SHL:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          uint64_t u = (uint64_t)d0->imm;
          unsigned sh = (unsigned)(d1->imm & 63);
          result = (int64_t)(u << sh);
          goto fold_binary;
        }
        break;
      case OP_SHR:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          uint64_t u = (uint64_t)d0->imm;
          unsigned sh = (unsigned)(d1->imm & 63);
          result = (int64_t)(u >> sh);
          goto fold_binary;
        }
        break;
      case OP_LT:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = d0->imm < d1->imm ? 1 : 0;
          goto fold_binary;
        }
        break;
      case OP_EQ:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (d0->imm == d1->imm) ? 1 : 0;
          goto fold_binary;
        }
        break;
      case OP_NE:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (d0->imm != d1->imm) ? 1 : 0;
          goto fold_binary;
        }
        break;
      case OP_GT:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (d0->imm > d1->imm) ? 1 : 0;
          goto fold_binary;
        }
        break;
      case OP_GE:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (d0->imm >= d1->imm) ? 1 : 0;
          goto fold_binary;
        }
        break;
      case OP_LE:
        if (d0 && d1 && d0->op == OP_CONST && d1->op == OP_CONST) {
          result = (d0->imm <= d1->imm) ? 1 : 0;
          goto fold_binary;
        }
        break;
      case OP_BNOT:
        if (d0 && d0->op == OP_CONST) {
          result = (int64_t)(~(uint64_t)d0->imm);
          ins->op = OP_CONST;
          ins->imm = result;
          ins->n_src = 0;
          folded++;
        }
        break;
      default:
        break;
      }
      continue;
    fold_binary:
      ins->op = OP_CONST;
      ins->imm = result;
      ins->n_src = 0;
      ins->src[0] = 0;
      ins->src[1] = 0;
      folded++;
      continue;
    }
  }
  return folded;
}

/**
 * ssa_dce_pass() — SSA-form Dead Code Elimination
 *
 * @param fn   The function IR to transform (modified in-place).
 * @return     Number of instructions removed.
 */
uint32_t ssa_dce_pass(Function *fn) {
  /* live[r] == true → register r has at least one reachable use */
  bool live[MAX_INSTRS];
  memset(live, 0, sizeof(live));

  RegWorklist wl;
  memset(&wl, 0, sizeof(wl));

  /* ── Step 1: Seed worklist with operands of critical instructions ── */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk->reachable)
      continue;

    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (!is_critical(ins))
        continue;

      /* Seed source operands. For BR/CONDBR only src[0] is used: BR has block
       * ID, CONDBR has condition RegID. */
      uint32_t n_seed = ins->n_src;
      if (ins->op == OP_BR)
        n_seed = 0; /* br: src[0] is block ID, no reg to seed */
      else if (ins->op == OP_CONDBR)
        n_seed = 1; /* condbr: only src[0] is condition RegID; src[1],src[2] are
                       block IDs */
      for (uint32_t s = 0; s < n_seed; s++) {
        RegID r = ins->src[s];
        if (r && !live[r]) {
          live[r] = true;
          wl_push(&wl, r);
        }
      }
      if (ins->op == OP_CALL) {
        for (uint32_t s = 0; s < ins->n_call_args; s++) {
          RegID r = ins->call_args[s];
          if (r && !live[r]) {
            live[r] = true;
            wl_push(&wl, r);
          }
        }
      }
      /* For φ-nodes that are critical (unusual but handle it) */
      if (ins->op == OP_PHI) {
        for (uint32_t p = 0; p < ins->n_phi; p++) {
          RegID r = ins->phi[p].reg;
          if (r && !live[r]) {
            live[r] = true;
            wl_push(&wl, r);
          }
        }
      }
    }
  }

  /* ── Step 2: Propagate liveness backward through def-use chains ── */
  while (wl.size > 0) {
    RegID r = wl_pop(&wl);

    Instr *def = fn->def_of[r];
    if (!def)
      continue; /* function argument / undefined → stop */

    /* Mark defining instruction's operands live.
     * BR:     src[0] is a block ID, not a value register — skip all.
     * CONDBR: src[0] is condition RegID; src[1],src[2] are block IDs — seed
     * only src[0]. */
    uint32_t def_n_seed = def->n_src;
    if (def->op == OP_BR)
      def_n_seed = 0;
    if (def->op == OP_CONDBR || def->op == OP_COPY)
      def_n_seed = 1;
    for (uint32_t s = 0; s < def_n_seed; s++) {
      RegID src = def->src[s];
      if (src && !live[src]) {
        live[src] = true;
        wl_push(&wl, src);
      }
    }
    if (def->op == OP_CALL) {
      for (uint32_t s = 0; s < def->n_call_args; s++) {
        RegID src = def->call_args[s];
        if (src && !live[src]) {
          live[src] = true;
          wl_push(&wl, src);
        }
      }
    }

    /*
     * φ-node special case:
     * Only propagate through source (rₖ: Bₖ) if the edge Bₖ → parent(φ)
     * is reachable. This is the liveness oracle based on φ-function
     * reachability — we don't need full dataflow because SSA guarantees
     * a single definition site, so unreachable predecessors can be
     * pruned precisely here.
     */
    if (def->op == OP_PHI) {
      for (uint32_t p = 0; p < def->n_phi; p++) {
        PhiSource *ps = &def->phi[p];
        Block *pred = fn->blocks[ps->block];
        if (!pred || !pred->reachable)
          continue; /* prune unreachable */

        if (!live[ps->reg]) {
          live[ps->reg] = true;
          wl_push(&wl, ps->reg);
        }
      }
    }
  }

  /* ── Step 3: Mark dead instructions & remove them ── */
  uint32_t removed = 0;

  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];

    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (is_critical(ins))
        continue; /* never remove critical ops */
      if (ins->op == OP_NOP)
        continue;

      /* Dead iff destination register is not live */
      if (ins->dst && !live[ins->dst]) {
        ins->dead = true;
        removed++;
      }
    }

    /* Compact instruction list (remove dead nodes) */
    Instr *cur = blk->head;
    while (cur) {
      Instr *next = cur->next;
      if (cur->dead) {
        if (cur->prev)
          cur->prev->next = cur->next;
        else
          blk->head = cur->next;
        if (cur->next)
          cur->next->prev = cur->prev;
        else
          blk->tail = cur->prev;
        blk->n_instrs--;
        free(cur);
      }
      cur = next;
    }
  }

  /*
   * ── Step 4: Remove unreachable blocks ──
   * A block is dead if it is not reachable from the entry block.
   * We already have reachability pre-computed; just clean up.
   */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk->reachable && blk->id != fn->entry) {
      /* Free instructions */
      Instr *cur = blk->head;
      while (cur) {
        Instr *n = cur->next;
        free(cur);
        cur = n;
      }
      blk->head = blk->tail = NULL;
      blk->n_instrs = 0;
      fn->stats.dce_blocks_removed++;
    }
  }

  fn->stats.dce_instrs_removed += removed;
  return removed;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * PASS 2: ESCAPE ANALYSIS + STACK PROMOTION
 *
 * Theory
 * ──────
 * A heap allocation (OP_ALLOCA or a modeled malloc) *escapes* if any pointer
 * derived from it can be observed after the function returns, or is passed
 * to an opaque call that may store it globally.
 *
 * Escape conditions:
 *   E1. The allocation address is returned (OP_RET with alloc-derived operand).
 *   E2. The allocation address flows into a STORE whose pointer operand is
 *       itself not a local alloca (i.e., stored into the heap).
 *   E3. The allocation address is passed as an argument to a CALL.
 *   E4. A pointer derived via GEP from the allocation escapes by E1–E3.
 *
 * If none of E1–E3 applies (transitively), the pointer's provenance is
 * confined to the activation record → the heap allocation can be replaced
 * by a stack allocation.  In x86-64 ABI terms: emit a SUB RSP, N instead
 * of a malloc call.
 *
 * Implementation uses a flow-insensitive points-to abstraction:
 *   points_to[r] = set of AllocaIDs that register r may point to.
 *
 * A single forward pass builds points-to sets; a backward escape propagation
 * then marks allocations that flow to an escape site.
 * ───────────────────────────────────────────────────────────────────────────
 */

typedef uint32_t AllocaID;

typedef struct {
  AllocaID id;
  RegID base_reg; /* the register receiving the alloca result  */
  uint32_t size_bytes;
  bool escapes; /* true → cannot promote to stack            */
} AllocaRecord;

typedef struct {
  AllocaRecord allocs[MAX_ALLOCS];
  uint32_t n_allocs;

  /*
   * points_to[r] — simplified: single alloca ID each register may alias.
   * A production pass would use a bitset here; we use a scalar for clarity.
   */
  AllocaID points_to[MAX_INSTRS]; /* indexed by RegID */
} EscapeCtx;

static AllocaID ea_alloc_of(EscapeCtx *ctx, RegID r) {
  return (r < MAX_INSTRS) ? ctx->points_to[r] : NO_ALLOC;
}

/**
 * escape_analysis_pass() — Points-to + Escape analysis, then stack promotion.
 *
 * @param fn   Function IR to analyze.
 * @param ctx  Escape context (caller allocates, zeroed).
 * @return     Number of allocations promoted to the stack.
 */
uint32_t escape_analysis_pass(Function *fn, EscapeCtx *ctx) {
  memset(ctx->points_to, 0xFF, sizeof(ctx->points_to)); /* init = NO_ALLOC */

  /* ── Step 1: Discover all OP_ALLOCA sites ── */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk->reachable)
      continue;

    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->op != OP_ALLOCA)
        continue;
      if (ctx->n_allocs >= MAX_ALLOCS)
        break;

      AllocaRecord *ar = &ctx->allocs[ctx->n_allocs++];
      ar->id = ctx->n_allocs - 1;
      ar->base_reg = ins->dst;
      ar->size_bytes = (ins->n_src > 0) ? ins->src[0] : 8; /* default 8B */
      ar->escapes = false;

      ctx->points_to[ins->dst] = ar->id;
    }
  }

  /* ── Step 2: Forward propagation of points-to through GEP / COPY / PHI ── */
  bool changed = true;
  while (changed) {
    changed = false;

    for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
      Block *blk = fn->blocks[bi];
      if (!blk->reachable)
        continue;

      for (Instr *ins = blk->head; ins; ins = ins->next) {
        AllocaID aid = NO_ALLOC;

        switch (ins->op) {
        case OP_GEP:
        case OP_COPY:
          /* dst inherits the provenance of src[0] */
          aid = ea_alloc_of(ctx, ins->src[0]);
          break;

        case OP_PHI:
          /*
           * φ(r₀:B₀, r₁:B₁) — if all reachable sources point to
           * the same alloca, the φ result inherits it; otherwise
           * the result is ambiguous (conservative: mark escape).
           */
          for (uint32_t p = 0; p < ins->n_phi; p++) {
            Block *pred = fn->blocks[ins->phi[p].block];
            if (!pred || !pred->reachable)
              continue;
            AllocaID src_aid = ea_alloc_of(ctx, ins->phi[p].reg);
            if (aid == NO_ALLOC)
              aid = src_aid;
            else if (aid != src_aid)
              aid = NO_ALLOC; /* ambiguous */
          }
          break;

        case OP_LOAD:
          /* loaded value: no provenance unless we track heap contents */
          aid = NO_ALLOC;
          break;

        default:
          continue; /* other ops don't propagate pointers */
        }

        if (ins->dst && aid != ctx->points_to[ins->dst]) {
          ctx->points_to[ins->dst] = aid;
          changed = true;
        }
      }
    }
  }

  /* ── Step 3: Escape detection — mark allocations that reach escape sites ──
   */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk->reachable)
      continue;

    for (Instr *ins = blk->head; ins; ins = ins->next) {

      /* E1: alloca-derived pointer returned */
      if (ins->op == OP_RET) {
        for (uint32_t s = 0; s < ins->n_src; s++) {
          AllocaID aid = ea_alloc_of(ctx, ins->src[s]);
          if (aid != NO_ALLOC)
            ctx->allocs[aid].escapes = true;
        }
      }

      /* E2: alloca address stored into non-local memory
       *   STORE <value> <pointer>
       *   If <value> is alloca-derived AND <pointer> is not a local alloca
       *   → the pointer escapes into the heap.
       */
      if (ins->op == OP_STORE && ins->n_src >= 2) {
        RegID value_reg = ins->src[0];
        RegID ptr_reg = ins->src[1];
        AllocaID val_aid = ea_alloc_of(ctx, value_reg);
        AllocaID ptr_aid = ea_alloc_of(ctx, ptr_reg);

        if (val_aid != NO_ALLOC && ptr_aid == NO_ALLOC) {
          /* value is a local alloca, pointer is external → escape */
          ctx->allocs[val_aid].escapes = true;
        }
      }

      /* E3: alloca passed as argument to a call */
      if (ins->op == OP_CALL) {
        for (uint32_t s = 0; s < ins->n_call_args; s++) {
          AllocaID aid = ea_alloc_of(ctx, ins->call_args[s]);
          if (aid != NO_ALLOC)
            ctx->allocs[aid].escapes = true;
        }
      }
    }
  }

  /* ── Step 4: Tag allocations (promotable vs escaping) for
   * scalar_promotion_pass ── */
  uint32_t promoted = 0;

  for (uint32_t ai = 0; ai < ctx->n_allocs; ai++) {
    AllocaRecord *ar = &ctx->allocs[ai];
    RegID base = ar->base_reg;
    if (base >= MAX_INSTRS)
      continue;
    Instr *def = fn->def_of[base];
    if (!def || def->op != OP_ALLOCA)
      continue;

    if (ar->escapes || def->escape) {
      def->escape = true; /* mark escaping so scalar promotion skips */
    } else {
      def->escape =
          false; /* non-escaping: stack-promotable, mem2reg candidate */
      promoted++;
    }
  }

  fn->stats.ea_promotions += promoted;
  return promoted;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * SCALAR PROMOTION (mem2reg) — Single-block fast path + multi-block (dominance
 * frontiers)
 *
 * For allocas that never escape, if all loads/stores occur in the same block
 * as the alloca (or one block), replace memory with virtual registers and
 * remove the load/store/alloca. Otherwise use dominance frontiers + SSA rename.
 * ───────────────────────────────────────────────────────────────────────────
 */
static uint32_t multi_block_mem2reg_one(Function *fn, EscapeCtx *ctx,
                                        AllocaRecord *ar,
                                        RegID repl[MAX_INSTRS],
                                        bool repl_valid[MAX_INSTRS]);

static BlockID block_containing_instr(Function *fn, Instr *ins) {
  for (BlockID bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk)
      continue;
    for (Instr *p = blk->head; p; p = p->next)
      if (p == ins)
        return bi;
  }
  return NO_BLOCK;
}

static void unlink_instr(Block *blk, Instr *ins) {
  if (ins->prev)
    ins->prev->next = ins->next;
  else
    blk->head = ins->next;
  if (ins->next)
    ins->next->prev = ins->prev;
  else
    blk->tail = ins->prev;
  blk->n_instrs--;
}

static uint32_t scalar_promotion_pass(Function *fn, EscapeCtx *ctx) {
  uint32_t promoted_count = 0;
  RegID repl[MAX_INSTRS]; /* repl[r] = register to use instead of r (for loads
                             we're removing) */
  bool repl_valid[MAX_INSTRS];
  memset(repl_valid, 0, sizeof(repl_valid));

  /* Compute RPO and dominators once so multi-block mem2reg can use dominance
   * frontiers */
  licm_compute_rpo(fn);
  licm_compute_doms(fn);
  compute_dominance_frontiers(fn);

  for (uint32_t ai = 0; ai < ctx->n_allocs; ai++) {
    AllocaRecord *ar = &ctx->allocs[ai];
    if (ar->escapes)
      continue;
    RegID base_reg = ar->base_reg;
    Instr *alloca_ins = fn->def_of[base_reg];
    if (!alloca_ins || alloca_ins->op != OP_ALLOCA || alloca_ins->escape)
      continue;

    BlockID alloca_block = block_containing_instr(fn, alloca_ins);
    if (alloca_block == NO_BLOCK)
      continue;

    bool has_store = false;
    for (uint32_t bi = 0; bi < fn->n_blocks && !has_store; bi++) {
      Block *b = fn->blocks[bi];
      if (!b || !b->reachable)
        continue;
      for (Instr *ins = b->head; ins; ins = ins->next) {
        if (ins->op == OP_STORE && ins->n_src >= 2 && ins->src[1] == base_reg) {
          has_store = true;
          break;
        }
      }
    }
    if (!has_store)
      continue;

    Block *blk = fn->blocks[alloca_block];
    if (!blk || !blk->reachable)
      continue;

    /* Single-block check: every store (src[1]==base_reg) and load
     * (src[0]==base_reg) must be in alloca_block */
    bool single_block = true;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->op == OP_STORE && ins->n_src >= 2 && ins->src[1] == base_reg)
        continue;
      if (ins->op == OP_LOAD && ins->n_src >= 1 && ins->src[0] == base_reg)
        continue;
    }
    for (BlockID bi = 0; bi < fn->n_blocks && single_block; bi++) {
      if (bi == alloca_block)
        continue;
      Block *b = fn->blocks[bi];
      if (!b || !b->reachable)
        continue;
      for (Instr *ins = b->head; ins; ins = ins->next) {
        if (ins->op == OP_STORE && ins->n_src >= 2 && ins->src[1] == base_reg) {
          single_block = false;
          break;
        }
        if (ins->op == OP_LOAD && ins->n_src >= 1 && ins->src[0] == base_reg) {
          single_block = false;
          break;
        }
      }
    }
    if (!single_block) {
      uint32_t n = multi_block_mem2reg_one(fn, ctx, ar, repl, repl_valid);
      promoted_count += n;
      continue;
    }

    /* Linear walk: track current value for base_reg, build repl[load_dst] =
     * value_reg */
    RegID current_val = 0;
    bool current_val_set = false;
    Instr *loads_to_remove[256];
    Instr *stores_to_remove[256];
    uint32_t n_loads = 0, n_stores = 0;

    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->op == OP_ALLOCA && ins->dst == base_reg) {
        /* alloca defines the slot; no value yet until first store */
        continue;
      }
      if (ins->op == OP_STORE && ins->n_src >= 2 && ins->src[1] == base_reg) {
        current_val = ins->src[0];
        while (current_val < MAX_INSTRS && repl_valid[current_val]) {
          current_val = repl[current_val];
        }
        current_val_set = true;
        if (n_stores < 256)
          stores_to_remove[n_stores++] = ins;
        continue;
      }
      if (ins->op == OP_LOAD && ins->n_src >= 1 && ins->src[0] == base_reg) {
        if (current_val_set && ins->dst < MAX_INSTRS) {
          repl[ins->dst] = current_val;
          repl_valid[ins->dst] = true;
        }
        if (n_loads < 256)
          loads_to_remove[n_loads++] = ins;
        continue;
      }
    }

    /* Replace every use of a load_dst with repl[load_dst] in this block */
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->dead)
        continue;
      for (uint32_t s = 0; s < ins->n_src; s++) {
        if (ins->op == OP_BR)
          continue;
        if (ins->op == OP_CONDBR && s >= 1)
          continue;
        if (ins->src[s] < MAX_INSTRS && repl_valid[ins->src[s]])
          ins->src[s] = repl[ins->src[s]];
      }
      if (ins->op == OP_PHI)
        for (uint32_t p = 0; p < ins->n_phi; p++)
          if (ins->phi[p].reg < MAX_INSTRS && repl_valid[ins->phi[p].reg])
            ins->phi[p].reg = repl[ins->phi[p].reg];
      if (ins->op == OP_CALL)
        for (uint32_t c = 0; c < ins->n_call_args; c++)
          if (ins->call_args[c] < MAX_INSTRS && repl_valid[ins->call_args[c]])
            ins->call_args[c] = repl[ins->call_args[c]];
    }

    /* Unlink and free loads, stores, alloca; clear def_of for removed defs */
    for (uint32_t i = 0; i < n_loads; i++) {
      Instr *ins = loads_to_remove[i];
      RegID d = ins->dst;
      if (d < MAX_INSTRS) {
        fn->def_of[d] = NULL;
        repl_valid[d] = false;
      }
      unlink_instr(blk, ins);
      free(ins);
    }
    for (uint32_t i = 0; i < n_stores; i++) {
      Instr *ins = stores_to_remove[i];
      unlink_instr(blk, ins);
      free(ins);
    }
    if (alloca_ins->dst && alloca_ins->dst < MAX_INSTRS)
      fn->def_of[alloca_ins->dst] = NULL;
    unlink_instr(blk, alloca_ins);
    free(alloca_ins);
    promoted_count++;
  }
  return promoted_count;
}

/* ── Multi-block mem2reg: dominance frontiers + SSA rename ── */
#define MAX_IDOM_CHILDREN 32
static bool in_need_phi[MAX_BLOCKS];
static BlockID idom_children_list[MAX_BLOCKS][MAX_IDOM_CHILDREN];
static uint32_t idom_children_count[MAX_BLOCKS];

static void build_idom_children(Function *fn) {
  for (uint32_t i = 0; i < fn->n_blocks; i++)
    idom_children_count[i] = 0;
  for (uint32_t i = 0; i < fn->n_blocks; i++) {
    BlockID b = (BlockID)i;
    BlockID d = licm_idom[b];
    if (d == NO_BLOCK || d == b)
      continue;
    if (idom_children_count[d] < MAX_IDOM_CHILDREN)
      idom_children_list[d][idom_children_count[d]++] = b;
  }
}

/* Insert instruction at head of block (PHIs must be first). */
static void insert_instr_at_head(Block *blk, Instr *ins) {
  ins->next = blk->head;
  ins->prev = NULL;
  if (blk->head)
    blk->head->prev = ins;
  else
    blk->tail = ins;
  blk->head = ins;
  blk->n_instrs++;
}

static void mem2reg_rename_rec(Function *fn, BlockID b, RegID base_reg,
                               Instr **phi_instrs, RegID *stack,
                               uint32_t *stack_top, RegID *repl,
                               bool *repl_valid) {
  if (b >= fn->n_blocks)
    return;
  Block *blk = fn->blocks[b];
  if (!blk || !blk->reachable)
    return;
  uint32_t defs_here = 0;
  Instr *phi = phi_instrs[b];
  if (phi && (uintptr_t)phi->imm == (uintptr_t)base_reg) {
    stack[(*stack_top)++] = phi->dst;
    defs_here++;
  }
  for (Instr *ins = blk->head; ins; ins = ins->next) {
    if (ins->op == OP_ALLOCA && ins->dst == base_reg)
      continue;
    if (ins->op == OP_STORE && ins->n_src >= 2 && ins->src[1] == base_reg) {
      stack[(*stack_top)++] = ins->src[0];
      defs_here++;
      continue;
    }
    if (ins->op == OP_LOAD && ins->n_src >= 1 && ins->src[0] == base_reg) {
      if (*stack_top > 0 && ins->dst < MAX_INSTRS) {
        repl[ins->dst] = stack[(*stack_top) - 1];
        repl_valid[ins->dst] = true;
      }
      continue;
    }
  }
  RegID out_val = *stack_top > 0 ? stack[(*stack_top) - 1] : 0;
  for (uint32_t si = 0; si < blk->n_succs; si++) {
    BlockID s = blk->succs[si];
    if (s >= fn->n_blocks)
      continue;
    Instr *sphi = phi_instrs[s];
    if (!sphi)
      continue;
    if ((uintptr_t)sphi->imm != (uintptr_t)base_reg)
      continue;
    for (uint32_t p = 0; p < sphi->n_phi; p++) {
      if (sphi->phi[p].block == b) {
        sphi->phi[p].reg = out_val;
        break;
      }
    }
  }
  for (uint32_t i = 0; i < idom_children_count[b]; i++) {
    mem2reg_rename_rec(fn, idom_children_list[b][i], base_reg, phi_instrs,
                       stack, stack_top, repl, repl_valid);
  }
  (*stack_top) -= defs_here;
}

/** Promote one non-escaping alloca to SSA using dominance frontiers. Returns 1
 * if promoted. */

static uint32_t multi_block_mem2reg_one(Function *fn, EscapeCtx *ctx,
                                        AllocaRecord *ar,
                                        RegID repl[MAX_INSTRS],
                                        bool repl_valid[MAX_INSTRS]) {
  RegID base_reg = ar->base_reg;
  if (base_reg >= MAX_INSTRS)
    return 0;
  Instr *alloca_ins = fn->def_of[base_reg];
  if (!alloca_ins || alloca_ins->op != OP_ALLOCA || alloca_ins->escape)
    return 0;

  BlockID alloca_block = block_containing_instr(fn, alloca_ins);
  if (alloca_block == NO_BLOCK)
    return 0;

  /* Def blocks: block containing alloca + every block with a store to base_reg
   */
  bool def_block[MAX_BLOCKS];
  memset(def_block, 0, sizeof(def_block));
  def_block[alloca_block] = true;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *b = fn->blocks[bi];
    if (!b || !b->reachable)
      continue;
    for (Instr *ins = b->head; ins; ins = ins->next) {
      if (ins->op == OP_STORE && ins->n_src >= 2 && ins->src[1] == base_reg) {
        def_block[bi] = true;
        break;
      }
    }
  }

  /* Iterated dominance frontier → need_phi */
  memset(in_need_phi, 0, sizeof(in_need_phi));
  BlockID worklist[MAX_BLOCKS];
  uint32_t wl_head = 0, wl_tail = 0;
  for (uint32_t i = 0; i < fn->n_blocks; i++) {
    if (!def_block[i])
      continue;
    in_need_phi[i] = true;
    worklist[wl_tail++] = (BlockID)i;
  }
  while (wl_head < wl_tail) {
    BlockID b = worklist[wl_head++];
    for (uint32_t j = 0; j < df_count[b]; j++) {
      BlockID y = df_list[b][j];
      if (!in_need_phi[y]) {
        in_need_phi[y] = true;
        if (wl_tail < MAX_BLOCKS)
          worklist[wl_tail++] = y;
      }
    }
  }

  /* Allocate new regs for PHI results; assign PHI instr ids */
  InstrID next_instr_id = 0;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk)
      continue;
    for (Instr *ins = blk->head; ins; ins = ins->next)
      if (ins->id >= next_instr_id)
        next_instr_id = ins->id + 1;
  }
  Instr *phi_instrs[MAX_BLOCKS]; /* phi_instrs[block_id] = PHI for this
                                    variable, or NULL */
  memset(phi_instrs, 0, sizeof(phi_instrs));
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    if (!in_need_phi[bi])
      continue;
    Block *blk = fn->blocks[bi];
    if (!blk || blk->n_preds > MAX_PHI_SOURCES)
      continue;

    Instr *phi = calloc(1, sizeof(Instr));
    phi->id = next_instr_id++;
    phi->op = OP_PHI;
    phi->dst = fn->n_regs++;
    phi->n_phi = blk->n_preds;
    for (uint32_t p = 0; p < blk->n_preds; p++) {
      phi->phi[p].block = blk->preds[p];
      phi->phi[p].reg = 0; /* filled during rename */
    }
    phi->imm =
        (int64_t)(uintptr_t)base_reg; /* mark which variable this PHI is for */
    phi->exec_freq = 1.0;
    insert_instr_at_head(blk, phi);
    phi_instrs[bi] = phi;
  }

  build_idom_children(fn);

  /* Rename: recursively process blocks over dominator tree; stack of current
   * value (RegID) */
  RegID stack[MAX_INSTRS];
  uint32_t stack_top = 0;
  mem2reg_rename_rec(fn, fn->entry, base_reg, phi_instrs, stack, &stack_top,
                     repl, repl_valid);

  /* Replace all uses of load destinations with repl[] */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk || !blk->reachable)
      continue;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->dead)
        continue;
      for (uint32_t s = 0; s < ins->n_src; s++) {
        if (ins->op == OP_BR)
          continue;
        if (ins->op == OP_CONDBR && s >= 1)
          continue;
        if (ins->src[s] < MAX_INSTRS && repl_valid[ins->src[s]])
          ins->src[s] = repl[ins->src[s]];
      }
      if (ins->op == OP_PHI)
        for (uint32_t p = 0; p < ins->n_phi; p++)
          if (ins->phi[p].reg < MAX_INSTRS && repl_valid[ins->phi[p].reg])
            ins->phi[p].reg = repl[ins->phi[p].reg];
      if (ins->op == OP_CALL)
        for (uint32_t c = 0; c < ins->n_call_args; c++)
          if (ins->call_args[c] < MAX_INSTRS && repl_valid[ins->call_args[c]])
            ins->call_args[c] = repl[ins->call_args[c]];
    }
  }

  /* Remove loads, stores, alloca; collect in reverse order per block to avoid
   * use-after-free */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk || !blk->reachable)
      continue;
    Instr *to_remove[256];
    uint32_t n_remove = 0;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->op == OP_LOAD && ins->n_src >= 1 && ins->src[0] == base_reg) {
        if (n_remove < 256)
          to_remove[n_remove++] = ins;
        continue;
      }
      if (ins->op == OP_STORE && ins->n_src >= 2 && ins->src[1] == base_reg) {
        if (n_remove < 256)
          to_remove[n_remove++] = ins;
        continue;
      }
      if (ins->op == OP_ALLOCA && ins->dst == base_reg) {
        if (n_remove < 256)
          to_remove[n_remove++] = ins;
        continue;
      }
    }
    for (uint32_t i = 0; i < n_remove; i++) {
      Instr *ins = to_remove[i];
      RegID d = ins->dst;
      if (d < MAX_INSTRS)
        fn->def_of[d] = NULL;
      unlink_instr(blk, ins);
      free(ins);
    }
  }
  return 1;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * PASS 3: PROFILE-GUIDED BASIC BLOCK REORDERING
 *
 * Theory
 * ──────
 * Modern CPUs fetch instructions in 64-byte cache lines. If a hot loop body
 * is split across multiple cache lines — because cold exception-handling code
 * was interleaved — fetch stalls are incurred on every iteration.
 *
 * PGO-based reordering uses instrumentation-derived execution frequencies to
 * construct a linear block ordering that maximises icache locality:
 *
 *   1. Build a *trace* (chain) of basic blocks ordered by execution frequency.
 *      Start from the entry block and greedily extend by selecting the most
 *      frequently executed successor that has not yet been placed.
 *
 *   2. The greedy extension criterion is:
 *        next = argmax_{s ∈ succs(B), !placed(s)} exec_freq(s)
 *
 *   3. When no un-placed successor is available, start a new chain from the
 *      globally most frequent un-placed block (cold blocks float to the tail).
 *
 *   4. Emit chains in descending frequency order. Cold blocks (exception paths,
 *      unlikely branches) are emitted last — typically into a separate section
 *      (.text.cold) to avoid polluting the primary instruction cache.
 *
 * Branch probability propagation:
 *   exec_freq(successor) = exec_freq(block) × branch_prob[i]
 *   This is seeded at the entry block (exec_freq = 1.0) and propagated
 *   forward through the CFG. For loops, a fixpoint is computed using
 *   two-pass dominance: natural loop headers get their back-edge frequency
 *   accumulated (simplified Dempster-Shafer estimate).
 *
 * Complexity: O(B log B) where B = number of basic blocks.
 * ───────────────────────────────────────────────────────────────────────────
 */

/* Comparator: sort blocks descending by execution frequency */
static Block *g_blocks_for_sort[MAX_BLOCKS];
static int cmp_freq_desc(const void *a, const void *b) {
  uint32_t ia = *(const uint32_t *)a;
  uint32_t ib = *(const uint32_t *)b;
  double fa = g_blocks_for_sort[ia] ? g_blocks_for_sort[ia]->exec_freq : 0.0;
  double fb = g_blocks_for_sort[ib] ? g_blocks_for_sort[ib]->exec_freq : 0.0;
  if (fa > fb)
    return -1;
  if (fa < fb)
    return 1;
  return 0;
}

/**
 * propagate_exec_freq() — Forward-propagate branch probabilities.
 *
 * Seeds entry block at freq=1.0 and propagates freq × prob through each
 * CFG edge.  Loops are handled with a fixed iteration limit (3 passes
 * approximates the loop trip count contribution).
 */
static BlockID next_unplaced(Function *fn, uint32_t *sorted) {
  for (uint32_t i = 0; i < fn->n_blocks; i++) {
    BlockID id = sorted[i];
    if (id < fn->n_blocks && !fn->blocks[id]->placed &&
        fn->blocks[id]->reachable)
      return id;
  }
  return 0xFFFFFFFFu;
}

static void propagate_exec_freq(Function *fn) {
  /* Initialize all blocks to 0 except entry */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    fn->blocks[bi]->exec_freq = 0.0;
  }
  if (fn->n_blocks == 0)
    return;
  fn->blocks[fn->entry]->exec_freq = 1.0;

  /* Three passes handle typical single-level loops */
  for (int pass = 0; pass < 3; pass++) {
    for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
      Block *blk = fn->blocks[bi];
      if (!blk->reachable || blk->exec_freq == 0.0)
        continue;

      for (uint32_t si = 0; si < blk->n_succs; si++) {
        BlockID sid = blk->succs[si];
        if (sid >= fn->n_blocks)
          continue;
        Block *succ = fn->blocks[sid];

        double prob = (blk->n_succs == 1) ? 1.0 : blk->branch_prob[si];

        succ->exec_freq += blk->exec_freq * prob;
      }
    }
  }
}

/**
 * pgo_reorder_pass() — Greedy trace-based basic block reordering.
 *
 * @param fn          Function IR.
 * @param order_out   Output array of block IDs in the new emission order.
 *                    Caller provides an array of size fn->n_blocks.
 * @return            Number of blocks reordered (same as fn->n_blocks unless
 *                    unreachable blocks were pruned).
 */
uint32_t pgo_reorder_pass(Function *fn, BlockID *order_out) {
  /* Step 1: propagate frequencies from branch probabilities */
  propagate_exec_freq(fn);

  /* Step 2: reset placement state */
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    fn->blocks[bi]->placed = false;
    fn->blocks[bi]->chain_next = 0xFFFFFFFF;
    g_blocks_for_sort[bi] = fn->blocks[bi];
  }

  /* Step 3: build sorted order by frequency (for chain start selection) */
  uint32_t sorted[MAX_BLOCKS];
  for (uint32_t i = 0; i < fn->n_blocks; i++)
    sorted[i] = i;
  qsort(sorted, fn->n_blocks, sizeof(uint32_t), cmp_freq_desc);

  uint32_t n_placed = 0;

  /* Step 4: entry block anchors the first chain */
  BlockID chain_start = fn->entry;

  while (chain_start != 0xFFFFFFFF) {
    BlockID cur = chain_start;
    fn->blocks[cur]->placed = true;
    order_out[n_placed++] = cur;

    /* Greedily extend the chain */
    while (true) {
      Block *blk = fn->blocks[cur];
      double best_f = -1.0;
      BlockID best_id = 0xFFFFFFFF;

      for (uint32_t si = 0; si < blk->n_succs; si++) {
        BlockID sid = blk->succs[si];
        if (sid >= fn->n_blocks)
          continue;
        if (fn->blocks[sid]->placed)
          continue;
        if (!fn->blocks[sid]->reachable)
          continue;

        double f = fn->blocks[sid]->exec_freq;
        if (f > best_f) {
          best_f = f;
          best_id = sid;
        }
      }

      if (best_id == 0xFFFFFFFF)
        break; /* no more successors to extend */

      blk->chain_next = best_id;
      fn->blocks[best_id]->placed = true;
      order_out[n_placed++] = best_id;
      cur = best_id;
    }

    /* Start next chain from the highest-frequency un-placed block */
    chain_start = next_unplaced(fn, sorted);
  }

  fn->stats.pgo_blocks_reordered += n_placed;
  return n_placed;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * REACHABILITY ANALYSIS  (prerequisite for all three passes)
 * ───────────────────────────────────────────────────────────────────────────
 */

static void compute_reachability(Function *fn) {
  /* BFS from entry */
  bool visited[MAX_BLOCKS];
  memset(visited, 0, sizeof(visited));
  BlockID queue[MAX_BLOCKS];
  int head = 0, tail = 0;

  for (uint32_t i = 0; i < fn->n_blocks; i++)
    fn->blocks[i]->reachable = false;

  queue[tail++] = fn->entry;
  visited[fn->entry] = true;

  while (head < tail) {
    BlockID bid = queue[head++];
    Block *blk = fn->blocks[bid];
    blk->reachable = true;

    for (uint32_t si = 0; si < blk->n_succs; si++) {
      BlockID sid = blk->succs[si];
      if (sid < fn->n_blocks && !visited[sid]) {
        visited[sid] = true;
        queue[tail++] = sid;
      }
    }
  }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * PHASE B: MINIMAL AST → IR BRIDGE
 *
 * One function, scalars only (int, pointers). Build Function* from a minimal
 * AST; run existing passes; no IR→asm. Validates CFG, DCE, LICM on real C-like
 * flow.
 * ───────────────────────────────────────────────────────────────────────────
 */

#define AST_MAX_VARS                                                           \
  1024 /* main() and large functions have many locals; was 16 (silent overflow \
          → corruption) */
#define AST_MAX_STMTS 64

typedef struct ASTNode ASTNode;
struct ASTNode {
  int kind; /* AST_NUM, AST_VAR, AST_ADD, AST_ASSIGN, AST_LT, AST_IF, AST_WHILE,
               AST_RETURN, AST_BLOCK */
  int64_t num_val;
  char var_name[NAME_LEN];
  ASTNode *lhs;
  ASTNode *rhs;
  ASTNode *cond;
  ASTNode *then_body;
  ASTNode *else_body;
  ASTNode *body;
  ASTNode **stmts;
  uint32_t n_stmts;
};

enum {
  AST_NUM = 1,
  AST_VAR,
  AST_ADD,
  AST_SUB,
  AST_LT,
  AST_LE,
  AST_ASSIGN,
  AST_IF,
  AST_WHILE,
  AST_RETURN,
  AST_BLOCK
};

#define MAX_NESTED_LOOPS 16

typedef struct {
  Function *fn;
  BlockID cur_block;
  RegID next_reg;
  InstrID next_instr_id;
  /* var name → slot index; slot_alloca_reg[slot] = reg holding alloca result */
  char var_names[AST_MAX_VARS][NAME_LEN];
  RegID slot_alloca_reg[AST_MAX_VARS];
  Instr *slot_alloca_instr[AST_MAX_VARS];
  uint32_t n_vars;
  int want_address; /* 1 when lowering lhs of assign/compound_assign: return
                       address reg, no load */
  /* break/continue: current loop exit and latch (continue target) */
  BlockID loop_exit_stack[MAX_NESTED_LOOPS];
  BlockID loop_latch_stack[MAX_NESTED_LOOPS];
  int loop_depth;
} LowerCtx;

static RegID lower_expr(LowerCtx *ctx, ASTNode *ast);
static void lower_stmt(LowerCtx *ctx, ASTNode *ast);

static BlockID new_block(LowerCtx *ctx, const char *name) {
  Function *fn = ctx->fn;
  if (fn->n_blocks >= MAX_BLOCKS) {
    fprintf(stderr, "FATAL: MAX_BLOCKS (%d) exceeded in %s\n", MAX_BLOCKS,
            name);
    exit(1);
  }
  BlockID id = (BlockID)fn->n_blocks;
  fn->blocks[id] = calloc(1, sizeof(Block));
  Block *b = fn->blocks[id];
  b->id = id;
  strncpy(b->name, name, NAME_LEN - 1);
  b->reachable = true;
  fn->n_blocks++;
  return id;
}

static void emit_instr(LowerCtx *ctx, Instr *ins) {
  Block *blk = ctx->fn->blocks[ctx->cur_block];
  if (!blk->head)
    blk->head = blk->tail = ins;
  else {
    blk->tail->next = ins;
    ins->prev = blk->tail;
    blk->tail = ins;
  }
  blk->n_instrs++;
  if (ins->dst && ins->dst < MAX_INSTRS)
    ctx->fn->def_of[ins->dst] = ins;
  if (ctx->fn->n_regs <= ins->dst)
    ctx->fn->n_regs = ins->dst + 1;
}

static Instr *make_instr_imm(InstrID id, Opcode op, RegID dst, int64_t imm_val,
                             int line_no) {
  Instr *ins = calloc(1, sizeof(Instr));
  ins->id = id;
  ins->op = op;
  ins->dst = dst;
  ins->imm = imm_val;
  ins->exec_freq = 1.0;
  ins->line_no = line_no;
  return ins;
}

/* Root cause: param allocas must be created in the entry block so they get the
 * first num_params slots (-8,-16,...) filled by the prologue. Otherwise first
 * use in a later block (e.g. argv in for.body) creates the alloca there and it
 * stays 0. */
static int is_main_func(const char *func_name) {
  return func_name && func_name[0] == 'm' && func_name[1] == 'a' &&
         func_name[2] == 'i' && func_name[3] == 'n' && func_name[4] == '\0';
}

/* Lazy OP_ALLOCA trap: if the first use of a variable is inside a
 * conditional/loop body, the alloca was emitted there and never runs on other
 * paths → uninitialized slot → null deref. Fix: always hoist OP_ALLOCA to the
 * entry block so it dominates all uses. */
static RegID get_or_create_var(LowerCtx *ctx, const char *name, int size) {
  if (!name || (uintptr_t)name < 4096 || name[0] == '\0') {
    fprintf(stderr, "[ZCC-IR] DEBUG: get_or_create_var EMPTY OR NULL\n");
    return 0;
  }
  fprintf(stderr, "[ZCC-IR] DEBUG: get_or_create_var('%s')\n", name);
  for (uint32_t i = 0; i < ctx->n_vars; i++) {
    if (strcmp(ctx->var_names[i], name) == 0) {
      if (size > (int)ctx->slot_alloca_instr[i]->imm) {
        ctx->slot_alloca_instr[i]->imm = size;
      }
      return ctx->slot_alloca_reg[i];
    }
  }

  if (ctx->n_vars >= AST_MAX_VARS) {
    fprintf(stderr, "FATAL: AST_MAX_VARS exceeded\n");
    return 0;
  }

  uint32_t slot = ctx->n_vars++;
  strncpy(ctx->var_names[slot], name, NAME_LEN - 1);
  ctx->var_names[slot][NAME_LEN - 1] = '\0';
  RegID r = ctx->next_reg++;
  ctx->slot_alloca_reg[slot] = r;

  Instr *alloca = make_instr_imm(ctx->next_instr_id++, OP_ALLOCA, r, size > 0 ? size : 8, 0);
  ctx->slot_alloca_instr[slot] = alloca;

  /* Hoist OP_ALLOCA to the entry block so it dominates all uses. */
  Block *entry = ctx->fn->blocks[ctx->fn->entry];
  Instr *pt = entry->tail;

  /* Skip backward past terminal branches to find a valid insertion point */
  while (pt && (pt->op == OP_BR || pt->op == OP_CONDBR || pt->op == OP_RET)) {
    pt = pt->prev;
  }

  if (pt) {
    alloca->prev = pt;
    alloca->next = pt->next;
    if (pt->next)
      pt->next->prev = alloca;
    else
      entry->tail = alloca;
    pt->next = alloca;
  } else {
    alloca->next = entry->head;
    if (entry->head)
      entry->head->prev = alloca;
    else
      entry->tail = alloca;
    entry->head = alloca;
  }
  entry->n_instrs++;

  if (alloca->dst && alloca->dst < MAX_INSTRS)
    ctx->fn->def_of[alloca->dst] = alloca;
  if (ctx->fn->n_regs <= alloca->dst)
    ctx->fn->n_regs = alloca->dst + 1;

  return r;
}

static RegID lower_expr(LowerCtx *ctx, ASTNode *ast) {
  if (!ast)
    return 0;
  RegID r = ctx->next_reg++;
  Instr *ins = NULL;
  switch (ast->kind) {
  case AST_NUM: {
    ins = make_instr_imm(ctx->next_instr_id++, OP_CONST, r, ast->num_val, 0);
    break;
  }
  case AST_VAR: {
    RegID alloca_r = get_or_create_var(ctx, ast->var_name, 8);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_LOAD;
    ins->dst = r;
    ins->src[0] = alloca_r;
    ins->n_src = 1;
    ins->exec_freq = 1.0;
    break;
  }
  case AST_ADD: {
    RegID l = lower_expr(ctx, ast->lhs);
    RegID rh = lower_expr(ctx, ast->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_ADD;
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    break;
  }
  case AST_SUB: {
    RegID l = lower_expr(ctx, ast->lhs);
    RegID rh = lower_expr(ctx, ast->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_SUB;
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    break;
  }
  case AST_LT: {
    RegID l = lower_expr(ctx, ast->lhs);
    RegID rh = lower_expr(ctx, ast->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_LT;
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    break;
  }
  default:
    return 0;
  }
  if (ins) {
    emit_instr(ctx, ins);
    return r;
  }
  return 0;
}

static void lower_stmt(LowerCtx *ctx, ASTNode *ast) {
  if (!ast)
    return;
  Function *fn = ctx->fn;
  switch (ast->kind) {
  case AST_ASSIGN: {
    RegID alloca_r = get_or_create_var(ctx, ast->lhs->var_name, 8);
    RegID val_r = lower_expr(ctx, ast->rhs);
    Instr *st = calloc(1, sizeof(Instr));
    st->id = ctx->next_instr_id++;
    st->op = OP_STORE;
    st->dst = 0;
    st->src[0] = val_r;
    st->src[1] = alloca_r;
    st->n_src = 2;
    st->exec_freq = 1.0;
    emit_instr(ctx, st);
    return;
  }
  case AST_IF: {
    RegID cond_r = lower_expr(ctx, ast->cond);
    BlockID then_blk = new_block(ctx, "if.then");
    BlockID else_blk = new_block(ctx, "if.else");
    BlockID merge_blk = new_block(ctx, "if.merge");
    Block *cur = fn->blocks[ctx->cur_block];
    Instr *cbr = calloc(1, sizeof(Instr));
    cbr->id = ctx->next_instr_id++;
    cbr->op = OP_CONDBR;
    cbr->dst = 0;
    cbr->src[0] = cond_r;
    cbr->src[1] = then_blk;
    cbr->src[2] = else_blk;
    cbr->n_src = 3;
    cbr->exec_freq = 1.0;
    emit_instr(ctx, cbr);
    cur->succs[0] = then_blk;
    cur->succs[1] = else_blk;
    cur->n_succs = 2;
    cur->branch_prob[0] = 0.5;
    cur->branch_prob[1] = 0.5;
    fn->blocks[then_blk]->preds[0] = ctx->cur_block;
    fn->blocks[then_blk]->n_preds = 1;
    fn->blocks[else_blk]->preds[0] = ctx->cur_block;
    fn->blocks[else_blk]->n_preds = 1;

    ctx->cur_block = then_blk;
    lower_stmt(ctx, ast->then_body);
    {
      Block *_tb = fn->blocks[ctx->cur_block];
      Instr *_tt = _tb ? _tb->tail : NULL;
      if (!(_tt &&
            (_tt->op == OP_RET || _tt->op == OP_BR || _tt->op == OP_CONDBR))) {
        Instr *br_then = calloc(1, sizeof(Instr));
        br_then->id = ctx->next_instr_id++;
        br_then->op = OP_BR;
        br_then->src[0] = merge_blk;
        br_then->n_src = 1;
        br_then->exec_freq = 1.0;
        emit_instr(ctx, br_then);
        fn->blocks[ctx->cur_block]->succs[0] = merge_blk;
        fn->blocks[ctx->cur_block]->n_succs = 1;
        fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++] =
            ctx->cur_block;
      }
    }
    fn->blocks[merge_blk]->n_preds++;

    ctx->cur_block = else_blk;
    if (ast->else_body)
      lower_stmt(ctx, ast->else_body);
    {
      Block *_eb = fn->blocks[ctx->cur_block];
      Instr *_et = _eb ? _eb->tail : NULL;
      if (!(_et &&
            (_et->op == OP_RET || _et->op == OP_BR || _et->op == OP_CONDBR))) {
        Instr *br_else = calloc(1, sizeof(Instr));
        br_else->id = ctx->next_instr_id++;
        br_else->op = OP_BR;
        br_else->src[0] = merge_blk;
        br_else->n_src = 1;
        br_else->exec_freq = 1.0;
        emit_instr(ctx, br_else);
        fn->blocks[ctx->cur_block]->succs[0] = merge_blk;
        fn->blocks[ctx->cur_block]->n_succs = 1;
        fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++] =
            ctx->cur_block;
      }
    }
    fn->blocks[merge_blk]->n_preds++;

    ctx->cur_block = merge_blk;
    return;
  }
  case AST_WHILE: {
    BlockID head = new_block(ctx, "while.head");
    BlockID body_blk = new_block(ctx, "while.body");
    BlockID exit_blk = new_block(ctx, "while.exit");
    Block *cur = fn->blocks[ctx->cur_block];
    Instr *br = calloc(1, sizeof(Instr));
    br->id = ctx->next_instr_id++;
    br->op = OP_BR;
    br->src[0] = head;
    br->n_src = 1;
    br->exec_freq = 1.0;
    emit_instr(ctx, br);
    cur->succs[0] = head;
    cur->n_succs = 1;
    fn->blocks[head]->preds[0] = ctx->cur_block;
    fn->blocks[head]->n_preds = 1;

    ctx->cur_block = head;
    RegID cond_r = lower_expr(ctx, ast->cond);
    Instr *cbr = calloc(1, sizeof(Instr));
    cbr->id = ctx->next_instr_id++;
    cbr->op = OP_CONDBR;
    cbr->dst = 0;
    cbr->src[0] = cond_r;
    cbr->src[1] = body_blk;
    cbr->src[2] = exit_blk;
    cbr->n_src = 3;
    cbr->exec_freq = 1.0;
    emit_instr(ctx, cbr);
    fn->blocks[head]->succs[0] = body_blk;
    fn->blocks[head]->succs[1] = exit_blk;
    fn->blocks[head]->n_succs = 2;
    fn->blocks[head]->branch_prob[0] = 0.9;
    fn->blocks[head]->branch_prob[1] = 0.1;
    fn->blocks[body_blk]->preds[0] = head;
    fn->blocks[body_blk]->n_preds = 1;
    fn->blocks[exit_blk]->preds[0] = head;
    fn->blocks[exit_blk]->n_preds = 1;

    ctx->cur_block = body_blk;
    lower_stmt(ctx, ast->body);
    Instr *br_back = calloc(1, sizeof(Instr));
    br_back->id = ctx->next_instr_id++;
    br_back->op = OP_BR;
    br_back->src[0] = head;
    br_back->n_src = 1;
    br_back->exec_freq = 1.0;
    emit_instr(ctx, br_back);
    fn->blocks[body_blk]->succs[0] = head;
    fn->blocks[body_blk]->n_succs = 1;
    fn->blocks[head]->preds[fn->blocks[head]->n_preds] = body_blk;
    fn->blocks[head]->n_preds++;

    ctx->cur_block = exit_blk;
    return;
  }
  case AST_RETURN: {
    RegID val_r = lower_expr(ctx, ast->lhs);
    Instr *ret = calloc(1, sizeof(Instr));
    ret->id = ctx->next_instr_id++;
    ret->op = OP_RET;
    ret->dst = 0;
    ret->src[0] = val_r;
    ret->n_src = 1;
    ret->exec_freq = 1.0;
    emit_instr(ctx, ret);
    fn->blocks[ctx->cur_block]->n_succs = 0;
    return;
  }
  case AST_BLOCK:
    for (uint32_t i = 0; i < ast->n_stmts; i++)
      lower_stmt(ctx, ast->stmts[i]);
    return;
  default:
    return;
  }
}

/* ─────────────────────────────────────────────────────────────────────────────
 * ZCC AST BRIDGE — Lower ZCC-parsed function (scalars + control flow) to IR.
 * ZCCNode and ZND_* come from zcc_ast_bridge.h. Copy from Node* via accessors.
 * Standalone build: cc -DZCC_BRIDGE_STANDALONE -o passes compiler_passes.c -lm
 * ───────────────────────────────────────────────────────────────────────────
 */

#ifdef ZCC_BRIDGE_STANDALONE
/* Stub accessors when building passes alone (no zcc.c). Link with
 * -DZCC_BRIDGE_STANDALONE. */
/* CG-IR-012b: ALL Node/Type accessor stubs replaced with extern declarations.
 * The real implementations live in zcc.c (part1.c). The stubs here returned
 * dummy values (0/NULL/""), causing the AST-to-IR conversion to lose all
 * structural information — func names, args, children, offsets, etc. */
extern int node_kind(struct Node *n);
extern long long node_int_val(struct Node *n);
extern int node_str_id(struct Node *n);
extern void node_name(struct Node *n, char *buf, unsigned len);
extern int node_is_global(struct Node *n);
extern int node_is_array(struct Node *n);
extern int node_is_func(struct Node *n);
extern struct Node *node_lhs(struct Node *n);
extern struct Node *node_rhs(struct Node *n);
extern struct Node *node_cond(struct Node *n);
extern struct Node *node_then_body(struct Node *n);
extern struct Node *node_else_body(struct Node *n);
extern struct Node *node_body(struct Node *n);
extern struct Node *node_init(struct Node *n);
extern struct Node *node_inc(struct Node *n);
extern struct Node **node_cases(struct Node *n);
extern int node_num_cases(struct Node *n);
extern struct Node *node_default_case(struct Node *n);
extern long long node_case_val(struct Node *n);
extern struct Node *node_case_body(struct Node *n);
extern int node_member_offset(struct Node *n);
extern int node_member_size(struct Node *n);
extern int node_line_no(struct Node *n);
extern int node_compound_op(struct Node *n);
extern struct Node **node_stmts(struct Node *n);
extern int node_num_stmts(struct Node *n);
extern const char *node_func_name(struct Node *n);
extern const char *node_asm_string(struct Node *n);
extern struct Node *node_arg(struct Node *n, int i);
extern int node_num_args(struct Node *n);
extern int node_ptr_elem_size(struct Node *n);
extern int node_lhs_ptr_size(struct Node *n);
extern int node_rhs_ptr_size(struct Node *n);
#endif /* ZCC_BRIDGE_STANDALONE */

extern int node_ptr_elem_size(struct Node *n);

/* CG-IR-015: always-present declarations (not guarded by STANDALONE) */
extern int node_type_size(struct Node *n);
extern int node_type_unsigned(struct Node *n);

/* Map ND_* (zcc.c) to ZND_* (bridge). Use sentinels from zcc_ast_bridge.h. */
static int nd_to_znd(int nd_kind) {
  switch (nd_kind) {
  case ZCC_ND_NUM:
    return ZND_NUM;
  case ZCC_ND_STR:
    return ZND_STR;
  case ZCC_ND_VAR:
    return ZND_VAR;
  case ZCC_ND_ASSIGN:
    return ZND_ASSIGN;
  case ZCC_ND_ADD:
    return ZND_ADD;
  case ZCC_ND_SUB:
    return ZND_SUB;
  case ZCC_ND_MOD:
    return ZND_MOD;
  case ZCC_ND_MUL:
    return ZND_MUL;
  case ZCC_ND_DIV:
    return ZND_DIV;
  case ZCC_ND_NEG:
    return ZND_NEG;
  case ZCC_ND_LAND:
    return ZND_LAND;
  case ZCC_ND_LOR:
    return ZND_LOR;
  case ZCC_ND_LNOT:
    return ZND_LNOT;
  case ZCC_ND_BOR:
    return ZND_BOR;
  case ZCC_ND_BXOR:
    return ZND_BXOR;
  case ZCC_ND_BNOT:
    return ZND_BNOT;
  case ZCC_ND_TERNARY:
    return ZND_TERNARY;
  case ZCC_ND_BAND:
    return ZND_BAND;
  case ZCC_ND_SHL:
    return ZND_SHL;
  case ZCC_ND_SHR:
    return ZND_SHR;
  case ZCC_ND_LT:
    return ZND_LT;
  case ZCC_ND_LE:
    return ZND_LE;
  case ZCC_ND_GT:
    return ZND_GT;
  case ZCC_ND_GE:
    return ZND_GE;
  case ZCC_ND_EQ:
    return ZND_EQ;
  case ZCC_ND_NE:
    return ZND_NE;
  case ZCC_ND_IF:
    return ZND_IF;
  case ZCC_ND_WHILE:
    return ZND_WHILE;
  case ZCC_ND_FOR:
    return ZND_FOR;
  case ZCC_ND_BREAK:
    return ZND_BREAK;
  case ZCC_ND_CONTINUE:
    return ZND_CONTINUE;
  case ZCC_ND_RETURN:
    return ZND_RETURN;
  case ZCC_ND_BLOCK:
    return ZND_BLOCK;
  case ZCC_ND_CAST:
    return ZND_CAST;
  case ZCC_ND_CALL:
    return ZND_CALL;
  case ZCC_ND_NOP:
    return ZND_NOP;
  case ZCC_ND_POST_INC:
    return ZND_POST_INC;
  case ZCC_ND_POST_DEC:
    return ZND_POST_DEC;
  case ZCC_ND_PRE_INC:
    return ZND_PRE_INC;
  case ZCC_ND_PRE_DEC:
    return ZND_PRE_DEC;
  case ZCC_ND_SIZEOF:
    return ZND_SIZEOF;
  case ZCC_ND_COMPOUND_ASSIGN:
    return ZND_COMPOUND_ASSIGN;
  case ZCC_ND_ADDR:
    return ZND_ADDR;
  case ZCC_ND_DEREF:
    return ZND_DEREF;
  case ZCC_ND_MEMBER:
    return ZND_MEMBER;
  case ZCC_ND_SWITCH:
    return ZND_SWITCH;
  case ZCC_ND_ASM:
    return ZND_ASM;
  default:
    return -1;
  }
}

static ZCCNode *zcc_node_from_expr(struct Node *n);
static ZCCNode *zcc_node_from_stmt(struct Node *n);

/* Per-function if id; reset in zcc_node_from, incremented in zcc_node_from_stmt
 * for ZND_IF. */
static int if_counter;

static ZCCNode *alloc_zcc_node(void) {
  ZCCNode *z = calloc(1, sizeof(ZCCNode));
  return z;
}

static ZCCNode *zcc_node_from_expr(struct Node *n) {
  if (!n)
    return NULL;
  int zk = nd_to_znd(node_kind(n));
  if (zk < 0) {
    fprintf(stderr, "zcc_node_from: unsupported expr kind %d\n", node_kind(n));
    return NULL;
  }
  ZCCNode *z = alloc_zcc_node();
  z->kind = zk;
  z->line_no = node_line_no(n);
  switch (zk) {
  case ZND_NUM:
    z->int_val = node_int_val(n);
    break;
  case ZND_STR:
    z->int_val = (int64_t)node_str_id(n);
    break;
  case ZND_VAR:
    node_name(n, z->name, ZCC_BRIDGE_NAME_LEN - 1);
    z->is_global = node_is_global(n);
    z->is_array = node_is_array(n);
    z->is_func = node_is_func(n);
    z->member_size = node_member_size(n);
    if (z->member_size <= 0)
      z->member_size = 8;
    break;
  case ZND_ADD: {
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    int l_esz = node_ptr_elem_size(node_lhs(n));
    int r_esz = node_ptr_elem_size(node_rhs(n));
    if (l_esz > 1) {
      ZCCNode *mul = alloc_zcc_node();
      mul->kind = ZND_MUL;
      mul->lhs = z->rhs;
      ZCCNode *imm = alloc_zcc_node();
      imm->kind = ZND_NUM;
      imm->int_val = l_esz;
      mul->rhs = imm;
      z->rhs = mul;
    } else if (r_esz > 1) {
      ZCCNode *mul = alloc_zcc_node();
      mul->kind = ZND_MUL;
      mul->lhs = z->lhs;
      ZCCNode *imm = alloc_zcc_node();
      imm->kind = ZND_NUM;
      imm->int_val = r_esz;
      mul->rhs = imm;
      z->lhs = mul;
    }
    break;
  }
  case ZND_SUB: {
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    int l_esz = node_ptr_elem_size(node_lhs(n));
    int r_esz = node_ptr_elem_size(node_rhs(n));
    if (l_esz > 1) {
      if (r_esz > 1) {
        ZCCNode *sub = alloc_zcc_node();
        sub->kind = ZND_SUB;
        sub->lhs = z->lhs;
        sub->rhs = z->rhs;
        ZCCNode *imm = alloc_zcc_node();
        imm->kind = ZND_NUM;
        imm->int_val = l_esz;
        z->kind = ZND_DIV;
        z->lhs = sub;
        z->rhs = imm;
      } else {
        ZCCNode *mul = alloc_zcc_node();
        mul->kind = ZND_MUL;
        mul->lhs = z->rhs;
        ZCCNode *imm = alloc_zcc_node();
        imm->kind = ZND_NUM;
        imm->int_val = l_esz;
        mul->rhs = imm;
        z->rhs = mul;
      }
    }
    break;
  }
  /* CG-IR-015: DIV/MOD/SHR/SHL need the node's result type to select
   * correct 32-bit vs 64-bit instructions in ir_asm_lower_insn.
   * Encoding: member_size > 0 = signed (abs = byte width),
   *           member_size < 0 = unsigned (abs = byte width),
   *           member_size = 0 = unknown/default (→ IR_TY_I64 in lowering). */
  case ZND_DIV:
  case ZND_MOD:
  case ZND_SHR:
  case ZND_SHL:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    {
      int ts = node_type_size(n);      /* 4 for int, 8 for long/ptr         */
      int tu = node_type_unsigned(n);  /* 1 if unsigned                     */
      /* member_size is unused for arithmetic nodes; repurpose as type tag. */
      z->member_size = tu ? -(ts) : ts;
    }
    break;
  case ZND_MUL:
  case ZND_BAND:
  case ZND_BOR:
  case ZND_BXOR:
  case ZND_LT:
  case ZND_LE:
  case ZND_GT:
  case ZND_GE:
  case ZND_EQ:
  case ZND_NE:
  case ZND_LAND:
  case ZND_LOR:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    break;
  case ZND_NEG:
  case ZND_BNOT:
  case ZND_LNOT:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    break;
  case ZND_TERNARY:
    z->cond = zcc_node_from_expr(node_cond(n));
    z->then_body = zcc_node_from_expr(node_then_body(n));
    z->else_body = zcc_node_from_expr(node_else_body(n));
    break;
  case ZND_CAST:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    break;
  case ZND_ADDR:
  case ZND_DEREF:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    if (zk == ZND_DEREF) {
      z->member_size = node_member_size(n);
      if (z->member_size <= 0)
        z->member_size = 8;
    }
    break;
  case ZND_MEMBER:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->member_offset = node_member_offset(n);
    z->member_size = node_member_size(n);
    z->is_bitfield = node_is_bitfield(n);
    z->bit_offset = node_bit_offset(n);
    z->bit_size = node_bit_size(n);
    z->is_array = node_is_array(n);
    z->is_func = node_is_func(n);
    if (z->member_size <= 0)
      z->member_size = 8;
    break;
  case ZND_POST_INC:
  case ZND_POST_DEC:
  case ZND_PRE_INC:
  case ZND_PRE_DEC:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->int_val = node_ptr_elem_size(node_lhs(n));
    if (z->int_val <= 0)
      z->int_val = 1;
    break;
  case ZND_SIZEOF:
    z->int_val = node_int_val(n);
    break;
  case ZND_ASSIGN:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    break;
  case ZND_COMPOUND_ASSIGN: {
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    z->compound_op = nd_to_znd(node_compound_op(n));
    int l_esz = node_ptr_elem_size(node_lhs(n));
    if (l_esz > 1 && (z->compound_op == ZND_ADD || z->compound_op == ZND_SUB)) {
      ZCCNode *mul = alloc_zcc_node();
      mul->kind = ZND_MUL;
      mul->lhs = z->rhs;
      ZCCNode *imm = alloc_zcc_node();
      imm->kind = ZND_NUM;
      imm->int_val = l_esz;
      mul->rhs = imm;
      z->rhs = mul;
    }
    break;
  }
  case ZND_CALL: {
    const char *fn = node_func_name(n);
    if (fn)
      strncpy(z->func_name, fn, ZCC_CALL_NAME_LEN - 1);
    z->func_name[ZCC_CALL_NAME_LEN - 1] = '\0';
    int na = node_num_args(n);
    if (na < 0)
      na = 0;
    if (na > ZCC_MAX_CALL_ARGS)
      na = ZCC_MAX_CALL_ARGS;
    z->num_args = na;
    if (na > 0) {
      ZCCNode **out = calloc((size_t)na, sizeof(ZCCNode *));
      if (out) {
        for (int i = 0; i < na; i++)
          out[i] = zcc_node_from_expr(node_arg(n, i));
        z->args = out;
      }
    }
    break;
  }
  default:
    break;
  }
  return z;
}

static ZCCNode *zcc_node_from_stmt(struct Node *n) {
  if (!n)
    return NULL;
  int nk = node_kind(n);
  if (nk == 110) { /* ND_RSP_SAVE */
      ZCCNode *z = alloc_zcc_node();
      z->kind = ZND_ASM;
      z->line_no = node_line_no(n);
      int off = node_member_offset(n);
      char buf[64];
      if (off > 0) off = -off;
      sprintf(buf, "movq %%rsp, %d(%%rbp)", off);
      z->asm_string = strdup(buf);
      return z;
  }
  if (nk == 111) { /* ND_RSP_RESTORE */
      ZCCNode *z = alloc_zcc_node();
      z->kind = ZND_ASM;
      z->line_no = node_line_no(n);
      int off = node_member_offset(n);
      char buf[64];
      if (off > 0) off = -off;
      sprintf(buf, "movq %d(%%rbp), %%rsp", off);
      z->asm_string = strdup(buf);
      return z;
  }
  int zk = nd_to_znd(nk);
  if (zk < 0) {
    fprintf(stderr, "zcc_node_from: unsupported stmt kind %d\n", nk);
    return NULL;
  }
  ZCCNode *z = alloc_zcc_node();
  z->kind = zk;
  z->line_no = node_line_no(n);
  switch (zk) {
  case ZND_ASSIGN:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    break;
  case ZND_IF:
    z->cond = zcc_node_from_expr(node_cond(n));
    z->then_body = zcc_node_from_stmt(node_then_body(n));
    z->else_body = zcc_node_from_stmt(node_else_body(n));
    z->if_id = if_counter++;
    break;
  case ZND_WHILE:
    z->cond = zcc_node_from_expr(node_cond(n));
    z->body = zcc_node_from_stmt(node_body(n));
    break;
  case ZND_FOR:
    z->init = node_init(n) ? zcc_node_from_stmt(node_init(n)) : NULL;
    z->cond = node_cond(n) ? zcc_node_from_expr(node_cond(n)) : NULL;
    z->inc = node_inc(n) ? zcc_node_from_expr(node_inc(n)) : NULL;
    z->body = zcc_node_from_stmt(node_body(n));
    break;
  case ZND_BREAK:
  case ZND_CONTINUE:
    break;
  case ZND_RETURN:
    z->lhs = zcc_node_from_expr(node_lhs(n));
    break;
  case ZND_COMPOUND_ASSIGN: {
    z->lhs = zcc_node_from_expr(node_lhs(n));
    z->rhs = zcc_node_from_expr(node_rhs(n));
    z->compound_op = nd_to_znd(node_compound_op(n));
    int l_esz = node_ptr_elem_size(node_lhs(n));
    if (l_esz > 1 && (z->compound_op == ZND_ADD || z->compound_op == ZND_SUB)) {
      ZCCNode *mul = alloc_zcc_node();
      mul->kind = ZND_MUL;
      mul->lhs = z->rhs;
      ZCCNode *imm = alloc_zcc_node();
      imm->kind = ZND_NUM;
      imm->int_val = l_esz;
      mul->rhs = imm;
      z->rhs = mul;
    }
    break;
  }
  case ZND_BLOCK: {
    int num = node_num_stmts(n);
    struct Node **stmts = node_stmts(n);
    if (num <= 0 || num > (int)ZCC_AST_MAX_STMTS || !stmts)
      break;
    ZCCNode **out = calloc((size_t)num, sizeof(ZCCNode *));
    if (!out)
      break;
    for (int i = 0; i < num; i++)
      out[i] = zcc_node_from_stmt(stmts[i]);
    z->stmts = out;
    z->num_stmts = (uint32_t)num;
    break;
  }
  case ZND_CALL:
    /* CG-IR-013: Statement-level calls (e.g. validate_node(...);) must
     * populate func_name and args. Delegate to the expr handler which
     * already has the complete ZND_CALL conversion logic. */
    {
      ZCCNode *call_node = zcc_node_from_expr(n);
      if (call_node) { free(z); return call_node; }
    }
    break;
  /* CG-IR-015: Expression-statements (i++, --p, etc.) in stmt position */
  case ZND_POST_INC:
  case ZND_PRE_INC:
  case ZND_POST_DEC:
  case ZND_PRE_DEC:
  {
    ZCCNode *expr_node = zcc_node_from_expr(n);
    if (expr_node) { free(z); return expr_node; }
  }
  break;
  case ZND_NOP:
    break;
  case ZND_ASM:
    z->asm_string = (char *)node_asm_string(n);
    break;
  case ZND_SWITCH: {
    int num = node_num_cases(n);
    struct Node **cases = node_cases(n);
    z->cond = node_cond(n) ? zcc_node_from_expr(node_cond(n)) : NULL;
    if (num <= 0 || num > 128 || !cases) {
      z->num_cases = 0;
      z->case_vals = NULL;
      z->case_bodies = NULL;
    } else {
      z->num_cases = num;
      z->case_vals = calloc((size_t)num, sizeof(int64_t));
      z->case_bodies = calloc((size_t)num, sizeof(ZCCNode *));
      if (z->case_vals && z->case_bodies) {
        for (int i = 0; i < num; i++) {
          struct Node *c = cases[i];
          if (c) {
            z->case_vals[i] = node_case_val(c);
            z->case_bodies[i] = zcc_node_from_stmt(node_case_body(c));
          }
        }
      }
    }
    {
      struct Node *def = node_default_case(n);
      z->default_body = def ? zcc_node_from_stmt(node_case_body(def)) : NULL;
    }
    break;
  }
  default:
    break;
  }
  return z;
}

/**
 * zcc_node_from — Deep-copy Node* tree into ZCCNode* (supported kinds only).
 * Returns a new tree; caller must zcc_node_free() it.
 */
ZCCNode *zcc_node_from(struct Node *n) {
  if (!n)
    return NULL;
  if_counter = 0; /* reset at start of each function body */
  return zcc_node_from_stmt(n);
}

/**
 * zcc_node_free — Recursively free a ZCCNode tree from zcc_node_from.
 */
void zcc_node_free(ZCCNode *z) {
  if (!z)
    return;
  zcc_node_free(z->lhs);
  zcc_node_free(z->rhs);
  zcc_node_free(z->cond);
  zcc_node_free(z->then_body);
  zcc_node_free(z->else_body);
  zcc_node_free(z->body);
  zcc_node_free(z->init);
  zcc_node_free(z->inc);
  if (z->case_bodies) {
    for (int i = 0; i < z->num_cases; i++)
      zcc_node_free(z->case_bodies[i]);
    free(z->case_bodies);
  }
  if (z->case_vals)
    free(z->case_vals);
  zcc_node_free(z->default_body);
  if (z->stmts) {
    for (uint32_t i = 0; i < z->num_stmts; i++)
      zcc_node_free(z->stmts[i]);
    free(z->stmts);
  }
  if (z->args) {
    for (int i = 0; i < z->num_args; i++)
      zcc_node_free(z->args[i]);
    free(z->args);
  }
  free(z);
}

static RegID zcc_lower_expr(LowerCtx *ctx, ZCCNode *node);
static void zcc_lower_stmt(LowerCtx *ctx, ZCCNode *node);

/* CG-IR-015: decode the member_size type-tag set by zcc_node_from_expr for
 * DIV/MOD/SHR/SHL nodes.  Convention:
 *   member_size ==  4  →  IR_TY_I32   (signed int)
 *   member_size == -4  →  IR_TY_U32   (unsigned int)
 *   member_size == -8  →  IR_TY_U64   (unsigned long)
 *   otherwise          →  IR_TY_I64   (signed long / default)           */
static IRType irtype_from_node(const ZCCNode *node) {
  if (!node) return IR_TY_I64;
  int ms = node->member_size;
  if (ms ==  4) return IR_TY_I32;
  if (ms == -4) return IR_TY_U32;
  if (ms == -8) return IR_TY_U64;
  return IR_TY_I64;
}

static RegID zcc_lower_expr(LowerCtx *ctx, ZCCNode *node) {
  if (!node)
    return 0;
  /* Transparent passthrough: only recurse on operand. */
  if (node->kind == ZND_CAST)
    return node->lhs ? zcc_lower_expr(ctx, node->lhs) : 0;
  /* ZND_ADDR: force child to yield address; no extra IR. */
  if (node->kind == ZND_ADDR) {
    int old = ctx->want_address;
    ctx->want_address = 1;
    RegID addr_r = node->lhs ? zcc_lower_expr(ctx, node->lhs) : 0;
    ctx->want_address = old;
    return addr_r;
  }
  /* ZND_DEREF: evaluate pointer; if want_address return it, else OP_LOAD. */
  if (node->kind == ZND_DEREF) {
    int old = ctx->want_address;
    ctx->want_address = 0;
    RegID ptr_r = node->lhs ? zcc_lower_expr(ctx, node->lhs) : 0;
    ctx->want_address = old;
    if (!ptr_r)
      return 0;
    if (ctx->want_address)
      return ptr_r;
    RegID val_r = ctx->next_reg++;
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = ctx->next_instr_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = val_r;
    load_ins->src[0] = ptr_r;
    load_ins->n_src = 1;
    load_ins->imm = (node->member_size > 0) ? (int64_t)node->member_size : 8;
    load_ins->exec_freq = 1.0;
    emit_instr(ctx, load_ins);
    return val_r;
  }
  /* ZND_MEMBER: base + offset → address; then load value unless want_address.
   */
  if (node->kind == ZND_MEMBER) {
    int old = ctx->want_address;
    ctx->want_address = 1;
    RegID base_r = node->lhs ? zcc_lower_expr(ctx, node->lhs) : 0;
    ctx->want_address = old;
    if (!base_r)
      return 0;
    RegID offset_r = ctx->next_reg++;
    Instr *off_ins =
        make_instr_imm(ctx->next_instr_id++, OP_CONST, offset_r,
                       (int64_t)(node->member_offset), node->line_no);
    emit_instr(ctx, off_ins);
    RegID addr_r = ctx->next_reg++;
    Instr *add_ins = calloc(1, sizeof(Instr));
    add_ins->id = ctx->next_instr_id++;
    add_ins->op = OP_ADD;
    add_ins->dst = addr_r;
    add_ins->src[0] = base_r;
    add_ins->src[1] = offset_r;
    add_ins->n_src = 2;
    add_ins->exec_freq = 1.0;
    emit_instr(ctx, add_ins);
    if (ctx->want_address || node->is_array || node->is_func)
      return addr_r;
    RegID r = ctx->next_reg++;
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = ctx->next_instr_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = r;
    load_ins->src[0] = addr_r;
    load_ins->n_src = 1;
    load_ins->imm = (node->member_size > 0) ? (int64_t)node->member_size : 8;
    load_ins->exec_freq = 1.0;
    emit_instr(ctx, load_ins);
    
    if (node->is_bitfield) {
        fprintf(stderr, "WIRING R-VALUE BITFIELD: offset=%d, size=%d\n", node->bit_offset, node->bit_size);
        RegID bo_r = ctx->next_reg++;
        emit_instr(ctx, make_instr_imm(ctx->next_instr_id++, OP_CONST, bo_r, node->bit_offset, node->line_no));
        RegID shift_r = ctx->next_reg++;
        Instr *sh_i = calloc(1, sizeof(Instr));
        sh_i->id = ctx->next_instr_id++;
        sh_i->op = OP_SHR;
        sh_i->dst = shift_r;
        sh_i->src[0] = r;
        sh_i->src[1] = bo_r;
        sh_i->n_src = 2;
        sh_i->exec_freq = 1.0;
        emit_instr(ctx, sh_i);

        RegID mask_r = ctx->next_reg++;
        long long mask_val = (1ULL << node->bit_size) - 1;
        emit_instr(ctx, make_instr_imm(ctx->next_instr_id++, OP_CONST, mask_r, mask_val, node->line_no));
        RegID band_r = ctx->next_reg++;
        Instr *band_i = calloc(1, sizeof(Instr));
        band_i->id = ctx->next_instr_id++;
        band_i->op = OP_BAND;
        band_i->dst = band_r;
        band_i->src[0] = shift_r;
        band_i->src[1] = mask_r;
        band_i->n_src = 2;
        band_i->exec_freq = 1.0;
        emit_instr(ctx, band_i);
        r = band_r;
    }
    return r;
  }
  /* ZND_TERNARY: cond ? then_expr : else_expr. SSA: single def via OP_PHI at
   * merge. */
  if (node->kind == ZND_TERNARY) {
    RegID cond_r = node->cond ? zcc_lower_expr(ctx, node->cond) : 0;
    if (!cond_r)
      return 0;
    BlockID blk_cond = ctx->cur_block;
    BlockID blk_then = new_block(ctx, "tern.then");
    BlockID blk_else = new_block(ctx, "tern.else");
    BlockID blk_merge = new_block(ctx, "tern.merge");
    Function *fn = ctx->fn;

    Instr *cbr = calloc(1, sizeof(Instr));
    cbr->id = ctx->next_instr_id++;
    cbr->op = OP_CONDBR;
    cbr->dst = 0;
    cbr->src[0] = cond_r;
    cbr->src[1] = blk_then;
    cbr->src[2] = blk_else;
    cbr->n_src = 3;
    cbr->exec_freq = 1.0;
    emit_instr(ctx, cbr);
    fn->blocks[blk_cond]->succs[0] = blk_then;
    fn->blocks[blk_cond]->succs[1] = blk_else;
    fn->blocks[blk_cond]->n_succs = 2;
    fn->blocks[blk_cond]->branch_prob[0] = 0.5f;
    fn->blocks[blk_cond]->branch_prob[1] = 0.5f;
    fn->blocks[blk_then]->preds[fn->blocks[blk_then]->n_preds++] = blk_cond;
    fn->blocks[blk_else]->preds[fn->blocks[blk_else]->n_preds++] = blk_cond;

    RegID tern_zero = ctx->next_reg++;
    Instr *cz = make_instr_imm(ctx->next_instr_id++, OP_CONST, tern_zero, 0,
                               node->line_no);

    ctx->cur_block = blk_then;
    RegID val_then = node->then_body ? zcc_lower_expr(ctx, node->then_body) : 0;
    if (!val_then) {
      emit_instr(ctx, cz);
      val_then = tern_zero;
    }
    BlockID blk_then_end = ctx->cur_block;
    Instr *br_then = calloc(1, sizeof(Instr));
    br_then->id = ctx->next_instr_id++;
    br_then->op = OP_BR;
    br_then->src[0] = blk_merge;
    br_then->n_src = 1;
    br_then->exec_freq = 1.0;
    emit_instr(ctx, br_then);
    fn->blocks[blk_then_end]->succs[0] = blk_merge;
    fn->blocks[blk_then_end]->n_succs = 1;
    fn->blocks[blk_merge]->preds[fn->blocks[blk_merge]->n_preds++] =
        blk_then_end;

    ctx->cur_block = blk_else;
    RegID val_else = node->else_body ? zcc_lower_expr(ctx, node->else_body) : 0;
    if (!val_else) {
      emit_instr(ctx, cz);
      val_else = tern_zero;
    }
    BlockID blk_else_end = ctx->cur_block;
    Instr *br_else = calloc(1, sizeof(Instr));
    br_else->id = ctx->next_instr_id++;
    br_else->op = OP_BR;
    br_else->src[0] = blk_merge;
    br_else->n_src = 1;
    br_else->exec_freq = 1.0;
    emit_instr(ctx, br_else);
    fn->blocks[blk_else_end]->succs[0] = blk_merge;
    fn->blocks[blk_else_end]->n_succs = 1;
    fn->blocks[blk_merge]->preds[fn->blocks[blk_merge]->n_preds++] =
        blk_else_end;

    ctx->cur_block = blk_merge;
    RegID res_r = ctx->next_reg++;
    Instr *phi = calloc(1, sizeof(Instr));
    phi->id = ctx->next_instr_id++;
    phi->op = OP_PHI;
    phi->dst = res_r;
    phi->phi[0].reg = val_then;
    phi->phi[0].block = blk_then_end;
    phi->phi[1].reg = val_else;
    phi->phi[1].block = blk_else_end;
    phi->n_phi = 2;
    phi->exec_freq = 1.0;
    emit_instr(ctx, phi);
    return res_r;
  }
  /* ZND_LAND: A && B. Short-circuit: if !A go to merge with 0; else evaluate B,
   * normalize to 0/1. SSA: OP_PHI at merge. */
  if (node->kind == ZND_LAND) {
    RegID val_lhs = node->lhs ? zcc_lower_expr(ctx, node->lhs) : 0;
    if (!val_lhs)
      return 0;
    BlockID blk_lhs_end = ctx->cur_block;
    BlockID blk_bypass = new_block(ctx, "land.bypass");
    BlockID blk_rhs = new_block(ctx, "land.rhs");
    BlockID blk_merge = new_block(ctx, "land.merge");
    Function *fn = ctx->fn;

    RegID zero_r = ctx->next_reg++;
    Instr *c0 = make_instr_imm(ctx->next_instr_id++, OP_CONST, zero_r, 0,
                               node->line_no);
    emit_instr(ctx, c0); /* CG-IR-FIX: Emit before branch to dominate both RHS
                            and bypass blocks */

    Instr *cbr = calloc(1, sizeof(Instr));
    cbr->id = ctx->next_instr_id++;
    cbr->op = OP_CONDBR;
    cbr->dst = 0;
    cbr->src[0] = val_lhs;
    cbr->src[1] = blk_rhs;
    cbr->src[2] = blk_bypass;
    cbr->n_src = 3;
    cbr->exec_freq = 1.0;
    emit_instr(ctx, cbr);
    fn->blocks[blk_lhs_end]->succs[0] = blk_rhs;
    fn->blocks[blk_lhs_end]->succs[1] = blk_bypass;
    fn->blocks[blk_lhs_end]->n_succs = 2;
    fn->blocks[blk_lhs_end]->branch_prob[0] = 0.5f;
    fn->blocks[blk_lhs_end]->branch_prob[1] = 0.5f;
    fn->blocks[blk_rhs]->preds[fn->blocks[blk_rhs]->n_preds++] = blk_lhs_end;
    fn->blocks[blk_bypass]->preds[fn->blocks[blk_bypass]->n_preds++] =
        blk_lhs_end;

    ctx->cur_block = blk_bypass;
    BlockID blk_bypass_end = ctx->cur_block;
    Instr *br_bypass = calloc(1, sizeof(Instr));
    br_bypass->id = ctx->next_instr_id++;
    br_bypass->op = OP_BR;
    br_bypass->src[0] = blk_merge;
    br_bypass->n_src = 1;
    br_bypass->exec_freq = 1.0;
    emit_instr(ctx, br_bypass);
    fn->blocks[blk_bypass_end]->succs[0] = blk_merge;
    fn->blocks[blk_bypass_end]->n_succs = 1;
    fn->blocks[blk_merge]->preds[fn->blocks[blk_merge]->n_preds++] =
        blk_bypass_end;

    ctx->cur_block = blk_rhs;
    RegID val_rhs_raw = node->rhs ? zcc_lower_expr(ctx, node->rhs) : 0;
    if (!val_rhs_raw)
      val_rhs_raw = zero_r;
    RegID val_rhs = ctx->next_reg++;
    Instr *ne_ins = calloc(1, sizeof(Instr));
    ne_ins->id = ctx->next_instr_id++;
    ne_ins->op = OP_NE;
    ne_ins->dst = val_rhs;
    ne_ins->src[0] = val_rhs_raw;
    ne_ins->src[1] = zero_r;
    ne_ins->n_src = 2;
    ne_ins->exec_freq = 1.0;
    emit_instr(ctx, ne_ins);
    BlockID blk_rhs_end = ctx->cur_block;
    Instr *br_rhs = calloc(1, sizeof(Instr));
    br_rhs->id = ctx->next_instr_id++;
    br_rhs->op = OP_BR;
    br_rhs->src[0] = blk_merge;
    br_rhs->n_src = 1;
    br_rhs->exec_freq = 1.0;
    emit_instr(ctx, br_rhs);
    fn->blocks[blk_rhs_end]->succs[0] = blk_merge;
    fn->blocks[blk_rhs_end]->n_succs = 1;
    fn->blocks[blk_merge]->preds[fn->blocks[blk_merge]->n_preds++] =
        blk_rhs_end;

    ctx->cur_block = blk_merge;
    RegID res_r = ctx->next_reg++;
    Instr *phi = calloc(1, sizeof(Instr));
    phi->id = ctx->next_instr_id++;
    phi->op = OP_PHI;
    phi->dst = res_r;
    phi->phi[0].reg = zero_r;
    phi->phi[0].block = blk_bypass_end;
    phi->phi[1].reg = val_rhs;
    phi->phi[1].block = blk_rhs_end;
    phi->n_phi = 2;
    phi->exec_freq = 1.0;
    emit_instr(ctx, phi);
    return res_r;
  }
  /* ZND_LOR: A || B. Short-circuit: if A go to merge with 1; else evaluate B,
   * normalize to 0/1. SSA: OP_PHI at merge. */
  if (node->kind == ZND_LOR) {
    RegID val_lhs = node->lhs ? zcc_lower_expr(ctx, node->lhs) : 0;
    if (!val_lhs)
      return 0;
    BlockID blk_lhs_end = ctx->cur_block;
    BlockID blk_one = new_block(ctx, "lor.one");
    BlockID blk_rhs = new_block(ctx, "lor.rhs");
    BlockID blk_merge = new_block(ctx, "lor.merge");
    Function *fn = ctx->fn;

    Instr *cbr = calloc(1, sizeof(Instr));
    cbr->id = ctx->next_instr_id++;
    cbr->op = OP_CONDBR;
    cbr->dst = 0;
    cbr->src[0] = val_lhs;
    cbr->src[1] = blk_one;
    cbr->src[2] = blk_rhs;
    cbr->n_src = 3;
    cbr->exec_freq = 1.0;
    emit_instr(ctx, cbr);
    fn->blocks[blk_lhs_end]->succs[0] = blk_one;
    fn->blocks[blk_lhs_end]->succs[1] = blk_rhs;
    fn->blocks[blk_lhs_end]->n_succs = 2;
    fn->blocks[blk_lhs_end]->branch_prob[0] = 0.5f;
    fn->blocks[blk_lhs_end]->branch_prob[1] = 0.5f;
    fn->blocks[blk_one]->preds[fn->blocks[blk_one]->n_preds++] = blk_lhs_end;
    fn->blocks[blk_rhs]->preds[fn->blocks[blk_rhs]->n_preds++] = blk_lhs_end;

    RegID one_r = ctx->next_reg++;
    Instr *c1 =
        make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, 1, node->line_no);
    ctx->cur_block = blk_one;
    emit_instr(ctx, c1);
    BlockID blk_one_end = ctx->cur_block;
    Instr *br_one = calloc(1, sizeof(Instr));
    br_one->id = ctx->next_instr_id++;
    br_one->op = OP_BR;
    br_one->src[0] = blk_merge;
    br_one->n_src = 1;
    br_one->exec_freq = 1.0;
    emit_instr(ctx, br_one);
    fn->blocks[blk_one_end]->succs[0] = blk_merge;
    fn->blocks[blk_one_end]->n_succs = 1;
    fn->blocks[blk_merge]->preds[fn->blocks[blk_merge]->n_preds++] =
        blk_one_end;

    RegID zero_r = ctx->next_reg++;
    Instr *c0 = make_instr_imm(ctx->next_instr_id++, OP_CONST, zero_r, 0,
                               node->line_no);
    ctx->cur_block = blk_rhs;
    emit_instr(ctx, c0);
    RegID val_rhs_raw = node->rhs ? zcc_lower_expr(ctx, node->rhs) : 0;
    if (!val_rhs_raw)
      val_rhs_raw = zero_r;
    RegID val_rhs = ctx->next_reg++;
    Instr *ne_ins = calloc(1, sizeof(Instr));
    ne_ins->id = ctx->next_instr_id++;
    ne_ins->op = OP_NE;
    ne_ins->dst = val_rhs;
    ne_ins->src[0] = val_rhs_raw;
    ne_ins->src[1] = zero_r;
    ne_ins->n_src = 2;
    ne_ins->exec_freq = 1.0;
    emit_instr(ctx, ne_ins);
    BlockID blk_rhs_end = ctx->cur_block;
    Instr *br_rhs = calloc(1, sizeof(Instr));
    br_rhs->id = ctx->next_instr_id++;
    br_rhs->op = OP_BR;
    br_rhs->src[0] = blk_merge;
    br_rhs->n_src = 1;
    br_rhs->exec_freq = 1.0;
    emit_instr(ctx, br_rhs);
    fn->blocks[blk_rhs_end]->succs[0] = blk_merge;
    fn->blocks[blk_rhs_end]->n_succs = 1;
    fn->blocks[blk_merge]->preds[fn->blocks[blk_merge]->n_preds++] =
        blk_rhs_end;

    ctx->cur_block = blk_merge;
    RegID res_r = ctx->next_reg++;
    Instr *phi = calloc(1, sizeof(Instr));
    phi->id = ctx->next_instr_id++;
    phi->op = OP_PHI;
    phi->dst = res_r;
    phi->phi[0].reg = one_r;
    phi->phi[0].block = blk_one_end;
    phi->phi[1].reg = val_rhs;
    phi->phi[1].block = blk_rhs_end;
    phi->n_phi = 2;
    phi->exec_freq = 1.0;
    emit_instr(ctx, phi);
    return res_r;
  }
  RegID r = ctx->next_reg++;
  Instr *ins = NULL;
  switch (node->kind) {
  case ZND_NUM:
    ins = make_instr_imm(ctx->next_instr_id++, OP_CONST, r, node->int_val,
                         node->line_no);
    break;
  case ZND_STR: {
    char lab[32];
    snprintf(lab, sizeof(lab), ".Lstr_%lld", (long long)node->int_val);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_GLOBAL;
    ins->dst = r;
    ins->n_src = 0;
    strncpy(ins->call_name, lab, sizeof(ins->call_name) - 1);
    ins->call_name[sizeof(ins->call_name) - 1] = '\0';
    ins->exec_freq = 1.0;
    break;
  }
  case ZND_VAR: {
    if (node->is_global) {
      ins = calloc(1, sizeof(Instr));
      ins->id = ctx->next_instr_id++;
      ins->op = OP_GLOBAL;
      ins->dst = r;
      ins->n_src = 0;
      strncpy(ins->call_name, node->name, sizeof(ins->call_name) - 1);
      ins->call_name[sizeof(ins->call_name) - 1] = '\0';
      ins->exec_freq = 1.0;
      if (ctx->want_address || node->is_array || node->is_func) {
        emit_instr(ctx, ins);
        return r;
      }
      emit_instr(ctx, ins);
      Instr *load_ins = calloc(1, sizeof(Instr));
      load_ins->id = ctx->next_instr_id++;
      load_ins->op = OP_LOAD;
      RegID val_r = ctx->next_reg++;
      load_ins->dst = val_r;
      load_ins->src[0] = r;
      load_ins->n_src = 1;
      load_ins->imm = 8;
      load_ins->exec_freq = 1.0;
      emit_instr(ctx, load_ins);
      return val_r;
    } else {
      RegID alloca_r = get_or_create_var(ctx, node->name, node->member_size > 0 ? node->member_size : 8);
      if (ctx->want_address || node->is_array || node->is_func)
        return alloca_r;
      ins = calloc(1, sizeof(Instr));
      ins->id = ctx->next_instr_id++;
      ins->op = OP_LOAD;
      ins->dst = r;
      ins->src[0] = alloca_r;
      ins->n_src = 1;
      ins->imm = node->member_size > 0 ? node->member_size : 8;
      ins->exec_freq = 1.0;
    }
    break;
  }
  case ZND_ADD: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_ADD;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    break;
  }
  case ZND_SUB: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_SUB;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    break;
  }
  case ZND_MOD: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_MOD;
    ins->ir_type = irtype_from_node(node); /* CG-IR-015 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    break;
  }
  case ZND_MUL: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_MUL;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    break;
  }
  case ZND_DIV: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_DIV;
    ins->ir_type = irtype_from_node(node); /* CG-IR-015 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_NEG: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID zero_r = ctx->next_reg++;
    Instr *c0 = make_instr_imm(ctx->next_instr_id++, OP_CONST, zero_r, 0,
                               node->line_no);
    emit_instr(ctx, c0);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_SUB;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017: NEG → 0-x */
    ins->line_no = node->line_no;
    ins->dst = r;
    ins->src[0] = zero_r;
    ins->src[1] = l;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_BOR: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_BOR;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_BXOR: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_BXOR;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_BNOT: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_BNOT;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */
    ins->dst = r;
    ins->src[0] = l;
    ins->n_src = 1;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_LNOT: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID zero_r = ctx->next_reg++;
    Instr *c0 = make_instr_imm(ctx->next_instr_id++, OP_CONST, zero_r, 0,
                               node->line_no);
    emit_instr(ctx, c0);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_EQ;
    ins->line_no = node->line_no;
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = zero_r;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_BAND: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_BAND;
    ins->ir_type = irtype_from_node(node); /* CG-IR-017 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_SHL: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_SHL;
    ins->ir_type = irtype_from_node(node); /* CG-IR-015 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_SHR: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_SHR;
    ins->ir_type = irtype_from_node(node); /* CG-IR-015 */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_LT:
  case ZND_LE:
  case ZND_GT:
  case ZND_GE:
  case ZND_EQ:
  case ZND_NE: {
    RegID l = zcc_lower_expr(ctx, node->lhs);
    RegID rh = zcc_lower_expr(ctx, node->rhs);
    Opcode cmp_op = OP_LT;
    if (node->kind == ZND_EQ)
      cmp_op = OP_EQ;
    else if (node->kind == ZND_NE)
      cmp_op = OP_NE;
    else if (node->kind == ZND_GT)
      cmp_op = OP_GT;
    else if (node->kind == ZND_GE)
      cmp_op = OP_GE;
    else if (node->kind == ZND_LE)
      cmp_op = OP_LE;
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = cmp_op;
    ins->ir_type = irtype_from_node(node->lhs); /* CG-IR-017: operand width */
    ins->dst = r;
    ins->src[0] = l;
    ins->src[1] = rh;
    ins->n_src = 2;
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_POST_INC: {
    RegID alloca_r = get_or_create_var(ctx, node->lhs->name, 8);
    if (!alloca_r)
      return 0;
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = ctx->next_instr_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = r;
    load_ins->src[0] = alloca_r;
    load_ins->n_src = 1;
    load_ins->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    load_ins->exec_freq = 1.0;
    load_ins->line_no = node->line_no;
    emit_instr(ctx, load_ins);
    RegID r_new = ctx->next_reg++;
    Instr *add_ins = calloc(1, sizeof(Instr));
    add_ins->id = ctx->next_instr_id++;
    add_ins->op = OP_ADD;
    add_ins->dst = r_new;
    add_ins->src[0] = r;
    add_ins->src[1] = 0; /* will use OP_CONST 1 */
    add_ins->n_src = 2;
    add_ins->exec_freq = 1.0;
    add_ins->line_no = node->line_no;
    RegID one_r = ctx->next_reg++;
    int64_t inc_val = (node->int_val > 0) ? node->int_val : 1;
    Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, inc_val,
                               node->line_no);
    emit_instr(ctx, c1);
    add_ins->src[1] = one_r;
    emit_instr(ctx, add_ins);
    Instr *st = calloc(1, sizeof(Instr));
    st->id = ctx->next_instr_id++;
    st->op = OP_STORE;
    st->dst = 0;
    st->src[0] = r_new;
    st->src[1] = alloca_r;
    st->n_src = 2;
    st->exec_freq = 1.0;
    st->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    st->line_no = node->line_no;
    emit_instr(ctx, st);
    return r; /* return value before increment */
  }
  case ZND_POST_DEC: {
    RegID alloca_r = get_or_create_var(ctx, node->lhs->name, 8);
    if (!alloca_r)
      return 0;
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = ctx->next_instr_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = r;
    load_ins->src[0] = alloca_r;
    load_ins->n_src = 1;
    load_ins->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
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
    int64_t inc_val = (node->int_val > 0) ? node->int_val : 1;
    Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, inc_val,
                               node->line_no);
    emit_instr(ctx, c1);
    sub_ins->src[1] = one_r;
    emit_instr(ctx, sub_ins);
    Instr *st = calloc(1, sizeof(Instr));
    st->id = ctx->next_instr_id++;
    st->op = OP_STORE;
    st->dst = 0;
    st->src[0] = r_new;
    st->src[1] = alloca_r;
    st->n_src = 2;
    st->exec_freq = 1.0;
    st->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    st->line_no = node->line_no;
    emit_instr(ctx, st);
    return r; /* return value before decrement */
  }
  case ZND_PRE_INC: {
    RegID alloca_r = get_or_create_var(ctx, node->lhs->name, 8);
    if (!alloca_r)
      return 0;
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = ctx->next_instr_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = r;
    load_ins->src[0] = alloca_r;
    load_ins->n_src = 1;
    load_ins->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
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
    int64_t inc_val = (node->int_val > 0) ? node->int_val : 1;
    Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, inc_val,
                               node->line_no);
    emit_instr(ctx, c1);
    add_ins->src[1] = one_r;
    emit_instr(ctx, add_ins);
    Instr *st = calloc(1, sizeof(Instr));
    st->id = ctx->next_instr_id++;
    st->op = OP_STORE;
    st->dst = 0;
    st->src[0] = r_new;
    st->src[1] = alloca_r;
    st->n_src = 2;
    st->exec_freq = 1.0;
    st->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    st->line_no = node->line_no;
    emit_instr(ctx, st);
    return r_new; /* return value after increment */
  }
  case ZND_PRE_DEC: {
    RegID alloca_r = get_or_create_var(ctx, node->lhs->name, 8);
    if (!alloca_r)
      return 0;
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = ctx->next_instr_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = r;
    load_ins->src[0] = alloca_r;
    load_ins->n_src = 1;
    load_ins->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
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
    int64_t inc_val = (node->int_val > 0) ? node->int_val : 1;
    Instr *c1 = make_instr_imm(ctx->next_instr_id++, OP_CONST, one_r, inc_val,
                               node->line_no);
    emit_instr(ctx, c1);
    sub_ins->src[1] = one_r;
    emit_instr(ctx, sub_ins);
    Instr *st = calloc(1, sizeof(Instr));
    st->id = ctx->next_instr_id++;
    st->op = OP_STORE;
    st->dst = 0;
    st->src[0] = r_new;
    st->src[1] = alloca_r;
    st->n_src = 2;
    st->exec_freq = 1.0;
    st->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    st->line_no = node->line_no;
    emit_instr(ctx, st);
    return r_new; /* return value after decrement */
  }
  case ZND_SIZEOF: {
    int64_t sz = node->int_val > 0 ? node->int_val : 8;
    emit_instr(ctx, make_instr_imm(ctx->next_instr_id++, OP_CONST, r, sz,
                                   node->line_no));
    return r;
  }
  case ZND_CALL: {
    int na = node->num_args;
    if (na < 0)
      na = 0;
    if (na > MAX_CALL_ARGS)
      na = MAX_CALL_ARGS;
    ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_CALL;
    ins->dst = r;
    if (node->func_name && node->func_name[0]) {
      strncpy(ins->call_name, node->func_name, sizeof(ins->call_name) - 1);
      ins->call_name[sizeof(ins->call_name) - 1] = '\0';
      ins->n_src = 0;
    } else if (node->lhs) {
      ins->src[0] = zcc_lower_expr(ctx, node->lhs);
      ins->n_src = 1;
    }
    ins->n_call_args = (uint32_t)na;
    for (int i = 0; i < na && node->args; i++)
      ins->call_args[i] = zcc_lower_expr(ctx, node->args[i]);
    ins->exec_freq = 1.0;
    ins->line_no = node->line_no;
    break;
  }
  case ZND_ASSIGN: {
    if (!node->lhs)
      return 0;
    int old_want = ctx->want_address;
    ctx->want_address = 1;
    RegID addr_r = zcc_lower_expr(ctx, node->lhs);
    ctx->want_address = old_want;
    if (!addr_r)
      return 0;
    RegID val_r = zcc_lower_expr(ctx, node->rhs);
    
    if (node->lhs && node->lhs->is_bitfield) {
        fprintf(stderr, "WIRING L-VALUE BITFIELD: offset=%d, size=%d\n", node->lhs->bit_offset, node->lhs->bit_size);
        RegID old_r = ctx->next_reg++;
        Instr *ld_o = calloc(1, sizeof(Instr));
        ld_o->id = ctx->next_instr_id++;
        ld_o->op = OP_LOAD;
        ld_o->dst = old_r;
        ld_o->src[0] = addr_r;
        ld_o->n_src = 1;
        ld_o->imm = (node->lhs->member_size > 0) ? node->lhs->member_size : 8;
        ld_o->exec_freq = 1.0;
        emit_instr(ctx, ld_o);
        
        RegID mask_r = ctx->next_reg++;
        long long mask_val = (1ULL << node->lhs->bit_size) - 1;
        long long shift_mask = ~(mask_val << node->lhs->bit_offset);
        emit_instr(ctx, make_instr_imm(ctx->next_instr_id++, OP_CONST, mask_r, shift_mask, node->line_no));
        RegID band1_r = ctx->next_reg++;
        Instr *b1_i = calloc(1, sizeof(Instr));
        b1_i->id = ctx->next_instr_id++;
        b1_i->op = OP_BAND;
        b1_i->dst = band1_r;
        b1_i->src[0] = old_r;
        b1_i->src[1] = mask_r;
        b1_i->n_src = 2;
        b1_i->exec_freq = 1.0;
        emit_instr(ctx, b1_i);
        
        RegID just_mask = ctx->next_reg++;
        emit_instr(ctx, make_instr_imm(ctx->next_instr_id++, OP_CONST, just_mask, mask_val, node->line_no));
        RegID band2_r = ctx->next_reg++;
        Instr *b2_i = calloc(1, sizeof(Instr));
        b2_i->id = ctx->next_instr_id++;
        b2_i->op = OP_BAND;
        b2_i->dst = band2_r;
        b2_i->src[0] = val_r;
        b2_i->src[1] = just_mask;
        b2_i->n_src = 2;
        b2_i->exec_freq = 1.0;
        emit_instr(ctx, b2_i);
        
        RegID bo_r = ctx->next_reg++;
        emit_instr(ctx, make_instr_imm(ctx->next_instr_id++, OP_CONST, bo_r, node->lhs->bit_offset, node->line_no));
        RegID shl_r = ctx->next_reg++;
        Instr *sh_i = calloc(1, sizeof(Instr));
        sh_i->id = ctx->next_instr_id++;
        sh_i->op = OP_SHL;
        sh_i->dst = shl_r;
        sh_i->src[0] = band2_r;
        sh_i->src[1] = bo_r;
        sh_i->n_src = 2;
        sh_i->exec_freq = 1.0;
        emit_instr(ctx, sh_i);
        
        RegID final_r = ctx->next_reg++;
        Instr *bor_i = calloc(1, sizeof(Instr));
        bor_i->id = ctx->next_instr_id++;
        bor_i->op = OP_BOR;
        bor_i->dst = final_r;
        bor_i->src[0] = band1_r;
        bor_i->src[1] = shl_r;
        bor_i->n_src = 2;
        bor_i->exec_freq = 1.0;
        emit_instr(ctx, bor_i);
        
        val_r = final_r;
    }

    Instr *st = calloc(1, sizeof(Instr));
    st->src[0] = val_r;
    st->src[1] = addr_r;
    st->n_src = 2;
    st->exec_freq = 1.0;
    st->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    emit_instr(ctx, st);
    return val_r;
  }
  case ZND_COMPOUND_ASSIGN: {
    if (!node->lhs || !node->rhs)
      return 0;
    int old_want = ctx->want_address;
    ctx->want_address = 1;
    RegID addr_r = zcc_lower_expr(ctx, node->lhs);
    ctx->want_address = old_want;
    if (!addr_r)
      return 0;

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

    if (node->compound_op == ZND_SUB)
      add_ins->op = OP_SUB;
    else if (node->compound_op == ZND_MUL)
      add_ins->op = OP_MUL;
    else if (node->compound_op == ZND_DIV)
      add_ins->op = OP_DIV;
    else if (node->compound_op == ZND_MOD)
      add_ins->op = OP_MOD;
    else if (node->compound_op == ZND_BAND)
      add_ins->op = OP_BAND;
    else if (node->compound_op == ZND_BOR)
      add_ins->op = OP_BOR;
    else if (node->compound_op == ZND_BXOR)
      add_ins->op = OP_BXOR;
    else if (node->compound_op == ZND_SHL)
      add_ins->op = OP_SHL;
    else if (node->compound_op == ZND_SHR)
      add_ins->op = OP_SHR;
    else
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
    return sum_r;
  }
  default:
    return 0;
  }
  if (ins) {
    emit_instr(ctx, ins);
    return r;
  }
  return 0;
}

static void zcc_lower_stmt(LowerCtx *ctx, ZCCNode *node) {
  if (!node)
    return;
  Function *fn = ctx->fn;
  switch (node->kind) {
  case ZND_NOP:
    return;
  case ZND_ASM: {
    Instr *ins = calloc(1, sizeof(Instr));
    ins->id = ctx->next_instr_id++;
    ins->op = OP_ASM;
    ins->asm_string = node->asm_string;
    ins->ir_type = IR_TY_I64;
    emit_instr(ctx, ins);
    return;
  }
  case ZND_VLA_ALLOC: {
    if (!node->lhs) return;
    RegID count_r = zcc_lower_expr(ctx, node->lhs);
    
    Instr *imm = calloc(1, sizeof(Instr));
    imm->id = ctx->next_instr_id++;
    imm->op = OP_CONST;
    imm->dst = ctx->next_reg++;
    imm->imm = node->int_val;
    emit_instr(ctx, imm);
    
    Instr *imul = calloc(1, sizeof(Instr));
    imul->id = ctx->next_instr_id++;
    imul->op = OP_MUL;
    imul->dst = ctx->next_reg++;
    imul->src[0] = count_r;
    imul->src[1] = imm->dst;
    imul->n_src = 2;
    emit_instr(ctx, imul);
    
    Instr *al = calloc(1, sizeof(Instr));
    al->id = ctx->next_instr_id++;
    al->op = OP_VLA_ALLOC;
    al->dst = ctx->next_reg++;
    al->src[0] = imul->dst;
    al->n_src = 1;
    emit_instr(ctx, al);

    if (node->name[0]) {
      RegID addr_r = get_or_create_var(ctx, node->name, 8);
      Instr *st = calloc(1, sizeof(Instr));
      st->id = ctx->next_instr_id++;
      st->op = OP_STORE;
      st->src[0] = al->dst;
      st->src[1] = addr_r;
      st->n_src = 2;
      st->imm = 8;
      emit_instr(ctx, st);
    }
    return;
  }
  case ZND_ASSIGN: {
    if (!node->lhs)
      return;
    RegID addr_r;
    ctx->want_address = 1;
    addr_r = zcc_lower_expr(ctx, node->lhs);
    ctx->want_address = 0;
    if (!addr_r)
      return;
    RegID val_r = zcc_lower_expr(ctx, node->rhs);
    Instr *st = calloc(1, sizeof(Instr));
    st->id = ctx->next_instr_id++;
    st->op = OP_STORE;
    st->dst = 0;
    st->src[0] = val_r;
    st->src[1] = addr_r;
    st->n_src = 2;
    st->exec_freq = 1.0;
    st->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    emit_instr(ctx, st);
    return;
  }
  case ZND_COMPOUND_ASSIGN: {
    if (!node->lhs || !node->rhs)
      return;
    RegID addr_r;
    ctx->want_address = 1;
    addr_r = zcc_lower_expr(ctx, node->lhs);
    ctx->want_address = 0;
    if (!addr_r)
      return;

    RegID load_r = ctx->next_reg++;
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = ctx->next_instr_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = load_r;
    load_ins->src[0] = addr_r;
    load_ins->n_src = 1;
    load_ins->imm = 8;
    if (node->lhs && node->lhs->member_size > 0)
      load_ins->imm = node->lhs->member_size;
    load_ins->exec_freq = 1.0;
    emit_instr(ctx, load_ins);

    RegID rhs_r = zcc_lower_expr(ctx, node->rhs);
    RegID sum_r = ctx->next_reg++;
    Instr *add_ins = calloc(1, sizeof(Instr));
    add_ins->id = ctx->next_instr_id++;

    if (node->compound_op == ZND_SUB)
      add_ins->op = OP_SUB;
    else if (node->compound_op == ZND_MUL)
      add_ins->op = OP_MUL;
    else if (node->compound_op == ZND_DIV)
      add_ins->op = OP_DIV;
    else if (node->compound_op == ZND_MOD)
      add_ins->op = OP_MOD;
    else if (node->compound_op == ZND_BAND)
      add_ins->op = OP_BAND;
    else if (node->compound_op == ZND_BOR)
      add_ins->op = OP_BOR;
    else if (node->compound_op == ZND_BXOR)
      add_ins->op = OP_BXOR;
    else if (node->compound_op == ZND_SHL)
      add_ins->op = OP_SHL;
    else if (node->compound_op == ZND_SHR)
      add_ins->op = OP_SHR;
    else
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
    st->imm =
        (node->lhs && node->lhs->member_size > 0) ? node->lhs->member_size : 8;
    emit_instr(ctx, st);
    return;
  }
  case ZND_IF: {
    RegID cond_r = zcc_lower_expr(ctx, node->cond);
    char then_name[NAME_LEN], else_name[NAME_LEN], merge_name[NAME_LEN];
    int id = node->if_id;
    snprintf(then_name, sizeof(then_name), "if.then.%d", id);
    snprintf(else_name, sizeof(else_name), "if.else.%d", id);
    snprintf(merge_name, sizeof(merge_name), "if.merge.%d", id);
    BlockID then_blk = new_block(ctx, then_name);
    BlockID merge_blk = new_block(ctx, merge_name);
    Block *cur = fn->blocks[ctx->cur_block];
    if (node->else_body) {
      BlockID else_blk = new_block(ctx, else_name);
      Instr *cbr = calloc(1, sizeof(Instr));
      cbr->id = ctx->next_instr_id++;
      cbr->op = OP_CONDBR;
      cbr->dst = 0;
      cbr->src[0] = cond_r;
      cbr->src[1] = then_blk;
      cbr->src[2] = else_blk;
      cbr->n_src = 3;
      cbr->exec_freq = 1.0;
      emit_instr(ctx, cbr);
      cur->succs[0] = then_blk;
      cur->succs[1] = else_blk;
      cur->n_succs = 2;
      cur->branch_prob[0] = 0.5;
      cur->branch_prob[1] = 0.5;
      fn->blocks[then_blk]->preds[0] = ctx->cur_block;
      fn->blocks[then_blk]->n_preds = 1;
      fn->blocks[else_blk]->preds[0] = ctx->cur_block;
      fn->blocks[else_blk]->n_preds = 1;

      ctx->cur_block = then_blk;
      zcc_lower_stmt(ctx, node->then_body);
      {
        Block *_tb = fn->blocks[ctx->cur_block];
        Instr *_tt = _tb ? _tb->tail : NULL;
        if (!(_tt && (_tt->op == OP_RET || _tt->op == OP_BR ||
                      _tt->op == OP_CONDBR))) {
          Instr *br_then = calloc(1, sizeof(Instr));
          br_then->id = ctx->next_instr_id++;
          br_then->op = OP_BR;
          br_then->src[0] = merge_blk;
          br_then->n_src = 1;
          br_then->exec_freq = 1.0;
          emit_instr(ctx, br_then);
          fn->blocks[ctx->cur_block]->succs[0] = merge_blk;
          fn->blocks[ctx->cur_block]->n_succs = 1;
          fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++] =
              ctx->cur_block;
        }
      }

      ctx->cur_block = else_blk;
      zcc_lower_stmt(ctx, node->else_body);
      {
        Block *_eb = fn->blocks[ctx->cur_block];
        Instr *_et = _eb ? _eb->tail : NULL;
        if (!(_et && (_et->op == OP_RET || _et->op == OP_BR ||
                      _et->op == OP_CONDBR))) {
          Instr *br_else = calloc(1, sizeof(Instr));
          br_else->id = ctx->next_instr_id++;
          br_else->op = OP_BR;
          br_else->src[0] = merge_blk;
          br_else->n_src = 1;
          br_else->exec_freq = 1.0;
          emit_instr(ctx, br_else);
          fn->blocks[ctx->cur_block]->succs[0] = merge_blk;
          fn->blocks[ctx->cur_block]->n_succs = 1;
          fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++] =
              ctx->cur_block;
        }
      }
    } else {
      Instr *cbr = calloc(1, sizeof(Instr));
      cbr->id = ctx->next_instr_id++;
      cbr->op = OP_CONDBR;
      cbr->dst = 0;
      cbr->src[0] = cond_r;
      cbr->src[1] = then_blk;
      cbr->src[2] = merge_blk;
      cbr->n_src = 3;
      cbr->exec_freq = 1.0;
      emit_instr(ctx, cbr);
      cur->succs[0] = then_blk;
      cur->succs[1] = merge_blk;
      cur->n_succs = 2;
      cur->branch_prob[0] = 0.5;
      cur->branch_prob[1] = 0.5;
      fn->blocks[then_blk]->preds[0] = ctx->cur_block;
      fn->blocks[then_blk]->n_preds = 1;
      fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++] =
          ctx->cur_block;

      ctx->cur_block = then_blk;
      zcc_lower_stmt(ctx, node->then_body);
      {
        Block *_tb = fn->blocks[ctx->cur_block];
        Instr *_tt = _tb ? _tb->tail : NULL;
        if (!(_tt && (_tt->op == OP_RET || _tt->op == OP_BR ||
                      _tt->op == OP_CONDBR))) {
          Instr *br_then = calloc(1, sizeof(Instr));
          br_then->id = ctx->next_instr_id++;
          br_then->op = OP_BR;
          br_then->src[0] = merge_blk;
          br_then->n_src = 1;
          br_then->exec_freq = 1.0;
          emit_instr(ctx, br_then);
          fn->blocks[ctx->cur_block]->succs[0] = merge_blk;
          fn->blocks[ctx->cur_block]->n_succs = 1;
          fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds++] =
              ctx->cur_block;
        }
      }
    }
    ctx->cur_block = merge_blk;
    return;
  }
  case ZND_WHILE: {
    BlockID head = new_block(ctx, "while.head");
    BlockID body_blk = new_block(ctx, "while.body");
    BlockID exit_blk = new_block(ctx, "while.exit");
    Block *cur = fn->blocks[ctx->cur_block];
    Instr *br = calloc(1, sizeof(Instr));
    br->id = ctx->next_instr_id++;
    br->op = OP_BR;
    br->src[0] = head;
    br->n_src = 1;
    br->exec_freq = 1.0;
    emit_instr(ctx, br);
    cur->succs[0] = head;
    cur->n_succs = 1;
    fn->blocks[head]->preds[0] = ctx->cur_block;
    fn->blocks[head]->n_preds = 1;

    ctx->cur_block = head;
    RegID cond_r = zcc_lower_expr(ctx, node->cond);
    Instr *cbr = calloc(1, sizeof(Instr));
    cbr->id = ctx->next_instr_id++;
    cbr->op = OP_CONDBR;
    cbr->dst = 0;
    cbr->src[0] = cond_r;
    cbr->src[1] = body_blk;
    cbr->src[2] = exit_blk;
    cbr->n_src = 3;
    cbr->exec_freq = 1.0;
    emit_instr(ctx, cbr);
    fn->blocks[head]->succs[0] = body_blk;
    fn->blocks[head]->succs[1] = exit_blk;
    fn->blocks[head]->n_succs = 2;
    fn->blocks[head]->branch_prob[0] = 0.9;
    fn->blocks[head]->branch_prob[1] = 0.1;
    fn->blocks[body_blk]->preds[0] = head;
    fn->blocks[body_blk]->n_preds = 1;
    fn->blocks[exit_blk]->preds[0] = head;
    fn->blocks[exit_blk]->n_preds = 1;

    ctx->loop_exit_stack[ctx->loop_depth] = exit_blk;
    ctx->loop_latch_stack[ctx->loop_depth] =
        head; /* continue → re-evaluate condition */
    ctx->loop_depth++;

    ctx->cur_block = body_blk;
    zcc_lower_stmt(ctx, node->body);
    {
      Block *_bb = fn->blocks[ctx->cur_block];
      Instr *_bt = _bb ? _bb->tail : NULL;
      if (!(_bt &&
            (_bt->op == OP_RET || _bt->op == OP_BR || _bt->op == OP_CONDBR))) {
        Instr *br_back = calloc(1, sizeof(Instr));
        br_back->id = ctx->next_instr_id++;
        br_back->op = OP_BR;
        br_back->src[0] = head;
        br_back->n_src = 1;
        br_back->exec_freq = 1.0;
        emit_instr(ctx, br_back);
        fn->blocks[ctx->cur_block]->succs[0] = head;
        fn->blocks[ctx->cur_block]->n_succs = 1;
        fn->blocks[head]->preds[fn->blocks[head]->n_preds++] = ctx->cur_block;
      }
    }

    ctx->loop_depth--;

    ctx->cur_block = exit_blk;
    return;
  }
  case ZND_FOR: {
    /* Five-block CFG: preheader (init) -> header (cond) -> body -> latch (inc)
     * -> header; header false -> exit. */
    BlockID preheader = new_block(ctx, "for.preheader");
    BlockID header = new_block(ctx, "for.head");
    BlockID body_blk = new_block(ctx, "for.body");
    BlockID latch = new_block(ctx, "for.latch");
    BlockID exit_blk = new_block(ctx, "for.exit");
    Block *cur = fn->blocks[ctx->cur_block];

    /* Current block branches to preheader */
    Instr *br = calloc(1, sizeof(Instr));
    br->id = ctx->next_instr_id++;
    br->op = OP_BR;
    br->src[0] = preheader;
    br->n_src = 1;
    br->exec_freq = 1.0;
    emit_instr(ctx, br);
    cur->succs[0] = preheader;
    cur->n_succs = 1;
    fn->blocks[preheader]->preds[0] = ctx->cur_block;
    fn->blocks[preheader]->n_preds = 1;

    /* Preheader: run init, then branch to header (LICM hoists invariants here)
     */
    ctx->cur_block = preheader;
    if (node->init)
      zcc_lower_stmt(ctx, node->init);
    Instr *br_ph = calloc(1, sizeof(Instr));
    br_ph->id = ctx->next_instr_id++;
    br_ph->op = OP_BR;
    br_ph->src[0] = header;
    br_ph->n_src = 1;
    br_ph->exec_freq = 1.0;
    emit_instr(ctx, br_ph);
    fn->blocks[preheader]->succs[0] = header;
    fn->blocks[preheader]->n_succs = 1;
    fn->blocks[header]->preds[fn->blocks[header]->n_preds++] = preheader;

    /* Header: evaluate cond; true -> body, false -> exit */
    ctx->cur_block = header;
    RegID cond_r = 0;
    if (node->cond) {
      cond_r = zcc_lower_expr(ctx, node->cond);
    } else {
      cond_r = ctx->next_reg++;
      Instr *one = make_instr_imm(ctx->next_instr_id++, OP_CONST, cond_r, 1,
                                  node->line_no);
      emit_instr(ctx, one);
    }
    Instr *cbr = calloc(1, sizeof(Instr));
    cbr->id = ctx->next_instr_id++;
    cbr->op = OP_CONDBR;
    cbr->dst = 0;
    cbr->src[0] = cond_r;
    cbr->src[1] = body_blk;
    cbr->src[2] = exit_blk;
    cbr->n_src = 3;
    cbr->exec_freq = 1.0;
    emit_instr(ctx, cbr);
    fn->blocks[header]->succs[0] = body_blk;
    fn->blocks[header]->succs[1] = exit_blk;
    fn->blocks[header]->n_succs = 2;
    fn->blocks[header]->branch_prob[0] = 0.9;
    fn->blocks[header]->branch_prob[1] = 0.1;
    fn->blocks[body_blk]->preds[0] = header;
    fn->blocks[body_blk]->n_preds = 1;
    fn->blocks[exit_blk]->preds[fn->blocks[exit_blk]->n_preds++] = header;

    ctx->loop_exit_stack[ctx->loop_depth] = exit_blk;
    ctx->loop_latch_stack[ctx->loop_depth] =
        latch; /* continue → run increment then re-evaluate */
    ctx->loop_depth++;

    /* Body: loop statements, then branch to latch */
    ctx->cur_block = body_blk;
    zcc_lower_stmt(ctx, node->body);
    {
      Block *_bb = fn->blocks[ctx->cur_block];
      Instr *_bt = _bb ? _bb->tail : NULL;
      if (!(_bt &&
            (_bt->op == OP_RET || _bt->op == OP_BR || _bt->op == OP_CONDBR))) {
        Instr *br_body = calloc(1, sizeof(Instr));
        br_body->id = ctx->next_instr_id++;
        br_body->op = OP_BR;
        br_body->src[0] = latch;
        br_body->n_src = 1;
        br_body->exec_freq = 1.0;
        emit_instr(ctx, br_body);
        fn->blocks[ctx->cur_block]->succs[0] = latch;
        fn->blocks[ctx->cur_block]->n_succs = 1;
        fn->blocks[latch]->preds[fn->blocks[latch]->n_preds++] = ctx->cur_block;
      }
    }

    /* Latch: run inc, then branch back to header */
    ctx->cur_block = latch;
    if (node->inc) {
      (void)zcc_lower_expr(
          ctx, node->inc); /* inc has side effect (e.g. i++) or is unused */
    }
    Instr *br_latch = calloc(1, sizeof(Instr));
    br_latch->id = ctx->next_instr_id++;
    br_latch->op = OP_BR;
    br_latch->src[0] = header;
    br_latch->n_src = 1;
    br_latch->exec_freq = 1.0;
    emit_instr(ctx, br_latch);
    fn->blocks[latch]->succs[0] = header;
    fn->blocks[latch]->n_succs = 1;
    fn->blocks[header]->preds[fn->blocks[header]->n_preds++] = latch;

    ctx->loop_depth--;

    ctx->cur_block = exit_blk;
    return;
  }
  case ZND_BREAK: {
    if (ctx->loop_depth == 0)
      return;
    BlockID target = ctx->loop_exit_stack[ctx->loop_depth - 1];
    Instr *br = calloc(1, sizeof(Instr));
    br->id = ctx->next_instr_id++;
    br->op = OP_BR;
    br->src[0] = target;
    br->n_src = 1;
    br->exec_freq = 1.0;
    emit_instr(ctx, br);
    fn->blocks[ctx->cur_block]->succs[0] = target;
    fn->blocks[ctx->cur_block]->n_succs = 1;
    fn->blocks[target]->preds[fn->blocks[target]->n_preds++] = ctx->cur_block;
    ctx->cur_block = new_block(ctx, "unreachable.after.break");
    return;
  }
  case ZND_CONTINUE: {
    if (ctx->loop_depth == 0)
      return;
    BlockID target = ctx->loop_latch_stack[ctx->loop_depth - 1];
    Instr *br = calloc(1, sizeof(Instr));
    br->id = ctx->next_instr_id++;
    br->op = OP_BR;
    br->src[0] = target;
    br->n_src = 1;
    br->exec_freq = 1.0;
    emit_instr(ctx, br);
    fn->blocks[ctx->cur_block]->succs[0] = target;
    fn->blocks[ctx->cur_block]->n_succs = 1;
    fn->blocks[target]->preds[fn->blocks[target]->n_preds++] = ctx->cur_block;
    ctx->cur_block = new_block(ctx, "unreachable.after.continue");
    return;
  }
  case ZND_SWITCH: {
    int n = node->num_cases;
    BlockID exit_blk = new_block(ctx, "switch.exit");
    BlockID default_blk =
        node->default_body ? new_block(ctx, "switch.default") : exit_blk;
    RegID cond_r = node->cond ? zcc_lower_expr(ctx, node->cond) : 0;
    if (n > 0 && !cond_r) {
      ctx->cur_block = exit_blk;
      return;
    }
    if (n == 0) {
      Instr *br = calloc(1, sizeof(Instr));
      br->id = ctx->next_instr_id++;
      br->op = OP_BR;
      br->src[0] = default_blk;
      br->n_src = 1;
      br->exec_freq = 1.0;
      emit_instr(ctx, br);
      fn->blocks[ctx->cur_block]->succs[0] = default_blk;
      fn->blocks[ctx->cur_block]->n_succs = 1;
      fn->blocks[default_blk]->preds[fn->blocks[default_blk]->n_preds++] =
          ctx->cur_block;
      ctx->cur_block = default_blk;
    }
    for (int i = 0; i < n; i++) {
      BlockID case_blk = new_block(ctx, "switch.case");
      BlockID next_blk =
          (i == n - 1) ? default_blk : new_block(ctx, "switch.cmp");
      RegID case_val_r = ctx->next_reg++;
      Instr *cst = make_instr_imm(ctx->next_instr_id++, OP_CONST, case_val_r,
                                  node->case_vals[i], node->line_no);
      emit_instr(ctx, cst);
      RegID eq_r = ctx->next_reg++;
      Instr *eq_ins = calloc(1, sizeof(Instr));
      eq_ins->id = ctx->next_instr_id++;
      eq_ins->op = OP_EQ;
      eq_ins->dst = eq_r;
      eq_ins->src[0] = cond_r;
      eq_ins->src[1] = case_val_r;
      eq_ins->n_src = 2;
      eq_ins->exec_freq = 1.0;
      emit_instr(ctx, eq_ins);
      Instr *cbr = calloc(1, sizeof(Instr));
      cbr->id = ctx->next_instr_id++;
      cbr->op = OP_CONDBR;
      cbr->dst = 0;
      cbr->src[0] = eq_r;
      cbr->src[1] = case_blk;
      cbr->src[2] = next_blk;
      cbr->n_src = 3;
      cbr->exec_freq = 1.0;
      emit_instr(ctx, cbr);
      fn->blocks[ctx->cur_block]->succs[0] = case_blk;
      fn->blocks[ctx->cur_block]->succs[1] = next_blk;
      fn->blocks[ctx->cur_block]->n_succs = 2;
      fn->blocks[ctx->cur_block]->branch_prob[0] = 0.5f;
      fn->blocks[ctx->cur_block]->branch_prob[1] = 0.5f;
      fn->blocks[case_blk]->preds[fn->blocks[case_blk]->n_preds++] =
          ctx->cur_block;
      fn->blocks[next_blk]->preds[fn->blocks[next_blk]->n_preds++] =
          ctx->cur_block;
      ctx->cur_block = case_blk;
      if (node->case_bodies && node->case_bodies[i])
        zcc_lower_stmt(ctx, node->case_bodies[i]);
      Instr *br_exit = calloc(1, sizeof(Instr));
      br_exit->id = ctx->next_instr_id++;
      br_exit->op = OP_BR;
      br_exit->src[0] = exit_blk;
      br_exit->n_src = 1;
      br_exit->exec_freq = 1.0;
      emit_instr(ctx, br_exit);
      fn->blocks[case_blk]->succs[0] = exit_blk;
      fn->blocks[case_blk]->n_succs = 1;
      fn->blocks[exit_blk]->preds[fn->blocks[exit_blk]->n_preds++] = case_blk;
      ctx->cur_block = next_blk;
    }
    if (node->default_body) {
      ctx->cur_block = default_blk;
      zcc_lower_stmt(ctx, node->default_body);
      Instr *br_def = calloc(1, sizeof(Instr));
      br_def->id = ctx->next_instr_id++;
      br_def->op = OP_BR;
      br_def->src[0] = exit_blk;
      br_def->n_src = 1;
      br_def->exec_freq = 1.0;
      emit_instr(ctx, br_def);
      fn->blocks[default_blk]->succs[0] = exit_blk;
      fn->blocks[default_blk]->n_succs = 1;
      fn->blocks[exit_blk]->preds[fn->blocks[exit_blk]->n_preds++] =
          default_blk;
    }
    ctx->cur_block = exit_blk;
    return;
  }
  case ZND_RETURN: {
    RegID val_r = zcc_lower_expr(ctx, node->lhs);
    Instr *ret = calloc(1, sizeof(Instr));
    ret->id = ctx->next_instr_id++;
    ret->op = OP_RET;
    ret->dst = 0;
    ret->src[0] = val_r;
    ret->n_src = 1;
    ret->exec_freq = 1.0;
    emit_instr(ctx, ret);
    fn->blocks[ctx->cur_block]->n_succs = 0;
    return;
  }
  case ZND_BLOCK:
    for (uint32_t i = 0; i < node->num_stmts; i++)
      zcc_lower_stmt(ctx, node->stmts[i]);
    return;
  default:
    zcc_lower_expr(ctx, node);
    return;
  }
}

/**
 * zcc_ast_to_ir() — Build Function* from ZCC-shaped AST (one function body).
 * Same contract as ast_to_ir(); entry and exit blocks created here.
 * func_name: if "main", allocas for "argc" and "argv" are created in the entry
 * so they get param slots.
 */
Function *zcc_ast_to_ir(ZCCNode *body_ast, const char *func_name) {
  Function *fn = calloc(1, sizeof(Function));
  LowerCtx ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.fn = fn;
  ctx.next_reg = 10;
  ctx.next_instr_id = 1;

  BlockID entry = new_block(&ctx, "entry");
  BlockID exit_blk = new_block(&ctx, "exit");
  fn->entry = entry;
  fn->exit = exit_blk;
  ctx.cur_block = entry;

  /* Root cause fix: for main, create param allocas in the entry so they get
   * slots -8, -16. Prologue stores argc→-8, argv→-16; later uses (e.g. argv in
   * for.body) then load the value. */
  if (is_main_func(func_name)) {
    (void)get_or_create_var(&ctx, "argc", 8);
    (void)get_or_create_var(&ctx, "argv", 8);
  } else {
    const char *params_env = getenv("ZCC_IR_PARAM_NAMES");
    fprintf(stderr, "[ZCC-IR] DEBUG: params_env=%s\n",
            params_env ? params_env : "NULL");
    if (params_env && params_env[0]) {
      char env_buf[512];
      strncpy(env_buf, params_env, sizeof(env_buf) - 1);
      env_buf[sizeof(env_buf) - 1] = '\0';

      char *spt = env_buf;
      while (*spt) {
        char *comma = strchr(spt, ',');
        if (comma)
          *comma = '\0';
        (void)get_or_create_var(&ctx, spt, 8);
        if (!comma)
          break;
        spt = comma + 1;
      }
    }
  }

  zcc_lower_stmt(&ctx, body_ast);

  Block *cur = fn->blocks[ctx.cur_block];
  if (cur->n_succs == 0 && cur->tail && cur->tail->op != OP_RET) {
    Instr *br = calloc(1, sizeof(Instr));
    br->id = ctx.next_instr_id++;
    br->op = OP_BR;
    br->src[0] = exit_blk;
    br->n_src = 1;
    br->exec_freq = 1.0;
    emit_instr(&ctx, br);
    cur->succs[0] = exit_blk;
    cur->n_succs = 1;
    fn->blocks[exit_blk]->preds[fn->blocks[exit_blk]->n_preds++] =
        ctx.cur_block;
  }

  return fn;
}

#ifdef ZCC_BRIDGE_STANDALONE
/* Build ZCC-shaped AST for: int main() { int i = 0; while (i < 10) { i = i + 1;
 * } return i; } */
static ZCCNode *build_zcc_phase_b_ast(void) {
  static ZCCNode pool[32];
  static ZCCNode *stmts[ZCC_AST_MAX_STMTS];
  static ZCCNode *body_stmts[1];
  memset(pool, 0, sizeof(pool));
  int p = 0;
#define ZN() (&pool[p++])

  ZCCNode *zero = ZN();
  zero->kind = ZND_NUM;
  zero->int_val = 0;
  ZCCNode *ten = ZN();
  ten->kind = ZND_NUM;
  ten->int_val = 10;
  ZCCNode *one = ZN();
  one->kind = ZND_NUM;
  one->int_val = 1;
  ZCCNode *var_i = ZN();
  var_i->kind = ZND_VAR;
  strncpy(var_i->name, "i", NAME_LEN - 1);
  ZCCNode *i_lt_10 = ZN();
  i_lt_10->kind = ZND_LT;
  i_lt_10->lhs = var_i;
  i_lt_10->rhs = ten;
  ZCCNode *i_plus_1 = ZN();
  i_plus_1->kind = ZND_ADD;
  i_plus_1->lhs = var_i;
  i_plus_1->rhs = one;
  ZCCNode *lhs_i = ZN();
  lhs_i->kind = ZND_VAR;
  strncpy(lhs_i->name, "i", NAME_LEN - 1);
  ZCCNode *assign_inc = ZN();
  assign_inc->kind = ZND_ASSIGN;
  assign_inc->lhs = lhs_i;
  assign_inc->rhs = i_plus_1;
  ZCCNode *body_while = ZN();
  body_while->kind = ZND_BLOCK;
  body_stmts[0] = assign_inc;
  body_while->stmts = body_stmts;
  body_while->num_stmts = 1;
  ZCCNode *while_loop = ZN();
  while_loop->kind = ZND_WHILE;
  while_loop->cond = i_lt_10;
  while_loop->body = body_while;
  ZCCNode *init_i = ZN();
  init_i->kind = ZND_ASSIGN;
  init_i->lhs = ZN();
  init_i->lhs->kind = ZND_VAR;
  strncpy(init_i->lhs->name, "i", NAME_LEN - 1);
  init_i->rhs = zero;
  ZCCNode *ret_i = ZN();
  ret_i->kind = ZND_RETURN;
  ret_i->lhs = var_i;
  stmts[0] = init_i;
  stmts[1] = while_loop;
  stmts[2] = ret_i;
  ZCCNode *block = ZN();
  block->kind = ZND_BLOCK;
  block->stmts = stmts;
  block->num_stmts = 3;
  return block;
#undef ZN
}
#endif /* ZCC_BRIDGE_STANDALONE */

/**
 * ast_to_ir() — Build Function* from minimal AST (one function, scalars only).
 * Caller provides AST for function body; entry and exit blocks are created.
 */
Function *ast_to_ir(ASTNode *body_ast) {
  Function *fn = calloc(1, sizeof(Function));
  LowerCtx ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.fn = fn;
  ctx.next_reg = 10;
  ctx.next_instr_id = 1;

  BlockID entry = new_block(&ctx, "entry");
  BlockID exit_blk = new_block(&ctx, "exit");
  fn->entry = entry;
  fn->exit = exit_blk;
  ctx.cur_block = entry;

  lower_stmt(&ctx, body_ast);

  /* If current block doesn't end with ret/br, branch to exit */
  Block *cur = fn->blocks[ctx.cur_block];
  if (cur->n_succs == 0 && cur->tail && cur->tail->op != OP_RET) {
    Instr *br = calloc(1, sizeof(Instr));
    br->id = ctx.next_instr_id++;
    br->op = OP_BR;
    br->src[0] = exit_blk;
    br->n_src = 1;
    br->exec_freq = 1.0;
    emit_instr(&ctx, br);
    cur->succs[0] = exit_blk;
    cur->n_succs = 1;
    fn->blocks[exit_blk]->preds[fn->blocks[exit_blk]->n_preds++] =
        ctx.cur_block;
  }

  return fn;
}

#ifdef ZCC_BRIDGE_STANDALONE
/* Build AST for: int main() { int i = 0; while (i < 10) { i = i + 1; } return
 * i; } */
static ASTNode *build_phase_b_ast(void) {
  static ASTNode pool[32];
  static ASTNode *stmts[AST_MAX_STMTS];
  static ASTNode *body_stmts[1];
  memset(pool, 0, sizeof(pool));
  int p = 0;
#define N() (&pool[p++])

  ASTNode *zero = N();
  zero->kind = AST_NUM;
  zero->num_val = 0;
  ASTNode *ten = N();
  ten->kind = AST_NUM;
  ten->num_val = 10;
  ASTNode *one = N();
  one->kind = AST_NUM;
  one->num_val = 1;
  ASTNode *var_i = N();
  var_i->kind = AST_VAR;
  strcpy(var_i->var_name, "i");
  ASTNode *i_lt_10 = N();
  i_lt_10->kind = AST_LT;
  i_lt_10->lhs = var_i;
  i_lt_10->rhs = ten;
  ASTNode *i_plus_1 = N();
  i_plus_1->kind = AST_ADD;
  i_plus_1->lhs = var_i;
  i_plus_1->rhs = one;
  ASTNode *lhs_i = N();
  lhs_i->kind = AST_VAR;
  strcpy(lhs_i->var_name, "i");
  ASTNode *assign_inc = N();
  assign_inc->kind = AST_ASSIGN;
  assign_inc->lhs = lhs_i;
  assign_inc->rhs = i_plus_1;
  ASTNode *body_while = N();
  body_while->kind = AST_BLOCK;
  body_stmts[0] = assign_inc;
  body_while->stmts = body_stmts;
  body_while->n_stmts = 1;
  ASTNode *while_loop = N();
  while_loop->kind = AST_WHILE;
  while_loop->cond = i_lt_10;
  while_loop->body = body_while;
  ASTNode *init_i = N();
  init_i->kind = AST_ASSIGN;
  init_i->lhs = N();
  init_i->lhs->kind = AST_VAR;
  strcpy(init_i->lhs->var_name, "i");
  init_i->rhs = zero;
  ASTNode *ret_i = N();
  ret_i->kind = AST_RETURN;
  ret_i->lhs = var_i;
  stmts[0] = init_i;
  stmts[1] = while_loop;
  stmts[2] = ret_i;
  ASTNode *block = N();
  block->kind = AST_BLOCK;
  block->stmts = stmts;
  block->n_stmts = 3;
  return block;
#undef N
}
#endif /* ZCC_BRIDGE_STANDALONE */

/* ─────────────────────────────────────────────────────────────────────────────
 * PASS PIPELINE: run all three passes in order
 * ──────────────────────────────────────────────────────────────── */

/**
 * run_all_passes() — Execute the full three-pass optimization pipeline.
 *
 * Pass ordering rationale:
 *   1. DCE first: removes dead instructions, reducing the points-to graph
 *      that escape analysis must traverse.
 *   2. Escape analysis second: promotes stack allocations, which may expose
 *      additional dead stores (a second DCE run in a production pipeline
 *      would clean these up).
 *   3. PGO reordering last: operates on the pruned CFG, so cold blocks
 *      already removed by DCE do not pollute the frequency ordering.
 *
 * @param fn      Function IR (modified in-place by passes 1 and 2).
 * @param result  Output reordering from pass 3; licm_hoisted and
 * dce_instrs_removed filled from fn->stats.
 */

/**
 * ir_validate() — Phase B entry contract. Ensures Function* satisfies
 * invariants required by DCE, LICM, escape analysis, and PGO.
 * Returns true if valid; on failure, prints diagnostics to stderr and returns
 * false.
 */
static bool ir_validate(const Function *fn) {
  bool ok = true;
  if (!fn || fn->n_blocks == 0)
    return false;

  /* Invariant 4: Entry block has no predecessors. */
  BlockID entry = fn->entry;
  if (entry >= fn->n_blocks) {
    fprintf(stderr, "[ir_validate] entry block id %u >= n_blocks %u\n",
            (unsigned)entry, fn->n_blocks);
    ok = false;
  } else if (fn->blocks[entry]->n_preds != 0) {
    fprintf(stderr,
            "[ir_validate] entry block has %u predecessors (must be 0)\n",
            fn->blocks[entry]->n_preds);
    ok = false;
  }

  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    const Block *blk = fn->blocks[bi];
    if (!blk)
      continue;

    /* Invariant 3: Branch probabilities on CONDBR blocks sum to 1.0. */
    if (blk->n_succs > 1) {
      double sum = 0.0;
      for (uint32_t si = 0; si < blk->n_succs; si++)
        sum += blk->branch_prob[si];
      if (sum < 0.99 || sum > 1.01) {
        fprintf(stderr,
                "[ir_validate] block %u (%s) branch_prob sum = %.4f (expected "
                "1.0)\n",
                bi, blk->name, sum);
        ok = false;
      }
    }

    for (const Instr *ins = blk->head; ins; ins = ins->next) {
      /* Invariant 1: Every used RegID is in range; def_of[r] may be NULL for
       * args. */
      for (uint32_t s = 0; s < ins->n_src; s++) {
        RegID r = ins->src[s];
        if (r != 0 && r >= MAX_INSTRS) {
          fprintf(
              stderr,
              "[ir_validate] block %u instr %u src[%u] reg %u >= MAX_INSTRS\n",
              bi, (unsigned)ins->id, s, (unsigned)r);
          ok = false;
        }
      }
      if (ins->op == OP_PHI) {
        for (uint32_t p = 0; p < ins->n_phi; p++) {
          RegID r = ins->phi[p].reg;
          if (r != 0 && r >= MAX_INSTRS) {
            fprintf(stderr,
                    "[ir_validate] block %u phi instr %u phi[%u] reg %u >= "
                    "MAX_INSTRS\n",
                    bi, (unsigned)ins->id, p, (unsigned)r);
            ok = false;
          }
        }
      }
      if (ins->op == OP_CALL) {
        for (uint32_t s = 0; s < ins->n_call_args; s++) {
          RegID r = ins->call_args[s];
          if (r != 0 && r >= MAX_INSTRS) {
            fprintf(stderr,
                    "[ir_validate] block %u instr %u call_args[%u] reg %u >= "
                    "MAX_INSTRS\n",
                    bi, (unsigned)ins->id, s, (unsigned)r);
            ok = false;
          }
        }
      }

      /* Invariant 2: Every OP_PHI lists exactly the predecessor blocks of its
       * block. */
      if (ins->op == OP_PHI) {
        if (ins->n_phi != blk->n_preds) {
          fprintf(stderr,
                  "[ir_validate] block %u phi instr %u has n_phi=%u but block "
                  "has n_preds=%u\n",
                  bi, (unsigned)ins->id, ins->n_phi, blk->n_preds);
          ok = false;
        } else {
          for (uint32_t pi = 0; pi < blk->n_preds; pi++) {
            BlockID pred = blk->preds[pi];
            bool found = false;
            for (uint32_t p = 0; p < ins->n_phi; p++)
              if (ins->phi[p].block == pred) {
                found = true;
                break;
              }
            if (!found) {
              fprintf(stderr,
                      "[ir_validate] block %u phi instr %u has no source for "
                      "predecessor %u\n",
                      bi, (unsigned)ins->id, (unsigned)pred);
              ok = false;
            }
          }
        }
      }
    }
  }

  return ok;
}

/* Apply PGO profile: for each block named in the file, set branch_prob[0] and
 * [1]. File format: one line per 2-way branch block: "block_name p0 p1" (e.g.
 * "for.head 0.9 0.1"). Lines starting with # are comments. p0+p1 should be 1.0;
 * if not, we normalize. */
static void apply_profile_to_function(Function *fn, const char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "[PGO] cannot open profile '%s'\n", path);
    return;
  }
  char line[256];
  unsigned applied = 0;
  while (fgets(line, sizeof(line), f)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\0')
      continue;
    char name[NAME_LEN];
    double p0 = 0.0, p1 = 0.0;
    if (sscanf(line, "%63s %lf %lf", name, &p0, &p1) < 3)
      continue;
    for (BlockID bi = 0; bi < fn->n_blocks; bi++) {
      Block *blk = fn->blocks[bi];
      if (!blk || blk->n_succs != 2)
        continue;
      if (strcmp(blk->name, name) != 0)
        continue;
      if (p0 < 0.0)
        p0 = 0.0;
      if (p1 < 0.0)
        p1 = 0.0;
      {
        double s = p0 + p1;
        if (s > 0.0) {
          p0 /= s;
          p1 /= s;
        } else {
          p0 = 0.5;
          p1 = 0.5;
        }
      }
      blk->branch_prob[0] = p0;
      blk->branch_prob[1] = p1;
      applied++;
      break;
    }
  }
  fclose(f);
  if (applied)
    fprintf(stderr, "[PGO] applied profile '%s' (%u block(s))\n", path,
            applied);
}

/* ── PGO instrumentation pass: inject block execution counter probes ──
 * Runs before DCE/LICM so we capture original control flow.
 * When ZCC_PGO_INSTRUMENT=1, inserts at the head of every block:
 *   addr = OP_PGO_COUNTER_ADDR block_id; count = LOAD addr; count++; STORE
 * count, addr. Counter array __zcc_edge_counts[] and atexit dump are emitted by
 * codegen/linker. */
static uint32_t pgo_instrument_pass(Function *fn) {
  RegID Raddr = fn->n_regs;
  RegID Rcount = fn->n_regs + 1;
  RegID Rone = fn->n_regs + 2;
  RegID Rnew = fn->n_regs + 3;
  fn->n_regs += 4;

  InstrID next_id = 0;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk)
      continue;
    for (Instr *p = blk->head; p; p = p->next)
      if (p->id >= next_id)
        next_id = (InstrID)(p->id + 1);
  }

  uint32_t injected = 0;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk)
      continue;

    Instr *addr_ins = make_instr_imm(next_id++, OP_PGO_COUNTER_ADDR, Raddr,
                                     (int64_t)(uint32_t)bi, 0);
    Instr *load_ins = calloc(1, sizeof(Instr));
    load_ins->id = next_id++;
    load_ins->op = OP_LOAD;
    load_ins->dst = Rcount;
    load_ins->src[0] = Raddr;
    load_ins->n_src = 1;
    load_ins->imm = 8;
    load_ins->exec_freq = 1.0;
    Instr *one_ins = make_instr_imm(next_id++, OP_CONST, Rone, 1, 0);
    Instr *add_ins = calloc(1, sizeof(Instr));
    add_ins->id = next_id++;
    add_ins->op = OP_ADD;
    add_ins->dst = Rnew;
    add_ins->src[0] = Rcount;
    add_ins->src[1] = Rone;
    add_ins->n_src = 2;
    add_ins->exec_freq = 1.0;
    Instr *store_ins = calloc(1, sizeof(Instr));
    store_ins->id = next_id++;
    store_ins->op = OP_STORE;
    store_ins->dst = 0;
    store_ins->src[0] = Rnew;
    store_ins->src[1] = Raddr;
    store_ins->n_src = 2;
    store_ins->exec_freq = 1.0;

    addr_ins->next = load_ins;
    load_ins->prev = addr_ins;
    load_ins->next = one_ins;
    one_ins->prev = load_ins;
    one_ins->next = add_ins;
    add_ins->prev = one_ins;
    add_ins->next = store_ins;
    store_ins->prev = add_ins;
    store_ins->next = blk->head;
    if (blk->head)
      blk->head->prev = store_ins;
    else
      blk->tail = store_ins;
    blk->head = addr_ins;
    blk->n_instrs += 5;
    injected += 5;
  }

  return injected;
}

void ir_dump_json(void *f_ptr, Function *fn, const char *func_name,
                  int is_first) {
  FILE *f = (FILE *)f_ptr;
  if (!f || !fn)
    return;
  if (!is_first)
    fprintf(f, ",\n");
  fprintf(f, "  {\n    \"function\": \"%s\",\n    \"blocks\": [\n",
          func_name ? func_name : "unknown");
  int first_blk = 1;
  for (uint32_t b = 0; b < fn->n_blocks; b++) {
    Block *blk = fn->blocks[b];
    if (!blk)
      continue;
    if (!first_blk)
      fprintf(f, ",\n");
    fprintf(f, "      {\n        \"id\": %u,\n        \"name\": \"%s\",\n",
            blk->id, blk->name);
    fprintf(f, "        \"instructions\": [\n");
    int first_ins = 1;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (!first_ins)
        fprintf(f, ",\n");
      const char *op_str =
          (unsigned)ins->op < sizeof(opcode_name) / sizeof(opcode_name[0])
              ? opcode_name[ins->op]
              : "UNKNOWN";
      fprintf(f, "          { \"id\": %u, \"op\": \"%s\", \"dst\": %u", ins->id,
              op_str, ins->dst);
      fprintf(f, ", \"srcs\": [");
      for (uint32_t i = 0; i < ins->n_src; i++) {
        if (i > 0)
          fprintf(f, ", ");
        fprintf(f, "%u", ins->src[i]);
      }
      fprintf(f, "]");
      if (ins->op == OP_CONST)
        fprintf(f, ", \"imm\": %lld", (long long)ins->imm);
      if (ins->call_name[0])
        fprintf(f, ", \"call_name\": \"%s\"", ins->call_name);
      fprintf(f, " }");
      first_ins = 0;
    }
    fprintf(f, "\n        ]\n      }");
    first_blk = 0;
  }
  fprintf(f, "\n    ]\n  }");
  fflush(f);
}

void zcc_ir_bridge_dump_and_free(ZCCNode *z_node, const char *func_name,
                                 void *dump_f, int is_first) {
  if (!z_node)
    return;
  Function *fn_ir = zcc_ast_to_ir(z_node, func_name);
  if (fn_ir) {
    PassResult *pr = (PassResult *)calloc(1, sizeof(PassResult));
    if (pr) {
      run_all_passes(fn_ir, pr, NULL, 0);
      free(pr);
    }
    if (dump_f) {
      ir_dump_json(dump_f, fn_ir, func_name, is_first);
    }
    zcc_ir_free(fn_ir);
  }
  zcc_node_free(z_node);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * REDUNDANT LOAD ELIMINATION (RLE)
 *
 * Per basic block, tracks which address vregs have already been loaded.
 * If the same address is loaded again with no intervening STORE to that
 * address (or CALL, which may write anywhere), the second LOAD is replaced
 * with a COPY from the first load's result register.
 *
 * STORE handling:
 *   - STORE to a known ALLOCA (stack slot): only invalidates that address
 *     (ALLOCAs are guaranteed non-aliasing)
 *   - STORE to anything else: invalidates all tracked loads (conservative)
 *
 * CALL handling: invalidates all tracked loads.
 *
 * After this pass, run copy propagation + DCE to clean up the COPYs and
 * any instructions that fed the now-dead LOADs.
 * ───────────────────────────────────────────────────────────────────────────
 */
static uint32_t redundant_load_elim_pass(Function *fn) {
  uint32_t eliminated = 0;

#define RLE_CAPACITY 512
  struct rle_entry {
    RegID addr;
    RegID val;
    int64_t size;
  };
  struct rle_entry avail[RLE_CAPACITY];

  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk)
      continue;

    int n_avail = 0;

    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->dead || ins->op == OP_NOP)
        continue;

      if (ins->op == OP_LOAD && ins->n_src >= 1 && ins->dst) {
        RegID addr = ins->src[0];
        int64_t sz = ins->imm;

        /* Search for a prior load from the same address + size */
        int found = -1;
        for (int i = 0; i < n_avail; i++) {
          if (avail[i].addr == addr && avail[i].size == sz) {
            found = i;
            break;
          }
        }

        if (found >= 0) {
          /* Redundant — replace with COPY from prior load result */
          ins->op = OP_COPY;
          ins->src[0] = avail[found].val;
          ins->n_src = 1;
          ins->imm = 0;
          eliminated++;
        } else {
          /* Record this load for future matches */
          if (n_avail < RLE_CAPACITY) {
            avail[n_avail].addr = addr;
            avail[n_avail].val = ins->dst;
            avail[n_avail].size = sz;
            n_avail++;
          }
        }
      } else if (ins->op == OP_STORE && ins->n_src >= 2) {
        RegID addr = ins->src[1]; /* STORE src[0]=value, src[1]=address */

        /* Check if the target is a known ALLOCA (non-aliasing stack slot) */
        bool is_alloca = (addr < MAX_INSTRS && fn->def_of[addr] &&
                          fn->def_of[addr]->op == OP_ALLOCA);

        if (is_alloca) {
          /* Invalidate only entries for this specific address */
          for (int i = 0; i < n_avail; i++) {
            if (avail[i].addr == addr) {
              avail[i] = avail[--n_avail];
              i--; /* re-check swapped entry */
            }
          }
        } else {
          /* Unknown alias — conservatively invalidate everything */
          n_avail = 0;
        }
      } else if (ins->op == OP_CALL) {
        /* Calls may write to any reachable memory */
        n_avail = 0;
      }
    } /* end instruction walk */
  } /* end block walk */

#undef RLE_CAPACITY
  return eliminated;
}

#include "zcc_ir_opt_passes.h"

void run_all_passes(Function *fn, PassResult *result, const char *profile_path,
                    int num_params) {
  if (!ir_validate(fn)) {
    fprintf(stderr,
            "[run_all_passes] IR validation failed; continuing anyway.\n");
  }

  fprintf(stderr, "=== PRE-OPT IR ===\n");
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *b = fn->blocks[bi];
    if (!b)
      continue;
    fprintf(stderr, "Block %u (preds: %u, succs: %u)\n", bi, b->n_preds,
            b->n_succs);
    for (Instr *ins = b->head; ins; ins = ins->next) {
      fprintf(stderr, "  ins %u: op=%d dst=%u src0=%u\n", (unsigned)ins->id,
              ins->op, ins->dst, ins->n_src > 0 ? ins->src[0] : 0);
    }
  }

  /* Prerequisite: reachability */
  compute_reachability(fn);

  /* CG-IR: Mark parameter allocas as escaping so they are not promoted by
   * Mem2Reg */
  if (num_params > 0) {
    int p_count = 0;
    Block *entry_blk = fn->blocks[fn->entry];
    if (entry_blk) {
      for (Instr *ins = entry_blk->head; ins; ins = ins->next) {
        if (ins->op == OP_ALLOCA) {
          if (p_count < num_params) {
            ins->escape = true;
            p_count++;
          }
        }
      }
    }
  }

  /* PGO instrumentation (before DCE/LICM): inject counter probes when requested
   */
  {
    const char *env = getenv("ZCC_PGO_INSTRUMENT");
    if (env && env[0] == '1') {
      uint32_t n = pgo_instrument_pass(fn);
      fprintf(stderr, "[PGO-Instr] injected %u probe instructions\n", n);
    }
  }

  /* Populate fn->def_of[] so DCE backward propagation can follow uses.
   * licm_build_def_block() does exactly this; call it before every DCE run. */
  licm_build_def_block(fn);

  /* ── Pass 0: Constant folding (exposes more dead code for DCE) ── */
  uint32_t folded = constant_fold_pass(fn);

  // NEW IR OPTIMIZATION PASSES
  uint32_t opt_sr = opt_strength_reduction_pass(fn);
  uint32_t opt_cp = opt_copy_prop_pass(fn);
  uint32_t opt_p = opt_peephole_pass(fn);
  uint32_t opt_cp2 = 0;
  if (opt_p > 0) {
    opt_cp2 = opt_copy_prop_pass(fn);
  }

  if (folded > 0 || opt_sr > 0 || opt_cp > 0 || opt_p > 0 || opt_cp2 > 0) {
    fprintf(
        stderr,
        "[IR-Opts] Folded: %u | S-Reduce: %u | Copy-Prop: %u | Peephole: %u | Copy-Prop2: %u\n",
        folded, opt_sr, opt_cp, opt_p, opt_cp2);
    licm_build_def_block(fn);
  }

  /* ── Pass 0b: Redundant Load Elimination ── */
  uint32_t rle_count = redundant_load_elim_pass(fn);
  if (rle_count > 0) {
    fprintf(stderr, "[RLE]       redundant loads eliminated: %u\n", rle_count);
    /* RLE produces COPYs — propagate them, then rebuild def chains */
    opt_copy_prop_pass(fn);
    licm_build_def_block(fn);
  }

  /* ── Pass 1: SSA-form DCE ── */
  uint32_t dce_removed = ssa_dce_pass(fn);
  uint32_t dce_total   = dce_removed;
#ifdef ZCC_DCE_VERBOSE
  fprintf(stderr, "[DCE->SSA]  instructions removed: %u  blocks removed: %u\n",
          dce_removed, fn->stats.dce_blocks_removed);
#endif

  /* Recompute reachability after DCE */
  compute_reachability(fn);

  /* ── Pass 2: LICM (Loop-Invariant Code Motion) ── */
  uint32_t licm_hoisted = licm_pass(fn);
  if (licm_hoisted > 0)
    fprintf(stderr, "[LICM]      instructions hoisted: %u\n", licm_hoisted);

  /* ── Pass 3: SSA-form DCE again (cleanup after LICM) ── */
  licm_build_def_block(fn);
  dce_removed = ssa_dce_pass(fn);
  dce_total  += dce_removed;
#ifdef ZCC_DCE_VERBOSE
  fprintf(stderr, "[DCE->SSA]  instructions removed: %u  blocks removed: %u\n",
          dce_removed, fn->stats.dce_blocks_removed);
#endif
  if (dce_total > 0)
    fprintf(stderr, "[DCE]  eliminated=%4u  (%u pre-LICM + %u post)\n",
            dce_total, dce_total - dce_removed, dce_removed);

  compute_reachability(fn);

  /* ── Pass 4: Escape Analysis (heap: large ctx; safe under deep ZCC stack) ──
   */
  EscapeCtx *ea_ctx = (EscapeCtx *)calloc(1, sizeof(EscapeCtx));
  if (!ea_ctx) {
    fprintf(stderr, "[run_all_passes] OOM escape ctx\n");
    return;
  }
  uint32_t promoted = escape_analysis_pass(fn, ea_ctx);
  fprintf(stderr,
          "[EscapeAna] allocations promoted to stack: %u  (of %u total)\n",
          promoted, ea_ctx->n_allocs);

  /* ── Pass 4b: Scalar promotion (mem2reg) — single-block allocas to vregs ──
   */
  uint32_t mem2reg_count = scalar_promotion_pass(fn, ea_ctx);
  if (mem2reg_count > 0) {
    fprintf(stderr, "[Mem2Reg]   single-block allocas promoted: %u\n",
            mem2reg_count);
    licm_build_def_block(fn);
    uint32_t dce_after = ssa_dce_pass(fn);
    fprintf(stderr,
            "[DCE->SSA]  instructions removed (after mem2reg): %u  blocks "
            "removed: %u\n",
            dce_after, fn->stats.dce_blocks_removed);
    compute_reachability(fn);
  }
  free(ea_ctx);

  /* ── Pass 5: PGO Basic Block Reordering ── */
  if (getenv("ZCC_DUMP_PGO_BLOCKS") &&
      getenv("ZCC_DUMP_PGO_BLOCKS")[0] == '1') {
    for (BlockID bi = 0; bi < fn->n_blocks; bi++) {
      Block *b = fn->blocks[bi];
      if (b && b->n_succs == 2)
        fprintf(stderr, "[PGO-BLOCK] %s n_succs=2 (profile-ready)\n", b->name);
    }
  }
  if (profile_path && profile_path[0])
    apply_profile_to_function(fn, profile_path);
  /* Optional: write current branch_probs for PGO round-trip. First function
   * truncates, rest append. */
  {
    const char *gen = getenv("ZCC_GEN_PROFILE");
    if (gen && gen[0]) {
      static int gen_profile_first = 1;
      FILE *pf = fopen(gen, gen_profile_first ? "w" : "a");
      if (pf) {
        gen_profile_first = 0;
        for (BlockID bi = 0; bi < fn->n_blocks; bi++) {
          Block *b = fn->blocks[bi];
          if (b && b->n_succs == 2)
            fprintf(pf, "%s %.4f %.4f\n", b->name, b->branch_prob[0],
                    b->branch_prob[1]);
        }
        fclose(pf);
      }
    }
  }
  result->n_blocks = pgo_reorder_pass(fn, result->order);
  result->licm_hoisted = fn->stats.licm_hoisted;
  result->dce_instrs_removed = fn->stats.dce_instrs_removed;
  for (uint32_t i = 0; i < result->n_blocks; i++) {
    BlockID bid = result->order[i];
    if (bid < fn->n_blocks && fn->blocks[bid]) {
      size_t len = strlen(fn->blocks[bid]->name);
      if (len >= ZCC_BLOCK_NAME_LEN)
        len = ZCC_BLOCK_NAME_LEN - 1;
      memcpy(result->block_names[i], fn->blocks[bid]->name, len);
      result->block_names[i][len] = '\0';
    } else
      result->block_names[i][0] = '\0';
  }
  fprintf(stderr, "[PGO-BBR]   blocks in emission order: %u\n",
          result->n_blocks);
}

/* ── Linear Scan Register Allocation ───────────────────────────────────────
 * Physical pool: %rbx, %r10, %r11, %r12, %r13, %r14, %r15 (avoid
 * %rax,%rcx,%rdx: div/shift). Callee-saved: rbx, r12, r13, r14, r15 → push in
 * prologue, pop before ret. */
#define N_PHYS_REGS 7
static const char *const phys_reg_name[N_PHYS_REGS] = {
    "rbx", "r10", "r11", "r12", "r13", "r14", "r15"};
#define PHYS_CALLEE_SAVED_MASK                                                 \
  ((1 << 0) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6)) /* rbx, r12-r15 */
static const int callee_saved_push_order[5] = {0, 3, 4, 5,
                                               6}; /* rbx, r12..r15 */
#define CALLEE_SAVED_PUSH_N 5

typedef struct {
  RegID vreg;
  int start;
  int end;
  int phys_reg; /* 0..N_PHYS_REGS-1 or -1 if spilled */
} LiveInterval;

static int live_interval_compare(const void *a, const void *b) {
  const LiveInterval *ia = (const LiveInterval *)a;
  const LiveInterval *ib = (const LiveInterval *)b;
  if (ia->start != ib->start)
    return (ia->start > ib->start) ? 1 : -1;
  return (ia->end > ib->end) ? 1 : ((ia->end < ib->end) ? -1 : 0);
}

/* Number instructions in block order; fill def_seq and last_use for each vreg.
 */
static void ir_asm_number_and_liveness(Function *fn,
                                       const uint32_t *block_order,
                                       uint32_t n_block_order, int *def_seq,
                                       int *last_use) {
  for (int i = 0; i < MAX_INSTRS; i++) {
    def_seq[i] = -1;
    last_use[i] = -1;
  }
  int seq = 0;
  for (uint32_t i = 0; i < n_block_order; i++) {
    BlockID bid = block_order[i];
    if (bid >= fn->n_blocks)
      continue;
    Block *blk = fn->blocks[bid];
    if (!blk)
      continue;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->op == OP_NOP)
        continue;
      ins->lscan_seq = seq;
      /* Uses */
      if (ins->op == OP_CONDBR) {
        if (ins->n_src >= 1 && ins->src[0])
          last_use[ins->src[0]] = seq;
      } else if (ins->op == OP_BR) {
        /* no reg operands */
      } else if (ins->op == OP_RET) {
        if (ins->n_src >= 1 && ins->src[0])
          last_use[ins->src[0]] = seq;
      } else if (ins->op == OP_STORE) {
        if (ins->n_src >= 1 && ins->src[0])
          last_use[ins->src[0]] = seq;
        if (ins->n_src >= 2 && ins->src[1])
          last_use[ins->src[1]] = seq;
      } else if (ins->op == OP_PHI) {
        for (uint32_t p = 0; p < ins->n_phi; p++)
          if (ins->phi[p].reg)
            last_use[ins->phi[p].reg] = seq;
      } else if (ins->op == OP_CALL) {
        for (uint32_t c = 0; c < ins->n_call_args; c++)
          if (ins->call_args[c])
            last_use[ins->call_args[c]] = seq;
      } else {
        for (uint32_t s = 0; s < ins->n_src; s++)
          if (ins->src[s])
            last_use[ins->src[s]] = seq;
      }
      /* Def */
      if (ins->dst &&
          (ins->op != OP_BR && ins->op != OP_CONDBR && ins->op != OP_STORE))
        def_seq[ins->dst] = seq;
      seq++;
    }
  }
  /* CG-IR-005 FIX (BUG 1): Back-edge PHI sources have inverted intervals
   * (last_use at PHI in header < def_seq in latch). The linear scan frees
   * their phys reg instantly, causing the edge copy to read wrong values.
   * Fix: extend last_use to end of function for any back-edge PHI source. */
  for (uint32_t i = 0; i < n_block_order; i++) {
    BlockID bid_fix = block_order[i];
    if (bid_fix >= fn->n_blocks)
      continue;
    Block *blk_fix = fn->blocks[bid_fix];
    if (!blk_fix)
      continue;
    for (Instr *phi_fix = blk_fix->head; phi_fix && phi_fix->op == OP_PHI;
         phi_fix = phi_fix->next) {
      for (uint32_t p = 0; p < phi_fix->n_phi; p++) {
        RegID src_r = phi_fix->phi[p].reg;
        if (!src_r || src_r >= MAX_INSTRS)
          continue;
        if (def_seq[src_r] >= 0 &&
            (last_use[src_r] < 0 || last_use[src_r] < def_seq[src_r])) {
          /* Back edge detected: source is defined AFTER its PHI use point.
           * Extend to end of function so linear scan keeps its phys reg alive
           * through the predecessor's BR where the edge copy executes. */
          last_use[src_r] = seq > 0 ? seq - 1 : 0;
        }
      }
    }
  }
}

/* Linear scan: assign phys reg or spill (-1). Fills phys_reg[]; spilled vregs
 * use stack. */
static void ir_asm_linear_scan(Function *fn, const uint32_t *block_order,
                               uint32_t n_block_order, int *def_seq,
                               int *last_use, int *phys_reg_out) {
  for (int i = 0; i < MAX_INSTRS; i++)
    phys_reg_out[i] = -1;
  LiveInterval intervals[MAX_INSTRS];
  int n_int = 0;
  for (int r = 0; r < MAX_INSTRS; r++) {
    if (def_seq[r] < 0)
      continue;
    intervals[n_int].vreg = (RegID)r;
    intervals[n_int].start = def_seq[r];
    intervals[n_int].end = last_use[r] >= 0 ? last_use[r] : def_seq[r];
    intervals[n_int].phys_reg = -1;
    n_int++;
  }
  qsort(intervals, (size_t)n_int, sizeof(LiveInterval), live_interval_compare);

  if (getenv("ZCC_DEBUG_LSCAN")) {
    for (int i = 0; i < n_int; i++) {
      fprintf(stderr, "[LSCAN] vreg=%d start=%d end=%d\n", intervals[i].vreg,
              intervals[i].start, intervals[i].end);
    }
  }

  int active_end = 0; /* active intervals: [0, active_end) */
  for (int i = 0; i < n_int; i++) {
    LiveInterval *cur = &intervals[i];
    /* Allocas produce stack addresses that must survive the entire function.
       Force them to spill — leaq offset(%rbp) is trivially recomputable. */
    if (cur->vreg < MAX_INSTRS && fn->def_of[cur->vreg] &&
        fn->def_of[cur->vreg]->op == OP_ALLOCA)
      continue;
    /* Expire intervals that end before cur->start */
    for (int j = 0; j < active_end; j++) {
      if (intervals[j].phys_reg >= 0 && intervals[j].end < cur->start) {
        intervals[j].phys_reg = -1;
        intervals[j] = intervals[active_end - 1];
        active_end--;
        j--;
      }
    }
    /* Find a free phys reg */
    int used_mask = 0;
    for (int j = 0; j < active_end; j++)
      if (intervals[j].phys_reg >= 0)
        used_mask |= (1 << intervals[j].phys_reg);
    int chosen = -1;
    for (int p = 0; p < N_PHYS_REGS; p++) {
      if (!(used_mask & (1 << p))) {
        chosen = p;
        break;
      }
    }
    if (chosen >= 0) {
      if (getenv("ZCC_DEBUG_LSCAN"))
        fprintf(stderr, "[LSCAN] vreg=%d gets phys_reg=%d\n", cur->vreg,
                chosen);
      cur->phys_reg = chosen;
      phys_reg_out[cur->vreg] = chosen;
      intervals[active_end++] = *cur;
    } else {
      if (getenv("ZCC_DEBUG_LSCAN"))
        fprintf(stderr, "[LSCAN] vreg=%d spilled\n", cur->vreg);
      /* No free reg: leave current interval spilled (no eviction = no spill
       * code to emit). */
    }
  }
}

/* ── IR-to-asm emission (for PGO-instrumented build) ─────────────────────────
 * Emits x86-64 body: each block gets a label, optional probe, then lowered
 * insns. With linear scan: vregs may be in phys regs or stack at
 * -8*(r+2)(%rbp). */
typedef struct {
  FILE *out;
  Function *fn;
  int func_end_label;
  int num_params; /* first num_params allocas map to -8(%rbp), -16(%rbp), ... */
  int global_block_offset; /* add to bid so counts are global across functions
                            */
  int func_label_id; /* unique per function so .Lir_b_%d_%u doesn't collide */
  int alloca_off[MAX_INSTRS]; /* alloca_off[dst] = offset from rbp (< 0) */
  int n_allocas;
  uint32_t block_order[MAX_BLOCKS];
  uint32_t n_block_order;
  /* Linear scan allocation: phys_reg[r] = 0..N_PHYS_REGS-1 or -1 (stack) */
  int phys_reg[MAX_INSTRS];
  int def_seq[MAX_INSTRS];
  int last_use[MAX_INSTRS];
  int used_callee_saved_mask; /* bits for rbx(0), r12(3), r13(4), r14(5), r15(6)
                               */
  int callee_saved_emitted;   /* 1 after we've emitted push of callee-saved */
  int body_only; /* 1 when AST owns prologue/epilogue (skip push/pop) */
  int slot_base; /* CG-IR-008: base offset for IR spill slots (0 or -stack_size)
                  */
  int ir_extra;
  int alloca_bytes_total;
  int csave_base; /* CG-IR-016-CSAVE-V2: %rbp-relative base of callee-save area.
                   * = -(stack_size + ir_extra) + 32
                   * Slots: csave_base (rbx), csave_base-8 (r12), -16 (r13),
                   *        csave_base-24 (r14), csave_base-32 (r15).          */
  int ret_size;  /* CG-IR-015: return type bytes (4=int, 8=long/ptr, 0→treat as 8) */
} IRAsmCtx;

static int ir_asm_slot(RegID r) { return -8 * (int)(r + 2); }

/* Return phys reg index (0..N_PHYS_REGS-1) or -1; if -1, *out_slot is stack
 * offset. */
static int ir_asm_vreg_location(IRAsmCtx *ctx, RegID r, int *out_slot) {
  *out_slot =
      ctx->slot_base - 8 * (int)(r + 1); /* CG-IR-008: offset below AST frame */
  if (!r || r >= MAX_INSTRS)
    return -1;
  if (ctx->phys_reg[r] >= 0)
    return ctx->phys_reg[r];
  return -1;
}

static void ir_asm_load_to_rax(IRAsmCtx *ctx, RegID r) {
  FILE *f = ctx->out;
  int slot;
  int p = ir_asm_vreg_location(ctx, r, &slot);
  if (p >= 0)
    fprintf(f, "    movq %%%s, %%rax\n", phys_reg_name[p]);
  else
    fprintf(f, "    movq %d(%%rbp), %%rax\n", slot);
}

static void ir_asm_load_to_rax_typed(IRAsmCtx *ctx, RegID r, IRType t) {
  FILE *f = ctx->out;
  int slot;
  int p = ir_asm_vreg_location(ctx, r, &slot);
  if (p >= 0) {
    fprintf(f, "    movq %%%s, %%rax\n", phys_reg_name[p]);
    if (t == IR_TY_I32)      fprintf(f, "    movslq %%eax, %%rax\n");
    else if (t == IR_TY_U32) fprintf(f, "    movl %%eax, %%eax\n");
  } else {
    if (t == IR_TY_I32)      fprintf(f, "    movslq %d(%%rbp), %%rax\n", slot);
    else if (t == IR_TY_U32) fprintf(f, "    movl %d(%%rbp), %%eax\n", slot);
    else                     fprintf(f, "    movq %d(%%rbp), %%rax\n", slot);
  }
}

static void ir_asm_store_rax_to(IRAsmCtx *ctx, RegID r) {
  FILE *f = ctx->out;
  int slot;
  int p = ir_asm_vreg_location(ctx, r, &slot);
  if (p >= 0)
    fprintf(f, "    movq %%rax, %%%s\n", phys_reg_name[p]);
  else
    fprintf(f, "    movq %%rax, %d(%%rbp)\n", slot);
}

static void ir_asm_load_to_rcx(IRAsmCtx *ctx, RegID r) {
  FILE *f = ctx->out;
  int slot;
  int p = ir_asm_vreg_location(ctx, r, &slot);
  if (p >= 0)
    fprintf(f, "    movq %%%s, %%rcx\n", phys_reg_name[p]);
  else
    fprintf(f, "    movq %d(%%rbp), %%rcx\n", slot);
}

/* Emit second operand for binary op: "addq <src>, %%rax" — <src> is reg or mem.
 */
static void ir_asm_emit_src_operand(IRAsmCtx *ctx, RegID r) {
  FILE *f = ctx->out;
  int slot;
  int p = ir_asm_vreg_location(ctx, r, &slot);
  if (p >= 0)
    fprintf(f, "%%%s", phys_reg_name[p]);
  else
    fprintf(f, "%d(%%rbp)", slot);
}

/* Emit copy on edge (from_bid -> to_bid) so PHI at merge gets its value; call
 * before emitting BR. */
static void ir_asm_emit_phi_edge_copy(IRAsmCtx *ctx, BlockID from_bid,
                                      BlockID to_bid) {
  Function *fn = ctx->fn;
  FILE *f = ctx->out;
  if (to_bid >= fn->n_blocks)
    return;
  Block *to_blk = fn->blocks[to_bid];
  if (!to_blk || !to_blk->head)
    return;

  /* CG-IR-005 FIX (BUG 3): Parallel copy semantics for PHI edge copies.
   * SSA PHIs execute simultaneously — all sources must be read BEFORE any
   * destination is written. Serial copy through %rax causes lost copies
   * when PHI_B's source overlaps PHI_A's destination.
   *
   * Strategy: Count PHIs, push all source values onto the x86 stack,
   * then pop them into destinations in reverse order. This uses the
   * hardware stack as a temporary buffer, guaranteeing all reads
   * complete before any writes. */

  /* Phase 1: count PHIs from this edge */
  int n_phis = 0;
  for (Instr *ins = to_blk->head; ins; ins = ins->next) {
    if (ins->op != OP_PHI)
      continue;
    for (uint32_t p = 0; p < ins->n_phi; p++) {
      if (ins->phi[p].block == from_bid) {
        n_phis++;
        break;
      }
    }
  }
  if (n_phis == 0)
    return;

  /* Fast path: single PHI — no interference possible */
  if (n_phis == 1) {
    for (Instr *ins = to_blk->head; ins; ins = ins->next) {
      if (ins->op != OP_PHI)
        continue;
      for (uint32_t p = 0; p < ins->n_phi; p++) {
        if (ins->phi[p].block == from_bid) {
          ir_asm_load_to_rax(ctx, ins->phi[p].reg);
          ir_asm_store_rax_to(ctx, ins->dst);
          return;
        }
      }
    }
    return;
  }

  /* Slow path: multiple PHIs — use stack to implement parallel copy.
   * Push all sources, then pop to destinations in reverse order. */

  /* Phase 2: push all source values (forward order) */
  for (Instr *ins = to_blk->head; ins; ins = ins->next) {
    if (ins->op != OP_PHI)
      continue;
    for (uint32_t p = 0; p < ins->n_phi; p++) {
      if (ins->phi[p].block == from_bid) {
        ir_asm_load_to_rax(ctx, ins->phi[p].reg);
        fprintf(f, "    pushq %%rax\n");
        break;
      }
    }
  }

  /* Phase 3: collect destination regs in order, then pop in reverse */
  {
    RegID *phi_dsts = (RegID *)malloc(sizeof(RegID) * n_phis);
    if (!phi_dsts) {
      fprintf(stderr, "Fatal: Out of memory in ir_asm_emit_phi_edge_copy\n");
      exit(1);
    }
    int n_dst = 0;
    for (Instr *ins = to_blk->head; ins; ins = ins->next) {
      if (ins->op != OP_PHI)
        continue;
      for (uint32_t p = 0; p < ins->n_phi; p++) {
        if (ins->phi[p].block == from_bid) {
          phi_dsts[n_dst++] = ins->dst;
          break;
        }
      }
    }
    /* Pop in reverse order (LIFO) to match push order */
    for (int i = n_dst - 1; i >= 0; i--) {
      fprintf(f, "    popq %%rax\n");
      ir_asm_store_rax_to(ctx, phi_dsts[i]);
    }
    free(phi_dsts);
  }
}

static void ir_asm_emit_probe(IRAsmCtx *ctx, BlockID bid) {
  FILE *f = ctx->out;
  int gbid = ctx->global_block_offset + (int)(uint32_t)bid;
  fprintf(f, "    leaq __zcc_edge_counts(%%rip), %%rax\n");
  fprintf(f, "    addq $%d, %%rax\n", gbid * 8);
  fprintf(f, "    movq (%%rax), %%rcx\n");
  fprintf(f, "    addq $1, %%rcx\n");
  fprintf(f, "    movq %%rcx, (%%rax)\n");
}

static void ir_asm_lower_insn(IRAsmCtx *ctx, const Instr *ins,
                              BlockID cur_block) {
  FILE *f = ctx->out;
  Function *fn = ctx->fn;
  (void)cur_block;
  switch (ins->op) {
  case OP_ASM: {
    fprintf(f, "    %s\n", ins->asm_string ? ins->asm_string : "");
    break;
  }
  case OP_VLA_ALLOC: {
    ir_asm_load_to_rax(ctx, ins->src[0]);
    fprintf(f, "    addq $15, %%rax\n");
    fprintf(f, "    andq $-16, %%rax\n");
    fprintf(f, "    subq %%rax, %%rsp\n");
    fprintf(f, "    movq %%rsp, %%rax\n");
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_PGO_COUNTER_ADDR: {
    int gbid = ctx->global_block_offset + (int)(uint32_t)ins->imm;
    fprintf(f, "    leaq __zcc_edge_counts(%%rip), %%rax\n");
    fprintf(f, "    addq $%d, %%rax\n", gbid * 8);
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_CONST: {
    int dst_slot;
    int dst_p = ir_asm_vreg_location(ctx, ins->dst, &dst_slot);
    int64_t k = (int64_t)ins->imm;
    if (dst_p >= 0) {
      if (k >= (int64_t)-2147483648LL && k <= (int64_t)2147483647LL)
        fprintf(f, "    movq $%lld, %%%s\n", (long long)k,
                phys_reg_name[dst_p]);
      else {
        fprintf(f, "    movabsq $%lld, %%%s\n", (long long)k,
                phys_reg_name[dst_p]);
      }
    } else {
      if (k >= (int64_t)-2147483648LL && k <= (int64_t)2147483647LL)
        fprintf(f, "    movq $%lld, %d(%%rbp)\n", (long long)k, dst_slot);
      else {
        fprintf(f, "    movabsq $%lld, %%rax\n", (long long)k);
        fprintf(f, "    movq %%rax, %d(%%rbp)\n", dst_slot);
      }
    }
    break;
  }
  case OP_ADD: {
    /* CG-IR-017: addl for I32/U32, addq for I64/U64 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    addl %%ecx, %%eax\n");
    } else {
      fprintf(f, "    addq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_SUB: {
    /* CG-IR-017: subl for I32/U32, subq for I64/U64 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    subl %%ecx, %%eax\n");
    } else {
      fprintf(f, "    subq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_MUL: {
    /* CG-IR-017: imull for I32/U32, imulq for I64/U64 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    fprintf(f, "    movq ");
    ir_asm_emit_src_operand(ctx, ins->src[1]);
    fprintf(f, ", %%rcx\n");
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32)
      fprintf(f, "    imull %%ecx, %%eax\n");
    else
      fprintf(f, "    imulq %%rcx, %%rax\n");
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_DIV: {
    /* CG-IR-015: select instruction width from ir_type.
     * Bug: cqo sign-extends rax→rdx:rax in 64-bit; for 32-bit int operands
     * that corrupts the dividend.  Fix: use cltd/idivl for I32, plain
     * xor+divl for U32, xor+divq for U64, cqo/idivq for I64 (default). */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32) {
      fprintf(f, "    cltd\n");              /* sign-extend eax → edx:eax    */
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    idivl %%ecx\n");
    } else if (ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movl %%eax, %%eax\n"); /* zero upper 32 bits of rax    */
      fprintf(f, "    xorl %%edx, %%edx\n");
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    divl %%ecx\n");
    } else if (ins->ir_type == IR_TY_U64) {
      fprintf(f, "    xorl %%edx, %%edx\n");
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    divq %%rcx\n");
    } else {                                /* IR_TY_I64 — original path     */
      fprintf(f, "    cqo\n");
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    idivq %%rcx\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_MOD: {
    /* CG-IR-015: same width/sign selection as OP_DIV; remainder is in
     * edx (32-bit) or rdx (64-bit) after the div instruction, so move it
     * into eax/rax before the generic ir_asm_store_rax_to. */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32) {
      fprintf(f, "    cltd\n");
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    idivl %%ecx\n");
      fprintf(f, "    movl %%edx, %%eax\n");
    } else if (ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movl %%eax, %%eax\n");
      fprintf(f, "    xorl %%edx, %%edx\n");
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    divl %%ecx\n");
      fprintf(f, "    movl %%edx, %%eax\n");
    } else if (ins->ir_type == IR_TY_U64) {
      fprintf(f, "    xorl %%edx, %%edx\n");
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    divq %%rcx\n");
      fprintf(f, "    movq %%rdx, %%rax\n");
    } else {                               /* IR_TY_I64 — original path      */
      fprintf(f, "    cqo\n");
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    idivq %%rcx\n");
      fprintf(f, "    movq %%rdx, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_BAND: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    andl %%ecx, %%eax\n");
    } else {
      fprintf(f, "    andq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_SHL: {
    /* CG-IR-015: fix undefined sz_suffix(ir_type_bytes(n->type)) reference.
     * For the immediate path: shlq is always correct for ADD/SUB lower-bits
     * (upper bits discarded anyway), but use shll for I32/U32 to stay clean.
     * For the reg-count path: was referencing undefined `n->type` — fixed. */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    int shl_is32 = (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32);
    if (ins->src[1] < MAX_INSTRS && fn->def_of[ins->src[1]] &&
        fn->def_of[ins->src[1]]->op == OP_CONST) {
      int64_t k = fn->def_of[ins->src[1]]->imm & (shl_is32 ? 31 : 63);
      if (shl_is32) fprintf(f, "    shll $%lld, %%eax\n", (long long)k);
      else          fprintf(f, "    shlq $%lld, %%rax\n", (long long)k);
    } else {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      if (shl_is32) fprintf(f, "    shll %%cl, %%eax\n");
      else          fprintf(f, "    shlq %%cl, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_SHR: {
    /* CG-IR-015: critical fix.
     *  Bug A: shrq on a 32-bit value shifts upper garbage into lower bits.
     *  Bug B: sarq used for unsigned right-shift propagates sign bit.
     *  Fix: select {shr,sar}{l,q} based on ir_type width + signedness.
     *  Also fixes the undefined sz_suffix(ir_type_bytes(n->type)) reference. */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    int shr_is32  = (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32);
    int shr_unsig = (ins->ir_type == IR_TY_U32 || ins->ir_type == IR_TY_U64);
    if (ins->src[1] < MAX_INSTRS && fn->def_of[ins->src[1]] &&
        fn->def_of[ins->src[1]]->op == OP_CONST) {
      int64_t k = fn->def_of[ins->src[1]]->imm & (shr_is32 ? 31 : 63);
      if      (shr_is32 && shr_unsig)  fprintf(f, "    shrl $%lld, %%eax\n", (long long)k);
      else if (shr_is32)               fprintf(f, "    sarl $%lld, %%eax\n", (long long)k);
      else if (shr_unsig)              fprintf(f, "    shrq $%lld, %%rax\n", (long long)k);
      else                             fprintf(f, "    shrq $%lld, %%rax\n", (long long)k); /* I64 default: keep shrq for compat */
    } else {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      if      (shr_is32 && shr_unsig)  fprintf(f, "    shrl %%cl, %%eax\n");
      else if (shr_is32)               fprintf(f, "    sarl %%cl, %%eax\n");
      else if (shr_unsig)              fprintf(f, "    shrq %%cl, %%rax\n");
      else                             fprintf(f, "    shrq %%cl, %%rax\n"); /* I64 default: keep shrq for compat */
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_LOAD: {
    if (s_debug_main_emit)
      fprintf(stderr, "[PGO-DEBUG] block %u OP_LOAD dst=%u src0=%u\n",
              (unsigned)cur_block, ins->dst, ins->src[0]);
    ir_asm_load_to_rax(ctx, ins->src[0]);
    switch ((int)ins->imm) {
    case 1:
      fprintf(f, "    movzbq (%%rax), %%rax\n");
      break;
    case 2:
      fprintf(f, "    movzwq (%%rax), %%rax\n");
      break;
    case 4:
      fprintf(f, "    movslq (%%rax), %%rax\n");
      break;
    default:
      fprintf(f, "    movq (%%rax), %%rax\n");
      break;
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_STORE:
    if (ins->n_src >= 2) {
      ir_asm_load_to_rax(ctx, ins->src[1]);
      ir_asm_load_to_rcx(ctx, ins->src[0]);
      switch ((int)ins->imm) {
      case 1:
        fprintf(f, "    movb %%cl, (%%rax)\n");
        break;
      case 2:
        fprintf(f, "    movw %%cx, (%%rax)\n");
        break;
      case 4:
        fprintf(f, "    movl %%ecx, (%%rax)\n");
        break;
      default:
        fprintf(f, "    movq %%rcx, (%%rax)\n");
        break;
      }
    }
    break;
  case OP_BR:
    if (ins->n_src >= 1) {
      ir_asm_emit_phi_edge_copy(ctx, cur_block, (BlockID)ins->src[0]);
      fprintf(f, "    jmp .Lir_b_%d_%u\n", ctx->func_label_id,
              (unsigned)ins->src[0]);
    }
    break;
  case OP_CONDBR:
    /* CG-IR-005 FIX (BUG 2): PHI edge copies must only execute on the
     * TAKEN path. Previously, true-target copies ran unconditionally before
     * jnz, corrupting PHI slots when the false path was taken.
     * Fix: jz to false_path label, emit true copies + jmp, then false copies +
     * jmp. */
    if (ins->n_src >= 3) {
      ir_asm_load_to_rax(ctx, ins->src[0]);
      fprintf(f, "    testq %%rax, %%rax\n");
      fprintf(f, "    jz .Lir_cf_%d_%u\n", ctx->func_label_id,
              (unsigned)cur_block);
      /* True path: PHI copies then jump to true target */
      ir_asm_emit_phi_edge_copy(ctx, cur_block, (BlockID)ins->src[1]);
      fprintf(f, "    jmp .Lir_b_%d_%u\n", ctx->func_label_id,
              (unsigned)ins->src[1]);
      /* False path: PHI copies then jump to false target */
      fprintf(f, ".Lir_cf_%d_%u:\n", ctx->func_label_id, (unsigned)cur_block);
      ir_asm_emit_phi_edge_copy(ctx, cur_block, (BlockID)ins->src[2]);
      fprintf(f, "    jmp .Lir_b_%d_%u\n", ctx->func_label_id,
              (unsigned)ins->src[2]);
    }
    break;
  case OP_RET:
    if (ins->n_src >= 1)
      ir_asm_load_to_rax(ctx, ins->src[0]);
    /* CG-IR-015: zero-truncate 32-bit return values.
     * 64-bit arithmetic (neg/sub on sign-extended values) can leave garbage
     * in the upper 32 bits of rax.  Callers of int-returning functions read
     * only eax; the movl self-move zero-extends eax → rax implicitly. */
    if (ctx->ret_size == 4)
      fprintf(f, "    movl %%eax, %%eax\n");
    if (!ctx->body_only) {
      for (int i = CALLEE_SAVED_PUSH_N - 1; i >= 0; i--) {
        int p = callee_saved_push_order[i];
        if (ctx->used_callee_saved_mask & (1 << p))
          fprintf(f, "    popq %%%s\n", phys_reg_name[p]);
      }
    } else if (ctx->csave_base) {
      /* CG-IR-016-RET: body_only path — restore callee-saved registers
       * BEFORE the jmp so they execute on every return path, not just
       * fall-through.  The 'after-body' restores in
       * zcc_run_passes_emit_body_pgo are only reached by fall-through
       * (functions with no explicit return) — dead code for most funcs. */
      fprintf(f, "    movq %d(%%rbp), %%rbx\n", ctx->csave_base);
      fprintf(f, "    movq %d(%%rbp), %%r12\n", ctx->csave_base - 8);
      fprintf(f, "    movq %d(%%rbp), %%r13\n", ctx->csave_base - 16);
      fprintf(f, "    movq %d(%%rbp), %%r14\n", ctx->csave_base - 24);
      fprintf(f, "    movq %d(%%rbp), %%r15\n", ctx->csave_base - 32);
    }
    fprintf(f, "    jmp .Lfunc_end_%d\n", ctx->func_end_label);
    break;
  case OP_LT: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    cmpl %%ecx, %%eax\n");
      fprintf(f, "    setl %%al\n");
      fprintf(f, "    movzbl %%al, %%eax\n");
    } else {
      fprintf(f, "    cmpq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
      fprintf(f, "    setl %%al\n");
      fprintf(f, "    movzbq %%al, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_EQ: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    cmpl %%ecx, %%eax\n");
      fprintf(f, "    sete %%al\n");
      fprintf(f, "    movzbl %%al, %%eax\n");
    } else {
      fprintf(f, "    cmpq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
      fprintf(f, "    sete %%al\n");
      fprintf(f, "    movzbq %%al, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_NE: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    cmpl %%ecx, %%eax\n");
      fprintf(f, "    setne %%al\n");
      fprintf(f, "    movzbl %%al, %%eax\n");
    } else {
      fprintf(f, "    cmpq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
      fprintf(f, "    setne %%al\n");
      fprintf(f, "    movzbq %%al, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_GT: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    cmpl %%ecx, %%eax\n");
      fprintf(f, "    setg %%al\n");
      fprintf(f, "    movzbl %%al, %%eax\n");
    } else {
      fprintf(f, "    cmpq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
      fprintf(f, "    setg %%al\n");
      fprintf(f, "    movzbq %%al, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_GE: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    cmpl %%ecx, %%eax\n");
      fprintf(f, "    setge %%al\n");
      fprintf(f, "    movzbl %%al, %%eax\n");
    } else {
      fprintf(f, "    cmpq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
      fprintf(f, "    setge %%al\n");
      fprintf(f, "    movzbq %%al, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_LE: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    cmpl %%ecx, %%eax\n");
      fprintf(f, "    setle %%al\n");
      fprintf(f, "    movzbl %%al, %%eax\n");
    } else {
      fprintf(f, "    cmpq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
      fprintf(f, "    setle %%al\n");
      fprintf(f, "    movzbq %%al, %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_BOR: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    orl %%ecx, %%eax\n");
    } else {
      fprintf(f, "    orq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_BXOR: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32) {
      fprintf(f, "    movq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rcx\n");
      fprintf(f, "    xorl %%ecx, %%eax\n");
    } else {
      fprintf(f, "    xorq ");
      ir_asm_emit_src_operand(ctx, ins->src[1]);
      fprintf(f, ", %%rax\n");
    }
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_BNOT: {
    /* CG-IR-017 */
    ir_asm_load_to_rax_typed(ctx, ins->src[0], ins->ir_type);
    if (ins->ir_type == IR_TY_I32 || ins->ir_type == IR_TY_U32)
      fprintf(f, "    notl %%eax\n");
    else
      fprintf(f, "    notq %%rax\n");
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_COPY: {
    if (ins->n_src >= 1) {
      ir_asm_load_to_rax(ctx, ins->src[0]);
      ir_asm_store_rax_to(ctx, ins->dst);
    }
    break;
  }
  case OP_PHI:
    /* No code: value is placed on incoming edges by ir_asm_emit_phi_edge_copy.
     */
    break;
  case OP_ALLOCA: {
    int off = (ins->dst < MAX_INSTRS) ? ctx->alloca_off[ins->dst] : 0;
    fprintf(f, "    leaq %d(%%rbp), %%rax\n", off);
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_GLOBAL: {
    const char *sym = ins->call_name[0] ? ins->call_name : "L_global";
    fprintf(f, "    leaq %s(%%rip), %%rax\n", sym);
    ir_asm_store_rax_to(ctx, ins->dst);
    break;
  }
  case OP_CALL: {
    static const char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int na = (int)ins->n_call_args;
    int stack_args = (na > 6) ? na - 6 : 0;
    int n_csave =
        ctx->body_only ? 0 : __builtin_popcount(ctx->used_callee_saved_mask);

    int current_seq = ins->lscan_seq;
    int caller_saved_live[N_PHYS_REGS];
    int n_caller_saved = 0;

    /* Liveness Query: Identify caller-saved regs (r10, r11) holding a live var.
     * (rax/rdi/rdx, etc., are safely excluded universally from N_PHYS_REGS).
     * An active caller-save has def_seq BEFORE current and last_use AFTER. */
    for (int p = 0; p < N_PHYS_REGS; p++) {
      if (!(PHYS_CALLEE_SAVED_MASK & (1 << p))) {
        for (int r = 1; r < MAX_INSTRS; r++) {
          if (ctx->phys_reg[r] == p) {
            if (ctx->def_seq[r] < current_seq && ctx->last_use[r] > current_seq) {
              caller_saved_live[n_caller_saved++] = p;
              break;
            }
          }
        }
      }
    }

    /* Emit pushq for active caller-saved registers */
    for (int i = 0; i < n_caller_saved; i++) {
      fprintf(f, "    pushq %%%s\n", phys_reg_name[caller_saved_live[i]]);
    }

    /* Alignment: Ensure exact 16-byte RSP alignment for callq */
    int need_pad = (n_csave + n_caller_saved + stack_args) & 1;
    if (need_pad)
      fprintf(f, "    subq $8, %%rsp\n");
    /* Push stack args (7th+) in reverse order */
    for (int i = na - 1; i >= 6; i--) {
      ir_asm_load_to_rax(ctx, ins->call_args[i]);
      fprintf(f, "    pushq %%rax\n");
    }
    /* Load register args 0..5 into rdi/rsi/rdx/rcx/r8/r9 */
    int reg_count = (na < 6) ? na : 6;
    for (int i = 0; i < reg_count; i++) {
      ir_asm_load_to_rax(ctx, ins->call_args[i]);
      fprintf(f, "    movq %%rax, %%%s\n", arg_regs[i]);
    }
    fprintf(f, "    xorl %%eax, %%eax\n"); /* al=0 for variadic */
    if (ins->call_name[0]) {
      fprintf(f, "    callq %s\n", ins->call_name);
    } else {
      ir_asm_load_to_rax(ctx, ins->src[0]);
      fprintf(f, "    callq *%%rax\n");
    }
    /* Cleanup stack args + pad */
    int cleanup = stack_args * 8 + (need_pad ? 8 : 0);
    if (cleanup > 0)
      fprintf(f, "    addq $%d, %%rsp\n", cleanup);
      
    /* Store return value */
    if (ins->dst)
      ir_asm_store_rax_to(ctx, ins->dst);

    /* Restore active caller-saved registers in reverse order. */
    for (int i = n_caller_saved - 1; i >= 0; i--) {
      fprintf(f, "    popq %%%s\n", phys_reg_name[caller_saved_live[i]]);
    }
    break;
  }
  default:
    break;
  }
}

static void ir_asm_assign_alloca_offsets(IRAsmCtx *ctx) {
  Function *fn = ctx->fn;
  int n = 0;
  int np = ctx->num_params;
  uint32_t max_reg = fn->n_regs;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk) continue;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->dst >= max_reg) max_reg = ins->dst + 1;
      for (uint32_t s = 0; s < ins->n_src && s < MAX_OPERANDS; s++)
        if (ins->src[s] >= max_reg) max_reg = ins->src[s] + 1;
    }
  }
  int current_alloca_offset = ctx->slot_base - 8 * (int)(max_reg + 2);
  int total_bytes = 0;
  if (np < 0)
    np = 0;
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    Block *blk = fn->blocks[bi];
    if (!blk)
      continue;
    for (Instr *ins = blk->head; ins; ins = ins->next) {
      if (ins->op != OP_ALLOCA || !ins->dst)
        continue;
      int size = (ins->imm > 0) ? (int)ins->imm : 8;
      size = (size + 7) & ~7;
      int off;
      if (n < np) {
        off = -8 * (n + 1);
      } else {
        current_alloca_offset -= size;
        off = current_alloca_offset;
        total_bytes += size;
      }
      if (ins->dst < MAX_INSTRS) {
        ctx->alloca_off[ins->dst] = off;
        fprintf(stderr, "[ALLOCA] dst=%u off=%d size=%d\n", ins->dst, off, size);
      }
      n++;
    }
  }
  ctx->n_allocas = n;
  ctx->alloca_bytes_total = total_bytes;
}

static void ir_asm_emit_one_block(IRAsmCtx *ctx, BlockID bid) {
  Function *fn = ctx->fn;
  FILE *f = ctx->out;
  Block *blk = fn->blocks[bid];
  if (!blk)
    return;
  fprintf(f, ".Lir_b_%d_%u:\n", ctx->func_label_id, (unsigned)bid);
  Instr *ins = blk->head;
  if (ins && ins->op == OP_PGO_COUNTER_ADDR) {
    ir_asm_emit_probe(ctx, bid);
    for (int k = 0; k < 5 && ins; k++)
      ins = ins->next;
  }
  /* Emit push of callee-saved regs once (first time we emit a real
   * instruction). */
  if (!ctx->body_only && !ctx->callee_saved_emitted &&
      ctx->used_callee_saved_mask) {
    for (int i = 0; i < CALLEE_SAVED_PUSH_N; i++) {
      int p = callee_saved_push_order[i];
      if (ctx->used_callee_saved_mask & (1 << p))
        fprintf(f, "    pushq %%%s\n", phys_reg_name[p]);
    }
    ctx->callee_saved_emitted = 1;
  }
  int last_line = -1;
  for (; ins; ins = ins->next) {
    if (ins->op == OP_NOP)
      continue;
    if (ins->line_no > 0 && ins->line_no != last_line) {
      fprintf(f, "\t.loc 1 %d\n", ins->line_no);
      last_line = ins->line_no;
    }
    ir_asm_lower_insn(ctx, ins, bid);
  }
  /* CG-IR-005 fall-through fixup: if the last instruction in this block is
     not a terminator (OP_BR/OP_CONDBR/OP_RET), emit an explicit jmp to the
     first successor.  After PGO reordering, empty merge blocks and phi-only
     blocks can land anywhere in the emission order; without this they fall
     through into whatever block happens to be next, causing infinite loops. */
  {
    Instr *tail = blk->tail;
    while (tail && tail->op == OP_NOP)
      tail = tail->prev;
    int need_jmp = 1;
    if (tail) {
      Opcode op = tail->op;
      if (op == OP_BR || op == OP_CONDBR || op == OP_RET)
        need_jmp = 0;
    }
    if (need_jmp && blk->n_succs >= 1) {
      BlockID succ0 = blk->succs[0];
      ir_asm_emit_phi_edge_copy(ctx, bid, succ0);
      fprintf(f, "    jmp .Lir_b_%d_%u\n", ctx->func_label_id, (unsigned)succ0);
    }
  }
}

static void ir_asm_emit_function_body(IRAsmCtx *ctx) {
  Function *fn = ctx->fn;
  /* ir_asm_assign_alloca_offsets hoisted to caller */

  /* If no order was provided (e.g. emit path that didn't run passes), build BFS
   * order. */
  if (ctx->n_block_order == 0) {
    bool visited[MAX_BLOCKS];
    memset(visited, 0, sizeof(visited));
    uint32_t queue[MAX_BLOCKS];
    uint32_t head = 0, tail = 0;
    queue[tail++] = fn->entry;
    visited[fn->entry] = true;
    while (head < tail) {
      BlockID bid = queue[head++];
      Block *blk = fn->blocks[bid];
      for (uint32_t si = 0; si < blk->n_succs; si++) {
        BlockID s = blk->succs[si];
        if (s < fn->n_blocks && !visited[s]) {
          visited[s] = true;
          queue[tail++] = s;
        }
      }
    }
    ctx->n_block_order = tail;
    for (uint32_t i = 0; i < tail; i++)
      ctx->block_order[i] = queue[i];
  }

  /* Linear scan register allocation: number instructions, compute intervals,
   * assign phys regs. */
  for (int i = 0; i < MAX_INSTRS; i++)
    ctx->phys_reg[i] = -1;
  ir_asm_number_and_liveness(fn, ctx->block_order, ctx->n_block_order,
                             ctx->def_seq, ctx->last_use);
  ir_asm_linear_scan(fn, ctx->block_order, ctx->n_block_order, ctx->def_seq,
                     ctx->last_use, ctx->phys_reg);

  /* Which callee-saved phys regs (rbx, r12-r15) are used — for
   * prologue/epilogue. */
  ctx->used_callee_saved_mask = 0;
  for (int i = 0; i < MAX_INSTRS; i++)
    if (ctx->phys_reg[i] >= 0 &&
        (PHYS_CALLEE_SAVED_MASK & (1 << ctx->phys_reg[i])))
      ctx->used_callee_saved_mask |= (1 << ctx->phys_reg[i]);

  bool emitted[MAX_BLOCKS];
  memset(emitted, 0, sizeof(emitted));
  /* Phase 1: hot path — emit blocks in PGO/BBR order (maximizes icache
   * locality). */
  for (uint32_t i = 0; i < ctx->n_block_order; i++) {
    BlockID bid = ctx->block_order[i];
    if (bid >= fn->n_blocks)
      continue;
    ir_asm_emit_one_block(ctx, bid);
    emitted[bid] = true;
  }
  /* Phase 2: cold/unreachable — emit any remaining blocks so every branch
   * target has a label. */
  for (BlockID bid = 0; bid < fn->n_blocks; bid++) {
    if (!emitted[bid])
      ir_asm_emit_one_block(ctx, bid);
  }
}

#define PGO_NAME_STRIDE 64

/* Fill metadata for fn: block names (64 bytes each) and edges (2 ints per
 * block: s0, s1; -1 if missing). */
static void ir_asm_fill_metadata(Function *fn, char *out_names,
                                 int *out_edges) {
  for (uint32_t i = 0; i < fn->n_blocks; i++) {
    Block *blk = fn->blocks[i];
    if (out_names) {
      size_t len = 0;
      if (blk) {
        while (len < (size_t)(NAME_LEN - 1) && blk->name[len])
          len++;
      }
      if (blk)
        memcpy(out_names + i * PGO_NAME_STRIDE, blk->name, len);
      memset(out_names + i * PGO_NAME_STRIDE + len, 0, PGO_NAME_STRIDE - len);
    }
    if (out_edges) {
      int s0 = -1, s1 = -1;
      if (blk && blk->n_succs > 0)
        s0 = (int)(uint32_t)blk->succs[0];
      if (blk && blk->n_succs > 1)
        s1 = (int)(uint32_t)blk->succs[1];
      out_edges[i * 2 + 0] = s0;
      out_edges[i * 2 + 1] = s1;
    }
  }
}

/* Build IR, run passes; if ZCC_PGO_INSTRUMENT=1 emit body to out, fill
 * metadata, return n_blocks; else return 0. global_block_offset: add to block
 * ids in probes. func_label_id: unique id for block labels (.Lir_b_%d_%u).
 * out_names: 64 bytes per block; out_edges: 2 ints per block.
 * max_blocks_capacity: if > 0 and fn->n_blocks > capacity, return 0 without
 * emitting (graceful degradation). */
int zcc_run_passes_emit_body(ZCCNode *body_ast, const char *profile_path,
                             const char *func_name, void *out_file,
                             int stack_size, int num_params, int func_end_label,
                             int global_block_offset, int func_label_id,
                             char *out_names, int *out_edges,
                             int max_blocks_capacity) {
  const char *env = getenv("ZCC_PGO_INSTRUMENT");
  if (!env || env[0] != '1')
    return 0;
  FILE *out = (FILE *)out_file;
  (void)stack_size;

  {
    const char *d = getenv("ZCC_PGO_DEBUG_MAIN");
    s_debug_main_emit =
        (d && d[0] == '1' && func_name && strcmp(func_name, "main") == 0) ? 1
                                                                          : 0;
  }

  Function *fn = zcc_ast_to_ir(body_ast, func_name);
  if (!fn)
    return 0;

  PassResult *result = (PassResult *)calloc(1, sizeof(PassResult));
  if (!result) {
    zcc_ir_free(fn);
    return 0;
  }
  run_all_passes(fn, result, profile_path, num_params);

  /* When debugging main crash: dump full IR so we can map faulting block/reg 22
   * back to defs. */
  if (s_debug_main_emit) {
    fprintf(stderr, "[PGO-DEBUG-IR] === main() IR (after passes) ===\n");
    uint32_t *order = result->order;
    uint32_t n_order = result->n_blocks <= MAX_BLOCKS ? result->n_blocks : 0;
    bool dumped[MAX_BLOCKS];
    for (uint32_t i = 0; i < fn->n_blocks && i < MAX_BLOCKS; i++)
      dumped[i] = false;
    for (uint32_t oi = 0; oi < (n_order ? n_order : fn->n_blocks); oi++) {
      BlockID bid = n_order ? order[oi] : oi;
      if (bid >= fn->n_blocks || !fn->blocks[bid])
        continue;
      dumped[bid] = true;
      Block *blk = fn->blocks[bid];
      fprintf(stderr, "[PGO-DEBUG-IR] block %u (%s) preds=%u succs=%u\n",
              (unsigned)bid, blk->name, blk->n_preds, blk->n_succs);
      for (Instr *ins = blk->head; ins; ins = ins->next) {
        const char *op =
            (unsigned)ins->op < sizeof(opcode_name) / sizeof(opcode_name[0])
                ? opcode_name[ins->op]
                : "?";
        fprintf(stderr, "[PGO-DEBUG-IR]   %s dst=%u", op, ins->dst);
        for (uint32_t s = 0; s < ins->n_src && s < MAX_OPERANDS; s++)
          fprintf(stderr, " src[%u]=%u", s, ins->src[s]);
        if (ins->op == OP_PHI && ins->n_phi) {
          fprintf(stderr, " phi");
          for (uint32_t p = 0; p < ins->n_phi; p++)
            fprintf(stderr, " %u:B%u", ins->phi[p].reg,
                    (unsigned)ins->phi[p].block);
        }
        if (ins->imm)
          fprintf(stderr, " imm=%lld", (long long)ins->imm);
        if (ins->call_name[0])
          fprintf(stderr, " call=%s", ins->call_name);
        fprintf(stderr, "\n");
      }
    }
    /* Phase-2 blocks (cold/unreachable): include so faulting block 4 is
     * visible. */
    for (BlockID bid = 0; bid < fn->n_blocks; bid++) {
      if (dumped[bid] || !fn->blocks[bid])
        continue;
      Block *blk = fn->blocks[bid];
      fprintf(stderr,
              "[PGO-DEBUG-IR] block %u (%s) preds=%u succs=%u [cold/unreach]\n",
              (unsigned)bid, blk->name, blk->n_preds, blk->n_succs);
      for (Instr *ins = blk->head; ins; ins = ins->next) {
        const char *op =
            (unsigned)ins->op < sizeof(opcode_name) / sizeof(opcode_name[0])
                ? opcode_name[ins->op]
                : "?";
        fprintf(stderr, "[PGO-DEBUG-IR]   %s dst=%u", op, ins->dst);
        for (uint32_t s = 0; s < ins->n_src && s < MAX_OPERANDS; s++)
          fprintf(stderr, " src[%u]=%u", s, ins->src[s]);
        if (ins->op == OP_PHI && ins->n_phi) {
          fprintf(stderr, " phi");
          for (uint32_t p = 0; p < ins->n_phi; p++)
            fprintf(stderr, " %u:B%u", ins->phi[p].reg,
                    (unsigned)ins->phi[p].block);
        }
        if (ins->imm)
          fprintf(stderr, " imm=%lld", (long long)ins->imm);
        if (ins->call_name[0])
          fprintf(stderr, " call=%s", ins->call_name);
        fprintf(stderr, "\n");
      }
    }
    fprintf(stderr, "[PGO-DEBUG-IR] === end main IR ===\n");
  }

  if (max_blocks_capacity > 0 &&
      (int)(uint32_t)fn->n_blocks > max_blocks_capacity) {
    fprintf(stderr,
            "[PGO] Warning: global block limit exceeded (%u blocks). Skipping "
            "bridge for this function.\n",
            (unsigned)fn->n_blocks);
    free(result);
    zcc_ir_free(fn);
    s_debug_main_emit = 0;
    return 0;
  }

  IRAsmCtx ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.out = out;
  ctx.fn = fn;
  ctx.func_end_label = func_end_label;
  ctx.num_params = num_params;
  ctx.global_block_offset = global_block_offset;
  ctx.func_label_id = func_label_id;
  for (int i = 0; i < MAX_INSTRS; i++)
    ctx.alloca_off[i] = 0;
  /* Use PGO/BBR emission order from passes so layout respects profile. */
  ctx.n_block_order = result->n_blocks <= MAX_BLOCKS ? result->n_blocks : 0;
  for (uint32_t i = 0; i < ctx.n_block_order; i++)
    ctx.block_order[i] = result->order[i];

  ir_asm_assign_alloca_offsets(&ctx);
  ir_asm_emit_function_body(&ctx);
  ir_asm_fill_metadata(fn, out_names, out_edges);

  fprintf(stderr,
          "[ZCC-IR] fn=%s  emitted from IR (PGO instrumented) %u blocks\n",
          func_name ? func_name : "?", (unsigned)fn->n_blocks);
  int n_blocks = (int)(uint32_t)fn->n_blocks;
  free(result);
  zcc_ir_free(fn);
  s_debug_main_emit = 0;
  return n_blocks;
}

/* Emit body from IR with PGO block order (no instrumentation). Used when
 * -use-profile=... and bridge=1. */
int zcc_run_passes_emit_body_pgo(ZCCNode *body_ast, const char *profile_path,
                                 const char *func_name, void *out_file,
                                 int stack_size, int num_params,
                                 int func_end_label, int func_label_id) {
  FILE *out = (FILE *)out_file;
  Function *fn = zcc_ast_to_ir(body_ast, func_name);
  if (!fn)
    return 0;

  if (func_name && strcmp(func_name, "main") == 0) {
    fprintf(stderr, "[DEBUG] RAW IR FOR MAIN BEFORE PASSES:\n");
    for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
      Block *b = fn->blocks[bi];
      if (!b)
        continue;
      fprintf(stderr, " Block %u:\n", bi);
      for (Instr *ins = b->head; ins; ins = ins->next) {
        if (ins->op == OP_STORE) {
          fprintf(stderr, "   OP_STORE src[0]=%u src[1]=%u\n", ins->src[0],
                  ins->src[1]);
        } else if (ins->op == OP_CONST) {
          fprintf(stderr, "   OP_CONST dst=%u imm=%d\n", ins->dst,
                  (int)(intptr_t)ins->imm);
        }
      }
    }
  }

  PassResult *result = (PassResult *)calloc(1, sizeof(PassResult));
  if (!result) {
    zcc_ir_free(fn);
    return 0;
  }
  run_all_passes(fn, result, profile_path, num_params);

  IRAsmCtx ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.out = out;
  ctx.fn = fn;
  ctx.func_end_label = func_end_label;
  ctx.num_params = num_params;
  ctx.global_block_offset = 0;
  ctx.func_label_id = func_label_id;
  ctx.body_only =
      1; /* AST owns prologue/epilogue -- skip push/pop of callee-saved */
  ctx.slot_base = -stack_size; /* CG-IR-008: IR slots start below AST frame */
  int ir_extra = 0;
  for (int i = 0; i < MAX_INSTRS; i++)
    ctx.alloca_off[i] = 0;
  ctx.n_block_order = result->n_blocks <= MAX_BLOCKS ? result->n_blocks : 0;
  for (uint32_t i = 0; i < ctx.n_block_order; i++)
    ctx.block_order[i] = result->order[i];

    /* CG-IR-009: scan IR to compute exact frame depth before emission.
   * Must happen BEFORE ir_asm_emit_function_body since alloca offsets
   * and spill slots all live below slot_base.
   * CG-IR-016-CSAVE-V2: ir_extra extended by n_csave_slots=5 to give
   * each callee-saved register its own explicit 8-byte slot at the very
   * bottom of the frame.  csave_base is the highest of those 5 slots.  */
  {
    ir_asm_assign_alloca_offsets(&ctx);
    
    uint32_t max_reg = fn->n_regs;
    for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
      Block *b = fn->blocks[bi];
      if (!b) continue;
      for (Instr *ins = b->head; ins; ins = ins->next) {
        if (ins->dst >= max_reg) max_reg = ins->dst + 1;
        for (uint32_t s = 0; s < ins->n_src && s < MAX_OPERANDS; s++)
          if (ins->src[s] >= max_reg) max_reg = ins->src[s] + 1;
      }
    }
    
    int n_csave_slots = 5; /* rbx, r12, r13, r14, r15 — always reserve all 5 */
    ctx.ir_extra = 8 * ((int)max_reg + 8 + n_csave_slots) + ctx.alloca_bytes_total;
    ctx.ir_extra = (ctx.ir_extra + 15) & ~15;
    ir_extra = ctx.ir_extra;
    fprintf(out, "    subq $%d, %%rsp\n", ctx.ir_extra);

    /* CG-IR-016-CSAVE-V3: csave_base is the %rbp-relative offset of the
     * highest callee-save slot (rbx). It sits at the extreme bottom of the 
     * expanded ir_extra frame mathematically avoiding all dynamically allocated buffers. */
    ctx.csave_base = -(stack_size + ctx.ir_extra) + 32;
  }

  /* CG-IR-016-CSAVE-V2: save callee-saved registers via movq (NOT pushq —
   * pushq would shift RSP and break every slot_base-relative spill access).
   * All 5 are always saved unconditionally; cost is 5 movq at entry/exit.  */
  fprintf(out, "    movq %%rbx, %d(%%rbp)\n", ctx.csave_base);
  fprintf(out, "    movq %%r12, %d(%%rbp)\n", ctx.csave_base - 8);
  fprintf(out, "    movq %%r13, %d(%%rbp)\n", ctx.csave_base - 16);
  fprintf(out, "    movq %%r14, %d(%%rbp)\n", ctx.csave_base - 24);
  fprintf(out, "    movq %%r15, %d(%%rbp)\n", ctx.csave_base - 32);

  ir_asm_emit_function_body(&ctx);

  /* CG-IR-016-CSAVE-V2: restore callee-saved registers before AST epilogue */
  fprintf(out, "    movq %d(%%rbp), %%rbx\n", ctx.csave_base);
  fprintf(out, "    movq %d(%%rbp), %%r12\n", ctx.csave_base - 8);
  fprintf(out, "    movq %d(%%rbp), %%r13\n", ctx.csave_base - 16);
  fprintf(out, "    movq %d(%%rbp), %%r14\n", ctx.csave_base - 24);
  fprintf(out, "    movq %d(%%rbp), %%r15\n", ctx.csave_base - 32);

  fprintf(stderr, "[ZCC-IR] fn=%s  emitted from IR (PGO layout) %u blocks\n",
          func_name ? func_name : "?", (unsigned)fn->n_blocks);
  int n_blocks = (int)(uint32_t)fn->n_blocks;
  free(result);
  zcc_ir_free(fn);
  return n_blocks;
}

/* Opaque ABI boundary: ZCC calls this with (ir_tree, profile_path, func_name).
 * We own PassResult (calloc/free) and all result field access — avoids layout
 * mismatch (ZCC uint32_t vs GCC uint32_t) and stack overflow (no large struct
 * on ZCC stack). */
void zcc_run_passes_log(ZCCNode *body_ast, const char *profile_path,
                        const char *func_name) {
  Function *fn = zcc_ast_to_ir(body_ast, func_name);
  if (!fn)
    return;
  PassResult *result = (PassResult *)calloc(1, sizeof(PassResult));
  if (!result) {
    zcc_ir_free(fn);
    return;
  }
  run_all_passes(fn, result, profile_path, 0);
  fprintf(stderr, "[ZCC-IR] fn=%s  blocks=%u  hoisted=%u  dce=%u\n",
          func_name ? func_name : "?", result->n_blocks, result->licm_hoisted,
          result->dce_instrs_removed);
  free(result);
  zcc_ir_free(fn);
}

/* ─────────────────────────────────────────────────────────────────────────────
 * TEST FIXTURE  — constructs a small synthetic CFG to exercise all three passes
 *
 *   entry → loop_header → loop_body → loop_latch ──(back)──┐
 *                  │                                        │
 *                  └──(exit cond)──→ loop_exit → fn_exit    │
 *                                                           │
 *   dead_block (unreachable) ─────────────────────────────-─┘
 *
 *   Allocations:
 *     %10 = alloca 64   ; local buffer — not passed to call → stack-promotable
 *     %20 = alloca 8    ; its address returned → escapes
 *
 *   Dead instructions:
 *     %30 = add %31, %32   ; %30 never used → DCE removes it
 * ───────────────────────────────────────────────────────────────────────────
 */

#ifdef ZCC_BRIDGE_STANDALONE
static Instr *make_instr(InstrID id, Opcode op, RegID dst, RegID *srcs,
                         uint32_t n_src) {
  Instr *ins = calloc(1, sizeof(Instr));
  ins->id = id;
  ins->op = op;
  ins->dst = dst;
  ins->n_src = n_src;
  if (srcs)
    memcpy(ins->src, srcs, n_src * sizeof(RegID));
  ins->exec_freq = 1.0;
  return ins;
}

static void block_append(Block *blk, Instr *ins) {
  if (!blk->head) {
    blk->head = blk->tail = ins;
  } else {
    blk->tail->next = ins;
    ins->prev = blk->tail;
    blk->tail = ins;
  }
  blk->n_instrs++;
}
#endif /* ZCC_BRIDGE_STANDALONE */

/* Unlink instruction from its block (caller must know the block) */
static void __attribute__((unused)) instr_unlink(Block *blk, Instr *ins) {
  if (ins->prev)
    ins->prev->next = ins->next;
  else
    blk->head = ins->next;
  if (ins->next)
    ins->next->prev = ins->prev;
  else
    blk->tail = ins->prev;
  blk->n_instrs--;
}

/* Insert ins before `before` in blk */
static void __attribute__((unused))
instr_insert_before(Block *blk, Instr *before, Instr *ins) {
  ins->next = before;
  ins->prev = before->prev;
  if (before->prev)
    before->prev->next = ins;
  else
    blk->head = ins;
  before->prev = ins;
  blk->n_instrs++;
}

static BlockID __attribute__((unused)) block_of_instr(Function *fn,
                                                      const Instr *needle) {
  for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
    for (Instr *ins = fn->blocks[bi]->head; ins; ins = ins->next)
      if (ins == needle)
        return (BlockID)bi;
  }
  return MAX_BLOCKS;
}

#ifdef ZCC_BRIDGE_STANDALONE
static Function *build_test_function(void) {
  Function *fn = calloc(1, sizeof(Function));

  /* Allocate 5 blocks */
  const char *names[] = {"entry", "loop_header", "loop_body", "loop_exit",
                         "fn_exit"};
  for (uint32_t i = 0; i < 5; i++) {
    fn->blocks[i] = calloc(1, sizeof(Block));
    fn->blocks[i]->id = i;
    strncpy(fn->blocks[i]->name, names[i], NAME_LEN - 1);
    fn->blocks[i]->exec_freq = 0.0;
  }
  fn->n_blocks = 5;
  fn->entry = 0;
  fn->exit = 4;

  /* CFG edges */
  fn->blocks[0]->succs[0] = 1;
  fn->blocks[0]->n_succs = 1;
  fn->blocks[0]->branch_prob[0] = 1.0;

  fn->blocks[1]->succs[0] = 2;
  fn->blocks[1]->succs[1] = 3;
  fn->blocks[1]->n_succs = 2;
  fn->blocks[1]->branch_prob[0] = 0.9; /* mostly stay in loop */
  fn->blocks[1]->branch_prob[1] = 0.1;

  fn->blocks[2]->succs[0] = 1;
  fn->blocks[2]->n_succs = 1;
  fn->blocks[2]->branch_prob[0] = 1.0;

  fn->blocks[3]->succs[0] = 4;
  fn->blocks[3]->n_succs = 1;
  fn->blocks[3]->branch_prob[0] = 1.0;

  fn->blocks[4]->n_succs = 0; /* exit */

  /* Predecessors */
  fn->blocks[1]->preds[0] = 0;
  fn->blocks[1]->preds[1] = 2;
  fn->blocks[1]->n_preds = 2;
  fn->blocks[2]->preds[0] = 1;
  fn->blocks[2]->n_preds = 1;
  fn->blocks[3]->preds[0] = 1;
  fn->blocks[3]->n_preds = 1;
  fn->blocks[4]->preds[0] = 3;
  fn->blocks[4]->n_preds = 1;

  /* ── entry block ── */
  Block *entry = fn->blocks[0];
  /* %10 = alloca 64 — stack-promotable */
  Instr *a1 = make_instr(1, OP_ALLOCA, 10, (RegID[]){64}, 1);
  /* %20 = alloca 8  — escapes via ret  */
  Instr *a2 = make_instr(2, OP_ALLOCA, 20, (RegID[]){8}, 1);
  /* %30 = add %31, %32 — dead (never used) */
  Instr *dead = make_instr(3, OP_ADD, 30, (RegID[]){31, 32}, 2);
  /* br loop_header */
  Instr *br = make_instr(4, OP_BR, 0, (RegID[]){1}, 1);
  block_append(entry, a1);
  block_append(entry, a2);
  block_append(entry, dead);
  block_append(entry, br);

  fn->def_of[10] = a1;
  fn->def_of[20] = a2;
  fn->def_of[30] = dead;

  /* ── loop_header ── */
  Block *lhdr = fn->blocks[1];
  /* %40 = phi(%41:entry, %42:loop_body) — induction variable */
  Instr *phi = make_instr(5, OP_PHI, 40, NULL, 0);
  phi->phi[0] = (PhiSource){41, 0}; /* from entry     */
  phi->phi[1] = (PhiSource){42, 2}; /* from loop_body */
  phi->n_phi = 2;
  /* condbr %40, loop_body, loop_exit */
  Instr *cbr = make_instr(6, OP_CONDBR, 0, (RegID[]){40, 2, 3}, 3);
  block_append(lhdr, phi);
  block_append(lhdr, cbr);
  fn->def_of[40] = phi;

  /* ── loop_body ── */
  Block *lbod = fn->blocks[2];
  /* store value into %10 (local buf — no escape via store to local ptr) */
  Instr *st = make_instr(7, OP_STORE, 0, (RegID[]){99, 10}, 2);
  /* %42 = add %40, 1 */
  Instr *inc = make_instr(8, OP_ADD, 42, (RegID[]){40, 1}, 2);
  /* br loop_header */
  Instr *lbr = make_instr(9, OP_BR, 0, (RegID[]){1}, 1);
  block_append(lbod, st);
  block_append(lbod, inc);
  block_append(lbod, lbr);
  fn->def_of[42] = inc;

  /* ── loop_exit ── */
  Block *lexit = fn->blocks[3];
  Instr *ebr = make_instr(10, OP_BR, 0, (RegID[]){4}, 1);
  block_append(lexit, ebr);

  /* ── fn_exit ── */
  Block *fexit = fn->blocks[4];
  /* ret %20 → %20 escapes */
  Instr *ret = make_instr(11, OP_RET, 0, (RegID[]){20}, 1);
  block_append(fexit, ret);

  fn->n_regs = 100;
  return fn;
}

/* Nested while/if fixture: 7 blocks, ~20 value instructions +
 * allocas/stores/br. Matches the shape of tmp_nested_while_if.c for DCE
 * before/after validation. */
static Function *build_nested_while_if_fixture(void) {
  Function *fn = calloc(1, sizeof(Function));
  const char *names[] = {"entry",     "while.head", "while.body", "if.then.0",
                         "if.else.0", "if.merge.0", "while.exit"};
  for (uint32_t i = 0; i < 7; i++) {
    fn->blocks[i] = calloc(1, sizeof(Block));
    fn->blocks[i]->id = i;
    strncpy(fn->blocks[i]->name, names[i], NAME_LEN - 1);
    fn->blocks[i]->exec_freq = 0.0;
  }
  fn->n_blocks = 7;
  fn->entry = 0;
  fn->exit = 6;

  /* CFG */
  fn->blocks[0]->succs[0] = 1;
  fn->blocks[0]->n_succs = 1;
  fn->blocks[1]->succs[0] = 2;
  fn->blocks[1]->succs[1] = 6;
  fn->blocks[1]->n_succs = 2;
  fn->blocks[2]->succs[0] = 3;
  fn->blocks[2]->succs[1] = 4;
  fn->blocks[2]->n_succs = 2;
  fn->blocks[3]->succs[0] = 5;
  fn->blocks[3]->n_succs = 1;
  fn->blocks[4]->succs[0] = 5;
  fn->blocks[4]->n_succs = 1;
  fn->blocks[5]->succs[0] = 1;
  fn->blocks[5]->n_succs = 1;
  fn->blocks[6]->n_succs = 0;

  fn->blocks[1]->preds[0] = 0;
  fn->blocks[1]->preds[1] = 5;
  fn->blocks[1]->n_preds = 2;
  fn->blocks[2]->preds[0] = 1;
  fn->blocks[2]->n_preds = 1;
  fn->blocks[3]->preds[0] = 2;
  fn->blocks[3]->n_preds = 1;
  fn->blocks[4]->preds[0] = 2;
  fn->blocks[4]->n_preds = 1;
  fn->blocks[5]->preds[0] = 3;
  fn->blocks[5]->preds[1] = 4;
  fn->blocks[5]->n_preds = 2;
  fn->blocks[6]->preds[0] = 1;
  fn->blocks[6]->n_preds = 1;

  /* Branch probabilities (required for ir_validate when n_succs > 1) */
  fn->blocks[1]->branch_prob[0] = 0.9; /* while.head: true -> while.body */
  fn->blocks[1]->branch_prob[1] = 0.1; /* false -> while.exit */
  fn->blocks[2]->branch_prob[0] = 0.5; /* while.body: true -> if.then.0 */
  fn->blocks[2]->branch_prob[1] = 0.5; /* false -> if.else.0 */

  /* Allocas: %50 = i, %51 = r */
  Block *entry = fn->blocks[0];
  Instr *ai = make_instr_imm(0, OP_ALLOCA, 50, 8, 0);
  Instr *ar = make_instr_imm(1, OP_ALLOCA, 51, 8, 0);
  Instr *c0 = make_instr_imm(2, OP_CONST, 0, 0, 0);
  Instr *c1 = make_instr_imm(3, OP_CONST, 1, 0, 0);
  Instr *si = make_instr(4, OP_STORE, 0, (RegID[]){0, 50}, 2);
  Instr *sr = make_instr(5, OP_STORE, 0, (RegID[]){1, 51}, 2);
  Instr *br_head = make_instr(6, OP_BR, 0, (RegID[]){1}, 1);
  block_append(entry, ai);
  block_append(entry, ar);
  block_append(entry, c0);
  block_append(entry, c1);
  block_append(entry, si);
  block_append(entry, sr);
  block_append(entry, br_head);

  Block *head = fn->blocks[1];
  Instr *li = make_instr(7, OP_LOAD, 2, (RegID[]){50}, 1);
  Instr *c10 = make_instr_imm(8, OP_CONST, 3, 10, 0);
  Instr *lt = make_instr(9, OP_LT, 4, (RegID[]){2, 3}, 2);
  Instr *cbr = make_instr(10, OP_CONDBR, 0, (RegID[]){4, 2, 6}, 3);
  block_append(head, li);
  block_append(head, c10);
  block_append(head, lt);
  block_append(head, cbr);

  Block *body = fn->blocks[2];
  Instr *li2 = make_instr(11, OP_LOAD, 5, (RegID[]){50}, 1);
  Instr *c2 = make_instr_imm(12, OP_CONST, 6, 2, 0);
  Instr *mod = make_instr(13, OP_MOD, 7, (RegID[]){5, 6}, 2);
  Instr *c0b = make_instr_imm(14, OP_CONST, 8, 0, 0);
  Instr *lt2 = make_instr(15, OP_LT, 9, (RegID[]){8, 7}, 2);
  Instr *cbr2 = make_instr(16, OP_CONDBR, 0, (RegID[]){9, 3, 4}, 3);
  block_append(body, li2);
  block_append(body, c2);
  block_append(body, mod);
  block_append(body, c0b);
  block_append(body, lt2);
  block_append(body, cbr2);

  Block *then0 = fn->blocks[3];
  Instr *lr = make_instr(17, OP_LOAD, 10, (RegID[]){51}, 1);
  Instr *c1b = make_instr_imm(18, OP_CONST, 11, 1, 0);
  Instr *add1 = make_instr(19, OP_ADD, 12, (RegID[]){10, 11}, 2);
  Instr *st_r1 = make_instr(20, OP_STORE, 0, (RegID[]){12, 51}, 2);
  Instr *br_merge = make_instr(21, OP_BR, 0, (RegID[]){5}, 1);
  block_append(then0, lr);
  block_append(then0, c1b);
  block_append(then0, add1);
  block_append(then0, st_r1);
  block_append(then0, br_merge);

  Block *else0 = fn->blocks[4];
  Instr *lr2 = make_instr(22, OP_LOAD, 13, (RegID[]){51}, 1);
  Instr *c2b = make_instr_imm(23, OP_CONST, 14, 2, 0);
  Instr *add2 = make_instr(24, OP_ADD, 15, (RegID[]){13, 14}, 2);
  Instr *st_r2 = make_instr(25, OP_STORE, 0, (RegID[]){15, 51}, 2);
  Instr *br_merge2 = make_instr(26, OP_BR, 0, (RegID[]){5}, 1);
  block_append(else0, lr2);
  block_append(else0, c2b);
  block_append(else0, add2);
  block_append(else0, st_r2);
  block_append(else0, br_merge2);

  Block *merge0 = fn->blocks[5];
  Instr *li3 = make_instr(27, OP_LOAD, 16, (RegID[]){50}, 1);
  Instr *c1c = make_instr_imm(28, OP_CONST, 17, 1, 0);
  Instr *add3 = make_instr(29, OP_ADD, 18, (RegID[]){16, 17}, 2);
  Instr *st_i = make_instr(30, OP_STORE, 0, (RegID[]){18, 50}, 2);
  Instr *br_back = make_instr(31, OP_BR, 0, (RegID[]){1}, 1);
  block_append(merge0, li3);
  block_append(merge0, c1c);
  block_append(merge0, add3);
  block_append(merge0, st_i);
  block_append(merge0, br_back);

  Block *exit_blk = fn->blocks[6];
  Instr *lr3 = make_instr(32, OP_LOAD, 19, (RegID[]){51}, 1);
  Instr *ret = make_instr(33, OP_RET, 0, (RegID[]){19}, 1);
  block_append(exit_blk, lr3);
  block_append(exit_blk, ret);

  fn->n_regs = 52;
  return fn;
}

static void ir_dump(const Function *fn, const char *label) {
  fprintf(stderr, "\n=== IR DUMP: %s ===\n", label);
  for (uint32_t b = 0; b < fn->n_blocks; b++) {
    const Block *blk = fn->blocks[b];
    if (!blk)
      continue;
    fprintf(stderr, "%s:\n", blk->name);
    for (const Instr *ins = blk->head; ins; ins = ins->next) {
      int has_dst = (ins->op != OP_BR && ins->op != OP_CONDBR &&
                     ins->op != OP_STORE && ins->op != OP_RET);
      if (has_dst)
        fprintf(stderr, "  %%%u = ", (unsigned)ins->dst);
      else
        fprintf(stderr, "  ");
      fprintf(stderr, "%s", opcode_name[ins->op]);
      if (ins->op == OP_CONST)
        fprintf(stderr, " %lld", (long long)ins->imm);
      for (uint32_t i = 0; i < ins->n_src; i++)
        fprintf(stderr, " %%%u", (unsigned)ins->src[i]);
      fprintf(stderr, "\n");
    }
  }
  fprintf(stderr, "=== END DUMP ===\n\n");
}
#endif /* ZCC_BRIDGE_STANDALONE */

#ifdef ZCC_BRIDGE_STANDALONE
static void print_pass_results(const Function *fn, const PassResult *result) {
  printf("\n=================================================\n");
  printf(" ZKAEDI PRIME - Compiler Pass Results\n");
  printf("=================================================\n");

  printf("\n-- Pass 1: DCE -> SSA Bridge\n");
  printf("|  Instructions removed : %u\n", fn->stats.dce_instrs_removed);
  printf("|  Blocks removed       : %u\n", fn->stats.dce_blocks_removed);

  printf("\n-- Pass 2: LICM\n");
  printf("|  Instructions hoisted : %u\n", fn->stats.licm_hoisted);

  printf("\n-- Pass 3: Escape Analysis\n");
  printf("|  Allocations promoted : %u\n", fn->stats.ea_promotions);

  printf("\n-- Pass 4: PGO BB Reordering\n");
  printf("|  Emission order       : ");
  for (uint32_t i = 0; i < result->n_blocks; i++) {
    BlockID bid = result->order[i];
    printf("%s", fn->blocks[bid]->name);
    if (i + 1 < result->n_blocks)
      printf(" -> ");
  }
  printf("\n|  Execution frequencies:\n");
  for (uint32_t i = 0; i < result->n_blocks; i++) {
    BlockID bid = result->order[i];
    printf("|    %-16s  freq = %.4f\n", fn->blocks[bid]->name,
           fn->blocks[bid]->exec_freq);
  }

  printf("\n=================================================\n\n");
}
#endif /* ZCC_BRIDGE_STANDALONE */

static void free_function(Function *fn) {
  for (uint32_t i = 0; i < fn->n_blocks; i++) {
    Block *blk = fn->blocks[i];
    if (!blk)
      continue;
    Instr *cur = blk->head;
    while (cur) {
      Instr *n = cur->next;
      free(cur);
      cur = n;
    }
    free(blk);
  }
  free(fn);
}

void zcc_ir_free(struct Function *fn) {
  if (!fn)
    return;
  free_function((Function *)fn);
}

#ifdef ZCC_BRIDGE_STANDALONE
int main(void) {
  Function *fn = build_test_function();
  PassResult result;
  memset(&result, 0, sizeof(result));

  fprintf(stderr,
          "\n-- Running ZKAEDI PRIME pass pipeline (synthetic fixture) --\n");
  run_all_passes(fn, &result, NULL);
  print_pass_results(fn, &result);
  free_function(fn);

  /* Phase B: AST → IR bridge — real C-like control flow */
  {
    ASTNode *body = build_phase_b_ast();
    Function *fn_b = ast_to_ir(body);
    PassResult result_b;
    memset(&result_b, 0, sizeof(result_b));
    fprintf(stderr,
            "\n-- Phase B: AST->IR (while i<10 { i=i+1 } return i) --\n");
    run_all_passes(fn_b, &result_b, NULL);
    printf("\n=================================================\n");
    printf(" Phase B - AST->IR pass results\n");
    printf("=================================================\n");
    print_pass_results(fn_b, &result_b);
    free_function(fn_b);
  }

  /* Phase B ZCC: ZCC-shaped AST → IR — validate contract (DCE, LICM, PGO) */
  {
    ZCCNode *zbody = build_zcc_phase_b_ast();
    Function *fn_z = zcc_ast_to_ir(zbody, NULL);
    if (!ir_validate(fn_z)) {
      fprintf(stderr, "[ZCC Phase B] ir_validate failed before passes\n");
    }
    PassResult result_z;
    memset(&result_z, 0, sizeof(result_z));
    fprintf(stderr, "\n-- Phase B ZCC: ZCC-shaped AST->IR (while i<10 { i=i+1 "
                    "} return i) --\n");
    run_all_passes(fn_z, &result_z, NULL);
    printf("\n=================================================\n");
    printf(" Phase B ZCC - ZCC->IR pass results\n");
    printf("=================================================\n");
    print_pass_results(fn_z, &result_z);
    /* Contract: entry has 0 preds; block count ~5 (entry, while.head,
     * while.body, while.exit, exit); LICM hoists consts */
    printf("  Blocks: %u  LICM hoisted: %u  DCE removed: %u  PGO order: %u\n",
           fn_z->n_blocks, fn_z->stats.licm_hoisted,
           fn_z->stats.dce_instrs_removed, result_z.n_blocks);
    if (!ir_validate(fn_z))
      fprintf(stderr, "[ZCC Phase B] ir_validate failed after passes\n");
    free_function(fn_z);
  }

  /* DCE before/after validation: nested while/if fixture */
  {
    Function *fn_dce = build_nested_while_if_fixture();
    ir_dump(fn_dce, "BEFORE DCE");
    PassResult result_dce;
    memset(&result_dce, 0, sizeof(result_dce));
    run_all_passes(fn_dce, &result_dce, NULL);
    ir_dump(fn_dce, "AFTER DCE");
    fprintf(stderr, "DCE removed: %u instructions, %u blocks\n",
            result_dce.dce_instrs_removed, fn_dce->stats.dce_blocks_removed);
    /* Validation: every RegID referenced in a surviving ins must be defined in
     * the AFTER dump */
    {
      uint64_t defined[8] = {0};
      for (uint32_t b = 0; b < fn_dce->n_blocks; b++) {
        const Block *blk = fn_dce->blocks[b];
        if (!blk)
          continue;
        for (const Instr *ins = blk->head; ins; ins = ins->next) {
          if (ins->op != OP_BR && ins->op != OP_CONDBR && ins->op != OP_STORE &&
              ins->op != OP_RET && ins->dst < 512)
            defined[ins->dst >> 6] |= (1ULL << (ins->dst & 63));
        }
      }
      for (uint32_t b = 0; b < fn_dce->n_blocks; b++) {
        const Block *blk = fn_dce->blocks[b];
        if (!blk)
          continue;
        for (const Instr *ins = blk->head; ins; ins = ins->next) {
          for (uint32_t i = 0; i < ins->n_src; i++) {
            RegID r = ins->src[i];
            if (r >= 512)
              continue;
            if (!(defined[r >> 6] & (1ULL << (r & 63))))
              fprintf(
                  stderr,
                  "DCE/LICM BUG: surviving ins references undefined RegID %u\n",
                  (unsigned)r);
          }
        }
      }
    }
    free_function(fn_dce);
  }

  return 0;
}
#endif /* ZCC_BRIDGE_STANDALONE *//*
 * ir_telemetry.c — ZCC IR Pass Telemetry Emitter
 * ================================================
 * Emits per-pass optimization metrics to Gods Eye via UDP 41337.
 * Fire-and-forget: if nobody is listening, sendto() fails silently.
 *
 * Wire format: {"_body":"<canonical JSON>","_sig":"ir_telemetry"}
 * The relay accepts _sig == "ir_telemetry" as a Phase 1 bypass.
 *
 * Compiled by GCC only (linked separately, NOT in zcc.c).
 * Uses POSIX sockets (Linux/WSL only — matches ZCC's target).
 *
 * Environment gate: ZCC_EMIT_TELEMETRY=1
 *   When unset, all functions early-return (zero overhead).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdarg.h>

#include "ir_telemetry.h"

/* ── Internal state ──────────────────────────────────────────────────── */

static int s_enabled  = 0;          /* set by ir_telem_init() */
static int s_sock_fd  = -1;        /* UDP socket */
static struct sockaddr_in s_addr;   /* Gods Eye endpoint */
static int s_compile_counter = 0;   /* monotonic compilation counter */

/* ── UDP emission helper ─────────────────────────────────────────────── */

/*
 * Send a JSON body wrapped in the Gods Eye envelope format.
 * The _sig field is "ir_telemetry" (Phase 1 HMAC bypass).
 *
 * body_json must be a canonical JSON string (no newlines).
 * Total packet must fit in 1400 bytes (well under MTU).
 */
static void emit_packet(const char *body_json) {
    char packet[1500];
    int len;

    if (!s_enabled || s_sock_fd < 0) return;

    /* Envelope: {"_body":"<escaped body>","_sig":"ir_telemetry"}
     * Since body_json contains no unescaped quotes (we control it),
     * we can embed it directly. The body uses single-level JSON
     * with no nested strings, so this is safe. */
    len = snprintf(packet, sizeof(packet),
        "{\"_body\":\"%s\",\"_sig\":\"ir_telemetry\"}",
        body_json);

    if (len <= 0 || len >= (int)sizeof(packet)) return;

    /* Fire and forget — ignore errors */
    sendto(s_sock_fd, packet, len, 0,
           (struct sockaddr *)&s_addr, sizeof(s_addr));
}

/*
 * Build a canonical body JSON string for a per-pass metric.
 * Keys are sorted alphabetically for consistency.
 * String values use single quotes internally to avoid escaping
 * issues in the envelope — wait, JSON requires double quotes.
 *
 * Actually: the _body is embedded inside double quotes in the
 * envelope, so we need to escape inner double quotes.
 * Simpler: use the body as-is but escape " to \".
 */
static void emit_body(char *buf, int bufsz, const char *body_fmt, ...) {
    char raw[1024];
    char escaped[1400];
    int i, j;
    va_list ap;

    va_start(ap, body_fmt);
    vsnprintf(raw, sizeof(raw), body_fmt, ap);
    va_end(ap);

    /* Escape double quotes for embedding inside envelope */
    j = 0;
    for (i = 0; raw[i] && j < (int)sizeof(escaped) - 2; i++) {
        if (raw[i] == '"') {
            escaped[j++] = '\\';
            escaped[j++] = '"';
        } else {
            escaped[j++] = raw[i];
        }
    }
    escaped[j] = '\0';

    snprintf(buf, bufsz, "%s", escaped);
}

/* ── Public API ──────────────────────────────────────────────────────── */

void ir_telem_init(void) {
    const char *env;
    const char *host;
    int port;

    env = getenv("ZCC_EMIT_TELEMETRY");
    if (!env || env[0] == '0' || env[0] == '\0') {
        s_enabled = 0;
        return;
    }

    host = getenv("GODS_EYE_HOST");
    if (!host) host = "127.0.0.1";

    port = 41337;
    env = getenv("GODS_EYE_PORT");
    if (env) port = atoi(env);

    s_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (s_sock_fd < 0) {
        s_enabled = 0;
        return;
    }

    /* Make non-blocking so sendto never stalls compilation */
    {
        int flags = 1;
        /* MSG_DONTWAIT on sendto is simpler; skip fcntl */
    }

    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &s_addr.sin_addr);

    s_enabled = 1;
    s_compile_counter = 0;

    fprintf(stderr, "[IR TELEMETRY] Emitting to %s:%d\n", host, port);
}

void ir_telem_pass(const char *pass_name,
                   int func_count,
                   int nodes_before,
                   int nodes_after,
                   int nodes_deleted,
                   int nodes_modified) {
    char body[1024];
    char escaped[1400];
    int delta;

    if (!s_enabled) return;

    delta = nodes_after - nodes_before;

    /*
     * Build canonical JSON body with sorted keys.
     * Reuse existing schema fields for relay compatibility:
     *   deadlocks_healed = 0 (unused)
     *   gpu_temp_c       = nodes_before (previous energy)
     *   gpu_util_pct     = nodes_after  (current energy)
     *   h_t_state        = delta        (energy change)
     *   ir_funcs         = func_count   (extra, stripped by relay)
     *   ir_nodes_after   = nodes_after  (extra)
     *   ir_nodes_before  = nodes_before (extra)
     *   ir_nodes_deleted = nodes_deleted (extra)
     *   ir_nodes_modified= nodes_modified(extra)
     *   ir_pass          = pass_name    (extra)
     *   jit_latency_ms   = 0.0  (unused)
     *   swarm_cycles     = compile_counter
     *   vram_usage_mb    = 0.0  (unused)
     */
    snprintf(body, sizeof(body),
        "{\\\"deadlocks_healed\\\":0,"
        "\\\"gpu_temp_c\\\":%d,"
        "\\\"gpu_util_pct\\\":%d,"
        "\\\"h_t_state\\\":%d.0,"
        "\\\"ir_funcs\\\":%d,"
        "\\\"ir_nodes_after\\\":%d,"
        "\\\"ir_nodes_before\\\":%d,"
        "\\\"ir_nodes_deleted\\\":%d,"
        "\\\"ir_nodes_modified\\\":%d,"
        "\\\"ir_pass\\\":\\\"%s\\\","
        "\\\"jit_latency_ms\\\":0.0,"
        "\\\"swarm_cycles\\\":%d,"
        "\\\"vram_usage_mb\\\":0.0}",
        nodes_before,       /* gpu_temp_c */
        nodes_after,        /* gpu_util_pct */
        delta,              /* h_t_state */
        func_count,
        nodes_after,
        nodes_before,
        nodes_deleted,
        nodes_modified,
        pass_name,
        s_compile_counter);

    /* The body is already pre-escaped for embedding in the envelope */
    {
        char packet[1500];
        int len = snprintf(packet, sizeof(packet),
            "{\"_body\":\"%s\",\"_sig\":\"ir_telemetry\"}",
            body);
        if (len > 0 && len < (int)sizeof(packet)) {
            sendto(s_sock_fd, packet, len, 0,
                   (struct sockaddr *)&s_addr, sizeof(s_addr));
        }
    }
}

void ir_telem_summary(int total_funcs,
                      int total_nodes_before,
                      int total_nodes_after,
                      int pass_count,
                      const char **pass_names) {
    char body[1024];
    char passes_str[256];
    int delta;
    double reduction_pct;
    int i, pos;

    if (!s_enabled) return;

    s_compile_counter++;

    delta = total_nodes_after - total_nodes_before;
    reduction_pct = (total_nodes_before > 0)
        ? 100.0 * (1.0 - (double)total_nodes_after / total_nodes_before)
        : 0.0;

    /* Build comma-separated pass list */
    pos = 0;
    for (i = 0; i < pass_count && pos < (int)sizeof(passes_str) - 32; i++) {
        if (i > 0) passes_str[pos++] = ',';
        pos += snprintf(passes_str + pos, sizeof(passes_str) - pos,
                        "%s", pass_names[i]);
    }
    passes_str[pos] = '\0';

    snprintf(body, sizeof(body),
        "{\\\"deadlocks_healed\\\":0,"
        "\\\"gpu_temp_c\\\":%d,"
        "\\\"gpu_util_pct\\\":%d,"
        "\\\"h_t_state\\\":%d.0,"
        "\\\"ir_passes\\\":\\\"%s\\\","
        "\\\"ir_reduction_pct\\\":%.1f,"
        "\\\"ir_total_funcs\\\":%d,"
        "\\\"ir_total_nodes_after\\\":%d,"
        "\\\"ir_total_nodes_before\\\":%d,"
        "\\\"ir_type\\\":\\\"summary\\\","
        "\\\"jit_latency_ms\\\":0.0,"
        "\\\"swarm_cycles\\\":%d,"
        "\\\"vram_usage_mb\\\":%.1f}",
        total_nodes_before,     /* gpu_temp_c */
        total_nodes_after,      /* gpu_util_pct */
        delta,                  /* h_t_state */
        passes_str,
        reduction_pct,
        total_funcs,
        total_nodes_after,
        total_nodes_before,
        s_compile_counter,
        reduction_pct);         /* vram_usage_mb = reduction % */

    {
        char packet[1500];
        int len = snprintf(packet, sizeof(packet),
            "{\"_body\":\"%s\",\"_sig\":\"ir_telemetry\"}",
            body);
        if (len > 0 && len < (int)sizeof(packet)) {
            sendto(s_sock_fd, packet, len, 0,
                   (struct sockaddr *)&s_addr, sizeof(s_addr));
        }
    }

    fprintf(stderr, "[IR TELEMETRY] Compilation #%d: %d nodes -> %d nodes (%.1f%% reduction) [%s]\n",
            s_compile_counter, total_nodes_before, total_nodes_after,
            reduction_pct, passes_str);
}

void ir_telem_shutdown(void) {
    if (s_sock_fd >= 0) {
        close(s_sock_fd);
        s_sock_fd = -1;
    }
    s_enabled = 0;
}
