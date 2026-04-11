# ZCC — A Self-Hosting C Compiler with an Optimizing IR Backend

ZCC is a self-hosting C compiler targeting x86-64 Linux (System V AMD64 ABI). It compiles a substantial subset of C, including structs, pointers, arrays, function pointers, enums, switch statements, and the full complement of arithmetic, bitwise, logical, and comparison operators. ZCC compiles itself — the bootstrap chain produces byte-identical output across stages.

The compiler has two codegen paths: a direct AST-to-assembly emitter for rapid compilation, and a 3-address SSA IR backend with a full optimization pipeline. Both paths produce correct, self-hosting output.

```
GCC → zcc₁ → zcc₁ compiles itself → zcc₂.# ZCC — A Self-Hosting C Compiler with an Optimizing IR Backend

ZCC is a self-hosting C compiler targeting x86-64 Linux (System V AMD64 ABI). It compiles a substantial subset of C, including structs, pointers, arrays, function pointers, enums, switch statements, and the full complement of arithmetic, bitwise, logical, and comparison operators. ZCC compiles itself — the bootstrap chain produces byte-identical output across stages.

The compiler has two codegen paths: a direct AST-to-assembly emitter for rapid compilation, and a 3-address SSA IR backend with a full optimization pipeline. Both paths produce correct, self-hosting output.

```

GCC → zcc₁ → zcc₁ compiles itself → zcc₂.s
                                      ↓
                              gcc assembles → zcc₂
                                      ↓
                              zcc₂ compiles itself → zcc₃.s
                                      ↓
                              cmp zcc₂.s zcc₃.s → IDENTICAL ✓

```

## Status (Apr 5 2026)

| Metric | Result |
|--------|--------|
| Self-hosting | ✅ `zcc2.s == zcc3.s` (byte-identical) |
| Regression tests | ✅ 21/21 pass |
| Differential fuzz (vs GCC) | ✅ 10/10 seeds pass |
| IR backend tests | ✅ 21/21 pass |
| Peephole elisions (self-compile) | 4,413 |
| Functions through IR | 175+ |
| Metacompiler | ✅ ZCC compiled a C compiler that emits correct x86-64 |

### Metacompiler Chain

ZCC can compile a working C compiler (`mincc2.c`) which itself emits correct x86-64 assembly:

```

ZCC → compile mincc2.c → mincc2 binary
mincc2 → parse "int add(int x) { return x + 1; }" → emit x86-64 ASM
gcc → assemble emitted ASM → binary
binary: add(41) = 42 ✓

```

### Programs Compiled and Verified Correct

| Program | What It Tests |
|---------|--------------|
| Bubblesort | Nested loops, array indexing, comparisons |
| Linked list (push/print) | Structs, malloc, pointer chasing, while loops |
| Fibonacci + Factorial | Recursion, multiple call depths |
| String utilities (strlen/strcpy/reverse) | Char arrays, pointer arithmetic |
| Quicksort | Recursive partitioning, pointer swaps |
| Linked list (insert/delete/search) | Recursive delete, struct mutation |
| Expression evaluator | Recursive descent, operator precedence, parentheses |
| Brainfuck interpreter | Nested loops, bracket matching, char array VM |
| Hash table | Pointer-to-pointer buckets, collision chaining, update-in-place |
| Binary search tree | Recursive insert/delete/inorder traversal |
| Stack-based VM | Opcodes, global state, instruction dispatch |
| Bump allocator | Static heap, pointer arithmetic, multi-array ops |
| Lexer (mincc.c) | Tokenizes C source — ZCC lexing C inside C |
| Metacompiler (mincc2.c) | Full compiler pipeline: lex → parse → emit x86-64 |

## Quick Start

```bash
# Clone
git clone https://github.com/zkaedii/zcc.git
cd zcc

# Bootstrap: GCC → zcc → zcc₂ → zcc₃ → verify
make selfhost

# Run the regression test suite
bash zcc_test_suite.sh --quick

# Compile a program
./zcc hello.c -o hello.s
gcc -o hello hello.s
./hello

# Run differential fuzzer (compare ZCC output vs GCC)
python3 zcc_fuzz.py --zcc ./zcc --count 100 --output-dir ./fuzz_out
```

## What ZCC Compiles

**Types:** `char`, `short`, `int`, `long`, `long long` (signed and unsigned), `void`, `enum`, `struct`, `union`, pointers, arrays, function pointers, `_Bool`

**Statements:** `if`/`else`, `while`, `do`/`while`, `for`, `switch`/`case`/`default`, `break`, `continue`, `return`, `goto`, labels, compound statements, expression statements

**Expressions:** full arithmetic (`+`, `-`, `*`, `/`, `%`), bitwise (`&`, `|`, `^`, `~`, `<<`, `>>`), logical (`&&`, `||`, `!`), comparison (`==`, `!=`, `<`, `<=`, `>`, `>=`), assignment (`=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`), increment/decrement (`++`, `--`, pre and post), ternary (`?:`), comma, `sizeof`, casts, address-of (`&`), dereference (`*`), member access (`.`, `->`), array subscript (`[]`), function calls (up to 6 register args per System V ABI)

**Declarations:** local variables, global variables, global arrays, string literals, function definitions, forward declarations, `extern`, `static`, `typedef`

**Preprocessor:** ZCC does not include a preprocessor. Use `cpp` or `gcc -E` for preprocessing before feeding source to ZCC.

## Architecture

ZCC is split across multiple source files that are concatenated into a single `zcc.c` for compilation:

```
┌─────────────────────────────────────────────────────────┐
│                    zcc.c (concatenated)                  │
│                                                         │
│  part1.c ─── Types, nodes, symbols, scope, allocator    │
│  part2.c ─── Lexer (tokenizer)                          │
│  part3.c ─── Recursive descent parser                   │
│  ir.h    ─── IR instruction set, data structures        │
│  ir_emit_dispatch.h ─ IR emission macros                │
│  ir_bridge.h ─────── AST ↔ IR translation layer         │
│  part4.c ─── x86-64 codegen, register allocator         │
│  part5.c ─── main(), peephole optimizer, globals        │
│  ir.c    ─── IR module, standalone lowerer              │
│  ir_to_x86.c ─────── Standalone IR-to-x86 lowerer      │
│                                                         │
├─────────────────────────────────────────────────────────┤
│              Linked separately:                         │
│                                                         │
│  compiler_passes.c ── IR optimization pipeline +        │
│                       body emitter (7,317 lines)        │
│  compiler_passes_ir.c ── IR pass helpers (570 lines)    │
└─────────────────────────────────────────────────────────┘
```

### Dual-Emission Codegen

ZCC has two code generation paths, selectable per-function:

**AST-Direct Path** (`codegen_expr` / `codegen_stmt` in `part4.c`)

- Tree-walk code generation directly from the AST
- Emits x86-64 assembly with no intermediate representation
- Includes strength reduction (multiply by 2/3/5/9 → `lea`), signed/unsigned dispatch, and a peephole optimizer
- This is the proven path — self-hosting has been verified through this backend for months

**IR Backend** (`compiler_passes.c`)

- Translates AST → 3-address SSA IR via `ir_bridge.h`
- Runs a full optimization pipeline (see below)
- Emits optimized x86-64 assembly from the IR
- Operates inside the AST's stack frame (`body_only` mode — AST owns prologue/epilogue, IR owns the function body)

The IR backend is gated by `ir_whitelisted()` in `part4.c`. When enabled, `codegen_func()` emits the AST prologue, delegates the body to `zcc_run_passes_emit_body_pgo()`, then emits the AST epilogue. This hybrid design allows incremental migration from AST to IR per-function.

### IR Instruction Set

The IR uses a register-machine model with unlimited virtual registers. Instructions:

**Arithmetic:** `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `NEG`
**Bitwise:** `AND`, `OR`, `XOR`, `SHL`, `SHR`, `NOT`
**Comparison:** `EQ`, `NE`, `LT`, `LE`, `GT`, `GE`
**Memory:** `LOAD`, `STORE`, `ALLOCA`, `ADDR`
**Control:** `BR`, `CONDBR`, `RET`, `CALL`, `PHI`
**Other:** `CONST`, `COPY`, `CAST`, `NOP`, `PGO_COUNTER_ADDR`

### Optimization Pipeline

`run_all_passes()` in `compiler_passes.c` executes these passes in order:

1. **Reachability Analysis** — marks unreachable blocks
2. **Parameter Escape Marking** — protects function parameters from Mem2Reg
3. **PGO Instrumentation** — optional counter probe injection
4. **Constant Folding** — evaluates compile-time constant expressions
5. **Strength Reduction** — simplifies expensive operations (multiply → shift)
6. **Copy Propagation** — eliminates redundant register copies
7. **Peephole Optimization** — pattern-matched simplifications at IR level
8. **Redundant Load Elimination** — removes duplicate memory loads
9. **Dead Code Elimination (SSA)** — removes instructions with no live uses
10. **Escape Analysis** — determines which allocations can be stack-promoted
11. **Scalar Promotion (Mem2Reg)** — promotes stack variables to SSA registers
    - Single-block fast path for variables used within one block
    - Multi-block path using dominance frontiers and iterated DF for PHI placement
    - SSA rename via recursive dominator tree walk
12. **PGO Block Reordering** — reorders basic blocks by execution frequency for icache locality

Measured results on ZCC compiling itself: 4,413 peephole instructions elided, 10.8% dead code eliminated across 175+ functions, 11 single-block allocas promoted to registers.

### Register Allocation

The IR backend uses a linear scan register allocator with liveness intervals computed from instruction numbering. Physical registers are assigned from the System V ABI callee-saved set (`rbx`, `r12`-`r15`). Spills go to stack slots below the AST frame via the `slot_base` offset.

### Peephole Optimizer

The AST-direct backend includes a post-emission peephole optimizer (`peephole_optimize` in `part5.c`) that scans the assembly output for redundant instruction patterns:

- Redundant `mov` (same source and destination)
- Push/pop of the same register
- Dead stores overwritten before use
- Redundant loads after stores to the same location
- Strength reduction patterns in emitted assembly

The peephole pass eliminates 4,413 instructions when ZCC compiles itself.

## Build System

```makefile
# Standard build (GCC bootstrap)
make

# Full self-hosting verification
make selfhost

# ASan build for debugging
make asan

# Clean all build artifacts
make clean
```

**Requirements:** GCC (or any C compiler for initial bootstrap), GNU Make, x86-64 Linux (native or WSL)

**Bootstrap chain:**

1. `gcc` compiles `zcc.c` + `compiler_passes.c` + `compiler_passes_ir.c` → `zcc` (stage 0)
2. `zcc` compiles `zcc.c` → `zcc2.s` (stage 1)
3. `gcc` assembles `zcc2.s` + passes → `zcc2` (stage 1 binary)
4. `zcc2` compiles `zcc.c` → `zcc3.s` (stage 2)
5. `cmp zcc2.s zcc3.s` — must be byte-identical (self-host invariant)

## Testing

### Regression Test Suite

`zcc_test_suite.sh` contains 21 targeted tests across 10 categories:

| Category | Tests | What It Covers |
|----------|-------|----------------|
| Basic Operations | 3 | Return values, arithmetic, if-branches |
| Loops / Mem2Reg | 3 | For-loops, while-loops, multiple local variables |
| Pointers / Memory | 2 | Pointer dereference, array pointer arithmetic |
| Function Calls | 3 | Simple calls, 6-arg calls, recursion (fibonacci) |
| Structs | 1 | Struct member access |
| Switch | 1 | Switch/case/default |
| Globals | 1 | Global variables, cross-function mutation |
| Complex Control Flow | 3 | Nested loops, ternary expressions, logical operators |
| Register Pressure | 2 | High register count, 8+ local variables (spill stress) |
| cc_alloc Pattern | 1 | Malloc + zero-fill loop (original crash pattern) |

```bash
# Quick mode — unit tests only, skip selfhost
bash zcc_test_suite.sh --quick

# Full mode — unit tests + selfhost verification
bash zcc_test_suite.sh

# Filtered output (recommended)
bash zcc_test_suite.sh --quick 2>/dev/null | grep -E 'PASS|FAIL|SKIP|Category|══'
```

Each test compiles a C program through both AST and IR paths, links both, runs both, and compares exit codes and stdout. Any divergence is a regression.

### Differential Fuzzer

`zcc_fuzz.py` generates random C programs and compares ZCC output against GCC:

```bash
# 100 programs
python3 zcc_fuzz.py --zcc ./zcc --count 100 --output-dir ./fuzz_out

# Verbose with saved mismatches
python3 zcc_fuzz.py --zcc ./zcc --count 10 --verbose --output-dir ./fuzz_out
```

As of Apr 5 2026: 10/10 seeds pass. The fuzzer covers unsigned arithmetic, shift operations, rotate patterns, signed/unsigned comparisons, nested ternary expressions, and multi-function checksum accumulation.

### Seed Regression Test

`test_seeds.sh` runs the 10 known seed programs and reports pass/fail per seed:

```bash
bash test_seeds.sh
bash test_seeds.sh ./zcc_new  # test a different binary
```

### IR Backend Verification

`verify_ir_backend.sh` performs a 7-stage semantic equivalence check:

1. GCC bootstraps `zcc_host`
2. `zcc_host` compiles itself → `zcc2_ast` (AST backend)
3. `zcc2_ast` compiles source → `reference.s`
4. `zcc2_ast` compiles source with `ZCC_IR_BACKEND=1` → `zcc3_ir.s`
5. Link `zcc3_ir.s` → `zcc3_ir` binary
6. `zcc3_ir` compiles source → `check.s`
7. `cmp reference.s check.s` — must match

### Status Report

`zcc_status.sh` generates `ZCC_STATUS.md` — a machine-readable snapshot of the compiler's state:

```bash
bash zcc_status.sh
cat ZCC_STATUS.md
```

## Bug Corpus

ZCC's development produced a compiler bug corpus with ground-truth fixes, CWE classifications, and PRIME energy scores. All bugs were discovered and fixed through self-hosting failure analysis and GDB register-level tracing.

| Bug ID | Title | CWE | Severity |
|--------|-------|-----|----------|
| CG-IR-003 | stdout pointer corruption (stale binary sign extension) | CWE-704 | Critical |
| CG-IR-004 | Phantom callee-save push/pop in body_only mode | CWE-682 | Critical |
| CG-IR-005 | PHI liveness inversion, CONDBR copies, serial lost-copy | CWE-682 | Critical |
| CG-IR-006 | Stack frame too small for IR spill slots | CWE-121 | Critical |
| CG-IR-007 | Call alignment counted phantom pushes | CWE-682 | High |
| CG-IR-008 | AST/IR stack slot collision — parameter overwrite | CWE-787 | Critical |
| CG-IR-009 | Pre-scan frame depth missing alloca bytes | CWE-131 | High |
| CG-IR-010 | 4-byte movl for pointer load/store truncation | CWE-704 | Critical |
| CG-IR-011 | Callee-saved register mismatch between AST and IR | CWE-682 | Critical |
| CG-IR-012b | 33 hollow accessor stubs returning zero | CWE-476 | Critical |
| CG-IR-013 | ZND_CALL missing from stmt handler | CWE-839 | Critical |
| CG-IR-014 | ZND_ASSIGN missing from expr handler | CWE-839 | Critical |
| CG-017 | Missing cltq sign-extension after int-returning call | CWE-704 | High |
| FOLD-001 | Ternary constant fold not copying int_val/type/cond | CWE-682 | High |
| LEX-001 | Unsigned literal suffix U/L discarded in lexer | CWE-704 | Critical |

**LEX-001** (Apr 5 2026) was the most impactful fix: the lexer was silently discarding `U` and `L` suffixes on integer literals, causing all downstream type inference on unsigned constants to be wrong. This poisoned signed/unsigned comparisons, shift operations, bitwise NOT, and rotate patterns across the entire compiler. Fixed in `part2.c` and `part3.c`.

## Source Statistics

| Component | Lines | Description |
|-----------|-------|-------------|
| part1.c | ~1,200 | Types, nodes, symbols, scope, memory allocator |
| part2.c | ~800 | Lexer — tokenization, keywords, string/char literals |
| part3.c | ~1,500 | Parser — recursive descent, all C expressions and statements |
| part4.c | ~2,635 | x86-64 codegen, IR gate, register allocator |
| part5.c | ~1,100 | main(), peephole optimizer, global emission |
| ir.h | ~200 | IR instruction set definition |
| ir_bridge.h | 186 | AST ↔ IR translation layer |
| ir.c | ~400 | IR module creation, standalone lowerer |
| ir_to_x86.c | ~300 | Standalone IR-to-x86 (not used in body_only path) |
| compiler_passes.c | 7,317 | IR optimization pipeline + body emitter |
| compiler_passes_ir.c | 570 | IR pass helper functions |
| **Total** | **~16,200** | |

## Environment Variables

| Variable | Effect |
|----------|--------|
| `ZCC_IR_BACKEND=1` | Enable IR backend for all functions (overrides `ir_whitelisted()`) |
| `ZCC_IR_LOWER=1` | Alternative IR enable flag |
| `ZCC_PGO_INSTRUMENT=1` | Inject PGO counter probes into IR |
| `ZCC_DUMP_PGO_BLOCKS=1` | Dump PGO block info to stderr |
| `ZCC_GEN_PROFILE=<path>` | Write branch probabilities to file |

## Platform

- **Target:** x86-64 Linux (System V AMD64 ABI)
- **Host:** Any system with GCC and Make (including WSL)
- **Register args:** 6 (`rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`)
- **Callee-saved:** `rbx`, `r12`, `r13`, `r14`, `r15`
- **Stack alignment:** 16-byte at call sites
- **Pointer width:** 8 bytes (LP64)
- **No external dependencies** beyond libc and libm

## Limitations

- No preprocessor (use `gcc -E` or `cpp`)
- No floating-point codegen (parsed but not emitted)
- No variadic functions (`...`)
- No bitfields in structs
- No VLA (variable-length arrays)
- No `_Complex`, `_Atomic`, `_Generic`
- No inline assembly
- No multi-file compilation (concatenate or preprocess into one file)
- No debug info emission (no DWARF)
- x86-64 Linux only (no ARM, no macOS, no Windows native)

## Related Projects

- [**zcc-compiler-bug-corpus**](https://huggingface.co/datasets/zkaedi/zcc-compiler-bug-corpus) — real codegen bugs with ground-truth fixes
- [**zkaedi-cc**](https://huggingface.co/spaces/zkaedi/zkaedi-cc) — 18-agent Hamiltonian C static vulnerability analyzer
- [**ZKAEDI-MINI**](https://huggingface.co/zkaedi/ZKAEDI-MINI-GGUF) — 7B parameter model (Qwen2.5 + LoRA) for compiler and smart contract analysis
- [**zkaedi.ai**](https://zkaedi.ai) — Smart contract auditing platform powered by ZKAEDI PRIME

## How It Works (for compiler enthusiasts)

### The Lexer (part2.c)

Hand-written lexer producing tokens for all C keywords, operators, literals (integer, string, char), and identifiers. Handles escape sequences, hex/octal literals, integer suffix flags (`U`, `L`, `UL`, `LL`), and multi-character operators (`<=`, `>=`, `==`, `!=`, `<<`, `>>`, `&&`, `||`, `+=`, etc.). Keywords are recognized via a lookup table with hash-based fast path. Suffix metadata is preserved and propagated to the parser for correct unsigned type assignment.

### The Parser (part3.c)

Recursive descent parser implementing C's operator precedence through separate functions per precedence level: `parse_assign` → `parse_ternary` → `parse_logor` → `parse_logand` → `parse_bitor` → ... → `parse_unary` → `parse_postfix` → `parse_primary`. Produces an AST of `Node` structures with type annotations resolved during parsing. Integer literals with `U`/`L` suffixes are assigned `ty_uint`/`ty_ulong` types at parse time.

### The Codegen (part4.c)

Tree-walk code generator that emits x86-64 assembly. `codegen_expr()` handles all expression nodes, `codegen_stmt()` handles all statement nodes, `codegen_func()` orchestrates function-level emission including prologue, parameter stores, body, and epilogue. Register allocation is performed by `allocate_registers()` using liveness analysis to assign callee-saved registers.

Strength reduction is built into the AST codegen: multiply by powers of 2 uses `shl`, multiply by 3/5/9 uses `lea`, divide by powers of 2 uses `sar`/`shr` (with sign correction for signed types), modulo by powers of 2 uses `and`. All signed/unsigned operations dispatch to the correct instruction variant. After any `call` instruction, the return value in `%eax` is sign- or zero-extended to `%rax` based on the callee's return type (fixes CG-017).

### The IR Bridge (ir_bridge.h)

Translation layer that maps AST `Node` structures to `ZCCNode` structures consumable by the IR. Handles type mapping (18 C types → IR type enum), variable name extraction, binary operator mapping, and function boundary management. The bridge is the narrowest point in the pipeline — all AST information must flow through these ~186 lines.

### The Optimizer (compiler_passes.c)

7,317 lines implementing 12 optimization passes over SSA-form IR. The core data structures are `Function` (containing `Block` arrays), `Block` (doubly-linked list of `Instr`), and `Instr` (with opcode, destination register, source registers, PHI sources, and execution frequency).

Dominance is computed via the standard iterative algorithm. Dominance frontiers are used for PHI placement in the multi-block Mem2Reg pass. Liveness intervals drive the linear scan register allocator. The body emitter (`ir_asm_emit_function_body`) walks blocks in PGO-derived order, emitting x86-64 assembly with correct PHI resolution via parallel edge copies.

### The Self-Host Invariant

The compiler is correct if and only if `zcc₂.s == zcc₃.s` (byte-identical). This is a stronger property than "compiles without errors" — it means the compiler's own output, when used to compile itself, produces exactly the same assembly. Any codegen bug that affects any function in the compiler itself will break this invariant.

## License

Apache 2.0

## Author

**ZKAEDI** — [@LongNumberGeek](https://twitter.com/LongNumberGeek) — [zkaedi.ai](https://zkaedi.ai) — [HuggingFace](https://huggingface.co/zkaedi)

Built solo. Every line, every bug, every fix.
                                      ↓
                              gcc assembles → zcc₂
                                      ↓
                              zcc₂ compiles itself → zcc₃.s
                                      ↓
                              cmp zcc₂.s zcc₃.s → IDENTICAL ✓

```

## Quick Start

```bash
# Clone
git clone https://github.com/zkaedi/zcc.git
cd zcc

# Bootstrap: GCC → zcc → zcc₂ → zcc₃ → verify
make selfhost

# Run the regression test suite
bash zcc_test_suite.sh --quick

# Compile a program
./zcc2 hello.c -o hello.s
gcc -o hello hello.s
./hello
```

## What ZCC Compiles

**Types:** `char`, `short`, `int`, `long`, `long long` (signed and unsigned), `void`, `enum`, `struct`, `union`, pointers, arrays, function pointers, `_Bool`

**Statements:** `if`/`else`, `while`, `do`/`while`, `for`, `switch`/`case`/`default`, `break`, `continue`, `return`, `goto`, labels, compound statements, expression statements

**Expressions:** full arithmetic (`+`, `-`, `*`, `/`, `%`), bitwise (`&`, `|`, `^`, `~`, `<<`, `>>`), logical (`&&`, `||`, `!`), comparison (`==`, `!=`, `<`, `<=`, `>`, `>=`), assignment (`=`, `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=`), increment/decrement (`++`, `--`, pre and post), ternary (`?:`), comma, `sizeof`, casts, address-of (`&`), dereference (`*`), member access (`.`, `->`), array subscript (`[]`), function calls (up to 6 register args per System V ABI)

**Declarations:** local variables, global variables, global arrays, string literals, function definitions, forward declarations, `extern`, `static`, `typedef`

**Preprocessor:** ZCC does not include a preprocessor. Use `cpp` or `gcc -E` for preprocessing before feeding source to ZCC.

## Architecture

ZCC is split across multiple source files that are concatenated into a single `zcc.c` for compilation:

```
┌─────────────────────────────────────────────────────────┐
│                    zcc.c (concatenated)                  │
│                                                         │
│  part1.c ─── Types, nodes, symbols, scope, allocator    │
│  part2.c ─── Lexer (tokenizer)                          │
│  part3.c ─── Recursive descent parser                   │
│  ir.h    ─── IR instruction set, data structures        │
│  ir_emit_dispatch.h ─ IR emission macros                │
│  ir_bridge.h ─────── AST ↔ IR translation layer         │
│  part4.c ─── x86-64 codegen, register allocator         │
│  part5.c ─── main(), peephole optimizer, globals        │
│  ir.c    ─── IR module, standalone lowerer              │
│  ir_to_x86.c ─────── Standalone IR-to-x86 lowerer      │
│                                                         │
├─────────────────────────────────────────────────────────┤
│              Linked separately:                         │
│                                                         │
│  compiler_passes.c ── IR optimization pipeline +        │
│                       body emitter (7,317 lines)        │
│  compiler_passes_ir.c ── IR pass helpers (570 lines)    │
└─────────────────────────────────────────────────────────┘
```

### Dual-Emission Codegen

ZCC has two code generation paths, selectable per-function:

**AST-Direct Path** (`codegen_expr` / `codegen_stmt` in `part4.c`)

- Tree-walk code generation directly from the AST
- Emits x86-64 assembly with no intermediate representation
- Includes strength reduction (multiply by 2/3/5/9 → `lea`), signed/unsigned dispatch, and a peephole optimizer
- This is the proven path — self-hosting has been verified through this backend for months

**IR Backend** (`compiler_passes.c`)

- Translates AST → 3-address SSA IR via `ir_bridge.h`
- Runs a full optimization pipeline (see below)
- Emits optimized x86-64 assembly from the IR
- Operates inside the AST's stack frame (`body_only` mode — AST owns prologue/epilogue, IR owns the function body)

The IR backend is gated by `ir_whitelisted()` in `part4.c`. When enabled, `codegen_func()` emits the AST prologue, delegates the body to `zcc_run_passes_emit_body_pgo()`, then emits the AST epilogue. This hybrid design allows incremental migration from AST to IR per-function.

### IR Instruction Set

The IR uses a register-machine model with unlimited virtual registers. Instructions:

**Arithmetic:** `ADD`, `SUB`, `MUL`, `DIV`, `MOD`, `NEG`
**Bitwise:** `AND`, `OR`, `XOR`, `SHL`, `SHR`, `NOT`
**Comparison:** `EQ`, `NE`, `LT`, `LE`, `GT`, `GE`
**Memory:** `LOAD`, `STORE`, `ALLOCA`, `ADDR`
**Control:** `BR`, `CONDBR`, `RET`, `CALL`, `PHI`
**Other:** `CONST`, `COPY`, `CAST`, `NOP`, `PGO_COUNTER_ADDR`

### Optimization Pipeline

`run_all_passes()` in `compiler_passes.c` executes these passes in order:

1. **Reachability Analysis** — marks unreachable blocks
2. **Parameter Escape Marking** — protects function parameters from Mem2Reg
3. **PGO Instrumentation** — optional counter probe injection
4. **Constant Folding** — evaluates compile-time constant expressions
5. **Strength Reduction** — simplifies expensive operations (multiply → shift)
6. **Copy Propagation** — eliminates redundant register copies
7. **Peephole Optimization** — pattern-matched simplifications at IR level
8. **Redundant Load Elimination** — removes duplicate memory loads
9. **Dead Code Elimination (SSA)** — removes instructions with no live uses
10. **Escape Analysis** — determines which allocations can be stack-promoted
11. **Scalar Promotion (Mem2Reg)** — promotes stack variables to SSA registers
    - Single-block fast path for variables used within one block
    - Multi-block path using dominance frontiers and iterated DF for PHI placement
    - SSA rename via recursive dominator tree walk
12. **PGO Block Reordering** — reorders basic blocks by execution frequency for icache locality

Measured results on ZCC compiling itself: 4,331 peephole instructions elided, 10.8% dead code eliminated across 175+ functions, 11 single-block allocas promoted to registers.

### Register Allocation

The IR backend uses a linear scan register allocator with liveness intervals computed from instruction numbering. Physical registers are assigned from the System V ABI callee-saved set (`rbx`, `r12`-`r15`). Spills go to stack slots below the AST frame via the `slot_base` offset.

### Peephole Optimizer

The AST-direct backend includes a post-emission peephole optimizer (`peephole_optimize` in `part5.c`) that scans the assembly output for redundant instruction patterns:

- Redundant `mov` (same source and destination)
- Push/pop of the same register
- Dead stores overwritten before use
- Redundant loads after stores to the same location
- Strength reduction patterns in emitted assembly

The peephole pass eliminates 4,331 instructions when ZCC compiles itself.

## Build System

```makefile
# Standard build (GCC bootstrap)
make

# Full self-hosting verification
make selfhost

# Clean all build artifacts
make clean
```

**Requirements:** GCC (or any C compiler for initial bootstrap), GNU Make, x86-64 Linux (native or WSL)

**Bootstrap chain:**

1. `gcc` compiles `zcc.c` + `compiler_passes.c` + `compiler_passes_ir.c` → `zcc` (stage 0)
2. `zcc` compiles `zcc.c` → `zcc2.s` (stage 1)
3. `gcc` assembles `zcc2.s` + passes → `zcc2` (stage 1 binary)
4. `zcc2` compiles `zcc.c` → `zcc3.s` (stage 2)
5. `cmp zcc2.s zcc3.s` — must be byte-identical (self-host invariant)

## Testing

### Regression Test Suite

`zcc_test_suite.sh` contains 21 targeted tests across 10 categories:

| Category | Tests | What It Covers |
|----------|-------|----------------|
| Basic Operations | 3 | Return values, arithmetic, if-branches |
| Loops / Mem2Reg | 3 | For-loops, while-loops, multiple local variables |
| Pointers / Memory | 2 | Pointer dereference, array pointer arithmetic |
| Function Calls | 3 | Simple calls, 6-arg calls, recursion (fibonacci) |
| Structs | 1 | Struct member access |
| Switch | 1 | Switch/case/default |
| Globals | 1 | Global variables, cross-function mutation |
| Complex Control Flow | 3 | Nested loops, ternary expressions, logical operators |
| Register Pressure | 2 | High register count, 8+ local variables (spill stress) |
| cc_alloc Pattern | 1 | Malloc + zero-fill loop (original crash pattern) |

```bash
# Quick mode — unit tests only, skip selfhost
bash zcc_test_suite.sh --quick

# Full mode — unit tests + selfhost verification
bash zcc_test_suite.sh

# Filtered output (recommended)
bash zcc_test_suite.sh --quick 2>/dev/null | grep -E 'PASS|FAIL|SKIP|Category|══'
```

Each test compiles a C program through both AST and IR paths, links both, runs both, and compares exit codes and stdout. Any divergence is a regression.

### IR Backend Verification

`verify_ir_backend.sh` performs a 7-stage semantic equivalence check:

1. GCC bootstraps `zcc_host`
2. `zcc_host` compiles itself → `zcc2_ast` (AST backend)
3. `zcc2_ast` compiles source → `reference.s`
4. `zcc2_ast` compiles source with `ZCC_IR_BACKEND=1` → `zcc3_ir.s`
5. Link `zcc3_ir.s` → `zcc3_ir` binary
6. `zcc3_ir` compiles source → `check.s`
7. `cmp reference.s check.s` — must match

### Status Report

`zcc_status.sh` generates `ZCC_STATUS.md` — a machine-readable snapshot of the compiler's state. Paste it into any conversation or issue for instant context.

```bash
bash zcc_status.sh
cat ZCC_STATUS.md
```

## Bug Corpus

ZCC's development produced a [12-entry compiler bug corpus](https://huggingface.co/datasets/zkaedi/zcc-compiler-bug-corpus) with ground-truth fixes, CWE classifications, and PRIME energy scores. All 12 bugs were discovered and fixed in a single debugging session through self-hosting failure analysis and GDB register-level tracing.

| Bug ID | Title | CWE | Severity |
|--------|-------|-----|----------|
| CG-IR-003 | stdout pointer corruption (stale binary sign extension) | CWE-704 | Critical |
| CG-IR-004 | Phantom callee-save push/pop in body_only mode | CWE-682 | Critical |
| CG-IR-005 | PHI liveness inversion, CONDBR copies, serial lost-copy | CWE-682 | Critical |
| CG-IR-006 | Stack frame too small for IR spill slots | CWE-121 | Critical |
| CG-IR-007 | Call alignment counted phantom pushes | CWE-682 | High |
| CG-IR-008 | AST/IR stack slot collision — parameter overwrite | CWE-787 | Critical |
| CG-IR-009 | Pre-scan frame depth missing alloca bytes | CWE-131 | High |
| CG-IR-010 | 4-byte movl for pointer load/store truncation | CWE-704 | Critical |
| CG-IR-011 | Callee-saved register mismatch between AST and IR | CWE-682 | Critical |
| CG-IR-012b | 33 hollow accessor stubs returning zero | CWE-476 | Critical |
| CG-IR-013 | ZND_CALL missing from stmt handler | CWE-839 | Critical |
| CG-IR-014 | ZND_ASSIGN missing from expr handler | CWE-839 | Critical |

The full dataset with before/after patterns, detection methods, and regression tests is available at [`zkaedi/zcc-compiler-bug-corpus`](https://huggingface.co/datasets/zkaedi/zcc-compiler-bug-corpus) on HuggingFace.

## Source Statistics

| Component | Lines | Description |
|-----------|-------|-------------|
| part1.c | ~1,200 | Types, nodes, symbols, scope, memory allocator |
| part2.c | ~800 | Lexer — tokenization, keywords, string/char literals |
| part3.c | ~1,500 | Parser — recursive descent, all C expressions and statements |
| part4.c | ~2,635 | x86-64 codegen, IR gate, register allocator |
| part5.c | ~1,100 | main(), peephole optimizer, global emission |
| ir.h | ~200 | IR instruction set definition |
| ir_bridge.h | 186 | AST ↔ IR translation layer |
| ir.c | ~400 | IR module creation, standalone lowerer |
| ir_to_x86.c | ~300 | Standalone IR-to-x86 (not used in body_only path) |
| compiler_passes.c | 7,317 | IR optimization pipeline + body emitter |
| compiler_passes_ir.c | 570 | IR pass helper functions |
| **Total** | **~16,200** | |

## Environment Variables

| Variable | Effect |
|----------|--------|
| `ZCC_IR_BACKEND=1` | Enable IR backend for all functions (overrides `ir_whitelisted()`) |
| `ZCC_IR_LOWER=1` | Alternative IR enable flag |
| `ZCC_PGO_INSTRUMENT=1` | Inject PGO counter probes into IR |
| `ZCC_DUMP_PGO_BLOCKS=1` | Dump PGO block info to stderr |
| `ZCC_GEN_PROFILE=<path>` | Write branch probabilities to file |

## Platform

- **Target:** x86-64 Linux (System V AMD64 ABI)
- **Host:** Any system with GCC and Make (including WSL)
- **Register args:** 6 (`rdi`, `rsi`, `rdx`, `rcx`, `r8`, `r9`)
- **Callee-saved:** `rbx`, `r12`, `r13`, `r14`, `r15`
- **Stack alignment:** 16-byte at call sites
- **Pointer width:** 8 bytes (LP64)
- **No external dependencies** beyond libc and libm

## Limitations

- No preprocessor (use `gcc -E` or `cpp`)
- No floating-point codegen (parsed but not emitted)
- No variadic functions (`...`)
- No bitfields in structs
- No VLA (variable-length arrays)
- No `_Complex`, `_Atomic`, `_Generic`
- No inline assembly
- No multi-file compilation (concatenate or preprocess into one file)
- No debug info emission (no DWARF)
- x86-64 Linux only (no ARM, no macOS, no Windows native)

## Related Projects

- [**zcc-compiler-bug-corpus**](https://huggingface.co/datasets/zkaedi/zcc-compiler-bug-corpus) — 12 real codegen bugs with ground-truth fixes
- [**zkaedi-cc**](https://huggingface.co/spaces/zkaedi/zkaedi-cc) — 18-agent Hamiltonian C static vulnerability analyzer
- [**ZKAEDI-MINI**](https://huggingface.co/zkaedi/ZKAEDI-MINI-GGUF) — 7B parameter model (Qwen2.5 + LoRA) for compiler and smart contract analysis
- [**zcc_ir_forge**](https://github.com/zkaedi) — Async pipeline feeding ZCC's IR output to local LLMs for optimization analysis
- [**zkaedi.ai**](https://zkaedi.ai) — Smart contract auditing platform powered by ZKAEDI PRIME

## How It Works (for compiler enthusiasts)

### The Lexer (part2.c)

Hand-written lexer producing tokens for all C keywords, operators, literals (integer, string, char), and identifiers. Handles escape sequences, hex/octal literals, and multi-character operators (`<=`, `>=`, `==`, `!=`, `<<`, `>>`, `&&`, `||`, `+=`, etc.). Keywords are recognized via a lookup table with hash-based fast path.

### The Parser (part3.c)

Recursive descent parser implementing C's operator precedence through separate functions per precedence level: `parse_assign` → `parse_ternary` → `parse_logor` → `parse_logand` → `parse_bitor` → ... → `parse_unary` → `parse_postfix` → `parse_primary`. Produces an AST of `Node` structures with type annotations resolved during parsing.

### The Codegen (part4.c)

Tree-walk code generator that emits x86-64 assembly. `codegen_expr()` handles all expression nodes, `codegen_stmt()` handles all statement nodes, `codegen_func()` orchestrates function-level emission including prologue, parameter stores, body, and epilogue. Register allocation is performed by `allocate_registers()` using liveness analysis to assign callee-saved registers.

Strength reduction is built into the AST codegen: multiply by powers of 2 uses `shl`, multiply by 3/5/9 uses `lea`, divide by powers of 2 uses `sar`/`shr` (with sign correction for signed types), modulo by powers of 2 uses `and`. All signed/unsigned operations dispatch to the correct instruction variant.

### The IR Bridge (ir_bridge.h)

Translation layer that maps AST `Node` structures to `ZCCNode` structures consumable by the IR. Handles type mapping (18 C types → IR type enum), variable name extraction, binary operator mapping, and function boundary management. The bridge is the narrowest point in the pipeline — all AST information must flow through these ~186 lines.

### The Optimizer (compiler_passes.c)

7,317 lines implementing 12 optimization passes over SSA-form IR. The core data structures are `Function` (containing `Block` arrays), `Block` (doubly-linked list of `Instr`), and `Instr` (with opcode, destination register, source registers, PHI sources, and execution frequency).

Dominance is computed via the standard iterative algorithm. Dominance frontiers are used for PHI placement in the multi-block Mem2Reg pass. Liveness intervals drive the linear scan register allocator. The body emitter (`ir_asm_emit_function_body`) walks blocks in PGO-derived order, emitting x86-64 assembly with correct PHI resolution via parallel edge copies.

### The Self-Host Invariant

The compiler is correct if and only if `zcc₂.s == zcc₃.s` (byte-identical). This is a stronger property than "compiles without errors" — it means the compiler's own output, when used to compile itself, produces exactly the same assembly. Any codegen bug that affects any function in the compiler itself will break this invariant.

## License

Apache 2.0

## Author

**ZKAEDI** — [@LongNumberGeek](https://twitter.com/LongNumberGeek) — [zkaedi.ai](https://zkaedi.ai) — [HuggingFace](https://huggingface.co/zkaedi)

Built solo. Every line, every bug, every fix.
