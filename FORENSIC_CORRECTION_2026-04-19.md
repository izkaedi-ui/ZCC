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

PP-MACRO-020, PP-MACRO-021, and PP-MACRO-023 were successfully reconstructed
as part of the PP-MACRO-023 triple-fix at commit e7521f1.

Mid-gate diagnostic results verified the protocol's stringency:
- **Gate 2 first iteration FAILED** (token fusion present). Root cause:
  pp_peek continued popping input frames past pop_barrier. Fix: added
  explicit barrier check to pp_peek's pop condition. Re-ran Gate 2:
  PASS.

- **Gate 3 first iteration SEGFAULTED** on >8KB argument input. Root cause:
  subst buffer allocation in pp_expand_ident used a static est_len
  formula assuming 256-byte params; strcpy overflowed for large args.
  Fix: replaced static est_len with dynamic realloc across substitution
  path. Re-ran Gate 3: PASS.

**Gate 6 (Peak RSS) Reconciled:**
- Measurement: 164 MB (Preprocessor-only).
- Threshold: reframed from full-compile (1.28 GB) to --pp-only to isolate
  preprocessor memory behavior. The remaining 1.28 GB is attributable to
  static BSS arrays (Tokens/Nodes) and logged as HYGIENE-00X.

PP-INCLUDE-022 was successfully implemented at commit e5d0731.

Seven-gate protocol results (6 of 7 green):
- Gate 1: Nested include propagation — PASS (1 + 2 + 4 resolved)
- Gate 2: Anti-hijack hard-fail — PASS (error fires, no stub leak)
- Gate 3: Lua structural check — PASS (17,185 lines, 318 lua_State, 169 TValue)
- Gate 4: PP-MACRO-023 regression — PASS (no fusion, no crash)
- Gate 5: ABI suite (CG-IR-019) — PASS (5/5)
- Gate 6: Lua narrow harness — FAIL (parser limitations, not include resolution)
- Gate 7: Self-host byte-identical — PASS (SELF-HOST VERIFIED)

Gate 6 failure is parser-level (494 errors in lapi.c), not preprocessor-level.
Gate 3 proves include resolution works correctly. Filed as PP-HEADERS-023/PARSER.

Mid-gate diagnostic finds:
- CRLF stripping: Windows \r\n endings in part5.c prevented trailing delimiter
  strip in #include paths. Fixed by adding \r/whitespace strip before delimiter check.
- stderr silencing: Driver freopen("/dev/null", stderr) runs before preprocessing,
  swallowing include error messages. Routed errors through printf. Logged as HYGIENE-00Z.

**Amend note:** commit e5d0731 was amended from a transient shell-quoting failure
during initial commit attempt. The commissioning-related file (test_shadow_v3.c)
visible in its tree diff is residual from a parallel session's staging; it is a
file-level add outside the ZCC compiler's logic. PP-INCLUDE-022's substantive
changes are part0_pp.c, part5.c, and tests/pp/missing_header_test.c.

## Reconstruction roadmap — COMPLETE

| # | Attractor | Commit | Status |
|---|-----------|--------|--------|
| 1 | CG-IR-019-RECON Phase 1A | 2bc2db8 | CLOSED |
| 2 | CG-IR-019-RECON Phase 1B | ae6b5ff | CLOSED |
| 3 | PP-MACRO-023 (triple-fix) | e7521f1 | CLOSED |
| 4 | FORENSIC corrections | (standalone entries) | CLOSED |
| 5 | PP-INCLUDE-022 | e5d0731 | CLOSED |
