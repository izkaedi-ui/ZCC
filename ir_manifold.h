/* ir_manifold.h — ZKAEDI PRIME Recursively Coupled Hamiltonian projection
 *                 over the ZCC IR. Produces topology tensors and per-node
 *                 energy scores consumable by the pass manager and by the
 *                 zcc-ir-prime training corpus exporter.
 *
 * Recurrence:
 *   H_t(n) = H_0(n) + eta * H_{t-1}(n) * sigmoid(gamma * H_{t-1}(n))
 *                   + sigma * N(0, 1 + beta * |H_{t-1}(n)|)
 *
 * Integration contract:
 *   - Allocated once per translation unit in ir_pm_run_default()
 *   - project() runs after CFG construction, before the first optimizing pass
 *   - evolve_step() runs once per pass-manager iteration
 *   - score_*() are pure reads; safe to call from any pass
 *   - export_corpus() runs at end-of-TU, gated behind --ir-export=path
 *
 * Determinism:
 *   - PRNG seeded from FNV-1a(func_name) XOR cfg.seed_salt
 *   - cfg.sigma = 0.0 disables the stochastic term entirely
 *   - Bootstrap parity (zcc2.s == zcc3.s) is preserved when sigma == 0
 *
 * Guards:
 *   - CG-IR-011: evolve_step() is a no-op on funcs entering with used_regs == 0
 *   - Divergence clamp: |H| > IR_MANIFOLD_CLAMP freezes further evolution
 *   - Empty funcs (node_count == 0): all ops return IR_MANIFOLD_OK, do nothing
 *   - Allocation failure: create() NULL is valid; all subsequent calls are no-ops
 */

#ifndef IR_MANIFOLD_H
#define IR_MANIFOLD_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "ir.h"           /* ir_func_t, ir_node_t, ir_op_t  */

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ *
 * Return codes
 * ------------------------------------------------------------------ */
typedef enum {
    IR_MANIFOLD_OK              =  0,
    IR_MANIFOLD_ERR_ALLOC       = -1,
    IR_MANIFOLD_ERR_INVALID     = -2,
    IR_MANIFOLD_ERR_UNPROJECTED = -3,  /* evolve/score before project */
    IR_MANIFOLD_ERR_DIVERGED    = -4,  /* |H| exceeded clamp          */
    IR_MANIFOLD_ERR_IO          = -5   /* export write failure        */
} ir_manifold_status_t;

/* ------------------------------------------------------------------ *
 * Canonical PRIME defaults (match the solver in the skill doc)
 * ------------------------------------------------------------------ */
#define IR_MANIFOLD_ETA_DEFAULT     0.40
#define IR_MANIFOLD_GAMMA_DEFAULT   0.30
#define IR_MANIFOLD_BETA_DEFAULT    0.10
#define IR_MANIFOLD_SIGMA_DEFAULT   0.05
#define IR_MANIFOLD_EPS_DEFAULT     1.0e-6
#define IR_MANIFOLD_MAX_STEPS       32
#define IR_MANIFOLD_CLAMP           1.0e6

typedef struct ir_manifold_config {
    double   eta;                 /* eta  -- recursive feedback gain       */
    double   gamma;               /* gamma -- sigmoid steepness            */
    double   beta;                /* beta  -- noise-energy coupling        */
    double   sigma;               /* sigma -- noise amplitude (0 = det.)   */
    double   eps;                 /* eps   -- L2 convergence threshold     */
    int      max_steps;           /* cap on evolve iterations              */
    uint64_t seed_salt;           /* XOR'd into per-function PRNG seed     */
    uint8_t  detect_bifurcation;  /* emit bifurcation telemetry 0/1        */
    uint8_t  deterministic;       /* force sigma=0 regardless of cfg       */
    uint8_t  reserved[6];
} ir_manifold_config_t;

void ir_manifold_config_defaults(ir_manifold_config_t *cfg);

/* ------------------------------------------------------------------ *
 * Opcode classes (bit-packed into features, stable IDs for corpus)
 * ------------------------------------------------------------------ */
typedef enum {
    IR_MFCLASS_UNKNOWN   = 0,
    IR_MFCLASS_ARITH     = 1,
    IR_MFCLASS_LOGIC     = 2,
    IR_MFCLASS_CMP       = 3,
    IR_MFCLASS_MEM_LOAD  = 4,
    IR_MFCLASS_MEM_STORE = 5,
    IR_MFCLASS_CTRL      = 6,   /* br, jmp, ret  */
    IR_MFCLASS_CALL      = 7,
    IR_MFCLASS_PHI       = 8,
    IR_MFCLASS_CAST      = 9,
    IR_MFCLASS_FLOAT     = 10,
    IR_MFCLASS_INTRIN    = 11,
    IR_MFCLASS_MAX       = 12
} ir_manifold_class_t;

/* ------------------------------------------------------------------ *
 * Per-node topology features (input to H_0)
 * ------------------------------------------------------------------ */
typedef struct ir_manifold_features {
    uint32_t node_id;
    uint32_t block_id;
    uint16_t opcode;
    uint16_t opcode_class;     /* ir_manifold_class_t */
    uint32_t in_degree;        /* use count           */
    uint32_t out_degree;       /* def fanout          */
    uint32_t loop_depth;
    uint32_t dom_depth;
    float    block_freq;       /* profile weight, 1.0 if absent */
    uint8_t  is_leader;        /* block entry         */
    uint8_t  is_terminator;
    uint8_t  on_critical_edge;
    uint8_t  reserved;
} ir_manifold_features_t;

/* ------------------------------------------------------------------ *
 * Opaque context
 * ------------------------------------------------------------------ */
typedef struct ir_manifold ir_manifold_t;

/* ------------------------------------------------------------------ *
 * Per-step statistics (written by evolve_step, read by telemetry)
 * ------------------------------------------------------------------ */
typedef struct ir_manifold_stats {
    int      step;
    double   energy_total;       /* sum H_t                  */
    double   energy_delta_l2;    /* ||H_t - H_{t-1}||_2      */
    double   energy_max_abs;     /* max |H_t|                */
    double   sigmoid_mean;
    uint32_t bifurcation_count;  /* sign flips in grad       */
    uint32_t nodes_visited;
    uint8_t  converged;          /* delta < eps              */
    uint8_t  diverged;           /* max_abs > clamp          */
    uint8_t  reserved[6];
} ir_manifold_stats_t;

/* ------------------------------------------------------------------ *
 * Lifecycle
 * ------------------------------------------------------------------ */
ir_manifold_t *ir_manifold_create (const ir_manifold_config_t *cfg);
void           ir_manifold_destroy(ir_manifold_t *m);
void           ir_manifold_reset  (ir_manifold_t *m);

/* ------------------------------------------------------------------ *
 * Projection -- extract features, build H_0. Idempotent per func.
 * ------------------------------------------------------------------ */
ir_manifold_status_t
ir_manifold_project(ir_manifold_t *m, const ir_func_t *func);

const ir_manifold_features_t *
ir_manifold_features_data(const ir_manifold_t *m, size_t *out_count);

/* ------------------------------------------------------------------ *
 * Evolution
 * ------------------------------------------------------------------ */
ir_manifold_status_t
ir_manifold_evolve_step(ir_manifold_t *m, ir_manifold_stats_t *out_stats);

ir_manifold_status_t
ir_manifold_evolve_to_fixpoint(ir_manifold_t *m);

/* ------------------------------------------------------------------ *
 * Queries (pure reads)
 * ------------------------------------------------------------------ */
double ir_manifold_score_node (const ir_manifold_t *m, uint32_t node_id);
double ir_manifold_score_block(const ir_manifold_t *m, uint32_t block_id);
double ir_manifold_gradient   (const ir_manifold_t *m,
                               uint32_t from_id, uint32_t to_id);
size_t ir_manifold_field_snapshot(const ir_manifold_t *m,
                                  double *out, size_t cap);
void   ir_manifold_current_stats(const ir_manifold_t *m,
                                 ir_manifold_stats_t *out);

/* ------------------------------------------------------------------ *
 * Pass-manager hooks
 * ------------------------------------------------------------------ */
void ir_manifold_on_pass_begin(ir_manifold_t *m,
                               uint32_t pass_id,
                               const char *pass_name);
void ir_manifold_on_pass_end  (ir_manifold_t *m,
                               uint32_t pass_id,
                               uint32_t nodes_removed,
                               uint32_t nodes_added);

/* ------------------------------------------------------------------ *
 * Corpus export -- JSONL, one record per func.
 * ------------------------------------------------------------------ */
ir_manifold_status_t
ir_manifold_export_corpus(const ir_manifold_t *m,
                          FILE *out,
                          const char *func_sig);

#ifdef __cplusplus
}
#endif

#endif /* IR_MANIFOLD_H */
