#!/usr/bin/env python3
"""CG-IR-011: Force all callee-saved register saves when IR backend is active.

The AST prologue only saves regs that allocate_registers() identified for the
AST code path. But the IR linear scan may assign different callee-saved regs.
Without saving them all, the IR body clobbers rbp/return address → RIP=0x1.

Fix: After allocate_registers(), if the IR backend will handle this function,
set used_regs = 0x1F (all 5 callee-saved: r12, r13, r14, r15, rbx).
"""
import sys

path = "/mnt/h/__DOWNLOADS/selforglinux/part4.c"
src = open(path).read()

old = "  used_regs = allocate_registers(func);\n"

new = """  used_regs = allocate_registers(func);
  /* CG-IR-011: When the IR backend will handle the body, its linear scan
   * may assign callee-saved regs that allocate_registers() didn't predict.
   * Force saving ALL 5 callee-saved regs (r12,r13,r14,r15,rbx) so the
   * IR body can't clobber them without a matching save/restore.
   * The stack space is already reserved (stack_size includes +40). */
  if (getenv("ZCC_IR_BACKEND") || getenv("ZCC_IR_LOWER") || ir_whitelisted(func->func_def_name))
    used_regs = 0x1F;
"""

if old not in src:
    print("ERROR: could not find 'used_regs = allocate_registers(func);'")
    sys.exit(1)

if "CG-IR-011" in src:
    print("CG-IR-011 already applied")
    sys.exit(0)

src = src.replace(old, new, 1)
open(path, "w").write(src)
print("OK: CG-IR-011 applied — all callee-saved regs forced when IR active")
