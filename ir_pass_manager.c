/*
 * ir_pass_manager.c — ZCC IR Pass Manager Implementation
 * ========================================================
 * Compiled by GCC only (linked separately, NOT concatenated into zcc.c).
 * Operates on ir_func_t* linked lists defined in ir.h.
 *
 * Phase 2: stub passes that count nodes (no mutations).
 * Steps 4-6 will implement real DCE, constant folding, and strength reduction.
 *
 * Default pipeline: DCE → const_fold → strength_reduce → DCE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ir.h"

/* ── Pass result type (duplicated from ir_pass_manager.h to avoid
 *    include ordering issues with ir.h) ──────────────────────────────── */

typedef struct {
    int nodes_before;
    int nodes_after;
    int nodes_deleted;
    int nodes_modified;
    int changed;
} ir_pass_result_t;

typedef ir_pass_result_t (*ir_pass_fn)(void *fn_ptr);

#define IR_PM_MAX_PASSES 16

typedef struct {
    const char    *name;
    ir_pass_fn     fn;
    int            enabled;
} ir_pass_entry_t;

typedef struct {
    ir_pass_entry_t passes[IR_PM_MAX_PASSES];
    int             count;
    int             verbose;
} ir_pass_manager_t;

/* ── Helpers ─────────────────────────────────────────────────────────── */

static int count_nodes(ir_func_t *fn) {
    int count = 0;
    ir_node_t *n = fn->head;
    while (n) {
        count++;
        n = n->next;
    }
    return count;
}

/* ── Built-in passes (Phase 2: stubs, count nodes only) ──────────────── */

static ir_pass_result_t ir_pass_dce(void *fn_ptr) {
    ir_func_t *fn = (ir_func_t *)fn_ptr;
    ir_pass_result_t r;
    memset(&r, 0, sizeof(r));
    r.nodes_before = count_nodes(fn);
    /* Phase 2: no-op — real DCE in Step 4 */
    r.nodes_after = r.nodes_before;
    return r;
}

static ir_pass_result_t ir_pass_const_fold(void *fn_ptr) {
    ir_func_t *fn = (ir_func_t *)fn_ptr;
    ir_pass_result_t r;
    memset(&r, 0, sizeof(r));
    r.nodes_before = count_nodes(fn);
    /* Phase 2: no-op — real constant folding in Step 5 */
    r.nodes_after = r.nodes_before;
    return r;
}

static ir_pass_result_t ir_pass_strength_reduce(void *fn_ptr) {
    ir_func_t *fn = (ir_func_t *)fn_ptr;
    ir_pass_result_t r;
    memset(&r, 0, sizeof(r));
    r.nodes_before = count_nodes(fn);
    /* Phase 2: no-op — real strength reduction in Step 6 */
    r.nodes_after = r.nodes_before;
    return r;
}

/* ── Registry API ────────────────────────────────────────────────────── */

static ir_pass_manager_t *ir_pm_create(void) {
    ir_pass_manager_t *pm = (ir_pass_manager_t *)calloc(1, sizeof(ir_pass_manager_t));
    if (!pm) {
        fprintf(stderr, "ir_pm_create: out of memory\n");
        exit(1);
    }
    return pm;
}

static void ir_pm_register(ir_pass_manager_t *pm, const char *name, ir_pass_fn fn) {
    if (pm->count >= IR_PM_MAX_PASSES) {
        fprintf(stderr, "ir_pm_register: too many passes (max %d)\n", IR_PM_MAX_PASSES);
        return;
    }
    pm->passes[pm->count].name = name;
    pm->passes[pm->count].fn = fn;
    pm->passes[pm->count].enabled = 1;
    pm->count++;
}

static void ir_pm_run(ir_pass_manager_t *pm, ir_module_t *mod) {
    int p;
    int f;
    int total_nodes_in = 0;
    int total_nodes_out = 0;

    for (p = 0; p < pm->count; p++) {
        int pass_before = 0;
        int pass_after = 0;
        int pass_deleted = 0;
        int pass_modified = 0;

        if (!pm->passes[p].enabled) continue;

        for (f = 0; f < mod->func_count; f++) {
            ir_pass_result_t r = pm->passes[p].fn(mod->funcs[f]);
            pass_before += r.nodes_before;
            pass_after += r.nodes_after;
            pass_deleted += r.nodes_deleted;
            pass_modified += r.nodes_modified;
        }

        if (pm->verbose) {
            fprintf(stderr, "  [IR Pass] %-18s: %d funcs, %d nodes -> %d nodes (%d deleted, %d modified)\n",
                    pm->passes[p].name, mod->func_count, pass_before, pass_after,
                    pass_deleted, pass_modified);
        }

        if (p == 0) total_nodes_in = pass_before;
        total_nodes_out = pass_after;
    }

    if (pm->verbose && pm->count > 0) {
        fprintf(stderr, "  [IR Pass] Pipeline complete: %d nodes -> %d nodes\n",
                total_nodes_in, total_nodes_out);
    }
}

static void ir_pm_free(ir_pass_manager_t *pm) {
    if (pm) free(pm);
}

/* ── Primary entry point (called from part5.c) ──────────────────────── */

void ir_pm_run_default(void *mod_ptr, int verbose) {
    ir_module_t *mod = (ir_module_t *)mod_ptr;
    ir_pass_manager_t *pm;

    if (!mod || mod->func_count == 0) return;

    pm = ir_pm_create();
    pm->verbose = verbose;

    /* Default pipeline: DCE → const_fold → strength_reduce → DCE */
    ir_pm_register(pm, "dce", ir_pass_dce);
    ir_pm_register(pm, "const_fold", ir_pass_const_fold);
    ir_pm_register(pm, "strength_reduce", ir_pass_strength_reduce);
    ir_pm_register(pm, "dce2", ir_pass_dce);

    ir_pm_run(pm, mod);
    ir_pm_free(pm);
}
