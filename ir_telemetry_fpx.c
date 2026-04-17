/*
 * ir_telemetry_fpx.c — 0x5100 packet block implementation.
 *
 * Fail-closed coercion stub: every edge defaults to UNIMPL, every
 * call is recorded. The empirical heatmap from these records decides
 * which edges get implemented first.
 *
 * Wire format: packets flow through the existing ir_telemetry_emit()
 * channel (same HMAC envelope, same port-8084 path). This file only
 * adds the FP-format event namespace; no existing telemetry behavior
 * is altered.
 */
#include <string.h>
#include <stdint.h>
#include "ir_telemetry_fpx.h"

/* ------------------------------------------------------------------ */
/* Coercion cost table — row=src, col=dst, value=gates (255=unimpl)   */
/* ------------------------------------------------------------------ */
/*
 * Index order matches IR_TY_* enum as added in ir.h. Adjust if the
 * enum layout in your tree differs.
 *
 *   0 = IR_TY_TOFP32
 *   1 = IR_TY_EMFP16_LIVE
 *   2 = IR_TY_SPSQ32_DIFF
 *   3 = IR_TY_F32      (stock)
 *   4 = IR_TY_F16      (stock)
 *   5 = IR_TY_I32      (stock)
 *   6 = IR_TY_I16      (stock)
 *   7 = (reserved — width pad)
 *
 * Legend:
 *   0   = identity (same type)
 *   1-254 = gate-cost estimate for the coercion lowering
 *   255 = not implemented — records attempt, fails closed
 *
 * The non-255 entries below are the ones we *know* are mechanically
 * cheap (e.g. TOFP→F32 is just a tile-reassemble + exponent rebuild).
 * Everything else is 255 until telemetry proves demand.
 */
#define N_FPX_TYPES 8

static uint8_t coerce_cost_table[N_FPX_TYPES][N_FPX_TYPES] = {
    /*          TOFP  EMFP  SPSQ   F32   F16   I32   I16  PAD */
    /* TOFP */ {   0,  255,  255,   40,  255,  255,  255, 255 },
    /* EMFP */ { 255,    0,  255,   25,   10,  255,  255, 255 },
    /* SPSQ */ { 255,  255,    0,   60,  255,  255,  255, 255 },
    /* F32  */ {  35,   30,   80,    0,   12,  255,  255, 255 },
    /* F16  */ { 255,   15,  255,   14,    0,  255,  255, 255 },
    /* I32  */ { 255,  255,  255,  255,  255,    0,  255, 255 },
    /* I16  */ { 255,  255,  255,  255,  255,  255,    0, 255 },
    /* PAD  */ { 255,  255,  255,  255,  255,  255,  255,   0 },
};

/*
 * Translate the IR_TY_* enum value to a table index. Keeping this as
 * a switch (rather than a direct subscript) insulates us from ir.h
 * re-numbering without breaking the telemetry table.
 */
static int fpx_type_to_idx(uint8_t ir_ty) {
    switch (ir_ty) {
        case IR_TY_TOFP32:       return 0;
        case IR_TY_EMFP16_LIVE:  return 1;
        case IR_TY_SPSQ32_DIFF:  return 2;
        case IR_TY_F32:          return 3;
        case IR_TY_F16:          return 4;
        case IR_TY_I32:          return 5;
        case IR_TY_I16:          return 6;
        default:                 return -1;
    }
}

/* ------------------------------------------------------------------ */
/* Packet counter — monotonic, wraps at 2^32                           */
/* ------------------------------------------------------------------ */
static uint32_t fpx_counter = 0;

void ir_tel_fpx_init(void) {
    fpx_counter = 0;
}

/* ------------------------------------------------------------------ */
/* Internal: emit via existing ir_telemetry channel                    */
/* ------------------------------------------------------------------ */
static void fpx_emit(ir_tel_fpx_packet_t *p) {
    p->counter = ++fpx_counter;
    /*
     * ir_telemetry_emit takes the event code + payload. We hand it
     * our packet as the payload blob. The existing HMAC envelope
     * layer wraps this before it hits the wire.
     */
    ir_telemetry_emit(p->event_code, p, sizeof(*p));
}

/* Helper to populate common fields */
static void fpx_pack(ir_tel_fpx_packet_t *p, uint16_t ev,
                     uint8_t src, uint8_t dst, uint8_t op,
                     uint8_t flags, uint32_t ctx) {
    memset(p, 0, sizeof(*p));
    p->event_code   = ev;
    p->src_type     = src;
    p->dst_type     = dst;
    p->op_kind      = op;
    p->flags        = flags;
    p->pass_id      = (uint16_t) ir_pass_manager_current_pass_id();
    p->context_hash = ctx;
}

/* ------------------------------------------------------------------ */
/* Public emitters                                                     */
/* ------------------------------------------------------------------ */
void ir_tel_fpx_emit_xfmt_fma(uint8_t src, uint8_t dst, uint32_t ctx) {
    ir_tel_fpx_packet_t p;
    fpx_pack(&p, IR_TEL_FPX_XFMT_FMA, src, dst, IR_FPX_OP_FMA, 0, ctx);
    fpx_emit(&p);
}

void ir_tel_fpx_emit_coerce_missing(uint8_t src, uint8_t dst,
                                    uint8_t op, uint32_t ctx) {
    ir_tel_fpx_packet_t p;
    fpx_pack(&p, IR_TEL_FPX_COERCE_MISSING, src, dst, op,
             IR_FPX_FLAG_FAIL_CLOSED, ctx);
    fpx_emit(&p);
}

void ir_tel_fpx_emit_coerce_ok(uint8_t src, uint8_t dst,
                               uint8_t op, uint8_t flags, uint32_t ctx) {
    ir_tel_fpx_packet_t p;
    fpx_pack(&p, IR_TEL_FPX_COERCE_OK, src, dst, op, flags, ctx);
    fpx_emit(&p);
}

void ir_tel_fpx_emit_attractor_step(uint32_t tile_id, uint32_t ctx) {
    ir_tel_fpx_packet_t p;
    fpx_pack(&p, IR_TEL_FPX_ATTRACTOR_STEP,
             IR_TY_EMFP16_LIVE, IR_TY_EMFP16_LIVE,
             IR_FPX_OP_LOAD, 0, ctx);
    /* stuff tile_id into a free field — counter is overwritten anyway */
    p.counter = tile_id;  /* ok: fpx_emit overwrites with real counter */
    fpx_emit(&p);
}

void ir_tel_fpx_emit_tile_cross(int level, uint8_t op, uint32_t ctx) {
    ir_tel_fpx_packet_t p;
    uint16_t ev = (level == 0)
                  ? IR_TEL_FPX_TILE_CROSS_L0
                  : IR_TEL_FPX_TILE_CROSS_L1;
    uint8_t flags = (level == 0) ? IR_FPX_FLAG_FAST_PATH : 0;
    fpx_pack(&p, ev, IR_TY_TOFP32, IR_TY_TOFP32, op, flags, ctx);
    fpx_emit(&p);
}

void ir_tel_fpx_emit_spsq_diff_roll(uint32_t ctx) {
    ir_tel_fpx_packet_t p;
    fpx_pack(&p, IR_TEL_FPX_SPSQ_DIFF_ROLL,
             IR_TY_SPSQ32_DIFF, IR_TY_SPSQ32_DIFF,
             IR_FPX_OP_MUL, 0, ctx);
    fpx_emit(&p);
}

/* ------------------------------------------------------------------ */
/* Coercion cost query — the function IR_CAST lowering calls           */
/* ------------------------------------------------------------------ */
uint8_t ir_tel_fpx_coerce_query(uint8_t src, uint8_t dst) {
    int si = fpx_type_to_idx(src);
    int di = fpx_type_to_idx(dst);
    if (si < 0 || di < 0) return IR_FPX_COERCE_UNIMPL;
    return coerce_cost_table[si][di];
}

uint8_t ir_tel_fpx_coerce_cast_attempt(uint8_t src, uint8_t dst, uint32_t ctx) {
    uint8_t cost = ir_tel_fpx_coerce_query(src, dst);

    /*
     * Record the query itself — this is the heatmap fuel.
     * Even a hit gets recorded so we see which edges are hot.
     */
    if (cost == IR_FPX_COERCE_UNIMPL) {
        ir_tel_fpx_emit_coerce_missing(src, dst, IR_FPX_OP_CAST, ctx);
    } else if (cost > 0) {
        ir_tel_fpx_emit_coerce_ok(src, dst, IR_FPX_OP_CAST, 0, ctx);
    }
    /* identity coercions (cost == 0) are not recorded — too noisy */
    return cost;
}
