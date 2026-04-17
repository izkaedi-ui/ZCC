/*
 * ir_telemetry_fpx.h — 0x5100 packet block: novel FP format events
 *
 * Layered on top of ir_telemetry.h. Additive; does not touch existing
 * telemetry signatures. Safe to include from any IR pass.
 *
 * Event namespace:
 *   0x5100  FPX_BASE            (reserved sentinel)
 *   0x5101  FPX_XFMT_FMA        cross-format FMA attempt
 *   0x5102  FPX_COERCE_MISSING  coercion edge not yet implemented
 *   0x5103  FPX_COERCE_OK       coercion succeeded (edge wired up)
 *   0x5104  FPX_WIDTH_MISMATCH  operand width delta (16↔32)
 *   0x5105  FPX_PRECISION_DROP  lossy downgrade flagged by static check
 *   0x5106  FPX_ATTRACTOR_STEP  EM-FP16-LIVE H-evolution tick
 *   0x5107  FPX_TILE_CROSS_L0   TOFP L0 boundary crossed (HIER-2L fallback)
 *   0x5108  FPX_TILE_CROSS_L1   TOFP L1 boundary crossed (FP32 fallback)
 *   0x5109  FPX_SPSQ_DIFF_ROLL  SPSQ diffusion residue carry rollover
 */
#ifndef IR_TELEMETRY_FPX_H
#define IR_TELEMETRY_FPX_H

#include <stdint.h>
#include "ir_telemetry.h"
#include "ir.h"

#define IR_TEL_FPX_BASE              0x5100
#define IR_TEL_FPX_XFMT_FMA          0x5101
#define IR_TEL_FPX_COERCE_MISSING    0x5102
#define IR_TEL_FPX_COERCE_OK         0x5103
#define IR_TEL_FPX_WIDTH_MISMATCH    0x5104
#define IR_TEL_FPX_PRECISION_DROP    0x5105
#define IR_TEL_FPX_ATTRACTOR_STEP    0x5106
#define IR_TEL_FPX_TILE_CROSS_L0     0x5107
#define IR_TEL_FPX_TILE_CROSS_L1     0x5108
#define IR_TEL_FPX_SPSQ_DIFF_ROLL    0x5109

/* Operation kind — matches zcc IR op categories */
#define IR_FPX_OP_FMA    0x01
#define IR_FPX_OP_ADD    0x02
#define IR_FPX_OP_MUL    0x03
#define IR_FPX_OP_CAST   0x04
#define IR_FPX_OP_LOAD   0x05
#define IR_FPX_OP_STORE  0x06

/* Flags */
#define IR_FPX_FLAG_LOSSY       0x01
#define IR_FPX_FLAG_SATURATED   0x02
#define IR_FPX_FLAG_FAIL_CLOSED 0x04
#define IR_FPX_FLAG_FAST_PATH   0x08

/*
 * Packet layout — 16 bytes wire-aligned. Matches the standard
 * ir_telemetry envelope used elsewhere in ZCC.
 */
typedef struct {
    uint16_t event_code;    /* 0x5101..0x5109 */
    uint8_t  src_type;      /* IR_TY_* enum value */
    uint8_t  dst_type;      /* IR_TY_* enum value, 0xFF if N/A */
    uint8_t  op_kind;       /* IR_FPX_OP_* */
    uint8_t  flags;         /* IR_FPX_FLAG_* bitfield */
    uint16_t pass_id;       /* IR pass identifier (from ir_pass_manager) */
    uint32_t context_hash;  /* rolling hash of source location */
    uint32_t counter;       /* monotonic sequence # for replay */
} ir_tel_fpx_packet_t;

_Static_assert(sizeof(ir_tel_fpx_packet_t) == 16,
               "fpx packet must be 16 bytes for wire alignment");

/*
 * High-level emitter API — call sites should use these, not the raw
 * packet struct, so we can evolve the wire format transparently.
 */
void ir_tel_fpx_init(void);

void ir_tel_fpx_emit_xfmt_fma(uint8_t src, uint8_t dst, uint32_t ctx);
void ir_tel_fpx_emit_coerce_missing(uint8_t src, uint8_t dst,
                                    uint8_t op, uint32_t ctx);
void ir_tel_fpx_emit_coerce_ok(uint8_t src, uint8_t dst,
                               uint8_t op, uint8_t flags, uint32_t ctx);
void ir_tel_fpx_emit_attractor_step(uint32_t tile_id, uint32_t ctx);
void ir_tel_fpx_emit_tile_cross(int level, uint8_t op, uint32_t ctx);
void ir_tel_fpx_emit_spsq_diff_roll(uint32_t ctx);

/*
 * Coercion-matrix query. Returns gate-cost estimate for (src → dst)
 * lowering, or IR_FPX_COERCE_UNIMPL (255) if edge is not wired.
 * Stub implementation records every call regardless of outcome —
 * this is what feeds the empirical heatmap.
 */
#define IR_FPX_COERCE_UNIMPL 255
#define IR_FPX_COERCE_IDENT  0

uint8_t ir_tel_fpx_coerce_query(uint8_t src, uint8_t dst);
uint8_t ir_tel_fpx_coerce_cast_attempt(uint8_t src, uint8_t dst, uint32_t ctx);

/* Convenience — true iff edge is implemented (cost < UNIMPL) */
static inline int ir_tel_fpx_coerce_supported(uint8_t src, uint8_t dst) {
    return ir_tel_fpx_coerce_query(src, dst) != IR_FPX_COERCE_UNIMPL;
}

#endif /* IR_TELEMETRY_FPX_H */
