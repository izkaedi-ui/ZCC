/*
 * zcc_ir_bridge.h — ZCC's view of the IR boundary
 * =================================================
 * ZCC sees ONLY these three function declarations.
 * No structs. No PassResult. No stdint.h needed.
 *
 * ABI CONTRACT:
 *   - ZCC passes char* pointers and primitive types only
 *   - GCC (compiler_passes_ir.c) owns all struct lifecycle
 *   - Prevents the 2,048-byte PassResult offset divergence
 *
 * Compiled by GCC into the main zcc binary alongside zcc.c.
 * In run_selfhost.sh: #include "zcc_ir_bridge.h" is replaced
 * with #include "zcc_ir_bridge_zcc.h" before ZCC sees its source.
 */

#ifndef ZCC_IR_BRIDGE_H
#define ZCC_IR_BRIDGE_H

#include <stddef.h>

/* Returns "zcc_ir_v1.0" — static string, no allocation */
extern const char *zcc_ir_version(void);

/*
 * Lower text IR to JSON.
 *   ir_text    : flat text IR (fprintf'd by part5.c)
 *   json_path  : output file path  (e.g. "zcc_ir_dump.json")
 *   source_file: original .c filename for provenance
 * Returns 0 on success, -1 on failure.
 */
extern int zcc_ir_lower(const char *ir_text,
                         const char *json_path,
                         const char *source_file);

/* Free a char* allocated by this module (currently unused, future-proofing) */
extern void zcc_ir_free(void *ptr);

#endif /* ZCC_IR_BRIDGE_H */
