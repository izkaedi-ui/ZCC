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
 *   cc -O2 -std=c17 -Wall -Wextra compiler_passes.c -o passes -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

/* ─────────────────────────────────────────────────────────────────────────────
 * SHARED IR PRIMITIVES
 * ─────────────────────────────────────────────────────────────────────────── */

#define MAX_OPERANDS    4
#define MAX_PHI_SOURCES 32
#define MAX_SUCCS       2
#define MAX_PREDS       16
#define MAX_INSTRS      4096
#define MAX_BLOCKS      512
#define MAX_ALLOCS      256
#define MAX_LOOPS       64
#define MAX_LOOP_BLOCKS 256
#define NAME_LEN        64

#define NO_BLOCK        0xFFFFFFFFu   /* sentinel: absent block ID    */
#define NO_ALLOC        0xFFFFFFFFu   /* sentinel: absent alloca ID   */

typedef uint32_t RegID;    /* virtual register identifier            */
typedef uint32_t BlockID;  /* basic block identifier                 */
typedef uint32_t InstrID;  /* instruction identifier within a block  */

typedef enum {
    OP_NOP = 0,
    OP_CONST,      /* immediate constant (Phase B lowering)           */
    OP_PHI,        /* φ(r₀:B₀, r₁:B₁, …)                */
    OP_ADD, OP_SUB, OP_MUL, OP_DIV,
    OP_LT,         /* less-than (Phase B lowering)                     */
    OP_LOAD, OP_STORE,
    OP_ALLOCA,     /* stack allocation — escape candidate  */
    OP_GEP,        /* GetElementPtr — tracks escape        */
    OP_CALL,       /* may cause escape                     */
    OP_RET,
    OP_BR,         /* unconditional branch                 */
    OP_CONDBR,     /* conditional branch                   */
    OP_COPY,
    OP_UNDEF,
} Opcode;

static const char *opcode_name[] __attribute__((unused)) = {
    "nop","const","phi","add","sub","mul","div","lt",
    "load","store","alloca","gep","call",
    "ret","br","condbr","copy","undef"
};

typedef struct {
    RegID   reg;     /* source register                    */
    BlockID block;   /* predecessor block the value comes from */
} PhiSource;

typedef struct Instr {
    InstrID  id;
    Opcode   op;
    RegID    dst;            /* destination register (0 = no def)   */
    RegID    src[MAX_OPERANDS];
    uint32_t n_src;

    /* φ-node sources (populated only when op == OP_PHI) */
    PhiSource phi[MAX_PHI_SOURCES];
    uint32_t  n_phi;

    /* Immediate (OP_CONST, or e.g. alloca size; 0 if unused) */
    int64_t   imm;

    /* Metadata */
    bool     dead;           /* marked by DCE                       */
    bool     escape;         /* marked by escape analysis           */
    double   exec_freq;      /* from PGO profile                    */

    struct Instr *next;
    struct Instr *prev;
} Instr;

typedef struct Block {
    BlockID  id;
    char     name[NAME_LEN];

    /* Instruction doubly-linked list */
    Instr   *head;
    Instr   *tail;
    uint32_t n_instrs;

    /* CFG edges */
    BlockID  succs[MAX_SUCCS];
    uint32_t n_succs;
    BlockID  preds[MAX_PREDS];
    uint32_t n_preds;

    /* Branch probabilities (succs[i] → branch_prob[i]) */
    double   branch_prob[MAX_SUCCS];

    /* Liveness (DCE / SSA pass) */
    uint64_t live_in[8];   /* bitset — up to 512 regs                */
    uint64_t live_out[8];

    /* PGO / reordering metadata */
    double   exec_freq;    /* estimated execution frequency          */
    bool     placed;       /* used during chain construction         */
    BlockID  chain_next;   /* next block in the PGO chain            */

    bool     reachable;    /* set by CFG reachability pass           */
} Block;

typedef struct {
    Block   *blocks[MAX_BLOCKS];
    uint32_t n_blocks;
    BlockID  entry;
    BlockID  exit;

    /* Def-use chains (reg → defining instruction) */
    Instr   *def_of[MAX_INSTRS];    /* indexed by RegID */
    BlockID  def_block[MAX_INSTRS]; /* block housing def_of[r]; NO_BLOCK if arg */
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
 * ─────────────────────────────────────────────────────────────────────────── */

static inline void  bs_set  (uint64_t *bs, RegID r){ bs[r>>6] |=  (1ULL<<(r&63)); }
static inline void  bs_clear(uint64_t *bs, RegID r){ bs[r>>6] &= ~(1ULL<<(r&63)); }
static inline bool  bs_get  (uint64_t *bs, RegID r){ return !!(bs[r>>6] & (1ULL<<(r&63))); }
static inline bool  bs_empty(uint64_t *bs)         {
    for(int i=0;i<8;i++) if(bs[i]) return false;
    return true;
}
static inline bool  bs_union(uint64_t *dst, const uint64_t *src) {
    bool changed = false;
    for(int i=0;i<8;i++){
        uint64_t before = dst[i];
        dst[i] |= src[i];
        changed |= (dst[i] != before);
    }
    return changed;
}
static inline void  bs_copy (uint64_t *dst, const uint64_t *src){
    memcpy(dst, src, 8*sizeof(uint64_t));
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
 *   1. Mark all registers used by critical instructions as LIVE (seed worklist).
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
 * ─────────────────────────────────────────────────────────────────────────── */

/* Worklist for backward liveness propagation */
typedef struct {
    RegID  items[MAX_INSTRS];
    int    head, tail, size;
} RegWorklist;

static void wl_push(RegWorklist *wl, RegID r){
    assert(wl->size < MAX_INSTRS);
    wl->items[wl->tail] = r;
    wl->tail = (wl->tail + 1) % MAX_INSTRS;
    wl->size++;
}
static RegID wl_pop(RegWorklist *wl){
    assert(wl->size > 0);
    RegID r = wl->items[wl->head];
    wl->head = (wl->head + 1) % MAX_INSTRS;
    wl->size--;
    return r;
}

/* Determine if an instruction is a critical side-effect anchor */
static bool is_critical(const Instr *ins){
    return ins->op == OP_STORE
        || ins->op == OP_CALL
        || ins->op == OP_RET
        || ins->op == OP_BR
        || ins->op == OP_CONDBR;
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
 * ─────────────────────────────────────────────────────────────────────────── */

/* ── Block-membership bitset (MAX_BLOCKS=512 → 8 × 64-bit words) ── */
#define BLKSET_WORDS 8
typedef uint64_t BlkSet[BLKSET_WORDS];

static inline void blkset_add (BlkSet bs, BlockID b) { bs[b>>6] |=  (1ULL<<(b&63)); }
static inline bool blkset_has (const BlkSet bs, BlockID b)
                                                     { return !!(bs[b>>6]&(1ULL<<(b&63))); }
static inline void blkset_zero(BlkSet bs)            { memset(bs,0,BLKSET_WORDS*sizeof(uint64_t)); }
static inline void blkset_merge(BlkSet dst,const BlkSet src)
                                                     { for(int i=0;i<BLKSET_WORDS;i++) dst[i]|=src[i]; }

/* ── RPO state (module-level, single-threaded) ── */
static uint32_t licm_rpo_arr[MAX_BLOCKS];   /* rpo_arr[rpo_idx] = BlockID  */
static uint32_t licm_rpo_of [MAX_BLOCKS];   /* rpo_of[BlockID]  = rpo_idx  */
static uint32_t licm_rpo_n;

/* ── Dominator tree ── */
static BlockID  licm_idom[MAX_BLOCKS];       /* licm_idom[b] = imm-dom of b */

/* ── Natural loop descriptor ── */
typedef struct {
    BlockID  header;
    BlockID  preheader;
    bool     has_preheader;
    BlkSet   body_bs;                        /* bitset of block IDs in loop  */
    BlockID  latches[MAX_LOOP_BLOCKS];
    uint32_t n_latches;
} LICMLoop;

/* ═════════════════════════════════════════════════════════════════
 * A.  Build fn->def_block[] and refresh fn->def_of[]
 * ═════════════════════════════════════════════════════════════════ */
static void licm_build_def_block(Function *fn)
{
    memset(fn->def_block, 0xFF, sizeof(fn->def_block));   /* NO_BLOCK */
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        Block *blk = fn->blocks[bi];
        if(!blk || !blk->reachable) continue;
        for(Instr *ins = blk->head; ins; ins = ins->next){
            if(ins->dst && ins->dst < MAX_INSTRS){
                fn->def_block[ins->dst] = bi;
                fn->def_of   [ins->dst] = ins;
            }
        }
    }
}

/* ═════════════════════════════════════════════════════════════════
 * B.  Iterative DFS post-order → RPO
 *     Iterative to avoid call-stack overflow on large CFGs.
 * ═════════════════════════════════════════════════════════════════ */
static void licm_compute_rpo(Function *fn)
{
    static bool     visited [MAX_BLOCKS];
    static uint32_t post    [MAX_BLOCKS];
    static BlockID  stk     [MAX_BLOCKS];
    static bool     expanded[MAX_BLOCKS];
    uint32_t post_n = 0;
    int top = -1;

    memset(visited,  0, fn->n_blocks * sizeof(bool));
    memset(licm_rpo_of, 0xFF, sizeof(licm_rpo_of));

    stk[++top] = fn->entry;
    expanded[top] = false;

    while(top >= 0){
        BlockID b = stk[top];
        if(!expanded[top]){
            if(visited[b]){ top--; continue; }
            visited[b]  = true;
            expanded[top] = true;
            Block *blk = fn->blocks[b];
            /* push successors in reverse so first succ is processed first */
            for(int si = (int)blk->n_succs - 1; si >= 0; si--){
                BlockID s = blk->succs[si];
                if(s < fn->n_blocks && !visited[s] && top + 1 < MAX_BLOCKS){
                    stk[++top]    = s;
                    expanded[top] = false;
                }
            }
        } else {
            post[post_n++] = b;
            top--;
        }
    }

    licm_rpo_n = post_n;
    for(uint32_t i = 0; i < licm_rpo_n; i++){
        licm_rpo_arr[i]           = post[licm_rpo_n - 1 - i];
        licm_rpo_of [post[licm_rpo_n - 1 - i]] = i;
    }
}

/* ═════════════════════════════════════════════════════════════════
 * C.  Cooper et al. 2001 iterative dominator tree
 * ═════════════════════════════════════════════════════════════════ */
static BlockID licm_intersect(BlockID b1, BlockID b2)
{
    uint32_t f1 = licm_rpo_of[b1], f2 = licm_rpo_of[b2];
    while(f1 != f2){
        while(f1 > f2){
            b1 = licm_idom[b1];
            if(b1 == NO_BLOCK) return NO_BLOCK;
            f1 = licm_rpo_of[b1];
        }
        while(f2 > f1){
            b2 = licm_idom[b2];
            if(b2 == NO_BLOCK) return NO_BLOCK;
            f2 = licm_rpo_of[b2];
        }
    }
    return b1;
}

static void licm_compute_doms(Function *fn)
{
    for(uint32_t i = 0; i < fn->n_blocks; i++) licm_idom[i] = NO_BLOCK;
    licm_idom[fn->entry] = fn->entry;

    bool changed = true;
    while(changed){
        changed = false;
        /* Iterate in RPO order; skip entry (index 0) */
        for(uint32_t ri = 1; ri < licm_rpo_n; ri++){
            BlockID b   = licm_rpo_arr[ri];
            Block  *blk = fn->blocks[b];
            if(!blk || !blk->reachable) continue;

            BlockID new_idom = NO_BLOCK;
            for(uint32_t pi = 0; pi < blk->n_preds; pi++){
                BlockID p = blk->preds[pi];
                if(p >= fn->n_blocks || licm_idom[p] == NO_BLOCK) continue;
                if(new_idom == NO_BLOCK) new_idom = p;
                else                     new_idom = licm_intersect(p, new_idom);
            }
            if(new_idom != NO_BLOCK && licm_idom[b] != new_idom){
                licm_idom[b] = new_idom;
                changed = true;
            }
        }
    }
}

/* Returns true if 'd' dominates 'b' (walks idom chain from b to root). */
static bool licm_dominates(BlockID d, BlockID b)
{
    uint32_t guard = 0;
    BlockID cur = b;
    while(guard++ < MAX_BLOCKS){
        if(cur == d) return true;
        if(licm_idom[cur] == NO_BLOCK) return false;
        if(licm_idom[cur] == cur) return (cur == d); /* reached root */
        cur = licm_idom[cur];
    }
    return false;
}

/* ═════════════════════════════════════════════════════════════════
 * E.  Natural loop body via reverse BFS from latch to header
 * ═════════════════════════════════════════════════════════════════ */
static void licm_loop_body(Function *fn, BlockID header, BlockID latch, BlkSet body)
{
    static bool    visited[MAX_BLOCKS];
    static BlockID queue  [MAX_BLOCKS];
    memset(visited, 0, fn->n_blocks * sizeof(bool));

    int qh = 0, qt = 0;
    blkset_add(body, header);
    visited[header] = true;

    if(!blkset_has(body, latch)){
        blkset_add(body, latch);
        visited[latch] = true;
        queue[qt++] = latch;
    }

    while(qh < qt){
        BlockID cur = queue[qh++];
        Block  *blk = fn->blocks[cur];
        for(uint32_t pi = 0; pi < blk->n_preds; pi++){
            BlockID p = blk->preds[pi];
            if(p >= fn->n_blocks || visited[p]) continue;
            visited[p] = true;
            blkset_add(body, p);
            if(qt < MAX_BLOCKS) queue[qt++] = p;
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
                                      const BlkSet loop_body, uint32_t *synth_id)
{
    if(fn->n_blocks >= MAX_BLOCKS) return (BlockID)NO_BLOCK;

    BlockID ph_id = fn->n_blocks++;
    fn->blocks[ph_id] = calloc(1, sizeof(Block));
    Block *ph = fn->blocks[ph_id];
    ph->id = ph_id;
    snprintf(ph->name, NAME_LEN, "preheader.%u", (unsigned)header);
    ph->reachable     = true;
    ph->exec_freq     = 0.0;
    ph->succs[0]      = header;
    ph->n_succs       = 1;
    ph->branch_prob[0]= 1.0;
    licm_idom[ph_id]  = NO_BLOCK;   /* dominator not in the tree yet */

    /* Preheader ends with an unconditional branch to the loop header */
    Instr *br        = calloc(1, sizeof(Instr));
    br->id           = (*synth_id)++;
    br->op           = OP_BR;
    br->src[0]       = header;
    br->n_src        = 1;
    br->exec_freq    = 1.0;
    ph->head = ph->tail = br;
    ph->n_instrs = 1;

    /* Collect non-loop predecessors of the header */
    Block *hdr = fn->blocks[header];
    BlockID nlp[MAX_PREDS];
    uint32_t n_nlp = 0;
    for(uint32_t pi = 0; pi < hdr->n_preds; pi++){
        BlockID p = hdr->preds[pi];
        if(!blkset_has(loop_body, p)) nlp[n_nlp++] = p;
    }

    /* Redirect non-loop predecessors to point at preheader */
    for(uint32_t i = 0; i < n_nlp; i++){
        BlockID p    = nlp[i];
        Block  *pblk = fn->blocks[p];

        /* Update successor list */
        for(uint32_t si = 0; si < pblk->n_succs; si++)
            if(pblk->succs[si] == header){ pblk->succs[si] = ph_id; break; }

        /* Patch branch-instruction target operands */
        for(Instr *ins = pblk->head; ins; ins = ins->next)
            if(ins->op == OP_BR || ins->op == OP_CONDBR)
                for(uint32_t s = 0; s < ins->n_src; s++)
                    if(ins->src[s] == header) ins->src[s] = ph_id;

        /* Register the predecessor of the preheader */
        if(ph->n_preds < MAX_PREDS) ph->preds[ph->n_preds++] = p;
    }

    /* Rebuild header predecessor list: keep only loop preds + preheader */
    uint32_t new_np = 0;
    for(uint32_t pi = 0; pi < hdr->n_preds; pi++){
        BlockID p = hdr->preds[pi];
        if(blkset_has(loop_body, p)) hdr->preds[new_np++] = p;
    }
    hdr->preds[new_np++] = ph_id;
    hdr->n_preds = new_np;

    /* Fix PHI nodes in header:
     * For each φ, collect the non-loop-pred sources into a single entry
     * via the preheader.  The first non-loop source value is used. */
    for(Instr *ins = hdr->head; ins; ins = ins->next){
        if(ins->op != OP_PHI) continue;
        RegID   ph_val = 0;
        bool    found  = false;
        uint32_t new_n = 0;
        for(uint32_t p = 0; p < ins->n_phi; p++){
            if(!blkset_has(loop_body, ins->phi[p].block)){
                if(!found){ ph_val = ins->phi[p].reg; found = true; }
                /* drop extra non-loop sources */
            } else {
                ins->phi[new_n++] = ins->phi[p];
            }
        }
        if(found){
            ins->phi[new_n].reg   = ph_val;
            ins->phi[new_n].block = ph_id;
            new_n++;
        }
        ins->n_phi = new_n;
    }

    return ph_id;
}

/* ═════════════════════════════════════════════════════════════════
 * G.  Alias map  — pointer registers stored-to inside the loop
 * ═════════════════════════════════════════════════════════════════ */
static void licm_build_alias(Function *fn, const BlkSet loop_body, uint64_t alias[8])
{
    memset(alias, 0, 8 * sizeof(uint64_t));
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        if(!blkset_has(loop_body, bi)) continue;
        for(Instr *ins = fn->blocks[bi]->head; ins; ins = ins->next){
            if(ins->op == OP_STORE && ins->n_src >= 2){
                RegID ptr = ins->src[1];
                if(ptr && ptr < MAX_INSTRS) bs_set(alias, ptr);
            }
        }
    }
}

/* ═════════════════════════════════════════════════════════════════
 * H.  Invariance predicate
 * ═════════════════════════════════════════════════════════════════ */
static bool licm_is_invariant(Function *fn, const Instr *ins,
                               const BlkSet loop_body,
                               const uint64_t alias[8])
{
    /* Never hoist side-effecting or control-flow instructions */
    switch(ins->op){
        case OP_STORE: case OP_CALL:  case OP_RET:
        case OP_BR:    case OP_CONDBR:case OP_PHI:
        case OP_NOP:   case OP_ALLOCA:case OP_UNDEF:
            return false;
        default: break;
    }

    /* LOADs are invariant only if the pointer is not aliased by a loop store */
    if(ins->op == OP_LOAD && ins->n_src >= 1){
        RegID ptr = ins->src[0];
        if(ptr && ptr < MAX_INSTRS && bs_get((uint64_t*)alias, ptr))
            return false;
    }

    /* All source operands must be defined outside the loop (or be fn args) */
    for(uint32_t s = 0; s < ins->n_src; s++){
        RegID r = ins->src[s];
        if(!r || r >= MAX_INSTRS) continue;
        BlockID db = fn->def_block[r];
        if(db == NO_BLOCK || db >= fn->n_blocks) continue; /* fn arg / constant */
        if(blkset_has(loop_body, db)) return false;         /* defined inside loop */
    }
    return true;
}

/* ═════════════════════════════════════════════════════════════════
 * LICM pass entry point
 * ═════════════════════════════════════════════════════════════════ */
uint32_t licm_pass(Function *fn)
{
    fn->stats.licm_hoisted             = 0;
    fn->stats.licm_preheaders_inserted = 0;
    if(fn->n_blocks == 0) return 0;

    /* A — def→block map */
    licm_build_def_block(fn);

    /* B + C — RPO and dominator tree */
    licm_compute_rpo(fn);
    licm_compute_doms(fn);

    /* D — Discover natural loops via back edges */
    static LICMLoop loops[MAX_LOOPS];
    uint32_t n_loops = 0;
    uint32_t synth_id = fn->n_regs + 10000;   /* synthetic IDs for preheader branches */

    for(uint32_t bi = 0; bi < fn->n_blocks && n_loops < MAX_LOOPS; bi++){
        Block *blk = fn->blocks[bi];
        if(!blk || !blk->reachable) continue;

        for(uint32_t si = 0; si < blk->n_succs; si++){
            BlockID h = blk->succs[si];
            if(h >= fn->n_blocks || licm_idom[h] == NO_BLOCK) continue;
            if(!licm_dominates(h, bi)) continue;   /* not a back edge */

            /* Back edge bi → h; h is the loop header */
            LICMLoop *lp = NULL;
            for(uint32_t li = 0; li < n_loops; li++)
                if(loops[li].header == h){ lp = &loops[li]; break; }

            if(!lp){
                if(n_loops >= MAX_LOOPS) break;
                lp = &loops[n_loops++];
                memset(lp, 0, sizeof(LICMLoop));
                lp->header     = h;
                lp->preheader  = (BlockID)NO_BLOCK;
                lp->has_preheader = false;
            }
            if(lp->n_latches < MAX_LOOP_BLOCKS) lp->latches[lp->n_latches++] = bi;

            /* E — compute (and merge) loop body */
            BlkSet extra; blkset_zero(extra);
            licm_loop_body(fn, h, bi, extra);
            blkset_merge(lp->body_bs, extra);
        }
    }

    if(n_loops == 0) return 0;

    fprintf(stderr, "[LICM]      %u natural loop(s) found\n", n_loops);

    /* F — Insert preheaders */
    for(uint32_t li = 0; li < n_loops; li++){
        LICMLoop *lp  = &loops[li];
        Block    *hdr = fn->blocks[lp->header];

        uint32_t n_outside = 0;
        for(uint32_t pi = 0; pi < hdr->n_preds; pi++)
            if(!blkset_has(lp->body_bs, hdr->preds[pi])) n_outside++;

        if(n_outside == 0) continue;  /* degenerate: all preds are back-edges */

        BlockID ph = licm_insert_preheader(fn, lp->header, lp->body_bs, &synth_id);
        if(ph != (BlockID)NO_BLOCK){
            lp->preheader     = ph;
            lp->has_preheader = true;
            fn->stats.licm_preheaders_inserted++;
            fprintf(stderr, "[LICM]      preheader '%s' inserted for loop header '%s'\n",
                    fn->blocks[ph]->name, fn->blocks[lp->header]->name);
        }
    }

    /* Rebuild def→block map after preheader insertion may have added blocks */
    licm_build_def_block(fn);
    /* Rebuild reachability so later passes see the new blocks */
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++)
        if(fn->blocks[bi]) fn->blocks[bi]->reachable = false;
    /* quick BFS re-mark */
    {
        bool vis[MAX_BLOCKS] = {false};
        BlockID q[MAX_BLOCKS]; int qh=0,qt=0;
        q[qt++] = fn->entry; vis[fn->entry] = true;
        while(qh < qt){
            BlockID cur = q[qh++];
            fn->blocks[cur]->reachable = true;
            for(uint32_t si=0;si<fn->blocks[cur]->n_succs;si++){
                BlockID s=fn->blocks[cur]->succs[si];
                if(s<fn->n_blocks && !vis[s]){ vis[s]=true; q[qt++]=s; }
            }
        }
    }

    /* G + H — Hoist for each loop, innermost-last discovery order */
    uint32_t total = 0;
    for(uint32_t li = 0; li < n_loops; li++){
        LICMLoop *lp = &loops[li];
        if(!lp->has_preheader) continue;

        Block *ph_blk  = fn->blocks[lp->preheader];
        Instr *ph_term = ph_blk->tail;   /* the OP_BR to header */

        uint64_t alias[8];
        licm_build_alias(fn, lp->body_bs, alias);

        /* Iterate until stable: a hoisted instruction may enable another */
        bool progress = true;
        while(progress){
            progress = false;

            for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
                if(!blkset_has(lp->body_bs, bi)) continue;
                Block *blk = fn->blocks[bi];
                if(!blk || !blk->reachable) continue;

                Instr *ins = blk->head;
                while(ins){
                    Instr *nxt = ins->next;
                    if(licm_is_invariant(fn, ins, lp->body_bs, alias)){
                        /* Unlink from loop block */
                        if(ins->prev) ins->prev->next = ins->next;
                        else          blk->head       = ins->next;
                        if(ins->next) ins->next->prev = ins->prev;
                        else          blk->tail       = ins->prev;
                        blk->n_instrs--;
                        ins->prev = ins->next = NULL;

                        /* Insert before preheader terminator */
                        ins->next  = ph_term;
                        ins->prev  = ph_term->prev;
                        if(ph_term->prev) ph_term->prev->next = ins;
                        else              ph_blk->head        = ins;
                        ph_term->prev = ins;
                        ph_blk->n_instrs++;

                        /* Update def→block map */
                        if(ins->dst && ins->dst < MAX_INSTRS)
                            fn->def_block[ins->dst] = lp->preheader;

                        total++;
                        progress = true;
                        fprintf(stderr,
                            "[LICM]      hoisted %%%u (%s) from '%s' → '%s'\n",
                            (unsigned)ins->dst,
                            opcode_name[ins->op],
                            blk->name,
                            ph_blk->name);
                    }
                    ins = nxt;
                }
            }

            /* Rebuild alias after each round (hoisted stores change alias set) */
            if(progress) licm_build_alias(fn, lp->body_bs, alias);
        }
    }

    fn->stats.licm_hoisted = total;
    return total;
}

/**
 * ssa_dce_pass() — SSA-form Dead Code Elimination
 *
 * @param fn   The function IR to transform (modified in-place).
 * @return     Number of instructions removed.
 */
uint32_t ssa_dce_pass(Function *fn)
{
    /* live[r] == true → register r has at least one reachable use */
    bool live[MAX_INSTRS] = {false};

    RegWorklist wl = {0};

    /* ── Step 1: Seed worklist with operands of critical instructions ── */
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        Block *blk = fn->blocks[bi];
        if(!blk->reachable) continue;

        for(Instr *ins = blk->head; ins; ins = ins->next){
            if(!is_critical(ins)) continue;

            /* Seed all source operands of this critical instruction */
            for(uint32_t s = 0; s < ins->n_src; s++){
                RegID r = ins->src[s];
                if(r && !live[r]){
                    live[r] = true;
                    wl_push(&wl, r);
                }
            }
            /* For φ-nodes that are critical (unusual but handle it) */
            if(ins->op == OP_PHI){
                for(uint32_t p = 0; p < ins->n_phi; p++){
                    RegID r = ins->phi[p].reg;
                    if(r && !live[r]){
                        live[r] = true;
                        wl_push(&wl, r);
                    }
                }
            }
        }
    }

    /* ── Step 2: Propagate liveness backward through def-use chains ── */
    while(wl.size > 0){
        RegID r = wl_pop(&wl);

        Instr *def = fn->def_of[r];
        if(!def) continue;   /* function argument / undefined → stop */

        /* Mark defining instruction's operands live */
        for(uint32_t s = 0; s < def->n_src; s++){
            RegID src = def->src[s];
            if(src && !live[src]){
                live[src] = true;
                wl_push(&wl, src);
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
        if(def->op == OP_PHI){
            for(uint32_t p = 0; p < def->n_phi; p++){
                PhiSource *ps  = &def->phi[p];
                Block     *pred = fn->blocks[ps->block];
                if(!pred || !pred->reachable) continue;  /* prune unreachable */

                if(!live[ps->reg]){
                    live[ps->reg] = true;
                    wl_push(&wl, ps->reg);
                }
            }
        }
    }

    /* ── Step 3: Mark dead instructions & remove them ── */
    uint32_t removed = 0;

    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        Block *blk = fn->blocks[bi];

        for(Instr *ins = blk->head; ins; ins = ins->next){
            if(is_critical(ins)) continue;   /* never remove critical ops */
            if(ins->op == OP_NOP)   continue;

            /* Dead iff destination register is not live */
            if(ins->dst && !live[ins->dst]){
                ins->dead = true;
                removed++;
            }
        }

        /* Compact instruction list (remove dead nodes) */
        Instr *cur = blk->head;
        while(cur){
            Instr *next = cur->next;
            if(cur->dead){
                if(cur->prev) cur->prev->next = cur->next;
                else          blk->head       = cur->next;
                if(cur->next) cur->next->prev = cur->prev;
                else          blk->tail       = cur->prev;
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
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        Block *blk = fn->blocks[bi];
        if(!blk->reachable && blk->id != fn->entry){
            /* Free instructions */
            Instr *cur = blk->head;
            while(cur){ Instr *n = cur->next; free(cur); cur = n; }
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
 * ─────────────────────────────────────────────────────────────────────────── */

typedef uint32_t AllocaID;

typedef struct {
    AllocaID  id;
    RegID     base_reg;     /* the register receiving the alloca result  */
    uint32_t  size_bytes;
    bool      escapes;      /* true → cannot promote to stack            */
} AllocaRecord;

typedef struct {
    AllocaRecord allocs[MAX_ALLOCS];
    uint32_t     n_allocs;

    /*
     * points_to[r] — simplified: single alloca ID each register may alias.
     * A production pass would use a bitset here; we use a scalar for clarity.
     */
    AllocaID     points_to[MAX_INSTRS];  /* indexed by RegID */
} EscapeCtx;

static AllocaID ea_alloc_of(EscapeCtx *ctx, RegID r){
    return (r < MAX_INSTRS) ? ctx->points_to[r] : NO_ALLOC;
}

/**
 * escape_analysis_pass() — Points-to + Escape analysis, then stack promotion.
 *
 * @param fn   Function IR to analyze.
 * @param ctx  Escape context (caller allocates, zeroed).
 * @return     Number of allocations promoted to the stack.
 */
uint32_t escape_analysis_pass(Function *fn, EscapeCtx *ctx)
{
    memset(ctx->points_to, 0xFF, sizeof(ctx->points_to)); /* init = NO_ALLOC */

    /* ── Step 1: Discover all OP_ALLOCA sites ── */
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        Block *blk = fn->blocks[bi];
        if(!blk->reachable) continue;

        for(Instr *ins = blk->head; ins; ins = ins->next){
            if(ins->op != OP_ALLOCA) continue;
            if(ctx->n_allocs >= MAX_ALLOCS) break;

            AllocaRecord *ar = &ctx->allocs[ctx->n_allocs++];
            ar->id        = ctx->n_allocs - 1;
            ar->base_reg  = ins->dst;
            ar->size_bytes = (ins->n_src > 0) ? ins->src[0] : 8; /* default 8B */
            ar->escapes   = false;

            ctx->points_to[ins->dst] = ar->id;
        }
    }

    /* ── Step 2: Forward propagation of points-to through GEP / COPY / PHI ── */
    bool changed = true;
    while(changed){
        changed = false;

        for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
            Block *blk = fn->blocks[bi];
            if(!blk->reachable) continue;

            for(Instr *ins = blk->head; ins; ins = ins->next){
                AllocaID aid = NO_ALLOC;

                switch(ins->op){
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
                    for(uint32_t p = 0; p < ins->n_phi; p++){
                        Block *pred = fn->blocks[ins->phi[p].block];
                        if(!pred || !pred->reachable) continue;
                        AllocaID src_aid = ea_alloc_of(ctx, ins->phi[p].reg);
                        if(aid == NO_ALLOC)        aid = src_aid;
                        else if(aid != src_aid)    aid = NO_ALLOC; /* ambiguous */
                    }
                    break;

                case OP_LOAD:
                    /* loaded value: no provenance unless we track heap contents */
                    aid = NO_ALLOC;
                    break;

                default:
                    continue; /* other ops don't propagate pointers */
                }

                if(ins->dst && aid != ctx->points_to[ins->dst]){
                    ctx->points_to[ins->dst] = aid;
                    changed = true;
                }
            }
        }
    }

    /* ── Step 3: Escape detection — mark allocations that reach escape sites ── */
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        Block *blk = fn->blocks[bi];
        if(!blk->reachable) continue;

        for(Instr *ins = blk->head; ins; ins = ins->next){

            /* E1: alloca-derived pointer returned */
            if(ins->op == OP_RET){
                for(uint32_t s = 0; s < ins->n_src; s++){
                    AllocaID aid = ea_alloc_of(ctx, ins->src[s]);
                    if(aid != NO_ALLOC) ctx->allocs[aid].escapes = true;
                }
            }

            /* E2: alloca address stored into non-local memory
             *   STORE <value> <pointer>
             *   If <value> is alloca-derived AND <pointer> is not a local alloca
             *   → the pointer escapes into the heap.
             */
            if(ins->op == OP_STORE && ins->n_src >= 2){
                RegID    value_reg = ins->src[0];
                RegID    ptr_reg   = ins->src[1];
                AllocaID val_aid   = ea_alloc_of(ctx, value_reg);
                AllocaID ptr_aid   = ea_alloc_of(ctx, ptr_reg);

                if(val_aid != NO_ALLOC && ptr_aid == NO_ALLOC){
                    /* value is a local alloca, pointer is external → escape */
                    ctx->allocs[val_aid].escapes = true;
                }
            }

            /* E3: alloca passed as argument to a call */
            if(ins->op == OP_CALL){
                for(uint32_t s = 0; s < ins->n_src; s++){
                    AllocaID aid = ea_alloc_of(ctx, ins->src[s]);
                    if(aid != NO_ALLOC) ctx->allocs[aid].escapes = true;
                }
            }
        }
    }

    /* ── Step 4: Promote non-escaping allocations ── */
    uint32_t promoted = 0;

    for(uint32_t ai = 0; ai < ctx->n_allocs; ai++){
        AllocaRecord *ar = &ctx->allocs[ai];
        if(ar->escapes) continue;

        /*
         * Promotion: tag the OP_ALLOCA instruction as a stack allocation.
         * In a full backend this would lower to SUB RSP / LEA RBP-offset.
         * Here we mark the instruction and record it for code generation.
         */
        RegID base = ar->base_reg;
        if(base < MAX_INSTRS){
            Instr *def = fn->def_of[base];
            if(def && def->op == OP_ALLOCA){
                /* Mark promoted: the escape field on the *instruction* */
                def->escape = false;  /* escape=false → promoted to stack */
                promoted++;
            }
        }
    }

    fn->stats.ea_promotions += promoted;
    return promoted;
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
 * ─────────────────────────────────────────────────────────────────────────── */

/* Comparator: sort blocks descending by execution frequency */
static Block *g_blocks_for_sort[MAX_BLOCKS];
static int cmp_freq_desc(const void *a, const void *b){
    uint32_t ia = *(const uint32_t*)a;
    uint32_t ib = *(const uint32_t*)b;
    double fa = g_blocks_for_sort[ia] ? g_blocks_for_sort[ia]->exec_freq : 0.0;
    double fb = g_blocks_for_sort[ib] ? g_blocks_for_sort[ib]->exec_freq : 0.0;
    if(fa > fb) return -1;
    if(fa < fb) return  1;
    return 0;
}

/**
 * propagate_exec_freq() — Forward-propagate branch probabilities.
 *
 * Seeds entry block at freq=1.0 and propagates freq × prob through each
 * CFG edge.  Loops are handled with a fixed iteration limit (3 passes
 * approximates the loop trip count contribution).
 */
static BlockID next_unplaced(Function *fn, uint32_t *sorted)
{
    for(uint32_t i = 0; i < fn->n_blocks; i++){
        BlockID id = sorted[i];
        if(id < fn->n_blocks && !fn->blocks[id]->placed
           && fn->blocks[id]->reachable)
            return id;
    }
    return 0xFFFFFFFFu;
}

static void propagate_exec_freq(Function *fn)
{
    /* Initialize all blocks to 0 except entry */
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        fn->blocks[bi]->exec_freq = 0.0;
    }
    if(fn->n_blocks == 0) return;
    fn->blocks[fn->entry]->exec_freq = 1.0;

    /* Three passes handle typical single-level loops */
    for(int pass = 0; pass < 3; pass++){
        for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
            Block *blk = fn->blocks[bi];
            if(!blk->reachable || blk->exec_freq == 0.0) continue;

            for(uint32_t si = 0; si < blk->n_succs; si++){
                BlockID sid = blk->succs[si];
                if(sid >= fn->n_blocks) continue;
                Block *succ = fn->blocks[sid];

                double prob = (blk->n_succs == 1)
                    ? 1.0
                    : blk->branch_prob[si];

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
uint32_t pgo_reorder_pass(Function *fn, BlockID *order_out)
{
    /* Step 1: propagate frequencies from branch probabilities */
    propagate_exec_freq(fn);

    /* Step 2: reset placement state */
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        fn->blocks[bi]->placed     = false;
        fn->blocks[bi]->chain_next = 0xFFFFFFFF;
        g_blocks_for_sort[bi]      = fn->blocks[bi];
    }

    /* Step 3: build sorted order by frequency (for chain start selection) */
    uint32_t sorted[MAX_BLOCKS];
    for(uint32_t i = 0; i < fn->n_blocks; i++) sorted[i] = i;
    qsort(sorted, fn->n_blocks, sizeof(uint32_t), cmp_freq_desc);

    uint32_t n_placed = 0;

    /* Step 4: entry block anchors the first chain */
    BlockID chain_start = fn->entry;

    while(chain_start != 0xFFFFFFFF){
        BlockID cur = chain_start;
        fn->blocks[cur]->placed = true;
        order_out[n_placed++]   = cur;

        /* Greedily extend the chain */
        while(true){
            Block   *blk     = fn->blocks[cur];
            double   best_f  = -1.0;
            BlockID  best_id = 0xFFFFFFFF;

            for(uint32_t si = 0; si < blk->n_succs; si++){
                BlockID sid = blk->succs[si];
                if(sid >= fn->n_blocks)        continue;
                if(fn->blocks[sid]->placed)    continue;
                if(!fn->blocks[sid]->reachable) continue;

                double f = fn->blocks[sid]->exec_freq;
                if(f > best_f){
                    best_f  = f;
                    best_id = sid;
                }
            }

            if(best_id == 0xFFFFFFFF) break;   /* no more successors to extend */

            blk->chain_next              = best_id;
            fn->blocks[best_id]->placed  = true;
            order_out[n_placed++]        = best_id;
            cur                          = best_id;
        }

        /* Start next chain from the highest-frequency un-placed block */
        chain_start = next_unplaced(fn, sorted);
    }

    fn->stats.pgo_blocks_reordered += n_placed;
    return n_placed;
}

/* ─────────────────────────────────────────────────────────────────────────────
 * REACHABILITY ANALYSIS  (prerequisite for all three passes)
 * ─────────────────────────────────────────────────────────────────────────── */

static void compute_reachability(Function *fn)
{
    /* BFS from entry */
    bool visited[MAX_BLOCKS] = {false};
    BlockID queue[MAX_BLOCKS];
    int head = 0, tail = 0;

    for(uint32_t i = 0; i < fn->n_blocks; i++)
        fn->blocks[i]->reachable = false;

    queue[tail++] = fn->entry;
    visited[fn->entry] = true;

    while(head < tail){
        BlockID bid = queue[head++];
        Block  *blk = fn->blocks[bid];
        blk->reachable = true;

        for(uint32_t si = 0; si < blk->n_succs; si++){
            BlockID sid = blk->succs[si];
            if(sid < fn->n_blocks && !visited[sid]){
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
 * AST; run existing passes; no IR→asm. Validates CFG, DCE, LICM on real C-like flow.
 * ─────────────────────────────────────────────────────────────────────────── */

#define AST_MAX_VARS  16
#define AST_MAX_STMTS 64

typedef struct ASTNode ASTNode;
struct ASTNode {
    int kind;           /* AST_NUM, AST_VAR, AST_ADD, AST_ASSIGN, AST_LT, AST_IF, AST_WHILE, AST_RETURN, AST_BLOCK */
    int64_t num_val;
    char    var_name[NAME_LEN];
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
    AST_NUM = 1, AST_VAR, AST_ADD, AST_SUB, AST_LT, AST_LE,
    AST_ASSIGN, AST_IF, AST_WHILE, AST_RETURN, AST_BLOCK
};

typedef struct {
    Function *fn;
    BlockID   cur_block;
    RegID     next_reg;
    InstrID   next_instr_id;
    /* var name → slot index; slot_alloca_reg[slot] = reg holding alloca result */
    char      var_names[AST_MAX_VARS][NAME_LEN];
    RegID     slot_alloca_reg[AST_MAX_VARS];
    uint32_t  n_vars;
} LowerCtx;

static RegID lower_expr(LowerCtx *ctx, ASTNode *ast);
static void  lower_stmt(LowerCtx *ctx, ASTNode *ast);

static BlockID new_block(LowerCtx *ctx, const char *name) {
    Function *fn = ctx->fn;
    if (fn->n_blocks >= MAX_BLOCKS) return MAX_BLOCKS;
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
    if (!blk->head) blk->head = blk->tail = ins;
    else { blk->tail->next = ins; ins->prev = blk->tail; blk->tail = ins; }
    blk->n_instrs++;
    if (ins->dst && ins->dst < MAX_INSTRS)
        ctx->fn->def_of[ins->dst] = ins;
    if (ctx->fn->n_regs <= ins->dst) ctx->fn->n_regs = ins->dst + 1;
}

static Instr *make_instr_imm(InstrID id, Opcode op, RegID dst, int64_t imm_val) {
    Instr *ins = calloc(1, sizeof(Instr));
    ins->id = id;
    ins->op = op;
    ins->dst = dst;
    ins->imm = imm_val;
    ins->exec_freq = 1.0;
    return ins;
}

static RegID get_or_create_var(LowerCtx *ctx, const char *name) {
    for (uint32_t i = 0; i < ctx->n_vars; i++)
        if (strcmp(ctx->var_names[i], name) == 0)
            return ctx->slot_alloca_reg[i];
    if (ctx->n_vars >= AST_MAX_VARS) return 0;
    uint32_t slot = ctx->n_vars++;
    strncpy(ctx->var_names[slot], name, NAME_LEN - 1);
    RegID r = ctx->next_reg++;
    ctx->slot_alloca_reg[slot] = r;
    Instr *alloca = make_instr_imm(ctx->next_instr_id++, OP_ALLOCA, r, 8);
    emit_instr(ctx, alloca);
    return r;
}

static RegID lower_expr(LowerCtx *ctx, ASTNode *ast) {
    if (!ast) return 0;
    RegID r = ctx->next_reg++;
    Instr *ins = NULL;
    switch (ast->kind) {
        case AST_NUM: {
            ins = make_instr_imm(ctx->next_instr_id++, OP_CONST, r, ast->num_val);
            break;
        }
        case AST_VAR: {
            RegID alloca_r = get_or_create_var(ctx, ast->var_name);
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
            ins->src[0] = l; ins->src[1] = rh;
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
            ins->src[0] = l; ins->src[1] = rh;
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
            ins->src[0] = l; ins->src[1] = rh;
            ins->n_src = 2;
            ins->exec_freq = 1.0;
            break;
        }
        default:
            return 0;
    }
    if (ins) { emit_instr(ctx, ins); return r; }
    return 0;
}

static void lower_stmt(LowerCtx *ctx, ASTNode *ast) {
    if (!ast) return;
    Function *fn = ctx->fn;
    switch (ast->kind) {
        case AST_ASSIGN: {
            RegID alloca_r = get_or_create_var(ctx, ast->lhs->var_name);
            RegID val_r = lower_expr(ctx, ast->rhs);
            Instr *st = calloc(1, sizeof(Instr));
            st->id = ctx->next_instr_id++;
            st->op = OP_STORE;
            st->dst = 0;
            st->src[0] = val_r; st->src[1] = alloca_r;
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
            cbr->src[0] = cond_r; cbr->src[1] = then_blk; cbr->src[2] = else_blk;
            cbr->n_src = 3;
            cbr->exec_freq = 1.0;
            emit_instr(ctx, cbr);
            cur->succs[0] = then_blk; cur->succs[1] = else_blk;
            cur->n_succs = 2;
            cur->branch_prob[0] = 0.5;
            cur->branch_prob[1] = 0.5;
            fn->blocks[then_blk]->preds[0] = ctx->cur_block; fn->blocks[then_blk]->n_preds = 1;
            fn->blocks[else_blk]->preds[0] = ctx->cur_block; fn->blocks[else_blk]->n_preds = 1;

            ctx->cur_block = then_blk;
            lower_stmt(ctx, ast->then_body);
            Instr *br_then = calloc(1, sizeof(Instr));
            br_then->id = ctx->next_instr_id++; br_then->op = OP_BR; br_then->src[0] = merge_blk; br_then->n_src = 1; br_then->exec_freq = 1.0;
            emit_instr(ctx, br_then);
            fn->blocks[then_blk]->succs[0] = merge_blk; fn->blocks[then_blk]->n_succs = 1;
            fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds] = then_blk;
            fn->blocks[merge_blk]->n_preds++;

            ctx->cur_block = else_blk;
            if (ast->else_body) lower_stmt(ctx, ast->else_body);
            Instr *br_else = calloc(1, sizeof(Instr));
            br_else->id = ctx->next_instr_id++; br_else->op = OP_BR; br_else->src[0] = merge_blk; br_else->n_src = 1; br_else->exec_freq = 1.0;
            emit_instr(ctx, br_else);
            fn->blocks[else_blk]->succs[0] = merge_blk; fn->blocks[else_blk]->n_succs = 1;
            fn->blocks[merge_blk]->preds[fn->blocks[merge_blk]->n_preds] = else_blk;
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
            br->id = ctx->next_instr_id++; br->op = OP_BR; br->src[0] = head; br->n_src = 1; br->exec_freq = 1.0;
            emit_instr(ctx, br);
            cur->succs[0] = head; cur->n_succs = 1;
            fn->blocks[head]->preds[0] = ctx->cur_block; fn->blocks[head]->n_preds = 1;

            ctx->cur_block = head;
            RegID cond_r = lower_expr(ctx, ast->cond);
            Instr *cbr = calloc(1, sizeof(Instr));
            cbr->id = ctx->next_instr_id++;
            cbr->op = OP_CONDBR;
            cbr->dst = 0;
            cbr->src[0] = cond_r; cbr->src[1] = body_blk; cbr->src[2] = exit_blk;
            cbr->n_src = 3;
            cbr->exec_freq = 1.0;
            emit_instr(ctx, cbr);
            fn->blocks[head]->succs[0] = body_blk; fn->blocks[head]->succs[1] = exit_blk;
            fn->blocks[head]->n_succs = 2;
            fn->blocks[head]->branch_prob[0] = 0.9;
            fn->blocks[head]->branch_prob[1] = 0.1;
            fn->blocks[body_blk]->preds[0] = head; fn->blocks[body_blk]->n_preds = 1;
            fn->blocks[exit_blk]->preds[0] = head; fn->blocks[exit_blk]->n_preds = 1;

            ctx->cur_block = body_blk;
            lower_stmt(ctx, ast->body);
            Instr *br_back = calloc(1, sizeof(Instr));
            br_back->id = ctx->next_instr_id++; br_back->op = OP_BR; br_back->src[0] = head; br_back->n_src = 1; br_back->exec_freq = 1.0;
            emit_instr(ctx, br_back);
            fn->blocks[body_blk]->succs[0] = head; fn->blocks[body_blk]->n_succs = 1;
            fn->blocks[head]->preds[fn->blocks[head]->n_preds] = body_blk;
            fn->blocks[head]->n_preds++;

            ctx->cur_block = exit_blk;
            return;
        }
        case AST_RETURN: {
            RegID val_r = lower_expr(ctx, ast->lhs);
            Instr *ret = calloc(1, sizeof(Instr));
            ret->id = ctx->next_instr_id++; ret->op = OP_RET; ret->dst = 0;
            ret->src[0] = val_r; ret->n_src = 1;
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

/**
 * ast_to_ir() — Build Function* from minimal AST (one function, scalars only).
 * Caller provides AST for function body; entry and exit blocks are created.
 */
Function *ast_to_ir(ASTNode *body_ast)
{
    Function *fn = calloc(1, sizeof(Function));
    LowerCtx ctx = {0};
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
        br->id = ctx.next_instr_id++; br->op = OP_BR; br->src[0] = exit_blk; br->n_src = 1; br->exec_freq = 1.0;
        emit_instr(&ctx, br);
        cur->succs[0] = exit_blk; cur->n_succs = 1;
        fn->blocks[exit_blk]->preds[fn->blocks[exit_blk]->n_preds++] = ctx.cur_block;
    }

    return fn;
}

/* Build AST for: int main() { int i = 0; while (i < 10) { i = i + 1; } return i; } */
static ASTNode *build_phase_b_ast(void)
{
    static ASTNode pool[32];
    static ASTNode *stmts[AST_MAX_STMTS];
    static ASTNode *body_stmts[1];
    memset(pool, 0, sizeof(pool));
    int p = 0;
#define N() (&pool[p++])

    ASTNode *zero = N(); zero->kind = AST_NUM; zero->num_val = 0;
    ASTNode *ten = N(); ten->kind = AST_NUM; ten->num_val = 10;
    ASTNode *one = N(); one->kind = AST_NUM; one->num_val = 1;
    ASTNode *var_i = N(); var_i->kind = AST_VAR; strcpy(var_i->var_name, "i");
    ASTNode *i_lt_10 = N(); i_lt_10->kind = AST_LT; i_lt_10->lhs = var_i; i_lt_10->rhs = ten;
    ASTNode *i_plus_1 = N(); i_plus_1->kind = AST_ADD; i_plus_1->lhs = var_i; i_plus_1->rhs = one;
    ASTNode *lhs_i = N(); lhs_i->kind = AST_VAR; strcpy(lhs_i->var_name, "i");
    ASTNode *assign_inc = N(); assign_inc->kind = AST_ASSIGN; assign_inc->lhs = lhs_i; assign_inc->rhs = i_plus_1;
    ASTNode *body_while = N(); body_while->kind = AST_BLOCK; body_stmts[0] = assign_inc; body_while->stmts = body_stmts; body_while->n_stmts = 1;
    ASTNode *while_loop = N(); while_loop->kind = AST_WHILE; while_loop->cond = i_lt_10; while_loop->body = body_while;
    ASTNode *init_i = N(); init_i->kind = AST_ASSIGN; init_i->lhs = N(); init_i->lhs->kind = AST_VAR; strcpy(init_i->lhs->var_name, "i"); init_i->rhs = zero;
    ASTNode *ret_i = N(); ret_i->kind = AST_RETURN; ret_i->lhs = var_i;
    stmts[0] = init_i; stmts[1] = while_loop; stmts[2] = ret_i;
    ASTNode *block = N(); block->kind = AST_BLOCK; block->stmts = stmts; block->n_stmts = 3;
    return block;
#undef N
}

/* ─────────────────────────────────────────────────────────────────────────────
 * PASS PIPELINE: run all three passes in order
 * ──────────────────────────────────────────────────────────────── */

typedef struct {
    BlockID  order[MAX_BLOCKS];
    uint32_t n_blocks;
} PassResult;

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
 * @param result  Output reordering from pass 3.
 */

/**
 * ir_validate() — Phase B entry contract. Ensures Function* satisfies
 * invariants required by DCE, LICM, escape analysis, and PGO.
 * Returns true if valid; on failure, prints diagnostics to stderr and returns false.
 */
static bool ir_validate(const Function *fn)
{
    bool ok = true;
    if (!fn || fn->n_blocks == 0) return false;

    /* Invariant 4: Entry block has no predecessors. */
    BlockID entry = fn->entry;
    if (entry >= fn->n_blocks) {
        fprintf(stderr, "[ir_validate] entry block id %u >= n_blocks %u\n", (unsigned)entry, fn->n_blocks);
        ok = false;
    } else if (fn->blocks[entry]->n_preds != 0) {
        fprintf(stderr, "[ir_validate] entry block has %u predecessors (must be 0)\n", fn->blocks[entry]->n_preds);
        ok = false;
    }

    for (uint32_t bi = 0; bi < fn->n_blocks; bi++) {
        const Block *blk = fn->blocks[bi];
        if (!blk) continue;

        /* Invariant 3: Branch probabilities on CONDBR blocks sum to 1.0. */
        if (blk->n_succs > 1) {
            double sum = 0.0;
            for (uint32_t si = 0; si < blk->n_succs; si++)
                sum += blk->branch_prob[si];
            if (sum < 0.99 || sum > 1.01) {
                fprintf(stderr, "[ir_validate] block %u (%s) branch_prob sum = %.4f (expected 1.0)\n",
                        bi, blk->name, sum);
                ok = false;
            }
        }

        for (const Instr *ins = blk->head; ins; ins = ins->next) {
            /* Invariant 1: Every used RegID is in range; def_of[r] may be NULL for args. */
            for (uint32_t s = 0; s < ins->n_src; s++) {
                RegID r = ins->src[s];
                if (r != 0 && r >= MAX_INSTRS) {
                    fprintf(stderr, "[ir_validate] block %u instr %u src[%u] reg %u >= MAX_INSTRS\n",
                            bi, (unsigned)ins->id, s, (unsigned)r);
                    ok = false;
                }
            }
            if (ins->op == OP_PHI) {
                for (uint32_t p = 0; p < ins->n_phi; p++) {
                    RegID r = ins->phi[p].reg;
                    if (r != 0 && r >= MAX_INSTRS) {
                        fprintf(stderr, "[ir_validate] block %u phi instr %u phi[%u] reg %u >= MAX_INSTRS\n",
                                bi, (unsigned)ins->id, p, (unsigned)r);
                        ok = false;
                    }
                }
            }

            /* Invariant 2: Every OP_PHI lists exactly the predecessor blocks of its block. */
            if (ins->op == OP_PHI) {
                if (ins->n_phi != blk->n_preds) {
                    fprintf(stderr, "[ir_validate] block %u phi instr %u has n_phi=%u but block has n_preds=%u\n",
                            bi, (unsigned)ins->id, ins->n_phi, blk->n_preds);
                    ok = false;
                } else {
                    for (uint32_t pi = 0; pi < blk->n_preds; pi++) {
                        BlockID pred = blk->preds[pi];
                        bool found = false;
                        for (uint32_t p = 0; p < ins->n_phi; p++)
                            if (ins->phi[p].block == pred) { found = true; break; }
                        if (!found) {
                            fprintf(stderr, "[ir_validate] block %u phi instr %u has no source for predecessor %u\n",
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

void run_all_passes(Function *fn, PassResult *result)
{
    if (!ir_validate(fn)) {
        fprintf(stderr, "[run_all_passes] IR validation failed; continuing anyway.\n");
    }

    /* Prerequisite: reachability */
    compute_reachability(fn);

    /* ── Pass 1: SSA-form DCE ── */
    uint32_t dce_removed = ssa_dce_pass(fn);
    fprintf(stderr,
        "[DCE->SSA]  instructions removed: %u  blocks removed: %u\n",
        dce_removed, fn->stats.dce_blocks_removed);

    /* Recompute reachability after DCE */
    compute_reachability(fn);

    /* ── Pass 2: LICM (Loop-Invariant Code Motion) ── */
    uint32_t licm_hoisted = licm_pass(fn);
    fprintf(stderr,
        "[LICM]      instructions hoisted: %u\n", licm_hoisted);

    /* ── Pass 3: SSA-form DCE again (cleanup after LICM) ── */
    dce_removed = ssa_dce_pass(fn);
    fprintf(stderr,
        "[DCE->SSA]  instructions removed: %u  blocks removed: %u\n",
        dce_removed, fn->stats.dce_blocks_removed);

    compute_reachability(fn);

    /* ── Pass 4: Escape Analysis + Stack Promotion ── */
    EscapeCtx ea_ctx = {0};
    uint32_t promoted = escape_analysis_pass(fn, &ea_ctx);
    fprintf(stderr,
        "[EscapeAna] allocations promoted to stack: %u  (of %u total)\n",
        promoted, ea_ctx.n_allocs);

    /* ── Pass 5: PGO Basic Block Reordering ── */
    result->n_blocks = pgo_reorder_pass(fn, result->order);
    fprintf(stderr,
        "[PGO-BBR]   blocks in emission order: %u\n", result->n_blocks);
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
 * ─────────────────────────────────────────────────────────────────────────── */

static Instr *make_instr(InstrID id, Opcode op, RegID dst,
                         RegID *srcs, uint32_t n_src)
{
    Instr *ins = calloc(1, sizeof(Instr));
    ins->id    = id;
    ins->op    = op;
    ins->dst   = dst;
    ins->n_src = n_src;
    if(srcs) memcpy(ins->src, srcs, n_src * sizeof(RegID));
    ins->exec_freq = 1.0;
    return ins;
}

static void block_append(Block *blk, Instr *ins){
    if(!blk->head){ blk->head = blk->tail = ins; }
    else          { blk->tail->next = ins; ins->prev = blk->tail; blk->tail = ins; }
    blk->n_instrs++;
}

/* Unlink instruction from its block (caller must know the block) */
static void __attribute__((unused)) instr_unlink(Block *blk, Instr *ins){
    if(ins->prev) ins->prev->next = ins->next;
    else          blk->head = ins->next;
    if(ins->next) ins->next->prev = ins->prev;
    else          blk->tail = ins->prev;
    blk->n_instrs--;
}

/* Insert ins before `before` in blk */
static void __attribute__((unused)) instr_insert_before(Block *blk, Instr *before, Instr *ins){
    ins->next = before;
    ins->prev = before->prev;
    if(before->prev) before->prev->next = ins;
    else             blk->head = ins;
    before->prev = ins;
    blk->n_instrs++;
}

static BlockID __attribute__((unused)) block_of_instr(Function *fn, const Instr *needle){
    for(uint32_t bi = 0; bi < fn->n_blocks; bi++){
        for(Instr *ins = fn->blocks[bi]->head; ins; ins = ins->next)
            if(ins == needle) return (BlockID)bi;
    }
    return MAX_BLOCKS;
}

static Function *build_test_function(void)
{
    Function *fn = calloc(1, sizeof(Function));

    /* Allocate 5 blocks */
    const char *names[] = {"entry","loop_header","loop_body","loop_exit","fn_exit"};
    for(uint32_t i = 0; i < 5; i++){
        fn->blocks[i] = calloc(1, sizeof(Block));
        fn->blocks[i]->id = i;
        strncpy(fn->blocks[i]->name, names[i], NAME_LEN-1);
        fn->blocks[i]->exec_freq = 0.0;
    }
    fn->n_blocks = 5;
    fn->entry    = 0;
    fn->exit     = 4;

    /* CFG edges */
    fn->blocks[0]->succs[0] = 1; fn->blocks[0]->n_succs = 1;
    fn->blocks[0]->branch_prob[0] = 1.0;

    fn->blocks[1]->succs[0] = 2; fn->blocks[1]->succs[1] = 3;
    fn->blocks[1]->n_succs  = 2;
    fn->blocks[1]->branch_prob[0] = 0.9;  /* mostly stay in loop */
    fn->blocks[1]->branch_prob[1] = 0.1;

    fn->blocks[2]->succs[0] = 1; fn->blocks[2]->n_succs = 1;
    fn->blocks[2]->branch_prob[0] = 1.0;

    fn->blocks[3]->succs[0] = 4; fn->blocks[3]->n_succs = 1;
    fn->blocks[3]->branch_prob[0] = 1.0;

    fn->blocks[4]->n_succs  = 0;  /* exit */

    /* Predecessors */
    fn->blocks[1]->preds[0] = 0; fn->blocks[1]->preds[1] = 2; fn->blocks[1]->n_preds = 2;
    fn->blocks[2]->preds[0] = 1; fn->blocks[2]->n_preds = 1;
    fn->blocks[3]->preds[0] = 1; fn->blocks[3]->n_preds = 1;
    fn->blocks[4]->preds[0] = 3; fn->blocks[4]->n_preds = 1;

    /* ── entry block ── */
    Block *entry = fn->blocks[0];
    /* %10 = alloca 64 — stack-promotable */
    Instr *a1   = make_instr(1, OP_ALLOCA, 10, (RegID[]){64}, 1);
    /* %20 = alloca 8  — escapes via ret  */
    Instr *a2   = make_instr(2, OP_ALLOCA, 20, (RegID[]){8},  1);
    /* %30 = add %31, %32 — dead (never used) */
    Instr *dead = make_instr(3, OP_ADD, 30, (RegID[]){31,32}, 2);
    /* br loop_header */
    Instr *br   = make_instr(4, OP_BR, 0, (RegID[]){1}, 1);
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
    Instr *phi  = make_instr(5, OP_PHI, 40, NULL, 0);
    phi->phi[0] = (PhiSource){41, 0};   /* from entry     */
    phi->phi[1] = (PhiSource){42, 2};   /* from loop_body */
    phi->n_phi  = 2;
    /* condbr %40, loop_body, loop_exit */
    Instr *cbr  = make_instr(6, OP_CONDBR, 0, (RegID[]){40,2,3}, 3);
    block_append(lhdr, phi);
    block_append(lhdr, cbr);
    fn->def_of[40] = phi;

    /* ── loop_body ── */
    Block *lbod = fn->blocks[2];
    /* store value into %10 (local buf — no escape via store to local ptr) */
    Instr *st   = make_instr(7, OP_STORE, 0, (RegID[]){99, 10}, 2);
    /* %42 = add %40, 1 */
    Instr *inc  = make_instr(8, OP_ADD,  42, (RegID[]){40, 1},  2);
    /* br loop_header */
    Instr *lbr  = make_instr(9, OP_BR, 0, (RegID[]){1}, 1);
    block_append(lbod, st);
    block_append(lbod, inc);
    block_append(lbod, lbr);
    fn->def_of[42] = inc;

    /* ── loop_exit ── */
    Block *lexit = fn->blocks[3];
    Instr *ebr   = make_instr(10, OP_BR, 0, (RegID[]){4}, 1);
    block_append(lexit, ebr);

    /* ── fn_exit ── */
    Block *fexit = fn->blocks[4];
    /* ret %20 → %20 escapes */
    Instr *ret   = make_instr(11, OP_RET, 0, (RegID[]){20}, 1);
    block_append(fexit, ret);

    fn->n_regs = 100;
    return fn;
}

static void print_pass_results(const Function *fn, const PassResult *result)
{
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
    for(uint32_t i = 0; i < result->n_blocks; i++){
        BlockID bid = result->order[i];
        printf("%s", fn->blocks[bid]->name);
        if(i + 1 < result->n_blocks) printf(" -> ");
    }
    printf("\n|  Execution frequencies:\n");
    for(uint32_t i = 0; i < result->n_blocks; i++){
        BlockID bid = result->order[i];
        printf("|    %-16s  freq = %.4f\n",
               fn->blocks[bid]->name,
               fn->blocks[bid]->exec_freq);
    }

    printf("\n=================================================\n\n");
}

static void free_function(Function *fn)
{
    for(uint32_t i = 0; i < fn->n_blocks; i++){
        Block *blk = fn->blocks[i];
        if(!blk) continue;
        Instr *cur = blk->head;
        while(cur){ Instr *n = cur->next; free(cur); cur = n; }
        free(blk);
    }
    free(fn);
}

int main(void)
{
    Function   *fn     = build_test_function();
    PassResult  result = {0};

    fprintf(stderr, "\n-- Running ZKAEDI PRIME pass pipeline (synthetic fixture) --\n");
    run_all_passes(fn, &result);
    print_pass_results(fn, &result);
    free_function(fn);

    /* Phase B: AST → IR bridge — real C-like control flow */
    {
        ASTNode *body = build_phase_b_ast();
        Function *fn_b = ast_to_ir(body);
        PassResult result_b = {0};
        fprintf(stderr, "\n-- Phase B: AST->IR (while i<10 { i=i+1 } return i) --\n");
        run_all_passes(fn_b, &result_b);
        printf("\n=================================================\n");
        printf(" Phase B - AST->IR pass results\n");
        printf("=================================================\n");
        print_pass_results(fn_b, &result_b);
        free_function(fn_b);
    }

    return 0;
}

/* ZKAEDI FORCE RENDER CACHE INVALIDATION */
