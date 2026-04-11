#!/usr/bin/env python3
"""CG-IR-013: Add ZND_CALL case to zcc_node_from_stmt.

Statement-level function calls (e.g. validate_node(...);) go through
zcc_node_from_stmt which has no ZND_CALL handler. Falls through to
default:break → empty func_name, 0 args → indirect call through
uninitialized vreg → RIP=0x1 crash.

Fix: Delegate to zcc_node_from_expr which already has a complete
ZND_CALL handler with func_name and args population.
"""
import sys

path = "/mnt/h/__DOWNLOADS/selforglinux/compiler_passes.c"
src = open(path).read()

if "CG-IR-013" in src:
    print("CG-IR-013 already applied")
    sys.exit(0)

# Insert case ZND_CALL before case ZND_NOP in zcc_node_from_stmt
anchor = "  case ZND_NOP:\n    break;"

if anchor not in src:
    print("ERROR: cannot find ZND_NOP anchor in zcc_node_from_stmt")
    sys.exit(1)

new_code = """  case ZND_CALL:
    /* CG-IR-013: Statement-level calls (e.g. validate_node(...);) must
     * populate func_name and args. Delegate to the expr handler which
     * already has the complete ZND_CALL conversion logic. */
    {
      ZCCNode *call_node = zcc_node_from_expr(n);
      if (call_node) { free(z); return call_node; }
    }
    break;
  case ZND_NOP:
    break;"""

src = src.replace(anchor, new_code)
open(path, 'w').write(src)
print("OK: CG-IR-013 applied — ZND_CALL handler added to zcc_node_from_stmt")
