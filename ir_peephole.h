/* ir_peephole.h — manifold-guided peephole pass for ZCC.
 *
 * Runs AFTER strength_reduce and BEFORE dce2 in the default pipeline.
 * Handles rewrite classes that strength_reduce does NOT cover:
 *   - Power-of-two multiply → shift
 *   - Redundant load-after-store (same address, no intervening store/call)
 *   - Branch-to-branch collapse
 *   - Redundant cast chains (sext(sext x) → sext x)
 *   - Double-negation / double-complement identities
 *
 * Manifold gradient drives candidate ordering only. Correctness is
 * carried by the pattern table; the manifold can be wrong and the worst
 * outcome is suboptimal code, never miscompilation.
 *
 * Bootstrap safety: --peephole-deterministic forces fixed iteration order
 * (CFG preorder, pattern index order). Required for zcc2.s == zcc3.s parity.
 */

#ifndef ZCC_IR_PEEPHOLE_H
#define ZCC_IR_PEEPHOLE_H

#include <stdint.h>
#include "ir.h"
#include "ir_manifold.h"        /* ir_manifold_t, gradient queries  */

#include "ir_pass_manager.h"    /* ir_pass_result_t, ir_pass_fn     */

#define IR_PEEP_MAX_PATTERNS     32
#define IR_PEEP_MAX_OUTER_PASSES  8
#define IR_PEEP_MAX_FIRES_PER_IT  4096

typedef enum {
    IR_PEEP_STRENGTH_POW2  = 0x01,  /* x*2^k → x<<k                  */
    IR_PEEP_REDUNDANT_LOAD = 0x02,  /* store p v; ... load p → v     */
    IR_PEEP_BRANCH_CHAIN   = 0x04,  /* br L1; L1: br L2 → br L2      */
    IR_PEEP_CAST_COLLAPSE  = 0x08,  /* cast(cast x) → cast x         */
    IR_PEEP_DOUBLE_NEG     = 0x10,  /* neg(neg x) / not(not x) → x   */
    IR_PEEP_ALL            = 0xFF
} ir_peep_class_t;

typedef struct ir_peephole_pattern {
    const char      *name;
    ir_peep_class_t  klass;
    /* try_apply mutates node in place (preferred, like strength_reduce)
     * OR unlinks it from fn (like DCE). Returns 1 on fire, 0 on no-match.
     * MUST NOT modify fn on mismatch. */
    int (*try_apply)(ir_func_t *fn, ir_node_t *n, int *exhausted);
} ir_peephole_pattern_t;

typedef struct ir_peephole_config {
    uint32_t        enabled_classes;     /* bitmask; default IR_PEEP_ALL  */
    double          gradient_threshold;  /* skip sites with |grad| below  */
    int             max_outer_passes;    /* default 8                     */
    int             max_fires_per_iter;  /* default 4096                  */
    uint8_t         manifold_guided;     /* 1 = sort by gradient          */
    uint8_t         deterministic;       /* bootstrap parity mode         */
    uint8_t         verbose;
    uint8_t         reserved;
} ir_peephole_config_t;

void ir_peephole_config_defaults(ir_peephole_config_t *cfg);

#ifdef __cplusplus
extern "C" {
#endif

/* Global configuration flags and setters */
extern int g_peephole_enabled;
extern int g_peephole_deterministic;
extern int g_peephole_verbose;

void ir_peephole_set_manifold(ir_manifold_t *m);

/* Direct invocation (for testing / non-pipeline callers) */
ir_pass_result_t ir_peephole_run(ir_func_t *fn,
                                      ir_manifold_t *m,        /* nullable    */
                                      const ir_peephole_config_t *cfg);

/* Registry-compatible wrapper */
ir_pass_result_t ir_pass_peephole(void *fn_ptr);

#ifdef __cplusplus
}
#endif

#endif
