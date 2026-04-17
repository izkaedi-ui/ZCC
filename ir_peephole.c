#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ir.h"
#include "ir_peephole.h"
#include "ir_telemetry.h"

/* ------------------------------------------------------------------ *
 * Configuration
 * ------------------------------------------------------------------ */
void ir_peephole_config_defaults(ir_peephole_config_t *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->enabled_classes    = IR_PEEP_ALL;
    cfg->gradient_threshold = 0.05;
    cfg->max_outer_passes   = IR_PEEP_MAX_OUTER_PASSES;
    cfg->max_fires_per_iter = IR_PEEP_MAX_FIRES_PER_IT;
    cfg->manifold_guided    = 0;
    cfg->deterministic      = 0;
    cfg->verbose            = 0;
}

/* ------------------------------------------------------------------ *
 * Global context flags
 * ------------------------------------------------------------------ */
int g_peephole_enabled = 0;
int g_peephole_deterministic = 0;
int g_peephole_verbose = 0;
ir_manifold_t *g_peephole_manifold = NULL;

void ir_peephole_set_manifold(ir_manifold_t *m) {
    g_peephole_manifold = m;
}

/* ------------------------------------------------------------------ *
 * Helpers
 * ------------------------------------------------------------------ */
static int count_nodes(ir_func_t *fn) {
    int count = 0;
    ir_node_t *n;
    for (n = fn->head; n; n = n->next) count++;
    return count;
}

static int is_power_of_two(long x, int *out_k) {
    if (x <= 0) return 0;
    if ((x & (x - 1)) != 0) return 0;
    int k = 0;
    while (x > 1) {
        x >>= 1;
        k++;
    }
    if (out_k) *out_k = k;
    return 1;
}

/* Local constant getter */
static int get_const(ir_func_t *fn, const char *name, long *out_val) {
    ir_node_t *n;
    if (!name || name[0] == '\0') return 0;
    for (n = fn->head; n; n = n->next) {
        if (n->op == IR_CONST && strcmp(n->dst, name) == 0) {
            *out_val = n->imm;
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ *
 * Starter Patterns
 * ------------------------------------------------------------------ */

/* Pattern 1: strength_pow2_mul 
 * mul x, 2^k -> shl x, k
 */
static int pat_strength_pow2_mul(ir_func_t *fn, ir_node_t *n, int *exhausted) {
    long val;
    int k;
    
    if (n->op != IR_MUL) return 0;

    /* Check src2 */
    if (get_const(fn, n->src2, &val) && is_power_of_two(val, &k)) {
        n->op = IR_SHL;
        char tmp[32];
        ir_fresh_tmp(fn, tmp);
        
        /* Create CONST node for the shift amount right before n */
        ir_node_t *k_node = ir_node_alloc();
        k_node->op = IR_CONST;
        k_node->type = n->type;
        k_node->imm = k;
        strncpy(k_node->dst, tmp, IR_NAME_MAX - 1);
        
        /* Insert k_node before n */
        /* Since we don't have prev pointers, we find it: */
        if (fn->head == n) {
            k_node->next = n;
            fn->head = k_node;
        } else {
            ir_node_t *prev = fn->head;
            while (prev && prev->next != n) prev = prev->next;
            if (prev) {
                k_node->next = n;
                prev->next = k_node;
            }
        }
        fn->node_count++;
        
        /* Update n to use the new shift amount */
        strncpy(n->src2, tmp, IR_NAME_MAX - 1);
        return 1;
    }
    
    /* Check src1 */
    if (get_const(fn, n->src1, &val) && is_power_of_two(val, &k)) {
        n->op = IR_SHL;
        char tmp[32];
        ir_fresh_tmp(fn, tmp);
        
        /* Create CONST node for the shift amount */
        ir_node_t *k_node = ir_node_alloc();
        k_node->op = IR_CONST;
        k_node->type = n->type;
        k_node->imm = k;
        strncpy(k_node->dst, tmp, IR_NAME_MAX - 1);
        
        if (fn->head == n) {
            k_node->next = n;
            fn->head = k_node;
        } else {
            ir_node_t *prev = fn->head;
            while (prev && prev->next != n) prev = prev->next;
            if (prev) {
                k_node->next = n;
                prev->next = k_node;
            }
        }
        fn->node_count++;
        
        /* Swap operands: src1 becomes the operand, src2 becomes the shift amount */
        strncpy(n->src1, n->src2, IR_NAME_MAX - 1);
        strncpy(n->src2, tmp, IR_NAME_MAX - 1);
        return 1;
    }

    return 0;
}

/* Pattern 2: double_neg
 * neg(neg x) -> copy x
 * not(not x) -> copy x
 */
static int pat_double_neg(ir_func_t *fn, ir_node_t *n, int *exhausted) {
    if (n->op != IR_NEG && n->op != IR_NOT) return 0;
    
    /* Find the definition of src1 */
    ir_node_t *def = NULL;
    ir_node_t *scan;
    for (scan = fn->head; scan && scan != n; scan = scan->next) {
        if (strcmp(scan->dst, n->src1) == 0) {
            def = scan;
        }
    }
    
    if (def && def->op == n->op) {
        /* Double negation found! n = neg(def), def = neg(x) */
        /* Rewrite n: dst = COPY(def->src1) */
        n->op = IR_COPY;
        strncpy(n->src1, def->src1, IR_NAME_MAX - 1);
        n->src2[0] = '\0';
        return 1;
    }
    
    return 0;
}

/* ------------------------------------------------------------------ *
 * Registered Patterns Array
 * ------------------------------------------------------------------ */

/* Pattern 3: redundant_load_after_store
 * IR_STORE *addr = val -> IR_LOAD new_val = *addr
 * replace load with IR_COPY new_val = val
 */
static int pat_redundant_load_after_store(ir_func_t *fn, ir_node_t *n, int *exhausted) {
    if (n->op != IR_STORE) return 0;
    
    char addr[IR_NAME_MAX+1];
    char val[IR_NAME_MAX+1];
    strncpy(addr, n->dst, IR_NAME_MAX);
    strncpy(val, n->src1, IR_NAME_MAX);
    
    int lookahead = 0;
    ir_node_t *curr = n->next;
    while (curr) {
        /* Basic Block boundaries - stop checking */
        if (curr->op == IR_LABEL || curr->op == IR_BR || curr->op == IR_BR_IF) {
            break;
        }
        
        if (lookahead >= 50) {
            if (exhausted) (*exhausted)++;
            break;
        }

        if (curr->op == IR_LOAD && strcmp(curr->src1, addr) == 0) {
            curr->op = IR_COPY;
            strncpy(curr->src1, val, IR_NAME_MAX);
            return 1;
        }
        if (curr->op == IR_STORE || curr->op == IR_CALL || curr->op == IR_ASM || curr->op == IR_RET) {
            break; /* State modified or opaque */
        }
        if (strcmp(curr->dst, addr) == 0 || strcmp(curr->dst, val) == 0) {
            break; /* Address or value variable overwritten */
        }
        curr = curr->next;
        lookahead++;
    }
    return 0;
}

/* Pattern 4: cast_collapse
 * dst1 = (type)src1 -> dst2 = (type)dst1
 * replace dst2's cast with src1
 */
static int pat_cast_collapse(ir_func_t *fn, ir_node_t *n, int *exhausted) {
    if (n->op != IR_CAST) return 0;
    
    char dst[IR_NAME_MAX+1];
    char src[IR_NAME_MAX+1];
    strncpy(dst, n->dst, IR_NAME_MAX);
    strncpy(src, n->src1, IR_NAME_MAX);
    
    int lookahead = 0;
    ir_node_t *curr = n->next;
    while (curr) {
        /* Basic Block boundaries - stop checking */
        if (curr->op == IR_LABEL || curr->op == IR_BR || curr->op == IR_BR_IF) {
            break;
        }
        
        if (lookahead >= 50) {
            if (exhausted) (*exhausted)++;
            break;
        }

        if (curr->op == IR_CAST && strcmp(curr->src1, dst) == 0) {
            strncpy(curr->src1, src, IR_NAME_MAX);
            return 1;
        }
        
        /* State modified or opaque boundaries */
        if (curr->op == IR_STORE || curr->op == IR_CALL || curr->op == IR_ASM || curr->op == IR_RET) {
            break; 
        }

        if (strcmp(curr->dst, dst) == 0 || strcmp(curr->dst, src) == 0) {
            break;
        }
        curr = curr->next;
        lookahead++;
    }
    return 0;
}

/* Pattern 5: branch_chain_collapse
 * IR_BR L1 -> ... L1: -> IR_BR L2
 * replace IR_BR L1 with IR_BR L2
 */
static int pat_branch_chain_collapse(ir_func_t *fn, ir_node_t *n, int *exhausted) {
    if (n->op != IR_BR) return 0;
    if (n->src1[0] == '\0') return 0;
    
    int lookahead = 0;
    ir_node_t *targ = fn->head;
    while (targ) {
        if (lookahead >= 500) { /* For full fn scan, higher bound to prevent timeouts, but still tractable */
            if (exhausted) (*exhausted)++;
            break;
        }

        if (targ->op == IR_LABEL && strcmp(targ->src1, n->src1) == 0) {
            ir_node_t *next_op = targ->next;
            while (next_op && next_op->op == IR_NOP) next_op = next_op->next;
            if (next_op && next_op->op == IR_BR && strcmp(n->src1, next_op->src1) != 0) {
                strncpy(n->src1, next_op->src1, IR_NAME_MAX);
                return 1;
            }
            break;
        }
        targ = targ->next;
        lookahead++;
    }
    return 0;
}

/* ------------------------------------------------------------------ *
 * Registered Patterns Array
 * ------------------------------------------------------------------ */
static const ir_peephole_pattern_t s_patterns[] = {
    {"strength_pow2_mul", IR_PEEP_STRENGTH_POW2, pat_strength_pow2_mul},
    {"double_neg",        IR_PEEP_DOUBLE_NEG,    pat_double_neg},
    {"redundant_load_after_store", IR_PEEP_REDUNDANT_LOAD, pat_redundant_load_after_store},
    {"cast_collapse",     IR_PEEP_CAST_COLLAPSE, pat_cast_collapse},
    {"branch_chain_collapse", IR_PEEP_BRANCH_CHAIN, pat_branch_chain_collapse},
};

static const int s_num_patterns = sizeof(s_patterns) / sizeof(s_patterns[0]);

/* ------------------------------------------------------------------ *
 * Candidate Sort (Manifold Guided)
 * ------------------------------------------------------------------ */
typedef struct {
    ir_node_t *node;
    double     priority;
} ir_peep_candidate_t;

static int cmp_candidate(const void *a, const void *b) {
    const ir_peep_candidate_t *ca = (const ir_peep_candidate_t *)a;
    const ir_peep_candidate_t *cb = (const ir_peep_candidate_t *)b;
    if (ca->priority < cb->priority) return 1;
    if (ca->priority > cb->priority) return -1;
    return 0;
}

/* ------------------------------------------------------------------ *
 * Driver Loop
 * ------------------------------------------------------------------ */
ir_pass_result_t ir_peephole_run(ir_func_t *fn,
                                      ir_manifold_t *m,
                                      const ir_peephole_config_t *cfg) {
    ir_pass_result_t result;
    int iter, i, p;
    int total_fires = 0;
    int total_exhausted = 0;
    ir_node_t *n;
    
    memset(&result, 0, sizeof(result));
    if (!fn || fn->node_count == 0) return result;
    
    result.nodes_before = count_nodes(fn);
    
    for (iter = 0; iter < cfg->max_outer_passes; iter++) {
        
        if (iter == 0 && cfg->verbose) {
            fprintf(stderr, "[DEBUG] IR Dump for %s:\n", fn->name);
            for (n = fn->head; n; n = n->next) {
                fprintf(stderr, "  %s = node(op=%d, s1=%s, s2=%s) val=%ld\n", 
                        n->dst, n->op, n->src1, n->src2, n->imm);
            }
        }
        
        /* 1. Build position map */
        int node_count = count_nodes(fn);
        if (node_count == 0) break;
        
        ir_node_t **pos_map = (ir_node_t **)calloc(node_count, sizeof(ir_node_t *));
        int pos = 0;
        for (n = fn->head; n && pos < node_count; n = n->next) {
            pos_map[pos++] = n;
        }
        
        /* 2. Collect candidates */
        ir_peep_candidate_t *sites = (ir_peep_candidate_t *)calloc(node_count, sizeof(ir_peep_candidate_t));
        int site_count = 0;
        for (pos = 0; pos < node_count; pos++) {
            sites[site_count].node = pos_map[pos];
            sites[site_count].priority = 0.0;
            
            if (cfg->manifold_guided && m) {
                sites[site_count].priority = fabs(ir_manifold_score_node(m, pos));
            }
            site_count++;
        }
        
        /* 3. Sort sites by priority desc */
        if (cfg->manifold_guided && !cfg->deterministic) {
            qsort(sites, site_count, sizeof(ir_peep_candidate_t), cmp_candidate);
        }
        
        /* 4. Apply patterns */
        int fires = 0;
        for (i = 0; i < site_count; i++) {
            if (cfg->manifold_guided && sites[i].priority < cfg->gradient_threshold) {
                continue;
            }
            
            for (p = 0; p < s_num_patterns; p++) {
                if (!(s_patterns[p].klass & cfg->enabled_classes)) continue;
                
                if (s_patterns[p].try_apply(fn, sites[i].node, &total_exhausted)) {
                    fires++;
                    total_fires++;
                    result.nodes_modified++;
                    if (cfg->verbose) {
                        fprintf(stderr, "  [Peep] Fire %s -> nodes: %d\n", s_patterns[p].name, fn->node_count);
                    }
                    break;
                }
            }
            if (fires >= cfg->max_fires_per_iter) break;
        }
        
        free(pos_map);
        free(sites);
        
        if (fires == 0) {
            iter++; /* Account for the break */
            break;
        }
    }
    
    ir_telem_peephole(fn->name, iter, total_fires, total_exhausted);
    
    result.nodes_after = count_nodes(fn);
    result.changed = (result.nodes_after != result.nodes_before) || (result.nodes_modified > 0);
    return result;
}

/* Registry-compatible wrapper */
ir_pass_result_t ir_pass_peephole(void *fn_ptr) {
    ir_func_t *fn = (ir_func_t *)fn_ptr;
    ir_peephole_config_t cfg;
    
    ir_peephole_config_defaults(&cfg);
    cfg.deterministic = g_peephole_deterministic;
    cfg.verbose = g_peephole_verbose;
    
    /* Manifold guidance is ON if the manifold is available */
    if (g_peephole_manifold) {
        cfg.manifold_guided = 1;
    } else {
        cfg.manifold_guided = 0;
    }
    
    return ir_peephole_run(fn, g_peephole_manifold, &cfg);
}
