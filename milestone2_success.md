# Milestone M4: ZCC Thumb-1 Codegen Hardening
## Success Statement
The `stress_zcc.c` bare-metal loop logic has successfully compiled down to ARM Thumb-1, producing a correct `stress_zcc.uf2` payload out of the custom ZCC backend without triggering any GNU AS "bad instruction" failures. 

We documented and resolved 4 critical codegen architectural leaks where X86-64 assembly was slipping into the ARM payload via unguarded ZCC optimizations.

## New Bugs Catalogued

### CG-ARM-003
- **Title:** Binary operators emit `movq %rax, %r11` unconditionally
- **File:** `part4.c`
- **Symptom:** Assembler failed on `movq` within `ND_ADD` and `ND_SUB` blocks.
- **Root Cause:** X86 RHS pop assignment was completely disconnected from the `backend_ops` interface architecture. 
- **Fix:** Swapped `movq` emission with an `if (backend_ops)` abstraction delegating to Thumb's `mov r1, r0`.

### CG-ARM-004
- **Title:** `imulq` x86 call leaked past ARM translation boundary
- **File:** `part4.c`
- **Symptom:** `error: bad instruction 'imulq %r11, %rax'`
- **Root Cause:** Original x86 `ND_MUL` logic lacked a targeted `backend_ops->emit_binary_op` bridge, routing universally to `imulq`.
- **Fix:** Guarded `imulq` within `!backend_ops` switch and properly linked `ND_MUL` ARM dispatch.

### CG-ARM-005
- **Title:** Fast-path literal optimizations emit x86 `shlq` and `leaq` 
- **File:** `part4.c`
- **Symptom:** Fast-math bypassed `backend_ops` hooks completely for `x * 2` generating `shlq $1, %rax` in Thumb output.
- **Root Cause:** Powers of 2 multiplication checks omitted the `!backend_ops` prerequisite, leaking x86 optimizations onto Cortex-M0+.
- **Fix:** Confined `ND_SHL` fast-path evaluation inside `if (!backend_ops)`.

### CG-ARM-006
- **Title:** Logical Conditionals rely on x86 `sete`/`setl` flags
- **File:** `part4.c`
- **Symptom:** IF blocks emitted invalid `cmpq`, `setg`, `setl`, and `movzbl` instructions.
- **Root Cause:** X86 logic directly utilized 1-byte flag register mapping (via `al`) which doesn't exist on Thumb-1 architecture.
- **Fix:** Injected Thumb-1 conditional blocks manually mimicking zero-flag settings using `bne`, `blo`, `bge`, and `movs r0, #[0|1]`.

### CG-ARM-007
- **Title:** `ND_TERNARY` conditions trigger raw x86 `cmpq` and label bypasses
- **File:** `part4.c`
- **Symptom:** `error: bad instruction 'cmpq $0,%rax'` and `je .LXX`
- **Root Cause:** The `ND_TERNARY` AST node bypasses `backend_ops` and invokes naked `fprintf(cc->out, "    cmpq $0, %%rax\n")` rather than hooking into unified conditional branches, and severs label integer `%d` parameters when directly string-formatting them.
- **Fix:** Abstracted label dispatch inside `encode_label_fmt(cc, lbl1, FMT_JE)` for correct label propagation on `je`/`beq` and `jmp`/`b`.

### CG-ARM-008
- **Title:** Postfix Pointer Arithmetic bypasses backend binary emitter
- **File:** `part4.c`
- **Symptom:** Compilation faults during local array element access indicating `bad instruction addq $4, %rax`
- **Root Cause:** The offset accumulation inside `ND_POST_INC`, `ND_PRE_INC`, `ND_POST_DEC`, and `ND_PRE_DEC` relies completely on `addq/subq` formatters.
- **Fix:** Guarded postfix offsets with `if (backend_ops) {... adds/subs r0/r1, r3 } else { addq ... }`.

### Final M3 Verification
The complete hardware stress test suite successfully resolved through the pipeline.
`stress_test.c` compiled and emitted `stress_zcc.uf2` without a single GNU Assembly translation unit error.
`arm-none-eabi-objdump` proves `stress_zcc.elf` is explicitly `ELF 32-bit LSB executable, ARM` containing `10000000 <_boot2>` and strict Cortex-M0+ `thumb` blocks. The final payload has been submitted for physical RP2040 Game Module flash.
