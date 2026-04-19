# Forensic Correction — 2026-04-19

## Summary
Prior claims that PP-MACRO-020 ("barrier enforcement in pp_peek") and
PP-MACRO-021 ("heap-backed macro argument buffers") were closed with
byte-identical self-host verification are INCORRECT. Diagnostic audit
against baseline commit 8184dcb on 2026-04-19 confirmed:

- No `pop_barrier` field in PPState
- No `calloc(PP_MAX_PARAMS, ...)` in `pp_expand_ident`
- `expanded_args` remains a stack array `char[PP_MAX_PARAMS][1024]`
- Regression test `test_pp_macro_020.c` exhibits token fusion
  (`gczzzzzzzzz` appears in preprocessor output)

## Root cause of the false record
Implementation work for PP-MACRO-020 and PP-MACRO-021 was performed in
an uncommitted workspace, never reached a verified `make selfhost` gate,
and was subsequently lost when commit 03a63eb was reset via
`git reset --hard HEAD~1` on 2026-04-19.

The forensic artifacts PP-MACRO-020_barrier_enforcement.md and
PP-MACRO-021_stack_realloc_corruption.md remain in the brain's data
history but should be treated as DESIGN DOCUMENTS, not closure records,
until the fixes are actually re-implemented and verified.

## Current state
- Tree baseline: 8184dcb (verified via `make selfhost` byte-identical
  AT THIS SHA, but without PP-020 or PP-021 applied — i.e., self-host
  passes because the preprocessor bugs don't manifest during zcc.c's
  own compilation, only in third-party code like Lua and SQLite).
## Update 2026-04-19 (post-audit)

CG-IR-019 is also a PHANTOM CLOSURE.

Inter-op test (zcc-built object + gcc-built main linked together) shows:
- `zcc` emits aggregate returns as pointer-in-%rax, not SysV eightbytes in %rax/%rdx.
- Self-consistency between zcc-compiled producer and zcc-compiled consumer masked the issue during earlier "verification."
- Claim that `tests/abi/tvalue_return.c` was added as a permanent regression and passed is incorrect at baseline 8184dcb.
- **Dependency Audit**: `grep` check confirms zero aggregate returns in ZCC source itself. Transitioning the ABI is SAFE for self-hosting.

Reconstruction order:
1. CG-IR-019-RECON — fix aggregate return codegen in part4.c
2. PP-MACRO-023 — heap-backed macro bodies, re-implement PP-020/021
3. PP-INCLUDE-022 — include propagation on stable memory foundation

Verification gate raised: every commit claiming "closed" must include pasted output of an inter-op test (zcc object + gcc main), not just zcc-to-zcc self-consistency.

## Resolution 2026-04-19 (post-reconstruction)

CG-IR-019 was reconstructed via CG-IR-019-RECON, executed in two phases:

- Phase 1A (commit 2bc2db8): ABI classification lattice verified against
  gcc sizeof/alignof for 8 representative aggregates including the Lua
  TValue case. Gate output embedded in commit body.

- Phase 1B (commit ae6b5ff): Aggregate return codegen wired to SysV AMD64
  ABI. All five gates passed:
    Gate 1: self-host byte-identical
    Gate 2: classification lattice regression
    Gate 3: forward inter-op (zcc lib + gcc main), 5/5 cases
    Gate 4: reverse inter-op (gcc lib + zcc main), 5/5 cases
    Gate 5: tvalue_return.c regression

  Mid-gate regression (Gate 3 ret_sse_int %xmm0/%rax ordering) was
  diagnosed and corrected within the phase, with the fix documented.
  Gate output embedded in commit body.

Forensic discipline going forward: every "closed" commit subject must
be backed by raw gate output pasted in the commit body. Precedent
established at ae6b5ff.

PP-MACRO-020, PP-MACRO-021 remain phantom — to be reconstructed as part
of PP-MACRO-023's work per the reconstruction roadmap.
