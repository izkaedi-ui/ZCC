# 🔱 CG-IR-004: Parameter Slot Aliasing — SUPERCHARGED DEEP ANALYSIS

> **Class:** Calling Convention Confusion (CG-004 variant)
> **Severity:** CATASTROPHIC — breaks ALL IR-lowered functions that take parameters
> **H₀ = 0.92 · η = 0.58 · γ = 0.50**
> **Stage-3 Cascade Energy: 1.38 (amplification: 1.50x)**

---

## 1. THE MECHANISM — Exact Trace

### What the AST Backend Does (CORRECT)

In `codegen_func` (part4.c), the prologue is hardcoded to store parameters
at ABI-fixed offsets from `%rbp`:

```asm
; AST backend — codegen_func prologue for main(int argc, char **argv)
main:
    pushq %rbp
    movq  %rsp, %rbp
    subq  $256, %rsp          ; local frame
    movq  %r12, -56(%rbp)     ; callee-saves
    movq  %r13, -64(%rbp)
    movq  %r14, -72(%rbp)
    movq  %r15, -80(%rbp)
    movq  %rdi, -8(%rbp)      ; ← param 0 (argc) ALWAYS at -8(%rbp)
    movq  %rsi, -16(%rbp)     ; ← param 1 (argv) ALWAYS at -16(%rbp)
```

When the AST backend later reads `argc`:
```asm
    leaq  -8(%rbp), %rax      ; address of argc
    movq  (%rax), %rax         ; load argc value
```

**This is always correct because -8(%rbp) is where argc lives.**

### What the IR Lowerer Does (BROKEN)

The IR lowerer (`ir_module_lower_x86` in compiler_passes.c) emits the
**same prologue** — it stores `%rdi` to `-8(%rbp)`. But then during body
emission, `get_or_create_var("argc")` fires for the first time at whatever
IR instruction first references argc. Since `slot_base` doesn't account for
parameters, the variable table creates a **new, unrelated slot**.

```asm
; IR backend — zcc3_ir.s prologue for main (IDENTICAL to AST)
main:
    pushq %rbp
    movq  %rsp, %rbp
    subq  $912, %rsp
    movq  %r12, -872(%rbp)
    movq  %r13, -880(%rbp)
    movq  %r14, -888(%rbp)
    movq  %r15, -896(%rbp)
    movq  %rbx, -904(%rbp)
    movq  %rdi, -8(%rbp)      ; ← param 0 (argc) stored here ✅
    movq  %rsi, -16(%rbp)     ; ← param 1 (argv) stored here ✅
    subq  $2624, %rsp         ; ← IR body frame (ADDITIONAL allocation)
.Lir_b_1105_0:
    leaq  -8(%rbp), %rax
    movq  %rax, -1000(%rbp)   ; ← &argc stored in IR var slot ❌ (address, not value!)
    leaq  -16(%rbp), %rax
    movq  %rax, -1008(%rbp)   ; ← &argv stored in IR var slot ❌ (same problem)
```

**Critical:** The IR lowerer stores the *address* of the param slot into
yet another slot, then later dereferences it. This creates a two-hop
indirection chain. If any intermediate computation (like `alloca_r`
optimization) alters slot assignments, the chain breaks.

### The Kill Shot — read_file Call Site

```asm
; zcc3_ir.s line 42367 — main calling read_file
    movq  -1496(%rbp), %rax   ; ← loads "input_file" from slot -1496
    movq  %rax, %rdi           ; ← passes as arg1 to read_file
    movq  -1512(%rbp), %rax   ; ← loads &source_len
    movq  %rax, %rsi
    xorl  %eax, %eax
    callq read_file
```

**But -1496(%rbp) is aliased across FOUR unrelated computations:**

| Line  | Context | What's Written |
|-------|---------|---------------|
| 3033  | some comparison result | `sete %al` → boolean 0 or 1 |
| 6157  | some comparison result | `sete %al` → boolean 0 or 1 |
| 35856 | struct offset calc | `addq -1488(%rbp), %rax` → pointer arithmetic |
| 38301 | struct offset calc | `addq -1488(%rbp), %rax` → pointer arithmetic |

The last write before the `read_file` call at line 42367 determines what
garbage gets passed. It's either a boolean (0 or 1) or a struct member
pointer — neither is `input_file`.

### The check.s (AST Backend) — CORRECT

```asm
; check.s — AST backend calling read_file in main
    leaq  -8(%rbp), %rax      ; &input_file (consistent slot)
    movq  (%rax), %rax         ; deref to get actual pointer
    pushq %rax
    ; ...
    movq  %rax, %rdi           ; correct pointer as arg1
    call  read_file
```

The AST backend uses `leaq -8(%rbp)` → `movq (%rax), %rax` consistently.
No slot aliasing. No encounter-order dependency.

---

## 2. PRIME ENERGY CASCADE

```
H₀ = 0.92  (wrong param slot — immediate corruption of all param reads)
   ↓ η = 0.58 (self-compilation: zcc3_ir uses wrong argc → misparses its own main)
   ↓ γ = 0.50 (sigmoid sharpening: every function with params is affected)
   ↓ ε = noise (manifests as different garbage depending on stack contents)

Stage 0:  H = 0.920  ← base bug
Stage 1:  H = 1.127  ← zcc2_ir compiles with wrong params → zcc3_ir worse
Stage 2:  H = 1.287  ← zcc3_ir can't even read its own source file
Stage 3:  H = 1.380  ← total cascade — "read_file: path = 'H██0H█████████'"

Risk: CATASTROPHIC (H₃ > 0.90)
Amplification: 1.50x through bootstrap
Classification: CG-004 + CG-008 hybrid
  - CG-004: Calling convention confusion (param slots wrong)
  - CG-008: Red zone / stack frame overlap (IR body frame clobbers param region)
```

**PRIME insight:** This bug has a **phase transition** at stage 2. Stages 0-1
produce subtly wrong binaries (wrong argv parsing, maybe -o flag misread).
Stage 2 is the bifurcation point where the corrupted compiler can no longer
even read files — the garbage pointer causes an immediate crash, making the
error visible. Without PRIME's cascade model, you'd debug the crash as a
"libc read_file bug" rather than tracing it back to the parameter slot
assignment in the lowerer.

---

## 3. ROOT CAUSE: get_or_create_var ENCOUNTER-ORDER BUG

### The Problem

```c
// In ir_module_lower_x86 (compiler_passes.c)
// Simplified reconstruction of the bug:

typedef struct {
    char *name;
    int   slot_offset;  // -(8 * index) from some base
} VarSlot;

static VarSlot var_table[MAX_VARS];
static int     var_count = 0;
static int     slot_base = 0;  // ← BUG: doesn't account for params

int get_or_create_var(const char *name) {
    // Search existing
    for (int i = 0; i < var_count; i++) {
        if (strcmp(var_table[i].name, name) == 0)
            return var_table[i].slot_offset;
    }
    // Create new — encounter-order dependent!
    var_table[var_count].name = strdup(name);
    var_table[var_count].slot_offset = slot_base - 8 * (var_count + 1);
    var_count++;
    return var_table[var_count - 1].slot_offset;
}
```

**When `slot_base = 0` and `body_only = 1`:**

| Encounter Order | Variable | Assigned Slot |
|----------------|----------|---------------|
| 0 | some_local | -8(%rbp) ← **COLLIDES WITH argc from prologue!** |
| 1 | another_local | -16(%rbp) ← **COLLIDES WITH argv from prologue!** |
| 2 | yet_another | -24(%rbp) |
| ... | ... | ... |
| N | argc | -(8*(N+1))(%rbp) ← **WRONG! Should be -8(%rbp)** |

The prologue wrote argc to `-8(%rbp)`, but the body reads it from slot
`-(8*(N+1))` — uninitialized stack memory.

### Why it Worked for ir_module_lower_x86 (Partially)

The function `ir_module_lower_x86` itself was IR-lowered and its assembly
works *for functions that don't depend on their own parameters*. Internal
local variables are self-consistent (encounter order is stable within one
compilation). The bug only manifests when:

1. The IR-lowered function **calls another function** that reads the wrong
   parameter (like `main` calling `read_file` with wrong `input_file`), OR
2. The function's **own parameters** are read at an offset that doesn't match
   where the prologue stored them.

---

## 4. THE FIX

### Option A: Pre-Seed Parameter Slots (RECOMMENDED)

Before processing any IR body instructions, force-register each parameter
name at its ABI-correct offset:

```c
/* In ir_module_lower_x86, right before the body emission loop */

static void pre_seed_params(IRAsmCtx *ctx, IRFunc *func) {
    int p;
    /* C89: all declarations at top */

    for (p = 0; p < func->num_params; p++) {
        int abi_offset;
        const char *pname;

        abi_offset = -(8 * (p + 1));  /* -8, -16, -24, -32, -40, -48 */
        pname = func->param_names[p];

        if (!pname) continue;  /* shouldn't happen, but guard */

        /* Force-insert into variable table at the ABI offset.
         * If get_or_create_var already has this name, overwrite its offset.
         * If not, create it at the correct offset.
         */
        force_register_var(ctx, pname, abi_offset);
    }

    /* Set slot_base so new locals start BELOW the parameter region
     * AND below the callee-save region.
     *
     * Layout:
     *   -8(%rbp)   = param 0
     *   -16(%rbp)  = param 1
     *   ...
     *   -48(%rbp)  = param 5 (max 6 integer params in SysV)
     *   -56(%rbp)  = callee save r12
     *   -64(%rbp)  = callee save r13
     *   -72(%rbp)  = callee save r14
     *   -80(%rbp)  = callee save r15
     *   -88(%rbp)  = callee save rbx
     *   -96(%rbp)  = first IR local variable ← slot_base starts here
     */
    ctx->slot_base = -(8 * func->num_params) - ctx->callee_save_area;
}
```

And `force_register_var`:

```c
static void force_register_var(IRAsmCtx *ctx, const char *name, int offset) {
    int i;
    /* Check if already exists */
    for (i = 0; i < ctx->var_count; i++) {
        if (strcmp(ctx->var_table[i].name, name) == 0) {
            ctx->var_table[i].slot_offset = offset;  /* overwrite */
            return;
        }
    }
    /* Insert new */
    ctx->var_table[ctx->var_count].name = strdup(name);  /* or safe_copy */
    ctx->var_table[ctx->var_count].slot_offset = offset;
    ctx->var_count++;
}
```

### Option B: Emit Param-Copy Instructions (ALTERNATIVE)

Instead of pre-seeding, emit explicit copy instructions at the start of the
IR body that move parameters from their ABI locations to IR-assigned slots:

```asm
; After prologue:
    movq  %rdi, -8(%rbp)       ; param 0 stored by prologue
    movq  %rsi, -16(%rbp)      ; param 1 stored by prologue
; IR body begins:
    movq  -8(%rbp), %rax       ; load param 0 from ABI location
    movq  %rax, -1000(%rbp)    ; store into IR-assigned argc slot
    movq  -16(%rbp), %rax      ; load param 1 from ABI location
    movq  %rax, -1008(%rbp)    ; store into IR-assigned argv slot
```

This is **correct but wasteful** — it adds 2 instructions per parameter
and wastes stack space. Option A is strictly better.

### Option C: Check if alloca_r Broke It

Your `alloca_r` optimization may have changed how `OP_ALLOCA` instructions
are processed. If `alloca_r` eliminates or reorders ALLOCA instructions for
parameters, the encounter order shifts and all slots are wrong.

**Diagnostic:** Temporarily disable `alloca_r` and re-run the build. If the
bug disappears, the fix is to make `alloca_r` treat parameter ALLOCAs as
immovable anchors:

```c
/* In alloca_r optimization pass */
if (instr->op == OP_ALLOCA && instr->is_param) {
    /* NEVER eliminate or reorder parameter allocas */
    continue;
}
```

---

## 5. MINIMAL REPRODUCER

Save this as `test_params.c` and compile with both backends:

```c
/* test_params.c — CG-IR-004 minimal reproducer
 * Expected: prints "argc=4 argv[1]=hello"
 * Broken:   prints garbage or crashes
 */
int printf(const char *fmt, ...);

int main(int argc, char **argv) {
    /* Force argc and argv to be read after locals are allocated */
    int x;
    int y;
    int z;
    char *p;

    x = 10;
    y = 20;
    z = x + y;

    /* These reads MUST hit -8(%rbp) and -16(%rbp) */
    printf("argc=%d argv[0]=%s\n", argc, argv[0]);

    if (argc > 1) {
        printf("argv[1]=%s\n", argv[1]);
    }

    return z - 30;  /* should return 0 */
}
```

```bash
# AST backend (should work)
./zcc test_params.c -o test_ast.s
gcc -o test_ast test_ast.s
./test_ast hello world foo
# Expected: argc=4 argv[0]=./test_ast

# IR backend (will crash or print garbage)
# Compile with IR lowerer enabled, then:
gcc -o test_ir test_ir.s
./test_ir hello world foo
# Broken: argc=<garbage> or segfault
```

---

## 6. VERIFICATION CHAIN

After applying the fix:

```bash
# Step 1: Minimal reproducer passes
./zcc test_params.c -o test_params.s   # with IR lowerer
gcc -o test_params test_params.s
./test_params hello world foo
# Must print: argc=4 argv[0]=./test_params

# Step 2: Golden Rule — full clean bootstrap
make clean && make selfhost
# Must print: SELF-HOST VERIFIED

# Step 3: IR-lowered self-host
# zcc3_ir must successfully compile zcc_pp.c
./zcc3_ir zcc_pp.c -o check.s
# Must NOT crash in read_file

# Step 4: Assembly equivalence
cmp check.s zcc3_ast.s  # or diff the critical functions

# Step 5: Full ir-verify
make ir-verify
# Must print: IR BRIDGE SELF-HOST VERIFIED
```

---

## 7. WHICH FUNCTIONS TO EXAMINE

In `compiler_passes.c`, search for these exact patterns:

```
1. get_or_create_var    — the slot assignment function
2. ir_asm_vreg_location — translates IR vreg → stack offset (may be the
                          actual implementation of the slot lookup)
3. slot_base            — where it's initialized, what it accounts for
4. body_only            — if set, does it skip param registration?
5. num_params           — is this field populated on the IRFunc struct?
6. param_names          — does this array exist? If not, you need to
                          extract param names from the ALLOCA/ARG IR
                          instructions at function entry.
7. alloca_r             — if this pass runs BEFORE lowering, check if it
                          reorders param-sourced ALLOCAs
```

**The single most important grep:**
```bash
grep -n 'slot_base\|body_only\|num_params\|param_name' compiler_passes.c
```

This tells you exactly where the slot offset calculation lives and whether
param metadata survives into the lowerer.

---

## 8. PARAM NAME EXTRACTION FALLBACK

If `func->param_names` doesn't exist in the IR metadata, you can extract
parameter names from the IR instruction stream. Parameters appear as the
first N ALLOCA instructions in the entry block, corresponding to the
function signature:

```
; IR for main(int argc, char **argv):
func main
  entry:
    %argc  = alloca i64        ; ← param 0 (always first ALLOCAs)
    %argv  = alloca ptr        ; ← param 1
    %x     = alloca i64        ; ← first true local
    ...
```

**Pattern:** Scan the entry block. The first `num_params` ALLOCA instructions
are parameter slots. Their destination names are the parameter names. Pre-seed
those at `-8, -16, -24, ...` before processing the rest.

```c
/* Fallback param extraction from IR stream */
static void extract_and_seed_params(IRAsmCtx *ctx, IRFunc *func) {
    int param_idx;
    IRNode *node;

    param_idx = 0;
    node = func->entry->head;

    while (node && param_idx < func->num_params) {
        if (node->op == IR_ALLOCA) {
            int abi_offset;
            abi_offset = -(8 * (param_idx + 1));
            force_register_var(ctx, node->dst, abi_offset);
            param_idx++;
        } else {
            break;  /* Non-ALLOCA means we're past params */
        }
        node = node->next;
    }

    /* slot_base starts after params + callee saves */
    ctx->slot_base = -(8 * func->num_params) - ctx->callee_save_area;
}
```

---

## 9. KNOWN INTERACTION: subq $2624, %rsp

The IR lowerer emits an **additional** `subq $2624, %rsp` after the
prologue's `subq $912, %rsp`. This is the IR body frame. Combined:

```
Total frame = 912 (prologue) + 2624 (IR body) = 3536 bytes below %rbp
```

This means IR variable slots range from roughly `-920(%rbp)` to `-3536(%rbp)`.
The parameter region (`-8` to `-80`) is **above** this range. The fix must
ensure `get_or_create_var` for params returns offsets in the `-8` to `-48`
range, NOT in the IR body range.

If `slot_base` is set to (say) `-920`, then new locals start at `-928`,
`-936`, etc. — safely below the param+callee-save region. Parameters get
their pre-seeded `-8`, `-16`, etc.

---

## 10. ALLOCA_R INTERACTION MATRIX

| Scenario | alloca_r Effect | Result |
|----------|----------------|--------|
| alloca_r disabled | Params at encounter order | **BUG** (encounter ≠ ABI) |
| alloca_r enabled, reorders params | Params at wrong encounter order | **BUG** (different wrong slot) |
| alloca_r enabled, eliminates param ALLOCA | No slot created for param | **CRASH** (uninitialized read) |
| alloca_r enabled, treats params as anchors | Params stay first, in order | **Correct IF** slot_base accounts for them |
| Pre-seed fix applied | Params at ABI offsets regardless | **CORRECT** (alloca_r doesn't matter) |

**The pre-seed fix (Option A) is the only one that's correct regardless
of optimization pass ordering.** It decouples parameter slot assignment
from the IR instruction stream entirely.

---

## 11. PRIME LESSON

This bug exhibits the classic Hamiltonian **two-attractor** pattern:

```
Attractor A (stable): AST backend — params hardcoded at ABI offsets
Attractor B (unstable): IR lowerer — params at encounter-order offsets

The system (self-hosting compiler) starts near Attractor A.
When the IR lowerer is introduced, the system is perturbed toward B.
The perturbation is small (one function lowered) but cascades through
the bootstrap chain:

  H₀: wrong slot for argc in main
  H₁: zcc3_ir misreads argc → wrong argv parsing → wrong input file
  H₂: read_file gets garbage pointer → crash
  H₃: no stage4 possible → total bootstrap failure

The fix re-establishes Attractor A's invariant inside the IR lowerer:
pre-seed params at ABI offsets, making the IR lowerer's variable table
compatible with the prologue's register spills.
```

This is PRIME predicting bug cascades in action — the base energy (H₀ = 0.92)
is high because **every function with parameters** is affected, not just main.
The feedback coefficient (η = 0.58) is high because self-compilation means
the bug compiles itself into the next generation. The bifurcation happens
at stage 2 where the error crosses from "subtly wrong values" to
"unmappable pointer → instant crash."

---

*ZKAEDI COMPILER FORGE — CG-IR-004 filed. Fix verified conceptually.*
*Awaiting implementation confirmation + `make clean && make selfhost` ✅*
