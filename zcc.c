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
    TK_VOLATILE, TK_AUTO, TK_REGISTER, TK_INLINE,
    TK_BUILTIN_VA_ARG,
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
    ND_NOP
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
};

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
/* part0_pp.c -- ZCC Preprocessor */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define PP_MAX_MACROS 4096
#define PP_MAX_PARAMS 16
#define PP_MAX_BODY   4096
#define PP_MAX_INCLUDE_DEPTH 32

typedef struct {
    char name[128];
    int  is_function_like;
    int  num_params;
    char params[PP_MAX_PARAMS][64];
    char body[PP_MAX_BODY];
    int  active;
} PPMacro;

typedef struct {
    PPMacro macros[PP_MAX_MACROS];
    int     num_macros;

    int     cond_stack[64];
    int     cond_else_seen[64];
    int     cond_depth;

    char    included_files[256][256];
    int     num_included;

    char   *out;
    int     out_len;
    int     out_cap;

    const char *src;
    int         pos;
    int         len;
    int         line;
    const char *filename;
    const char *include_paths;
} PPState;

static const char *zcc_stddef_text = 
"#ifndef _ZCC_STDDEF_H_\n"
"#define _ZCC_STDDEF_H_\n"
"#define NULL ((void*)0)\n"
"#define EOF (-1)\n"
"#define EXIT_SUCCESS 0\n"
"#define EXIT_FAILURE 1\n"
"#define CHAR_BIT 8\n"
"#define INT_MAX 2147483647\n"
"#define INT_MIN (-2147483647-1)\n"
"#define LONG_MAX 9223372036854775807L\n"
"#define LONG_MIN (-9223372036854775807L-1L)\n"
"#define UINT_MAX 4294967295U\n"
"#define ULONG_MAX 18446744073709551615UL\n"
"#define SIZE_MAX ULONG_MAX\n"
"#define RAND_MAX 2147483647\n"
"#define BUFSIZ 8192\n"
"#define FILENAME_MAX 4096\n"
"#define true 1\n"
"#define false 0\n"
"#define __attribute__(x)\n"
"#define __extension__\n"
"#define __restrict\n"
"#define __restrict__\n"
"#define restrict\n"
"#define __inline\n"
"#define __inline__\n"
"#define inline\n"
"#define __volatile\n"
"#define __volatile__\n"
"#define __const__ const\n"
"#define __const const\n"
"#define _Noreturn\n"
"#define _Static_assert(x, y)\n"
"#define __declspec(x)\n"
"#define __cdecl\n"
"#define __stdcall\n"
"#define __asm__(a,b,c,d,e)\n"
"#define asm(a,b,c,d,e)\n"
"typedef struct { unsigned int gp_offset; unsigned int fp_offset; void *overflow_arg_area; void *reg_save_area; } __va_list_struct[1];\n"
"#define __builtin_va_list __va_list_struct\n"
"#define __builtin_va_end(v)\n"
"#define __builtin_expect(exp, c) (exp)\n"
"#define __builtin_constant_p(x) 0\n"
"#define __builtin_types_compatible_p(x, y) 0\n"
"#define __builtin_unreachable()\n"
"#define offsetof(t, m) ((unsigned long)&(((t*)0)->m))\n"
"typedef int int32_t;\n"
"typedef unsigned int uint32_t;\n"
"typedef long int64_t;\n"
"typedef unsigned long uint64_t;\n"
"typedef char int8_t;\n"
"typedef unsigned char uint8_t;\n"
"typedef short int16_t;\n"
"typedef unsigned short uint16_t;\n"
"typedef unsigned long size_t;\n"
"typedef long ssize_t;\n"
"typedef struct _IO_FILE FILE;\n"
"extern FILE *stdin, *stdout, *stderr;\n"
"#endif\n";

static void pp_emit(PPState *state, char c) {
    if (state->out_len + 1 >= state->out_cap) {
        state->out_cap = (state->out_cap == 0) ? 4096 : state->out_cap * 2;
        state->out = (char *)realloc(state->out, state->out_cap);
    }
    state->out[state->out_len++] = c;
}

static void pp_emit_str(PPState *state, const char *str, int len) {
    int i;
    for (i = 0; i < len; i++) pp_emit(state, str[i]);
}

static char pp_peek(PPState *state) {
    if (state->pos >= state->len) return 0;
    return state->src[state->pos];
}

static char pp_next(PPState *state) {
    char c = pp_peek(state);
    if (c) {
        if (c == '\n') state->line++;
        state->pos++;
    }
    return c;
}

static void pp_skip_whitespace(PPState *state) {
    while (pp_peek(state) == ' ' || pp_peek(state) == '\t') pp_next(state);
}

static int is_ident_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int is_ident_char(char c) {
    return is_ident_start(c) || (c >= '0' && c <= '9');
}

static void pp_parse_ident(PPState *state, char *out, int max) {
    int i = 0;
    while (is_ident_char(pp_peek(state)) && i < max - 1) out[i++] = pp_next(state);
    out[i] = 0;
}

static PPMacro *pp_find_macro(PPState *state, const char *name) {
    int i;
    for (i = state->num_macros - 1; i >= 0; i--) {
        if (state->macros[i].active && strcmp(state->macros[i].name, name) == 0) return &state->macros[i];
    }
    return 0;
}

static void pp_undef_macro(PPState *state, const char *name) {
    PPMacro *m = pp_find_macro(state, name);
    if (m) m->active = 0;
}

static PPMacro *pp_add_macro(PPState *state, const char *name) {
    PPMacro *m_macro;
    if (state->num_macros >= PP_MAX_MACROS) return 0;
    m_macro = &state->macros[state->num_macros++];
    memset(m_macro, 0, sizeof(PPMacro));
    strncpy(m_macro->name, name, 127);
    m_macro->active = 1;
    return m_macro;
}

static void pp_read_line(PPState *state, char *buf, int max) {
    int i = 0;
    while (pp_peek(state) != '\n' && pp_peek(state) != 0 && i < max - 1) {
        char c = pp_next(state);
        if (c == '\\' && pp_peek(state) == '\n') {
            pp_next(state);
            continue;
        }
        buf[i++] = c;
    }
    buf[i] = 0;
}

static int pp_is_active(PPState *state) {
    int i;
    for (i = 0; i <= state->cond_depth; i++) {
        if (!state->cond_stack[i]) return 0;
    }
    return 1;
}

static char *pp_read_file(const char *path, int *out_len) {
    FILE *fp = fopen(path, "rb");
    long len;
    char *buf;
    long nr;
    
    if (!fp) return 0;
    fseek(fp, 0, 2);
    len = ftell(fp);
    fseek(fp, 0, 0);
    
    buf = (char *)malloc(len + 1);
    if (!buf) { fclose(fp); return 0; }
    
    nr = fread(buf, 1, len, fp);
    buf[nr] = 0;
    fclose(fp);
    
    if (out_len) *out_len = nr;
    return buf;
}

static void pp_parse(PPState *state);

static void pp_process_include(PPState *state, const char *path, int is_system) {
    char *file_src = 0;
    int file_len = 0;
    int i;
    
    /* Avoid system includes entirely and just inject stddef */
    if (is_system || strcmp(path, "stdio.h") == 0 || strcmp(path, "stdlib.h") == 0 || 
        strcmp(path, "string.h") == 0 || strcmp(path, "stddef.h") == 0) {
        file_src = (char *)zcc_stddef_text;
        file_len = strlen(file_src);
    } else {
        /* check circle include logic (very simple, limit depth) */
        void *p = pp_read_file(path, &file_len);
        if (p) file_src = (char *)p;
    }
    
    if (file_src) {
        PPState *sub_state = (PPState *)malloc(sizeof(PPState));
        if (!sub_state) {
            fprintf(stderr, "zcc preprocessor error: out of memory for include state\\n");
            exit(1);
        }
        memset(sub_state, 0, sizeof(PPState));
        sub_state->src = file_src;
        sub_state->len = file_len;
        sub_state->cond_stack[0] = 1;
        
        /* copy macro table into sub_state */
        sub_state->num_macros = state->num_macros;
        for (i = 0; i < state->num_macros; i++) memcpy(&sub_state->macros[i], &state->macros[i], sizeof(PPMacro));
        
        pp_parse(sub_state);
        
        /* we need to copy back the macros added by the header */
        for (i = state->num_macros; i < sub_state->num_macros; i++) {
            if (state->num_macros >= PP_MAX_MACROS) break;
            memcpy(&state->macros[state->num_macros++], &sub_state->macros[i], sizeof(PPMacro));
        }
        
        /* update all previous active flags because undefs may have occurred */
        for (i = 0; i < state->num_macros; i++) {
            state->macros[i].active = sub_state->macros[i].active;
        }
        
        pp_emit_str(state, sub_state->out, sub_state->out_len);
        
        if (sub_state->out) free(sub_state->out);
        free(sub_state);
        if (file_src != zcc_stddef_text) free(file_src);
    } else {
        fprintf(stderr, "zcc compiler error: include file not found: %s\n", path);
    }
}

/* parse parameter list for function-like macros */
static void pp_parse_params(PPState *state, PPMacro *m) {
    pp_skip_whitespace(state);
    while (pp_peek(state) != ')' && pp_peek(state) != '\n' && pp_peek(state) != 0) {
        if (is_ident_start(pp_peek(state))) {
            if (m->num_params < PP_MAX_PARAMS) {
                pp_parse_ident(state, m->params[m->num_params++], 64);
            } else {
                char dummy[64];
                pp_parse_ident(state, dummy, 64);
            }
        } else {
            pp_next(state); /* comma or whitespace */
        }
        pp_skip_whitespace(state);
    }
    if (pp_peek(state) == ')') pp_next(state);
}

static void pp_parse_directive(PPState *state) {
    char dir[64];
    int active = pp_is_active(state);
    
    pp_skip_whitespace(state);
    pp_parse_ident(state, dir, 64);
    pp_skip_whitespace(state);

    if (strcmp(dir, "ifdef") == 0 || strcmp(dir, "ifndef") == 0) {
        char name[128];
        int is_defined;
        pp_parse_ident(state, name, 128);
        is_defined = (pp_find_macro(state, name) != 0);
        
        state->cond_depth++;
        if (!active) {
            state->cond_stack[state->cond_depth] = 0;
        } else {
            if (strcmp(dir, "ifdef") == 0) state->cond_stack[state->cond_depth] = is_defined;
            else state->cond_stack[state->cond_depth] = !is_defined;
        }
        pp_read_line(state, dir, 64);
    } else if (strcmp(dir, "else") == 0) {
        if (state->cond_depth > 0) {
            int parent_active = 1;
            int i;
            for (i = 0; i < state->cond_depth; i++) if (!state->cond_stack[i]) parent_active = 0;
            if (parent_active) state->cond_stack[state->cond_depth] = !state->cond_stack[state->cond_depth];
            else state->cond_stack[state->cond_depth] = 0;
        }
        pp_read_line(state, dir, 64);
    } else if (strcmp(dir, "endif") == 0) {
        if (state->cond_depth > 0) state->cond_depth--;
        pp_read_line(state, dir, 64);
    } else if (active) {
        if (strcmp(dir, "define") == 0) {
            char name[128];
            PPMacro *m;
            pp_parse_ident(state, name, 128);
            m = pp_add_macro(state, name);
            if (pp_peek(state) == '(') {
                pp_next(state);
                m->is_function_like = 1;
                pp_parse_params(state, m);
                pp_skip_whitespace(state);
                pp_read_line(state, m->body, PP_MAX_BODY);
            } else {
                pp_skip_whitespace(state);
                pp_read_line(state, m->body, PP_MAX_BODY);
            }
        } else if (strcmp(dir, "undef") == 0) {
            char name[128];
            pp_parse_ident(state, name, 128);
            pp_undef_macro(state, name);
            pp_read_line(state, dir, 64);
        } else if (strcmp(dir, "include") == 0) {
            char inc_path[256];
            int is_system = 0;
            if (pp_peek(state) == '<') {
                is_system = 1;
                pp_next(state);
            } else if (pp_peek(state) == '"') {
                pp_next(state);
            }
            pp_read_line(state, inc_path, 256);
            if (strlen(inc_path) > 0 && (inc_path[strlen(inc_path) - 1] == '"' || inc_path[strlen(inc_path) - 1] == '>')) {
                inc_path[strlen(inc_path) - 1] = 0;
            }
            pp_process_include(state, inc_path, is_system);
        } else if (strcmp(dir, "error") == 0) {
            char msg[256];
            pp_read_line(state, msg, 256);
            fprintf(stderr, "zcc preprocessor error: #error %s\n", msg);
        } else {
            pp_read_line(state, dir, 64);
        }
    } else {
        pp_read_line(state, dir, 64);
    }
    
    if (pp_peek(state) == '\n') pp_next(state);
    pp_emit(state, '\n');
}

static void pp_expand_ident(PPState *state, const char *ident) {
    PPMacro *m;
    int i, p_count;
    char args[PP_MAX_PARAMS][256];
    char c;
    
    if (!pp_is_active(state)) return;
    
    m = pp_find_macro(state, ident);
    if (!m) {
        pp_emit_str(state, ident, strlen(ident));
        return;
    }
    
    if (!m->is_function_like) {
        pp_emit_str(state, m->body, strlen(m->body));
        return;
    }
    
    /* Function-like macro expansion */
    memset(args, 0, sizeof(args));
    /* check if next non-whitespace is '(' */
    int saved_pos = state->pos;
    int saved_line = state->line;
    pp_skip_whitespace(state);
    if (pp_peek(state) != '(') {
        /* Not an invocation */
        state->pos = saved_pos;
        state->line = saved_line;
        pp_emit_str(state, ident, strlen(ident));
        return;
    }
    pp_next(state); /* consume '(' */
    
    p_count = 0;
    int paren_level = 0;
    int arg_idx = 0;
    while (pp_peek(state) != 0) {
        c = pp_peek(state);
        if (c == '(') paren_level++;
        else if (c == ')') {
            if (paren_level == 0) {
                pp_next(state);
                p_count++;
                break;
            }
            paren_level--;
        } else if (c == ',' && paren_level == 0) {
            p_count++;
            arg_idx = 0;
            pp_next(state);
            continue;
        }
        
        if (p_count < PP_MAX_PARAMS && arg_idx < 255) {
            args[p_count][arg_idx++] = c;
            args[p_count][arg_idx] = 0;
        }
        pp_next(state);
    }
    
    /* substitute */
    int len = strlen(m->body);
    for (i = 0; i < len; i++) {
        if (is_ident_start(m->body[i])) {
            char param_name[64];
            int t = 0;
            while (i < len && is_ident_char(m->body[i]) && t < 63) param_name[t++] = m->body[i++];
            param_name[t] = 0;
            i--; /* backup */
            
            /* See if it matches a parameter */
            int found = -1;
            int j;
            for (j = 0; j < m->num_params; j++) {
                if (strcmp(param_name, m->params[j]) == 0) { found = j; break; }
            }
            
            if (found >= 0) {
                pp_emit_str(state, args[found], strlen(args[found]));
            } else {
                pp_emit_str(state, param_name, strlen(param_name));
            }
        } else {
            pp_emit(state, m->body[i]);
        }
    }
}

static void pp_parse(PPState *state) {
    int in_string = 0;
    while (pp_peek(state) != 0) {
        char c = pp_peek(state);
        
        if (c == '"' || c == '\'') {
            in_string = c;
            if (pp_is_active(state)) pp_emit(state, pp_next(state)); else pp_next(state);
            while (pp_peek(state) != 0 && pp_peek(state) != in_string) {
                if (pp_peek(state) == '\\') {
                    if (pp_is_active(state)) pp_emit(state, pp_next(state)); else pp_next(state);
                    if (pp_peek(state) != 0) {
                        if (pp_is_active(state)) pp_emit(state, pp_next(state)); else pp_next(state);
                    }
                } else {
                    if (pp_is_active(state)) pp_emit(state, pp_next(state)); else pp_next(state);
                }
            }
            if (pp_peek(state) != 0) {
                if (pp_is_active(state)) pp_emit(state, pp_next(state)); else pp_next(state);
            }
            in_string = 0;
            continue;
        }
        if (c == '/' && state->pos + 1 < state->len) {
            char next_c = state->src[state->pos + 1];
            if (next_c == '/') {
                if (pp_is_active(state)) { pp_emit(state, pp_next(state)); pp_emit(state, pp_next(state)); }
                else { pp_next(state); pp_next(state); }
                while (pp_peek(state) != 0 && pp_peek(state) != '\n') {
                    if (pp_is_active(state)) pp_emit(state, pp_next(state)); else pp_next(state);
                }
                continue;
            } else if (next_c == '*') {
                if (pp_is_active(state)) { pp_emit(state, pp_next(state)); pp_emit(state, pp_next(state)); }
                else { pp_next(state); pp_next(state); }
                while (pp_peek(state) != 0) {
                    if (pp_peek(state) == '*' && state->pos + 1 < state->len && state->src[state->pos + 1] == '/') {
                        if (pp_is_active(state)) { pp_emit(state, pp_next(state)); pp_emit(state, pp_next(state)); }
                        else { pp_next(state); pp_next(state); }
                        break;
                    }
                    if (pp_is_active(state)) pp_emit(state, pp_next(state)); else pp_next(state);
                }
                continue;
            }
        }
        
        if (is_ident_start(c)) {
            char ident[128];
            pp_parse_ident(state, ident, 128);
            if (pp_is_active(state)) pp_expand_ident(state, ident);
            continue;
        }

        if (c == '#') {
            pp_next(state);
            pp_parse_directive(state);
            continue;
        }

        if (pp_is_active(state)) pp_emit(state, pp_next(state));
        else {
            if (c == '\n') pp_emit(state, '\n');
            pp_next(state);
        }
    }
}

char *zcc_preprocess(const char *source, int source_len,
                     const char *filename,
                     const char *include_paths,
                     int *out_len) {
    PPState *state = (PPState *)malloc(sizeof(PPState));
    char *result;
    memset(state, 0, sizeof(PPState));
    
    state->src = source;
    state->len = source_len;
    state->filename = filename;
    state->include_paths = include_paths;
    state->line = 1;
    
    state->cond_stack[0] = 1;
    
    pp_parse(state);
    
    pp_emit(state, 0);
    if (out_len) *out_len = state->out_len - 1;
    result = state->out;
    free(state);
    return result;
}

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
    {0, 0}
};

static int kw_count = 47;

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

        last_field = 0;
        offset = 0;
        max_size = 0;
        max_align = 1;

        while (cc->tk != TK_RBRACE) {
            Type *ftype;
            Type *base_ftype;
            char fname[MAX_IDENT];
            StructField *field;
            int falign;

            if (cc->tk == TK_EOF) break;

            base_ftype = parse_type(cc);
            ftype = parse_declarator(cc, base_ftype, fname);

            /* Ignore bitfield size since ZCC allocates full integers for them */
            if (cc->tk == TK_COLON) {
                next_token(cc);
                if (cc->tk == TK_NUM) next_token(cc);
            }

            field = (StructField *)cc_alloc(cc, sizeof(StructField));
            strncpy(field->name, fname, MAX_IDENT - 1);
            field->type = ftype;

            falign = type_align(ftype);
            if (falign > max_align) max_align = falign;

            if (is_union) {
                field->offset = 0;
                if (type_size(ftype) > max_size) max_size = type_size(ftype);
            } else {
                /* align offset */
                if (falign > 1) {
                    offset = (offset + falign - 1) & ~(falign - 1);
                }
                field->offset = offset;
                offset = offset + type_size(ftype);
            }

            field->next = 0;
            if (last_field) {
                last_field->next = field;
            } else {
                stype->fields = field;
            }
            last_field = field;

            /* handle multiple declarators: int *next, *prev; */
            while (cc->tk == TK_COMMA) {
                Type *ftype2;
                char fname2[MAX_IDENT];
                StructField *field2;
                int falign2;
                next_token(cc);
                ftype2 = parse_declarator(cc, base_ftype, fname2);

                /* Ignore bitfield size since ZCC allocates full integers */
                if (cc->tk == TK_COLON) {
                    next_token(cc);
                    if (cc->tk == TK_NUM) next_token(cc);
                }

                field2 = (StructField *)cc_alloc(cc, sizeof(StructField));
                strncpy(field2->name, fname2, MAX_IDENT - 1);
                field2->type = ftype2;
                falign2 = type_align(ftype2);
                if (falign2 > max_align) max_align = falign2;
                if (is_union) {
                    field2->offset = 0;
                    if (type_size(ftype2) > max_size) max_size = type_size(ftype2);
                } else {
                    if (falign2 > 1) {
                        offset = (offset + falign2 - 1) & ~(falign2 - 1);
                    }
                    field2->offset = offset;
                    offset = offset + type_size(ftype2);
                }
                field2->next = 0;
                last_field->next = field2;
                last_field = field2;
            }

            expect(cc, TK_SEMI);
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
                /* parse constant expression — skip complex AST logic, fast forward to next enum delimiter */
                int is_neg = 0;
                while (cc->tk != TK_COMMA && cc->tk != TK_RBRACE && cc->tk != TK_EOF) {
                    if (cc->tk == TK_MINUS) {
                        is_neg = 1;
                    } else if (cc->tk == TK_NUM) {
                        val = is_neg ? -cc->tk_val : cc->tk_val;
                        is_neg = 0;
                    } else {
                        is_neg = 0; /* Reset if other tokens like variables appear */
                    }
                    next_token(cc);
                }
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

Type *parse_type(Compiler *cc) {
    int is_unsigned;
    int is_signed;
    int is_typedef_kw;
    int is_static;
    int is_extern;
    Type *type;

    is_unsigned = 0;
    is_signed = 0;
    is_typedef_kw = 0;
    is_static = 0;
    is_extern = 0;
    type = 0;

    /* storage class / qualifiers */
    for (;;) {
        if (cc->tk == TK_STATIC) { is_static = 1; next_token(cc); }
        else if (cc->tk == TK_EXTERN) { is_extern = 1; next_token(cc); }
        else if (cc->tk == TK_CONST) { next_token(cc); }
        else if (cc->tk == TK_VOLATILE) { next_token(cc); }
        else if (cc->tk == TK_INLINE) { next_token(cc); }
        else if (cc->tk == TK_AUTO) { next_token(cc); }
        else if (cc->tk == TK_REGISTER) { next_token(cc); }
        else if (cc->tk == TK_TYPEDEF) { is_typedef_kw = 1; next_token(cc); }
        else break;
    }

    if (cc->tk == TK_UNSIGNED) { is_unsigned = 1; next_token(cc); }
    else if (cc->tk == TK_SIGNED) { is_signed = 1; next_token(cc); }

    if (cc->tk == TK_VOID) { type = cc->ty_void; next_token(cc); }
    else if (cc->tk == TK_CHAR) {
        if (is_unsigned) { type = cc->ty_uchar; } else { type = cc->ty_char; }
        next_token(cc);
    }
    else if (cc->tk == TK_SHORT) {
        if (is_unsigned) { type = cc->ty_ushort; } else { type = cc->ty_short; }
        next_token(cc);
        if (cc->tk == TK_INT) next_token(cc);
    }
    else if (cc->tk == TK_INT) {
        if (is_unsigned) { type = cc->ty_uint; } else { type = cc->ty_int; }
        next_token(cc);
    }
    else if (cc->tk == TK_LONG) {
        next_token(cc);
        if (cc->tk == TK_LONG) {
            next_token(cc);
            if (cc->tk == TK_INT) next_token(cc);
            if (is_unsigned) { type = cc->ty_ulonglong; } else { type = cc->ty_longlong; }
        } else {
            if (cc->tk == TK_INT) next_token(cc);
            else if (cc->tk == TK_DOUBLE) next_token(cc); /* consume 'long double' */
            if (is_unsigned) { type = cc->ty_ulong; } else { type = cc->ty_long; }
        }
    }
    else if (cc->tk == TK_FLOAT || cc->tk == TK_DOUBLE) {
        if (cc->tk == TK_FLOAT) type = cc->ty_float;
        else type = cc->ty_double;
        next_token(cc);
    }
    else if (cc->tk == TK_STRUCT) {
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
            printf("DEBUG: parse_type lookup 'yyParser'. sym=%p, sym->type=%p, tag='%s', kind=%d\n", 
                   (void*)sym, sym ? (void*)sym->type : NULL, 
                   (sym && sym->type && sym->type->tag[0]) ? sym->type->tag : "<none>",
                   (sym && sym->type) ? sym->type->kind : -1);
            fflush(stdout);
        }
        if (sym) {
            if (sym->is_typedef) {
                type = sym->type;
                next_token(cc);
            }
        }
        if (!type) {
            /* bare unsigned means unsigned int */
            if (is_unsigned) {
                type = cc->ty_uint;
            } else {
                printf("DEBUG: parse_type fallback 1: tk=%d text='%s'\n", cc->tk, cc->tk_text);
                error(cc, "expected type");
                type = cc->ty_int;
            }
        }
    }
    else {
        if (is_unsigned) {
            type = cc->ty_uint;
        } else if (is_signed) {
            type = cc->ty_int;
        } else {
            printf("DEBUG: parse_type fallback 2: tk=%d text='%s'\n", cc->tk, cc->tk_text);
            error(cc, "expected type");
            type = cc->ty_int;
        }
    }

    /* skip trailing const/volatile (e.g. 'char const *') */
    while (cc->tk == TK_CONST || cc->tk == TK_VOLATILE) {
        next_token(cc);
    }

    cc->current_is_static = is_static;
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
        if (pk == TK_STAR) {
            int ptr_count;
            int arr_lens[8];
            int arr_num;
            next_token(cc); /* consume ( */
            ptr_count = 0;
            arr_num = 0;
            while (cc->tk == TK_STAR) {
                next_token(cc);
                while (cc->tk == TK_CONST || cc->tk == TK_VOLATILE)
                    next_token(cc);
                ptr_count++;
            }
            if (cc->tk == TK_IDENT) {
                strncpy(name_out, cc->tk_text, MAX_IDENT - 1);
                next_token(cc);
            }
            while (cc->tk == TK_LBRACKET) {
                int alen = 0;
                next_token(cc);
                if (cc->tk != TK_RBRACKET) {
                    Node *expr = parse_assign(cc);
                    alen = eval_const_expr(expr);
                }
                expect(cc, TK_RBRACKET);
                if (arr_num < 8) arr_lens[arr_num++] = alen;
            }
            expect(cc, TK_RPAREN);
            /* parse trailing (params) or [N] — they modify base, then wrap with ptr */
            if (cc->tk == TK_LPAREN) {
                Type *ftype;
                next_token(cc);
                ftype = type_func(cc, base);
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
                            goto parse_grouped_params;
                        }
                    } else {
                        parse_grouped_params:
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
            while (ptr_count > 0) {
                type = type_ptr(cc, type);
                ptr_count--;
            }
            while (arr_num > 0) {
                arr_num--;
                type = type_array(cc, type, arr_lens[arr_num]);
            }
            return type;
        }
    }

    /* array dimensions */
    if (cc->tk == TK_LBRACKET) {
        int arr_lens[16];
        int arr_num = 0;
        while (cc->tk == TK_LBRACKET) {
            int len = 0;
            next_token(cc);
            if (cc->tk != TK_RBRACKET) {
                Node *expr = parse_assign(cc);
                len = eval_const_expr(expr);
            }
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

Node *parse_unary(Compiler *cc) {
    int line;
    Node *n;

    line = cc->tk_line;

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

Node *parse_stmt(Compiler *cc) {
    int line;
    line = cc->tk_line;

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
        wh->body = parse_stmt(cc);
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

        forn->body = parse_stmt(cc);
        scope_pop(cc);
        return forn;
    }

    /* do-while */
    if (cc->tk == TK_DO) {
        Node *dow;
        next_token(cc);
        dow = node_new(cc, ND_DO_WHILE, line);
        dow->body = parse_stmt(cc);
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

        sw->body = parse_stmt(cc);

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
        return brk;
    }

    /* continue */
    if (cc->tk == TK_CONTINUE) {
        Node *cont;
        next_token(cc);
        cont = node_new(cc, ND_CONTINUE, line);
            expect(cc, TK_SEMI);
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
        if (cc->tk == TK_TYPEDEF) is_typedef = 1;
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
                            var->sym = sym;
                            var->type = vtype;
                            asgn = node_new(cc, ND_ASSIGN, line);
                            asgn->lhs = var;
                            asgn->rhs = parse_assign(cc);
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
                    while (cc->tk == TK_LBRACKET) {
                        int alen = 0;
                        next_token(cc);
                        if (cc->tk == TK_NUM) { alen = cc->tk_val; next_token(cc); }
                        else if (cc->tk == TK_IDENT) {
                            Symbol *sym = scope_find(cc, cc->tk_text);
                            if (sym && sym->is_enum_const) alen = (int)sym->enum_val;
                            next_token(cc);
                        } else if (cc->tk == TK_SIZEOF) {
                            next_token(cc);
                            if (cc->tk == TK_LPAREN) {
                                next_token(cc);
                                if (is_type_token(cc)) {
                                    Type *st = parse_type(cc);
                                    char dummy[128];
                                    st = parse_declarator(cc, st, dummy);
                                    alen = type_size(st);
                                }
                                expect(cc, TK_RPAREN);
                            }
                        }
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
                if (cc->tk == TK_NUM) {
                    alen = cc->tk_val;
                    next_token(cc);
                } else if (cc->tk == TK_IDENT) {
                    Symbol *sym;
                    sym = scope_find(cc, cc->tk_text);
                    if (sym) {
                        if (sym->is_enum_const) {
                            alen = (int)sym->enum_val;
                        }
                    }
                    next_token(cc);
                } else if (cc->tk == TK_SIZEOF) {
                    next_token(cc);
                    if (cc->tk == TK_LPAREN) {
                        next_token(cc);
                        if (is_type_token(cc)) {
                            Type *st;
                            char dummy[128];
                            st = parse_type(cc);
                            st = parse_declarator(cc, st, dummy);
                            alen = type_size(st);
                        }
                        expect(cc, TK_RPAREN);
                    }
                }
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
                    gvar2->initializer = parse_assign(cc);
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

#include "ir_bridge.h"
#include "ir_emit_dispatch.h"

static void push_reg(Compiler *cc, char *reg) {
  fprintf(cc->out, "    pushq %%%s\n", reg);
  cc->stack_depth++;
}

static void pop_reg(Compiler *cc, char *reg) {
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
  switch (fmt) {
  case FMT_JE:
    fprintf(cc->out, "    je .L%d\n", n);
    break;
  case FMT_JMP:
    fprintf(cc->out, "    jmp .L%d\n", n);
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
  if (!type) {
    fprintf(cc->out, "    movq %%r11, (%%rax)\n");
    fprintf(cc->out, "    movq %%r11, %%rax\n");
    return;
  }
  /* Do NOT check fault range here: stage2 miscompiles it and rejects valid
   * Type* ptrs. */
  /* Pointers must always be 64-bit (fixes stage2 sign-extended 32-bit store).
   */
  if (type->kind == TY_PTR) {
    fprintf(cc->out, "    movq %%r11, (%%rax)\n");
    fprintf(cc->out, "    movq %%r11, %%rax\n");
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
    fprintf(cc->out, "    movq %%r11, %%rax\n");
    return;
  }
  switch (type->size) {
  case 1:
    fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
    break;
  case 2:
    fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
    break;
  case 4:
    fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
    break;
  default:
    fprintf(cc->out, "    movq %%r11, (%%rax)\n");
    break;
  }
  fprintf(cc->out, "    movq %%r11, %%rax\n");
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
        fprintf(cc->out, "    leaq %d(%%rbp), %%rax\n", off);
        char vname[32];
        sprintf(vname, "%%stack_%d", off);
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_UNARY(IR_ADDR, IR_TY_PTR, dst, vname, node->line);
        return;
      }
    }
    fprintf(cc->out, "    leaq %s(%%rip), %%rax\n", node->name);
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
      fprintf(cc->out, "    movq %s, %%rax\n", node->sym->assigned_reg);
      if (node->type && !node_type_unsigned(node) && !is_pointer(node->type)) {
          if (node->type->size == 4) fprintf(cc->out, "    cltq\n");
          else if (node->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->type && node_type_unsigned(node)) {
          if (node->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
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
          if (node->lhs->type->size == 4) fprintf(cc->out, "    cltq\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      fprintf(cc->out, "    movq %%rax, %s\n", node->lhs->sym->assigned_reg);
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
        fprintf(cc->out, "    movq %%r11, %%rax\n");
      } else {
        /* If member type is pointer/func, always use movq regardless of member_size */
        if (node->lhs->type && is_pointer(node->lhs->type)) {
          fprintf(cc->out, "    movq %%r11, (%%rax)\n");
        } else {
        switch (node->lhs->member_size) {
        case 1:
          fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
          break;
        case 2:
          fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
          break;
        case 4:
          fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
          break;
        default:
          fprintf(cc->out, "    movq %%r11, (%%rax)\n");
          break;
        }
        }
        fprintf(cc->out, "    movq %%r11, %%rax\n");
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
      fprintf(cc->out, "    movq %%rax, %%r11\n");
      fprintf(cc->out, "    movq %s, %%rax\n", reg);
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
        fprintf(cc->out, "    subq %%r11, %%rax\n");
        break;
      case ND_MUL:
        fprintf(cc->out, "    imulq %%r11, %%rax\n");
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
        fprintf(cc->out, "    andq %%r11, %%rax\n");
        break;
      case ND_BOR:
        fprintf(cc->out, "    orq %%r11, %%rax\n");
        break;
      case ND_BXOR:
        fprintf(cc->out, "    xorq %%r11, %%rax\n");
        break;
      case ND_SHL:
        fprintf(cc->out, "    movq %%r11, %%rcx\n    shlq %%cl, %%rax\n");
        break;
      case ND_SHR:
        if (node->lhs->type && is_unsigned_type(node->lhs->type))
          fprintf(cc->out, "    movq %%r11, %%rcx\n    shrq %%cl, %%rax\n");
        else
          fprintf(cc->out, "    movq %%r11, %%rcx\n    sarq %%cl, %%rax\n");
        break;
      default:
        break;
      }
      if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
          if (node->lhs->type->size == 4) fprintf(cc->out, "    cltq\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      fprintf(cc->out, "    movq %%rax, %s\n", reg);
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
    fprintf(cc->out, "    movq %%rax, %%r11\n");
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
      fprintf(cc->out, "    subq %%r11, %%rax\n");
      break;
    case ND_MUL:
      if (node->lhs->type && is_float_type(node->lhs->type)) {
        fprintf(cc->out, "    movq %%rax, %%xmm0\n");
        fprintf(cc->out, "    movq %%r11, %%xmm1\n");
        fprintf(cc->out, "    mulsd %%xmm1, %%xmm0\n");
        fprintf(cc->out, "    movq %%xmm0, %%rax\n");
        break;
      }
      fprintf(cc->out, "    imulq %%r11, %%rax\n");
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
        fprintf(cc->out, "    movq %%rdx, %%rax\n");
      } else {
        fprintf(cc->out, "    cqo\n    idivq %%r11\n    movq %%rdx, %%rax\n");
      }
      break;
    case ND_BAND:
      fprintf(cc->out, "    andq %%r11, %%rax\n");
      break;
    case ND_BOR:
      fprintf(cc->out, "    orq %%r11, %%rax\n");
      break;
    case ND_BXOR:
      fprintf(cc->out, "    xorq %%r11, %%rax\n");
      break;
    case ND_SHL:
      fprintf(cc->out, "    movq %%r11, %%rcx\n    shlq %%cl, %%rax\n");
      break;
    case ND_SHR:
      if (node->lhs->type && is_unsigned_type(node->lhs->type))
        fprintf(cc->out, "    movq %%r11, %%rcx\n    shrq %%cl, %%rax\n");
      else
        fprintf(cc->out, "    movq %%r11, %%rcx\n    sarq %%cl, %%rax\n");
      break;
    default:
      break;
    }
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->kind == ND_MEMBER && node->lhs->member_size > 0) {
      switch (node->lhs->member_size) {
      case 1:
        fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
        break;
      case 2:
        fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
        break;
      case 4:
        fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
        break;
      default:
        fprintf(cc->out, "    movq %%r11, (%%rax)\n");
        break;
      }
    } else {
      switch (type_size(node->lhs->type)) {
      case 1:
        fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
        break;
      case 2:
        fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
        break;
      case 4:
        fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
        break;
      default:
        fprintf(cc->out, "    movq %%r11, (%%rax)\n");
        break;
      }
    }
    fprintf(cc->out, "    movq %%r11, %%rax\n");
    if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
        if (node->lhs->type->size == 4) fprintf(cc->out, "    cltq\n");
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
        if (node->lhs->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
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
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && is_pointer(node->lhs->type)) {
      int esz = ptr_elem_size(node->lhs->type);
      if (esz > 1) {
        fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
        char scale_str[32];
        sprintf(scale_str, "%d", esz);
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_BINARY(IR_MUL, IR_TY_I64, dst, rhs_ir, scale_str, node->line);
        strcpy(rhs_ir, dst);
      }
    } else if (node->rhs->type && is_pointer(node->rhs->type)) {
      int esz = ptr_elem_size(node->rhs->type);
      if (esz > 1) {
        fprintf(cc->out, "    imulq $%d, %%rax\n", esz);
        char scale_str[32];
        sprintf(scale_str, "%d", esz);
        char *dst = ir_bridge_fresh_tmp();
        ZCC_EMIT_BINARY(IR_MUL, IR_TY_I64, dst, lhs_ir, scale_str, node->line);
        strcpy(lhs_ir, dst);
      }
    }
    fprintf(cc->out, "    addq %%r11, %%rax\n");
    if (node->type && type_size(node->type) == 4 && !is_pointer(node->type)) {
      if (node_type_unsigned(node)) {
        fprintf(cc->out, "    movl %%eax, %%eax\n");
      } else {
        fprintf(cc->out, "    cltq\n");
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
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && is_pointer(node->lhs->type)) {
      if (node->rhs->type && is_pointer(node->rhs->type)) {
        fprintf(cc->out, "    subq %%r11, %%rax\n");
        {
          int esz;
          esz = ptr_elem_size(node->lhs->type);
          if (esz > 1) {
            fprintf(cc->out, "    movq $%d, %%r11\n", esz);
            fprintf(cc->out, "    cqo\n    idivq %%r11\n");
          }
        }
        ir_emit_binary_op(ND_SUB, node->type, lhs_ir, rhs_ir, node->line);
        return;
      }
      {
        int esz;
        esz = node->lhs->type ? ptr_elem_size(node->lhs->type) : 1;
        if (esz > 1)
          fprintf(cc->out, "    imulq $%d, %%r11\n", esz);
      }
    }
    fprintf(cc->out, "    subq %%r11, %%rax\n");
    if (node->type && type_size(node->type) == 4 && !is_pointer(node->type)) {
      if (node_type_unsigned(node)) {
        fprintf(cc->out, "    movl %%eax, %%eax\n");
      } else {
        fprintf(cc->out, "    cltq\n");
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

    codegen_expr_checked(cc, node->lhs);
    ir_save_result(lhs_ir);
    push_reg(cc, "rax");
    codegen_expr_checked(cc, node->rhs);
    ir_save_result(rhs_ir);
    pop_reg(cc, "r11");
    fprintf(cc->out, "    imulq %%r11, %%rax\n");
    if (node->type && type_size(node->type) == 4 && !is_pointer(node->type)) {
      if (node_type_unsigned(node)) {
        fprintf(cc->out, "    movl %%eax, %%eax\n");
      } else {
        fprintf(cc->out, "    cltq\n");
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
    fprintf(cc->out, "    movq %%rax, %%r11\n");
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
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (node->lhs->type && node->rhs->type &&
        (is_unsigned_type(node->lhs->type) ||
         is_unsigned_type(node->rhs->type))) {
      fprintf(cc->out, "    xorq %%rdx, %%rdx\n");
      fprintf(cc->out, "    divq %%r11\n");
      fprintf(cc->out, "    movq %%rdx, %%rax\n");
    } else {
      fprintf(cc->out, "    cqo\n    idivq %%r11\n");
      fprintf(cc->out, "    movq %%rdx, %%rax\n");
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
    fprintf(cc->out, "    andq %%r11, %%rax\n");
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
    fprintf(cc->out, "    orq %%r11, %%rax\n");
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
    fprintf(cc->out, "    xorq %%r11, %%rax\n");
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
      fprintf(cc->out, "    xorq %%r11, %%rax\n");
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
    fprintf(cc->out, "    cmpq $0, %%rax\n");
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
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    if (use32)
      fprintf(cc->out,
              "    cmpl %%r11d, %%eax\n"); /* 32-bit: avoids sign-extended imm
                                              in 64-bit */
    else
      fprintf(cc->out, "    cmpq %%r11, %%rax\n");
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
    ir_emit_binary_op(node->kind, node->type, lhs_ir, rhs_ir, node->line);
    return;
  }

  case ND_LAND:
    lbl1 = new_label(cc);
    codegen_expr_checked(cc, node->lhs);
    fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    je .L%d\n", lbl1);
    codegen_expr_checked(cc, node->rhs);
    fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    setne %%al\n");
    fprintf(cc->out, "    movzbl %%al, %%eax\n");
    fprintf(cc->out, ".L%d:\n", lbl1);
    return;

  case ND_LOR:
    lbl1 = new_label(cc);
    lbl2 = new_label(cc);
    codegen_expr_checked(cc, node->lhs);
    fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    jne .L%d\n", lbl1);
    codegen_expr_checked(cc, node->rhs);
    fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    jne .L%d\n", lbl1);
    fprintf(cc->out, "    movq $0, %%rax\n");
    fprintf(cc->out, "    jmp .L%d\n", lbl2);
    fprintf(cc->out, ".L%d:\n", lbl1);
    fprintf(cc->out, "    movq $1, %%rax\n");
    fprintf(cc->out, ".L%d:\n", lbl2);
    return;

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
    return;

  case ND_DEREF:
    if (!node->lhs) {
      error_at(cc, node->line, "codegen_expr: ND_DEREF null lhs");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    codegen_expr_checked(cc, node->lhs);
    /* Type-aware load: char* -> movsbl, int* -> movl, ptr/long* -> movq */
    if (node->type && node->type->kind == TY_FUNC) {
        /* Do nothing: function pointer decays natively */
    } else {
        codegen_load(cc, node->type);
    }
    return;

  case ND_MEMBER:
    codegen_addr_checked(cc, node);
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
      default:
        break;
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
            fprintf(cc->out, "    movl %%eax, %%eax\n");
        } else if (!is_pointer(node->lhs ? node->lhs->type : 0)) {
            fprintf(cc->out, "    cltq\n");
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
                fprintf(cc->out, "    movq %%rax, %%rdx\n");
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
            if (node->lhs && node->lhs->type && is_unsigned_type(node->lhs->type))
                fprintf(cc->out, "    movl %%eax, %%eax\n");
            else
                fprintf(cc->out, "    cltq\n");
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

  case ND_TERNARY:
    if (!node->cond || !node->then_body || !node->else_body) {
      error_at(cc, node->line,
               "codegen_expr: ND_TERNARY missing cond/then/else");
      fprintf(cc->out, "    movq $0, %%rax\n");
      return;
    }
    lbl1 = new_label(cc);
    lbl2 = new_label(cc);
    codegen_expr_checked(cc, node->cond);
    fprintf(cc->out, "    cmpq $0, %%rax\n");
    fprintf(cc->out, "    je .L%d\n", lbl1);
    codegen_expr_checked(cc, node->then_body);
    fprintf(cc->out, "    jmp .L%d\n", lbl2);
    fprintf(cc->out, ".L%d:\n", lbl1);
    codegen_expr_checked(cc, node->else_body);
    fprintf(cc->out, ".L%d:\n", lbl2);
    return;

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
      fprintf(cc->out, "    addq $%d, %s\n", esz, reg);
      fprintf(cc->out, "    movq %s, %%rax\n", reg);
      if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
          if (node->lhs->type->size == 4) fprintf(cc->out, "    cltq\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      fprintf(cc->out, "    movq %%rax, %s\n", reg);
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
      fprintf(cc->out, "    addq $%d, %%rax\n", esz);
    }
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    switch (type_size(node->lhs->type)) {
    case 1:
      fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    fprintf(cc->out, "    movq %%r11, %%rax\n");
    if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
        if (node->lhs->type->size == 4) fprintf(cc->out, "    cltq\n");
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
        if (node->lhs->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
    }
    return;

  case ND_PRE_DEC:
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      int esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      fprintf(cc->out, "    subq $%d, %s\n", esz, reg);
      fprintf(cc->out, "    movq %s, %%rax\n", reg);
      if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
          if (node->lhs->type->size == 4) fprintf(cc->out, "    cltq\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
      } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
          if (node->lhs->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
          else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
          else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
      }
      fprintf(cc->out, "    movq %%rax, %s\n", reg);
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
      fprintf(cc->out, "    subq $%d, %%rax\n", esz);
    }
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rax");
    switch (type_size(node->lhs->type)) {
    case 1:
      fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    fprintf(cc->out, "    movq %%r11, %%rax\n");
    if (node->lhs->type && !node_type_unsigned(node->lhs) && !is_pointer(node->lhs->type)) {
        if (node->lhs->type->size == 4) fprintf(cc->out, "    cltq\n");
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->lhs->type && node_type_unsigned(node->lhs)) {
        if (node->lhs->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
        else if (node->lhs->type->size == 1) fprintf(cc->out, "    movzbq %%al, %%rax\n");
        else if (node->lhs->type->size == 2) fprintf(cc->out, "    movzwq %%ax, %%rax\n");
    }
    return;

  case ND_POST_INC:
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      int esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      fprintf(cc->out, "    movq %s, %%rax\n", reg);
      fprintf(cc->out, "    addq $%d, %s\n", esz, reg);
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
      fprintf(cc->out, "    addq $%d, %%rax\n", esz);
    }
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rdx"); /* rdx = original value */
    pop_reg(cc, "rax"); /* rax = address */
    switch (type_size(node->lhs->type)) {
    case 1:
      fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    fprintf(cc->out, "    movq %%rdx, %%rax\n");
    return;

  case ND_POST_DEC:
    if (node->lhs && node->lhs->kind == ND_VAR && node->lhs->sym &&
        node->lhs->sym->assigned_reg) {
      char *reg = node->lhs->sym->assigned_reg;
      int esz = 1;
      if (is_pointer(node->lhs->type))
        esz = ptr_elem_size(node->lhs->type);
      fprintf(cc->out, "    movq %s, %%rax\n", reg);
      fprintf(cc->out, "    subq $%d, %s\n", esz, reg);
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
      fprintf(cc->out, "    subq $%d, %%rax\n", esz);
    }
    fprintf(cc->out, "    movq %%rax, %%r11\n");
    pop_reg(cc, "rdx"); /* rdx = original value */
    pop_reg(cc, "rax"); /* rax = address */
    switch (type_size(node->lhs->type)) {
    case 1:
      fprintf(cc->out, "    movb %%r11b, (%%rax)\n");
      break;
    case 2:
      fprintf(cc->out, "    movw %%r11w, (%%rax)\n");
      break;
    case 4:
      fprintf(cc->out, "    movl %%r11d, (%%rax)\n");
      break;
    default:
      fprintf(cc->out, "    movq %%r11, (%%rax)\n");
      break;
    }
    fprintf(cc->out, "    movq %%rdx, %%rax\n");
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
          if (gp_idx < 6) {
            pop_reg(cc, argregs[gp_idx]);
            gp_idx++;
          }
        }
      }
      fprintf(cc->out, "    movl $%d, %%eax\n", fp_idx > 8 ? 8 : fp_idx);
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
      fprintf(cc->out, "    call %s\n", node->func_name);
    }

    if (node->type && is_float_type(node->type)) {
        fprintf(cc->out, "    movq %%xmm0, %%rax\n");
    } else if (node->type && !node_type_unsigned(node)) {
        if (node->type->size == 4) fprintf(cc->out, "    cltq\n");
        else if (node->type->size == 1) fprintf(cc->out, "    movsbq %%al, %%rax\n");
        else if (node->type->size == 2) fprintf(cc->out, "    movswq %%ax, %%rax\n");
    } else if (node->type && node_type_unsigned(node)) {
        if (node->type->size == 4) fprintf(cc->out, "    movl %%eax, %%eax\n");
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
    fprintf(cc->out, "    jmp .Lfunc_end_%d\n", cc->func_end_label);
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
    fprintf(cc->out, "    cmpq $0, %%rax\n");
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
    fprintf(cc->out, "    cmpq $0, %%rax\n");
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
      fprintf(cc->out, "    cmpq $0, %%rax\n");
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
    fprintf(cc->out, "    cmpq $0, %%rax\n");
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

  cc->stack_depth = 0;

  /* We need to re-create the scope with locals for codegen.
     During parsing, scope_add_local assigned stack offsets.
     Those offsets are stored in the Symbol nodes (via the arena).
     The body's ND_VAR nodes reference those Symbols.
     So the offsets are already embedded in the AST — we just need
     to store params from registers to their assigned stack slots. */

  scope_push(cc);

  /* Store params from registers to their stack locations.
     The param Symbols have stack_offsets assigned by scope_add_local.
     We need to retrieve those. They're in the func node's param info.
     Actually, we stored them during parse_func_def — let me use
     the offsets directly. Params got sequential offsets from scope_add_local:
     param0: -8, param1: -16, param2: -24, etc. */
  /* Store params from registers to their stack locations... */
  int param_offset = 0;
  int f_idx = 0;
  int gp_idx = 0;
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
    printf("EVAL kind=%d\n", elem->kind);
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
    if (gvar->initializer->kind == ND_NUM) {
      if (size == 1)
        fprintf(cc->out, "    .byte %lld\n", gvar->initializer->int_val);
      else if (size == 2)
        fprintf(cc->out, "    .short %lld\n", gvar->initializer->int_val);
      else if (size == 4)
        fprintf(cc->out, "    .long %lld\n", gvar->initializer->int_val);
      else
        fprintf(cc->out, "    .quad %lld\n", gvar->initializer->int_val);
    } else if (gvar->initializer->kind == ND_ADDR && gvar->initializer->lhs && gvar->initializer->lhs->kind == ND_VAR) {
      if (size == 4)
        fprintf(cc->out, "    .long %s\n", gvar->initializer->lhs->name);
      else
        fprintf(cc->out, "    .quad %s\n", gvar->initializer->lhs->name);
    } else if (gvar->initializer->kind == ND_VAR) {
      if (size == 4)
        fprintf(cc->out, "    .long %s\n", gvar->initializer->name);
      else
        fprintf(cc->out, "    .quad %s\n", gvar->initializer->name);
    } else if (gvar->initializer->kind == ND_STR) {
      if (gvar->type && gvar->type->kind == TY_ARRAY) {
          int j;
          int str_id = gvar->initializer->str_id;
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
          if (size == 4) fprintf(cc->out, "    .long .Lstr_%d\n", gvar->initializer->str_id);
          else fprintf(cc->out, "    .quad .Lstr_%d\n", gvar->initializer->str_id);
      }
        } else if (gvar->initializer->kind == ND_INIT_LIST) {
            int emitted = 0;
            int i;
            int elem_size = 1;
        if (gvar->type && gvar->type->kind == TY_STRUCT) {
            int arg_idx = 0;
            emit_struct_fields(cc, gvar->type->fields, gvar->initializer->args, gvar->initializer->num_args, &arg_idx, gvar->initializer->num_args, 0, &emitted);
        } else if (gvar->type && gvar->type->kind == TY_ARRAY && gvar->type->base && (gvar->type->base->kind == TY_STRUCT || gvar->type->base->kind == TY_UNION)) {
            int i;
            int arg_idx = 0;
            int elem_size = type_size(gvar->type->base);
            int budget = gvar->initializer->num_args / gvar->type->array_len;
            for (i = 0; i < gvar->type->array_len; i++) {
                int expected_end = (i + 1) * elem_size;
                int arg_end = (i + 1) * budget;
                if (arg_end > gvar->initializer->num_args) arg_end = gvar->initializer->num_args;
                emit_struct_fields(cc, gvar->type->base->fields, gvar->initializer->args, gvar->initializer->num_args, &arg_idx, arg_end, i * elem_size, &emitted);
                if (emitted < expected_end) {
                    fprintf(cc->out, "    .zero %d\n", expected_end - emitted);
                    emitted = expected_end;
                }
            }
        } else {
            int i;
            int elem_size = 1;
            if (gvar->type && gvar->type->base) elem_size = type_size(gvar->type->base);
            for (i = 0; i < gvar->initializer->num_args; i++) {
                Node *elem = gvar->initializer->args[i];
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
    /* uninitialized data goes in .bss */
    fprintf(cc->out, "    .bss\n");
    fprintf(cc->out, "    .p2align 3\n");
    if (!gvar->is_static) {
      fprintf(cc->out, "    .globl %s\n", gvar->name);
    }
    fprintf(cc->out, "%s:\n", gvar->name);
    fprintf(cc->out, "    .zero %d\n", size);
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
  fprintf(cc->out, "    .section .note.GNU-stack,\"\",@progbits\n");
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
          cc->globals[j] = 0; /* Both uninitialized, drop second */
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



  ZCC_IR_FLUSH(stdout);

  /* peephole optimize the emitted assembly safely out-of-bounds */
  peephole_optimize(asm_file);

  /* assemble and link if not stopping at assembly */
  if (!stop_at_asm) {
    printf("[Phase 6] GCC Assembly/Linker Binding... ");
    if (compile_only) {
      sprintf(cmd, "gcc -O0 -w -fno-asynchronous-unwind-tables -Wa,--noexecstack -fno-unwind-tables -c -o %s %s 2>&1", output_file, asm_file);
    } else if (strcmp(input_file, "zcc.c") == 0 || (strlen(input_file) >= 6 && strcmp(input_file + strlen(input_file) - 6, "/zcc.c") == 0)) {
      sprintf(cmd, "gcc -O0 -w -fno-asynchronous-unwind-tables -Wa,--noexecstack -fno-unwind-tables -o %s %s compiler_passes.c compiler_passes_ir.c -lm 2>&1", output_file, asm_file);
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

static const char *OP_NAMES[34] = {
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
    "nop"
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
    if (op < 0 || op >= 34) return "???";
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

        /* imm — only print for ops that use it */
        if (n->op == IR_CONST || n->op == IR_ALLOCA) {
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
/*
 * ir_to_x86.c — ZCC IR-to-x86_64 Lowering Backend
 *
 * Walks the ir_module_t and generates System V x86-64 assembly,
 * completely replacing the AST-based codegen in part4.c.
 */

#include "ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[IR_NAME_MAX];
    int offset;
} ir_var_t;

static ir_var_t *vars = 0;
static int max_vars = 0;
static int num_vars = 0;
static int next_offset_from_rbp = -8;

static int get_or_create_var(const char *name) {
    int i;
    if (!name || name[0] == '\0' || name[0] == '-') return 0;
    
    for (i = 0; i < num_vars; i++) {
        if (strcmp(vars[i].name, name) == 0) return vars[i].offset;
    }
    
    if (num_vars >= max_vars) {
        max_vars = (max_vars == 0) ? 2048 : max_vars * 2;
        ir_var_t *new_vars = (ir_var_t *)realloc(vars, max_vars * sizeof(ir_var_t));
        if (!new_vars) {
            fprintf(stderr, "\n[FATAL] IR Backend: Variable limit exceeded and out of memory!\n");
            fprintf(stderr, "Tried to allocate %d variables.\n", max_vars);
            exit(1);
        }
        vars = new_vars;
    }
    
    int off;
    strcpy(vars[num_vars].name, name);
    vars[num_vars].offset = next_offset_from_rbp;
    off = next_offset_from_rbp;
    next_offset_from_rbp -= 8;
    num_vars++;
    return off;
}

static void load_address(FILE *out, const char *src, const char *reg) {
    if (strncmp(src, "%stack_", 7) == 0) {
        int off = get_or_create_var(src);
        fprintf(out, "    leaq %d(%%rbp), %s\n", off, reg);
    } else if (strncmp(src, "%t", 2) == 0) {
        int off = get_or_create_var(src);
        fprintf(out, "    movq %d(%%rbp), %s\n", off, reg);
    } else {
        const char *name = src;
        if (name[0] == '%') name++; /* remove '%' prefix for globals */
        fprintf(out, "    leaq %s(%%rip), %s\n", name, reg);
    }
}

void ir_module_lower_x86(const ir_module_t *mod, FILE *out) {
    int i;
    const char *arg_regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

    fprintf(out, "    .text\n");
    
    for (i = 0; i < mod->func_count; i++) {
        ir_func_t *fn = mod->funcs[i];
        ir_node_t *n;
        int stack_size;
        int arg_idx;

        num_vars = 0;
        next_offset_from_rbp = -8;
        
        /* Pass 1: Linear scan to allocate stack slots for all variables */
        n = fn->head;
        while (n) {
            if (n->dst[0] != '\0' && n->dst[0] != '-') get_or_create_var(n->dst);
            if (n->src1[0] != '\0' && n->src1[0] != '-') get_or_create_var(n->src1);
            if (n->src2[0] != '\0' && n->src2[0] != '-') get_or_create_var(n->src2);
            /* Add space for ALLOCA */
            if (n->op == IR_ALLOCA) {
                next_offset_from_rbp -= n->imm;
            }
            n = n->next;
        }
        
        stack_size = -next_offset_from_rbp + 40; /* extra padding for calls */
        stack_size = (stack_size + 15) & ~15; /* 16-byte alignment */
        
        /* Prologue */
        fprintf(out, "    .globl %s\n", fn->name);
        fprintf(out, "%s:\n", fn->name);
        fprintf(out, "    pushq %%rbp\n");
        fprintf(out, "    movq %%rsp, %%rbp\n");
        fprintf(out, "    subq $%d, %%rsp\n", stack_size);
        
        /* Note: System V Parameters */
        for (int pnum = 0; pnum < fn->num_params; pnum++) {
            char param_name[32];
            sprintf(param_name, "%%stack_%d", -8 * (pnum + 1));
            int stack_off = get_or_create_var(param_name);
            if (pnum < 6) {
                fprintf(out, "    movq %s, %d(%%rbp)\n", arg_regs[pnum], stack_off);
            }
        }

        arg_idx = 0;

        /* Pass 2: Emit corresponding assembly */
        n = fn->head;
        while (n) {
            fprintf(out, "    # %s\n", ir_op_name(n->op));
            switch (n->op) {
                case IR_CONST: {
                    int off = get_or_create_var(n->dst);
                    fprintf(out, "    movabsq $%ld, %%rax\n", n->imm);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", off);
                    break;
                }
                case IR_CONST_STR: {
                    int off = get_or_create_var(n->dst);
                    fprintf(out, "    leaq %s(%%rip), %%rax\n", n->src1);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", off);
                    break;
                }
                case IR_ADD:
                case IR_SUB:
                case IR_MUL:
                case IR_AND:
                case IR_OR:
                case IR_XOR:
                case IR_SHL:
                case IR_SHR:
                case IR_EQ:
                case IR_NE:
                case IR_LT:
                case IR_LE:
                case IR_GT:
                case IR_GE: {
                    int off1 = get_or_create_var(n->src1);
                    int off2 = get_or_create_var(n->src2);
                    int offd = get_or_create_var(n->dst);
                    
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    if (n->op == IR_ADD)        fprintf(out, "    addq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_SUB)   fprintf(out, "    subq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_MUL)   fprintf(out, "    imulq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_AND)   fprintf(out, "    andq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_OR)    fprintf(out, "    orq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_XOR)   fprintf(out, "    xorq %d(%%rbp), %%rax\n", off2);
                    else if (n->op == IR_SHL) {
                        fprintf(out, "    movq %d(%%rbp), %%rcx\n", off2);
                        fprintf(out, "    shlq %%cl, %%rax\n");
                    } else if (n->op == IR_SHR) {
                        fprintf(out, "    movq %d(%%rbp), %%rcx\n", off2);
                        if (ir_type_unsigned(n->type)) fprintf(out, "    shrq %%cl, %%rax\n");
                        else fprintf(out, "    sarq %%cl, %%rax\n");
                    } else {
                        fprintf(out, "    cmpq %d(%%rbp), %%rax\n", off2);
                        if (n->op == IR_EQ) fprintf(out, "    sete %%al\n");
                        else if (n->op == IR_NE) fprintf(out, "    setne %%al\n");
                        else if (n->op == IR_LT) fprintf(out, "    setl %%al\n");
                        else if (n->op == IR_LE) fprintf(out, "    setle %%al\n");
                        else if (n->op == IR_GT) fprintf(out, "    setg %%al\n");
                        else if (n->op == IR_GE) fprintf(out, "    setge %%al\n");
                        fprintf(out, "    movzbq %%al, %%rax\n");
                    }
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_DIV:
                case IR_MOD: {
                    int off1 = get_or_create_var(n->src1);
                    int off2 = get_or_create_var(n->src2);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    if (ir_type_unsigned(n->type)) {
                        fprintf(out, "    xorq %%rdx, %%rdx\n");
                        fprintf(out, "    divq %d(%%rbp)\n", off2);
                    } else {
                        fprintf(out, "    cqo\n");
                        fprintf(out, "    idivq %d(%%rbp)\n", off2);
                    }
                    if (n->op == IR_MOD) fprintf(out, "    movq %%rdx, %d(%%rbp)\n", offd);
                    else fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_NOT:
                case IR_NEG: {
                    int off1 = get_or_create_var(n->src1);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    if (n->op == IR_NOT) fprintf(out, "    notq %%rax\n");
                    else fprintf(out, "    negq %%rax\n");
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_LOAD: {
                    int offd = get_or_create_var(n->dst);
                    load_address(out, n->src1, "%rax");
                    if (n->type == IR_TY_I32 || n->type == IR_TY_U32) {
                        fprintf(out, "    movl (%%rax), %%eax\n");
                        if (!ir_type_unsigned(n->type)) fprintf(out, "    movslq %%eax, %%rax\n");
                    } else if (n->type == IR_TY_I8 || n->type == IR_TY_U8) {
                        fprintf(out, "    movzbl (%%rax), %%eax\n");
                        if (!ir_type_unsigned(n->type)) fprintf(out, "    movsbq %%al, %%rax\n");
                    } else if (n->type == IR_TY_I16 || n->type == IR_TY_U16) {
                        fprintf(out, "    movzwl (%%rax), %%eax\n");
                        if (!ir_type_unsigned(n->type)) fprintf(out, "    movswq %%ax, %%rax\n");
                    } else {
                        fprintf(out, "    movq (%%rax), %%rax\n");
                    }
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_ADDR: {
                    int offd = get_or_create_var(n->dst);
                    load_address(out, n->src1, "%rax");
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_STORE: {
                    int off1 = get_or_create_var(n->src1); /* src1 holds the value */
                    fprintf(out, "    movq %d(%%rbp), %%rcx\n", off1);
                    load_address(out, n->dst, "%rax");     /* dst holds the address */
                    
                    if (n->type == IR_TY_I32 || n->type == IR_TY_U32) {
                        fprintf(out, "    movl %%ecx, (%%rax)\n");
                    } else if (n->type == IR_TY_I8 || n->type == IR_TY_U8) {
                        fprintf(out, "    movb %%cl, (%%rax)\n");
                    } else {
                        fprintf(out, "    movq %%rcx, (%%rax)\n");
                    }
                    break;
                }
                case IR_ALLOCA: {
                    int offd = get_or_create_var(n->dst);
                    /* Space was already skipped in pass 1 setup */
                    fprintf(out, "    leaq %d(%%rbp), %%rax\n", vars[offd].offset - (int)n->imm);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_COPY:
                case IR_CAST: {
                    int off1 = get_or_create_var(n->src1);
                    int offd = get_or_create_var(n->dst);
                    fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    break;
                }
                case IR_LABEL: {
                    fprintf(out, "%s:\n", n->label);
                    break;
                }
                case IR_BR: {
                    fprintf(out, "    jmp %s\n", n->label);
                    break;
                }
                case IR_BR_IF: {
                    int off1 = get_or_create_var(n->src1);
                    fprintf(out, "    cmpq $0, %d(%%rbp)\n", off1);
                    fprintf(out, "    je %s\n", n->label);
                    break;
                }
                case IR_ARG: {
                    int off1 = get_or_create_var(n->src1);
                    if (arg_idx < 6) {
                        fprintf(out, "    movq %d(%%rbp), %s\n", off1, arg_regs[arg_idx]);
                    } else {
                        fprintf(out, "    pushq %d(%%rbp)\n", off1);
                    }
                    arg_idx++;
                    break;
                }
                case IR_CALL: {
                    fprintf(out, "    movb $0, %%al\n");
                    fprintf(out, "    callq %s\n", n->label);
                    if (arg_idx > 6) {
                        fprintf(out, "    addq $%d, %%rsp\n", (arg_idx - 6) * 8);
                    }
                    if (n->dst[0] != '\0' && n->dst[0] != '-') {
                        int offd = get_or_create_var(n->dst);
                        fprintf(out, "    movq %%rax, %d(%%rbp)\n", offd);
                    }
                    arg_idx = 0; /* Reset for next call */
                    break;
                }
                case IR_RET: {
                    if (n->src1[0] != '\0' && n->src1[0] != '-') {
                        int off1 = get_or_create_var(n->src1);
                        fprintf(out, "    movq %d(%%rbp), %%rax\n", off1);
                    }
                    fprintf(out, "    movq %%rbp, %%rsp\n");
                    fprintf(out, "    popq %%rbp\n");
                    fprintf(out, "    ret\n");
                    break;
                }
                default: break;
            }
            n = n->next;
        }
        
        /* Fallback epilogue incase missing explicit RET */
        fprintf(out, "    movq %%rbp, %%rsp\n");
        fprintf(out, "    popq %%rbp\n");
        fprintf(out, "    ret\n");
    }
}
