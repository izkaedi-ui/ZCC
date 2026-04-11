/*
 * zcc_ir_bridge_zcc.h — ZCC-parseable substitute
 * ================================================
 * Identical declarations to zcc_ir_bridge.h but without
 * #include <stddef.h> which ZCC cannot resolve.
 *
 * In run_selfhost.sh, the sed pipeline substitutes:
 *   #include "zcc_ir_bridge.h"  →  #include "zcc_ir_bridge_zcc.h"
 *
 * ZCC compiles this version. GCC compiles the real one.
 * The resulting .o files link cleanly because the ABI
 * signatures are identical (3 pointer/int args, no structs).
 */

#ifndef ZCC_IR_BRIDGE_ZCC_H
#define ZCC_IR_BRIDGE_ZCC_H

extern const char *zcc_ir_version(void);
extern int zcc_ir_lower(const char *ir_text,
                         const char *json_path,
                         const char *source_file);
extern void zcc_ir_free(void *ptr);

#endif /* ZCC_IR_BRIDGE_ZCC_H */
