# ZCC Debugging Cheat Sheet

## Quick start

- **Symptom**: Crash or wrong output when compiling/running.
- **Boot**: Use the "Agentic OS" prompt in `AGENTS.md` (7-Phase Protocol + Phase 5.5 Stubbing + Squasher).
- **Full spec**: `docs/DEBUG_PROTOCOL.md`
- **Tools**: `scripts/stub_functions.py`, Double-Crumbs logs on stderr.

## 7-Phase protocol (order matters)

| Phase | Name        | Output |
|-------|-------------|--------|
| 0     | Intent      | One-sentence bug statement + exact repro command. |
| 1     | Landscape   | 10+ failure modes (Lexer/Parser/AST/Arena/Codegen), tag Likely/Unlikely. |
| 2     | Invariants  | Golden Rules (codegen) + Semantic Truths (wrong output). |
| 3     | Observability | ZCC:AST at codegen entry; ZCC:EMIT before CALL / stack adjust. |
| 4     | Reproduce   | Build + run repro; capture stderr (crumbs) and exit code. |
| 5     | Locate      | Last crumb → routine + node + fn + line; delta-debug to minimal repro. |
| 5.5   | Stubbing    | `stub_functions.py --keep main,<fn>,<routine>`; test; recurse. |
| 6     | Surgical fix| One minimal edit; map to one invariant; no extra changes. |
| 7     | Squasher    | CMP test, 7-arg stress, promotion stress, void-check; then victory. |

## Double-Crumbs (Phase 3)

- **ZCC:AST** `ZCC:AST <routine> <node_kind> [extra] fn=<name> line=<n>` at entry of `codegen_expr` / `codegen_stmt`.
- **ZCC:EMIT** `ZCC:EMIT <op> <operands> sp_off=<n> [regs=...]` before `emit_call()` and `emit_stack_adjust()`.
- **Use**: `./zcc2 in.c 2> debug.log` → `tail -n 1 debug.log` → last crumb = crash path; use `--keep main,<fn>` in stub script.

## Invariants (Phase 2)

- **Golden Rules**: RSP ≡ 0 (mod 16) before CALL; no Red Zone reliance; char/short promoted; pointers 8 bytes; RDI,RSI,RDX,RCX,R8,R9,R10,R11 clobbered by call.
- **Semantic Truths (wrong output)**: Short-circuit (&&/|| RHS not run when LHS decides); Signedness (IDIV vs DIV); Promotion (char+char→int); Struct value (full copy).

## Reduction (Phase 5 / 5.5)

- Halving: remove/stub code not on crash path; keep `main` + functions on path (from crumbs).
- Stubbing: `stub_functions.py in.c --keep main,<fn_from_crumb> --out stubbed.c` → build and test; if crash persists, bug in kept code; if not, un-stub and try another block.
- List functions: `stub_functions.py in.c --list`
- If stubbing breaks build (e.g. missing global): turn global into static init instead of deleting.

## Squasher (Phase 7) — must all pass

- [ ] **CMP**: `./zcc zcc.c -o zcc2 && ./zcc2 zcc.c -o zcc3 && cmp zcc2 zcc3`
- [ ] **7-arg stress**: function with 8 args compiles and runs.
- [ ] **Promotion stress**: `(unsigned char)255 + (unsigned char)1` → 256.
- [ ] **Void-check**: no use-after-free in arena (no pointer to block that may be reset).

## Ghost invariant (alignment)

If last `ZCC:EMIT` before crash is `CALL` with `sp_off=-24` (or any non-multiple of 16), the bug is **misaligned call**; fix by adjusting RSP before CALL so RSP % 16 == 0.
