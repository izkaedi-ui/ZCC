# 🔱 ZCC v1.0.1 — NEXT AXIS: CHEATCODE EDITION

> Each option gets 2-3 cheatcodes that dramatically accelerate progress,
> plus concrete verification commands to prove each cheatcode works.

---

## OPTION 1: Enable IR Passes Globally

**Base:** Widen the IR lowering gate so all 171 functions go through
DCE → LICM → Constant Fold → Escape Analysis → Mem2Reg → PGO reorder,
not just `ir_module_lower_x86`.

### Cheatcode 1A: Incremental Function Whitelist

Don't flip the global switch. Add a whitelist array and enable
functions one-at-a-time, smallest-first:

```c
/* In codegen_func (part4.c) — gated IR lowering */
static const char *ir_whitelist[] = {
    "is_alpha", "is_digit", "is_alnum", "is_space",  /* 4-line functions */
    "hex_val", "peek_char", "read_char",              /* simple accessors */
    "is_power_of_2_val", "log2_of",                   /* pure math */
    "new_label", "push_reg", "pop_reg",               /* emit helpers */
    NULL
};

int use_ir = 0;
for (int i = 0; ir_whitelist[i]; i++) {
    if (strcmp(func->name, ir_whitelist[i]) == 0) { use_ir = 1; break; }
}
```

Start with leaf functions (no calls, no complex control flow). Add 5-10
per session. Each batch gets a `make selfhost` gate. First failure tells
you exactly which function has an unhandled AST node type.

**Verification:**
```bash
# After adding batch N to whitelist:
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && make clean && make selfhost"
# SELF-HOST VERIFIED = batch N is clean

# Count how many functions are now IR-lowered:
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  ZCC_EMIT_IR=1 ./zcc zcc.c -o /dev/null 2>&1 | \
  grep 'emitted from IR' | wc -l"
# Track: 1 → 5 → 15 → 40 → 171
```

**PRIME mapping:** Each function is a point in the energy landscape. Leaf
functions have low H₀ (simple, unlikely to break). Complex functions
(`codegen_expr`, `parse_primary`) have high H₀. The whitelist traverses
the landscape from low-energy attractors to high-energy ridges.

---

### Cheatcode 1B: DCE Delta Oracle

You already have `zcc_ir_optimized.json` (15,402 instructions after DCE
on the full IR dump). Use it as a **regression oracle**: when you enable
IR lowering for function X, compare its DCE results against the known-good
counts from the standalone DCE pass.

```bash
# Extract per-function instruction counts from existing optimized IR:
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  python3 -c \"
import json
with open('zcc_ir_optimized.json') as f:
    data = json.load(f)
for func in data.get('functions', []):
    name = func.get('name', '?')
    count = len(func.get('instructions', []))
    if count > 0:
        print(f'{name}: {count} instrs')
\" | sort -t: -k2 -n | head -20"
```

If a function's IR-lowered instruction count diverges from the oracle by
more than 10%, something changed in the lowering — investigate before
proceeding.

**Verification:** The oracle file already exists. Just parse and compare.

---

### Cheatcode 1C: Pass Bisection on Failure

When a newly-whitelisted function breaks `make selfhost`, don't debug
blindly. Disable passes one at a time to isolate which pass introduced
the divergence:

```bash
# In compiler_passes.c, add kill switches:
# SKIP_DCE=1  SKIP_LICM=1  SKIP_FOLD=1  SKIP_MEM2REG=1

# Then bisect:
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  SKIP_LICM=1 make clean && make selfhost"
# If this passes → LICM broke it
# If this fails → LICM is innocent, try SKIP_DCE=1
```

**Verification:** Each env var disables exactly one pass. The first one
that makes `make selfhost` pass again is your culprit.

**PRIME mapping:** Pass bisection is gradient descent on the bug cascade
energy landscape. Each disabled pass removes one energy term. The pass
that zeroes the gradient is the attractor.

---

## OPTION 2: Build matrix_host.c Stress Test

**Base:** Standalone compute-heavy C file compiled through both backends,
output values compared for correctness.

### Cheatcode 2A: One Function Per Pass

Don't write a monolithic matrix library. Write one function that's a
**perfect target** for each optimization pass:

```c
/* constant_fold_target: all operations on compile-time constants */
int fold_test(void) {
    int a = 7;
    int b = 13;
    int c = a * b + 3;    /* should fold to 94 at IR level */
    int d = c / 2;        /* should fold to 47 */
    return d;             /* DCE eliminates a, b, c */
}

/* licm_target: loop-invariant computation inside hot loop */
int licm_test(int n) {
    int sum = 0;
    int i = 0;
    while (i < n) {
        int base = 42 * 17;  /* LICM should hoist this */
        sum = sum + base + i;
        i = i + 1;
    }
    return sum;
}

/* dce_target: dead assignments that should be eliminated */
int dce_test(int x) {
    int dead1 = x * 3;    /* never used → DCE kills */
    int dead2 = x + 7;    /* never used → DCE kills */
    int live = x * 2;
    return live;
}

/* regpressure_target: 8+ simultaneous live values */
int pressure_test(int a, int b, int c, int d) {
    int e = a + b;
    int f = c + d;
    int g = e * f;
    int h = a - d;
    int i = b * c;
    int j = g + h;        /* 6+ values live here */
    int k = i - j;
    int m = e + f + g + h + i + j + k;  /* all live */
    return m;
}

/* escape_target: local allocation that doesn't escape */
int escape_test(void) {
    int arr[4];           /* should be promoted to registers */
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    return arr[0] + arr[1] + arr[2] + arr[3];  /* 100 */
}
```

Each function returns a known value. The `main` function calls all of
them and prints PASS/FAIL.

**Verification:**
```bash
# AST backend
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  ./zcc matrix_host.c -o matrix_ast.s && \
  gcc -o matrix_ast matrix_ast.s && \
  ./matrix_ast"
# Expected: all PASS

# IR backend (when enabled for these functions)
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  ZCC_IR_LOWER=1 ./zcc matrix_host.c -o matrix_ir.s && \
  gcc -o matrix_ir matrix_ir.s && \
  ./matrix_ir"
# Expected: all PASS (same values)
```

---

### Cheatcode 2B: Instruction Count Diff Per Function

After compiling through both backends, count instructions per function
to measure optimization impact:

```bash
# Count instructions in each function (AST backend)
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  awk '/^[a-z_]+:/{name=\$1} /^    /{count[name]++} \
  END{for(n in count) print count[n], n}' matrix_ast.s | sort -rn"

# Same for IR backend
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  awk '/^[a-z_]+:/{name=\$1} /^    /{count[name]++} \
  END{for(n in count) print count[n], n}' matrix_ir.s | sort -rn"
```

This gives you a concrete "IR saved X instructions in function Y" metric
for every function. The delta is the optimization dividend.

**Verification:** The awk scripts work on any x86-64 assembly file.
A positive delta (AST > IR) means the passes helped. A negative delta
means the IR lowerer is generating worse code — investigate.

---

### Cheatcode 2C: Self-Checking Golden Values

Embed expected values as compile-time constants so the test binary
validates itself without any external harness:

```c
int main(void) {
    int fail = 0;
    if (fold_test() != 47)      { printf("FAIL fold\n"); fail = 1; }
    if (licm_test(100) != 71650) { printf("FAIL licm\n"); fail = 1; }
    if (dce_test(5) != 10)      { printf("FAIL dce\n"); fail = 1; }
    if (pressure_test(1,2,3,4) != 42) { printf("FAIL pressure\n"); fail = 1; }
    if (escape_test() != 100)   { printf("FAIL escape\n"); fail = 1; }
    if (!fail) printf("ALL PASS\n");
    return fail;
}
```

**Verification:** Exit code 0 = all pass. Can be added to `make test`
target for CI-style regression gating.

**PRIME mapping:** Each function is a probe into a specific region of the
optimization energy landscape. `fold_test` probes constant propagation
attractors. `licm_test` probes loop invariant fixpoints. `pressure_test`
probes the spill/reload phase boundary.

---

## OPTION 3: Codegen Parity Diff

**Base:** Pick one ZCC function, compile through both backends, diff
the assembly instruction-by-instruction.

### Cheatcode 3A: Normalized Assembly Diff

Raw diffs are noisy — different label names, different stack offsets,
different register choices all create false divergences. Normalize first:

```bash
# Strip labels, normalize registers, strip .loc directives
normalize_asm() {
    sed -E '
        /^\./d                          # strip directives
        /^[a-zA-Z_].*:/d              # strip labels
        s/%r(ax|bx|cx|dx|si|di|bp|sp)/%REG/g   # normalize regs
        s/%r[0-9]+/%REG/g
        s/-?[0-9]+\(%rbp\)/%SLOT/g    # normalize stack slots
        s/\.L[a-zA-Z0-9_]+/%LABEL/g   # normalize label refs
        /^$/d                          # strip blank lines
    ' "$1"
}

# Then diff the normalized streams:
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  normalize_asm <(sed -n '/^peephole_optimize:/,/^[a-z]/p' check.s) > /tmp/ast_norm.s && \
  normalize_asm <(sed -n '/^peephole_optimize:/,/^[a-z]/p' zcc3_ir.s) > /tmp/ir_norm.s && \
  diff /tmp/ast_norm.s /tmp/ir_norm.s | head -60"
```

What remains after normalization is **pure semantic divergence**: different
instruction selection, different operation ordering, missing or extra
operations. Every remaining diff line is meaningful.

**Verification:** Run on a known-identical function first (one that both
backends compile through the AST path). The normalized diff should be empty.
If it's not, the normalizer needs tuning.

---

### Cheatcode 3B: Three-Column Rosetta Stone

Build a three-column view: Source Line → IR Instruction → x86 Assembly.
The `.loc` directives in the assembly map back to source lines. The IR
dump maps instructions to function names. Cross-reference all three:

```bash
# Extract .loc-annotated assembly for one function
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  sed -n '/^peephole_optimize:/,/^[a-z_]*:/p' zcc3_ir.s | \
  grep -E '^\s+(\.loc|movq|addq|subq|cmpq|callq|jmp|jnz|je|leaq|xorl|pushq|popq)' | \
  head -80"
```

Each `.loc 1 NNNN` tells you which source line generated the following
instructions. You can then look at the IR dump to see what the IR
intermediate looked like for that same source line.

**Verification:** Pick line 6465 (start of `read_file`). The `.loc`
annotation in zcc3_ir.s should point to 6460-6470. The IR dump should
show the corresponding LOAD/STORE/CALL sequence. All three should tell
the same story.

---

### Cheatcode 3C: Divergence Classification Taxonomy

Not all divergences are bugs. Classify each diff line into one of four
buckets:

| Category | Meaning | Action |
|----------|---------|--------|
| **OPT-WIN** | IR backend produces fewer instructions | Celebrate |
| **OPT-MISS** | AST backend produces fewer instructions | Investigate: pass missed an opportunity |
| **NEUTRAL** | Different but equivalent (e.g., `addq` vs `leaq`) | Ignore |
| **BUG** | Semantically different (wrong value, wrong branch) | Fix immediately |

Count each category. A healthy IR backend should be 60%+ OPT-WIN,
30% NEUTRAL, <10% OPT-MISS, 0% BUG.

**Verification:** Apply to `read_file` (now that CG-IR-004 is fixed).
The parameter loads should be NEUTRAL (different instruction sequence,
same result). The `fopen` call setup should be NEUTRAL. Any BUG entries
mean a new CG-IR-00X.

**PRIME mapping:** The divergence taxonomy IS the energy landscape.
OPT-WIN = lower energy state. OPT-MISS = local minimum the passes
couldn't escape. BUG = energy spike (H₀ > 0.8). The overall distribution
is the system's temperature — a cold system (mostly OPT-WIN) is well-optimized.

---

## OPTION 4: Register Allocation Pressure Testing

**Base:** Stress-test the linear scan allocator with increasing live
variable counts, measure spill rates, decide if graph coloring is needed.

### Cheatcode 4A: Live Range Visualization

The `def_seq[]` and `last_use[]` arrays in `IRAsmCtx` already contain
everything needed to visualize live ranges. Dump them:

```c
/* Add to ir_asm_emit_function, after linear scan completes: */
fprintf(stderr, "[REGALLOC] %s: %d vregs, %d physical\n",
        fn->name, fn->n_regs, N_PHYS_REGS);
for (int r = 0; r < fn->n_regs; r++) {
    if (ctx.def_seq[r] || ctx.last_use[r])
        fprintf(stderr, "  vreg %3d: def=%4d last_use=%4d phys=%2d %s\n",
                r, ctx.def_seq[r], ctx.last_use[r],
                ctx.phys_reg[r],
                ctx.phys_reg[r] < 0 ? "SPILLED" : "");
}
```

This produces a complete spill map. Every `SPILLED` line is a register
the allocator couldn't fit. The gap between `def` and `last_use` is the
live range length — long ranges increase pressure.

**Verification:**
```bash
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  ZCC_EMIT_IR=1 ./zcc zcc.c -o /dev/null 2>&1 | \
  grep 'SPILLED' | wc -l"
# This counts total spills across all functions
# Baseline before any regalloc changes
```

---

### Cheatcode 4B: Pressure Gradient Micro-Benchmarks

Write functions with exactly N simultaneous live values. Find the
pressure cliff where spills start:

```c
/* 4 live — should fit in 6 regs, no spills */
int p4(int a, int b, int c, int d) {
    return a + b + c + d;
}

/* 6 live — exactly fills the register pool */
int p6(int a, int b) {
    int c = a * 2; int d = b * 3;
    int e = c + d; int f = a - b;
    return c + d + e + f + a + b;
}

/* 8 live — MUST spill 2 values */
int p8(int a, int b) {
    int c = a+1; int d = b+2; int e = a*3; int f = b*4;
    int g = c+d; int h = e+f;
    return a + b + c + d + e + f + g + h;
}

/* 12 live — heavy spilling */
int p12(int a, int b) {
    int c=a+1; int d=b+2; int e=a*3; int f=b*4;
    int g=c+d; int h=e+f; int i=g*2; int j=h*3;
    int k=i+j; int m=a+b;
    return c+d+e+f+g+h+i+j+k+m+a+b;
}
```

**Verification:**
```bash
# Count spill instructions (movq to/from stack) per function:
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  for fn in p4 p6 p8 p12; do \
    count=\$(sed -n \"/^\${fn}:/,/^[a-z]/p\" pressure_test.s | \
      grep -c 'movq.*(%rbp)'); \
    echo \"\$fn: \$count stack accesses\"; \
  done"
# Expected curve: p4=low, p6=moderate, p8=high, p12=very high
```

**PRIME mapping:** The spill curve IS a phase transition. Below 6 live
values = ordered phase (all in registers). Above 6 = disordered phase
(spilling). The transition point is the critical temperature. Graph
coloring would raise the critical temperature by finding better
colorings; linear scan with better heuristics would smooth the curve.

---

### Cheatcode 4C: Callee-Saved Mask Audit

Your `used_callee_saved_mask` tracks which callee-saved registers the
function actually uses. If the mask is always 0x7F (all 7 bits), the
allocator is greedily using all callee-saved regs even when not needed,
wasting prologue/epilogue push/pop cycles.

```bash
# Dump mask per function:
wsl -e sh -c "cd /mnt/h/__DOWNLOADS/selforglinux && \
  ZCC_EMIT_IR=1 ./zcc zcc.c -o /dev/null 2>&1 | \
  grep 'callee_saved_mask' | sort | uniq -c | sort -rn | head -10"
```

If most functions have mask=0x01 (just rbx), the allocator is efficient.
If most have mask=0x7F, it's burning push/pop cycles unnecessarily.
The fix: order register allocation to prefer caller-saved scratch regs
(r10, r11) before touching callee-saved (rbx, r12-r15).

**Verification:** Count total `pushq`/`popq` instructions in the
generated assembly. After reordering preference: total should drop.

---

## SUMMARY MATRIX

| Option | Cheatcodes | Risk | Sessions | Compound Value |
|--------|-----------|------|----------|----------------|
| 1: Global IR Passes | Whitelist, DCE Oracle, Pass Bisection | Medium-High | 2-4 | 🔱🔱🔱🔱🔱 (every function optimized) |
| 2: matrix_host.c | One-Per-Pass, Instr Count, Self-Check | Low | 1 | 🔱🔱🔱 (regression suite + metrics) |
| 3: Codegen Parity | Normalized Diff, Rosetta, Taxonomy | Low | 1-2 | 🔱🔱🔱 (quality map + bug detector) |
| 4: Regalloc Pressure | Live Range Viz, Pressure Gradient, Mask Audit | Medium | 2-3 | 🔱🔱🔱🔱 (perf gains in hot loops) |

**Maximum compound path:** 2 → 3 → 1 → 4
- Session 1: matrix_host.c with self-checking golden values (2A+2C)
- Session 2: Codegen parity on matrix functions (3A+3C)
- Sessions 3-5: Whitelist IR functions using matrix suite as gate (1A+1B)
- Sessions 6-7: Regalloc tuning on the pressure benchmarks (4A+4B)
