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
    int     cond_satisfied[64];
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
    int in_str = 0;
    int in_char = 0;
    while (1) {
        int c_peek = pp_peek(state);
        if (c_peek == '\n') break;
        if (c_peek == 0) break;
        if (i >= max - 1) break;

        if (!in_str && !in_char && state->pos + 1 < state->len) {
            if (state->src[state->pos] == '/' && state->src[state->pos + 1] == '/') {
                for (;;) {
                    int p = pp_peek(state);
                    if (p == '\n') break;
                    if (p == 0) break;
                    pp_next(state);
                }
                break;
            }
            if (state->src[state->pos] == '/' && state->src[state->pos + 1] == '*') {
                pp_next(state); pp_next(state);
                for (;;) {
                    int p = pp_peek(state);
                    if (p == 0) break;
                    if (state->pos + 1 < state->len && state->src[state->pos] == '*' && state->src[state->pos + 1] == '/') {
                        pp_next(state); pp_next(state);
                        break;
                    }
                    if (p == '\n') {
                        buf[i++] = '\n';
                    }
                    pp_next(state);
                }
                if (i > 0 && buf[i-1] != ' ') buf[i++] = ' ';
                continue;
            }
        }
        
        char c = pp_next(state);
        
        if (c == '\\') {
            int nxt = pp_peek(state);
            if (nxt == '\n') {
                pp_next(state);
                continue;
            }
            if (nxt == '"' || nxt == '\'') {
                buf[i++] = c;
                if (i < max - 1) buf[i++] = pp_next(state);
                continue;
            }
        }
        if (c == '"' && !in_char) in_str = !in_str;
        if (c == '\'' && !in_str) in_char = !in_char;
        buf[i++] = c;
    }
    
    /* right trim trailing whitespace from the macro body */
    while (i > 0 && (buf[i-1] == ' ' || buf[i-1] == '\t' || buf[i-1] == '\r')) {
        i--;
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


static void pp_eval_skip_ws(const char **p) {
    while (**p == ' ' || **p == '\t' || **p == '\r' || **p == '\n') (*p)++;
}

static int pp_eval_ident_len(const char *p) {
    int i = 0;
    while ((p[i] >= 'a' && p[i] <= 'z') || (p[i] >= 'A' && p[i] <= 'Z') || (p[i] >= '0' && p[i] <= '9') || p[i] == '_') i++;
    return i;
}

static int pp_eval_or(PPState *state, const char **p);

static int pp_eval_primary(PPState *state, const char **p) {
    pp_eval_skip_ws(p);
    if (**p == '(') {
        (*p)++;
        int val = pp_eval_or(state, p);
        pp_eval_skip_ws(p);
        if (**p == ')') (*p)++;
        return val;
    }
    
    if (**p >= '0' && **p <= '9') {
        int val = 0;
        while (**p >= '0' && **p <= '9') {
            val = val * 10 + (**p - '0');
            (*p)++;
        }
        return val;
    }
    
    int len = pp_eval_ident_len(*p);
    if (len > 0) {
        if (strncmp(*p, "defined", len) == 0 && len == 7) {
            (*p) += len;
            pp_eval_skip_ws(p);
            int has_paren = 0;
            if (**p == '(') { has_paren = 1; (*p)++; pp_eval_skip_ws(p); }
            int id_len = pp_eval_ident_len(*p);
            char ident[128];
            if (id_len > 127) id_len = 127;
            strncpy(ident, *p, id_len);
            ident[id_len] = 0;
            (*p) += id_len;
            pp_eval_skip_ws(p);
            if (has_paren && **p == ')') (*p)++;
            
            PPMacro *m = pp_find_macro(state, ident);
            return m ? 1 : 0;
        } else {
            char ident[128];
            if (len > 127) len = 127;
            strncpy(ident, *p, len);
            ident[len] = 0;
            (*p) += len;
            
            PPMacro *m = pp_find_macro(state, ident);
            if (m) {
                if (m->body[0]) return atoi(m->body);
                return 0;
            }
            return 0;
        }
    }
    return 0;
}

static int pp_eval_unary(PPState *state, const char **p) {
    pp_eval_skip_ws(p);
    if (**p == '!') {
        (*p)++;
        return !pp_eval_unary(state, p);
    }
    return pp_eval_primary(state, p);
}

static int pp_eval_compare(PPState *state, const char **p) {
    int val = pp_eval_unary(state, p);
    pp_eval_skip_ws(p);
    
    if ((*p)[0] == '=' && (*p)[1] == '=') { (*p) += 2; return val == pp_eval_unary(state, p); }
    if ((*p)[0] == '!' && (*p)[1] == '=') { (*p) += 2; return val != pp_eval_unary(state, p); }
    if ((*p)[0] == '>' && (*p)[1] == '=') { (*p) += 2; return val >= pp_eval_unary(state, p); }
    if ((*p)[0] == '<' && (*p)[1] == '=') { (*p) += 2; return val <= pp_eval_unary(state, p); }
    if ((*p)[0] == '>') { (*p)++; return val > pp_eval_unary(state, p); }
    if ((*p)[0] == '<') { (*p)++; return val < pp_eval_unary(state, p); }
    
    return val;
}

static int pp_eval_and(PPState *state, const char **p) {
    int val = pp_eval_compare(state, p);
    while (1) {
        pp_eval_skip_ws(p);
        if ((*p)[0] == '&' && (*p)[1] == '&') {
            (*p) += 2;
            int right = pp_eval_compare(state, p);
            val = val && right;
        } else {
            break;
        }
    }
    return val;
}

static int pp_eval_or(PPState *state, const char **p) {
    int val = pp_eval_and(state, p);
    while (1) {
        pp_eval_skip_ws(p);
        if ((*p)[0] == '|' && (*p)[1] == '|') {
            (*p) += 2;
            int right = pp_eval_and(state, p);
            val = val || right;
        } else {
            break;
        }
    }
    return val;
}

static int pp_eval_expr(PPState *state, const char *expr) {
    const char *p = expr;
    return pp_eval_or(state, &p);
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
        state->cond_else_seen[state->cond_depth] = 0;
        if (!active) {
            state->cond_stack[state->cond_depth] = 0;
        } else {
            if (strcmp(dir, "ifdef") == 0) state->cond_stack[state->cond_depth] = is_defined;
            else state->cond_stack[state->cond_depth] = !is_defined;
        }
        state->cond_satisfied[state->cond_depth] = state->cond_stack[state->cond_depth];
        
        int new_active = 1;
        int i;
        for (i = 0; i <= state->cond_depth; i++) if (!state->cond_stack[i]) new_active = 0;

        pp_read_line(state, dir, 64);
        } else if (strcmp(dir, "if") == 0) {
        state->cond_depth++;
        state->cond_else_seen[state->cond_depth] = 0;
        if (!active) {
            state->cond_stack[state->cond_depth] = 0;
            state->cond_satisfied[state->cond_depth] = 0;
        } else {
            char expr[256];
            pp_read_line(state, expr, 256);
            int result = pp_eval_expr(state, expr);
            state->cond_stack[state->cond_depth] = result;
            state->cond_satisfied[state->cond_depth] = result;
            
            int new_active = 1;
            int i;
            for (i = 0; i <= state->cond_depth; i++) if (!state->cond_stack[i]) new_active = 0;

        }
    } else if (strcmp(dir, "elif") == 0) {
        if (state->cond_depth <= 0) {
            fprintf(stderr, "#elif without #if\\n");
        } else if (state->cond_else_seen[state->cond_depth]) {
            fprintf(stderr, "#elif after #else\\n");
        } else {
            char expr[256];
            pp_read_line(state, expr, 256);
            if (state->cond_satisfied[state->cond_depth]) {
                state->cond_stack[state->cond_depth] = 0;
            } else {
                int active_parent = 1;
                int i;
                for (i = 0; i < state->cond_depth; i++) if (!state->cond_stack[i]) active_parent = 0;
                if (!active_parent) {
                    state->cond_stack[state->cond_depth] = 0;
                } else {
                    int result = pp_eval_expr(state, expr);
                    state->cond_stack[state->cond_depth] = result;
                    if (result) state->cond_satisfied[state->cond_depth] = 1;
                }
            }
        }
} else if (strcmp(dir, "else") == 0) {
        if (state->cond_depth > 0) {
            state->cond_else_seen[state->cond_depth] = 1;
            int parent_active = 1;
            int i;
            for (i = 0; i < state->cond_depth; i++) if (!state->cond_stack[i]) parent_active = 0;
            if (parent_active) {
                if (state->cond_satisfied[state->cond_depth]) {
                    state->cond_stack[state->cond_depth] = 0;
                } else {
                    state->cond_stack[state->cond_depth] = 1;
                    state->cond_satisfied[state->cond_depth] = 1;
                }
            } else {
                state->cond_stack[state->cond_depth] = 0;
            }
        }
        pp_read_line(state, dir, 64);
    } else if (strcmp(dir, "endif") == 0) {

        if (state->cond_depth > 0) {
            state->cond_satisfied[state->cond_depth] = 0;
            state->cond_else_seen[state->cond_depth] = 0;
            state->cond_depth--;
        }
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
    int in_string = 0;
    int in_char = 0;
    while (pp_peek(state) != 0) {
        c = pp_peek(state);
        
        if (c == '\\') {
            if (p_count < PP_MAX_PARAMS && arg_idx < 255) {
                args[p_count][arg_idx++] = c;
                args[p_count][arg_idx] = 0;
            }
            pp_next(state);
            c = pp_peek(state);
            if (c == 0) break;
            if (p_count < PP_MAX_PARAMS && arg_idx < 255) {
                args[p_count][arg_idx++] = c;
                args[p_count][arg_idx] = 0;
            }
            pp_next(state);
            continue;
        }
        
        if (c == '"' && !in_char) in_string = !in_string;
        else if (c == '\'' && !in_string) in_char = !in_char;
        
        if (!in_string && !in_char) {
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
        
        static int logged_1429 = 0;
        if (state->line == 1429 && !logged_1429) {
            FILE *f = fopen("pp_trace.log", "a");
            if (f) { fprintf(f, "LINE 1429 REACHED! active=%d depth=%d cond[1]=%d new_active=%d\\n", pp_is_active(state), state->cond_depth, state->cond_stack[1], state->cond_stack[0] && state->cond_stack[1] && state->cond_stack[2] && state->cond_stack[3]); fclose(f); }
            logged_1429 = 1;
        }
        
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
            if (state->line > 3680) {
                FILE *trace = fopen("pp_trace.log", "a");
                if (trace) {
                    fprintf(trace, "HIT '#' AT LINE %d (active=%d)\\n", state->line, pp_is_active(state));
                    fclose(trace);
                }
            }
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
    
    FILE *dump = fopen("pp_dump.c", "wb");
    if (dump) {
        fwrite(result, 1, state->out_len - 1, dump);
        fclose(dump);
    }
    
    free(state);
    return result;
}
