# ZCC Status Report
Generated: 2026-04-04 13:58:45 PDT

## Build Health

| Component | Status |
|-----------|--------|
| AST Selfhost | VERIFIED (zcc2.s == zcc3.s) |
| IR Backend | FAILED |
| IR Functions | 176 compiled through IR |
| Blacklist Hits | 0
0 functions fell back to AST |
| IR Gate | ir_whitelisted() returns ? |
| Blacklist | node_new,cc_func: %s\n,ZCC_IR_LOWER,rdi,rsi,rdx,rcx,r8,r9,    .text\n,,,ZCC_IR_PARAM_NAMES,ZCC_IR_PARAM_NAMES |

## Source Sizes

| File | Lines |
|------|-------|
| zcc_pp.c (concatenated) | 7792 |
| part4.c (codegen) | 2635 |
| compiler_passes.c (IR passes) | 7317 |
| compiler_passes_ir.c (IR helpers) | 570 |

## CG-IR Bug Fixes Applied
 CG-IR-005 CG-IR-008 CG-IR-009 CG-IR-011 CG-IR-012 CG-IR-013

## Optimization Pass Stats (last IR run)
- [IR-Opts] Folded: 9 | S-Reduce: 2 | Copy-Prop: 0 | Peephole: 16
- [RLE]       redundant loads eliminated: 2
- [DCE->SSA]  instructions removed (after mem2reg): 55  blocks removed: 366
- [EscapeAna] allocations promoted to stack: 12  (of 15 total)
- [Mem2Reg]   single-block allocas promoted: 11

## Architecture

- Dual-emission: AST-direct (part4.c codegen_expr/codegen_stmt) + IR backend (compiler_passes.c)
- IR gate: ir_whitelisted() in part4.c controls which functions use IR
- Hybrid frame: AST owns prologue/epilogue, IR owns body (body_only=1, slot_base=-stack_size)
- Bootstrap: GCC -> zcc -> zcc2.s -> zcc2 -> zcc3.s -> cmp zcc2.s zcc3.s
- Build: make clean && make selfhost
- IR test: bash verify_ir_backend.sh
- Environment: Windows + WSL, PowerShell -> wsl -e sh -c
- Working dir: /mnt/h/__DOWNLOADS/selforglinux

## Key Code Locations

| What | File | Line(s) |
|------|------|---------|
| ir_whitelisted gate | part4.c | ~1890 |
| codegen_func | part4.c | ~1909 |
| IR body entry | compiler_passes.c | zcc_run_passes_emit_body_pgo ~5394 |
| run_all_passes | compiler_passes.c | ~4451 |
| Mem2Reg (single-block) | compiler_passes.c | scalar_promotion_pass ~1478 |
| Mem2Reg (multi-block) | compiler_passes.c | multi_block_mem2reg_one ~1681 |
| PHI edge copy | compiler_passes.c | ir_asm_emit_phi_edge_copy ~4826 |
| IRAsmCtx struct | compiler_passes.c | ~4775 |
| ir_asm_vreg_location | compiler_passes.c | ~4790 |

## Known Issues

- LICM pass is commented out in run_all_passes

## Next Steps (suggested)
- Register allocation improvements (reduce spills)
- Re-enable LICM pass
- ASan run to confirm SARIF CWE-416/415 findings
- Per-function regression test suite (zcc_test_suite.sh)

---

## ZCC SQLite Milestone — April 10, 2026
- SQLite 3.45.0 compiled by ZCC
- Full SQL round trip verified:
  open rc=0
  SELECT 1 = 1
  CREATE TABLE rc=0
  INSERT rc=0  
  SELECT x = 42
- All rc=0, zero errors, zero segfaults

Bugs closed to achieve this:
  CG-IR-007: movslq width
  va_list phases 1-3: System V ABI
  Global struct initializer: recursive emitter
  Array-of-struct: budget cursor
  Array parameter decay
  ND_NEG: negative array initializers → yyRuleInfoNRhs
  struct-by-value Token ABI
  __atomic_* inline
  cltq pointer corruption (8 sites)
  __builtin_va_end linker
  Makefile -no-pie
  sizeof(char_array) = 8 bug
  Octal escape sequences unimplemented

🔱 ZKAEDI PRIME: CONVERGED
