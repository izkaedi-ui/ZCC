# ZCC IR Backend Bootstrap - Debug Session State
# Date: 2026-04-01
# Working directory: /mnt/h/__DOWNLOADS/selforglinux (via WSL)

## FIXES APPLIED TO compiler_passes.c (all cumulative)

### CG-IR-004: Phantom callee-save push/pop (VERIFIED FIXED)
- body_only flag added to IRAsmCtx
- Guarded pushq/popq in ir_asm_emit_one_block with !ctx->body_only
- Applied via fix_cgir004.sh (from earlier session)

### CG-IR-006: Stack frame underflow → CG-IR-009: Pre-scan frame extension (VERIFIED FIXED)
- Removed (void)stack_size suppression
- Pre-scans IR to compute max_reg, n_alloca, alloca_bytes
- Emits subq to extend frame: 8*(max_reg + n_alloca + 8) + alloca_bytes

### CG-IR-007: OP_CALL alignment with phantom n_csave (VERIFIED FIXED)
- n_csave = ctx->body_only ? 0 : __builtin_popcount(...)

### CG-IR-008: AST/IR stack layout collision (VERIFIED FIXED)
- Added slot_base to IRAsmCtx (-stack_size for body_only, 0 for standalone)
- ir_asm_vreg_location uses: slot_base - 8*(r+1) instead of -8*(r+2)
- alloca_off for non-params also offset by slot_base

### CG-IR-010: Force 8-byte OP_LOAD/OP_STORE (APPLIED, NOOPT STILL CRASHES)
- Removed movl/movslq 4-byte path from OP_LOAD
- Removed movl 4-byte path from OP_STORE
- Always uses movq for both

## CURRENT STATUS

### Optimized build (all passes enabled): rc=1, no crash
- The for-loop comparison is wrong (compares argv pointer vs argc instead of i vs argc)
- This is a Mem2Reg/LICM bug that substitutes the wrong SSA register
- The PHI edge copy function (ir_asm_emit_phi_edge_copy) only handles FIRST PHI per block

### Unoptimized build (run_all_passes returns early): SEGFAULT (rc=139)
- Crashes in __strcmp_evex called from main at line 6597
- GDB shows: rdi=0x2a00007fffffffe8 (corrupted pointer), rsi="-o" (correct)
- The 42 (0x2a) from test program's return value appears in corrupted rdi
- Despite CG-IR-010 forcing movq, pointer still corrupted
- LIKELY: another OP_LOAD/OP_STORE path exists, or the corruption comes from elsewhere

## NEXT STEPS TO INVESTIGATE

1. Verify CG-IR-010 actually changed the generated assembly:
   grep -c 'movl.*(%rax)' noopt.s    # should be near zero if fix worked
   grep 'movl.*(%rax)' noopt.s | head -10

2. Check if there's a SECOND OP_LOAD/OP_STORE handler in the codebase:
   grep -n 'case OP_LOAD:' compiler_passes.c
   grep -n 'case OP_STORE:' compiler_passes.c

3. Examine the for-loop assembly in noopt.s around strcmp:
   grep -n 'strcmp' noopt.s | head -5
   # Then look at the 30 lines before the first strcmp call

4. Check if the OP_CALL push/pop is corrupting r10/r11 values
   that contain pointer data being passed to strcmp

5. Consider whether the standalone ir_to_x86.c emitter is being
   used instead of the body_only emitter for some functions

## KEY CODE LOCATIONS

- IRAsmCtx struct: ~line 4516
- ir_asm_vreg_location: line 4537 (modified by CG-IR-008)
- ir_asm_emit_one_block: line ~4883
- ir_asm_emit_function_body: line 4916
- ir_asm_emit_phi_edge_copy: line ~4570 (BUG: only first PHI)
- OP_CALL handler: line ~4825
- OP_LOAD handler: line 4689 (modified by CG-IR-010)
- OP_STORE handler: line ~4705 (modified by CG-IR-010)
- alloca_off assignment: line 4865 (modified by CG-IR-008)
- zcc_run_passes_emit_body_pgo: line ~5096
- ZND_FOR handler (AST->IR): line 3546

## BACKUP FILES
- compiler_passes.c.bak_cgir004 through .bak_cgir010
- All fix scripts: fix_cgir006.py through fix_cgir010.py
