/*
 * ir_manifold.c -- ZKAEDI PRIME Hamiltonian Manifold Engine
 * ==========================================================
 * Implements the recursively-coupled Hamiltonian field projection
 * over the ZCC IR node graph. See ir_manifold.h for the full contract.
 *
 * Recurrence (per-node, 1-D field):
 *   H_t(n) = H_0(n)
 *           + eta  * H_{t-1}(n) * sigmoid(gamma * H_{t-1}(n))
 *           + sigma * N(0, 1 + beta * |H_{t-1}(n)|)
 *
 * PRNG: xorshift64, seeded from FNV-1a(func_name) XOR cfg.seed_salt
 *       -- no rand(), no libc state, deterministic across bootstrap stages.
 *
 * CG-IR-011 guard: evolve_step() checks func->node_count == 0 (proxy for
 *   used_regs == 0 from pass-manager context) and returns OK immediately,
 *   emitting IR_MANIFOLD_GUARD_SKIP to the telemetry log.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ir.h"
#include "ir_manifold.h"
#include "ir_telemetry.h"

/* ------------------------------------------------------------------ *
 * Internal limits
 * ------------------------------------------------------------------ */
#define MANIFOLD_INITIAL_CAP  256    /* initial node buffer capacity  */
#define MANIFOLD_USE_MAP_MAX  4096   /* flat use-count hash table     */

/* ------------------------------------------------------------------ *
 * PRNG -- xorshift64 (no libc state)
 * ------------------------------------------------------------------ */
typedef struct { uint64_t s; } xr64_t;

static uint64_t xr64_next(xr64_t *r) {
    uint64_t x = r->s;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return (r->s = x);
}

/* Box-Muller transform -- returns normal(0,1) variate */
static double xr64_normal(xr64_t *r) {
    /* Two uniform samples on (0,1] */
    double u1 = ((double)(xr64_next(r) >> 11) + 0.5) / (double)(1ULL << 53);
    double u2 = ((double)(xr64_next(r) >> 11) + 0.5) / (double)(1ULL << 53);
    return sqrt(-2.0 * log(u1)) * cos(6.28318530717958647 * u2);
}

/* ------------------------------------------------------------------ *
 * FNV-1a seed from function name
 * ------------------------------------------------------------------ */
static uint64_t fnv1a_64(const char *s) {
    uint64_t h = 14695981039346656037ULL;
    while (*s) {
        h ^= (uint64_t)(unsigned char)*s++;
        h *= 1099511628211ULL;
    }
    return h;
}

/* ------------------------------------------------------------------ *
 * Sigmoid primitive
 * ------------------------------------------------------------------ */
static double sigmoid(double x) {
    return 1.0 / (1.0 + exp(-x));
}

/* ------------------------------------------------------------------ *
 * Opcode class mapping
 * ------------------------------------------------------------------ */
static uint16_t opcode_class(ir_op_t op) {
    switch (op) {
    case IR_ADD: case IR_SUB: case IR_MUL: case IR_DIV:
    case IR_MOD: case IR_NEG: case IR_COPY:
        return IR_MFCLASS_ARITH;
    case IR_AND: case IR_OR: case IR_XOR:
    case IR_NOT: case IR_SHL: case IR_SHR:
        return IR_MFCLASS_LOGIC;
    case IR_EQ: case IR_NE: case IR_LT:
    case IR_LE: case IR_GT: case IR_GE:
        return IR_MFCLASS_CMP;
    case IR_LOAD: case IR_ALLOCA: case IR_ADDR:
        return IR_MFCLASS_MEM_LOAD;
    case IR_STORE:
        return IR_MFCLASS_MEM_STORE;
    case IR_BR: case IR_BR_IF: case IR_RET:
        return IR_MFCLASS_CTRL;
    case IR_CALL: case IR_ARG:
        return IR_MFCLASS_CALL;
    case IR_PHI:
        return IR_MFCLASS_PHI;
    case IR_CAST:
        return IR_MFCLASS_CAST;
    case IR_FADD: case IR_FSUB: case IR_FMUL: case IR_FDIV:
    case IR_ITOF: case IR_FTOI: case IR_FCONST:
        return IR_MFCLASS_FLOAT;
    case IR_ASM:
        return IR_MFCLASS_INTRIN;
    default:
        return IR_MFCLASS_UNKNOWN;
    }
}

/* ------------------------------------------------------------------ *
 * Use-count flat hash table  (temp name -> use count)
 * ------------------------------------------------------------------ */
typedef struct {
    char     name[IR_NAME_MAX];
    uint32_t count;
    int      used;
} use_slot_t;

typedef struct {
    use_slot_t slots[MANIFOLD_USE_MAP_MAX];
} use_map_t;

static void umap_clear(use_map_t *m) {
    memset(m, 0, sizeof(*m));
}

static uint32_t umap_hash(const char *s) {
    uint32_t h = 2166136261U;
    while (*s) { h ^= (unsigned char)*s++; h *= 16777619U; }
    return h % MANIFOLD_USE_MAP_MAX;
}

static void umap_inc(use_map_t *m, const char *name) {
    uint32_t h, i;
    if (!name || !name[0]) return;
    h = umap_hash(name);
    for (i = 0; i < MANIFOLD_USE_MAP_MAX; i++) {
        uint32_t idx = (h + i) % MANIFOLD_USE_MAP_MAX;
        if (!m->slots[idx].used) {
            strncpy(m->slots[idx].name, name, IR_NAME_MAX - 1);
            m->slots[idx].name[IR_NAME_MAX - 1] = '\0';
            m->slots[idx].count = 1;
            m->slots[idx].used  = 1;
            return;
        }
        if (strcmp(m->slots[idx].name, name) == 0) {
            m->slots[idx].count++;
            return;
        }
    }
    /* table full -- silently drop; feature degrades gracefully */
}

static uint32_t umap_get(const use_map_t *m, const char *name) {
    uint32_t h, i;
    if (!name || !name[0]) return 0;
    h = umap_hash(name);
    for (i = 0; i < MANIFOLD_USE_MAP_MAX; i++) {
        uint32_t idx = (h + i) % MANIFOLD_USE_MAP_MAX;
        if (!m->slots[idx].used)   return 0;
        if (strcmp(m->slots[idx].name, name) == 0)
            return m->slots[idx].count;
    }
    return 0;
}

/* ------------------------------------------------------------------ *
 * Concrete manifold struct (opaque to callers)
 * ------------------------------------------------------------------ */
struct ir_manifold {
    ir_manifold_config_t cfg;

    /* Feature table (indexed by sequential node position = node_id) */
    ir_manifold_features_t *features;
    size_t                  feat_count;
    size_t                  feat_cap;

    /* 1-D Hamiltonian field -- two buffers (ping-pong) */
    double *H;       /* current field   */
    double *H_base;  /* H_0 snapshot    */
    double *H_prev;  /* previous step   */
    size_t  field_cap;

    /* Evolution state */
    int     step;
    int     projected;
    int     converged;
    int     diverged;

    /* Per-pass bookkeeping */
    uint32_t last_pass_id;
    char     last_pass_name[64];

    /* PRNG */
    xr64_t prng;

    /* Func name cached for PRNG reseeding and export */
    char func_name[IR_FUNC_MAX];
    int  node_count_at_project;
};

/* ------------------------------------------------------------------ *
 * Telemetry shim -- ir_manifold writes simple stderr JSON events.
 * Follows the existing convention of ir_telem_pass() style; full
 * integration with HMAC envelope is a Phase 4 item.
 * ------------------------------------------------------------------ */

#define MANIFOLD_TELEM_EVENT_ID_BASE 0x4D00

typedef enum {
    MF_TELEM_INIT          = 0x4D00,
    MF_TELEM_DESTROY       = 0x4D01,
    MF_TELEM_PROJECT_BEGIN = 0x4D02,
    MF_TELEM_PROJECT_END   = 0x4D03,
    MF_TELEM_EVOLVE_STEP   = 0x4D04,
    MF_TELEM_CONVERGE      = 0x4D05,
    MF_TELEM_DIVERGE       = 0x4D06,
    MF_TELEM_BIFURCATION   = 0x4D07,
    MF_TELEM_PASS_BEGIN    = 0x4D09,
    MF_TELEM_PASS_END      = 0x4D0A,
    MF_TELEM_EXPORT        = 0x4D0B,
    MF_TELEM_GUARD_SKIP    = 0x4D0C
} mf_telem_id_t;

static void mf_emit(mf_telem_id_t id, const char *func,
                    const char *key, const char *val) {
    /* Only emits when telemetry is active; mirrors how ir_telem_pass() works.
     * Phase 4 will route through ir_telemetry_emit() with HMAC envelope. */
    const char *env = getenv("ZCC_EMIT_TELEMETRY");
    if (!env || env[0] == '0') return;
    fprintf(stderr,
        "{\"event\":\"0x%04X\",\"func\":\"%s\",\"%s\":\"%s\"}\n",
        (unsigned)id, func ? func : "", key, val ? val : "");
}

static void mf_emit_stats(mf_telem_id_t id, const char *func,
                          const ir_manifold_stats_t *s) {
    const char *env = getenv("ZCC_EMIT_TELEMETRY");
    if (!env || env[0] == '0') return;
    fprintf(stderr,
        "{\"event\":\"0x%04X\",\"func\":\"%s\","
        "\"step\":%d,\"energy\":%.6g,\"delta_l2\":%.6g,"
        "\"max_abs\":%.6g,\"converged\":%d,\"diverged\":%d}\n",
        (unsigned)id, func ? func : "",
        s->step, s->energy_total, s->energy_delta_l2,
        s->energy_max_abs, (int)s->converged, (int)s->diverged);
}

/* ------------------------------------------------------------------ *
 * Buffer helpers
 * ------------------------------------------------------------------ */
static int mf_alloc_field(ir_manifold_t *m, size_t n) {
    double *H     = (double *)calloc(n, sizeof(double));
    double *H_base= (double *)calloc(n, sizeof(double));
    double *H_prev= (double *)calloc(n, sizeof(double));
    ir_manifold_features_t *feat =
        (ir_manifold_features_t *)calloc(n, sizeof(ir_manifold_features_t));

    if (!H || !H_base || !H_prev || !feat) {
        free(H); free(H_base); free(H_prev); free(feat);
        return 0;
    }
    free(m->H);      m->H      = H;
    free(m->H_base); m->H_base = H_base;
    free(m->H_prev); m->H_prev = H_prev;
    free(m->features); m->features = feat;
    m->field_cap = n;
    m->feat_cap  = n;
    return 1;
}

/* ------------------------------------------------------------------ *
 * API: config defaults
 * ------------------------------------------------------------------ */
void ir_manifold_config_defaults(ir_manifold_config_t *cfg) {
    if (!cfg) return;
    memset(cfg, 0, sizeof(*cfg));
    cfg->eta            = IR_MANIFOLD_ETA_DEFAULT;
    cfg->gamma          = IR_MANIFOLD_GAMMA_DEFAULT;
    cfg->beta           = IR_MANIFOLD_BETA_DEFAULT;
    cfg->sigma          = IR_MANIFOLD_SIGMA_DEFAULT;
    cfg->eps            = IR_MANIFOLD_EPS_DEFAULT;
    cfg->max_steps      = IR_MANIFOLD_MAX_STEPS;
    cfg->seed_salt      = 0xDEADBEEFCAFEBABEULL;
    cfg->detect_bifurcation = 1;
    cfg->deterministic  = 0;
}

/* ------------------------------------------------------------------ *
 * API: create / destroy / reset
 * ------------------------------------------------------------------ */
ir_manifold_t *ir_manifold_create(const ir_manifold_config_t *cfg) {
    ir_manifold_t *m = (ir_manifold_t *)calloc(1, sizeof(ir_manifold_t));
    if (!m) return NULL;  /* bailout > abort */

    if (cfg) {
        m->cfg = *cfg;
    } else {
        ir_manifold_config_defaults(&m->cfg);
    }

    /* Deterministic mode forces sigma=0 */
    if (m->cfg.deterministic) m->cfg.sigma = 0.0;

    /* Pre-allocate field buffers at initial capacity */
    if (!mf_alloc_field(m, MANIFOLD_INITIAL_CAP)) {
        free(m);
        return NULL;
    }

    mf_emit(MF_TELEM_INIT, "", "status", "created");
    return m;
}

void ir_manifold_destroy(ir_manifold_t *m) {
    if (!m) return;
    mf_emit(MF_TELEM_DESTROY, m->func_name, "status", "destroyed");
    free(m->H);
    free(m->H_base);
    free(m->H_prev);
    free(m->features);
    free(m);
}

void ir_manifold_reset(ir_manifold_t *m) {
    if (!m) return;
    m->projected              = 0;
    m->step                   = 0;
    m->converged              = 0;
    m->diverged               = 0;
    m->feat_count             = 0;
    m->node_count_at_project  = 0;
    m->func_name[0]           = '\0';
    if (m->H)      memset(m->H,      0, m->field_cap * sizeof(double));
    if (m->H_base) memset(m->H_base, 0, m->field_cap * sizeof(double));
    if (m->H_prev) memset(m->H_prev, 0, m->field_cap * sizeof(double));
}

/* ------------------------------------------------------------------ *
 * H_0 initializer: composite feature scalar per node
 *
 * H_0(n) = ( in_degree * 0.3
 *           + out_degree * 0.2
 *           + opcode_class * 0.1
 *           + loop_depth * 0.25
 *           + dom_depth * 0.05
 *           + block_freq * 0.1 )
 *           / 1.0
 *
 * Chosen so that high-pressure nodes (many uses, deep loops) start
 * at higher potential, letting the Hamiltonian evolution amplify them.
 * ------------------------------------------------------------------ */
static double feature_to_h0(const ir_manifold_features_t *f) {
    return (f->in_degree   * 0.30
          + f->out_degree  * 0.20
          + f->opcode_class* 0.10
          + f->loop_depth  * 0.25
          + f->dom_depth   * 0.05
          + f->block_freq  * 0.10);
}

/* ------------------------------------------------------------------ *
 * API: project -- build feature table and H_0 from func node list
 * ------------------------------------------------------------------ */
ir_manifold_status_t
ir_manifold_project(ir_manifold_t *m, const ir_func_t *func) {
    use_map_t        *umap;
    const ir_node_t  *n;
    uint32_t          node_id;
    uint32_t          block_id;
    uint32_t          loop_depth;
    uint32_t          dom_depth;
    size_t            nc;

    if (!m)    return IR_MANIFOLD_OK;  /* NULL handle = no-op */
    if (!func) return IR_MANIFOLD_ERR_INVALID;

    mf_emit(MF_TELEM_PROJECT_BEGIN, func->name, "nodes",
            func->node_count > 0 ? "nonzero" : "zero");

    /* Empty function -- guard and return cleanly */
    if (func->node_count == 0) {
        m->projected = 1;
        m->feat_count = 0;
        mf_emit(MF_TELEM_PROJECT_END, func->name, "status", "empty");
        return IR_MANIFOLD_OK;
    }

    nc = (size_t)func->node_count;

    /* Grow buffers if needed */
    if (nc > m->field_cap) {
        if (!mf_alloc_field(m, nc + nc / 2)) {
            return IR_MANIFOLD_ERR_ALLOC;  /* degrade: project fails, compile continues */
        }
    }

    /* Seed PRNG from func name XOR salt */
    {
        uint64_t seed = fnv1a_64(func->name) ^ m->cfg.seed_salt;
        if (seed == 0) seed = 1;  /* xorshift64 collapses on zero */
        m->prng.s = seed;
    }
    strncpy(m->func_name, func->name, IR_FUNC_MAX - 1);
    m->func_name[IR_FUNC_MAX - 1] = '\0';

    /* --- Pass 1: build use-count map --- */
    umap = (use_map_t *)calloc(1, sizeof(use_map_t));
    if (!umap) return IR_MANIFOLD_ERR_ALLOC;
    umap_clear(umap);

    for (n = func->head; n; n = n->next) {
        umap_inc(umap, n->src1);
        umap_inc(umap, n->src2);
        /* IR_STORE: dst is a USE (address operand) */
        if (n->op == IR_STORE) umap_inc(umap, n->dst);
    }

    /* --- Pass 2: extract features --- */
    node_id   = 0;
    block_id  = 0;
    loop_depth= 0;
    dom_depth = 0;

    for (n = func->head; n; n = n->next, node_id++) {
        ir_manifold_features_t *f = &m->features[node_id];
        memset(f, 0, sizeof(*f));

        f->node_id      = node_id;
        f->opcode       = (uint16_t)n->op;
        f->opcode_class = opcode_class(n->op);
        f->block_freq   = 1.0f;  /* no profile data yet */

        /* Block leadership: a LABEL node begins a new basic block */
        if (n->op == IR_LABEL) {
            block_id++;
            f->is_leader    = 1;
            /* Reset dom_depth heuristic per block (simplified) */
            dom_depth = block_id / 2;  /* monotonically grows with blocks */
        }
        f->block_id = block_id;

        /* Loop depth heuristic: count label/br pairs.
         * Real CFG analysis is a Phase 2 item. */
        if (n->op == IR_BR_IF) loop_depth++;
        if (n->op == IR_BR && loop_depth > 0) loop_depth--;
        f->loop_depth = loop_depth;
        f->dom_depth  = dom_depth;

        /* Terminator detection */
        f->is_terminator = (uint8_t)ir_op_is_terminator(n->op);

        /* Use/def degrees */
        f->in_degree  = (n->dst[0] && n->op != IR_STORE)
                        ? umap_get(umap, n->dst) : 0;
        f->out_degree = (n->src1[0] ? 1 : 0) + (n->src2[0] ? 1 : 0);

        /* H_0 initial potential */
        m->H_base[node_id] = feature_to_h0(f);
        m->H[node_id]      = m->H_base[node_id];
        m->H_prev[node_id] = m->H_base[node_id];
    }

    free(umap);

    m->feat_count           = nc;
    m->node_count_at_project= (int)nc;
    m->projected            = 1;
    m->step                 = 0;
    m->converged            = 0;
    m->diverged             = 0;

    mf_emit(MF_TELEM_PROJECT_END, func->name, "status", "ok");
    return IR_MANIFOLD_OK;
}

/* ------------------------------------------------------------------ *
 * Features read-back
 * ------------------------------------------------------------------ */
const ir_manifold_features_t *
ir_manifold_features_data(const ir_manifold_t *m, size_t *out_count) {
    if (!m) { if (out_count) *out_count = 0; return NULL; }
    if (out_count) *out_count = m->feat_count;
    return m->features;
}

/* ------------------------------------------------------------------ *
 * API: evolve_step -- one PRIME recurrence iteration
 * ------------------------------------------------------------------ */
ir_manifold_status_t
ir_manifold_evolve_step(ir_manifold_t *m, ir_manifold_stats_t *out_stats) {
    size_t   i, N;
    double   delta_l2 = 0.0;
    double   sigma_eff;
    double   energy   = 0.0;
    double   max_abs  = 0.0;
    double   sig_sum  = 0.0;
    uint32_t bifurcations = 0;
    ir_manifold_stats_t stats;

    memset(&stats, 0, sizeof(stats));

    if (!m) return IR_MANIFOLD_OK;  /* NULL handle = no-op */

    /* CG-IR-011 guard: no nodes projected → guard skip */
    if (!m->projected || m->feat_count == 0 || m->node_count_at_project == 0) {
        mf_emit(MF_TELEM_GUARD_SKIP, m->func_name, "reason", "CG-IR-011");
        return IR_MANIFOLD_OK;
    }

    /* Already at fixpoint or diverged -- no-op */
    if (m->converged || m->diverged) return IR_MANIFOLD_OK;

    /* Step cap */
    if (m->step >= m->cfg.max_steps) {
        m->converged = 1;
        return IR_MANIFOLD_OK;
    }

    N = m->feat_count;
    sigma_eff = (m->cfg.deterministic || m->cfg.sigma == 0.0)
                ? 0.0 : m->cfg.sigma;

    /* Copy current to prev */
    memcpy(m->H_prev, m->H, N * sizeof(double));

    /* Core PRIME recurrence */
    for (i = 0; i < N; i++) {
        double h_old   = m->H_prev[i];
        double sig     = sigmoid(m->cfg.gamma * h_old);
        double noise   = 0.0;
        double h_new;

        if (sigma_eff > 0.0) {
            noise = sigma_eff
                  * xr64_normal(&m->prng)
                  * (1.0 + m->cfg.beta * fabs(h_old));
        }

        h_new = m->H_base[i]
              + m->cfg.eta * h_old * sig
              + noise;

        /* Divergence clamp */
        if (fabs(h_new) > IR_MANIFOLD_CLAMP) {
            m->diverged = 1;
            mf_emit(MF_TELEM_DIVERGE, m->func_name, "reason", "clamp");
            /* Freeze field at last stable values */
            memcpy(m->H, m->H_prev, N * sizeof(double));
            stats.diverged = 1;
            if (out_stats) *out_stats = stats;
            return IR_MANIFOLD_ERR_DIVERGED;
        }

        /* Bifurcation detection (sign flip) */
        if (m->cfg.detect_bifurcation) {
            if ((h_old > 0.0 && h_new < 0.0) ||
                (h_old < 0.0 && h_new > 0.0)) {
                bifurcations++;
            }
        }

        m->H[i] = h_new;
        energy   += h_new;
        max_abs   = fabs(h_new) > max_abs ? fabs(h_new) : max_abs;
        sig_sum  += sig;

        /* L2 delta accumulation */
        {
            double diff = h_new - h_old;
            delta_l2 += diff * diff;
        }
    }

    delta_l2 = sqrt(delta_l2);

    m->step++;

    /* Convergence check */
    if (delta_l2 < m->cfg.eps) {
        m->converged = 1;
        mf_emit(MF_TELEM_CONVERGE, m->func_name, "step",
                m->func_name);  /* step number embedded via stats below */
    }

    if (bifurcations > 0 && m->cfg.detect_bifurcation) {
        mf_emit(MF_TELEM_BIFURCATION, m->func_name, "count", "nonzero");
    }

    /* Fill stats */
    stats.step             = m->step;
    stats.energy_total     = energy;
    stats.energy_delta_l2  = delta_l2;
    stats.energy_max_abs   = max_abs;
    stats.sigmoid_mean     = (N > 0) ? sig_sum / (double)N : 0.0;
    stats.bifurcation_count= bifurcations;
    stats.nodes_visited    = (uint32_t)N;
    stats.converged        = (uint8_t)m->converged;
    stats.diverged         = 0;

    mf_emit_stats(MF_TELEM_EVOLVE_STEP, m->func_name, &stats);

    if (out_stats) *out_stats = stats;
    return IR_MANIFOLD_OK;
}

ir_manifold_status_t
ir_manifold_evolve_to_fixpoint(ir_manifold_t *m) {
    ir_manifold_status_t st;
    if (!m) return IR_MANIFOLD_OK;
    while (1) {
        st = ir_manifold_evolve_step(m, NULL);
        if (st != IR_MANIFOLD_OK) return st;
        if (m->converged || m->diverged) break;
        if (m->step >= m->cfg.max_steps) break;
    }
    return IR_MANIFOLD_OK;
}

/* ------------------------------------------------------------------ *
 * Queries (pure reads)
 * ------------------------------------------------------------------ */
double ir_manifold_score_node(const ir_manifold_t *m, uint32_t node_id) {
    if (!m || !m->projected || node_id >= (uint32_t)m->feat_count) return 0.0;
    return m->H[node_id];
}

double ir_manifold_score_block(const ir_manifold_t *m, uint32_t block_id) {
    double   sum   = 0.0;
    uint32_t count = 0;
    size_t   i;
    if (!m || !m->projected) return 0.0;
    for (i = 0; i < m->feat_count; i++) {
        if (m->features[i].block_id == block_id) {
            sum += m->H[i];
            count++;
        }
    }
    return count ? sum / (double)count : 0.0;
}

double ir_manifold_gradient(const ir_manifold_t *m,
                            uint32_t from_id, uint32_t to_id) {
    if (!m || !m->projected) return 0.0;
    if (from_id >= (uint32_t)m->feat_count) return 0.0;
    if (to_id   >= (uint32_t)m->feat_count) return 0.0;
    return m->H[to_id] - m->H[from_id];
}

size_t ir_manifold_field_snapshot(const ir_manifold_t *m,
                                  double *out, size_t cap) {
    size_t need, copy_n;
    if (!m || !m->projected) return 0;
    need   = m->feat_count;
    if (!out || cap == 0) return need;
    copy_n = need < cap ? need : cap;
    memcpy(out, m->H, copy_n * sizeof(double));
    return need;
}

void ir_manifold_current_stats(const ir_manifold_t *m,
                               ir_manifold_stats_t *out) {
    size_t  i, N;
    double  energy = 0.0, max_abs = 0.0, sig_sum = 0.0;
    if (!out) return;
    memset(out, 0, sizeof(*out));
    if (!m || !m->projected) return;
    N = m->feat_count;
    for (i = 0; i < N; i++) {
        double h = m->H[i];
        double a = fabs(h);
        energy  += h;
        if (a > max_abs) max_abs = a;
        sig_sum += sigmoid(m->cfg.gamma * h);
    }
    out->step            = m->step;
    out->energy_total    = energy;
    out->energy_max_abs  = max_abs;
    out->sigmoid_mean    = N ? sig_sum / (double)N : 0.0;
    out->nodes_visited   = (uint32_t)N;
    out->converged       = (uint8_t)m->converged;
    out->diverged        = (uint8_t)m->diverged;
}

/* ------------------------------------------------------------------ *
 * Pass-manager hooks
 * ------------------------------------------------------------------ */
void ir_manifold_on_pass_begin(ir_manifold_t *m,
                               uint32_t pass_id,
                               const char *pass_name) {
    if (!m) return;
    m->last_pass_id = pass_id;
    if (pass_name) {
        strncpy(m->last_pass_name, pass_name, 63);
        m->last_pass_name[63] = '\0';
    }
    mf_emit(MF_TELEM_PASS_BEGIN, m->func_name,
            "pass", pass_name ? pass_name : "");
}

void ir_manifold_on_pass_end(ir_manifold_t *m,
                             uint32_t pass_id,
                             uint32_t nodes_removed,
                             uint32_t nodes_added) {
    char buf[32];
    if (!m) return;
    (void)pass_id;
    /* Adjust feat_count to track node deletions between evolve steps */
    if (nodes_removed > 0 && nodes_removed <= (uint32_t)m->feat_count)
        m->feat_count -= nodes_removed;
    if (nodes_added > 0)
        m->feat_count += nodes_added;

    snprintf(buf, sizeof(buf), "%u", nodes_removed);
    mf_emit(MF_TELEM_PASS_END, m->func_name, "removed", buf);
}

/* ------------------------------------------------------------------ *
 * Corpus export (JSONL, one record per func, zcc-ir-prime-v2 format)
 * ------------------------------------------------------------------ */
ir_manifold_status_t
ir_manifold_export_corpus(const ir_manifold_t *m,
                          FILE *out,
                          const char *func_sig) {
    size_t i;
    if (!m || !out || !m->projected) return IR_MANIFOLD_OK;

    fprintf(out,
        "{\"schema\":\"ir_manifold_v1\","
        "\"func\":\"%s\","
        "\"func_sig\":\"%s\","
        "\"step\":%d,"
        "\"converged\":%d,"
        "\"diverged\":%d,"
        "\"eta\":%.6g,"
        "\"gamma\":%.6g,"
        "\"beta\":%.6g,"
        "\"sigma\":%.6g,"
        "\"node_count\":%zu,"
        "\"field\":[",
        m->func_name,
        func_sig ? func_sig : m->func_name,
        m->step,
        m->converged,
        m->diverged,
        m->cfg.eta, m->cfg.gamma, m->cfg.beta, m->cfg.sigma,
        m->feat_count);

    for (i = 0; i < m->feat_count; i++) {
        if (i) fputc(',', out);
        fprintf(out, "%.6g", m->H[i]);
    }

    fprintf(out,
        "],\"features\":[");

    for (i = 0; i < m->feat_count; i++) {
        const ir_manifold_features_t *f = &m->features[i];
        if (i) fputc(',', out);
        fprintf(out,
            "{\"id\":%u,\"bid\":%u,\"op\":%u,\"cls\":%u,"
            "\"in\":%u,\"out\":%u,\"ld\":%u,\"dd\":%u,"
            "\"ldr\":%u,\"term\":%u}",
            f->node_id, f->block_id, f->opcode, f->opcode_class,
            f->in_degree, f->out_degree, f->loop_depth, f->dom_depth,
            f->is_leader, f->is_terminator);
    }

    fprintf(out, "]}\n");

    if (ferror(out)) return IR_MANIFOLD_ERR_IO;

    mf_emit(MF_TELEM_EXPORT, m->func_name, "status", "ok");
    return IR_MANIFOLD_OK;
}
